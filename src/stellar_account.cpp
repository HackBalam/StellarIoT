#include "stellar_account.h"
#include "stellar_utils.h"

// ============================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================

StellarAccount::StellarAccount(StellarKeypair* keypair, StellarNetwork* network) {
    this->keypair = keypair;
    this->network = network;
    this->lastError = "";
    this->cacheTimestamp = 0;
    
    // Inicializar caché vacío
    cachedInfo.accountId = keypair->getPublicKey();
    cachedInfo.sequence = "0";
    cachedInfo.subentryCount = 0;
    cachedInfo.nativeBalance = 0.0f;
    cachedInfo.exists = false;
    cachedInfo.lastError = "";
    
    StellarUtils::infoPrint("Account", "Account manager initialized");
}

StellarAccount::~StellarAccount() {
    // No poseemos keypair ni network, solo los usamos
}

// ============================================
// INFORMACIÓN DE CUENTA
// ============================================

AccountInfo StellarAccount::getAccountInfo() {
    StellarUtils::debugPrint("Account", "Getting account info");
    
    // Si el caché es válido, usarlo
    if (isCacheValid()) {
        StellarUtils::debugPrint("Account", "Using cached data");
        return cachedInfo;
    }
    
    // Actualizar caché
    if (!updateCache()) {
        // Retornar info con error
        AccountInfo errorInfo = cachedInfo;
        errorInfo.exists = false;
        errorInfo.lastError = lastError;
        return errorInfo;
    }
    
    return cachedInfo;
}

float StellarAccount::getBalance() {
    AccountInfo info = getAccountInfo();
    
    if (!info.exists) {
        StellarUtils::errorPrint("Account", "Account does not exist");
        return -1.0f;
    }
    
    return info.nativeBalance;
}

uint64_t StellarAccount::getSequenceNumber() {
    AccountInfo info = getAccountInfo();
    
    if (!info.exists) {
        StellarUtils::errorPrint("Account", "Account does not exist");
        return 0;
    }
    
    // Convertir string a uint64
    // Nota: Arduino no tiene strtoull, usamos workaround
    uint64_t sequence = 0;
    for (size_t i = 0; i < info.sequence.length(); i++) {
        char c = info.sequence[i];
        if (c >= '0' && c <= '9') {
            sequence = sequence * 10 + (c - '0');
        }
    }
    
    return sequence;
}

bool StellarAccount::isAccountActive() {
    AccountInfo info = getAccountInfo();
    return info.exists;
}

String StellarAccount::getPublicKey() const {
    return keypair->getPublicKey();
}

// ============================================
// FONDEO (Solo Testnet)
// ============================================

bool StellarAccount::fundAccount() {
    StellarUtils::infoPrint("Account", "Funding account with Friendbot");
    
    if (network->getNetworkType() != STELLAR_TESTNET) {
        lastError = "Friendbot only available on testnet";
        StellarUtils::errorPrint("Account", lastError.c_str());
        return false;
    }
    
    String publicKey = keypair->getPublicKey();
    
    if (!network->fundWithFriendbot(publicKey.c_str())) {
        lastError = "Friendbot request failed: " + network->getLastError();
        StellarUtils::errorPrint("Account", lastError.c_str());
        return false;
    }
    
    // Esperar un momento para que la cuenta se propague
    delay(2000);
    
    // Invalidar caché y refrescar
    refreshCache();
    
    // Verificar que la cuenta ahora existe
    if (cachedInfo.exists) {
        StellarUtils::infoPrint("Account", "Account funded successfully");
        return true;
    } else {
        lastError = "Account funded but not yet visible";
        StellarUtils::errorPrint("Account", lastError.c_str());
        return false;
    }
}

// ============================================
// CACHÉ
// ============================================

void StellarAccount::refreshCache() {
    cacheTimestamp = 0;  // Invalidar caché
    updateCache();
}

bool StellarAccount::isCacheValid() const {
    if (cacheTimestamp == 0) {
        return false;
    }
    
    uint32_t now = millis();
    uint32_t age = now - cacheTimestamp;
    
    // Manejar overflow de millis() (cada ~49 días)
    if (now < cacheTimestamp) {
        return false;
    }
    
    return (age < CACHE_LIFETIME_MS);
}

bool StellarAccount::updateCache() {
    StellarUtils::debugPrint("Account", "Updating cache from Horizon");
    
    String publicKey = keypair->getPublicKey();
    String response = network->getAccount(publicKey.c_str());
    
    if (response.length() == 0) {
        // Cuenta no existe o error de red
        String networkError = network->getLastError();
        
        if (networkError.indexOf("404") >= 0 || 
            networkError.indexOf("not found") >= 0) {
            // Cuenta no existe (no es error)
            lastError = "";
            cachedInfo.exists = false;
            cachedInfo.nativeBalance = 0.0f;
            cachedInfo.sequence = "0";
            cachedInfo.subentryCount = 0;
            cacheTimestamp = millis();
            
            StellarUtils::debugPrint("Account", "Account does not exist yet");
            return true;
        } else {
            // Error de red real
            lastError = "Network error: " + networkError;
            StellarUtils::errorPrint("Account", lastError.c_str());
            return false;
        }
    }
    
    // Parsear respuesta
    if (!parseAccountData(response, cachedInfo)) {
        lastError = "Failed to parse account data";
        StellarUtils::errorPrint("Account", lastError.c_str());
        return false;
    }
    
    cachedInfo.exists = true;
    cacheTimestamp = millis();
    lastError = "";
    
    StellarUtils::debugPrint("Account", "Cache updated successfully");
    return true;
}

bool StellarAccount::parseAccountData(const String& json, AccountInfo& info) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        StellarUtils::errorPrint("Account", "JSON parse error");
        return false;
    }
    
    // Extraer campos
    if (!doc.containsKey("id") || !doc.containsKey("sequence")) {
        StellarUtils::errorPrint("Account", "Missing required fields");
        return false;
    }
    
    info.accountId = doc["id"].as<String>();
    info.sequence = doc["sequence"].as<String>();
    info.subentryCount = doc["subentry_count"].as<uint32_t>();
    
    // Extraer balance nativo (XLM)
    info.nativeBalance = 0.0f;
    
    if (doc.containsKey("balances")) {
        JsonArray balances = doc["balances"];
        
        for (JsonObject balance : balances) {
            if (balance["asset_type"] == "native") {
                String balanceStr = balance["balance"].as<String>();
                info.nativeBalance = balanceStr.toFloat();
                break;
            }
        }
    }
    
    return true;
}