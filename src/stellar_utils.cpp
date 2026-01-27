#include "stellar_utils.h"
#include <esp_system.h>
#include <esp_heap_caps.h>

// ============================================
// CONVERSIONES DE MONEDA
// ============================================

float StellarUtils::stroopsToXLM(int64_t stroops) {
    return (float)stroops / 10000000.0f;
}

int64_t StellarUtils::xlmToStroops(float xlm) {
    // Redondear para evitar problemas de precisión
    return (int64_t)(xlm * 10000000.0f + 0.5f);
}

// ============================================
// ENCODING/DECODING
// ============================================

// Tabla Base64 estándar
static const char BASE64_CHARS[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

String StellarUtils::base64Encode(const uint8_t* data, size_t length) {
    String encoded = "";
    encoded.reserve((length * 4 / 3) + 4);  // Pre-allocate
    
    int i = 0;
    uint8_t array3[3];
    uint8_t array4[4];
    
    while (length--) {
        array3[i++] = *(data++);
        
        if (i == 3) {
            array4[0] = (array3[0] & 0xfc) >> 2;
            array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
            array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
            array4[3] = array3[2] & 0x3f;
            
            for (i = 0; i < 4; i++) {
                encoded += BASE64_CHARS[array4[i]];
            }
            i = 0;
        }
    }
    
    // Manejar bytes restantes
    if (i > 0) {
        for (int j = i; j < 3; j++) {
            array3[j] = '\0';
        }
        
        array4[0] = (array3[0] & 0xfc) >> 2;
        array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
        array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
        
        for (int j = 0; j < i + 1; j++) {
            encoded += BASE64_CHARS[array4[j]];
        }
        
        while (i++ < 3) {
            encoded += '=';
        }
    }
    
    return encoded;
}

bool StellarUtils::base64Decode(const char* input, uint8_t* output, size_t* outputLength) {
    if (!input || !output || !outputLength) {
        return false;
    }
    
    size_t inputLength = strlen(input);
    if (inputLength % 4 != 0) {
        return false;
    }
    
    // Tabla de decodificación inversa
    static int decodeTable[256];
    static bool tableInitialized = false;
    
    if (!tableInitialized) {
        memset(decodeTable, -1, sizeof(decodeTable));
        for (int i = 0; i < 64; i++) {
            decodeTable[(int)BASE64_CHARS[i]] = i;
        }
        tableInitialized = true;
    }
    
    size_t outIdx = 0;
    uint8_t buffer[4];
    int bufferIdx = 0;
    
    for (size_t i = 0; i < inputLength; i++) {
        char c = input[i];
        
        if (c == '=') break;  // Padding
        
        int value = decodeTable[(int)c];
        if (value == -1) {
            return false;  // Carácter inválido
        }
        
        buffer[bufferIdx++] = value;
        
        if (bufferIdx == 4) {
            output[outIdx++] = (buffer[0] << 2) | (buffer[1] >> 4);
            output[outIdx++] = (buffer[1] << 4) | (buffer[2] >> 2);
            output[outIdx++] = (buffer[2] << 6) | buffer[3];
            bufferIdx = 0;
        }
    }
    
    // Manejar bytes finales
    if (bufferIdx == 2) {
        output[outIdx++] = (buffer[0] << 2) | (buffer[1] >> 4);
    } else if (bufferIdx == 3) {
        output[outIdx++] = (buffer[0] << 2) | (buffer[1] >> 4);
        output[outIdx++] = (buffer[1] << 4) | (buffer[2] >> 2);
    }
    
    *outputLength = outIdx;
    return true;
}

String StellarUtils::hexEncode(const uint8_t* data, size_t length) {
    String hex = "";
    hex.reserve(length * 2);
    
    for (size_t i = 0; i < length; i++) {
        char buf[3];
        sprintf(buf, "%02x", data[i]);
        hex += buf;
    }
    
    return hex;
}

bool StellarUtils::hexDecode(const char* input, uint8_t* output, size_t* outputLength) {
    if (!input || !output || !outputLength) {
        return false;
    }
    
    size_t inputLength = strlen(input);
    if (inputLength % 2 != 0) {
        return false;
    }
    
    *outputLength = inputLength / 2;
    
    for (size_t i = 0; i < *outputLength; i++) {
        char high = input[i * 2];
        char low = input[i * 2 + 1];
        
        uint8_t h, l;
        
        // Convertir high nibble
        if (high >= '0' && high <= '9') h = high - '0';
        else if (high >= 'a' && high <= 'f') h = high - 'a' + 10;
        else if (high >= 'A' && high <= 'F') h = high - 'A' + 10;
        else return false;
        
        // Convertir low nibble
        if (low >= '0' && low <= '9') l = low - '0';
        else if (low >= 'a' && low <= 'f') l = low - 'a' + 10;
        else if (low >= 'A' && low <= 'F') l = low - 'A' + 10;
        else return false;
        
        output[i] = (h << 4) | l;
    }

    return true;
}

String StellarUtils::urlEncode(const String& str) {
    String encoded = "";
    char c;
    char code0;
    char code1;

    for (size_t i = 0; i < str.length(); i++) {
        c = str.charAt(i);

        // Caracteres seguros que no necesitan encoding
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else {
            // Convertir a %XX
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9) {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9) {
                code0 = c - 10 + 'A';
            }
            encoded += '%';
            encoded += code0;
            encoded += code1;
        }
    }

    return encoded;
}

