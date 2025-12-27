#!/bin/bash
# Pre-commit hooks setup script
# Installs git hooks to enforce code quality standards

set -e

HOOKS_DIR=".git/hooks"
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Setting up pre-commit hooks..."

# Create hooks directory if it doesn't exist
mkdir -p "$PROJECT_ROOT/$HOOKS_DIR"

# Pre-commit hook: Check for code style and security issues
cat > "$PROJECT_ROOT/$HOOKS_DIR/pre-commit" << 'EOF'
#!/bin/bash
# Pre-commit hook: Code quality checks

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

FAILED=0

# Check for C source files to be committed
C_FILES=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.[ch]$' || true)

if [ -n "$C_FILES" ]; then
    echo -e "${YELLOW}Running code quality checks...${NC}"
    
    # Check for TODO/FIXME comments that should be documented
    for file in $C_FILES; do
        if grep -n "TODO\|FIXME" "$file" | grep -v "FIXME:" >/dev/null 2>&1; then
            echo -e "${YELLOW}Warning: Found TODO/FIXME in $file - ensure documented in CHANGELOG${NC}"
        fi
    done
    
    # Check for potential memory leaks (basic check)
    if grep -n "malloc\|calloc" $C_FILES | grep -v "free" >/dev/null 2>&1; then
        echo -e "${YELLOW}Info: Found malloc/calloc - verify cleanup in error paths${NC}"
    fi
fi

# Check for trailing whitespace
if git diff --cached --check 2>/dev/null; then
    echo -e "${GREEN}✓ No trailing whitespace${NC}"
else
    echo -e "${RED}✗ Trailing whitespace found${NC}"
    FAILED=1
fi

# Check for merge conflicts
if git diff --cached | grep -q "^<<<<<<<\|^=======\|^>>>>>>>"; then
    echo -e "${RED}✗ Merge conflict markers found${NC}"
    FAILED=1
fi

# Verify CHANGELOG.md is updated (warn only)
if ! git diff --cached --name-only | grep -q "CHANGELOG.md"; then
    echo -e "${YELLOW}⚠ Consider updating CHANGELOG.md${NC}"
fi

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ Pre-commit checks passed${NC}"
    exit 0
else
    echo -e "${RED}✗ Pre-commit checks failed${NC}"
    exit 1
fi
EOF

chmod +x "$PROJECT_ROOT/$HOOKS_DIR/pre-commit"

# Pre-push hook: Verify all tests pass
cat > "$PROJECT_ROOT/$HOOKS_DIR/pre-push" << 'EOF'
#!/bin/bash
# Pre-push hook: Run tests before pushing

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}Running tests before push...${NC}"

# Try to run tests if test infrastructure exists
if [ -d "tests" ]; then
    if [ -f "tests/Makefile" ]; then
        echo -e "${YELLOW}Building and running tests...${NC}"
        cd tests
        if make test >/dev/null 2>&1; then
            echo -e "${GREEN}✓ All tests passed${NC}"
        else
            echo -e "${RED}✗ Tests failed - abort push${NC}"
            cd ..
            exit 1
        fi
        cd ..
    fi
fi

echo -e "${GREEN}✓ Ready to push${NC}"
exit 0
EOF

chmod +x "$PROJECT_ROOT/$HOOKS_DIR/pre-push"

echo -e "\033[0;32m✓ Pre-commit hooks installed successfully\033[0m"
echo "Hooks location: $PROJECT_ROOT/$HOOKS_DIR"
echo ""
echo "Available hooks:"
echo "  - pre-commit: Runs before git commit (whitespace, merge conflicts, CHANGELOG)"
echo "  - pre-push: Runs before git push (tests verification)"
