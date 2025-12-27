# Security Policy

## Overview

Cryptcat is designed with security as a core principle. This document explains our security model, known limitations, and vulnerability reporting process.

## Threat Model

### Protected Against
- **Passive Eavesdropping** — All network traffic is encrypted with Twofish-256-CFB
- **Message Tampering** — HMAC-SHA256 detects any modification
- **Weak Passwords** — PBKDF2 with 200,000 iterations strengthens passwords
- **Replay Attacks** — Sequence numbers prevent message replay
- **Timing Attacks** — Constant-time comparison for HMAC verification

### Assumptions
- Adversary cannot control the password (obtains it through other means)
- System's random number generator is cryptographically secure
- Underlying platform provides secure memory operations
- Compiler doesn't introduce vulnerabilities (e.g., buffer overflows)

### Known Limitations
- **No Forward Secrecy** — Compromise of a session key may reveal past messages (planned for v1.2)
- **No Authentication** — Parties are not authenticated beyond knowing the password; no protection against MITM if password is intercepted
- **No Key Exchange** — Session keys must be pre-shared or exchanged through secure channel
- **IV Reuse Risk** — Sessions use independent IVs but don't negotiate them; IV collision unlikely in practice
- **No Protection Against Physical Access** — Keys can be extracted from running process memory

## Security Considerations

### Cryptographic Parameters

**Key Derivation**
- Algorithm: PBKDF2-HMAC-SHA256
- Iterations: 200,000
- Salt: 32 bytes, randomly generated
- Output: 64 bytes (32 for encryption, 32 for HMAC)

**Encryption**
- Algorithm: Twofish-256-CFB
- Key Size: 256 bits (32 bytes)
- IV Size: 128 bits (16 bytes)
- IV: Generated randomly per session, transmitted in each message

**Authentication**
- Algorithm: HMAC-SHA256
- Key Size: 256 bits (32 bytes)
- Output: 256 bits (32 bytes)
- Computed over: Sequence number + ciphertext

**Wire Format**
```
[8-byte sequence number (big-endian)] + [ciphertext] + [32-byte HMAC]
```

### Memory Safety

All sensitive data is:
- **Zeroed** before freeing using `memset(ptr, 0, size)` or compiler builtins
- **Locked** in memory (where available) to prevent swap-out
- **Validated** with bounds checking before use
- **Isolated** in dedicated structures to prevent accidental leakage

### Input Validation

All external input is:
- **Size-checked** against buffer limits
- **Format-validated** before parsing
- **Type-checked** where applicable
- **Range-validated** for numeric values

### Known Issues & Mitigations

| Issue | Impact | Mitigation |
|-------|--------|-----------|
| Session keys are symmetric | Both parties share same key | Use per-direction keys in future versions; manage key access carefully |
| No certificate validation | MITM possible on first connection | Use pre-shared secrets or out-of-band authentication |
| Password entropy matters | Weak passwords are weaker | Document password requirements; advise minimum 16 characters |
| Codebase is C | Buffer overflow possible | Extensive testing, sanitizers in CI, code review |

## Vulnerability Reporting

### Responsible Disclosure

If you discover a security vulnerability:

1. **Do NOT** open a public GitHub issue
2. **Email** security@example.com (planned) with:
   - Vulnerability description
   - Affected version(s)
   - Steps to reproduce
   - Potential impact
   - Suggested fix (if available)
3. **Wait** for acknowledgment (response within 48 hours)
4. **Work with** maintainers to develop and test a fix
5. **Respect** embargo period (typically 90 days) before public disclosure

### Patch Process

1. Maintainer confirms vulnerability
2. Fix is developed and tested
3. Security patch release is prepared
4. Vulnerability is disclosed after patch release
5. Credit is given (unless you prefer anonymity)

## Security Best Practices for Users

### Password Management
- Use **strong, unique passwords** (16+ characters recommended)
- Include **uppercase, lowercase, numbers, and symbols**
- Avoid **dictionary words or predictable patterns**
- Never **reuse passwords** across different systems
- Store passwords **securely** (password manager)

### Operational Security
- **Keep software updated** — Apply security patches promptly
- **Verify integrity** — Check file hashes/signatures when available
- **Isolate secrets** — Don't expose passwords in logs or version control
- **Monitor usage** — Review logs for suspicious activity
- **Secure channels** — Exchange passwords through secure, out-of-band methods

### Network Security
- **Use VPN/TLS** for initial password exchange
- **Monitor connections** for unexpected traffic
- **Enable firewalls** on both client and server
- **Use strong authentication** at the OS/network layer

## Future Security Enhancements (v1.2+)

- [ ] Perfect Forward Secrecy (ECDH key exchange)
- [ ] Certificate-based authentication (X.509)
- [ ] Hardware security module (HSM) support
- [ ] Side-channel attack mitigations
- [ ] Per-message authentication codes (MAC)
- [ ] Rekeying on regular schedule
- [ ] Post-quantum cryptography (NewHope, Kyber)

## Compliance

This project aims to follow:
- **OWASP Top 10** — Secure coding practices
- **NIST Guidelines** — Cryptographic algorithm selection
- **CWE/SANS Top 25** — Common weakness avoidance
- **Best Practices** — Industry standards and patterns

## Testing & Validation

Security is validated through:
- **Fuzzing** — Random input testing (planned AFL integration)
- **Sanitizers** — Memory safety checks (ASAN/UBSAN in CI)
- **Static Analysis** — Clang analyzer, cppcheck
- **Code Review** — All changes reviewed before merge
- **Penetration Testing** — Regular security audits (planned)

## Security Audit History

| Date | Auditor | Result | Report |
|------|---------|--------|--------|
| (pending) | (TBD) | (TBD) | (TBD) |

## Resources

- [CRYPTCAT_ARCHITECTURE.txt](../CRYPTCAT_ARCHITECTURE.txt) — Detailed implementation
- [CONTRIBUTING.md](../CONTRIBUTING.md) — Development guidelines
- [OpenSSL Documentation](https://www.openssl.org/docs/) — Crypto library reference
- [OWASP Cryptographic Storage Cheat Sheet](https://cheatsheetseries.owasp.org/cheatsheets/Cryptographic_Storage_Cheat_Sheet.html)

## Questions?

For security inquiries:
- **Vulnerabilities**: Email with details (responsible disclosure)
- **Implementation**: Open issue or discussion on GitHub
- **General questions**: Start a discussion in GitHub Discussions

---

**Last Updated**: December 2024  
**Version**: 1.0.0
