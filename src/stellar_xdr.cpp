#include "stellar_xdr.h"
#include "stellar_utils.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 512
#define CAPACITY_INCREMENT 256

// ============================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================

XDREncoder::XDREncoder() {
    buffer = (uint8_t*)malloc(INITIAL_CAPACITY);
    if (!buffer) {
        StellarUtils::errorPrint("XDR", "Failed to allocate buffer");
        capacity = 0;
        position = 0;
        return;
    }
    
    capacity = INITIAL_CAPACITY;
    position = 0;
    
    StellarUtils::debugPrint("XDR", "Encoder initialized");
}

XDREncoder::~XDREncoder() {
    if (buffer) {
        free(buffer);
        buffer = nullptr;
    }
}

// ============================================
// BUFFER MANAGEMENT
// ============================================

void XDREncoder::ensureCapacity(size_t needed) {
    if (position + needed > capacity) {
        size_t newCapacity = capacity;
        
        while (newCapacity < position + needed) {
            newCapacity += CAPACITY_INCREMENT;
        }
        
        uint8_t* newBuffer = (uint8_t*)realloc(buffer, newCapacity);
        
        if (newBuffer) {
            buffer = newBuffer;
            capacity = newCapacity;
            StellarUtils::debugPrint("XDR", 
                ("Buffer expanded to " + String(newCapacity) + " bytes").c_str());
        } else {
            StellarUtils::errorPrint("XDR", "Failed to expand buffer");
        }
    }
}

void XDREncoder::append(const uint8_t* data, size_t length) {
    ensureCapacity(length);
    memcpy(buffer + position, data, length);
    position += length;
}

void XDREncoder::reset() {
    position = 0;
    StellarUtils::debugPrint("XDR", "Encoder reset");
}

String XDREncoder::toHex() const {
    return StellarUtils::hexEncode(buffer, position);
}

// ============================================
// TIPOS BÁSICOS XDR
// ============================================

void XDREncoder::encodeUint32(uint32_t value) {
    uint8_t bytes[4];
    
    // Big-endian (network byte order)
    bytes[0] = (value >> 24) & 0xFF;
    bytes[1] = (value >> 16) & 0xFF;
    bytes[2] = (value >> 8) & 0xFF;
    bytes[3] = value & 0xFF;
    
    append(bytes, 4);
}

void XDREncoder::encodeUint64(uint64_t value) {
    uint8_t bytes[8];
    
    // Big-endian
    bytes[0] = (value >> 56) & 0xFF;
    bytes[1] = (value >> 48) & 0xFF;
    bytes[2] = (value >> 40) & 0xFF;
    bytes[3] = (value >> 32) & 0xFF;
    bytes[4] = (value >> 24) & 0xFF;
    bytes[5] = (value >> 16) & 0xFF;
    bytes[6] = (value >> 8) & 0xFF;
    bytes[7] = value & 0xFF;
    
    append(bytes, 8);
}

void XDREncoder::encodeInt32(int32_t value) {
    encodeUint32((uint32_t)value);
}

void XDREncoder::encodeInt64(int64_t value) {
    encodeUint64((uint64_t)value);
}

void XDREncoder::encodeBool(bool value) {
    encodeUint32(value ? 1 : 0);
}

void XDREncoder::encodeString(const char* str) {
    if (!str) {
        encodeUint32(0);
        return;
    }
    
    uint32_t length = strlen(str);
    
    // Encodear longitud
    encodeUint32(length);
    
    // Encodear string
    append((const uint8_t*)str, length);
    
    // Padding a múltiplo de 4 bytes
    uint32_t padding = (4 - (length % 4)) % 4;
    
    if (padding > 0) {
        uint8_t zeros[3] = {0, 0, 0};
        append(zeros, padding);
    }
}

void XDREncoder::encodeBytes(const uint8_t* data, size_t length) {
    // Encodear longitud
    encodeUint32(length);
    
    // Encodear datos
    append(data, length);
    
    // Padding
    uint32_t padding = (4 - (length % 4)) % 4;
    
    if (padding > 0) {
        uint8_t zeros[3] = {0, 0, 0};
        append(zeros, padding);
    }
}

void XDREncoder::encodePublicKey(const uint8_t* publicKey) {
    // PublicKey en Stellar:
    // - type: uint32 (0 = PUBLIC_KEY_TYPE_ED25519)
    // - key: 32 bytes
    
    encodeUint32(0);  // PUBLIC_KEY_TYPE_ED25519
    append(publicKey, 32);
}

// ============================================
// ESTRUCTURAS STELLAR
// ============================================

void XDREncoder::encodeAsset(AssetType type) {
    // Para MVP solo soportamos native (XLM)
    encodeUint32(type);
    
    if (type == ASSET_TYPE_NATIVE) {
        // Native asset no tiene campos adicionales
    } else {
        // TODO: Implementar para alphanum4 y alphanum12
        StellarUtils::errorPrint("XDR", "Non-native assets not implemented yet");
    }
}

void XDREncoder::encodeMemo(MemoType type, const char* text) {
    // Encodear tipo de memo
    encodeUint32(type);
    
    switch (type) {
        case MEMO_NONE:
            // Sin datos adicionales
            break;
            
        case MEMO_TEXT:
            if (text) {
                // Verificar longitud (máximo 28 bytes)
                size_t len = strlen(text);
                if (len > 28) {
                    StellarUtils::errorPrint("XDR", "Memo text too long (max 28 bytes)");
                    encodeString("");
                } else {
                    encodeString(text);
                }
            } else {
                encodeString("");
            }
            break;
            
        case MEMO_ID:
            // TODO: Encodear uint64 ID
            StellarUtils::errorPrint("XDR", "MEMO_ID not implemented yet");
            break;
            
        case MEMO_HASH:
        case MEMO_RETURN:
            // TODO: Encodear 32 bytes hash
            StellarUtils::errorPrint("XDR", "MEMO_HASH/RETURN not implemented yet");
            break;
    }
}

void XDREncoder::encodePaymentOp(const uint8_t* destination, int64_t amount) {
    StellarUtils::debugPrint("XDR", "Encoding Payment operation");
    
    // Payment Operation estructura:
    // - MuxedAccount destination
    // - Asset asset
    // - int64 amount
    
    // MuxedAccount (simplificado - solo ED25519, sin multiplexing)
    // type: uint32 (0 = KEY_TYPE_ED25519)
    encodeUint32(0);  // KEY_TYPE_ED25519
    
    // ed25519: 32 bytes
    append(destination, 32);
    
    // Asset (native XLM)
    encodeAsset(ASSET_TYPE_NATIVE);
    
    // Amount en stroops
    encodeInt64(amount);
    
    StellarUtils::debugPrint("XDR", 
        ("Payment: " + String((long long)amount) + " stroops").c_str());
}