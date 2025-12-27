# SYSTEM SETUP GUIDE — How to Run Cryptcat

**Created**: December 27, 2025  
**Platform**: Windows  
**Status**: Installation Guide

---

## ⚠️ CURRENT STATUS

Your system is **MISSING KEY REQUIREMENTS**:

```
❌ CMAKE ............ NOT INSTALLED (Required for building)
❌ C COMPILER ....... NOT INSTALLED (Required for compilation)
❌ OPENSSL .......... NOT INSTALLED (Required for cryptography)
```

---

## 📋 WHAT YOU NEED

### For Windows, you need to install:

#### 1. **Visual Studio 2022 Build Tools** (C Compiler)
- Includes: MSVC compiler, CMake, build tools
- Size: ~4 GB
- Time: 15-30 minutes

#### 2. **CMake** (Build System)
- Version: 3.14 or later
- Size: ~100 MB
- Time: 5 minutes

#### 3. **OpenSSL** (Cryptography)
- Version: 3.0 or later  
- Size: ~200 MB
- Time: 5 minutes

---

## 🚀 STEP-BY-STEP INSTALLATION

### Option 1: Quick Installation (Recommended)

#### Step 1: Install Visual Studio Build Tools
```powershell
# Option A: Using Chocolatey (if you have it)
choco install visualstudio2022buildtools cmake openssl -y

# Option B: Manual installation (see below)
```

**Manual Download Links:**
1. **Visual Studio 2022 Build Tools**
   - Download: https://visualstudio.microsoft.com/downloads/
   - Version: "Build Tools for Visual Studio 2022"
   - During installation:
     ✅ Check "Desktop development with C++"
     ✅ Check "CMake tools for Windows"
   - Click "Install"

2. **CMake** (if not included above)
   - Download: https://cmake.org/download/
   - Choose: Windows x64 Installer
   - Run installer, choose "Add CMake to system PATH"

3. **OpenSSL**
   - Download: https://slproweb.com/products/Win32OpenSSL.html
   - Choose: "Win64 OpenSSL v3.x (Msi Installer)"
   - Run installer, accept defaults
   - When asked: Choose "Copy OpenSSL DLLs to The Windows system directory"

---

## ✅ VERIFICATION STEPS

After installation, verify everything:

### Step 1: Close and reopen PowerShell
```powershell
# Press: Win + X
# Click: Windows PowerShell (Admin)
# Or PowerShell 7+
```

### Step 2: Run verification
```powershell
cd h:\Cryptcat
powershell -File verify_setup.ps1
```

Expected output: ✅ All checks passing

---

## 📝 DETAILED INSTALLATION GUIDE

### For Visual Studio Build Tools (Most Important)

1. **Download**: Go to https://visualstudio.microsoft.com/downloads/
2. **Find**: "Build Tools for Visual Studio 2022" (scroll down)
3. **Click**: "Download"
4. **Run**: The installer
5. **Select Workloads**:
   - ✅ "Desktop development with C++"
   - ✅ "CMake tools for Windows"
6. **Installation Location**: Keep default (C:\Program Files...)
7. **Click**: "Install"
8. **Wait**: 15-30 minutes for installation
9. **Restart**: Computer (important!)

### For CMake (If not in Visual Studio)

1. **Download**: https://cmake.org/download/
2. **Choose**: "cmake-3.x.x-windows-x86_64.msi"
3. **Run**: Installer
4. **Select**: "Add CMake to the system PATH for current user"
5. **Click**: "Finish"

### For OpenSSL

1. **Download**: https://slproweb.com/products/Win32OpenSSL.html
2. **Choose**: "Win64 OpenSSL v3.x.x (Msi Installer)" (3.0+)
3. **Run**: Installer
4. **When Asked**: "Copy OpenSSL DLLs to The Windows system directory"
   - Select: "Yes"
5. **Click**: "Finish"

---

## 🍫 Alternative: Chocolatey (If You Have It)

If you have Chocolatey installed:

```powershell
# Run as Administrator
choco install visualstudio2022buildtools cmake openssl -y

# Wait for installation to complete
# Restart your computer
```

---

## 🔍 HOW TO VERIFY INSTALLATION

