#include "stellar_network.h"
#include "stellar_utils.h"

// Definir constantes estáticas
const char* StellarNetwork::TESTNET_HORIZON = "https://horizon-testnet.stellar.org";
const char* StellarNetwork::MAINNET_HORIZON = "https://horizon.stellar.org";
const char* StellarNetwork::FRIENDBOT_URL = "https://friendbot.stellar.org";
const char* StellarNetwork::TESTNET_PASSPHRASE = "Test SDF Network ; September 2015";
const char* StellarNetwork::MAINNET_PASSPHRASE = "Public Global Stellar Network ; September 2015";

// ============================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================

StellarNetwork::StellarNetwork(NetworkType type) {
    networkType = type;
    timeout = 30000;  // 30 segundos
    maxRetries = 3;
    lastError = "";
    
    // Configurar URLs según red
    if (type == STELLAR_TESTNET) {
        horizonUrl = TESTNET_HORIZON;
        networkPassphrase = TESTNET_PASSPHRASE;
    } else {
        horizonUrl = MAINNET_HORIZON;
        networkPassphrase = MAINNET_PASSPHRASE;
    }
    
    StellarUtils::infoPrint("Network", 
        type == STELLAR_TESTNET ? "Initialized (TESTNET)" : "Initialized (MAINNET)");
}

StellarNetwork::~StellarNetwork() {
    // Nada que limpiar
}

// ============================================
// CONFIGURACIÓN
// ============================================

void StellarNetwork::setNetwork(NetworkType type) {
    networkType = type;
    
    if (type == STELLAR_TESTNET) {
        horizonUrl = TESTNET_HORIZON;
        networkPassphrase = TESTNET_PASSPHRASE;
        StellarUtils::infoPrint("Network", "Switched to TESTNET");
    } else {
        horizonUrl = MAINNET_HORIZON;
        networkPassphrase = MAINNET_PASSPHRASE;
        StellarUtils::infoPrint("Network", "Switched to MAINNET");
    }
}

void StellarNetwork::setHorizonURL(const char* url) {
    if (url) {
        horizonUrl = String(url);
        StellarUtils::infoPrint("Network", ("Custom Horizon: " + horizonUrl).c_str());
    }
}

void StellarNetwork::setTimeout(uint32_t seconds) {
    timeout = seconds * 1000;
    StellarUtils::debugPrint("Network", ("Timeout set to " + String(seconds) + "s").c_str());
}

void StellarNetwork::setMaxRetries(uint8_t count) {
    maxRetries = count;
    StellarUtils::debugPrint("Network", ("Max retries: " + String(count)).c_str());
}

// ============================================
// ESTADO
// ============================================

bool StellarNetwork::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

// ============================================
// HTTP METHODS GENÉRICOS
// ============================================

String StellarNetwork::httpGet(const char* endpoint) {
    if (!isConnected()) {
        lastError = "WiFi not connected";
        StellarUtils::errorPrint("Network", lastError.c_str());
        return "";
    }
    
    String url = horizonUrl + String(endpoint);
    StellarUtils::debugPrint("Network", ("GET " + url).c_str());
    
    return httpGetWithRetry(url.c_str());
}

String StellarNetwork::httpPost(const char* endpoint, const char* body) {
    if (!isConnected()) {
        lastError = "WiFi not connected";
        StellarUtils::errorPrint("Network", lastError.c_str());
        return "";
    }
    
    String url = horizonUrl + String(endpoint);
    StellarUtils::debugPrint("Network", ("POST " + url).c_str());
    
    return httpPostWithRetry(url.c_str(), body);
}

// ============================================
// HTTP HELPERS CON RETRY
// ============================================

String StellarNetwork::httpGetWithRetry(const char* url) {
    HTTPClient http;
    WiFiClientSecure client;
    String response = "";

    // Permitir conexiones HTTPS sin verificar certificado (para IoT)
    client.setInsecure();

    for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
        if (attempt > 0) {
            uint32_t backoff = 1000 * (1 << attempt);  // Exponential backoff
            StellarUtils::debugPrint("Network",
                ("Retry #" + String(attempt + 1) + " after " + String(backoff) + "ms").c_str());
            delay(backoff);
        }

        http.begin(client, url);
        http.setTimeout(timeout);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("User-Agent", "Stellar-IoT-SDK/0.1.0");
        
        int httpCode = http.GET();
        
        if (httpCode == HTTP_CODE_OK) {
            response = http.getString();
            http.end();
            lastError = "";
            
            StellarUtils::debugPrint("Network", "Request successful");
            return response;
            
        } else if (httpCode > 0) {
            // Obtener respuesta de error
            response = http.getString();
            parseError(response);
            http.end();
            
            // No reintentar errores 4xx (client errors)
            if (httpCode >= 400 && httpCode < 500) {
                StellarUtils::errorPrint("Network", 
                    ("HTTP " + String(httpCode) + ": " + lastError).c_str());
                return "";
            }
            
        } else {
            lastError = "HTTP request failed: " + http.errorToString(httpCode);
            StellarUtils::errorPrint("Network", lastError.c_str());
        }
        
        http.end();
    }
    
    lastError = "Max retries exceeded";
    StellarUtils::errorPrint("Network", lastError.c_str());
    return "";
}

