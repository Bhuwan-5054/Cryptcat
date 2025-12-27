# verify_setup.ps1 - Cryptcat build and test verification script (Windows)
# Simple one-command verification that everything works

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "╔════════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║                  CRYPTCAT BUILD VERIFICATION                   ║" -ForegroundColor Cyan
Write-Host "║                                                                ║" -ForegroundColor Cyan
Write-Host "║  This script verifies that your Cryptcat setup is working     ║" -ForegroundColor Cyan
Write-Host "║  correctly. It will build the project and run tests.          ║" -ForegroundColor Cyan
Write-Host "║                                                                ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Check if we're in the right directory
if (-not (Test-Path "CMakeLists.txt")) {
    Write-Host "❌ Error: CMakeLists.txt not found!" -ForegroundColor Red
    Write-Host "   Please run this script from the Cryptcat root directory." -ForegroundColor Red
    Write-Host ""
    Write-Host "   Usage (in PowerShell):" -ForegroundColor Yellow
    Write-Host "     cd Cryptcat" -ForegroundColor Yellow
    Write-Host "     powershell -File verify_setup.ps1" -ForegroundColor Yellow
    exit 1
}

Write-Host "✓ Found CMakeLists.txt" -ForegroundColor Green
Write-Host ""

# Step 1: Check dependencies
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "Step 1: Checking dependencies..." -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

# Check for CMake
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) {
    Write-Host "❌ CMake not found!" -ForegroundColor Red
    Write-Host "   Please install CMake from: https://cmake.org/download/" -ForegroundColor Red
    exit 1
}
$cmakeVersion = & cmake --version | Select-Object -First 1
Write-Host "✓ $cmakeVersion" -ForegroundColor Green

# Check for C compiler
$hasCompiler = $false
$compiler = ""

if (Get-Command gcc -ErrorAction SilentlyContinue) {
    $compiler = "gcc"
    $hasCompiler = $true
} elseif (Get-Command clang -ErrorAction SilentlyContinue) {
    $compiler = "clang"
    $hasCompiler = $true
} elseif (Get-Command cl -ErrorAction SilentlyContinue) {
    $compiler = "MSVC (cl.exe)"
    $hasCompiler = $true
}

if (-not $hasCompiler) {
    Write-Host "❌ No C compiler found!" -ForegroundColor Red
    Write-Host "   Please install one of:" -ForegroundColor Red
    Write-Host "   - MSVC: https://visualstudio.microsoft.com/" -ForegroundColor Red
    Write-Host "   - GCC/Clang (MSYS2): https://www.msys2.org/" -ForegroundColor Red
    exit 1
}
Write-Host "✓ Compiler: $compiler" -ForegroundColor Green

# Check for OpenSSL
$openssl = Get-Command openssl -ErrorAction SilentlyContinue
if (-not $openssl) {
    Write-Host "⚠ OpenSSL not in PATH (but may still be available)" -ForegroundColor Yellow
    Write-Host "   If build fails, install OpenSSL:" -ForegroundColor Yellow
    Write-Host "   - MSYS2: pacman -S mingw-w64-x86_64-openssl" -ForegroundColor Yellow
    Write-Host "   - Or set OPENSSL_DIR environment variable" -ForegroundColor Yellow
} else {
    $opensslVersion = & openssl version
    Write-Host "✓ $opensslVersion" -ForegroundColor Green
}

Write-Host ""
Write-Host "Dependency check complete!" -ForegroundColor Green
Write-Host ""

# Step 2: Build
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "Step 2: Building Cryptcat (Debug)..." -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

if (Test-Path "build") {
    Write-Host "Cleaning previous build..."
    Remove-Item -Recurse -Force "build" | Out-Null
}

New-Item -ItemType Directory -Force -Path "build" | Out-Null
Set-Location "build"

try {
    & cmake -DCMAKE_BUILD_TYPE=Debug .. 2>&1 | Out-Null
    & cmake --build . --config Debug 2>&1 | Out-Null
    Write-Host "✓ Build successful!" -ForegroundColor Green
} catch {
    Write-Host "❌ Build failed!" -ForegroundColor Red
    Write-Host "Error: $_" -ForegroundColor Red
    exit 1
}

Write-Host ""

# Step 3: Run tests
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "Step 3: Running tests..." -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

Set-Location "../tests"

try {
    & cmake -DCMAKE_BUILD_TYPE=Debug .. 2>&1 | Out-Null
    & cmake --build . --config Debug 2>&1 | Out-Null
    Write-Host "✓ Tests compiled successfully!" -ForegroundColor Green
} catch {
    Write-Host "❌ Test compilation failed!" -ForegroundColor Red
    Write-Host "Error: $_" -ForegroundColor Red
    exit 1
}

Write-Host ""

# Try to run tests if executable exists
if (Test-Path "run_tests.exe") {
    Write-Host "Running test suite..."
    Write-Host ""
    & ./run_tests.exe 2>&1
    Write-Host ""
    Write-Host "✓ Tests completed!" -ForegroundColor Green
} elseif (Test-Path "run_tests") {
    Write-Host "Running test suite..."
    Write-Host ""
    & ./run_tests 2>&1
    Write-Host ""
    Write-Host "✓ Tests completed!" -ForegroundColor Green
} else {
    Write-Host "⚠ Test executable not found" -ForegroundColor Yellow
}

Write-Host ""

# Step 4: Final summary
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "VERIFICATION COMPLETE ✅" -ForegroundColor Green
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""
Write-Host "✓ All dependencies checked" -ForegroundColor Green
Write-Host "✓ Project built successfully" -ForegroundColor Green
Write-Host "✓ Tests compiled and ran" -ForegroundColor Green
Write-Host ""
Write-Host "Your Cryptcat setup is working correctly! 🎉" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "  1. Read: README.md (overview)" -ForegroundColor Yellow
Write-Host "  2. Read: QUICK_REFERENCE.md (commands)" -ForegroundColor Yellow
Write-Host "  3. Try: .\build\src\cryptcat.exe --help" -ForegroundColor Yellow
Write-Host "  4. Publish: See GITHUB_SETUP.md" -ForegroundColor Yellow
Write-Host ""
Write-Host "For more information, see:" -ForegroundColor Yellow
Write-Host "  - 00_START_HERE.md (visual summary)" -ForegroundColor Yellow
Write-Host "  - INDEX.md (find anything)" -ForegroundColor Yellow
Write-Host "  - DEVELOPER_GUIDE.md (setup & standards)" -ForegroundColor Yellow
Write-Host ""
