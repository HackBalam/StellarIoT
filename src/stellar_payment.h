#ifndef STELLAR_PAYMENT_H
#define STELLAR_PAYMENT_H

#include <Arduino.h>
#include "stellar_keypair.h"
#include "stellar_network.h"
#include "stellar_account.h"
#include "stellar_xdr.h"
#include "stellar_crypto.h"

/**
 * Operaciones de pago en Stellar
 * 
 * Maneja:
 * - Construcción de transacciones de pago
 * - Firma de transacciones
 * - Envío a la red
 * - Verificación de estado
 */

enum TransactionStatus {
    TX_PENDING,
    TX_SUCCESS,
    TX_FAILED,
    TX_UNKNOWN
};

struct PaymentResult {
    bool success;
    String transactionHash;
    TransactionStatus status;
    String error;
    uint32_t ledger;
};

class StellarPayment {
public:
    StellarPayment(
        StellarKeypair* keypair,
        StellarNetwork* network,
        StellarAccount* account
    );
    ~StellarPayment();
    
    // ============================================
    // ENVÍO DE PAGOS
    // ============================================
    
    /**
     * Envía un pago simple en XLM
     * 
     * @param destination Public key destino (G...)
     * @param amount Cantidad en XLM (float)
     * @param memo Memo de texto (opcional, máx 28 bytes)
     * @return Resultado del pago
     */
    PaymentResult sendPayment(
        const char* destination,
        float amount,
        const char* memo = nullptr
    );
    
    /**
     * Construye transacción de pago (sin enviar)
     * Útil para inspección o firma offline
     * 
     * @param destination Public key destino
     * @param amount Cantidad en XLM
     * @param memo Memo opcional
     * @param sequenceNumber Sequence number (0 = auto)
     * @return XDR de la transacción en base64
     */
    String buildPaymentTransaction(
        const char* destination,
        float amount,
        const char* memo = nullptr,
        uint64_t sequenceNumber = 0
    );
    
    // ============================================
    // ESTADO DE TRANSACCIONES
    // ============================================
    
    /**
     * Obtiene hash de la última transacción enviada
     * 
     * @return Hash de TX o string vacío
     */
    String getLastTransactionHash() const { return lastTxHash; }
    
    /**
     * Obtiene estado de la última transacción
     * 
     * @return Estado de la transacción
     */
    TransactionStatus getLastTransactionStatus();
    
    /**
     * Verifica estado de una transacción específica
     * 
     * @param txHash Hash de la transacción
     * @return Estado de la transacción
     */
    TransactionStatus getTransactionStatus(const char* txHash);
    
    /**
     * Obtiene último error
     * 
     * @return Mensaje de error o string vacío
     */
    String getLastError() const { return lastError; }
    
private:
    StellarKeypair* keypair;
    StellarNetwork* network;
    StellarAccount* account;
    String lastError;
    String lastTxHash;
    
    // Constantes
    static const uint32_t BASE_FEE = 100;  // 0.00001 XLM en stroops
    static const uint32_t MAX_MEMO_LENGTH = 28;
    
    // Helpers privados
    bool validatePaymentParams(
        const char* destination,
        float amount,
        const char* memo
    );
    
    String buildTransactionEnvelope(
        const uint8_t* sourcePublicKey,
        uint64_t sequenceNumber,
        const uint8_t* destinationPublicKey,
        int64_t amountStroops,
        const char* memo
    );
    
    String signTransaction(
        const uint8_t* transactionHash,
        const uint8_t signature[64]
    );
};

#endif // STELLAR_PAYMENT_H