// ============================================
// CHECKSUMS
// ============================================

uint16_t StellarUtils::crc16XModem(const uint8_t* data, size_t length) {
    uint16_t crc = 0x0000;
    
    for (size_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
        
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc = crc << 1;
            }
        }
    }
    
    return crc;
}

// ============================================
// VALIDACIÓN
// ============================================

bool StellarUtils::isValidAddress(const char* address) {
    if (!address) return false;
    
    size_t len = strlen(address);
    if (len != 56) return false;
    
    // Debe empezar con G (public) o S (secret)
    char first = address[0];
    if (first != 'G' && first != 'S') return false;
    
    // Verificar caracteres base32 válidos
    const char* validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    for (size_t i = 1; i < len; i++) {
        if (!strchr(validChars, address[i])) {
            return false;
        }
    }
    
    return true;
}

bool StellarUtils::isValidAmount(float xlm) {
    if (xlm <= 0.0f) return false;
    
    // Máximo XLM que cabe en int64 stroops
    // 9223372036854775807 stroops = 922337203685.4775807 XLM
    if (xlm > 922337203685.4775807f) return false;
    
    return true;
}

bool StellarUtils::isValidMemo(const char* memo) {
    if (!memo) return true;  // NULL memo es válido
    
    size_t len = strlen(memo);
    return (len <= 28);  // Stellar limit
}

// ============================================
// LOGGING Y DEBUG
// ============================================

void StellarUtils::debugPrint(const char* tag, const char* message) {
    #if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    Serial.print("[DEBUG][");
    Serial.print(millis());
    Serial.print("][");
    Serial.print(tag);
    Serial.print("] ");
    Serial.println(message);
    #endif
}

void StellarUtils::debugPrintHex(const char* tag, const uint8_t* data, size_t length) {
    #if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    Serial.print("[DEBUG][");
    Serial.print(millis());
    Serial.print("][");
    Serial.print(tag);
    Serial.print("] HEX: ");
    
    for (size_t i = 0; i < length; i++) {
        if (data[i] < 16) Serial.print("0");
        Serial.print(data[i], HEX);
        Serial.print(" ");
        
        // Nueva línea cada 16 bytes
        if ((i + 1) % 16 == 0 && i < length - 1) {
            Serial.print("\n                              ");
        }
    }
    Serial.println();
    #endif
}

void StellarUtils::errorPrint(const char* tag, const char* error) {
    #if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_ERROR
    Serial.print("[ERROR][");
    Serial.print(millis());
    Serial.print("][");
    Serial.print(tag);
    Serial.print("] ");
    Serial.println(error);
    #endif
}

void StellarUtils::infoPrint(const char* tag, const char* info) {
    #if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
    Serial.print("[INFO][");
    Serial.print(millis());
    Serial.print("][");
    Serial.print(tag);
    Serial.print("] ");
    Serial.println(info);
    #endif
}

// ============================================
// HELPERS DE MEMORIA
// ============================================

void StellarUtils::secureZero(void* buffer, size_t size) {
    if (!buffer || size == 0) return;
    
    // Usar volatile para prevenir optimización del compilador
    volatile uint8_t* ptr = (volatile uint8_t*)buffer;
    while (size--) {
        *ptr++ = 0;
    }
}

uint32_t StellarUtils::getFreeHeap() {
    return esp_get_free_heap_size();
}

String StellarUtils::getMemoryInfo() {
    String info = "";
    info += "Free Heap: ";
    info += String(esp_get_free_heap_size());
    info += " bytes\n";
    info += "Min Free Heap: ";
    info += String(esp_get_minimum_free_heap_size());
    info += " bytes\n";
    info += "Largest Free Block: ";
    info += String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    info += " bytes";
    return info;
}