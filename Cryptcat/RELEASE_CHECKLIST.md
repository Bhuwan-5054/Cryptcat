# Pre-Release Checklist

Complete this checklist before releasing a new version of Cryptcat.

---

## Version Information

**Release Version**: v___.___.___  
**Release Date**: YYYY-MM-DD  
**Previous Version**: v___.___.___  
**Release Type**: ☐ Major ☐ Minor ☐ Patch ☐ Release Candidate

---

## Code Quality

### Testing
- [ ] All unit tests pass: `make test`
- [ ] Integration tests pass: `make test`
- [ ] Performance benchmarks run: `make test-bench`
- [ ] Code coverage maintained: `make coverage`
- [ ] No new test failures
- [ ] New features have test coverage (>80%)

### Memory Safety
- [ ] AddressSanitizer tests pass: `make test-asan`
- [ ] UndefinedBehaviorSanitizer tests pass: `make test-ubsan`
- [ ] No memory leaks detected: `valgrind` analysis
- [ ] No use-after-free or buffer overflows
- [ ] No data races in concurrent code

### Code Quality
- [ ] All code formatted: `make format`
- [ ] No linting issues: `make lint`
- [ ] No compiler warnings (with `-Wall -Wextra`)
- [ ] Code review completed (GitHub PR)
- [ ] Security review completed
- [ ] Architecture review (if major changes)

### Documentation
- [ ] CHANGELOG.md updated with all changes
- [ ] API documentation complete
- [ ] Inline code comments updated
- [ ] README.md reflects new features
- [ ] Example code updated (if applicable)
- [ ] Known limitations documented

---

## Security

### Cryptography
- [ ] No new cryptographic algorithms without peer review
- [ ] Existing crypto algorithms unchanged (or bumped version)
- [ ] PBKDF2 iteration count appropriate (≥200,000)
- [ ] IV/nonce generation verified as random
- [ ] Key material properly zeroed after use
- [ ] No hardcoded secrets or test keys in production code

### Input Validation
- [ ] All user inputs validated
- [ ] Buffer bounds checked
- [ ] No format string vulnerabilities
- [ ] No SQL injection (N/A for pure C)
- [ ] No path traversal vulnerabilities
- [ ] Reject overly large inputs

### Error Handling
- [ ] All error paths tested
- [ ] No error information leakage (e.g., timing sidechannels)
- [ ] Proper error messages (no sensitive data)
- [ ] Error recovery verified
- [ ] Graceful shutdown implemented

### Security Audit
- [ ] Internal security review completed
- [ ] No OWASP Top 10 issues
- [ ] SEI CERT C guidelines followed
- [ ] CWE awareness (common weaknesses)
- [ ] No known exploitable patterns

---

## Build & Deployment

### Cross-Platform Builds
- [ ] Linux build passes: `./ci.sh linux`
- [ ] macOS build passes: `./ci.sh macos`
- [ ] Windows build passes: `./ci.sh windows`
- [ ] MinGW/MSYS2 build verified
- [ ] MSVC build verified
- [ ] No platform-specific regressions

### Build System
- [ ] CMake configuration clean
- [ ] No missing dependencies
- [ ] OpenSSL version compatibility verified
- [ ] Header file paths correct
- [ ] Linking flags appropriate
- [ ] Binary size reasonable (no bloat)

### Docker
- [ ] Dockerfile builds successfully: `docker build .`
- [ ] Docker image runs: `docker run -it cryptcat:latest /bin/bash`
- [ ] Multi-stage build optimized
- [ ] Image size acceptable (<100MB)
- [ ] Security: no root-required runtime

---

## Continuous Integration

### GitHub Actions
- [ ] CI workflow passes: `make build`
- [ ] Release workflow tested: `git tag test-v1.0`
- [ ] All matrix builds pass (Ubuntu, macOS, Windows)
- [ ] Sanitizer jobs pass
- [ ] Coverage job completes
- [ ] Artifact uploads work
- [ ] No secrets exposed in logs

### Artifacts
- [ ] Binary artifacts generated correctly
- [ ] Linux artifact (`cryptcat-linux-x64`)
- [ ] macOS artifact (`cryptcat-macos-x64`)
- [ ] Windows artifact (`cryptcat-windows-x64.exe`)
- [ ] All artifacts are executable
- [ ] Artifact sizes reasonable

---

## Documentation

### User Documentation
- [ ] README.md is current
- [ ] Quick start guide updated
- [ ] Installation instructions accurate
- [ ] Usage examples provided
- [ ] Troubleshooting guide current
- [ ] FAQ updated

### Developer Documentation
- [ ] DEVELOPER_GUIDE.md up-to-date
- [ ] API documentation complete
- [ ] Code examples included
- [ ] Architecture documented
- [ ] Contributing guidelines current
- [ ] Development environment setup documented

### Release Notes
- [ ] CHANGELOG.md entry created
- [ ] Features summarized
- [ ] Bug fixes listed
- [ ] Breaking changes documented
- [ ] Migration guide (if needed)
- [ ] Known issues documented
- [ ] Credits included

### Other Documentation
- [ ] SECURITY.md updated (if security changes)
- [ ] ROADMAP.md reflects this release
- [ ] VERSION file updated (if exists)
- [ ] Man pages updated (if exists)
- [ ] Wiki updated (if applicable)

