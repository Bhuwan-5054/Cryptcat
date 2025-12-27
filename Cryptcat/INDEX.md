# Cryptcat Documentation Index

**Last Updated**: December 27, 2025  
**Project Status**: ✅ v1.0.0 Production Ready

---

## 📖 Start Here

**New to Cryptcat?** Start with these in order:

1. [README.md](README.md) — 5-minute overview
2. [Quick Start Guide](#quick-start-guide) — Get running in minutes
3. [SECURITY.md](SECURITY.md) — Understand security model
4. [ROADMAP.md](ROADMAP.md) — See what's planned

---

## Quick Start Guide

### For Impatient Users

```bash
# 1. Get the code
git clone https://github.com/Bhuwan-5054/Cryptcat
cd Cryptcat

# 2. Build
mkdir build && cd build && cmake .. && make

# 3. Run
./src/cryptcat --help
./src/cryptcat -k "password" -l -p 4444  # Server
./src/cryptcat -k "password" localhost 4444  # Client (another terminal)
```

### For Developers (Dev Container)

1. Open folder in VS Code
2. Click "Reopen in Container"
3. Done! (Full environment with all tools)

---

## 📚 Complete Documentation Map

### Core Documentation

| Document | Purpose | Audience |
|----------|---------|----------|
| [README.md](README.md) | Project overview, quick start, features | Everyone |
| [CRYPTCAT_ARCHITECTURE.txt](CRYPTCAT_ARCHITECTURE.txt) | Technical architecture, protocol details, design decisions | Developers, auditors |
| [SECURITY.md](SECURITY.md) | Threat model, crypto details, vulnerability reporting | Users, auditors, security researchers |
| [PROJECT_STATUS.md](PROJECT_STATUS.md) | v1.0.0 completion summary, statistics | Project managers, users |

### Developer Resources

| Document | Purpose | For |
|----------|---------|-----|
| [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) | Setup, coding standards, testing, troubleshooting | New developers |
| [CONTRIBUTING.md](CONTRIBUTING.md) | Contribution guidelines, code review process | Contributors |
| [ROADMAP.md](ROADMAP.md) | Planned features through v2.0 | Feature planning, contributors |
| [CHANGELOG.md](CHANGELOG.md) | Version history, release notes | Users, maintainers |
| [SESSION_SUMMARY.md](SESSION_SUMMARY.md) | What was added in this session | Project overview |
| [PERFORMANCE.md](PERFORMANCE.md) | Performance baselines, profiling guide | Performance engineers |
| [RELEASE_CHECKLIST.md](RELEASE_CHECKLIST.md) | Pre-release verification | Maintainers |

### Configuration & Setup

| Document | Purpose |
|----------|---------|
| [.editorconfig](.editorconfig) | IDE formatting rules (4-space indent, etc.) |
| [.devcontainer/README.md](.devcontainer/README.md) | Dev Container usage |
| [.devcontainer/devcontainer.json](.devcontainer/devcontainer.json) | VS Code/Codespaces config |
| [tests/README.md](tests/README.md) | Test build instructions |

### Infrastructure

| File | Purpose |
|------|---------|
| [CMakeLists.txt](CMakeLists.txt) | Primary build configuration |
| [Makefile](Makefile) | Convenience targets (root level) |
| [tests/Makefile](tests/Makefile) | Test build (Unix/Linux/macOS) |
| [tests/build_tests.ps1](tests/build_tests.ps1) | Test build (Windows) |
| [Dockerfile](Dockerfile) | Container image |
| [.dockerignore](.dockerignore) | Docker build excludes |
| [.gitignore](.gitignore) | Git excludes |
| [LICENSE](LICENSE) | MIT License |

### GitHub Workflows & Templates

| File | Purpose |
|------|---------|
| [.github/workflows/ci.yml](.github/workflows/ci.yml) | Continuous integration (build, test, sanitizers, coverage) |
| [.github/workflows/release.yml](.github/workflows/release.yml) | Automated release builds |
| [.github/ISSUE_TEMPLATE/bug_report.md](.github/ISSUE_TEMPLATE/bug_report.md) | Bug report template |
| [.github/ISSUE_TEMPLATE/feature_request.md](.github/ISSUE_TEMPLATE/feature_request.md) | Feature request template |
| [.github/ISSUE_TEMPLATE/security_report.md](.github/ISSUE_TEMPLATE/security_report.md) | Security vulnerability template |
| [.github/pull_request_template.md](.github/pull_request_template.md) | PR checklist |

---

## 📖 Documentation by Purpose

### "I want to understand the project"
→ [README.md](README.md)  
→ [CRYPTCAT_ARCHITECTURE.txt](CRYPTCAT_ARCHITECTURE.txt)  
→ [PROJECT_STATUS.md](PROJECT_STATUS.md)

### "I want to use Cryptcat securely"
→ [README.md](README.md) (Quick Start)  
→ [SECURITY.md](SECURITY.md) (Threat model, known limitations)  
→ [examples/](examples/) (Code samples)

### "I want to develop features"
→ [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) (Setup, standards)  
→ [CONTRIBUTING.md](CONTRIBUTING.md) (Process, PR guidelines)  
→ [ROADMAP.md](ROADMAP.md) (What to build next)

### "I want to run tests"
→ [tests/README.md](tests/README.md) (Test build instructions)  
→ [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md#testing) (Test writing guide)

### "I found a security issue"
→ [SECURITY.md](SECURITY.md) (Reporting process)  
→ [.github/ISSUE_TEMPLATE/security_report.md](.github/ISSUE_TEMPLATE/security_report.md) (Template)

### "I want to deploy with Docker"
→ [Dockerfile](Dockerfile)  
→ [.docker/README.md](.docker/README.md)  
→ [README.md](README.md#option-4-docker) (Docker quick start)

### "I want to publish on GitHub"
→ [GITHUB_SETUP.md](GITHUB_SETUP.md) (Repository setup, CI/CD config)

### "I want to understand performance"
→ [PERFORMANCE.md](PERFORMANCE.md) (Baselines, profiling, optimization)

### "I want to prepare a release"
→ [RELEASE_CHECKLIST.md](RELEASE_CHECKLIST.md) (Pre-release verification)  
→ [CHANGELOG.md](CHANGELOG.md) (Release notes template)

### "I need to set up my environment"
→ [.env.example](.env.example) (Environment variables)  
→ [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md#development-environment) (Setup guide)

---

## 🔧 Quick Reference: Make Targets

Available at root level (`make <target>`):

```
Build:
  make build              Build debug version
  make build-release      Build release version

Testing:
  make test              Run all tests
  make test-unit         Run unit tests only
  make test-asan         Run with AddressSanitizer
  make test-ubsan        Run with UndefinedBehaviorSanitizer
  make test-bench        Run performance benchmarks

Code Quality:
  make lint              Static analysis
  make format            Auto-format code
  make format-check      Check format (non-destructive)

Coverage:
  make coverage          Generate coverage report
  make coverage-view     Open in browser

Installation:
  make install           Install to /usr/local/bin
  make uninstall         Remove installation

Docker:
  make docker-build      Build container image
  make docker-run        Run container interactively

Development:
  make pre-commit-setup  Install git hooks
  make pre-commit-test   Test pre-commit hooks
  make dev-setup         Full dev environment setup

Cleanup:
  make clean             Remove build artifacts
  make distclean         Remove everything
```

---

## 📊 Project Statistics

| Metric | Count |
|--------|-------|
| Source code files | 17 |
| Public API headers | 8 |
| Test files | 5+ |
| Documentation files | 15+ |
| GitHub workflows | 2 |
| Test cases | 50+ |
| Lines of code | ~5,000 |
| Lines of documentation | ~5,000 |
| Supported platforms | 3 (Linux, macOS, Windows) |

---

## 🎯 Common Tasks

### I want to...

**...use Cryptcat to encrypt data**
```bash
# See: README.md and examples/
cryptcat -k "password" -l -p 4444
```

**...fix a bug**
1. Read: DEVELOPER_GUIDE.md#fixing-a-bug
2. Create branch: `git checkout -b fix/bug-name`
3. Write failing test
4. Fix bug
5. Run: `make test`
6. Submit PR

**...add a new feature**
1. Read: ROADMAP.md (does it exist?)
2. Open GitHub issue
3. Read: DEVELOPER_GUIDE.md#adding-a-new-feature
4. Implement with tests
5. Submit PR with references

**...report a security issue**
→ [SECURITY.md](SECURITY.md) (process)  
→ Use template: `.github/ISSUE_TEMPLATE/security_report.md`

**...write tests**
→ [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md#writing-new-tests)  
→ [tests/README.md](tests/README.md)

**...understand the protocol**
→ [CRYPTCAT_ARCHITECTURE.txt](CRYPTCAT_ARCHITECTURE.txt#protocol-specification)

**...optimize performance**
→ [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md#optimizing-performance)

---

## 📋 File Organization

```
Cryptcat/
├── README.md                          ← Start here!
├── PROJECT_STATUS.md                  ← v1.0.0 summary
├── SECURITY.md                        ← Threat model
├── CONTRIBUTING.md                    ← How to contribute
├── DEVELOPER_GUIDE.md                 ← Dev setup & standards
├── ROADMAP.md                         ← Feature roadmap
├── CHANGELOG.md                       ← Version history
├── CRYPTCAT_ARCHITECTURE.txt          ← Technical details
├── SESSION_SUMMARY.md                 ← What was added
├── PERFORMANCE.md                     ← Performance baselines
├── GITHUB_SETUP.md                    ← GitHub publication guide
├── RELEASE_CHECKLIST.md               ← Pre-release checklist
├── .env.example                       ← Environment variables
├── LICENSE                            ← MIT License
│
├── src/                               ← Source code
│   ├── core/                          ← Encryption & protocol
│   ├── platform/                      ← OS-specific code
│   ├── utils/                         ← Utilities
│   └── include/                       ← Public APIs
│
├── tests/                             ← Test suite
│   ├── unit/                          ← Unit tests
│   ├── integration/                   ← Integration tests
│   ├── performance/                   ← Benchmarks
│   ├── frameworks/                    ← Test infrastructure
│   ├── Makefile                       ← Unix build
│   ├── build_tests.ps1                ← Windows build
│   └── README.md                      ← Test guide
│
├── examples/                          ← Example code
│   ├── basic/                         ← Simple examples
│   ├── advanced/                      ← Complex examples
│   └── README.md                      ← Examples guide
│
├── .github/
│   ├── workflows/                     ← CI/CD (2 workflows)
│   └── ISSUE_TEMPLATE/                ← Issue templates (4)
│
├── .devcontainer/                     ← VS Code Dev Container
│   ├── devcontainer.json              ← Config
│   └── README.md                      ← Usage guide
│
├── Dockerfile                         ← Container image
├── .dockerignore                      ← Docker excludes
├── Makefile                           ← Root targets
├── CMakeLists.txt                     ← CMake config
├── .editorconfig                      ← Editor rules
├── .gitignore                         ← Git excludes
```

---

## 🔗 External References

**Cryptography Standards**:
- [Twofish Cipher](https://www.schneier.com/academic/twofish/)
- [HMAC-SHA256](https://tools.ietf.org/html/rfc4868)
- [PBKDF2](https://tools.ietf.org/html/rfc8018)
- [CFB Mode](https://csrc.nist.gov/publications/detail/sp/800-38a/final)

**Development Tools**:
- [CMake Documentation](https://cmake.org/documentation/)
- [OpenSSL API](https://www.openssl.org/docs/)
- [AddressSanitizer](https://github.com/google/sanitizers)
- [VS Code Dev Containers](https://containers.dev/)

**Standards & Best Practices**:
- [C11 Standard](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
- [OWASP C Coding Practices](https://cheatsheetseries.owasp.org/cheatsheets/C_Programming_Language_Cheat_Sheet.html)
- [SEI CERT C Coding Guidelines](https://wiki.sei.cmu.edu/confluence/display/c/SEI+CERT+C+Coding+Standard)

---

## 💬 Getting Help

| Question | Resource |
|----------|----------|
| How do I get started? | [README.md](README.md) + [Quick Start](#quick-start-guide) |
| How do I use Cryptcat? | [examples/README.md](examples/README.md) + [SECURITY.md](SECURITY.md) |
| How do I develop features? | [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) + [ROADMAP.md](ROADMAP.md) |
| How do I report bugs? | [GitHub Issues](https://github.com/Bhuwan-5054/Cryptcat/issues) + [bug_report.md](.github/ISSUE_TEMPLATE/bug_report.md) |
| How do I report security issues? | [SECURITY.md](SECURITY.md) + [security_report.md](.github/ISSUE_TEMPLATE/security_report.md) |
| How do I ask questions? | [GitHub Discussions](https://github.com/Bhuwan-5054/Cryptcat/discussions) |
| How do I contribute? | [CONTRIBUTING.md](CONTRIBUTING.md) + [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) |

---

## ✅ Verification Checklist

Before using Cryptcat in production:

- [ ] Read [SECURITY.md](SECURITY.md) and understand threat model
- [ ] Review [CRYPTCAT_ARCHITECTURE.txt](CRYPTCAT_ARCHITECTURE.txt)
- [ ] Run test suite: `make test`
- [ ] Check security with sanitizers: `make test-asan`
- [ ] Verify code coverage: `make coverage`
- [ ] Review examples in [examples/](examples/)
- [ ] Understand password requirements (PBKDF2 security)
- [ ] Test in your environment first
- [ ] Monitor logs for errors
- [ ] Keep OpenSSL updated

---

## 🚀 Next Steps

1. **Fork & Clone**: Get the code
   ```bash
   git clone https://github.com/Bhuwan-5054/Cryptcat
   ```

2. **Read & Understand**: Start with README.md

3. **Build & Test**: Run the build
   ```bash
   make build && make test
   ```

4. **Contribute**: See [CONTRIBUTING.md](CONTRIBUTING.md)

5. **Stay Updated**: Watch the [CHANGELOG.md](CHANGELOG.md)

---

## 📄 License

Cryptcat is MIT licensed. See [LICENSE](LICENSE) for details.

---

**Last Updated**: December 27, 2025  
**Project Status**: ✅ v1.0.0 Production Ready  
**Community**: GitHub Issues, Discussions, and Pull Requests welcome!

**Happy encrypting! 🔒**
