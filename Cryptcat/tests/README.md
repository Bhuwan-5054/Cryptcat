# Cryptcat Tests — Build & Run

This document explains how to build and run the test suite for Cryptcat on Windows and Unix-like environments.

Prerequisites
- OpenSSL development libraries installed (headers + libs)
- A C compiler: GCC/Clang (MSYS2/MinGW or WSL) or MSVC (Visual Studio)

Options

1) MSYS2 / MinGW (recommended on Windows)
- Install MSYS2: https://www.msys2.org/
- Open MSYS2 MinGW 64-bit shell
- Update packages and install toolchain:

```bash
pacman -Syu
pacman -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-openssl
```

- Build from `tests/`:

```bash
cd /c/Path/To/Cryptcat/tests
make
./run_tests
```

2) WSL (Ubuntu or similar)
- Install WSL and a distribution (Ubuntu)
- Install build tools and OpenSSL:

```bash
sudo apt update
sudo apt install build-essential libssl-dev pkg-config cmake
```

- Build from repository inside WSL filesystem:

```bash
cd /mnt/h/Cryptcat/tests
make
./run_tests
```

3) Native Windows + MSVC (Visual Studio)
- Open "x64 Native Tools Command Prompt for VS" (Developer Command Prompt)
- Install OpenSSL (Win64) and set `OPENSSL_DIR` to the OpenSSL install root (contains `include/` and `lib/`)
- From `tests/` run the helper script to attempt build with `cl`:

```powershell
cd H:\Cryptcat\tests
powershell -ExecutionPolicy Bypass -File .\build_tests.ps1
```

Notes
- The `tests/build_tests.ps1` script detects `gcc`, `clang`, or `cl` and attempts to compile & run tests.
- If using MSYS2/MinGW, prefer running `make` inside the MinGW shell to get correct toolchain and path semantics.
- If OpenSSL headers/libs are in non-standard locations, set `OPENSSL_DIR` when using MSVC.

Troubleshooting
- "No supported C compiler found": install MSYS2/MinGW, LLVM/Clang, or run in WSL with `build-essential`.
- Link errors for OpenSSL: ensure dev package installed and library search paths include OpenSSL `lib`.

Running tests manually
- Build binary (Makefile): `make` in `tests/` creates `run_tests` (or `run_tests.exe` on Windows).
- Execute: `./run_tests` or `.



- If you want, I can: add CI config (GitHub Actions), add MSVC-specific docs, or automate OpenSSL detection further.Contactun_tests.exe` on PowerShell.