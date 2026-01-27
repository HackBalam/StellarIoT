#ifndef STELLAR_XDR_H
#define STELLAR_XDR_H

#include <Arduino.h>

/**
 * Serialización XDR para Stellar Protocol
 * 
 * XDR (External Data Representation) es el formato binario
 * usado por Stellar para codificar transacciones.
 * 
 * Este módulo implementa un subconjunto mínimo para el MVP:
 * - Payment operations
 * - Simple transactions
 * - Memo TEXT
 */

// Tipos de Asset según protocolo Stellar
enum AssetType {
    ASSET_TYPE_NATIVE = 0,              // XLM
    ASSET_TYPE_CREDIT_ALPHANUM4 = 1,    // Tokens 4 chars (USD, BTC, etc)
    ASSET_TYPE_CREDIT_ALPHANUM12 = 2    // Tokens 12 chars
};

// Tipos de Operación
enum OperationType {
    CREATE_ACCOUNT = 0,
    PAYMENT = 1,
    PATH_PAYMENT_STRICT_RECEIVE = 2,
    MANAGE_SELL_OFFER = 3,
    CREATE_PASSIVE_SELL_OFFER = 4,
    SET_OPTIONS = 5,
    CHANGE_TRUST = 6,
    ALLOW_TRUST = 7,
    ACCOUNT_MERGE = 8,
    INFLATION = 9,
    MANAGE_DATA = 10,
    BUMP_SEQUENCE = 11,
    MANAGE_BUY_OFFER = 12,
    PATH_PAYMENT_STRICT_SEND = 13
};

// Tipos de Memo
enum MemoType {
    MEMO_NONE = 0,
    MEMO_TEXT = 1,
    MEMO_ID = 2,
    MEMO_HASH = 3,
    MEMO_RETURN = 4
};

/**
 * Encoder XDR para construir transacciones Stellar
 * 
 * Uso:
 * 1. Crear encoder
 * 2. Encodear cada campo en orden
 * 3. Obtener bytes con getData()
 */
class XDREncoder {
public:
    XDREncoder();
    ~XDREncoder();
    
    // ============================================
    // TIPOS BÁSICOS XDR
    // ============================================
    
    /**
     * Encodea uint32 (4 bytes big-endian)
     */
    void encodeUint32(uint32_t value);
    
    /**
     * Encodea uint64 (8 bytes big-endian)
     */
    void encodeUint64(uint64_t value);
    
    /**
     * Encodea int32 (4 bytes big-endian)
     */
    void encodeInt32(int32_t value);
    
    /**
     * Encodea int64 (8 bytes big-endian)
     */
    void encodeInt64(int64_t value);
    
    /**
     * Encodea bool (uint32: 0 o 1)
     */
    void encodeBool(bool value);
    
    /**
     * Encodea string con longitud y padding
     * Formato: length (4 bytes) + data + padding a múltiplo de 4
     */
    void encodeString(const char* str);
    
    /**
     * Encodea bytes arbitrarios con longitud y padding
     */
    void encodeBytes(const uint8_t* data, size_t length);
    
    /**
     * Encodea clave pública Ed25519 (32 bytes)
     * Formato Stellar: type (uint32) + key (32 bytes)
     */
    void encodePublicKey(const uint8_t* publicKey);
    
    // ============================================
    // ESTRUCTURAS STELLAR
    // ============================================
    
    /**
     * Encodea Asset (para MVP solo native XLM)
     */
    void encodeAsset(AssetType type);
    
    /**
     * Encodea Memo
     * Para MVP solo MEMO_NONE y MEMO_TEXT
     */
    void encodeMemo(MemoType type, const char* text = nullptr);

    void append(const uint8_t* data, size_t length);
    
    /**
     * Encodea Payment Operation
     * 
     * @param destination Clave pública destino (32 bytes)
     * @param amount Cantidad en stroops (int64)
     */
    void encodePaymentOp(const uint8_t* destination, int64_t amount);
    
    // ============================================
    // OBTENER DATOS
    // ============================================
    
    /**
     * Obtiene puntero a los datos encodados
     * 
     * @return Puntero a buffer
     */
    const uint8_t* getData() const { return buffer; }
    
    /**
     * Obtiene tamaño actual de datos encodados
     * 
     * @return Tamaño en bytes
     */
    size_t getSize() const { return position; }
    
    /**
     * Reinicia el encoder (limpia buffer)
     */
    void reset();
    
    /**
     * Obtiene el contenido como String hex (para debug)
     */
    String toHex() const;
    
private:
    uint8_t* buffer;
    size_t position;
    size_t capacity;
    
    void ensureCapacity(size_t needed);
};

#endif // STELLAR_XDR_H