---

## Version Management

### Version Bumping
- [ ] Version number follows Semantic Versioning (MAJOR.MINOR.PATCH)
- [ ] Version updated in:
  - [ ] CMakeLists.txt
  - [ ] CHANGELOG.md
  - [ ] README.md (version references)
  - [ ] package.json (if Node.js)
  - [ ] Any VERSION files

### Git Tags
- [ ] Tag created: `git tag -a v1.0.0 -m "Release: v1.0.0"`
- [ ] Tag is annotated (not lightweight)
- [ ] Tag message is descriptive
- [ ] Tag signature verified (if using GPG)

---

## Dependency Review

### External Dependencies
- [ ] All dependencies updated: `npm audit`, `brew upgrade`, etc.
- [ ] No deprecated dependencies
- [ ] License compatibility verified (all compatible with MIT)
- [ ] Security vulnerabilities addressed
- [ ] OpenSSL version appropriate (≥3.0)
- [ ] Dependency list documented (DEPENDENCIES.md)

### Sub-dependencies
- [ ] Transitive dependencies checked
- [ ] No known vulnerabilities in sub-deps
- [ ] Version pinning appropriate
- [ ] Lockfile updated (if using package manager)

---

## Release Preparation

### Pre-Release Testing
- [ ] Full test suite passes
- [ ] Integration tests pass
- [ ] Real-world scenario tested
- [ ] Upgrade path tested (if not v1.0)
- [ ] Downgrade not required (breaking changes noted)
- [ ] No critical bugs known

### Release Candidate (if applicable)
- [ ] RC version tagged
- [ ] RC announced to community
- [ ] Feedback period (≥1 week)
- [ ] Issues addressed
- [ ] Final release prepared

### Git Repository
- [ ] All commits pushed to main
- [ ] No uncommitted changes
- [ ] Branch status: main (clean, no pending PRs)
- [ ] History is clean (no accidental commits)

---

## Release Publication

### GitHub Release
- [ ] Release created on GitHub
- [ ] Tag pushed: `git push origin v1.0.0`
- [ ] Release title set
- [ ] Release description populated (from CHANGELOG)
- [ ] Pre-release flag set correctly
- [ ] Assets uploaded:
  - [ ] cryptcat-linux-x64
  - [ ] cryptcat-macos-x64
  - [ ] cryptcat-windows-x64.exe
  - [ ] Checksums (SHA256)

### GitHub Actions
- [ ] Release workflow triggered
- [ ] All builds completed
- [ ] Artifacts verified
- [ ] No workflow errors
- [ ] Build times acceptable

### Documentation
- [ ] GitHub Pages updated (if applicable)
- [ ] Wiki updated
- [ ] Project homepage updated
- [ ] Package repositories prepared

---

## Community & Announcement

### Communication
- [ ] Release notes reviewed
- [ ] Announcement drafted
- [ ] Breaking changes clearly documented
- [ ] Upgrade path explained
- [ ] Community notified:
  - [ ] GitHub Discussions
  - [ ] Email list (if applicable)
  - [ ] Twitter/social media
  - [ ] HackerNews, Reddit (if major release)

### Feedback Channels
- [ ] Issues enabled for bug reports
- [ ] Discussions enabled for questions
- [ ] Security contact available
- [ ] Support contact available

---

## Post-Release

### Monitoring
- [ ] Monitor issue tracker for bug reports
- [ ] Check CI/CD for any failures
- [ ] Monitor GitHub Discussions
- [ ] Track download statistics
- [ ] Monitor community feedback

### Hot-Fix Readiness
- [ ] If critical bugs found, hot-fix process ready
- [ ] Branch created for hot-fixes (if needed)
- [ ] Re-release procedure documented

### Analytics
- [ ] Download counts tracked
- [ ] User feedback collected
- [ ] Usage statistics gathered
- [ ] Performance reports reviewed

---

## Sign-Off

By signing below, you confirm this release is ready for publication.

**Prepared By**: _________________ **Date**: _________

**Reviewed By**: _________________ **Date**: _________

**Approved By**: _________________ **Date**: _________

---

## Next Release Preparation

**Next Version**: v___.___.___ **Planned Date**: YYYY-MM-DD

Key improvements for next release:
- [ ] Feature 1
- [ ] Feature 2
- [ ] Bug fix 1
- [ ] Performance improvement 1

---

## Notes

Additional notes, concerns, or observations:

```
[Your notes here]
```

---

## Checklist Usage

### For v1.0.0 Release
1. Copy this file to `release_v1.0.0.md`
2. Fill in version information
3. Go through each section
4. Check off items as you complete them
5. Get sign-offs
6. Keep for historical record
7. Archive in `releases/` directory

### For Future Releases
1. Copy `RELEASE_CHECKLIST.md`
2. Rename to `release_v1.x.x.md`
3. Update version numbers
4. Follow same process
5. Keep for audit trail

---

**Last Updated**: December 27, 2025  
**Version**: 1.0 of checklist  
**Maintained By**: Cryptcat Team

For questions, see [CONTRIBUTING.md](CONTRIBUTING.md) or [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md).
