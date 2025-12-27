# Build and run Cryptcat tests on Windows
# Usage: .\build_tests.ps1

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot

Write-Host "Project root: $projectRoot"

# Detect compiler
$gcc = Get-Command gcc -ErrorAction SilentlyContinue
$clang = Get-Command clang -ErrorAction SilentlyContinue
$cl = Get-Command cl -ErrorAction SilentlyContinue

# Choose platform source
$platform_src = Join-Path $projectRoot 'src\platform\win_network.c'

# Common include paths
$inc1 = "-I$projectRoot\src\include"
$inc2 = "-I$projectRoot\tests\frameworks"

# Source files
$sources = @(
    "'$projectRoot\\tests\\frameworks\\test_runner.c'",
    "'$projectRoot\\tests\\frameworks\\test_main.c'",
    "'$projectRoot\\tests\\unit\\test_crypto.c'",
    "'$projectRoot\\src\\core\\crypto_engine.c'",
    "'$platform_src'",
    "'$projectRoot\\src\\platform\\os_utils.c'",
    "'$projectRoot\\src\\utils\\logger.c'",
    "'$projectRoot\\src\\utils\\error_handler.c'",
    "'$projectRoot\\src\\utils\\memory_utils.c'",
    "'$projectRoot\\src\\utils\\string_utils.c'",
    "'$projectRoot\\src\\utils\\hex_utils.c'"
)

# Output binary
$out = "run_tests.exe"

if ($gcc) {
    Write-Host "Found GCC. Building with gcc..."
    $cflags = @('-Wall','-Wextra','-O2','-g','-std=c11')
    $ldflags = @('-lssl','-lcrypto','-lws2_32','-lcrypt32')

    $args = @()
    $args += $cflags
    $args += $inc1
    $args += $inc2
    $args += $sources
    $args += "-o"
    $args += $out
    $args += $ldflags

    # Run gcc
    $cmd = "gcc " + ($args -join ' ')
    Write-Host $cmd
    iex $cmd
    if ($LASTEXITCODE -ne 0) { throw "Compilation failed with exit code $LASTEXITCODE" }

    Write-Host "Build succeeded: $out"

    Write-Host "Running tests..."
    .\$out
    exit $LASTEXITCODE
}

elseif ($clang) {
    Write-Host "Found Clang. Building with clang..."
    # Use same flags as gcc
    $cflags = @('-Wall','-Wextra','-O2','-g','-std=c11')
    $ldflags = @('-lssl','-lcrypto','-lws2_32','-lcrypt32')

    $cmd = "clang " + ($cflags + $inc1 + $inc2 + $sources + @('-o',$out) + $ldflags) -join ' '
    Write-Host $cmd
    iex $cmd
    if ($LASTEXITCODE -ne 0) { throw "Compilation failed with exit code $LASTEXITCODE" }

    Write-Host "Build succeeded: $out"
    Write-Host "Running tests..."
    .\$out
    exit $LASTEXITCODE
}

elseif ($cl) {
    Write-Host "MSVC cl.exe detected. Attempting to build with MSVC (cl)."

    # Try to locate OpenSSL include/lib from OPENSSL_DIR env
    $openssl_include = $null
    $openssl_lib = $null
    if ($env:OPENSSL_DIR) {
        $openssl_include = Join-Path $env:OPENSSL_DIR 'include'
        $openssl_lib = Join-Path $env:OPENSSL_DIR 'lib'
    }

    $inc_args = @()
    $inc_args += ("/I" + "$projectRoot\\src\\include")
    $inc_args += ("/I" + "$projectRoot\\tests\\frameworks")
    if ($openssl_include) { $inc_args += ("/I" + $openssl_include) }

    # Prepare source list (unquoted entries OK for cl array)
    $srcs = @(
        "$projectRoot\\tests\\frameworks\\test_runner.c",
        "$projectRoot\\tests\\frameworks\\test_main.c",
        "$projectRoot\\tests\\unit\\test_crypto.c",
        "$projectRoot\\src\\core\\crypto_engine.c",
        $platform_src,
        "$projectRoot\\src\\platform\\os_utils.c",
        "$projectRoot\\src\\utils\\logger.c",
        "$projectRoot\\src\\utils\\error_handler.c",
        "$projectRoot\\src\\utils\\memory_utils.c",
        "$projectRoot\\src\\utils\\string_utils.c",
        "$projectRoot\\src\\utils\\hex_utils.c"
    )

    $cl_args = @('/nologo', '/W3', '/O2', '/MD') + $inc_args + $srcs + @('/Fe' + $out)

    # Linker libs
    $link_args = @('/link')
    if ($openssl_lib) { $link_args += ("/LIBPATH:" + $openssl_lib) }
    $link_args += @('libssl.lib','libcrypto.lib','ws2_32.lib','crypt32.lib')

    $full_args = $cl_args + $link_args

    Write-Host "cl.exe " + ($full_args -join ' ')
    & cl.exe @full_args
    if ($LASTEXITCODE -ne 0) { throw "MSVC compilation failed with exit code $LASTEXITCODE" }

    Write-Host "Build succeeded: $out"
    Write-Host "Running tests..."
    .\$out
    exit $LASTEXITCODE
}

else {
    Write-Host "No supported C compiler found (gcc/clang/cl)."
    Write-Host "Options: install MSYS2/MinGW (gcc), LLVM/Clang, or use WSL and run 'make' in tests/."
    exit 3
}
