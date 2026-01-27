#ifndef STELLAR_STORAGE_H
#define STELLAR_STORAGE_H

#include <Arduino.h>
#include <SPIFFS.h>
#include "stellar_keypair.h"
#include "stellar_crypto.h"

/**
 * Almacenamiento seguro de keypairs
 * 
 * Características:
 * - Encriptación AES-256-GCM
 * - Derivación de clave con PBKDF2 (10000 iteraciones)
 * - Almacenamiento en SPIFFS (flash)
 * - Salt único por wallet
 * - Verificación de integridad con authentication tag
 */

#define STORAGE_MAGIC 0x53544C52  // "STLR" en ASCII
#define STORAGE_VERSION 1
#define PBKDF2_ITERATIONS 10000

class SecureWallet {
public:
    SecureWallet();
    ~SecureWallet();
    
    // ============================================
    // GUARDAR WALLET
    // ============================================
    
    /**
     * Guarda keypair encriptado en flash (SPIFFS)
     * 
     * @param keypair Keypair a guardar
     * @param password Password para encriptar (min 8 caracteres)
     * @return true si éxito
     */
    bool saveToFlash(const StellarKeypair* keypair, const char* password);
    
    // ============================================
    // CARGAR WALLET
    // ============================================
    
    /**
     * Carga keypair desde flash
     * 
     * @param password Password para desencriptar
     * @return Keypair o nullptr si falla
     */
    static StellarKeypair* loadFromFlash(const char* password);
    
    // ============================================
    // VERIFICACIÓN
    // ============================================
    
    /**
     * Verifica si existe wallet en flash
     * 
     * @return true si existe
     */
    static bool existsInFlash();
    
    /**
     * Verifica si el wallet está encriptado
     * (Siempre true en esta implementación)
     * 
     * @return true
     */
    bool isEncrypted() const { return true; }
    
    /**
     * Obtiene public key sin desencriptar
     * (Solo si está cacheado)
     * 
     * @return Public key o string vacío
     */
    String getCachedPublicKey() const { return cachedPublicKey; }
    
    // ============================================
    // BORRAR
    // ============================================
    
    /**
     * Elimina wallet del flash
     * CUIDADO: Esta acción es irreversible
     * 
     * @return true si éxito
     */
    static bool deleteFromFlash();
    
private:
    String cachedPublicKey;
    
    // Estructura de almacenamiento encriptado
    struct StoredWallet {
        uint32_t magic;           // 4 bytes: MAGIC number
        uint8_t version;          // 1 byte: versión del formato
        uint8_t salt[16];         // 16 bytes: salt para PBKDF2
        uint8_t iv[12];           // 12 bytes: IV para AES-GCM
        uint8_t ciphertext[32];   // 32 bytes: secret key encriptada
        uint8_t tag[16];          // 16 bytes: authentication tag
        uint8_t publicKey[32];    // 32 bytes: public key (plaintext)
        uint16_t checksum;        // 2 bytes: checksum de validación
    };  // Total: 115 bytes
    
    // Path del archivo en SPIFFS
    static const char* WALLET_PATH;
    
    // Encriptar keypair
    bool encrypt(
        const StellarKeypair* keypair,
        const char* password,
        StoredWallet* output
    );
    
    // Desencriptar keypair
    static StellarKeypair* decrypt(
        const StoredWallet* input,
        const char* password
    );
    
    // Calcular checksum de la estructura
    static uint16_t calculateChecksum(const StoredWallet* wallet);
    
    // Validar password
    static bool isPasswordValid(const char* password);
};

#endif // STELLAR_STORAGE_H