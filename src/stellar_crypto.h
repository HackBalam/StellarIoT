#ifndef STELLAR_CRYPTO_H
#define STELLAR_CRYPTO_H

#include <Arduino.h>
#include <Ed25519.h>
#include <SHA256.h>
#include <AES.h>
#include <GCM.h>

/**
 * Módulo de criptografía para Stellar IoT SDK
 * 
 * Proporciona funciones criptográficas necesarias para:
 * - Firmar transacciones (Ed25519)
 * - Hash de transacciones (SHA-256)
 * - Almacenamiento seguro (AES-256-GCM)
 * - Derivación de claves (PBKDF2)
 */

class StellarCrypto {
public:
    // ============================================
    // ED25519 OPERATIONS (Firma Digital)
    // ============================================
    
    /**
     * Genera un par de claves Ed25519
     * Usa el RNG por hardware del ESP32
     * 
     * @param publicKey Buffer de 32 bytes para la clave pública
     * @param privateKey Buffer de 32 bytes para la clave privada
     * @return true si éxito
     */
    static bool generateKeypair(uint8_t publicKey[32], uint8_t privateKey[32]);
    
    /**
     * Deriva la clave pública desde la clave privada
     * 
     * @param publicKey Buffer de salida (32 bytes)
     * @param privateKey Clave privada (32 bytes)
     * @return true si éxito
     */
    static bool derivePublicKey(uint8_t publicKey[32], const uint8_t privateKey[32]);
    
    /**
     * Firma datos con Ed25519
     * Usado para firmar transacciones Stellar
     * 
     * @param privateKey Clave privada (32 bytes)
     * @param publicKey Clave pública (32 bytes)
     * @param message Datos a firmar
     * @param messageLen Longitud de los datos
     * @param signature Buffer de salida (64 bytes)
     * @return true si éxito
     */
    static bool sign(
        const uint8_t* privateKey,
        const uint8_t* publicKey,
        const uint8_t* message,
        size_t messageLen,
        uint8_t signature[64]
    );
    
    /**
     * Verifica una firma Ed25519
     * 
     * @param publicKey Clave pública (32 bytes)
     * @param message Datos firmados
     * @param messageLen Longitud
     * @param signature Firma a verificar (64 bytes)
     * @return true si la firma es válida
     */
    static bool verify(
        const uint8_t* publicKey,
        const uint8_t* message,
        size_t messageLen,
        const uint8_t signature[64]
    );
    
    // ============================================
    // SHA-256 (Hash)
    // ============================================
    
    /**
     * Calcula SHA-256 de datos
     * 
     * @param data Datos a hashear
     * @param length Longitud de datos
     * @param hash Buffer de salida (32 bytes)
     */
    static void sha256(const uint8_t* data, size_t length, uint8_t hash[32]);
    
    /**
     * Calcula SHA-256 de múltiples buffers
     * Útil para: SHA256(networkPassphrase + transactionXDR)
     * 
     * @param data1 Primer buffer
     * @param len1 Longitud primer buffer
     * @param data2 Segundo buffer
     * @param len2 Longitud segundo buffer
     * @param hash Buffer de salida (32 bytes)
     */
    static void sha256Multiple(
        const uint8_t* data1, size_t len1,
        const uint8_t* data2, size_t len2,
        uint8_t hash[32]
    );
    
    // ============================================
    // AES-256-GCM (Encriptación)
    // ============================================
    
    /**
     * Encripta datos con AES-256-GCM
     * Proporciona confidencialidad Y autenticación
     * 
     * @param plaintext Datos a encriptar
     * @param plaintextLen Longitud
     * @param key Clave de encriptación (32 bytes)
     * @param iv Vector de inicialización (12 bytes)
     * @param ciphertext Buffer de salida (mismo tamaño que plaintext)
     * @param tag Tag de autenticación (16 bytes)
     * @return true si éxito
     */
    static bool encryptAES256GCM(
        const uint8_t* plaintext,
        size_t plaintextLen,
        const uint8_t key[32],
        const uint8_t iv[12],
        uint8_t* ciphertext,
        uint8_t tag[16]
    );
    
    /**
     * Desencripta datos con AES-256-GCM
     * Verifica autenticidad antes de retornar datos
     * 
     * @param ciphertext Datos encriptados
     * @param ciphertextLen Longitud
     * @param key Clave de desencriptación (32 bytes)
     * @param iv Vector de inicialización (12 bytes)
     * @param tag Tag de autenticación (16 bytes)
     * @param plaintext Buffer de salida
     * @return true si éxito y tag válido
     */
    static bool decryptAES256GCM(
        const uint8_t* ciphertext,
        size_t ciphertextLen,
        const uint8_t key[32],
        const uint8_t iv[12],
        const uint8_t tag[16],
        uint8_t* plaintext
    );
    
    // ============================================
    // PBKDF2 (Key Derivation)
    // ============================================
    
    /**
     * Deriva clave desde password usando PBKDF2-HMAC-SHA256
     * Usado para convertir passwords en claves de encriptación
     * 
     * @param password Password del usuario
     * @param salt Salt aleatorio (16 bytes)
     * @param iterations Número de iteraciones (min 10000)
     * @param key Buffer de salida (32 bytes)
     * @return true si éxito
     */
    static bool deriveKeyPBKDF2(
        const char* password,
        const uint8_t salt[16],
        uint32_t iterations,
        uint8_t key[32]
    );
    
    // ============================================
    // RANDOM (Generación Aleatoria)
    // ============================================
    
    /**
     * Genera bytes aleatorios usando hardware RNG del ESP32
     * Criptográficamente seguro
     * 
     * @param buffer Buffer de salida
     * @param length Número de bytes a generar
     */
    static void randomBytes(uint8_t* buffer, size_t length);
};

#endif // STELLAR_CRYPTO_H