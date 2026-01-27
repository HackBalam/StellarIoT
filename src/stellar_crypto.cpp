#include "stellar_crypto.h"
#include "stellar_utils.h"
#include <esp_system.h>

// ============================================
// ED25519 OPERATIONS
// ============================================

bool StellarCrypto::generateKeypair(uint8_t publicKey[32], uint8_t privateKey[32]) {
    StellarUtils::debugPrint("Crypto", "Generating Ed25519 keypair");
    
    // Generar seed aleatorio con hardware RNG del ESP32
    randomBytes(privateKey, 32);
    
    // Derivar public key desde private key
    Ed25519::derivePublicKey(publicKey, privateKey);
    
    StellarUtils::debugPrintHex("Crypto", publicKey, 32);
    
    return true;
}

bool StellarCrypto::derivePublicKey(uint8_t publicKey[32], const uint8_t privateKey[32]) {
    Ed25519::derivePublicKey(publicKey, privateKey);
    return true;
}

bool StellarCrypto::sign(
    const uint8_t* privateKey,
    const uint8_t* publicKey,
    const uint8_t* message,
    size_t messageLen,
    uint8_t signature[64]
) {
    StellarUtils::debugPrint("Crypto", "Signing with Ed25519");
    
    // Firmar con Ed25519
    Ed25519::sign(signature, privateKey, publicKey, message, messageLen);
    
    return true;
}

bool StellarCrypto::verify(
    const uint8_t* publicKey,
    const uint8_t* message,
    size_t messageLen,
    const uint8_t signature[64]
) {
    return Ed25519::verify(signature, publicKey, message, messageLen);
}

// ============================================
// SHA-256
// ============================================

void StellarCrypto::sha256(const uint8_t* data, size_t length, uint8_t hash[32]) {
    SHA256 sha;
    sha.reset();
    sha.update(data, length);
    sha.finalize(hash, 32);
}

void StellarCrypto::sha256Multiple(
    const uint8_t* data1, size_t len1,
    const uint8_t* data2, size_t len2,
    uint8_t hash[32]
) {
    SHA256 sha;
    sha.reset();
    sha.update(data1, len1);
    sha.update(data2, len2);
    sha.finalize(hash, 32);
}

// ============================================
// AES-256-GCM
// ============================================

bool StellarCrypto::encryptAES256GCM(
    const uint8_t* plaintext,
    size_t plaintextLen,
    const uint8_t key[32],
    const uint8_t iv[12],
    uint8_t* ciphertext,
    uint8_t tag[16]
) {
    StellarUtils::debugPrint("Crypto", "Encrypting with AES-256-GCM");
    
    GCM<AES256> gcm;
    
    // Configurar clave
    gcm.setKey(key, 32);
    
    // Configurar IV
    gcm.setIV(iv, 12);
    
    // Encriptar
    gcm.encrypt(ciphertext, plaintext, plaintextLen);
    
    // Calcular tag de autenticación
    gcm.computeTag(tag, 16);
    
    StellarUtils::debugPrint("Crypto", "Encryption successful");
    
    return true;
}

bool StellarCrypto::decryptAES256GCM(
    const uint8_t* ciphertext,
    size_t ciphertextLen,
    const uint8_t key[32],
    const uint8_t iv[12],
    const uint8_t tag[16],
    uint8_t* plaintext
) {
    StellarUtils::debugPrint("Crypto", "Decrypting with AES-256-GCM");
    
    GCM<AES256> gcm;
    
    // Configurar clave
    gcm.setKey(key, 32);
    
    // Configurar IV
    gcm.setIV(iv, 12);
    
    // Desencriptar
    gcm.decrypt(plaintext, ciphertext, ciphertextLen);
    
    // Calcular tag esperado
    uint8_t computedTag[16];
    gcm.computeTag(computedTag, 16);
    
    // Verificar tag (autenticación)
    bool tagValid = (memcmp(tag, computedTag, 16) == 0);
    
    if (!tagValid) {
        StellarUtils::errorPrint("Crypto", "Authentication tag mismatch");
        // Limpiar plaintext por seguridad
        memset(plaintext, 0, ciphertextLen);
        return false;
    }
    
    StellarUtils::debugPrint("Crypto", "Decryption successful");
    return true;
}

// ============================================
// PBKDF2
// ============================================

bool StellarCrypto::deriveKeyPBKDF2(
    const char* password,
    const uint8_t salt[16],
    uint32_t iterations,
    uint8_t key[32]
) {
    StellarUtils::debugPrint("Crypto", "Deriving key with PBKDF2");
    
    if (iterations < 1000) {
        StellarUtils::errorPrint("Crypto", "Iterations too low (min 1000)");
        return false;
    }
    
    SHA256 sha;
    uint8_t block[32];
    uint8_t temp[32];
    size_t passwordLen = strlen(password);
    
    // PBKDF2 con 1 bloque (32 bytes de salida)
    // U1 = HMAC(password, salt || 0x00000001)
    sha.resetHMAC((const uint8_t*)password, passwordLen);
    sha.update(salt, 16);
    
    // Counter (big-endian)
    uint8_t counter[4] = {0, 0, 0, 1};
    sha.update(counter, 4);
    sha.finalizeHMAC((const uint8_t*)password, passwordLen, block, 32);
    
    // Copiar U1 al resultado
    memcpy(key, block, 32);
    
    // Iteraciones restantes: Ui = HMAC(password, Ui-1)
    for (uint32_t i = 1; i < iterations; i++) {
        sha.resetHMAC((const uint8_t*)password, passwordLen);
        sha.update(block, 32);
        sha.finalizeHMAC((const uint8_t*)password, passwordLen, temp, 32);
        
        // XOR con resultado acumulado
        for (int j = 0; j < 32; j++) {
            key[j] ^= temp[j];
        }
        
        memcpy(block, temp, 32);
    }
    
    StellarUtils::debugPrint("Crypto", "Key derivation complete");
    return true;
}

// ============================================
// RANDOM
// ============================================

void StellarCrypto::randomBytes(uint8_t* buffer, size_t length) {
    // Usar hardware RNG del ESP32 (criptográficamente seguro)
    esp_fill_random(buffer, length);
}