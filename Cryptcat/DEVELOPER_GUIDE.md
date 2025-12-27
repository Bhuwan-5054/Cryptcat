# Cryptcat Developer Guide

Welcome to Cryptcat development! This guide covers everything you need to know to contribute effectively.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Development Environment](#development-environment)
3. [Code Organization](#code-organization)
4. [Coding Standards](#coding-standards)
5. [Testing](#testing)
6. [Building & Deployment](#building--deployment)
7. [Security Considerations](#security-considerations)
8. [Common Tasks](#common-tasks)
9. [Troubleshooting](#troubleshooting)

---

## Getting Started

### Prerequisites

- **Git**: Version control (https://git-scm.com/)
- **Compiler**: GCC 11+ or Clang 13+
- **CMake**: 3.14+ (build system)
- **OpenSSL**: Development libraries
- **Make**: Build automation
- **Valgrind/AddressSanitizer**: Memory debugging (optional)

### Initial Setup

```bash
# Clone the repository
git clone https://github.com/Bhuwan-5054/Cryptcat.git
cd Cryptcat

# Set up pre-commit hooks
bash scripts/setup-hooks.sh

# Build the project
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run tests
cd ../tests
make && ./run_tests

# View help
../build/src/cryptcat --help
```

---

## Development Environment

### Option 1: Local Development (Recommended for experienced developers)

#### Linux/macOS
```bash
# Install dependencies
# Ubuntu/Debian
sudo apt update && sudo apt install build-essential libssl-dev cmake git

# macOS (Homebrew)
brew install gcc openssl cmake

# Clone and setup
git clone https://github.com/Bhuwan-5054/Cryptcat.git
cd Cryptcat
make build  # Uses root Makefile convenience target
```

#### Windows
```powershell
# Install MSYS2 from https://www.msys2.org/
# Open MSYS2 MinGW 64-bit and run:
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-openssl mingw-w64-x86_64-cmake

# Clone and build
git clone https://github.com/Bhuwan-5054/Cryptcat.git
cd Cryptcat
mkdir build && cd build
cmake ..
make
```

### Option 2: VS Code + Dev Container (Recommended for new developers)

1. Install [Docker Desktop](https://www.docker.com/products/docker-desktop)
2. Install VS Code with "Dev Containers" extension
3. Open Cryptcat folder in VS Code
4. Click "Reopen in Container"
5. Done! Full environment ready

See [.devcontainer/README.md](.devcontainer/README.md) for details.

### Option 3: GitHub Codespaces (Cloud-based, no local setup)

1. Fork repository on GitHub
2. Click "Code" → "Codespaces" → "Create codespace on main"
3. Wait ~2 minutes for container to build
4. Start developing in your browser

---

## Code Organization

### Directory Structure

```
src/
├── core/                    # Core encryption and networking
│   ├── crypto_engine.c      # Twofish-256-CFB, HMAC-SHA256, PBKDF2
│   ├── network_layer.c      # Connection management
│   ├── protocol.c           # Message framing, handshake
│   ├── file_transfer.c      # Encrypted file transfer
│   └── ...
│
├── platform/                # OS-specific implementations
│   ├── unix_network.c       # POSIX socket API
│   ├── win_network.c        # WinSock2 API
│   └── os_utils.c           # Cross-platform utilities
│
├── utils/                   # Utility modules
│   ├── logger.c/h           # Structured logging
│   ├── error_handler.c/h    # Error reporting
│   ├── memory_utils.c/h     # Secure memory handling
│   ├── config_parser.c/h    # INI config parsing
│   └── ...
│
└── include/                 # Public API headers
    ├── crypto.h             # Cryptography interface
    ├── network.h            # Network interface
    ├── protocol.h           # Protocol interface
    └── ...

tests/
├── unit/                    # Unit tests (single module)
├── integration/             # Integration tests (multi-module)
├── performance/             # Performance benchmarks
├── frameworks/              # Test infrastructure
├── Makefile                 # Unix build
├── build_tests.ps1          # Windows build
└── README.md                # Test documentation
```

### Key Files

- **src/include/crypto.h** — Public cryptography API
- **src/core/crypto_engine.c** — Encryption implementation
- **CRYPTCAT_ARCHITECTURE.txt** — Complete architecture documentation
- **tests/frameworks/test_harness.h** — Test macro definitions
- **CMakeLists.txt** — CMake configuration

---

## Coding Standards

### C Language Standards

- **Standard**: C11 (ISO/IEC 9899:2011)
- **Extensions**: Platform-specific as needed (POSIX, Windows API)

### Formatting

**Indentation & Spacing**:
- Use 4 spaces for indentation (no tabs in source files)
- Maximum line length: 100 characters
- One space around operators: `a = b + c;`
- No space before function call parentheses: `func(a, b);`

**Naming Conventions**:

| Entity | Style | Example |
|--------|-------|---------|
| Functions | snake_case | `crypto_encrypt_data()` |
| Variables | snake_case | `message_length` |
| Constants | UPPER_SNAKE_CASE | `MAX_BUFFER_SIZE` |
| Macros | UPPER_SNAKE_CASE | `CRYPTO_KEY_SIZE` |
| Types | PascalCase with _t suffix | `crypto_session_t` |
| Structs | PascalCase with _s suffix | `crypto_config_s` |

**File Headers**:

```c
/**
 * module_name.c
 * 
 * Brief description of what this module does.
 * 
 * Author: Your Name <your.email@example.com>
 * Date: YYYY-MM-DD
 * 
 * This module handles [detailed description].
 * 
 * Dependencies: other_module.h
 */

#include <stdio.h>
#include "crypto.h"
```

**Function Documentation**:

```c
/**
 * Encrypts data using the session's cipher.
 * 
 * Args:
 *   session: Active cryptographic session
 *   plaintext: Data to encrypt (NOT NULL)
 *   plaintext_len: Length of plaintext in bytes
 *   ciphertext: Buffer for encrypted data (must be >= plaintext_len + 32)
 *   ciphertext_len: Pointer to receive actual ciphertext length
 * 
 * Returns:
 *   0 on success
 *   -1 on error (call crypto_get_error() for details)
 *   -ENOMEM if buffer is too small
 * 
 * Notes:
 *   - IV is randomly generated per message
 *   - HMAC is appended to ciphertext
 *   - Session must be initialized with crypto_session_create()
 * 
 * Security:
 *   - Constant-time comparison for HMAC verification
 *   - No plaintext remains in memory after encryption
 */
int crypto_encrypt_data(crypto_session_t *session,
                        const uint8_t *plaintext,
                        size_t plaintext_len,
                        uint8_t *ciphertext,
                        size_t *ciphertext_len);
```

### Error Handling

Use the "goto cleanup" pattern:

```c
int function_name(void) {
    int result = 0;
    uint8_t *buffer = NULL;
    
    // Validate inputs
    if (input == NULL) {
        result = -EINVAL;
        goto cleanup;
    }
    
    // Allocate resources
    buffer = malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        result = -ENOMEM;
        goto cleanup;
    }
    
    // Perform operation
    if (operation_failed()) {
        result = -EIO;
        goto cleanup;
    }
    
    result = 0;
    
cleanup:
    if (buffer != NULL) {
        memset(buffer, 0, BUFFER_SIZE);
        free(buffer);
    }
    return result;
}
```

### Security Practices

1. **Secure Memory Handling**:
   ```c
   // Always zero sensitive data before freeing
   memset(key, 0, key_size);
   free(key);
   ```

2. **Input Validation**:
   ```c
   if (data == NULL || data_len == 0 || data_len > MAX_SIZE) {
       return -EINVAL;
   }
   ```

3. **Bounds Checking**:
   ```c
   // Always verify buffer sizes
   if (output_size < required_size) {
       return -EOVERFLOW;
   }
   ```

4. **Avoid Common Pitfalls**:
   - ❌ Don't use `strcpy()` — use `strncpy()` or `snprintf()`
   - ❌ Don't ignore return values — always check
   - ❌ Don't cast malloc — it's unnecessary and hides errors
   - ✅ Do check all allocations for NULL
   - ✅ Do use `sizeof` on variables, not types

---

## Testing

### Running Tests

```bash
# Build and run all tests
cd tests
make && ./run_tests

# Run specific test suite
./run_tests test_crypto

# Run with verbose output
./run_tests -v

# Run with coverage
cd ..
make coverage
```

### Writing New Tests

Create test file: `tests/unit/test_mymodule.c`

```c
#include <stdio.h>
#include "../frameworks/test_harness.h"
#include "../../src/include/mymodule.h"

// Test fixture (optional)
static void setup(void) {
    // Called before each test
}

static void teardown(void) {
    // Called after each test
}

// Test case
static void test_myfunction_returns_success(void) {
    int result = myfunction_good_input();
    TEST_ASSERT_EQUAL(0, result);
}

static void test_myfunction_returns_error_on_null(void) {
    int result = myfunction(NULL);
    TEST_ASSERT_NOT_EQUAL(0, result);
}

// Register tests (required)
static void __attribute__((constructor)) register_tests(void) {
    test_suite_t *suite = test_suite_create("MyModule", setup, teardown);
    test_register(suite, test_myfunction_returns_success);
    test_register(suite, test_myfunction_returns_error_on_null);
}
```

### Available Test Macros

```c
// Assertions
TEST_ASSERT_EQUAL(expected, actual)          // Compare integers
TEST_ASSERT_NOT_EQUAL(unexpected, actual)    // Compare integers (not equal)
TEST_ASSERT_NULL(ptr)                        // Check pointer is NULL
TEST_ASSERT_NOT_NULL(ptr)                    // Check pointer is not NULL
TEST_ASSERT_TRUE(condition)                  // Check boolean
TEST_ASSERT_FALSE(condition)                 // Check boolean
TEST_ASSERT_MEMORY_EQUAL(expected, actual, size)  // Compare byte buffers

// Test control
TEST_SKIP(message)                           // Skip test with message
```

---

## Building & Deployment

### Development Build

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug \
       -DENABLE_SANITIZERS=ON \
       -DENABLE_COVERAGE=ON ..
make
```

### Release Build

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
       -DENABLE_LTO=ON \
       -DENABLE_STRIP=ON ..
make
cmake --install . --prefix /usr/local
```

### Docker Build

```bash
docker build -t cryptcat:latest .
docker run -it cryptcat:latest /bin/bash
```

### CI/CD Pipeline

GitHub Actions runs automatically on:
- **Push to main** — Build, test, coverage
- **Pull requests** — Build, test, sanitizers
- **Tags (v*)** — Create release with artifacts

See `.github/workflows/` for configuration.

---

## Security Considerations

### When Implementing Cryptography

1. **Use established libraries** — Don't implement crypto yourself
2. **Use secure random** — OpenSSL `RAND_bytes()`, not `rand()`
3. **Constant-time comparisons** — Use `CRYPTO_memcmp()` for HMACs
4. **Key material** — Always zero before freeing
5. **Side-channel resistance** — Avoid data-dependent branches in crypto code

### Before Committing Code

1. Run tests: `make test`
2. Check sanitizers: `make test-asan`
3. Verify no hardcoded secrets
4. Review error handling
5. Check memory for leaks: `valgrind ./build/tests/run_tests`

### Security Review Checklist

- [ ] All user input validated
- [ ] Buffer sizes checked before access
- [ ] Error codes checked and handled
- [ ] Sensitive data zeroed before free
- [ ] No format string vulnerabilities
- [ ] No integer overflows
- [ ] No race conditions
- [ ] Cryptographic operations verified

---

## Common Tasks

### Adding a New Feature

1. Create issue on GitHub describing feature
2. Create branch: `git checkout -b feature/my-feature`
3. Implement in `src/` with tests
4. Add tests to `tests/unit/`
5. Update documentation
6. Run full test suite: `make test`
7. Submit pull request with description and closes #issue

### Fixing a Bug

1. Create issue on GitHub describing bug (if not exists)
2. Create branch: `git checkout -b fix/bug-description`
3. Write failing test first: `tests/unit/test_module.c`
4. Fix the bug in `src/`
5. Run tests to verify fix: `make test`
6. Update CHANGELOG.md
7. Submit PR

### Adding Documentation

1. Create/update markdown file in `docs/`
2. Link from main README.md or relevant docs
3. Keep images in `docs/images/`
4. Use consistent formatting
5. Run spell check before submitting

### Optimizing Performance

1. Establish baseline: `make test-bench`
2. Profile code: `valgrind --tool=callgrind`
3. Identify hotspots
4. Optimize with measurements
5. Verify no regressions: `make test-bench`
6. Document changes

---

## Troubleshooting

### Build Issues

**CMake not found**:
```bash
# Ubuntu/Debian
sudo apt install cmake

# macOS
brew install cmake

# Windows MSYS2
pacman -S mingw-w64-x86_64-cmake
```

**OpenSSL development headers not found**:
```bash
# Ubuntu/Debian
sudo apt install libssl-dev

# macOS
brew install openssl

# Windows MSYS2
pacman -S mingw-w64-x86_64-openssl
```

**Compiler errors with C11**:
```bash
# Ensure C11 flag is set
gcc -std=c11 -Wall -Wextra file.c

# Or use CMake
cmake -DCMAKE_C_STANDARD=11 ..
```

### Test Issues

**Tests fail with undefined references**:
```bash
# Rebuild from clean state
rm -rf build tests/build
make clean
make test
```

**Valgrind reports false positives**:
```bash
# Use suppression file for OpenSSL
valgrind --suppressions=/usr/share/valgrind/default.supp ./program
```

### Runtime Issues

**"Permission denied" when running binary**:
```bash
# Make executable
chmod +x build/src/cryptcat

# Or reinstall
make install
```

**Memory leaks detected**:
```bash
# Run with detailed output
ASAN_OPTIONS=verbosity=2:leak_check_at_exit=full ./build/src/cryptcat
```

---

## Getting Help

- **Questions?** Open GitHub Discussion
- **Bug report?** Open GitHub Issue with template
- **Security issue?** Email security@cryptcat-project.dev
- **Documentation?** Improve CRYPTCAT_ARCHITECTURE.txt or SECURITY.md
- **Not sure?** Ask in GitHub Discussions before implementing

---

## Code Review Process

1. **Author** submits PR with description
2. **CI** runs tests, sanitizers, coverage check
3. **Reviewers** approve changes
4. **Maintainer** merges to main
5. **CI** builds release artifacts

**PR Checklist** (author responsibility):
- [ ] Tests added/updated
- [ ] All tests pass: `make test`
- [ ] No sanitizer errors: `make test-asan`
- [ ] Code formatted: `make format`
- [ ] Documentation updated
- [ ] CHANGELOG.md updated
- [ ] No hardcoded secrets
- [ ] Commit messages follow convention

**Commit Message Format**:
```
<type>(<scope>): <subject>

<body>

<footer>
```

Example:
```
fix(crypto): prevent timing attack in HMAC comparison

Replace simple byte-by-byte comparison with constant-time
comparison to prevent timing side-channel attacks.

Closes #123
```

---

**Questions?** Join our community at GitHub Discussions or Cryptcat Discord server.

**Happy coding! 🚀**
