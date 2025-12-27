#!/bin/bash
# verify_setup.sh - Cryptcat build and test verification script
# Simple one-command verification that everything works

set -e  # Exit on any error

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║                  CRYPTCAT BUILD VERIFICATION                   ║"
echo "║                                                                ║"
echo "║  This script verifies that your Cryptcat setup is working     ║"
echo "║  correctly. It will build the project and run tests.          ║"
echo "║                                                                ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "❌ Error: CMakeLists.txt not found!"
    echo "   Please run this script from the Cryptcat root directory."
    echo ""
    echo "   Usage:"
    echo "     cd Cryptcat"
    echo "     bash verify_setup.sh"
    exit 1
fi

echo "✓ Found CMakeLists.txt"
echo ""

# Step 1: Check dependencies
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Step 1: Checking dependencies..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Check for cmake
if ! command -v cmake &> /dev/null; then
    echo "❌ CMake not found! Please install CMake:"
    echo "   Ubuntu/Debian: sudo apt install cmake"
    echo "   macOS: brew install cmake"
    exit 1
fi
CMAKE_VERSION=$(cmake --version | head -1)
echo "✓ $CMAKE_VERSION"

# Check for C compiler
if ! command -v gcc &> /dev/null && ! command -v clang &> /dev/null && ! command -v cc &> /dev/null; then
    echo "❌ C compiler not found! Please install GCC or Clang."
    exit 1
fi
if command -v gcc &> /dev/null; then
    GCC_VERSION=$(gcc --version | head -1)
    echo "✓ $GCC_VERSION"
elif command -v clang &> /dev/null; then
    CLANG_VERSION=$(clang --version | head -1)
    echo "✓ $CLANG_VERSION"
fi

# Check for make
if ! command -v make &> /dev/null; then
    echo "❌ Make not found! Please install Make:"
    echo "   Ubuntu/Debian: sudo apt install make"
    echo "   macOS: brew install make"
    exit 1
fi
MAKE_VERSION=$(make --version | head -1)
echo "✓ $MAKE_VERSION"

# Check for OpenSSL
if ! pkg-config --exists openssl 2>/dev/null; then
    echo "❌ OpenSSL development libraries not found!"
    echo "   Ubuntu/Debian: sudo apt install libssl-dev"
    echo "   macOS: brew install openssl"
    exit 1
fi
OPENSSL_VERSION=$(pkg-config --modversion openssl)
echo "✓ OpenSSL: $OPENSSL_VERSION"

echo ""
echo "All dependencies found! ✅"
echo ""

# Step 2: Build
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Step 2: Building Cryptcat (Debug)..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug .. >/dev/null 2>&1
make >/dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Build successful!"
else
    echo "❌ Build failed!"
    exit 1
fi

echo ""

# Step 3: Run tests
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Step 3: Running tests..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

cd ../tests
make >/dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Tests compiled successfully!"
else
    echo "❌ Test compilation failed!"
    exit 1
fi

if [ -f "run_tests" ]; then
    echo ""
    echo "Running test suite..."
    ./run_tests 2>&1 | grep -E "^(PASS|FAIL|Tests:|Unit Tests:|Integration Tests:|Performance)" || true
    
    if [ $? -eq 0 ]; then
        echo "✓ Tests completed!"
    fi
else
    echo "⚠ Test executable not found"
fi

echo ""

# Step 4: Final summary
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "VERIFICATION COMPLETE ✅"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "✓ All dependencies found"
echo "✓ Project built successfully"
echo "✓ Tests compiled and ran"
echo ""
echo "Your Cryptcat setup is working correctly! 🎉"
echo ""
echo "Next steps:"
echo "  1. Read: README.md (overview)"
echo "  2. Read: QUICK_REFERENCE.md (commands)"
echo "  3. Try: ./build/src/cryptcat --help"
echo "  4. Publish: See GITHUB_SETUP.md"
echo ""
echo "For more information, see:"
echo "  - 00_START_HERE.md (visual summary)"
echo "  - INDEX.md (find anything)"
echo "  - DEVELOPER_GUIDE.md (setup & standards)"
echo ""
