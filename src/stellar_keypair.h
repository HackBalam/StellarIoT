#ifndef STELLAR_KEYPAIR_H
#define STELLAR_KEYPAIR_H

#include <Arduino.h>
#include "stellar_crypto.h"

/**
 * Gestión de keypairs de Stellar
 * 
 * Maneja:
 * - Generación de keypairs Ed25519
 * - Conversión a formato Stellar (base32 con checksum)
 * - Importación desde secret key o mnemonic
 * - Firma de transacciones
 */

class StellarKeypair {
public:
    StellarKeypair();
    ~StellarKeypair();
    
    // ============================================
    // CREACIÓN E IMPORTACIÓN
    // ============================================
    
    /**
     * Genera un nuevo keypair aleatorio
     * 
     * @return Nuevo keypair
     */
    static StellarKeypair* generate();
    
    /**
     * Importa desde secret key en formato Stellar (S...)
     * 
     * @param secretKey String en formato Stellar (56 caracteres)
     * @return Keypair o nullptr si inválido
     */
    static StellarKeypair* fromSecret(const char* secretKey);
    
    /**
     * Importa desde mnemonic BIP39
     * 
     * @param words Frase mnemónica (12 palabras separadas por espacios)
     * @return Keypair o nullptr si inválido
     */
    static StellarKeypair* fromMnemonic(const char* words);
    
    // ============================================
    // EXPORTACIÓN
    // ============================================
    
    /**
     * Obtiene public key en formato Stellar (G...)
     * 
     * @return String de 56 caracteres (G + 55 caracteres base32)
     */
    String getPublicKey() const;
    
    /**
     * Obtiene secret key en formato Stellar (S...)
     * CUIDADO: Esta es información sensible
     * 
     * @return String de 56 caracteres (S + 55 caracteres base32)
     */
    String getSecretKey() const;
    
    /**
     * Obtiene mnemonic (solo si fue generado con generate())
     * 
     * @return Frase de 12 palabras o string vacío
     */
    String getMnemonic() const;
    
    /**
     * Obtiene raw bytes de public key
     * 
     * @return Puntero a 32 bytes
     */
    const uint8_t* getRawPublicKey() const { return publicKey; }
    
    /**
     * Obtiene raw bytes de secret key
     * CUIDADO: Esta es información sensible
     * 
     * @return Puntero a 32 bytes
     */
    const uint8_t* getRawSecretKey() const { return secretKey; }
    
    // ============================================
    // FIRMA
    // ============================================
    
    /**
     * Firma datos (usado para firmar transacciones)
     * 
     * @param data Datos a firmar
     * @param length Longitud
     * @param signature Buffer de salida (64 bytes)
     * @return true si éxito
     */
    bool sign(const uint8_t* data, size_t length, uint8_t signature[64]) const;
    
    /**
     * Verifica firma
     * 
     * @param data Datos firmados
     * @param length Longitud
     * @param signature Firma (64 bytes)
     * @return true si firma válida
     */
    bool verify(const uint8_t* data, size_t length, const uint8_t signature[64]) const;
    
private:
    uint8_t publicKey[32];
    uint8_t secretKey[32];
    String mnemonicPhrase;
    bool hasMnemonic;
    
    // Base32 encoding/decoding para formato Stellar
    String encodePublicKey(const uint8_t* key) const;
    String encodeSecretKey(const uint8_t* seed) const;
    bool decodeSecretKey(const char* encoded, uint8_t* seed) const;
    
    // Base32 con checksum CRC16
    String base32EncodeWithChecksum(uint8_t version, const uint8_t* data, size_t length) const;
    bool base32DecodeWithChecksum(const char* encoded, uint8_t* version, uint8_t* data, size_t* length) const;
    
    // Mnemonic BIP39 (simplificado)
    void generateMnemonic();
    bool mnemonicToSeed(const char* words, uint8_t seed[32]);
};

#endif // STELLAR_KEYPAIR_H