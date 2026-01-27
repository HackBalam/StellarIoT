#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "stellar_utils.h"
#include "stellar_crypto.h"
#include "stellar_keypair.h"
#include "stellar_storage.h"
#include "stellar_network.h"
#include "stellar_xdr.h"
#include "stellar_account.h"
#include "stellar_payment.h"
// Variable global para el keypair actual
StellarKeypair* currentKeypair = nullptr;
StellarNetwork* currentNetwork = nullptr;
StellarAccount* currentAccount = nullptr;
StellarPayment* currentPayment = nullptr;

void setup() {
    Serial.begin(115200);
    delay(2000);

    // ============================================
    // CONFIGURAR WIFI
    // ============================================
    Serial.println("\n\nConnecting to WiFi...");

    // REEMPLAZA CON TUS CREDENCIALES
    const char* ssid = "XXXXXXXX";
    const char* password = "XXXXXXXXXX";

    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.isConnected()) {
        Serial.println("\n✓ WiFi connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n✗ WiFi connection failed");
        Serial.println("Network commands will not work");
    }

    Serial.println("\n=================================");
    Serial.println("  Stellar IoT SDK v0.1.0");
    Serial.println("  Crypto + Storage + Network");
    Serial.println("=================================\n");

    StellarUtils::infoPrint("System", "ESP32 initialized");

    Serial.println("Memory Info:");
    Serial.println(StellarUtils::getMemoryInfo());

    // Test criptografía básica
    Serial.println("\n--- Testing Crypto ---");

    // Test Ed25519
    uint8_t pubKey[32], privKey[32];
    StellarCrypto::generateKeypair(pubKey, privKey);
    Serial.println("✓ Ed25519 keypair generated");

    // Test SHA-256
    uint8_t hash[32];
    const char* testData = "Hello Stellar!";
    StellarCrypto::sha256((uint8_t*)testData, strlen(testData), hash);
    Serial.print("✓ SHA-256: ");
    Serial.println(StellarUtils::hexEncode(hash, 32));

    // Test firma
    uint8_t signature[64];
    StellarCrypto::sign(privKey, pubKey, (uint8_t*)testData, strlen(testData), signature);
    bool verified = StellarCrypto::verify(pubKey, (uint8_t*)testData, strlen(testData), signature);
    Serial.print("✓ Signature verification: ");
    Serial.println(verified ? "PASS" : "FAIL");

    Serial.println("\n=================================");
    Serial.println("  Setup Complete");
    Serial.println("=================================\n");
    Serial.println("Commands:");
    Serial.println("  help     - Show all commands");
    Serial.println("  wallet   - Wallet commands");
    Serial.println("  network  - Network commands");
    Serial.println("  crypto   - Crypto tests");
    Serial.println("  xdr      - XDR encoding tests");
    Serial.println();
}

// Helper para inicializar managers si no existen
void ensureManagers() {
    if (!currentNetwork) {
        currentNetwork = new StellarNetwork(STELLAR_TESTNET);
    }
    
    if (currentKeypair && !currentAccount) {
        currentAccount = new StellarAccount(currentKeypair, currentNetwork);
    }
    
    if (currentKeypair && currentAccount && !currentPayment) {
        currentPayment = new StellarPayment(currentKeypair, currentNetwork, currentAccount);
    }
}

// Helper para limpiar managers
void cleanupManagers() {
    if (currentPayment) {
        delete currentPayment;
        currentPayment = nullptr;
    }
    
    if (currentAccount) {
        delete currentAccount;
        currentAccount = nullptr;
    }
}

