# Compiler settings
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Wpedantic -O2
DEBUG_FLAGS = -std=c++11 -Wall -Wextra -Wpedantic -g -DDEBUG -fsanitize=address,leak
RELEASE_FLAGS = -std=c++11 -Wall -Wextra -Wpedantic -O3 -DNDEBUG

# Project settings
PROJECT = yaml_parser
TEST_EXEC = test_yaml
EXAMPLE_EXEC = example_yaml

# Source files
TEST_SRC = test_yaml.cpp yaml.cpp
EXAMPLE_SRC = example_yaml.cpp  
  

# Default target
.PHONY: all test debug release clean install example help

all: test

# Test targets
test: $(TEST_EXEC)
	@echo "=== Running Tests ==="
	./$(TEST_EXEC)

$(TEST_EXEC): $(TEST_SRC)  
	$(CXX) $(CXXFLAGS) $(TEST_SRC) -o $(TEST_EXEC)

# Debug build with sanitizers
debug: CXXFLAGS = $(DEBUG_FLAGS)
debug: $(TEST_EXEC)
	@echo "=== Running Debug Tests ==="
	./$(TEST_EXEC)

# Release optimized build
release: CXXFLAGS = $(RELEASE_FLAGS)
release: $(TEST_EXEC)
	@echo "=== Running Release Tests ==="
	./$(TEST_EXEC)

# Example usage
example: $(EXAMPLE_EXEC)

$(EXAMPLE_EXEC): $(EXAMPLE_SRC) $(HEADER)
	$(CXX) $(CXXFLAGS) $(EXAMPLE_SRC) -o $(EXAMPLE_EXEC)
	./$(EXAMPLE_EXEC)

# Validation targets
validate: $(HEADER)
	@echo "=== Header Validation ==="
	$(CXX) $(CXXFLAGS) -fsyntax-only -DYAML_IMPLEMENTATION $(HEADER)
	@echo "Header syntax: OK"

syntax-check: validate

# Performance test
performance: CXXFLAGS = $(RELEASE_FLAGS) -DPERFORMANCE_TEST
performance: $(TEST_EXEC)
	@echo "=== Performance Test ==="
	time ./$(TEST_EXEC)

# Memory test with valgrind
memory-test: debug
	@echo "=== Memory Test with Valgrind ==="
	valgrind --leak-check=full --show-leak-kinds=all ./$(TEST_EXEC)

# Static analysis
static-analysis: $(HEADER)
	@echo "=== Static Analysis ==="
	cppcheck --enable=all --std=c++11 --suppress=missingIncludeSystem $(TEST_SRC)

# Coverage (requires gcov)
coverage: CXXFLAGS = -std=c++11 -Wall -g -fprofile-arcs -ftest-coverage
coverage: $(TEST_EXEC)
	./$(TEST_EXEC)
	gcov $(TEST_SRC)
	@echo "Coverage files generated: *.gcov"

# Installation
install: $(HEADER)
	@echo "Installing header-only library..."
	mkdir -p $(PREFIX)/include/yaml
	cp $(HEADER) $(PREFIX)/include/yaml/
	@echo "Installed to $(PREFIX)/include/yaml/"

PREFIX ?= /usr/local

# Package creation
package:
	@echo "Creating package..."
	mkdir -p yaml-parser-package
	cp $(HEADER) yaml-parser-package/
	cp Makefile yaml-parser-package/
	cp $(TEST_SRC) yaml-parser-package/
	cp README.md yaml-parser-package/ 2>/dev/null || touch yaml-parser-package/README.md
	tar -czf yaml-parser-$(shell date +%Y%m%d).tar.gz yaml-parser-package/
	rm -rf yaml-parser-package/
	@echo "Package created: yaml-parser-$(shell date +%Y%m%d).tar.gz"

# Documentation generation (requires doxygen)
docs:
	@echo "Generating documentation..."
	doxygen Doxyfile 2>/dev/null || echo "Doxyfile not found, skipping docs"

# Clean targets
clean:
	rm -f $(TEST_EXEC) $(EXAMPLE_EXEC)
	rm -f *.o *.gcov *.gcda *.gcno
	rm -f *.tar.gz
	rm -rf yaml-parser-package/

clean-all: clean
	rm -rf docs/

# Help
help:
	@echo "=== YAML Parser Makefile ==="
	@echo "Available targets:"
	@echo "  all          - Build and run tests (default)"
	@echo "  test         - Build and run tests"
	@echo "  debug        - Build with debug flags and run tests"
	@echo "  release      - Build optimized version and run tests"
	@echo "  example      - Build example usage"
	@echo "  validate     - Validate header syntax"
	@echo "  performance  - Run performance tests"
	@echo "  memory-test  - Run with valgrind (requires valgrind)"
	@echo "  coverage     - Generate coverage report (requires gcov)"
	@echo "  static-analysis - Run static analysis (requires cppcheck)"
	@echo "  install      - Install header (PREFIX=/usr/local)"
	@echo "  package      - Create distribution package"
	@echo "  docs         - Generate documentation (requires doxygen)"
	@echo "  clean        - Remove build artifacts"
	@echo "  clean-all    - Remove all generated files"
	@echo "  help         - Show this help"
	@echo ""
	@echo "C++11 compatible | Header-only library"

