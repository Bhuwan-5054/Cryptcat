# Cryptcat Project Makefile
# Convenience targets for common development tasks

.PHONY: help build test clean install uninstall docs coverage lint format pre-commit

# Default target
help:
	@echo "╔═══════════════════════════════════════════════════════════╗"
	@echo "║          Cryptcat Development Makefile                    ║"
	@echo "╚═══════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Available targets:"
	@echo ""
	@echo "  build          - Build Cryptcat (Debug mode)"
	@echo "  build-release  - Build Cryptcat (Release mode)"
	@echo "  test           - Build and run all tests"
	@echo "  test-unit      - Run unit tests only"
	@echo "  test-integration - Run integration tests only"
	@echo "  test-bench     - Run performance benchmarks"
	@echo "  test-asan      - Run tests with AddressSanitizer"
	@echo "  test-ubsan     - Run tests with UndefinedBehaviorSanitizer"
	@echo ""
	@echo "  coverage       - Generate code coverage report"
	@echo "  coverage-view  - Open coverage report in browser"
	@echo "  lint           - Check code with static analysis"
	@echo "  format         - Auto-format code (C files)"
	@echo "  format-check   - Check code formatting without modifying"
	@echo ""
	@echo "  install        - Install Cryptcat to /usr/local/bin (Linux/macOS)"
	@echo "  uninstall      - Remove Cryptcat from /usr/local/bin"
	@echo "  clean          - Remove build artifacts"
	@echo "  distclean      - Clean everything including build/ and dist/"
	@echo ""
	@echo "  docker-build   - Build Docker image"
	@echo "  docker-run     - Run Docker container interactively"
	@echo ""
	@echo "  pre-commit-setup - Install git pre-commit hooks"
	@echo "  pre-commit-test - Test pre-commit hooks"
	@echo ""
	@echo "  docs           - Generate API documentation"
	@echo "  help           - Show this help message"
	@echo ""

# Build targets
build:
	@echo "Building Cryptcat (Debug)..."
	@mkdir -p build
	@cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make

build-release:
	@echo "Building Cryptcat (Release)..."
	@mkdir -p build
	@cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make

# Test targets
test: build
	@echo "Running all tests..."
	@cd tests && make && ./run_tests

test-unit: build
	@echo "Running unit tests..."
	@cd tests && make && ./run_tests 2>&1 | grep -A1 "^Unit Tests:"

test-integration: build
	@echo "Running integration tests..."
	@cd tests && make && ./run_tests 2>&1 | grep -A1 "^Integration Tests:"

test-bench: build
	@echo "Running performance benchmarks..."
	@cd tests && make benchmark && ./benchmark_crypto

test-asan: build
	@echo "Running tests with AddressSanitizer..."
	@cd build && ASAN_OPTIONS=verbosity=1:halt_on_error=1 make test

test-ubsan: build
	@echo "Running tests with UndefinedBehaviorSanitizer..."
	@cd build && UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 make test

# Code coverage
coverage: clean
	@echo "Building with code coverage instrumentation..."
	@mkdir -p build
	@cd build && cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON .. && make
	@echo "Running tests to collect coverage data..."
	@cd tests && make && ./run_tests >/dev/null 2>&1
	@echo "Generating coverage report..."
	@lcov --directory build --capture --output-file build/coverage.info
	@lcov --remove build/coverage.info '/usr/*' '*/tests/*' --output-file build/coverage.info
	@genhtml build/coverage.info --output-directory build/coverage_report
	@echo "Coverage report generated: build/coverage_report/index.html"

coverage-view:
	@if [ -f build/coverage_report/index.html ]; then \
		open build/coverage_report/index.html; \
	else \
		echo "Coverage report not found. Run 'make coverage' first."; \
	fi

# Code quality
lint:
	@echo "Running static analysis (cppcheck)..."
	@cppcheck --enable=all --suppress=missingIncludeSystem src/

