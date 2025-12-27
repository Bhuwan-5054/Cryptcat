# GitHub Repository Setup Guide

This guide helps you publish Cryptcat to GitHub and configure it for community collaboration.

---

## Prerequisites

- [Git](https://git-scm.com/) installed and configured
- [GitHub account](https://github.com/signup)
- Cryptcat repository cloned locally

---

## Step 1: Initialize Local Git Repository

If not already initialized:

```bash
cd Cryptcat

# Initialize git (if not already done)
git init

# Configure user (if not already configured)
git config user.name "Your Name"
git config user.email "your.email@example.com"

# Add all files
git add .

# Create initial commit
git commit -m "Initial commit: Cryptcat v1.0.0 - Secure encrypted networking tool"
```

---

## Step 2: Create GitHub Repository

1. Go to [GitHub New Repository](https://github.com/new)
2. **Repository name**: `Cryptcat` (or your preferred name)
3. **Description**: "Secure Netcat replacement with Twofish-256 encryption, HMAC authentication, and replay protection"
4. **Visibility**: Public (for open-source)
5. **Initialize repository**: Select **No** (we already have commits)
6. Click **Create repository**

---

## Step 3: Connect Local to GitHub

GitHub will show commands. Run these in your Cryptcat directory:

```bash
# Add remote (replace USERNAME with your GitHub username)
git remote add origin https://github.com/USERNAME/Cryptcat.git

# Rename branch to 'main' (if using 'master')
git branch -M main

# Push to GitHub
git push -u origin main
```

---

## Step 4: Configure Repository Settings

### Branch Protection (Recommended)

1. Go to **Settings** → **Branches**
2. Click **Add rule**
3. Branch name pattern: `main`
4. Check:
   - ✅ Require a pull request before merging
   - ✅ Require status checks to pass before merging
   - ✅ Require code reviews
5. Click **Create**

### Enable GitHub Pages (Optional)

For documentation website:

1. **Settings** → **Pages**
2. **Source**: `main` branch
3. **Folder**: `/ (root)`
4. Click **Save**

Docs will be available at: `https://username.github.io/Cryptcat/`

### Configure Actions (CI/CD)

1. **Settings** → **Actions** → **General**
2. Check:
   - ✅ Allow all actions and reusable workflows
3. Save

### Add Deploy Key (Optional, for automated releases)

For GitHub Actions to create releases:

1. **Settings** → **Secrets and variables** → **Actions**
2. No additional setup needed (GitHub automatically provides `GITHUB_TOKEN`)

---

## Step 5: Add Topics & Description

1. Go to repository home
2. Click **About** (gear icon on right)
3. Add **Description**: "Secure encrypted networking tool with Twofish-256 encryption"
4. Add **Topics** (comma-separated):
   ```
   encryption, cryptography, security, networking, netcat, 
   open-source, c, cross-platform, twofish
   ```
5. Check **Include in search**: Yes
6. Click **Save changes**

---

## Step 6: Create First Release

1. Go to **Releases** (right sidebar)
2. Click **Create a new release**
3. **Tag version**: `v1.0.0`
4. **Release title**: `Cryptcat v1.0.0 - Stable Release`
5. **Description**: Copy from [CHANGELOG.md](CHANGELOG.md#v100-2024-12-27)
6. **Pre-release**: Uncheck (this is stable)
7. Click **Publish release**

---

## Step 7: Enable Discussions (Optional but Recommended)

1. **Settings** → **Features**
2. Check ✅ **Discussions**
3. Click **Save**

This enables GitHub Discussions for community Q&A.

---

## Step 8: Customize GitHub Profile/Organization (Optional)

Create a `README.md` in a special repository:

1. Create repository: `USERNAME/USERNAME` (name = your GitHub username)
2. Add content:
   ```markdown
   # About Me
   
   Creator of Cryptcat - a secure, open-source encryption tool.
   
   - 🔒 Security-focused developer
   - 🚀 Open-source enthusiast
   - 📚 [Check out my projects](https://github.com/yourusername?tab=repositories)
   ```

---

## Step 9: Verify Setup

### Check CI/CD Works

1. Make a test commit:
   ```bash
   git commit --allow-empty -m "test: verify CI/CD pipeline"
   git push origin main
   ```

2. Go to **Actions** tab
3. Verify **CI** workflow runs:
   - ✅ Builds on Linux, macOS, Windows
   - ✅ Tests pass
   - ✅ Sanitizers run
   - ✅ Coverage generated

### Test Release Workflow

1. Create a tag:
   ```bash
   git tag v1.0.1 -m "Test release"
   git push origin v1.0.1
   ```

2. Go to **Actions** tab
3. Verify **Release** workflow:
   - ✅ Creates release notes
   - ✅ Builds artifacts
   - ✅ Creates GitHub Release

4. Delete test release:
   ```bash
   git tag -d v1.0.1
   git push origin --delete v1.0.1
   ```

---

## Optional Enhancements

### Add License Badge

Update [README.md](README.md) with:

```markdown
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
```

### Add GitHub Status Badge

Update [README.md](README.md) with (replace USERNAME):

```markdown
[![Build Status](https://github.com/USERNAME/Cryptcat/actions/workflows/ci.yml/badge.svg)](https://github.com/USERNAME/Cryptcat/actions)
```

### Setup Automatic Code Coverage

1. Sign up at [Codecov.io](https://codecov.io)
2. Add repository
3. Update `.github/workflows/ci.yml` with:

```yaml
- name: Upload coverage to Codecov
  uses: codecov/codecov-action@v3
  with:
    file: ./build/coverage.info
```

4. Add badge to README.md:

```markdown
[![codecov](https://codecov.io/gh/USERNAME/Cryptcat/branch/main/graph/badge.svg)](https://codecov.io/gh/USERNAME/Cryptcat)
```

### Enable Renovate (Dependency Updates)

1. Create `.renovaterc.json`:

```json
{
  "extends": ["config:base"],
  "schedule": ["before 3am on Monday"]
}
```

2. Install [Renovate Bot](https://github.com/apps/renovate)
3. Approve dependency update PRs

---

## Post-Setup Checklist

- [ ] Repository created on GitHub
- [ ] Local repository connected (`git remote -v` shows origin)
- [ ] Pushed to GitHub (`git push origin main`)
- [ ] CI/CD pipeline runs successfully
- [ ] First release created (v1.0.0)
- [ ] Topics added
- [ ] Description complete
- [ ] Branch protection enabled
- [ ] Discussions enabled (optional)
- [ ] Badges added to README (optional)

---

## Continuous Updates

### For Each Release

```bash
# Create tag
git tag v1.0.1 -m "Release: version 1.0.1"

# Push tag (triggers release workflow)
git push origin v1.0.1

# GitHub Actions will automatically:
# 1. Build cross-platform artifacts
# 2. Create GitHub Release
# 3. Upload artifacts
```

### Update Documentation

1. Update [CHANGELOG.md](CHANGELOG.md)
2. Update version numbers in relevant files
3. Commit: `git commit -am "docs: prepare for v1.0.1"`
4. Tag: `git tag v1.0.1`
5. Push: `git push origin main && git push origin v1.0.1`

---

## Troubleshooting

### "fatal: pathspec 'origin' did not match any files"

You haven't added the remote yet:

```bash
git remote add origin https://github.com/USERNAME/Cryptcat.git
```

### "Permission denied (publickey)"

Setup SSH key:

```bash
# Generate SSH key
ssh-keygen -t ed25519 -C "your.email@example.com"

# Add to GitHub: Settings → SSH and GPG keys → New SSH key
# Copy output of:
cat ~/.ssh/id_ed25519.pub

# Test connection
ssh -T git@github.com
```

### CI/CD doesn't run

1. Check **.github/workflows/** files exist
2. Verify workflow syntax: `git push` triggers workflow
3. Check **Actions** tab for error messages
4. Verify OpenSSL is available in CI environment

### Release artifacts not uploading

1. Verify `release.yml` syntax
2. Check `GITHUB_TOKEN` has write permissions
3. Verify binary paths in workflow match build output

---

## Resources

- [GitHub Docs](https://docs.github.com/)
- [GitHub Actions Quickstart](https://docs.github.com/en/actions/quickstart)
- [Release Management](https://docs.github.com/en/repositories/releasing-projects-on-github)
- [GitHub Discussions Setup](https://docs.github.com/en/discussions)

---

## Next Steps

1. **Publish Cryptcat** on GitHub following above steps
2. **Announce** to relevant communities
3. **Gather feedback** from users
4. **Plan v1.1** improvements based on feedback
5. **Build community** around the project

---

**Welcome to open-source! 🚀**
