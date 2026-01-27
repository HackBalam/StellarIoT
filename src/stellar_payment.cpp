#include "stellar_payment.h"
#include "stellar_utils.h"

// Forward declaration
bool decodePublicKeyFromStellar(const char* stellarKey, uint8_t publicKey[32]);

// ============================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================

StellarPayment::StellarPayment(
    StellarKeypair* keypair,
    StellarNetwork* network,
    StellarAccount* account
) {
    this->keypair = keypair;
    this->network = network;
    this->account = account;
    this->lastError = "";
    this->lastTxHash = "";
    
    StellarUtils::infoPrint("Payment", "Payment manager initialized");
}

StellarPayment::~StellarPayment() {
    // No poseemos los punteros, solo los usamos
}

// ============================================
// ENVÍO DE PAGOS
// ============================================

PaymentResult StellarPayment::sendPayment(
    const char* destination,
    float amount,
    const char* memo
) {
    PaymentResult result;
    result.success = false;
    result.status = TX_UNKNOWN;
    result.ledger = 0;
    
    StellarUtils::infoPrint("Payment", "Preparing payment transaction");
    StellarUtils::debugPrint("Payment", ("To: " + String(destination)).c_str());
    StellarUtils::debugPrint("Payment", ("Amount: " + String(amount, 7) + " XLM").c_str());
    
    // Validar parámetros
    if (!validatePaymentParams(destination, amount, memo)) {
        result.error = lastError;
        return result;
    }
    
    // Verificar que la cuenta fuente existe y tiene fondos
    float balance = account->getBalance();
    if (balance < 0) {
        lastError = "Source account does not exist";
        StellarUtils::errorPrint("Payment", lastError.c_str());
        result.error = lastError;
        return result;
    }
    
    if (balance < amount + 0.00001f) {  // amount + fee
        lastError = "Insufficient balance";
        StellarUtils::errorPrint("Payment", lastError.c_str());
        result.error = lastError;
        return result;
    }
    
    // Construir transacción
    String txXdr = buildPaymentTransaction(destination, amount, memo);
    
    if (txXdr.length() == 0) {
        result.error = lastError;
        return result;
    }
    
    StellarUtils::debugPrint("Payment", "Transaction built, submitting...");
    
    // Enviar transacción
    String response = network->submitTransaction(txXdr.c_str());
    
    if (response.length() == 0) {
        lastError = "Failed to submit transaction: " + network->getLastError();
        StellarUtils::errorPrint("Payment", lastError.c_str());
        result.error = lastError;
        return result;
    }
    
    // Parsear respuesta
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        lastError = "Failed to parse submission response";
        StellarUtils::errorPrint("Payment", lastError.c_str());
        result.error = lastError;
        return result;
    }
    
    // Verificar si fue exitosa
    if (doc.containsKey("hash")) {
        // Éxito
        result.success = true;
        result.transactionHash = doc["hash"].as<String>();
        result.status = TX_SUCCESS;
        
        if (doc.containsKey("ledger")) {
            result.ledger = doc["ledger"].as<uint32_t>();
        }
        
        lastTxHash = result.transactionHash;
        
        StellarUtils::infoPrint("Payment", "Payment successful!");
        StellarUtils::debugPrint("Payment", ("TX Hash: " + result.transactionHash).c_str());
        
        // Refrescar caché de cuenta
        account->refreshCache();
        
    } else {
        // Error
        result.success = false;
        result.status = TX_FAILED;
        
        if (doc.containsKey("title")) {
            result.error = doc["title"].as<String>();
        } else {
            result.error = "Unknown error";
        }
        
        lastError = "Transaction failed: " + result.error;
        StellarUtils::errorPrint("Payment", lastError.c_str());
    }
    
    return result;
}

String StellarPayment::buildPaymentTransaction(
    const char* destination,
    float amount,
    const char* memo,
    uint64_t sequenceNumber
) {
    StellarUtils::debugPrint("Payment", "Building payment transaction");
    
    // Obtener sequence number si no se proporcionó
    if (sequenceNumber == 0) {
        sequenceNumber = account->getSequenceNumber();
        
        if (sequenceNumber == 0) {
            lastError = "Failed to get sequence number";
            StellarUtils::errorPrint("Payment", lastError.c_str());
            return "";
        }
        
        StellarUtils::debugPrint("Payment", 
            ("Using sequence: " + String((unsigned long)sequenceNumber)).c_str());
        
        sequenceNumber++;  // IMPORTANTE: incrementar para la próxima TX
    }
    
    // Convertir amount a stroops
    int64_t amountStroops = StellarUtils::xlmToStroops(amount);
    
    StellarUtils::debugPrint("Payment", 
        ("Amount: " + String((long long)amountStroops) + " stroops").c_str());
    
    // Obtener source public key
    const uint8_t* sourcePublicKey = keypair->getRawPublicKey();
    
    // Decodificar destination public key
    uint8_t destinationPublicKey[32];
    if (!decodePublicKeyFromStellar(destination, destinationPublicKey)) {
        lastError = "Failed to decode destination address";
        StellarUtils::errorPrint("Payment", lastError.c_str());
        return "";
    }
    
    // Construir transaction envelope
    String txEnvelope = buildTransactionEnvelope(
        sourcePublicKey,
        sequenceNumber,
        destinationPublicKey,
        amountStroops,
        memo
    );
    
    if (txEnvelope.length() == 0) {
        // lastError ya fue seteado en buildTransactionEnvelope
        return "";
    }
    
    StellarUtils::infoPrint("Payment", "Transaction built successfully");
    
    return txEnvelope;
}