format:
	@echo "Formatting C code..."
	@find src tests/unit tests/integration tests/performance tests/frameworks -name "*.c" -o -name "*.h" | xargs clang-format -i
	@echo "Code formatted!"

format-check:
	@echo "Checking code formatting..."
	@find src tests/unit tests/integration tests/performance tests/frameworks -name "*.c" -o -name "*.h" | xargs clang-format --dry-run -Werror

# Installation targets (Unix/Linux/macOS only)
install: build-release
	@if [ ! -f build/src/cryptcat ]; then \
		echo "Error: cryptcat binary not found in build/src/"; \
		exit 1; \
	fi
	@echo "Installing Cryptcat to /usr/local/bin..."
	@sudo cp build/src/cryptcat /usr/local/bin/
	@sudo chmod 755 /usr/local/bin/cryptcat
	@echo "Installation complete! Run: cryptcat --help"

uninstall:
	@echo "Removing Cryptcat from /usr/local/bin..."
	@sudo rm -f /usr/local/bin/cryptcat
	@echo "Uninstall complete!"

# Cleanup targets
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf build/
	@rm -rf tests/coverage_report
	@rm -rf dist/
	@find . -name "*.o" -delete
	@find . -name "*.a" -delete
	@find . -name "*~" -delete
	@echo "Clean complete!"

distclean: clean
	@echo "Removing all generated files..."
	@rm -rf .vscode/.c_cpp_properties.json
	@rm -rf compile_commands.json
	@echo "Distclean complete!"

# Docker targets
docker-build:
	@echo "Building Docker image..."
	@docker build -t cryptcat:latest .
	@echo "Docker image built: cryptcat:latest"

docker-run: docker-build
	@echo "Running Cryptcat in Docker..."
	@docker run -it cryptcat:latest /bin/bash

docker-shell:
	@docker run -it -v $$(pwd):/workspace cryptcat:latest /bin/bash

# Git hooks
pre-commit-setup:
	@echo "Setting up pre-commit hooks..."
	@bash scripts/setup-hooks.sh

pre-commit-test:
	@echo "Testing pre-commit hooks..."
	@bash .git/hooks/pre-commit

# Documentation
docs:
	@echo "Generating documentation..."
	@mkdir -p build/docs
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen Doxyfile; \
		echo "Documentation generated in build/docs/"; \
	else \
		echo "Doxygen not installed. Skipping documentation generation."; \
	fi

# Development convenience
dev-setup:
	@echo "Setting up development environment..."
	@make pre-commit-setup
	@if command -v code >/dev/null 2>&1; then \
		echo "Installing VS Code extensions..."; \
		code --install-extension ms-vscode.cpptools; \
		code --install-extension ms-vscode.cmake-tools; \
	fi
	@echo "Development environment setup complete!"

# Git workflow helpers
commit-msg:
	@echo "Remember to include:"
	@echo "  - Type: feat, fix, docs, style, refactor, test, ci"
	@echo "  - Scope: core, tests, build, docs, security"
	@echo "  - Body: Explain what and why"
	@echo "  - Footer: Closes #issue"

bump-patch:
	@echo "Bumping patch version..."
	@current=$$(grep "^version =" Cargo.toml | sed "s/.*= \"\([^\"]*\)\".*/\1/"); \
	major=$$(echo $$current | cut -d. -f1); \
	minor=$$(echo $$current | cut -d. -f2); \
	patch=$$(echo $$current | cut -d. -f3 | cut -d- -f1); \
	new=$$major.$$minor.$$((patch+1)); \
	echo "Version: $$current -> $$new"

# Phony targets (don't create files with these names)
.PHONY: help build build-release test test-unit test-integration test-bench test-asan test-ubsan
.PHONY: coverage coverage-view lint format format-check
.PHONY: install uninstall clean distclean
.PHONY: docker-build docker-run docker-shell
.PHONY: pre-commit-setup pre-commit-test
.PHONY: docs dev-setup commit-msg bump-patch
