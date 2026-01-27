#include "stellar_keypair.h"
#include "stellar_utils.h"

// Alfabeto Base32 de Stellar (RFC 4648)
static const char STELLAR_BASE32_ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

// Tabla inversa para decodificación
static int base32Index(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= '2' && c <= '7') return c - '2' + 26;
    return -1;
}

// Wordlist BIP39 simplificada (primeras 128 palabras para demo)
// En producción usar la lista completa de 2048 palabras
static const char* BIP39_WORDLIST[] = {
    "abandon", "ability", "able", "about", "above", "absent", "absorb", "abstract",
    "absurd", "abuse", "access", "accident", "account", "accuse", "achieve", "acid",
    "acoustic", "acquire", "across", "act", "action", "actor", "actress", "actual",
    "adapt", "add", "addict", "address", "adjust", "admit", "adult", "advance",
    "advice", "aerobic", "affair", "afford", "afraid", "again", "age", "agent",
    "agree", "ahead", "aim", "air", "airport", "aisle", "alarm", "album",
    "alcohol", "alert", "alien", "all", "alley", "allow", "almost", "alone",
    "alpha", "already", "also", "alter", "always", "amateur", "amazing", "among",
    "amount", "amused", "analyst", "anchor", "ancient", "anger", "angle", "angry",
    "animal", "ankle", "announce", "annual", "another", "answer", "antenna", "antique",
    "anxiety", "any", "apart", "apology", "appear", "apple", "approve", "april",
    "arch", "arctic", "area", "arena", "argue", "arm", "armed", "armor",
    "army", "around", "arrange", "arrest", "arrive", "arrow", "art", "artefact",
    "artist", "artwork", "ask", "aspect", "assault", "asset", "assist", "assume",
    "asthma", "athlete", "atom", "attack", "attend", "attitude", "attract", "auction",
    "audit", "august", "aunt", "author", "auto", "autumn", "average", "avocado"
};

static const int BIP39_WORDLIST_SIZE = 128;

// ============================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================

StellarKeypair::StellarKeypair() : hasMnemonic(false) {
    memset(publicKey, 0, 32);
    memset(secretKey, 0, 32);
}

StellarKeypair::~StellarKeypair() {
    // Limpiar memoria sensible de forma segura
    StellarUtils::secureZero(secretKey, 32);
    mnemonicPhrase = "";
}

// ============================================
// CREACIÓN E IMPORTACIÓN
// ============================================

StellarKeypair* StellarKeypair::generate() {
    StellarUtils::infoPrint("Keypair", "Generating new keypair");
    
    StellarKeypair* kp = new StellarKeypair();
    
    // Generar keypair Ed25519
    if (!StellarCrypto::generateKeypair(kp->publicKey, kp->secretKey)) {
        delete kp;
        return nullptr;
    }
    
    // Generar mnemonic para backup
    kp->generateMnemonic();
    kp->hasMnemonic = true;
    
    StellarUtils::infoPrint("Keypair", "Keypair generated successfully");
    StellarUtils::debugPrint("Keypair", ("Public: " + kp->getPublicKey()).c_str());
    
    return kp;
}

StellarKeypair* StellarKeypair::fromSecret(const char* secretKey) {
    StellarUtils::infoPrint("Keypair", "Importing from secret key");
    
    if (!StellarUtils::isValidAddress(secretKey)) {
        StellarUtils::errorPrint("Keypair", "Invalid secret key format");
        return nullptr;
    }
    
    if (secretKey[0] != 'S') {
        StellarUtils::errorPrint("Keypair", "Secret key must start with 'S'");
        return nullptr;
    }
    
    StellarKeypair* kp = new StellarKeypair();
    
    // Decodificar secret key
    if (!kp->decodeSecretKey(secretKey, kp->secretKey)) {
        StellarUtils::errorPrint("Keypair", "Failed to decode secret key");
        delete kp;
        return nullptr;
    }
    
    // Derivar public key
    if (!StellarCrypto::derivePublicKey(kp->publicKey, kp->secretKey)) {
        StellarUtils::errorPrint("Keypair", "Failed to derive public key");
        delete kp;
        return nullptr;
    }
    
    StellarUtils::infoPrint("Keypair", "Keypair imported successfully");
    StellarUtils::debugPrint("Keypair", ("Public: " + kp->getPublicKey()).c_str());
    
    return kp;
}