String StellarNetwork::httpPostWithRetry(const char* url, const char* body) {
    HTTPClient http;
    WiFiClientSecure client;
    String response = "";

    // Permitir conexiones HTTPS sin verificar certificado (para IoT)
    client.setInsecure();

    for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
        if (attempt > 0) {
            uint32_t backoff = 1000 * (1 << attempt);
            StellarUtils::debugPrint("Network",
                ("Retry #" + String(attempt + 1) + " after " + String(backoff) + "ms").c_str());
            delay(backoff);
        }

        http.begin(client, url);
        http.setTimeout(timeout);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        http.addHeader("User-Agent", "Stellar-IoT-SDK/0.1.0");
        
        int httpCode = http.POST(body);
        
        if (httpCode == HTTP_CODE_OK) {
            response = http.getString();
            http.end();
            lastError = "";
            
            StellarUtils::debugPrint("Network", "POST successful");
            return response;
            
        } else if (httpCode > 0) {
            response = http.getString();
            parseError(response);
            http.end();
            
            // No reintentar errores 4xx
            if (httpCode >= 400 && httpCode < 500) {
                StellarUtils::errorPrint("Network", 
                    ("HTTP " + String(httpCode) + ": " + lastError).c_str());
                return "";
            }
            
        } else {
            lastError = "HTTP request failed: " + http.errorToString(httpCode);
            StellarUtils::errorPrint("Network", lastError.c_str());
        }
        
        http.end();
    }
    
    lastError = "Max retries exceeded";
    StellarUtils::errorPrint("Network", lastError.c_str());
    return "";
}

void StellarNetwork::parseError(const String& response) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        lastError = "Failed to parse error response";
        return;
    }
    
    // Intentar extraer mensaje de error de Horizon
    if (doc.containsKey("title")) {
        lastError = doc["title"].as<String>();
    } else if (doc.containsKey("detail")) {
        lastError = doc["detail"].as<String>();
    } else {
        lastError = "Unknown error";
    }

    // Siempre intentar extraer result_codes si existen
    if (doc.containsKey("extras") && doc["extras"].containsKey("result_codes")) {
        JsonObject resultCodes = doc["extras"]["result_codes"];
        if (resultCodes.containsKey("transaction")) {
            lastError += " [" + resultCodes["transaction"].as<String>() + "]";
        }
        if (resultCodes.containsKey("operations")) {
            JsonArray ops = resultCodes["operations"];
            for (size_t i = 0; i < ops.size(); i++) {
                lastError += " op[" + String(i) + "]:" + ops[i].as<String>();
            }
        }
    }
}

// ============================================
// HORIZON API ESPECÍFICOS
// ============================================

String StellarNetwork::getAccount(const char* accountId) {
    if (!StellarUtils::isValidAddress(accountId) || accountId[0] != 'G') {
        lastError = "Invalid account ID";
        StellarUtils::errorPrint("Network", lastError.c_str());
        return "";
    }
    
    String endpoint = "/accounts/" + String(accountId);
    return httpGet(endpoint.c_str());
}

String StellarNetwork::getAccountPayments(
    const char* accountId,
    const char* cursor,
    uint8_t limit
) {
    if (!StellarUtils::isValidAddress(accountId) || accountId[0] != 'G') {
        lastError = "Invalid account ID";
        StellarUtils::errorPrint("Network", lastError.c_str());
        return "";
    }
    
    String endpoint = "/accounts/" + String(accountId) + 
                      "/payments?order=desc&limit=" + String(limit);
    
    if (cursor != nullptr && strlen(cursor) > 0) {
        endpoint += "&cursor=" + String(cursor);
    }
    
    return httpGet(endpoint.c_str());
}

String StellarNetwork::submitTransaction(const char* txXdrBase64) {
    if (!txXdrBase64 || strlen(txXdrBase64) == 0) {
        lastError = "Empty transaction XDR";
        StellarUtils::errorPrint("Network", lastError.c_str());
        return "";
    }
    
    StellarUtils::infoPrint("Network", "Submitting transaction...");

    String body = "tx=" + StellarUtils::urlEncode(String(txXdrBase64));
    String response = httpPost("/transactions", body.c_str());
    
    if (response.length() > 0) {
        StellarUtils::infoPrint("Network", "Transaction submitted successfully");
    }
    
    return response;
}

String StellarNetwork::getTransaction(const char* txHash) {
    if (!txHash || strlen(txHash) != 64) {
        lastError = "Invalid transaction hash";
        StellarUtils::errorPrint("Network", lastError.c_str());
        return "";
    }
    
    String endpoint = "/transactions/" + String(txHash);
    return httpGet(endpoint.c_str());
}

bool StellarNetwork::fundWithFriendbot(const char* accountId) {
    if (networkType != STELLAR_TESTNET) {
        lastError = "Friendbot only available on testnet";
        StellarUtils::errorPrint("Network", lastError.c_str());
        return false;
    }
    
    if (!StellarUtils::isValidAddress(accountId) || accountId[0] != 'G') {
        lastError = "Invalid account ID";
        StellarUtils::errorPrint("Network", lastError.c_str());
        return false;
    }
    
    StellarUtils::infoPrint("Network", "Funding account with Friendbot...");
    
    String url = String(FRIENDBOT_URL) + "/?addr=" + String(accountId);
    String response = httpGetWithRetry(url.c_str());
    
    bool success = (response.length() > 0);
    
    if (success) {
        StellarUtils::infoPrint("Network", "Account funded successfully (10,000 XLM)");
    } else {
        StellarUtils::errorPrint("Network", "Friendbot funding failed");
    }
    
    return success;
}