### Check each component:

```powershell
# Check CMake
cmake --version
# Expected: cmake version 3.14+

# Check C Compiler
cl
# Expected: Microsoft (R) C/C++ Optimizing Compiler

# Check OpenSSL
openssl version
# Expected: OpenSSL 3.0.0+
```

If all show versions → ✅ Ready to build!

---

## 🏗️ ONCE INSTALLED - HOW TO RUN CRYPTCAT

### Build the project:
```powershell
cd h:\Cryptcat
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Run tests:
```powershell
ctest --output-on-failure
```

### Use Cryptcat:
```powershell
# Server (listen on port 4444)
.\src\Release\cryptcat.exe -k "password" -l -p 4444

# Client (connect to server)
.\src\Release\cryptcat.exe -k "password" localhost 4444
```

---

## ❓ TROUBLESHOOTING

### Problem: "cmake: command not found"
**Solution**: 
1. Restart PowerShell (close and open new window)
2. Or manually add CMake to PATH:
   ```powershell
   $env:PATH += ";C:\Program Files\CMake\bin"
   ```

### Problem: "cl: command not found"
**Solution**:
1. Visual Studio Build Tools didn't install properly
2. Reinstall Visual Studio Build Tools
3. Make sure to check "Desktop development with C++"

### Problem: "OpenSSL not found"
**Solution**:
1. Rerun OpenSSL installer
2. Choose "Copy OpenSSL DLLs to Windows system directory"
3. Restart computer

### Problem: Build fails with "libcrypto not found"
**Solution**:
1. Add OpenSSL to PATH:
   ```powershell
   $env:PATH += ";C:\Program Files\OpenSSL-Win64\bin"
   ```

---

## 📊 INSTALLATION TIME ESTIMATE

| Component | Time | Notes |
|-----------|------|-------|
| Visual Studio Build Tools | 20-30 min | Largest download |
| CMake | 5 min | Quick install |
| OpenSSL | 5 min | Quick install |
| **TOTAL** | **30-40 min** | One-time only |

---

## 🎯 QUICK CHECKLIST

**Before you start:**
- [ ] Internet connection (good bandwidth)
- [ ] Administrator privileges on computer
- [ ] 10 GB free disk space
- [ ] 30-40 minutes of time

**Installation order:**
- [ ] 1. Visual Studio Build Tools (20-30 min)
- [ ] 2. CMake (5 min) - if not in VS
- [ ] 3. OpenSSL (5 min)
- [ ] 4. Restart computer
- [ ] 5. Verify with: `verify_setup.ps1`

**After verification:**
- [ ] Run: `cd h:\Cryptcat\build && cmake ..`
- [ ] Build: `cmake --build . --config Release`
- [ ] Test: `ctest --output-on-failure`
- [ ] Use: `src\Release\cryptcat.exe -k "password" -l -p 4444`

---

## 🌐 ALTERNATIVE: Docker

If you don't want to install all this, you can use **Docker**:

```powershell
# Build Docker image
docker build -t cryptcat .

# Run in Docker
docker run cryptcat -k "password" -l -p 4444
```

Requires: Docker Desktop installed (~2 GB)

---

## ✨ AFTER SETUP

Once installed, you can:
- ✅ Build Cryptcat: `cmake --build . --config Release`
- ✅ Run tests: `ctest --output-on-failure`
- ✅ Use encryption: `cryptcat.exe`
- ✅ View documentation: See PROJECT_REPORT folder

---

## 📞 NEED HELP?

See documentation:
- **Quick Start**: [README.md](../README.md)
- **Developer Guide**: [DEVELOPER_GUIDE.md](../DEVELOPER_GUIDE.md)
- **Troubleshooting**: [CRYPTCAT_ARCHITECTURE.txt](../CRYPTCAT_ARCHITECTURE.txt)
- **Technical Details**: [PROJECT_REPORT/01_TECHNICAL_SPECIFICATIONS.md](../PROJECT_REPORT/01_TECHNICAL_SPECIFICATIONS.md)

---

**Next Step**: Follow the installation steps above, then run `verify_setup.ps1` to confirm everything works!

---

*Cryptcat v1.0.0 — Easy to build, secure to use.*
