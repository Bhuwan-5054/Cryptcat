# Contributing to Cryptcat

Thank you for your interest in contributing to Cryptcat! This document provides guidelines and instructions for getting involved.

## Code of Conduct

Be respectful, inclusive, and constructive in all interactions. We value diverse perspectives and welcome everyone.

## Getting Started

### Prerequisites
- C11-compliant compiler (GCC, Clang, or MSVC)
- OpenSSL development libraries
- Git
- CMake or Make

### Setting Up Your Development Environment

1. **Fork and Clone**
   ```bash
   git clone https://github.com/YOUR_USERNAME/Cryptcat.git
   cd Cryptcat
   git remote add upstream https://github.com/Bhuwan-5054/Cryptcat.git
   ```

2. **Create a Feature Branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Install Dependencies**
   - **Linux (Ubuntu/Debian)**:
     ```bash
     sudo apt install build-essential libssl-dev cmake pkg-config
     ```
   - **macOS**:
     ```bash
     brew install openssl cmake
     ```
   - **Windows (MSYS2)**:
     ```bash
     pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-openssl cmake
     ```

4. **Build**
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

5. **Run Tests**
   ```bash
   cd tests
   make
   ./run_tests
   ```

## Making Changes

### Code Standards

We follow consistent coding standards:

- **Style**: 4-space indentation (no tabs), 80-character line limit
- **Naming**: `snake_case` for functions/variables, `UPPER_CASE` for constants
- **Comments**: Doxygen-style headers for public APIs, explain complex algorithms
- **Error Handling**: Use `goto cleanup` pattern with resource cleanup
- **Memory Safety**: Secure zeroing of sensitive data, bounds checking, no buffer overflows

### Example Pattern
```c
/**
 * Perform some operation.
 * 
 * @param input Input data
 * @return SUCCESS on success, error code on failure
 */
int function_name(const void *input) {
    int result = SUCCESS;
    
    /* Validate inputs */
    if (!input) {
        result = ERROR_INVALID_PARAM;
        goto cleanup;
    }
    
    /* Allocate resources */
    resource_t *res = allocate_resource();
    if (!res) {
        result = ERROR_MEMORY;
        goto cleanup;
    }
    
    /* Perform operation */
    result = do_work(res, input);
    if (result != SUCCESS) {
        goto cleanup;
    }
    
cleanup:
    if (res) free_resource(res);
    return result;
}
```

### Security Considerations

- **Never** log passwords, keys, or sensitive data
- **Always** validate external input (size, format, range)
- Use **constant-time comparisons** for cryptographic operations
- **Zero** memory before freeing sensitive data
- **Check** all return values, especially from crypto functions
- Use **locked memory** for keys when available

### Tests

When adding new functionality:

1. **Write unit tests** in `tests/unit/` for individual functions
2. **Add integration tests** in `tests/integration/` for workflows
3. **Include performance tests** in `tests/performance/` if performance-critical
4. Use test harness macros: `TEST_ASSERT_EQUAL()`, `TEST_ASSERT_NOT_NULL()`, etc.
5. Register test suite via `__attribute__((constructor))` and `test_register_suite()`
6. Ensure tests run without sanitizer warnings:
   ```bash
   cd tests
   CC=clang CFLAGS="-fsanitize=address,undefined" LDFLAGS="-fsanitize=address,undefined" make
   ./run_tests
   ```

Example test:
```c
TEST_CASE(test_my_feature) {
    int result = my_function("input");
    TEST_ASSERT_EQUAL(SUCCESS, result);
    return TEST_PASS;
}
```

## Submitting Changes

### Before You Commit

1. **Run tests** locally and ensure all pass:
   ```bash
   cd tests
   make clean
   make
   ./run_tests
   ```

2. **Run sanitizers** to catch memory errors:
   ```bash
   CC=clang CFLAGS="-fsanitize=address,undefined" LDFLAGS="-fsanitize=address,undefined" make
   ./run_tests
   ```

3. **Format your code** (indent, line length, consistency)

4. **Update documentation** if changing APIs or behavior

### Commit Message Format

Write clear, descriptive commit messages:

```
[module] Short description of change

Longer explanation if needed. Reference issue numbers like "Fixes #123".
Keep lines under 72 characters. Separate subject from body with blank line.
```

Examples:
- `[crypto] Add PBKDF2 key derivation tests`
- `[network] Fix memory leak in socket cleanup (Fixes #45)`
- `[docs] Update README with build instructions for WSL`

### Pull Request Process

1. **Push to your fork**:
   ```bash
   git push origin feature/your-feature-name
   ```

2. **Open a Pull Request** on GitHub with:
   - Clear title describing the change
   - Description of what changed and why
   - Reference to any related issues
   - Confirmation that tests pass and no warnings are present

3. **CI Checks**: Ensure GitHub Actions passes (builds on Linux/macOS/Windows, all tests pass, no sanitizer warnings)

4. **Code Review**: Maintainers will review your changes. Be responsive to feedback.

5. **Merge**: Once approved, your PR will be merged!

## Reporting Bugs

Found a bug? Please report it:

1. **Check existing issues** to avoid duplicates
2. **Create an issue** with:
   - Clear title and description
   - Steps to reproduce
   - Expected vs actual behavior
   - Environment (OS, compiler, OpenSSL version)
   - Relevant logs or output

Example:
```
Title: Encryption fails with non-ASCII passwords
Description:
When using a password containing non-ASCII UTF-8 characters,
crypto_session_create() returns NULL.

Steps to reproduce:
1. Create session with password "café"
2. Call crypto_encrypt()
3. Observe NULL pointer dereference

Environment:
- OS: Ubuntu 22.04
- Compiler: GCC 11.2
- OpenSSL 3.0
```

## Feature Requests

Have an idea? Open an issue with:
- Clear title and description
- Use case and motivation
- Proposed implementation (if applicable)
- Any relevant context or examples

## Documentation

Help improve docs! You can:
- Fix typos or unclear explanations
- Add examples or clarifications
- Update architecture documentation
- Add API documentation

See [docs/](docs/) for existing documentation.

## Development Tips

### Debugging

Enable debug logging:
```bash
make DEBUG=1
```

Use GDB:
```bash
gdb ./cryptcat
(gdb) run -k "password" -l -p 4444
(gdb) break crypto_encrypt
```

### Memory Checking

Use Valgrind:
```bash
valgrind --leak-check=full ./run_tests
```

### Code Analysis

Use static analysis tools:
```bash
clang --analyze src/core/*.c
cppcheck src/
```

## Project Structure

Key directories:
- `src/core/` — Main cryptographic and networking logic
- `src/platform/` — OS-specific implementations
- `src/utils/` — Utility functions (logging, error handling, etc.)
- `tests/` — Unit, integration, and performance tests
- `docs/` — Documentation and specifications
- `examples/` — Example usage code

## Questions?

- Check [CRYPTCAT_ARCHITECTURE.txt](CRYPTCAT_ARCHITECTURE.txt) for detailed design
- Read [README.md](README.md) for project overview
- Look at existing code for patterns and conventions
- Open an issue for discussions

## Licensing

By contributing, you agree that your contributions will be licensed under the MIT License.

---

**Thank you for contributing to Cryptcat!** 🔐
