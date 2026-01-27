#ifndef STELLAR_NETWORK_H
#define STELLAR_NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/**
 * Cliente de red para Stellar Horizon API
 * 
 * Maneja:
 * - Conexión a testnet/mainnet
 * - Llamadas HTTP GET/POST a Horizon
 * - Reintentos automáticos
 * - Manejo de errores
 */

enum NetworkType {
    STELLAR_TESTNET = 0,
    STELLAR_MAINNET = 1
};

class StellarNetwork {
public:
    StellarNetwork(NetworkType type = STELLAR_TESTNET);
    ~StellarNetwork();
    
    // ============================================
    // CONFIGURACIÓN
    // ============================================
    
    /**
     * Cambia la red (testnet/mainnet)
     * 
     * @param type Tipo de red
     */
    void setNetwork(NetworkType type);
    
    /**
     * Configura URL personalizada de Horizon
     * 
     * @param url URL del servidor Horizon
     */
    void setHorizonURL(const char* url);
    
    /**
     * Configura timeout para requests HTTP
     * 
     * @param seconds Timeout en segundos (default: 30)
     */
    void setTimeout(uint32_t seconds);
    
    /**
     * Configura número máximo de reintentos
     * 
     * @param count Número de reintentos (default: 3)
     */
    void setMaxRetries(uint8_t count);
    
    // ============================================
    // GETTERS
    // ============================================
    
    NetworkType getNetworkType() const { return networkType; }
    const char* getHorizonURL() const { return horizonUrl.c_str(); }
    const char* getNetworkPassphrase() const { return networkPassphrase.c_str(); }
    uint32_t getTimeout() const { return timeout; }
    
    // ============================================
    // HTTP METHODS GENÉRICOS
    // ============================================
    
    /**
     * Realiza HTTP GET a Horizon
     * 
     * @param endpoint Endpoint (ej: "/accounts/GABC...")
     * @return Respuesta JSON o string vacío si error
     */
    String httpGet(const char* endpoint);
    
    /**
     * Realiza HTTP POST a Horizon
     * 
     * @param endpoint Endpoint
     * @param body Body del POST (form-encoded)
     * @return Respuesta JSON o string vacío si error
     */
    String httpPost(const char* endpoint, const char* body);
    
    // ============================================
    // HORIZON API ESPECÍFICOS
    // ============================================
    
    /**
     * Obtiene información de una cuenta
     * GET /accounts/{account_id}
     * 
     * @param accountId Public key de la cuenta (G...)
     * @return JSON con datos de la cuenta
     */
    String getAccount(const char* accountId);
    
    /**
     * Obtiene pagos de una cuenta
     * GET /accounts/{account_id}/payments
     * 
     * @param accountId Public key (G...)
     * @param cursor Cursor para paginación (opcional)
     * @param limit Número de resultados (default: 10)
     * @return JSON con lista de pagos
     */
    String getAccountPayments(
        const char* accountId,
        const char* cursor = nullptr,
        uint8_t limit = 10
    );
    
    /**
     * Envía transacción a la red
     * POST /transactions
     * 
     * @param txXdrBase64 Transacción en XDR codificada en base64
     * @return JSON con resultado de la transacción
     */
    String submitTransaction(const char* txXdrBase64);
    
    /**
     * Obtiene información de una transacción
     * GET /transactions/{hash}
     * 
     * @param txHash Hash de la transacción
     * @return JSON con datos de la transacción
     */
    String getTransaction(const char* txHash);
    
    /**
     * Fondea cuenta en testnet usando Friendbot
     * Solo disponible en testnet
     * 
     * @param accountId Public key (G...)
     * @return true si éxito
     */
    bool fundWithFriendbot(const char* accountId);
    
    // ============================================
    // ESTADO
    // ============================================
    
    /**
     * Verifica si hay conexión WiFi
     * 
     * @return true si conectado
     */
    bool isConnected() const;
    
    /**
     * Obtiene último error
     * 
     * @return Mensaje de error o string vacío
     */
    String getLastError() const { return lastError; }
    
private:
    NetworkType networkType;
    String horizonUrl;
    String networkPassphrase;
    uint32_t timeout;           // En milisegundos
    uint8_t maxRetries;
    String lastError;
    
    // URLs por defecto
    static const char* TESTNET_HORIZON;
    static const char* MAINNET_HORIZON;
    static const char* FRIENDBOT_URL;
    
    // Network passphrases (para firmar transacciones)
    static const char* TESTNET_PASSPHRASE;
    static const char* MAINNET_PASSPHRASE;
    
    // HTTP helpers con retry
    String httpGetWithRetry(const char* url);
    String httpPostWithRetry(const char* url, const char* body);
    
    // Parse error desde respuesta Horizon
    void parseError(const String& response);
};

#endif // STELLAR_NETWORK_H