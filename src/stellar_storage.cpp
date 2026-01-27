#include "stellar_storage.h"
#include "stellar_utils.h"

// Definir constante estática
const char* SecureWallet::WALLET_PATH = "/wallet.dat";

// ============================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================

SecureWallet::SecureWallet() {
    cachedPublicKey = "";
}

SecureWallet::~SecureWallet() {
    // Limpiar cache
    cachedPublicKey = "";
}

// ============================================
// GUARDAR WALLET
// ============================================

bool SecureWallet::saveToFlash(const StellarKeypair* keypair, const char* password) {
    if (!keypair) {
        StellarUtils::errorPrint("Storage", "Keypair is null");
        return false;
    }
    
    if (!isPasswordValid(password)) {
        StellarUtils::errorPrint("Storage", "Password must be at least 8 characters");
        return false;
    }
    
    StellarUtils::infoPrint("Storage", "Saving wallet to flash");
    
    // Inicializar SPIFFS si no está
    if (!SPIFFS.begin(true)) {
        StellarUtils::errorPrint("Storage", "Failed to mount SPIFFS");
        return false;
    }
    
    // Encriptar keypair
    StoredWallet wallet;
    if (!encrypt(keypair, password, &wallet)) {
        StellarUtils::errorPrint("Storage", "Encryption failed");
        return false;
    }
    
    // Abrir archivo para escritura
    File file = SPIFFS.open(WALLET_PATH, "w");
    if (!file) {
        StellarUtils::errorPrint("Storage", "Failed to open file for writing");
        return false;
    }
    
    // Escribir estructura completa
    size_t written = file.write((uint8_t*)&wallet, sizeof(StoredWallet));
    file.close();
    
    if (written != sizeof(StoredWallet)) {
        StellarUtils::errorPrint("Storage", "Failed to write complete wallet");
        SPIFFS.remove(WALLET_PATH);  // Limpiar archivo corrupto
        return false;
    }
    
    // Cachear public key
    cachedPublicKey = keypair->getPublicKey();
    
    StellarUtils::infoPrint("Storage", "Wallet saved successfully");
    StellarUtils::debugPrint("Storage", ("Size: " + String(sizeof(StoredWallet)) + " bytes").c_str());
    
    return true;
}

// ============================================
// CARGAR WALLET
// ============================================

StellarKeypair* SecureWallet::loadFromFlash(const char* password) {
    if (!isPasswordValid(password)) {
        StellarUtils::errorPrint("Storage", "Password must be at least 8 characters");
        return nullptr;
    }
    
    StellarUtils::infoPrint("Storage", "Loading wallet from flash");
    
    // Inicializar SPIFFS
    if (!SPIFFS.begin(false)) {
        StellarUtils::errorPrint("Storage", "Failed to mount SPIFFS");
        return nullptr;
    }
    
    // Verificar que existe el archivo
    if (!SPIFFS.exists(WALLET_PATH)) {
        StellarUtils::errorPrint("Storage", "Wallet file not found");
        return nullptr;
    }
    
    // Abrir archivo
    File file = SPIFFS.open(WALLET_PATH, "r");
    if (!file) {
        StellarUtils::errorPrint("Storage", "Failed to open wallet file");
        return nullptr;
    }
    
    // Verificar tamaño
    size_t fileSize = file.size();
    if (fileSize != sizeof(StoredWallet)) {
        StellarUtils::errorPrint("Storage", "Invalid wallet file size");
        file.close();
        return nullptr;
    }
    
    // Leer estructura
    StoredWallet wallet;
    size_t read = file.read((uint8_t*)&wallet, sizeof(StoredWallet));
    file.close();
    
    if (read != sizeof(StoredWallet)) {
        StellarUtils::errorPrint("Storage", "Failed to read complete wallet");
        return nullptr;
    }
    
    // Desencriptar
    StellarKeypair* keypair = decrypt(&wallet, password);
    
    if (keypair) {
        StellarUtils::infoPrint("Storage", "Wallet loaded successfully");
        StellarUtils::debugPrint("Storage", ("Public: " + keypair->getPublicKey()).c_str());
    } else {
        StellarUtils::errorPrint("Storage", "Decryption failed - wrong password?");
    }
    
    // Limpiar memoria sensible
    StellarUtils::secureZero(&wallet, sizeof(StoredWallet));
    
    return keypair;
}

// ============================================
// VERIFICACIÓN
// ============================================

bool SecureWallet::existsInFlash() {
    if (!SPIFFS.begin(false)) {
        return false;
    }
    
    return SPIFFS.exists(WALLET_PATH);
}

// ============================================
// BORRAR
// ============================================

bool SecureWallet::deleteFromFlash() {
    StellarUtils::infoPrint("Storage", "Deleting wallet from flash");
    
    if (!SPIFFS.begin(false)) {
        StellarUtils::errorPrint("Storage", "Failed to mount SPIFFS");
        return false;
    }
    
    if (!SPIFFS.exists(WALLET_PATH)) {
        StellarUtils::errorPrint("Storage", "Wallet file not found");
        return false;
    }
    
    bool success = SPIFFS.remove(WALLET_PATH);
    
    if (success) {
        StellarUtils::infoPrint("Storage", "Wallet deleted successfully");
    } else {
        StellarUtils::errorPrint("Storage", "Failed to delete wallet");
    }
    
    return success;
}

