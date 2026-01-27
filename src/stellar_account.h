#ifndef STELLAR_ACCOUNT_H
#define STELLAR_ACCOUNT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "stellar_keypair.h"
#include "stellar_network.h"

/**
 * Gestión de cuentas Stellar
 * 
 * Proporciona funciones para:
 * - Consultar información de cuenta
 * - Obtener balance
 * - Obtener sequence number
 * - Verificar existencia
 * - Fondear con Friendbot (testnet)
 */

struct AccountInfo {
    String accountId;
    String sequence;
    uint32_t subentryCount;
    float nativeBalance;
    bool exists;
    String lastError;
};

class StellarAccount {
public:
    StellarAccount(StellarKeypair* keypair, StellarNetwork* network);
    ~StellarAccount();
    
    // ============================================
    // INFORMACIÓN DE CUENTA
    // ============================================
    
    /**
     * Obtiene información completa de la cuenta
     * 
     * @return Estructura AccountInfo con todos los datos
     */
    AccountInfo getAccountInfo();
    
    /**
     * Obtiene solo el balance en XLM
     * 
     * @return Balance en XLM (float) o -1 si error
     */
    float getBalance();
    
    /**
     * Obtiene el sequence number actual
     * Necesario para construir transacciones
     * 
     * @return Sequence number (uint64) o 0 si error
     */
    uint64_t getSequenceNumber();
    
    /**
     * Verifica si la cuenta existe en la red
     * 
     * @return true si la cuenta está activa
     */
    bool isAccountActive();
    
    /**
     * Obtiene la clave pública de la cuenta
     * 
     * @return Public key (G...)
     */
    String getPublicKey() const;
    
    // ============================================
    // FONDEO (Solo Testnet)
    // ============================================
    
    /**
     * Fondea la cuenta usando Friendbot
     * Solo funciona en testnet
     * Crea la cuenta con 10,000 XLM
     * 
     * @return true si éxito
     */
    bool fundAccount();
    
    // ============================================
    // CACHÉ
    // ============================================
    
    /**
     * Fuerza actualización de caché
     * Útil después de enviar transacciones
     */
    void refreshCache();
    
    /**
     * Obtiene último error
     * 
     * @return Mensaje de error o string vacío
     */
    String getLastError() const { return lastError; }
    
private:
    StellarKeypair* keypair;
    StellarNetwork* network;
    String lastError;
    
    // Caché de información
    AccountInfo cachedInfo;
    uint32_t cacheTimestamp;
    static const uint32_t CACHE_LIFETIME_MS = 10000;  // 10 segundos
    
    // Helpers
    bool isCacheValid() const;
    bool updateCache();
    bool parseAccountData(const String& json, AccountInfo& info);
};

#endif // STELLAR_ACCOUNT_H