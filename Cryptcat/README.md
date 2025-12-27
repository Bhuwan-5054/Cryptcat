# Cryptcat — Secure Netcat Replacement

[![Build Status](https://github.com/Bhuwan-5054/Cryptcat/actions/workflows/ci.yml/badge.svg)](https://github.com/Bhuwan-5054/Cryptcat/actions)
[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](CHANGELOG.md)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Code Coverage](https://img.shields.io/badge/coverage-70%25+-green.svg)](#testing)
[![C Standard](https://img.shields.io/badge/c%20standard-C11-lightgrey.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))

**Encrypted connections made simple.** Cryptcat is a cross-platform, open-source secure networking tool that replaces Netcat with end-to-end Twofish-256 encryption, HMAC authentication, and replay protection.

**[📖 Documentation](CRYPTCAT_ARCHITECTURE.txt) | [🛣️ Roadmap](ROADMAP.md) | [🔒 Security](SECURITY.md) | [🤝 Contributing](CONTRIBUTING.md)**

## Features

✅ **Twofish-256-CFB Encryption** — Fast, secure symmetric encryption  
✅ **HMAC-SHA256 Authentication** — Detect tampering and verify integrity  
✅ **PBKDF2 Key Derivation** — 200,000 iterations for password-based keys  
✅ **Replay Protection** — Sequence numbers prevent message replay attacks  
✅ **Cross-Platform** — Windows, Linux, and macOS support  
✅ **File Transfer** — Encrypted, chunked transfers with SHA256 verification  
✅ **Chat Mode** — Real-time encrypted messaging  
✅ **P2P Networking** — Decentralized peer discovery and mesh routing  
✅ **Command Execution** — Secure remote shell capability  
✅ **Comprehensive Test Suite** — Unit, integration, performance, and sanitizer tests  

## Quick Start

### Prerequisites

- **Compiler**: GCC/Clang (MSYS2/MinGW or WSL on Windows) or MSVC
- **OpenSSL**: Development libraries (headers + libs)
- **Build Tool**: CMake, Make, or PowerShell (Windows)

### Option 1: MSYS2 / MinGW (Windows)

```bash
# Install MSYS2: https://www.msys2.org/
# Open MSYS2 MinGW 64-bit shell and install:
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-openssl cmake

# Clone and build
git clone https://github.com/Bhuwan-5054/Cryptcat
cd Cryptcat
mkdir build && cd build
cmake ..
make

# Run
./cryptcat --help
```

### Option 2: WSL / Linux / macOS

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential libssl-dev cmake pkg-config

# macOS (Homebrew)
brew install openssl cmake

# Clone and build
git clone https://github.com/Bhuwan-5054/Cryptcat
cd Cryptcat
mkdir build && cd build
cmake ..
make

# Run
./cryptcat --help
```

### Option 3: Windows + MSVC

1. Open **x64 Native Tools Command Prompt for VS** (from Visual Studio)
2. Install OpenSSL for Windows and set `OPENSSL_DIR` environment variable
3. From `Cryptcat` root:
   ```powershell
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022"
   cmake --build . --config Release
   cryptcat.exe --help
   ```

### Option 4: Development Container (VS Code)

Fastest way to start developing:

1. Install Docker Desktop and VS Code "Dev Containers" extension
2. Open Cryptcat folder in VS Code
3. Click "Reopen in Container" when prompted
4. Full development environment loads automatically

See [.devcontainer/README.md](.devcontainer/README.md) for details.

## Usage

### Listen Mode (Server)
```bash
cryptcat -k "my_password" -l -p 4444
```

### Connect Mode (Client)
```bash
cryptcat -k "my_password" 192.168.1.100 4444
```

### Encrypted Chat
```bash
# Server
cryptcat -k "secret" -c -l -p 4444

# Client
cryptcat -k "secret" -c 192.168.1.100 4444
```

### File Transfer
```bash
# Send file
cryptcat -k "password" -f document.pdf 192.168.1.100 5555

# Receive on port 5555
cryptcat -k "password" -l -p 5555
```

## Project Structure

```
Cryptcat/
├── src/
│   ├── core/              # Core encryption & protocol (8 modules)
│   │   ├── crypto_engine.c    # Twofish-256-CFB + HMAC-SHA256
│   │   ├── network_layer.c    # Connection management
│   │   ├── protocol.c         # Message framing & handshake
│   │   ├── file_transfer.c    # Chunked file transfer
│   │   └── ...
│   ├── platform/          # OS-specific implementations
│   │   ├── unix_network.c     # POSIX sockets
│   │   ├── win_network.c      # WinSock2
│   │   └── os_utils.c         # Cross-platform utilities
│   ├── utils/             # Logging, error handling, crypto utils
│   │   ├── logger.c, error_handler.c, memory_utils.c, etc.
│   └── include/           # Header files (8 public APIs)
│
├── tests/                 # Comprehensive test suite
│   ├── unit/              # Unit tests for individual modules
│   │   └── test_crypto.c      # Cryptographic function tests
│   ├── integration/       # Client-server communication tests
│   │   └── test_end_to_end.c
│   ├── performance/       # Benchmarks
│   │   └── benchmark_crypto.c # Throughput & latency
│   ├── frameworks/        # Test harness infrastructure
│   │   ├── test_harness.h
│   │   ├── test_runner.c
│   │   └── test_main.c
│   ├── Makefile           # Build test suite (Unix/Linux)
│   ├── build_tests.ps1    # Build test suite (Windows)
│   └── README.md          # Test-specific build instructions
│
├── scripts/               # Build, development, and maintenance scripts
├── .github/workflows/     # GitHub Actions CI/CD
│   └── ci.yml             # Multi-platform builds + sanitizers + coverage
│
├── CMakeLists.txt         # CMake build configuration
├── Makefile               # Traditional Make support
├── CRYPTCAT_ARCHITECTURE.txt  # Detailed architecture & implementation guide
└── README.md              # This file
```

## Building

### CMake (Recommended)
```bash
mkdir build && cd build
cmake ..
make
```

### Traditional Make
```bash
make
```

## Testing

### Run All Tests (Unix/Linux/macOS)
```bash
cd tests
make
./run_tests
```

### Run All Tests (Windows + MSYS2)
```bash
cd tests
make
./run_tests
```

### Run All Tests (Windows + MSVC)
```powershell
cd tests
powershell -ExecutionPolicy Bypass -File .\build_tests.ps1
```

### Test Coverage
GitHub Actions automatically runs:
- **Unit Tests** — 50+ test cases for cryptographic functions
- **Integration Tests** — End-to-end client-server scenarios
- **Performance Benchmarks** — Throughput and latency measurement
- **Sanitizers** — Address Sanitizer + Undefined Behavior Sanitizer (ASAN/UBSAN)
- **Code Coverage** — lcov reports uploaded as artifacts

See [tests/README.md](tests/README.md) for detailed test build instructions.

## Architecture

The project follows a **layered architecture**:

```
┌─────────────────────────────────────┐
│       Application Layer             │  CLI, GUI, Config
├─────────────────────────────────────┤
│       Service Layer                 │  File transfer, Chat, P2P
├─────────────────────────────────────┤
│       Protocol Layer                │  Handshake, Message framing
├─────────────────────────────────────┤
│       Network Layer                 │  Cross-platform sockets
├─────────────────────────────────────┤
│       Cryptography Layer            │  Twofish-256, HMAC, PBKDF2
└─────────────────────────────────────┘
```

### Encryption Details

1. **Key Derivation**: Password → PBKDF2-HMAC-SHA256 (200,000 iterations) → 64 bytes
   - First 32 bytes: Encryption key
   - Next 32 bytes: HMAC key

2. **Per-Message Encryption**:
   - Random 16-byte IV
   - Twofish-256-CFB mode
   - Sequence number for replay protection
   - HMAC-SHA256 over (sequence + ciphertext)

3. **Wire Format**:
   ```
   [8-byte sequence] + [ciphertext] + [32-byte HMAC]
   ```

For full details, see [CRYPTCAT_ARCHITECTURE.txt](CRYPTCAT_ARCHITECTURE.txt).

## Security Considerations

✅ **Implemented**:
- Twofish-256-CFB encryption with random IVs
- HMAC-SHA256 for authentication
- Sequence numbers + timestamp validation
- Secure memory zeroing for sensitive data
- Input validation and bounds checking

🔲 **Future Enhancements**:
- Perfect Forward Secrecy (ECDH key exchange)
- Certificate-based authentication
- Hardware security module support
- Side-channel attack mitigations

## Development

### Code Standards
- C11 standard with platform-specific extensions
- 4-space indentation (no tabs)
- 80-character line limit
- snake_case for functions/variables, UPPER_CASE for constants
- Comprehensive error handling with `goto cleanup` pattern
- Secure memory practices (zeroing, locked memory for keys)

### Coding Example
```c
int function_name(params) {
    int result = SUCCESS;
    
    /* Validate inputs */
    if (!valid_input) {
        result = ERROR_INVALID_PARAM;
        goto cleanup;
    }
    
    /* Allocate resources */
    resource_t *res = allocate_resource();
    if (!res) {
        result = ERROR_MEMORY;
        goto cleanup;
    }
    
    /* Main logic */
    result = perform_operation(res);
    if (result != SUCCESS) {
        goto cleanup;
    }
    
cleanup:
    if (res) free_resource(res);
    return result;
}
```

### Adding Tests
1. Create test file in `tests/unit/` or `tests/integration/`
2. Use test harness macros: `TEST_CASE()`, `TEST_ASSERT_EQUAL()`, etc.
3. Register suite via `__attribute__((constructor))` and `test_register_suite()`
4. Add source to `tests/Makefile`
5. Run `make` in `tests/`

## Continuous Integration

GitHub Actions automatically:
- ✅ Builds on Ubuntu (gcc), macOS (clang), Windows (MSYS2 + MinGW)
- ✅ Runs unit, integration, and performance tests
- ✅ Checks for memory errors (ASAN/UBSAN)
- ✅ Generates code coverage reports
- ✅ Uploads artifacts for inspection

See `.github/workflows/ci.yml` for workflow details.

## Documentation

- **[CRYPTCAT_ARCHITECTURE.txt](CRYPTCAT_ARCHITECTURE.txt)** — Full architecture, technical specs, and implementation details
- **[tests/README.md](tests/README.md)** — Test suite build and run instructions
- **[docs/](docs/)** — API documentation, protocol specifications, and development guides

## Performance Targets

- Encryption speed: > 100 MB/s on modern hardware
- Connection setup: < 100ms
- File transfer overhead: < 5%
- Memory usage: < 50MB base + 1MB per connection
- CPU usage: < 10% during idle, < 50% during transfer

See `tests/performance/benchmark_crypto.c` for benchmark tests.

## Contributing

Contributions are welcome! Please:
1. Follow the code standards outlined above
2. Add tests for new features
3. Ensure CI passes (no sanitizer warnings)
4. Update documentation
5. Keep cryptographic implementations reviewed and conservative

## License

MIT License — See LICENSE file for details.

## Contact

- **Author**: Bhuwan-5054
- **Repository**: https://github.com/Bhuwan-5054/Cryptcat
- **Issues**: Report bugs and request features on GitHub Issues

## Roadmap

### ✅ Completed (v1.0)
- Core cryptographic engine (Twofish-256-CFB + HMAC-SHA256)
- Network abstraction layer (POSIX + WinSock2)
- Protocol implementation (handshake, message framing)
- File transfer system with integrity checking
- Cross-platform builds (Windows, Linux, macOS)
- Comprehensive test suite (unit, integration, performance)
- CI/CD pipeline (GitHub Actions)

### 🚧 In Progress (v1.1)
- GUI application (Electron + Vue.js)
- Fuzzing infrastructure for security testing
- Code coverage tracking and CI integration

### 🔲 Planned (v1.2+)
- Plugin system for extensibility
- Mobile applications (iOS/Android)
- Cloud synchronization
- Advanced monitoring and analytics
- Certificate-based authentication
- Perfect Forward Secrecy (ECDH)

---

**Cryptcat: Secure networking, simplified.**