// ============================================
// FUNCIONES PRIVADAS
// ============================================

bool SecureWallet::encrypt(
    const StellarKeypair* keypair,
    const char* password,
    StoredWallet* output
) {
    StellarUtils::debugPrint("Storage", "Encrypting keypair");
    
    // Inicializar estructura
    output->magic = STORAGE_MAGIC;
    output->version = STORAGE_VERSION;
    
    // Generar salt aleatorio
    StellarCrypto::randomBytes(output->salt, 16);
    
    // Derivar encryption key con PBKDF2
    uint8_t key[32];
    if (!StellarCrypto::deriveKeyPBKDF2(password, output->salt, PBKDF2_ITERATIONS, key)) {
        StellarUtils::errorPrint("Storage", "Key derivation failed");
        return false;
    }
    
    // Generar IV aleatorio
    StellarCrypto::randomBytes(output->iv, 12);
    
    // Encriptar secret key
    const uint8_t* secretKey = keypair->getRawSecretKey();
    if (!StellarCrypto::encryptAES256GCM(
        secretKey,
        32,
        key,
        output->iv,
        output->ciphertext,
        output->tag
    )) {
        // Limpiar key sensible
        StellarUtils::secureZero(key, 32);
        StellarUtils::errorPrint("Storage", "Encryption failed");
        return false;
    }
    
    // Limpiar key sensible
    StellarUtils::secureZero(key, 32);
    
    // Copiar public key (plaintext - no es sensible)
    memcpy(output->publicKey, keypair->getRawPublicKey(), 32);
    
    // Calcular checksum de toda la estructura (excepto el checksum mismo)
    output->checksum = calculateChecksum(output);
    
    StellarUtils::debugPrint("Storage", "Encryption complete");
    
    return true;
}

StellarKeypair* SecureWallet::decrypt(
    const StoredWallet* input,
    const char* password
) {
    StellarUtils::debugPrint("Storage", "Decrypting keypair");
    
    // Verificar magic number
    if (input->magic != STORAGE_MAGIC) {
        StellarUtils::errorPrint("Storage", "Invalid magic number");
        return nullptr;
    }
    
    // Verificar versión
    if (input->version != STORAGE_VERSION) {
        StellarUtils::errorPrint("Storage", "Unsupported version");
        return nullptr;
    }
    
    // Verificar checksum
    uint16_t expectedChecksum = calculateChecksum(input);
    if (input->checksum != expectedChecksum) {
        StellarUtils::errorPrint("Storage", "Checksum mismatch - file corrupted");
        return nullptr;
    }
    
    // Derivar encryption key
    uint8_t key[32];
    if (!StellarCrypto::deriveKeyPBKDF2(password, input->salt, PBKDF2_ITERATIONS, key)) {
        StellarUtils::errorPrint("Storage", "Key derivation failed");
        return nullptr;
    }
    
    // Desencriptar secret key
    uint8_t decryptedSecret[32];
    if (!StellarCrypto::decryptAES256GCM(
        input->ciphertext,
        32,
        key,
        input->iv,
        input->tag,
        decryptedSecret
    )) {
        // Limpiar datos sensibles
        StellarUtils::secureZero(key, 32);
        StellarUtils::secureZero(decryptedSecret, 32);
        StellarUtils::errorPrint("Storage", "Decryption failed - wrong password");
        return nullptr;
    }
    
    // Limpiar key
    StellarUtils::secureZero(key, 32);
    
    // Crear keypair temporal para obtener el formato Stellar
    StellarKeypair tempKp;
    memcpy((void*)tempKp.getRawSecretKey(), decryptedSecret, 32);
    String secretKeyStr = tempKp.getSecretKey();
    
    // Limpiar secret key temporal
    StellarUtils::secureZero(decryptedSecret, 32);
    
    // Crear keypair final desde secret key
    StellarKeypair* keypair = StellarKeypair::fromSecret(secretKeyStr.c_str());
    
    if (!keypair) {
        StellarUtils::errorPrint("Storage", "Failed to create keypair from decrypted data");
        return nullptr;
    }
    
    // Verificar que el public key coincida
    const uint8_t* derivedPublicKey = keypair->getRawPublicKey();
    if (memcmp(derivedPublicKey, input->publicKey, 32) != 0) {
        StellarUtils::errorPrint("Storage", "Public key mismatch - data corrupted");
        delete keypair;
        return nullptr;
    }
    
    StellarUtils::debugPrint("Storage", "Decryption complete");
    
    return keypair;
}

uint16_t SecureWallet::calculateChecksum(const StoredWallet* wallet) {
    // Calcular checksum de todo excepto el campo checksum
    size_t dataSize = sizeof(StoredWallet) - sizeof(uint16_t);
    return StellarUtils::crc16XModem((const uint8_t*)wallet, dataSize);
}

bool SecureWallet::isPasswordValid(const char* password) {
    if (!password) return false;
    
    size_t len = strlen(password);
    
    // Mínimo 8 caracteres
    if (len < 8) {
        return false;
    }
    
    // Máximo 128 caracteres (razonable)
    if (len > 128) {
        return false;
    }
    
    return true;
}