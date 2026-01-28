# Stellar IoT SDK - MVP

A lightweight Stellar blockchain SDK for ESP32 IoT devices. Send payments, manage accounts, and interact with Stellar Network directly from microcontrollers.

## Features

- **Ed25519 Cryptography** - Sign transactions securely
- **Wallet Management** - Generate, import, and encrypt keypairs
- **Secure Storage** - AES-256-GCM encrypted wallet storage in flash
- **Network Integration** - Connect to Stellar Horizon (testnet/mainnet)
- **Payment Operations** - Send XLM payments with memo support
- **Account Management** - Check balance, fund accounts, view history
- **XDR Serialization** - Build and sign Stellar transactions

## Hardware Requirements

- ESP32 DevKit (any variant)
- USB cable
- WiFi connection

## No ESP32? Use Wokwi Simulator

If you don't have a physical ESP32, you can run the project using [Wokwi](https://wokwi.com/), a free online ESP32 simulator.

### Setup Wokwi

1. **Create a Wokwi account** at [wokwi.com](https://wokwi.com/)

2. **Install the Wokwi extension** in Visual Studio Code:
   - Open VS Code
   - Go to Extensions (Ctrl+Shift+X)
   - Search for "Wokwi Simulator"
   - Click Install

3. **Activate the free license**:
   - Press `F1` and type "Wokwi: Request a new License"
   - Follow the instructions to get your free license

4. **Configure WiFi for Wokwi** - Keep the default settings in `src/main.cpp`:
   ```cpp
   const char* ssid = "Wokwi-GUEST";
   const char* password = "";
   ```

5. **Run the simulation**:
   - Open the `diagram.json` file in VS Code
   - Press the Play button or use `F1` > "Wokwi: Start Simulator"
   - The Serial Monitor will appear automatically

## Installation

### 1. Install PlatformIO

```bash
# Install PlatformIO CLI
pip install platformio
```

Or use PlatformIO IDE extension for VS Code.

### 2. Clone Repository

```bash
git clone https://github.com/your-repo/stellar-iot-sdk.git
cd stellar-iot-sdk
```

### 3. Configure WiFi

Edit `src/main.cpp` and set your WiFi credentials:

```cpp
// Line ~35 in setup()
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

### 4. Configure Serial Port

Edit `platformio.ini` and set your COM port:

```ini
# For Windows (check Device Manager)
upload_port = COM3
monitor_port = COM3

# For Linux
upload_port = /dev/ttyUSB0
monitor_port = /dev/ttyUSB0

# For Mac
upload_port = /dev/cu.usbserial-0001
monitor_port = /dev/cu.usbserial-0001
```

### 5. Upload Firmware

```bash
# Hold FLASH button, press RST, release both
pio run -t upload

# Open serial monitor
pio device monitor
```

## Commands

### Wallet Commands

| Command | Description |
|---------|-------------|
| `wallet new` | Generate new wallet |
| `wallet save` | Save wallet to flash (encrypted) |
| `wallet load` | Load wallet from flash |
| `wallet show` | Display current wallet info |
| `wallet delete` | Delete saved wallet |
| `wallet import` | Import from secret key |

### Network Commands

| Command | Description |
|---------|-------------|
| `network test` | Test Horizon connection |
| `network fund` | Fund account with Friendbot (testnet only) |
| `network balance` | Check account balance |
| `network info` | Get account information |

### Payment Commands

| Command | Description |
|---------|-------------|
| `pay send` | Send XLM payment |
| `pay status` | Check last transaction status |
| `pay history` | View payment history (last 10) |

### Utility Commands

| Command | Description |
|---------|-------------|
| `help` | Show all commands |
| `memory` | Show memory statistics |
| `crypto` | Run crypto tests |
| `xdr` | Test XDR encoding |

## Quick Start

```
> wallet new
Wallet generated!
Public Key: GBRPYHIL2CI3FNQ...

> network fund
Account funded with 10,000 XLM (testnet)

> pay send
Enter destination: GAAZI4TCR3TY5OJHC...
Enter amount: 1.5
Enter memo: Test payment
Payment successful!
```

## Project Structure

```
stellar-iot-sdk/
├── src/
│   ├── main.cpp                - Main program with CLI
│   ├── stellar_utils.*         - Base utilities
│   ├── stellar_crypto.*        - Ed25519, SHA-256, AES-256
│   ├── stellar_keypair.*       - Key management
│   ├── stellar_storage.*       - Encrypted storage
│   ├── stellar_network.*       - Horizon API client
│   ├── stellar_xdr.*           - XDR serialization
│   ├── stellar_account.*       - Account management
│   └── stellar_payment.*       - Payment operations
├── platformio.ini              - PlatformIO configuration
└── README.md
```

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| ArduinoJson | ^6.21.3 | JSON parsing |
| Crypto | ^0.4.0 | Cryptographic operations |

## Memory Usage

| Resource | Usage |
|----------|-------|
| RAM | ~83KB (25% of 327KB) |
| Flash | ~770KB (59% of 1.3MB) |

## Network Configuration

By default, the SDK connects to Stellar Testnet. To switch networks, edit `src/main.cpp`:

```cpp
// Change from STELLAR_TESTNET to STELLAR_MAINNET
currentNetwork = new StellarNetwork(STELLAR_MAINNET);
```

## Security Notes

- Private keys are encrypted with AES-256-GCM before storage
- Passwords are hashed with PBKDF2 (10,000 iterations)
- Hardware RNG (ESP32) used for key generation
- Always use strong passwords (min 8 characters)

## Troubleshooting

### Upload fails

```bash
# Press and hold FLASH button
# Press and release RST button
# Release FLASH button
# Then run upload command
pio run -t upload
```

### Serial monitor shows garbage

Check baud rate is set to `115200` in `platformio.ini`:

```ini
monitor_speed = 115200
```

### WiFi connection fails

- Verify SSID and password are correct
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- Check router is within range

### Transaction fails

- Verify account is funded (use `network fund` on testnet)
- Check destination address is valid (starts with G)
- Ensure sufficient balance for amount + fee

## License

MIT License
