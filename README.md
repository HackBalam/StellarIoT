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

## Getting Started

Choose your setup method:

| Option | Requirements | Best for |
|--------|--------------|----------|
| **Option A: Simulator** | Just a computer | Testing, learning, no hardware |
| **Option B: Physical ESP32** | ESP32 + USB cable | Real IoT deployment |

---

## Step 1: Install PlatformIO (Required for both options)

### Install via VS Code (Recommended)
1. Open VS Code
2. Go to Extensions (`Ctrl+Shift+X`)
3. Search for "PlatformIO IDE"
4. Click Install

### Or install via command line
```bash
pip install platformio
```

### Troubleshooting: `pio` command not recognized

If after installing you get `'pio' is not recognized`, try one of these solutions:

**Solution 1:** Use Python module directly:
```bash
python -m platformio run
```

**Solution 2:** Add to PATH manually:

Windows (PowerShell as Admin):
```powershell
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";$env:USERPROFILE\.platformio\penv\Scripts", "User")
```

Linux/Mac:
```bash
export PATH="$HOME/.platformio/penv/bin:$PATH"
```

Then restart your terminal.

---

## Step 2: Clone the Repository

```bash
git clone https://github.com/your-repo/stellar-iot-sdk.git
cd stellar-iot-sdk
```

---

## Option A: Wokwi Simulator (No ESP32 needed)

Use this option if you don't have a physical ESP32.

### A1. Install Wokwi Extension
1. Open VS Code
2. Go to Extensions (`Ctrl+Shift+X`)
3. Search for "Wokwi Simulator"
4. Click Install

### A2. Get Wokwi License (Free)
1. Create an account at [wokwi.com](https://wokwi.com/)
2. In VS Code, press `F1`
3. Type "Wokwi: Request a new License"
4. Follow the instructions

### A3. Configure WiFi for Simulator
Make sure `src/main.cpp` has these settings (default):
```cpp
const char* ssid = "Wokwi-GUEST";
const char* password = "";
```

### A4. Build and Run
```bash
pio run
```

Then in VS Code:
1. Open `diagram.json`
2. Press the Play button (or `F1` > "Wokwi: Start Simulator")
3. The Serial Monitor will appear automatically

**Done! Skip to the [Commands](#commands) section.**

---

## Option B: Physical ESP32

Use this option if you have an ESP32 development board.

### B1. Hardware Required
- ESP32 DevKit (any variant)
- USB cable
- WiFi network (2.4GHz)

### B2. Configure WiFi
Edit `src/main.cpp` and set your WiFi credentials:
```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

### B3. Configure Serial Port
Edit `platformio.ini` and set your COM port:
```ini
# Windows (check Device Manager)
upload_port = COM3
monitor_port = COM3

# Linux
upload_port = /dev/ttyUSB0
monitor_port = /dev/ttyUSB0

# Mac
upload_port = /dev/cu.usbserial-0001
monitor_port = /dev/cu.usbserial-0001
```

### B4. Upload Firmware
```bash
# If upload fails, hold FLASH button while pressing RST
pio run -t upload

# Open serial monitor
pio device monitor
```

**Done! Continue to the [Commands](#commands) section.**

---

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