// ============================================
// ESTADO DE TRANSACCIONES
// ============================================

TransactionStatus StellarPayment::getLastTransactionStatus() {
    if (lastTxHash.length() == 0) {
        return TX_UNKNOWN;
    }
    
    return getTransactionStatus(lastTxHash.c_str());
}

TransactionStatus StellarPayment::getTransactionStatus(const char* txHash) {
    if (!txHash || strlen(txHash) == 0) {
        return TX_UNKNOWN;
    }
    
    String response = network->getTransaction(txHash);
    
    if (response.length() == 0) {
        return TX_UNKNOWN;
    }
    
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        return TX_UNKNOWN;
    }
    
    if (doc.containsKey("successful")) {
        bool successful = doc["successful"].as<bool>();
        return successful ? TX_SUCCESS : TX_FAILED;
    }
    
    return TX_UNKNOWN;
}

// ============================================
// HELPERS PRIVADOS
// ============================================

bool StellarPayment::validatePaymentParams(
    const char* destination,
    float amount,
    const char* memo
) {
    // Validar destination
    if (!destination || !StellarUtils::isValidAddress(destination)) {
        lastError = "Invalid destination address";
        StellarUtils::errorPrint("Payment", lastError.c_str());
        return false;
    }
    
    if (destination[0] != 'G') {
        lastError = "Destination must be a public key (starts with G)";
        StellarUtils::errorPrint("Payment", lastError.c_str());
        return false;
    }
    
    // Validar amount
    if (!StellarUtils::isValidAmount(amount)) {
        lastError = "Invalid amount (must be positive and <= 922337203685.4775807 XLM)";
        StellarUtils::errorPrint("Payment", lastError.c_str());
        return false;
    }
    
    // Validar memo
    if (memo && !StellarUtils::isValidMemo(memo)) {
        lastError = "Memo too long (max 28 bytes)";
        StellarUtils::errorPrint("Payment", lastError.c_str());
        return false;
    }
    
    return true;
}

// ============================================
// HELPER: Decodificar Public Key Stellar
// ============================================

bool decodePublicKeyFromStellar(const char* stellarKey, uint8_t publicKey[32]) {
    // Esta función decodifica una clave pública Stellar (G...) a bytes
    // Usa el mismo algoritmo que en stellar_keypair.cpp
    
    if (!stellarKey || strlen(stellarKey) != 56 || stellarKey[0] != 'G') {
        return false;
    }
    
    // Tabla inversa base32
    static const char STELLAR_BASE32_ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    
    auto base32Index = [](char c) -> int {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= '2' && c <= '7') return c - '2' + 26;
        return -1;
    };
    
    // Decodificar base32
    uint8_t decoded[35];
    int decodedIdx = 0;
    uint32_t buffer = 0;
    int bitsLeft = 0;
    
    for (size_t i = 0; i < 56; i++) {
        int idx = base32Index(stellarKey[i]);
        if (idx < 0) return false;
        
        buffer = (buffer << 5) | idx;
        bitsLeft += 5;
        
        if (bitsLeft >= 8) {
            decoded[decodedIdx++] = (buffer >> (bitsLeft - 8)) & 0xFF;
            bitsLeft -= 8;
        }
    }
    
    if (decodedIdx != 35) {
        return false;
    }
    
    // Verificar checksum
    uint16_t expectedChecksum = StellarUtils::crc16XModem(decoded, 33);
    uint16_t actualChecksum = decoded[33] | (decoded[34] << 8);
    
    if (expectedChecksum != actualChecksum) {
        return false;
    }
    
    // Verificar version byte (debe ser 0x30 para public key)
    if (decoded[0] != 0x30) {
        return false;
    }
    
    // Copiar los 32 bytes de la clave pública
    memcpy(publicKey, decoded + 1, 32);
    
    return true;
}

