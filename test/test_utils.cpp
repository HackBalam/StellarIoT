#include <unity.h>
#include "../src/stellar_utils.h"

void test_stroops_to_xlm() {
    TEST_ASSERT_EQUAL_FLOAT(1.0f, StellarUtils::stroopsToXLM(10000000));
    TEST_ASSERT_EQUAL_FLOAT(0.5f, StellarUtils::stroopsToXLM(5000000));
    TEST_ASSERT_EQUAL_FLOAT(10.123456f, StellarUtils::stroopsToXLM(101234560));
}

void test_xlm_to_stroops() {
    TEST_ASSERT_EQUAL_INT64(10000000, StellarUtils::xlmToStroops(1.0f));
    TEST_ASSERT_EQUAL_INT64(5000000, StellarUtils::xlmToStroops(0.5f));
    TEST_ASSERT_EQUAL_INT64(101234560, StellarUtils::xlmToStroops(10.123456f));
}

void test_base64_encode() {
    uint8_t data[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};  // "Hello"
    String encoded = StellarUtils::base64Encode(data, 5);
    TEST_ASSERT_EQUAL_STRING("SGVsbG8=", encoded.c_str());
}

void test_base64_decode() {
    uint8_t output[10];
    size_t outputLength;
    bool success = StellarUtils::base64Decode("SGVsbG8=", output, &outputLength);
    
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL(5, outputLength);
    TEST_ASSERT_EQUAL_UINT8(0x48, output[0]);  // H
    TEST_ASSERT_EQUAL_UINT8(0x65, output[1]);  // e
    TEST_ASSERT_EQUAL_UINT8(0x6C, output[2]);  // l
    TEST_ASSERT_EQUAL_UINT8(0x6C, output[3]);  // l
    TEST_ASSERT_EQUAL_UINT8(0x6F, output[4]);  // o
}

void test_hex_encode() {
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    String hex = StellarUtils::hexEncode(data, 4);
    TEST_ASSERT_EQUAL_STRING("deadbeef", hex.c_str());
}

void test_hex_decode() {
    uint8_t output[4];
    size_t outputLength;
    bool success = StellarUtils::hexDecode("deadbeef", output, &outputLength);
    
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL(4, outputLength);
    TEST_ASSERT_EQUAL_UINT8(0xDE, output[0]);
    TEST_ASSERT_EQUAL_UINT8(0xAD, output[1]);
    TEST_ASSERT_EQUAL_UINT8(0xBE, output[2]);
    TEST_ASSERT_EQUAL_UINT8(0xEF, output[3]);
}

void test_crc16_xmodem() {
    uint8_t data[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
    uint16_t crc = StellarUtils::crc16XModem(data, 5);
    
    // CRC16-XModem de "Hello" = 0x3130
    TEST_ASSERT_EQUAL_UINT16(0x3130, crc);
}

void test_valid_address() {
    // Public key válida
    TEST_ASSERT_TRUE(StellarUtils::isValidAddress(
        "GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H"
    ));
    
    // Secret key válida
    TEST_ASSERT_TRUE(StellarUtils::isValidAddress(
        "SBGWSG6BTNCKCOB3DIFBGCVMUPQFYPA2G4O34RMTB343OYPXU5DJDVMN"
    ));
    
    // Inválidas
    TEST_ASSERT_FALSE(StellarUtils::isValidAddress("INVALID"));
    TEST_ASSERT_FALSE(StellarUtils::isValidAddress(""));
    TEST_ASSERT_FALSE(StellarUtils::isValidAddress(nullptr));
}

void test_valid_amount() {
    TEST_ASSERT_TRUE(StellarUtils::isValidAmount(1.0f));
    TEST_ASSERT_TRUE(StellarUtils::isValidAmount(0.0000001f));
    TEST_ASSERT_TRUE(StellarUtils::isValidAmount(1000000.0f));
    
    TEST_ASSERT_FALSE(StellarUtils::isValidAmount(0.0f));
    TEST_ASSERT_FALSE(StellarUtils::isValidAmount(-1.0f));
}

void test_valid_memo() {
    TEST_ASSERT_TRUE(StellarUtils::isValidMemo("Hello World"));
    TEST_ASSERT_TRUE(StellarUtils::isValidMemo("1234567890123456789012345678"));  // 28 chars
    TEST_ASSERT_TRUE(StellarUtils::isValidMemo(""));
    TEST_ASSERT_TRUE(StellarUtils::isValidMemo(nullptr));
    
    TEST_ASSERT_FALSE(StellarUtils::isValidMemo("12345678901234567890123456789"));  // 29 chars
}

void setup() {
    delay(2000);  // Esperar a que el serial esté listo
    
    UNITY_BEGIN();
    
    RUN_TEST(test_stroops_to_xlm);
    RUN_TEST(test_xlm_to_stroops);
    RUN_TEST(test_base64_encode);
    RUN_TEST(test_base64_decode);
    RUN_TEST(test_hex_encode);
    RUN_TEST(test_hex_decode);
    RUN_TEST(test_crc16_xmodem);
    RUN_TEST(test_valid_address);
    RUN_TEST(test_valid_amount);
    RUN_TEST(test_valid_memo);
    
    UNITY_END();
}

void loop() {
    // Nada aquí
}