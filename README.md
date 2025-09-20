

# YAML Parser C++11

A lightweight, single-header YAML parser and serializer for C++11 with comprehensive feature support and robust error handling.

[![CI Status](https://github.com/akadjoker/yaml_parser/workflows/C%2B%2B%20CI/badge.svg)](https://github.com/YOUR_USERNAME/yaml-parser-cpp/actions)
[![Tests Passing](https://img.shields.io/badge/tests-29%2F29%20passing-brightgreen.svg)](#testing)
[![Memory Safe](https://img.shields.io/badge/memory-safe-brightgreen.svg)](#memory-safety)
[![C++11](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://isocpp.org/)
[![Address Sanitizer](https://img.shields.io/badge/ASAN-enabled-red.svg)](#debugging)
[![Code Coverage](https://img.shields.io/badge/coverage-tracked-blue.svg)](#code-coverage)


## Features

- **Single Header Library** - Easy integration, just include `yaml_single.hpp`
- **C++11 Compatible** - Works with older compilers and embedded systems
- **Memory Safe** - RAII-based memory management with zero leaks
- **Comprehensive Type Support** - Strings, numbers, booleans, null, sequences, mappings
- **Smart String Parsing** - Handles unquoted strings with spaces intelligently
- **Flow Style Support** - Parses `{key: value}` and `[item1, item2]` syntax
- **Nested Structures** - Deep nesting of mappings and sequences
- **Round-trip Serialization** - Parse YAML → modify → serialize back to YAML
- **Robust Error Handling** - Detailed exception messages with line/column info
- **Performance Optimized** - Sub-millisecond parsing for typical configurations
- **Error Handling**: Detailed error reporting with line/column information
- **Serialization**: Convert parsed data back to YAML format

## Quick Start

### Installation

Simply copy `yaml.hpp` and `yaml.cpp` to your project directory.

Or 

```cpp
#define YAML_IMPLEMENTATION   
#include "yaml_single.hpp"
```


### Basic Usage 

```cpp
#include "yaml.hpp"
#include <iostream>

int main() {
    std::string yamlText = R"(
        name: John Doe
        age: 30
        active: true
        hobbies:
          - reading
          - coding
          - gaming
        address:
          street: 123 Main Street
          city: New York
    )";
    
    try {
        yaml::YamlValue root = yaml::parse(yamlText);
        
        // Access values
        std::cout << "Name: " << root["name"].asString() << std::endl;
        std::cout << "Age: " << root["age"].asInt() << std::endl;
        std::cout << "Active: " << root["active"].asBool() << std::endl;
        
        // Access nested values
        std::cout << "City: " << root["address"]["city"].asString() << std::endl;
        
        // Iterate through sequences
        for (size_t i = 0; i < root["hobbies"].size(); ++i) {
            std::cout << "Hobby: " << root["hobbies"][i].asString() << std::endl;
        }
        
    } catch (const yaml::YamlException& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
    }
    
    return 0;
}
```

### Type Checking and Conversion

```cpp
yaml::YamlValue value = yaml::parse("count: 42");

// Type checking
if (value["count"].isNumber()) {
    std::cout << "It's a number!" << std::endl;
}

// Type conversion
int count = value["count"].asInt();
double countFloat = value["count"].asNumber();

// Safe access with contains()
if (value.contains("optional_field")) {
    std::string field = value["optional_field"].asString();
}
```

### Working with Complex Structures

```cpp
std::string config = R"(
    database:
      host: localhost
      port: 5432
      credentials:
        username: admin
        password: secret
    servers:
      - name: web-01
        ip: 192.168.1.10
      - name: web-02
        ip: 192.168.1.11
    features: {ssl: true, debug: false}
)";

yaml::YamlValue root = yaml::parse(config);

// Access nested mappings
std::string dbHost = root["database"]["host"].asString();
int dbPort = root["database"]["port"].asInt();

// Work with sequences
auto& servers = root["servers"].asSequence();
for (const auto& server : servers) {
    std::cout << server["name"].asString() << ": " 
              << server["ip"].asString() << std::endl;
}

// Flow syntax
bool sslEnabled = root["features"]["ssl"].asBool();
```

### Compilation

```bash
g++ -std=c++11 -Wall -Wextra -O2 your_program.cpp yaml.cpp -o your_program
```


## API Reference

### YamlValue Class

The main class for representing YAML values of any type.

#### Type Checking

```cpp
bool isNull() const;      // null values
bool isBool() const;      // true/false
bool isNumber() const;    // integers and floats
bool isString() const;    // text strings
bool isSequence() const;  // arrays/lists
bool isMapping() const;   // objects/dictionaries
```


#### Value Access

```cpp
bool asBool() const;                    // Get boolean value
int asInt() const;                      // Get integer value  
double asNumber() const;                // Get numeric value
const std::string& asString() const;    // Get string value
Sequence& asSequence();                 // Get array reference
Mapping& asMapping();                   // Get object reference
```


#### Container Operations

```cpp
YamlValue& operator[](const std::string& key);  // Access by key
YamlValue& operator[](size_t index);            // Access by index
size_t size() const;                            // Get container size
bool empty() const;                             // Check if empty
bool contains(const std::string& key) const;   // Check key exists
```

### Dynamic Data Structure

```cpp
// Create YAML structure programmatically
yaml::YamlValue root;
root["name"] = yaml::YamlValue("John Doe");
root["age"] = yaml::YamlValue(30);
root["hobbies"] = yaml::YamlValue(yaml::YamlValue::Sequence{
    yaml::YamlValue("reading"),
    yaml::YamlValue("coding")
});

// Serialize to string
std::string output = root.serialize();
std::cout << output << std::endl;
```

### Configuration File
```yaml
# config.yaml
app:
  name: My Application
  version: 1.2.0
  debug: false

database:
  host: db.example.com
  port: 5432
  pool_size: 10

logging:
  level: info
  file: /var/log/app.log
```

```cpp
std::ifstream file("config.yaml");
std::string content((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());

yaml::YamlValue config = yaml::parse(content);

std::string appName = config["app"]["name"].asString();
bool debugMode = config["app"]["debug"].asBool();
int dbPort = config["database"]["port"].asInt();
```

### Parsing Functions

```cpp
yaml::YamlValue yaml::parse(const std::string& yaml_text);
```


## Supported YAML Features

### ✅ Supported

- **Scalars**: strings, integers, floats, booleans, null
- **Sequences**: `[item1, item2]` and block style with `-`
- **Mappings**: `{key: value}` and block style with `:`
- **Nested structures**: unlimited depth
- **Quoted strings**: single and double quotes with escape sequences
- **Unquoted strings**: intelligent parsing including strings with spaces
- **Flow style**: inline JSON-like syntax
- **Empty structures**: `{}`, `[]`
- **Comments**: `#` line comments
- **Multi-line values**: strings spanning multiple lines


### ❌ Not Supported

- **Anchors \& Aliases**: `&ref` and `*ref`
- **Multi-document**: `---` separators
- **Advanced scalars**: timestamps, binary data
- **Complex keys**: non-string mapping keys
- **Merge keys**: `<<:`
- **Tags**: `!!str`, `!!int`, etc.


## Testing

The library includes comprehensive tests covering  different scenarios:

```bash
# Compile and run tests
make test

# Run specific test suites
make test-original    # Original 10 tests
make test-extended    # Extended 19 tests
make test-all        # All tests

# Memory leak checking (requires valgrind)
make valgrind
```


### Test Coverage

- Basic types (string, number, boolean, null)
- Complex nested structures
- Edge cases and error conditions
- Memory stress testing (100+ iterations)
- Performance benchmarks
- Round-trip serialization
- Error handling and exception safety


## Performance

Benchmarks on typical configuration files:

- **Simple config** (10 keys): < 1ms
- **Complex nested** (100+ keys): < 5ms
- **Memory usage**: Minimal overhead with RAII
- **Zero memory leaks**: Verified with stress testing


## Memory Management

The library uses RAII (Resource Acquisition Is Initialization) for automatic memory management:

- **Automatic cleanup**: All memory freed when objects go out of scope
- **Copy semantics**: Deep copying with proper resource management
- **Move semantics**: Efficient transfers with C++11 move constructors
- **Exception safety**: Memory cleaned up even during exceptions


## Error Handling

```cpp
try {
    yaml::YamlValue root = yaml::parse(yaml_text);
    // Use root...
} catch (const yaml::YamlException& e) {
    std::cout << "Parse error: " << e.what() 
              << " at line " << e.line 
              << ", column " << e.column << std::endl;
}
```


## Real-World Usage

Perfect for:

- **Configuration files**: Application settings, server configs
- **Data exchange**: Simple API responses, data serialization
- **Embedded systems**: Lightweight parsing with minimal dependencies
- **Build systems**: CMake, Make integration
- **IoT applications**: Resource-constrained environments


## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Add tests for new functionality
4. Ensure all tests pass (`make test-all`)
5. Commit changes (`git commit -am 'Add amazing feature'`)
6. Push to branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

## License

MIT License - see LICENSE file for details.

## Author

Created with focus on simplicity, performance, and reliability for C++11 projects.


## Changelog

### v1.0.0
- Initial release
- Core YAML parsing functionality
- Comprehensive test suite
- Memory safety validation


***

**Note**: This is a focused YAML parser optimized for common use cases. For full YAML 1.2 specification compliance, consider [yaml-cpp](https://github.com/jbeder/yaml-cpp) or [RapidYAML](https://github.com/biojppm/rapidyaml).
<span style="display:none">[^1][^10][^2][^3][^4][^5][^6][^7][^8][^9]</span>

<div style="text-align: center">⁂</div>

[^1]: https://github.com/jbeder/yaml-cpp

[^2]: https://git.uibk.ac.at/csaz9385/marian-dev-but-it-actually-builds/-/blob/main/src/3rd_party/yaml-cpp/README.md

[^3]: https://stackoverflow.com/questions/365155/parse-yaml-files-in-c

[^4]: https://docs.ros.org/en/iron/p/rcl_yaml_param_parser/generated/index.html

[^5]: https://waterprogramming.wordpress.com/2012/02/09/using-yaml-in-c/

[^6]: https://fortran-lang.discourse.group/t/a-yaml-parser-for-fortran-fortran-yaml-cpp/2022

[^7]: https://vcpkg.link/ports/yaml-cpp

[^8]: https://www.reddit.com/r/golang/comments/9e58oe/how_to_develop_a_simple_language_parser_like_yaml/

[^9]: https://answers.opencv.org/question/226967/how-do-i-write-into-yml-format-and-read-it-with-opencv-and-c/

[^10]: https://www.reddit.com/r/cpp_questions/comments/m7hbvq/modern_yaml_library_with_clear_docs/