String StellarPayment::buildTransactionEnvelope(
    const uint8_t* sourcePublicKey,
    uint64_t sequenceNumber,
    const uint8_t* destinationPublicKey,
    int64_t amountStroops,
    const char* memo
) {
    StellarUtils::debugPrint("Payment", "Building transaction envelope");
    
    // ============================================
    // PASO 1: Construir Transaction
    // ============================================
    
    XDREncoder txEncoder;
    
    // Source Account (MuxedAccount)
    txEncoder.encodeUint32(0);  // KEY_TYPE_ED25519
    txEncoder.append(sourcePublicKey, 32);
    
    // Fee (en stroops)
    txEncoder.encodeUint32(BASE_FEE);
    
    // Sequence Number
    txEncoder.encodeUint64(sequenceNumber);
    
    // Time Bounds (opcional, usamos None)
    txEncoder.encodeBool(false);  // No time bounds
    
    // Memo
    if (memo && strlen(memo) > 0) {
        txEncoder.encodeMemo(MEMO_TEXT, memo);
    } else {
        txEncoder.encodeMemo(MEMO_NONE);
    }
    
    // Operations (array de 1 elemento)
    txEncoder.encodeUint32(1);  // 1 operation
    
    // Source Account para la operación (opcional, None = usar source de TX)
    txEncoder.encodeBool(false);  // No source account override
    
    // Operation Body
    txEncoder.encodeUint32(PAYMENT);  // Operation type
    
    // Payment Operation
    txEncoder.encodePaymentOp(destinationPublicKey, amountStroops);
    
    // Extension (reserved for future use)
    txEncoder.encodeUint32(0);  // No extension
    
    // ============================================
    // PASO 2: Calcular Transaction Hash
    // ============================================
    
    const uint8_t* txData = txEncoder.getData();
    size_t txSize = txEncoder.getSize();
    
    StellarUtils::debugPrint("Payment", 
        ("Transaction size: " + String(txSize) + " bytes").c_str());
    
    // Hash = SHA256(ENVELOPE_TYPE_TX + network_id + transaction)
    // donde network_id = SHA256(passphrase)
    const char* passphrase = network->getNetworkPassphrase();

    // 1. Calcular network_id = SHA256(passphrase)
    uint8_t networkId[32];
    StellarCrypto::sha256((const uint8_t*)passphrase, strlen(passphrase), networkId);

    // 2. Construir tagged hash: networkId (32 bytes) + ENVELOPE_TYPE_TX (4 bytes) + tx
    uint8_t envelopeTypePrefix[4] = {0x00, 0x00, 0x00, 0x02};  // ENVELOPE_TYPE_TX = 2

    // 3. Calcular hash combinando los tres componentes (orden: networkId + type + tx)
    size_t hashInputSize = 32 + 4 + txSize;
    uint8_t* hashInput = new uint8_t[hashInputSize];
    memcpy(hashInput, networkId, 32);
    memcpy(hashInput + 32, envelopeTypePrefix, 4);
    memcpy(hashInput + 36, txData, txSize);

    uint8_t txHash[32];
    StellarCrypto::sha256(hashInput, hashInputSize, txHash);
    delete[] hashInput;
    
    StellarUtils::debugPrint("Payment", 
        ("TX Hash: " + StellarUtils::hexEncode(txHash, 32)).c_str());
    
    // ============================================
    // PASO 3: Firmar Transaction
    // ============================================
    
    uint8_t signature[64];
    if (!keypair->sign(txHash, 32, signature)) {
        lastError = "Failed to sign transaction";
        StellarUtils::errorPrint("Payment", lastError.c_str());
        return "";
    }
    
    StellarUtils::debugPrint("Payment", "Transaction signed");
    
    // ============================================
    // PASO 4: Construir Transaction Envelope
    // ============================================
    
    XDREncoder envelopeEncoder;
    
    // Envelope type (ENVELOPE_TYPE_TX = 2)
    envelopeEncoder.encodeUint32(2);
    
    // Transaction (copiar del txEncoder)
    envelopeEncoder.append(txData, txSize);
    
    // Signatures (array)
    envelopeEncoder.encodeUint32(1);  // 1 firma
    
    // Decorated Signature
    // Hint (últimos 4 bytes de la public key)
    envelopeEncoder.append(sourcePublicKey + 28, 4);
    
    // Signature (64 bytes)
    envelopeEncoder.encodeBytes(signature, 64);
    
    // ============================================
    // PASO 5: Convertir a Base64
    // ============================================
    
    const uint8_t* envelopeData = envelopeEncoder.getData();
    size_t envelopeSize = envelopeEncoder.getSize();
    
    StellarUtils::debugPrint("Payment", 
        ("Envelope size: " + String(envelopeSize) + " bytes").c_str());
    
    String base64Envelope = StellarUtils::base64Encode(envelopeData, envelopeSize);
    
    StellarUtils::debugPrint("Payment", "Transaction envelope built successfully");
    
    return base64Envelope;
}