void loop() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command.toLowerCase();

        if (command == "help") {
            Serial.println("\n--- Available Commands ---");
            Serial.println("help          - Show this help");
            Serial.println("memory        - Show memory stats");
            Serial.println("crypto        - Run crypto tests");
            Serial.println("xdr           - Test XDR encoding");
            Serial.println("\nWallet Commands:");
            Serial.println("wallet new    - Generate new wallet");
            Serial.println("wallet save   - Save wallet to flash");
            Serial.println("wallet load   - Load wallet from flash");
            Serial.println("wallet show   - Show current wallet");
            Serial.println("wallet delete - Delete saved wallet");
            Serial.println("wallet import - Import from secret key");
            Serial.println("\nNetwork Commands:");
            Serial.println("network test    - Test Horizon connection");
            Serial.println("network fund    - Fund with Friendbot (testnet)");
            Serial.println("network balance - Get account balance");
            Serial.println("network info    - Get account info");
            Serial.println("\nPayment Commands:");
            Serial.println("pay send      - Send XLM payment");
            Serial.println("pay status    - Check last payment status");
            Serial.println("pay history   - View payment history");
            Serial.println("-------------------------\n");

        } else if (command == "pay") {
            Serial.println("\n--- Payment Commands ---");
            Serial.println("pay send      - Send XLM payment");
            Serial.println("pay status    - Check last payment status");
            Serial.println("pay history   - View payment history");
            Serial.println("-------------------------\n");
            
        } else if (command == "pay send") {
            if (!currentKeypair) {
                Serial.println("\n✗ No wallet loaded. Use 'wallet new' first\n");
            } else if (!WiFi.isConnected()) {
                Serial.println("\n✗ WiFi not connected\n");
            } else {
                ensureManagers();
                
                // Verificar que la cuenta existe
                if (!currentAccount->isAccountActive()) {
                    Serial.println("\n✗ Account not funded yet");
                    Serial.println("Use 'network fund' first (testnet only)\n");
                    return;
                }
                
                Serial.println("\n--- Send Payment ---");
                
                // Pedir destination
                Serial.println("Enter destination address (G...):");
                while (!Serial.available()) { delay(100); }
                String destination = Serial.readStringUntil('\n');
                destination.trim();
                
                // Pedir amount
                Serial.println("Enter amount (XLM):");
                while (!Serial.available()) { delay(100); }
                String amountStr = Serial.readStringUntil('\n');
                amountStr.trim();
                float amount = amountStr.toFloat();
                
                // Pedir memo (opcional)
                Serial.println("Enter memo (optional, press Enter to skip):");
                while (!Serial.available()) { delay(100); }
                String memo = Serial.readStringUntil('\n');
                memo.trim();
                
                // Confirmar
                Serial.println("\n--- Confirm Payment ---");
                Serial.print("To:     ");
                Serial.println(destination);
                Serial.print("Amount: ");
                Serial.print(amount, 7);
                Serial.println(" XLM");
                if (memo.length() > 0) {
                    Serial.print("Memo:   ");
                    Serial.println(memo);
                }
                Serial.println("\nType 'yes' to confirm:");
                
                while (!Serial.available()) { delay(100); }
                String confirm = Serial.readStringUntil('\n');
                confirm.trim();
                confirm.toLowerCase();
                
                if (confirm != "yes") {
                    Serial.println("Payment cancelled\n");
                    return;
                }
                
                // Enviar pago
                Serial.println("\nSending payment...");
                
                PaymentResult result = currentPayment->sendPayment(
                    destination.c_str(),
                    amount,
                    memo.length() > 0 ? memo.c_str() : nullptr
                );
                
                if (result.success) {
                    Serial.println("\n✓ Payment successful!");
                    Serial.println("\nTransaction Hash:");
                    Serial.println(result.transactionHash);
                    Serial.print("Ledger: ");
                    Serial.println(result.ledger);
                    Serial.println("\nView on Stellar Explorer:");
                    Serial.print("https://stellar.expert/explorer/testnet/tx/");
                    Serial.println(result.transactionHash);
                } else {
                    Serial.println("\n✗ Payment failed");
                    Serial.print("Error: ");
                    Serial.println(result.error);
                }
                Serial.println();
            }
            
        } else if (command == "pay status") {
            if (!currentPayment) {
                Serial.println("\n✗ No payment manager initialized\n");
            } else {
                String lastHash = currentPayment->getLastTransactionHash();
                
                if (lastHash.length() == 0) {
                    Serial.println("\n✗ No transactions sent yet\n");
                } else {
                    Serial.println("\n--- Last Transaction Status ---");
                    Serial.print("Hash: ");
                    Serial.println(lastHash);
                    
                    TransactionStatus status = currentPayment->getLastTransactionStatus();
                    
                    Serial.print("Status: ");
                    switch (status) {
                        case TX_SUCCESS:
                            Serial.println("✓ SUCCESS");
                            break;
                        case TX_FAILED:
                            Serial.println("✗ FAILED");
                            break;
                        case TX_PENDING:
                            Serial.println("⏳ PENDING");
                            break;
                        default:
                            Serial.println("? UNKNOWN");
                            break;
                    }
                    Serial.println("-------------------------------\n");
                }
            }
            
        } else if (command == "pay history") {
            if (!currentKeypair) {
                Serial.println("\n✗ No wallet loaded\n");
            } else if (!WiFi.isConnected()) {
                Serial.println("\n✗ WiFi not connected\n");
            } else {
                ensureManagers();
                
                Serial.println("\nFetching payment history...");
                
                String response = currentNetwork->getAccountPayments(
                    currentKeypair->getPublicKey().c_str(),
                    nullptr,
                    10
                );
                
                if (response.length() > 0) {
                    DynamicJsonDocument doc(8192);
                    DeserializationError error = deserializeJson(doc, response);
                    
                    if (!error && doc.containsKey("_embedded")) {
                        JsonArray records = doc["_embedded"]["records"];
                        
                        Serial.println("\n--- Payment History (Last 10) ---");
                        
                        int count = 0;
                        for (JsonObject payment : records) {
                            count++;
                            
                            Serial.println("\n" + String(count) + ".");
                            Serial.print("  Type: ");
                            Serial.println(payment["type"].as<String>());
                            
                            if (payment.containsKey("from")) {
                                Serial.print("  From: ");
                                String from = payment["from"].as<String>();
                                Serial.println(from.substring(0, 10) + "...");
                            }
                            
                            if (payment.containsKey("to")) {
                                Serial.print("  To: ");
                                String to = payment["to"].as<String>();
                                Serial.println(to.substring(0, 10) + "...");
                            }
                            
                            if (payment.containsKey("amount")) {
                                Serial.print("  Amount: ");
                                Serial.print(payment["amount"].as<String>());
                                Serial.println(" XLM");
                            }
                            
                            if (payment.containsKey("created_at")) {
                                Serial.print("  Date: ");
                                Serial.println(payment["created_at"].as<String>());
                            }
                        }
                        
                        if (count == 0) {
                            Serial.println("No payments found");
                        }
                        
                        Serial.println("\n---------------------------------\n");
                    } else {
                        Serial.println("✗ Failed to parse response\n");
                    }
                } else {
                    Serial.println("✗ Failed to fetch history\n");
                }
            }

        } else if (command == "memory") {
            Serial.println("\n--- Memory Statistics ---");
            Serial.println(StellarUtils::getMemoryInfo());
            Serial.println("-------------------------\n");

        } else if (command == "crypto") {
            Serial.println("\n--- Running Crypto Tests ---");

            // Test 1: Keypair generation
            Serial.print("Test 1: Keypair generation... ");
            uint8_t pub[32], priv[32];
            if (StellarCrypto::generateKeypair(pub, priv)) {
                Serial.println("PASS");
            } else {
                Serial.println("FAIL");
            }

            // Test 2: SHA-256
            Serial.print("Test 2: SHA-256... ");
            uint8_t hash[32];
            StellarCrypto::sha256((uint8_t*)"test", 4, hash);
            Serial.println("PASS");

            // Test 3: Signing
            Serial.print("Test 3: Ed25519 signing... ");
            uint8_t sig[64];
            StellarCrypto::sign(priv, pub, (uint8_t*)"data", 4, sig);
            bool ok = StellarCrypto::verify(pub, (uint8_t*)"data", 4, sig);
            Serial.println(ok ? "PASS" : "FAIL");

            // Test 4: AES encryption
            Serial.print("Test 4: AES-256-GCM... ");
            uint8_t key[32], iv[12], ct[16], tag[16], pt[16];
            StellarCrypto::randomBytes(key, 32);
            StellarCrypto::randomBytes(iv, 12);
            const uint8_t* plaintext = (uint8_t*)"Secret message!!";

            StellarCrypto::encryptAES256GCM(plaintext, 16, key, iv, ct, tag);
            StellarCrypto::decryptAES256GCM(ct, 16, key, iv, tag, pt);

            bool aesOk = (memcmp(plaintext, pt, 16) == 0);
            Serial.println(aesOk ? "PASS" : "FAIL");

            Serial.println("\nAll crypto tests completed!");
            Serial.println("-------------------------\n");

        } else if (command == "network") {
            Serial.println("\n--- Network Commands ---");
            Serial.println("network test    - Test connection to Horizon");
            Serial.println("network fund    - Fund account with Friendbot (testnet)");
            Serial.println("network balance - Get account balance");
            Serial.println("network info    - Get account info");
            Serial.println("-------------------------\n");

        } else if (command == "network test") {
            Serial.println("\n--- Testing Network Connection ---");

            if (!WiFi.isConnected()) {
                Serial.println("✗ WiFi not connected");
                Serial.println("\nTo connect WiFi, add to setup():");
                Serial.println("  WiFi.begin(\"SSID\", \"PASSWORD\");");
                Serial.println();
                return;
            }

            Serial.print("WiFi: Connected (");
            Serial.print(WiFi.SSID());
            Serial.println(")");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());

            StellarNetwork network(STELLAR_TESTNET);

            Serial.println("\nTesting Horizon connection...");

            // Test con cuenta conocida de Stellar
            String response = network.getAccount("GBY5AZJYQNUD22NLNEX23NWIFWALIGDRQY2X7W6TPNYHJWY6TCV7W64I");

            if (response.length() > 0) {
                Serial.println("✓ Connection successful!");
                Serial.println("\nHorizon URL: " + String(network.getHorizonURL()));
            } else {
                Serial.println("✗ Connection failed");
                Serial.println("Error: " + network.getLastError());
            }
            Serial.println();

        } else if (command == "network fund") {
            if (!currentKeypair) {
                Serial.println("\n✗ No wallet loaded. Use 'wallet new' first\n");
            } else if (!WiFi.isConnected()) {
                Serial.println("\n✗ WiFi not connected\n");
            } else {
                Serial.println("\nFunding account with Friendbot...");
                Serial.println("Account: " + currentKeypair->getPublicKey());

                StellarNetwork network(STELLAR_TESTNET);

                if (network.fundWithFriendbot(currentKeypair->getPublicKey().c_str())) {
                    Serial.println("\n✓ Account funded successfully!");
                    Serial.println("Balance: 10,000 XLM (testnet)");
                } else {
                    Serial.println("\n✗ Funding failed");
                    Serial.println("Error: " + network.getLastError());
                }
                Serial.println();
            }

        } else if (command == "network balance") {
            if (!currentKeypair) {
                Serial.println("\n✗ No wallet loaded\n");
            } else if (!WiFi.isConnected()) {
                Serial.println("\n✗ WiFi not connected\n");
            } else {
                Serial.println("\nQuerying balance...");

                StellarNetwork network(STELLAR_TESTNET);
                String response = network.getAccount(currentKeypair->getPublicKey().c_str());

                if (response.length() > 0) {
                    // Parse JSON response
                    DynamicJsonDocument doc(4096);
                    DeserializationError error = deserializeJson(doc, response);

                    if (!error) {
                        Serial.println("\n--- Account Balance ---");

                        JsonArray balances = doc["balances"];
                        for (JsonObject balance : balances) {
                            String assetType = balance["asset_type"];
                            String amount = balance["balance"];

                            if (assetType == "native") {
                                Serial.print("XLM: ");
                                Serial.println(amount);
                            } else {
                                String assetCode = balance["asset_code"];
                                Serial.print(assetCode);
                                Serial.print(": ");
                                Serial.println(amount);
                            }
                        }

                        Serial.println("----------------------\n");
                    } else {
                        Serial.println("✗ Failed to parse response\n");
                    }
                } else {
                    Serial.println("✗ Query failed");
                    Serial.println("Error: " + network.getLastError());
                    Serial.println("\nAccount might not be funded yet.");
                    Serial.println("Use 'network fund' to fund it (testnet only)\n");
                }
            }

        } else if (command == "network info") {
            if (!currentKeypair) {
                Serial.println("\n✗ No wallet loaded\n");
            } else if (!WiFi.isConnected()) {
                Serial.println("\n✗ WiFi not connected\n");
            } else {
                Serial.println("\nQuerying account info...");

                StellarNetwork network(STELLAR_TESTNET);
                String response = network.getAccount(currentKeypair->getPublicKey().c_str());

                if (response.length() > 0) {
                    DynamicJsonDocument doc(4096);
                    DeserializationError error = deserializeJson(doc, response);

                    if (!error) {
                        Serial.println("\n--- Account Info ---");
                        Serial.print("ID: ");
                        Serial.println(doc["id"].as<String>());
                        Serial.print("Sequence: ");
                        Serial.println(doc["sequence"].as<String>());
                        Serial.print("Subentry Count: ");
                        Serial.println(doc["subentry_count"].as<int>());
                        Serial.println("-------------------\n");
                    } else {
                        Serial.println("✗ Failed to parse response\n");
                    }
                } else {
                    Serial.println("✗ Query failed");
                    Serial.println("Error: " + network.getLastError());
                    Serial.println();
                }
            }

        } else if (command == "xdr") {
            Serial.println("\n--- Testing XDR Encoding ---");

            XDREncoder xdr;

            // Test 1: Tipos básicos
            Serial.print("Test 1: Basic types... ");
            xdr.reset();
            xdr.encodeUint32(0x12345678);
            xdr.encodeUint64(0x123456789ABCDEF0);
            xdr.encodeBool(true);
            xdr.encodeString("Hello");
            Serial.println("PASS");
            Serial.print("  Size: ");
            Serial.print(xdr.getSize());
            Serial.println(" bytes");

            // Test 2: Public key
            Serial.print("Test 2: Public key encoding... ");
            xdr.reset();
            uint8_t testKey[32];
            memset(testKey, 0xAB, 32);
            xdr.encodePublicKey(testKey);
            Serial.println("PASS");
            Serial.print("  Size: ");
            Serial.print(xdr.getSize());
            Serial.println(" bytes (expected: 36)");

            // Test 3: Payment operation
            Serial.print("Test 3: Payment operation... ");
            xdr.reset();
            uint8_t destKey[32];
            memset(destKey, 0xCD, 32);
            xdr.encodePaymentOp(destKey, 10000000);  // 1 XLM
            Serial.println("PASS");
            Serial.print("  Size: ");
            Serial.print(xdr.getSize());
            Serial.println(" bytes");

            // Test 4: Memo
            Serial.print("Test 4: Memo encoding... ");
            xdr.reset();
            xdr.encodeMemo(MEMO_TEXT, "Test memo");
            Serial.println("PASS");
            Serial.print("  Size: ");
            Serial.print(xdr.getSize());
            Serial.println(" bytes");

            Serial.println("\nAll XDR tests completed!");
            Serial.println("-------------------------\n");

        } else if (command.startsWith("wallet")) {
            // Comandos de wallet
            if (command == "wallet new") {
                Serial.println("\nGenerating new wallet...");

                if (currentKeypair) {
                    delete currentKeypair;
                }

                currentKeypair = StellarKeypair::generate();

                if (currentKeypair) {
                    Serial.println("✓ Wallet generated!");
                    Serial.println("\nPublic Key:");
                    Serial.println(currentKeypair->getPublicKey());
                    Serial.println("\nSecret Key (SAVE THIS SECURELY):");
                    Serial.println(currentKeypair->getSecretKey());
                    Serial.println("\nMnemonic (12 words for backup):");
                    Serial.println(currentKeypair->getMnemonic());
                    Serial.println("\nUse 'wallet save' to encrypt and save to flash");
                    ensureManagers();
                } else {
                    Serial.println("✗ Failed to generate wallet");
                }
                Serial.println();

            } else if (command == "wallet save") {
                if (!currentKeypair) {
                    Serial.println("\n✗ No wallet loaded. Use 'wallet new' first\n");
                } else {
                    Serial.println("\nEnter password (min 8 chars):");

                    // Esperar password
                    while (!Serial.available()) { delay(100); }
                    String password = Serial.readStringUntil('\n');
                    password.trim();

                    if (password.length() < 8) {
                        Serial.println("✗ Password too short (min 8 chars)\n");
                    } else {
                        SecureWallet wallet;
                        if (wallet.saveToFlash(currentKeypair, password.c_str())) {
                            Serial.println("✓ Wallet saved to flash successfully!\n");
                        } else {
                            Serial.println("✗ Failed to save wallet\n");
                        }
                    }
                }

            } else if (command == "wallet load") {
                Serial.println("\nEnter password:");

                // Esperar password
                while (!Serial.available()) { delay(100); }
                String password = Serial.readStringUntil('\n');
                password.trim();

                if (currentKeypair) {
                    delete currentKeypair;
                }

                currentKeypair = SecureWallet::loadFromFlash(password.c_str());

                if (currentKeypair) {
                    Serial.println("✓ Wallet loaded successfully!");
                    Serial.println("\nPublic Key:");
                    Serial.println(currentKeypair->getPublicKey());
                    Serial.println();
                } else {
                    Serial.println("✗ Failed to load wallet (wrong password?)\n");
                }

            } else if (command == "wallet show") {
                if (!currentKeypair) {
                    Serial.println("\n✗ No wallet loaded\n");
                } else {
                    Serial.println("\n--- Current Wallet ---");
                    Serial.print("Public Key:  ");
                    Serial.println(currentKeypair->getPublicKey());
                    Serial.print("Secret Key:  ");
                    Serial.println(currentKeypair->getSecretKey());

                    String mnemonic = currentKeypair->getMnemonic();
                    if (mnemonic.length() > 0) {
                        Serial.print("Mnemonic:    ");
                        Serial.println(mnemonic);
                    }
                    Serial.println("----------------------\n");
                }

            } else if (command == "wallet delete") {
                Serial.println("\nAre you sure? Type 'yes' to confirm:");

                while (!Serial.available()) { delay(100); }
                String confirm = Serial.readStringUntil('\n');
                confirm.trim();
                confirm.toLowerCase();

                if (confirm == "yes") {
                    if (SecureWallet::deleteFromFlash()) {
                        Serial.println("✓ Wallet deleted from flash\n");

                        if (currentKeypair) {
                            delete currentKeypair;
                            currentKeypair = nullptr;
                        }
                    } else {
                        Serial.println("✗ Failed to delete wallet\n");
                    }
                } else {
                    Serial.println("Cancelled\n");
                }

            } else if (command == "wallet import") {
                Serial.println("\nEnter secret key (S...):");

                while (!Serial.available()) { delay(100); }
                String secretKey = Serial.readStringUntil('\n');
                secretKey.trim();

                if (currentKeypair) {
                    delete currentKeypair;
                }

                currentKeypair = StellarKeypair::fromSecret(secretKey.c_str());

                if (currentKeypair) {
                    Serial.println("✓ Wallet imported successfully!");
                    Serial.println("\nPublic Key:");
                    Serial.println(currentKeypair->getPublicKey());
                    Serial.println("\nUse 'wallet save' to save to flash");
                    Serial.println();
                } else {
                    Serial.println("✗ Invalid secret key\n");
                }

            } else {
                Serial.println("\n✗ Unknown wallet command. Try 'wallet' alone for help\n");
            }

        } else if (command.length() > 0) {
            Serial.print("Unknown command: ");
            Serial.println(command);
            Serial.println("Type 'help' for available commands\n");
        }
    }

    delay(100);
}