StellarKeypair* StellarKeypair::fromMnemonic(const char* words) {
    StellarUtils::infoPrint("Keypair", "Importing from mnemonic");
    
    if (!words || strlen(words) == 0) {
        StellarUtils::errorPrint("Keypair", "Empty mnemonic");
        return nullptr;
    }
    
    StellarKeypair* kp = new StellarKeypair();
    
    // Convertir mnemonic a seed
    if (!kp->mnemonicToSeed(words, kp->secretKey)) {
        StellarUtils::errorPrint("Keypair", "Failed to derive seed from mnemonic");
        delete kp;
        return nullptr;
    }
    
    // Derivar public key
    if (!StellarCrypto::derivePublicKey(kp->publicKey, kp->secretKey)) {
        StellarUtils::errorPrint("Keypair", "Failed to derive public key");
        delete kp;
        return nullptr;
    }
    
    kp->mnemonicPhrase = String(words);
    kp->hasMnemonic = true;
    
    StellarUtils::infoPrint("Keypair", "Keypair imported from mnemonic");
    StellarUtils::debugPrint("Keypair", ("Public: " + kp->getPublicKey()).c_str());
    
    return kp;
}

// ============================================
// EXPORTACIÓN
// ============================================

String StellarKeypair::getPublicKey() const {
    return encodePublicKey(publicKey);
}

String StellarKeypair::getSecretKey() const {
    return encodeSecretKey(secretKey);
}

String StellarKeypair::getMnemonic() const {
    return hasMnemonic ? mnemonicPhrase : "";
}

// ============================================
// FIRMA
// ============================================

bool StellarKeypair::sign(const uint8_t* data, size_t length, uint8_t signature[64]) const {
    return StellarCrypto::sign(secretKey, publicKey, data, length, signature);
}

bool StellarKeypair::verify(const uint8_t* data, size_t length, const uint8_t signature[64]) const {
    return StellarCrypto::verify(publicKey, data, length, signature);
}

// ============================================
// BASE32 ENCODING (Formato Stellar)
// ============================================

String StellarKeypair::encodePublicKey(const uint8_t* key) const {
    // Version byte para public key: 6 << 3 = 0x30
    return base32EncodeWithChecksum(0x30, key, 32);
}

String StellarKeypair::encodeSecretKey(const uint8_t* seed) const {
    // Version byte para secret key: 18 << 3 = 0x90
    return base32EncodeWithChecksum(0x90, seed, 32);
}

bool StellarKeypair::decodeSecretKey(const char* encoded, uint8_t* seed) const {
    uint8_t version;
    size_t length;
    
    if (!base32DecodeWithChecksum(encoded, &version, seed, &length)) {
        return false;
    }
    
    // Verificar que sea secret key (version 0x90) y longitud correcta
    return (version == 0x90 && length == 32);
}

String StellarKeypair::base32EncodeWithChecksum(uint8_t version, const uint8_t* data, size_t length) const {
    // Construir payload: version byte + data
    uint8_t payload[33];
    payload[0] = version;
    memcpy(payload + 1, data, length);
    
    // Calcular checksum CRC16-XModem
    uint16_t checksum = StellarUtils::crc16XModem(payload, 33);
    
    // Construir buffer completo: payload + checksum (little-endian)
    uint8_t fullPayload[35];
    memcpy(fullPayload, payload, 33);
    fullPayload[33] = checksum & 0xFF;          // Low byte primero
    fullPayload[34] = (checksum >> 8) & 0xFF;   // High byte segundo
    
    // Encodear en base32
    String encoded = "";
    encoded.reserve(56);  // Pre-allocate para 56 caracteres
    
    uint32_t buffer = 0;
    int bitsLeft = 0;
    
    for (int i = 0; i < 35; i++) {
        buffer = (buffer << 8) | fullPayload[i];
        bitsLeft += 8;
        
        while (bitsLeft >= 5) {
            int index = (buffer >> (bitsLeft - 5)) & 0x1F;
            encoded += STELLAR_BASE32_ALPHABET[index];
            bitsLeft -= 5;
        }
    }
    
    // Manejar bits restantes
    if (bitsLeft > 0) {
        int index = (buffer << (5 - bitsLeft)) & 0x1F;
        encoded += STELLAR_BASE32_ALPHABET[index];
    }
    
    return encoded;
}

