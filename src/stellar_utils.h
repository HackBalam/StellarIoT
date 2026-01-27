#ifndef STELLAR_UTILS_H
#define STELLAR_UTILS_H

#include <Arduino.h>

class StellarUtils {
public:
    // CONVERSIONES DE MONEDA
    static float stroopsToXLM(int64_t stroops);
    static int64_t xlmToStroops(float xlm);
    
    // ENCODING/DECODING
    static String base64Encode(const uint8_t* data, size_t length);
    static bool base64Decode(const char* input, uint8_t* output, size_t* outputLength);
    static String hexEncode(const uint8_t* data, size_t length);
    static bool hexDecode(const char* input, uint8_t* output, size_t* outputLength);
    static String urlEncode(const String& str);
    
    // CHECKSUMS
    static uint16_t crc16XModem(const uint8_t* data, size_t length);
    
    // VALIDACIÓN
    static bool isValidAddress(const char* address);
    static bool isValidAmount(float xlm);
    static bool isValidMemo(const char* memo);
    
    // LOGGING Y DEBUG
    static void debugPrint(const char* tag, const char* message);
    static void debugPrintHex(const char* tag, const uint8_t* data, size_t length);
    static void errorPrint(const char* tag, const char* error);
    static void infoPrint(const char* tag, const char* info);
    
    // HELPERS DE MEMORIA
    static void secureZero(void* buffer, size_t size);
    static uint32_t getFreeHeap();
    static String getMemoryInfo();
};

#endif