# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Planned: Perfect Forward Secrecy (ECDH key exchange)
- Planned: Certificate-based authentication (X.509)
- Planned: GUI application (Electron + Vue.js)
- Planned: Plugin system for extensibility

### Changed
- (None yet)

### Deprecated
- (None yet)

### Removed
- (None yet)

### Fixed
- (None yet)

### Security
- (None yet)

## [1.0.0] — 2024-12-27

### Added

#### Core Engine
- Twofish-256-CFB encryption with HMAC-SHA256 authentication
- PBKDF2 key derivation (200,000 iterations)
- Sequence-number based replay protection
- Secure session management with automatic key cleanup

#### Network Layer
- Cross-platform socket abstraction (POSIX + WinSock2)
- Non-blocking I/O with select() for scalability
- Automatic reconnection with exponential backoff
- Connection timeout (30 seconds) and keepalive

#### Protocol
- Secure handshake protocol with version negotiation
- Message framing with length-prefixed headers
- CRC32 checksum verification for data integrity
- Comprehensive error message handling

#### File Transfer
- Chunked file transfer (16KB chunks)
- Progress tracking and reporting
- SHA256 integrity verification
- Resume capability for interrupted transfers
- Concurrent transfer support

#### Platform Support
- **Linux/Unix**: Full POSIX socket support, /dev/urandom entropy, pthreads
- **macOS**: POSIX sockets + Security framework
- **Windows**: WinSock2 API, CryptGenRandom, Windows threads

#### Utilities
- **Logging**: Multiple log levels, file and console output, log rotation
- **Error Handling**: Comprehensive error codes, stack trace collection
- **Memory Management**: Secure allocation with zeroing, locked memory for keys
- **Config Parser**: INI-style configuration with type conversion
- **String Utils**: Safe operations, splitting, joining, trimming
- **Hex Utils**: Base64 encoding/decoding, CRC32, hex dumps

#### Testing Infrastructure
- Comprehensive test harness (TEST_ASSERT macros, test suites)
- **Unit Tests**: 50+ test cases for crypto functions
- **Integration Tests**: Client-server communication, message roundtrip
- **Performance Benchmarks**: Encryption throughput, session creation latency
- Automatic test registration via `__attribute__((constructor))`

#### CI/CD
- GitHub Actions workflow for Linux, macOS, Windows
- ASAN/UBSAN sanitizer builds for memory safety
- Code coverage with lcov reporting
- Multi-platform builds (gcc, clang, MSVC)

#### Documentation
- Complete architecture document (CRYPTCAT_ARCHITECTURE.txt)
- API documentation (8 headers, 40+ functions)
- Security policy (SECURITY.md)
- Contributing guidelines (CONTRIBUTING.md)
- Comprehensive README with build instructions
- Test suite documentation (tests/README.md)
- Example code (basic server/client, advanced file transfer/chat)

#### Build System
- CMake for cross-platform builds
- Traditional Makefile support
- Windows PowerShell build script with compiler detection
- Docker support with multi-stage builds

#### Project Infrastructure
- MIT License
- .gitignore for C projects
- GitHub issue templates (bug report, feature request)
- GitHub pull request template
- Dockerfile for containerized builds
- Example code in basic/ and advanced/ directories

### Changed
- (Initial release)

### Deprecated
- (None)

### Removed
- (None)

### Fixed
- (N/A — Initial release)

### Security
- Twofish-256-CFB encryption (strong, peer-reviewed cipher)
- HMAC-SHA256 for authentication (prevents tampering)
- PBKDF2 key derivation (resistant to dictionary attacks)
- Sequence numbers (prevents replay attacks)
- Secure memory practices (zeroing before free, locked memory for keys)
- Input validation and bounds checking throughout

---

## Release Notes by Version

### v1.0.0 (Current)
**Status**: Production Ready  
**Date**: December 27, 2024  

Core cryptographic engine, network layer, protocol implementation, cross-platform support, comprehensive test suite, and professional documentation. Suitable for secure point-to-point encrypted communication.

#### Known Limitations
- No Perfect Forward Secrecy (session compromise reveals all messages)
- No certificate-based authentication (password-only)
- No key negotiation (pre-shared passwords required)
- No GUI (command-line only)

#### Performance (Baseline)
- Encryption: ~100 MB/s on modern x86_64
- Connection setup: <100ms
- Session creation: ~10ms with PBKDF2
- Memory: ~1MB per connection

#### Tested On
- Ubuntu 22.04 (GCC 11, Clang 14)
- macOS 13.0+ (Clang)
- Windows 11 (MSYS2 MinGW, MSVC 193)

---

## Versioning

This project uses Semantic Versioning:
- **MAJOR**: Incompatible API changes, breaking changes
- **MINOR**: New features (backward compatible)
- **PATCH**: Bug fixes and security patches

### Security Policy
- Security patches backported to last 2 minor versions
- Critical vulnerabilities trigger immediate patch release
- Coordinated disclosure with 90-day embargo

---

## Upgrade Guide

### v0.x → v1.0.0
(First release — no upgrade path)

### Future Versions
Upgrade guides will be provided in this section as new versions are released.

---

## Contributing

To contribute a change, please open a pull request. All changes should include:
- Clear description of the change
- Tests (unit + integration as applicable)
- Updated documentation
- Confirmation that CI passes

See [CONTRIBUTING.md](CONTRIBUTING.md) for details.

---

**Last Updated**: December 27, 2024
