#ifndef STELLAR_WEBSERVER_H
#define STELLAR_WEBSERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "stellar_keypair.h"
#include "stellar_network.h"
#include "stellar_account.h"
#include "stellar_payment.h"
#include "stellar_storage.h"
#include "stellar_crypto.h"
#include "stellar_utils.h"

/**
 * HTTP Web Server for Stellar IoT SDK
 *
 * Serves a responsive single-page dashboard to control the ESP32
 * via browser. Exposes REST API endpoints for all wallet, network,
 * and payment commands.
 *
 * Usage:
 *   StellarWebServer webServer(&keypair, &network, &account, &payment);
 *   webServer.begin();      // call in setup()
 *   webServer.handle();     // call in loop()
 */
class StellarWebServer {
public:
    StellarWebServer(
        StellarKeypair** keypair,
        StellarNetwork** network,
        StellarAccount** account,
        StellarPayment** payment,
        int port = 80
    );

    void begin();
    void handle();

private:
    WebServer      _server;
    StellarKeypair** _keypair;
    StellarNetwork** _network;
    StellarAccount** _account;
    StellarPayment** _payment;

    void _setupRoutes();

    // Send a JSON response {success, message, data, error}
    void _sendJson(bool success,
                   const String& message = "",
                   const String& data    = "",
                   const String& error   = "");

    // Mirror of ensureManagers() / cleanupDependents() from main.cpp
    void _ensureManagers();
    void _cleanupDependents();

    // ── Route handlers ──────────────────────────────────────
    void _handleRoot();
    void _handleStatus();

    void _handleWalletNew();
    void _handleWalletSave();
    void _handleWalletLoad();
    void _handleWalletShow();
    void _handleWalletDelete();
    void _handleWalletImport();

    void _handleNetworkTest();
    void _handleNetworkFund();
    void _handleNetworkBalance();
    void _handleNetworkInfo();

    void _handlePaySend();
    void _handlePayStatus();
    void _handlePayHistory();

    void _handleCryptoTest();
    void _handleMemory();
};

#endif // STELLAR_WEBSERVER_H