bool StellarKeypair::base32DecodeWithChecksum(const char* encoded, uint8_t* version, uint8_t* data, size_t* length) const {
    if (strlen(encoded) != 56) {
        return false;
    }
    
    // Decodear base32
    uint8_t decoded[35];
    int decodedIdx = 0;
    uint32_t buffer = 0;
    int bitsLeft = 0;
    
    for (size_t i = 0; i < 56; i++) {
        int idx = base32Index(encoded[i]);
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
    
    // Verificar checksum (little-endian)
    uint16_t expectedChecksum = StellarUtils::crc16XModem(decoded, 33);
    uint16_t actualChecksum = decoded[33] | (decoded[34] << 8);
    
    if (expectedChecksum != actualChecksum) {
        StellarUtils::errorPrint("Keypair", "Checksum verification failed");
        return false;
    }
    
    // Extraer version y data
    *version = decoded[0];
    memcpy(data, decoded + 1, 32);
    *length = 32;
    
    return true;
}

// ============================================
// MNEMONIC BIP39 (Simplificado)
// ============================================

void StellarKeypair::generateMnemonic() {
    StellarUtils::debugPrint("Keypair", "Generating mnemonic phrase");
    
    // Generar 128 bits de entropía (12 palabras)
    uint8_t entropy[16];
    StellarCrypto::randomBytes(entropy, 16);
    
    // Calcular checksum (primeros 4 bits del SHA256)
    uint8_t hash[32];
    StellarCrypto::sha256(entropy, 16, hash);
    uint8_t checksum = hash[0] >> 4;  // 4 bits
    
    // Combinar entropy + checksum = 132 bits = 12 palabras (11 bits cada una)
    mnemonicPhrase = "";
    
    uint32_t buffer = 0;
    int bitsLeft = 0;
    int wordCount = 0;
    
    // Procesar 16 bytes de entropía
    for (int i = 0; i < 16; i++) {
        buffer = (buffer << 8) | entropy[i];
        bitsLeft += 8;
        
        while (bitsLeft >= 11 && wordCount < 11) {
            int index = (buffer >> (bitsLeft - 11)) & 0x7FF;  // 11 bits
            
            if (wordCount > 0) mnemonicPhrase += " ";
            
            // Usar módulo del tamaño de la wordlist
            mnemonicPhrase += BIP39_WORDLIST[index % BIP39_WORDLIST_SIZE];
            
            bitsLeft -= 11;
            wordCount++;
        }
    }
    
    // Última palabra con checksum
    if (wordCount < 12) {
        buffer = (buffer << 4) | checksum;
        bitsLeft += 4;
        
        int index = (buffer >> (bitsLeft - 11)) & 0x7FF;
        mnemonicPhrase += " ";
        mnemonicPhrase += BIP39_WORDLIST[index % BIP39_WORDLIST_SIZE];
    }
    
    StellarUtils::debugPrint("Keypair", "Mnemonic generated");
}

bool StellarKeypair::mnemonicToSeed(const char* words, uint8_t seed[32]) {
    // Implementación simplificada:
    // En producción usar PBKDF2(mnemonic, "mnemonic" + passphrase, 2048, HMAC-SHA512)
    // Para MVP: usar SHA256 directo del mnemonic
    
    StellarCrypto::sha256((const uint8_t*)words, strlen(words), seed);
    
    return true;
}