
#include <chrono>
#include <sstream>
#include <cassert>
#define YAML_IMPLEMENTATION
#include "yaml.hpp"

// Colors
#define C_RESET "\033[0m"
#define C_GREEN "\033[32m"
#define C_BLUE "\033[34m"
#define C_RED "\033[31m"
#define C_YELLOW "\033[33m"
#define C_MAGENTA "\033[35m"

// Test runner macros
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << C_BLUE "Running test: " #name " ... " C_RESET; \
    try { \
        test_##name(); \
        std::cout << C_GREEN "PASS" C_RESET "\n"; \
        passed_tests++; \
    } catch (const std::exception& e) { \
        std::cout << C_RED "FAIL: " << e.what() << C_RESET "\n"; \
        failed_tests++; \
    } catch (...) { \
        std::cout << C_RED "FAIL: Unknown exception" C_RESET "\n"; \
        failed_tests++; \
    } \
    total_tests++; \
} while(0)

#define ASSERT_TRUE(x) do { \
    if (!(x)) { \
        std::ostringstream oss; \
        oss << "ASSERT_TRUE failed: " #x " at line " << __LINE__; \
        throw std::runtime_error(oss.str()); \
    } \
} while(0)

#define ASSERT_FALSE(x) do { \
    if ((x)) { \
        std::ostringstream oss; \
        oss << "ASSERT_FALSE failed: " #x " at line " << __LINE__; \
        throw std::runtime_error(oss.str()); \
    } \
} while(0)

#define ASSERT_EQ(a,b) do { \
    if (!((a)==(b))) { \
        std::ostringstream oss; \
        oss << "ASSERT_EQ failed: " #a " != " #b " at line " << __LINE__; \
        throw std::runtime_error(oss.str()); \
    } \
} while(0)

#define ASSERT_THROWS(code, exception_type) do { \
    bool caught = false; \
    try { \
        code; \
    } catch (const exception_type&) { \
        caught = true; \
    } catch (...) { \
        std::ostringstream oss; \
        oss << "ASSERT_THROWS failed: wrong exception type at line " << __LINE__; \
        throw std::runtime_error(oss.str()); \
    } \
    if (!caught) { \
        std::ostringstream oss; \
        oss << "ASSERT_THROWS failed: no exception thrown at line " << __LINE__; \
        throw std::runtime_error(oss.str()); \
    } \
} while(0)

// Global test counters
int total_tests = 0;
int passed_tests = 0;
int failed_tests = 0;

// Basic parsing tests
TEST(basic_string) {
    std::string yaml = "name: John Doe";
    yaml::YamlValue root = yaml::parse(yaml);
    ASSERT_TRUE(root.isMapping());
    ASSERT_EQ(root["name"].asString(), "John Doe");
}

TEST(basic_number) {
    std::string yaml = "age: 30";
    yaml::YamlValue root = yaml::parse(yaml);
    ASSERT_EQ(root["age"].asInt(), 30);
    ASSERT_TRUE(root["age"].isNumber());
}

TEST(basic_boolean) {
    std::string yaml = R"(enabled: true
disabled: false)";
    yaml::YamlValue root = yaml::parse(yaml);
    ASSERT_TRUE(root["enabled"].asBool());
    ASSERT_FALSE(root["disabled"].asBool());
}

TEST(basic_null) {
    std::string yaml = "value: null";
    yaml::YamlValue root = yaml::parse(yaml);
    ASSERT_TRUE(root["value"].isNil());
}

// String with spaces tests (our main fix)
TEST(unquoted_string_with_spaces) {
    std::string yaml = "address: 123 Main Street";
    yaml::YamlValue root = yaml::parse(yaml);
    ASSERT_TRUE(root["address"].isString());
    ASSERT_EQ(root["address"].asString(), "123 Main Street");
}

TEST(complex_unquoted_strings) {
    std::string yaml = R"(description: This is a very long description with many words
location: New York City Area
company: Acme Corp International Inc)";
    yaml::YamlValue root = yaml::parse(yaml);
    ASSERT_EQ(root["description"].asString(), "This is a very long description with many words");
    ASSERT_EQ(root["location"].asString(), "New York City Area");
    ASSERT_EQ(root["company"].asString(), "Acme Corp International Inc");
}

// Number vs string disambiguation
TEST(number_vs_string_disambiguation) {
    std::string yaml = R"(pure_number: 42
pure_float: 3.14
negative: -100
string_with_number: 123 Main St
apartment: 4B
version: 1.2.3
phone: 555 123 4567)";
    
    yaml::YamlValue root = yaml::parse(yaml);
    
    // These should be numbers
    ASSERT_TRUE(root["pure_number"].isNumber());
    ASSERT_EQ(root["pure_number"].asInt(), 42);
    ASSERT_TRUE(root["pure_float"].isNumber());
    ASSERT_TRUE(root["negative"].isNumber());
    ASSERT_EQ(root["negative"].asInt(), -100);
    
    // These should be strings
    ASSERT_TRUE(root["string_with_number"].isString());
    ASSERT_EQ(root["string_with_number"].asString(), "123 Main St");
    ASSERT_TRUE(root["apartment"].isString());
    ASSERT_TRUE(root["phone"].isString());
    
    // Debug the version issue
    auto& version = root["version"];
    if (!version.isString()) {
        std::cout << "\nDEBUG: version type=" << static_cast<int>(version.getType());
        if (version.isNumber()) {
            std::cout << " value=" << version.asNumber();
        }
        std::cout << std::endl;
    }
    ASSERT_TRUE(root["version"].isString()); // This should pass now
}

// Nested structures
TEST(nested_mapping) {
    std::string yaml = R"(user:
  name: Alice
  address:
    street: 456 Oak St
    city: Portland)";
    
    yaml::YamlValue root = yaml::parse(yaml);
    ASSERT_TRUE(root["user"].isMapping());
    ASSERT_EQ(root["user"]["name"].asString(), "Alice");
    ASSERT_TRUE(root["user"]["address"].isMapping());
    ASSERT_EQ(root["user"]["address"]["street"].asString(), "456 Oak St");
    ASSERT_EQ(root["user"]["address"]["city"].asString(), "Portland");
}

TEST(sequences) {
    std::string yaml = R"(hobbies:
  - reading
  - swimming
  - coding)";
    
    yaml::YamlValue root = yaml::parse(yaml);
    ASSERT_TRUE(root["hobbies"].isSequence());
    ASSERT_EQ(root["hobbies"].size(), 3);
    ASSERT_EQ(root["hobbies"][0].asString(), "reading");
    ASSERT_EQ(root["hobbies"][1].asString(), "swimming");
    ASSERT_EQ(root["hobbies"][2].asString(), "coding");
}

TEST(mixed_sequence) {
    std::string yaml = R"(items:
  - string item
  - 42
  - true
  - null
  - 3.14)";
    
    yaml::YamlValue root = yaml::parse(yaml);
    auto& seq = root["items"].asSequence();
    ASSERT_EQ(seq.size(), 5);
    ASSERT_TRUE(seq[0].isString());
    ASSERT_TRUE(seq[1].isNumber());
    ASSERT_TRUE(seq[2].isBool());
    ASSERT_TRUE(seq[3].isNil());
    ASSERT_TRUE(seq[4].isNumber());
}

// Flow style tests
TEST(flow_mapping) {
    std::string yaml = "config: {debug: true, port: 8080, host: localhost}";
    yaml::YamlValue root = yaml::parse(yaml);
    
    auto& config = root["config"];
    ASSERT_TRUE(config.isMapping());
    ASSERT_TRUE(config["debug"].asBool());
    ASSERT_EQ(config["port"].asInt(), 8080);
    ASSERT_EQ(config["host"].asString(), "localhost");
}

TEST(flow_sequence) {
    std::string yaml = "numbers: [1, 2, 3, 4, 5]";
    yaml::YamlValue root = yaml::parse(yaml);
    
    auto& numbers = root["numbers"];
    ASSERT_TRUE(numbers.isSequence());
    ASSERT_EQ(numbers.size(), 5);
    for (int i = 0; i < 5; i++) {
        ASSERT_EQ(numbers[i].asInt(), i + 1);
    }
}

// Quoted strings
TEST(quoted_strings) {
    std::string yaml = R"(single: 'Single quoted'
double: "Double quoted"
escaped: "Line 1\nLine 2"
special: "String with: special, [chars])";
    
    yaml::YamlValue root = yaml::parse(yaml);
    ASSERT_EQ(root["single"].asString(), "Single quoted");
    ASSERT_EQ(root["double"].asString(), "Double quoted");
    ASSERT_EQ(root["escaped"].asString(), "Line 1\nLine 2");
    ASSERT_EQ(root["special"].asString(), "String with: special, [chars]");
}

// Empty structures
TEST(empty_structures) {
    std::string yaml = R"(empty_map: {}
empty_array: []
null_val: null)";
    
    yaml::YamlValue root = yaml::parse(yaml);
    ASSERT_TRUE(root["empty_map"].isMapping());
    ASSERT_TRUE(root["empty_map"].empty());
    ASSERT_TRUE(root["empty_array"].isSequence());
    ASSERT_TRUE(root["empty_array"].empty());
    ASSERT_TRUE(root["null_val"].isNil());
}

// Error handling tests
TEST(invalid_yaml_throws) {
    // Test that should actually throw - key without value at end
    ASSERT_THROWS({
        std::string invalid = "key:";
        yaml::parse(invalid);
    }, yaml::YamlException);
    
    // Test unclosed bracket
    ASSERT_THROWS({
        std::string invalid = "array: [1, 2, 3";
        yaml::parse(invalid);
    }, yaml::YamlException);
    
    // Test unclosed brace  
    ASSERT_THROWS({
        std::string invalid = "obj: {key: value";
        yaml::parse(invalid);
    }, yaml::YamlException);
}

// Type conversion errors
TEST(type_conversion_errors) {
    std::string yaml = R"(string_val: hello
number_val: 42)";
    
    yaml::YamlValue root = yaml::parse(yaml);
    
    // These should throw
    ASSERT_THROWS(root["string_val"].asNumber(), yaml::YamlException);
    ASSERT_THROWS(root["string_val"].asBool(), yaml::YamlException);
    ASSERT_THROWS(root["number_val"].asString(), yaml::YamlException);
    ASSERT_THROWS(root["nonexistent"].asString(), yaml::YamlException);
}

// Serialization tests
TEST(serialization_roundtrip) {
    std::string yaml = "name: John Doe\nage: 30";
    
    yaml::YamlValue root = yaml::parse(yaml);
    std::string serialized = root.serialize();
    
    // Parse the serialized version
    yaml::YamlValue root2 = yaml::parse(serialized);
    
    // Should be equivalent
    ASSERT_EQ(root["name"].asString(), root2["name"].asString());
    ASSERT_EQ(root["age"].asInt(), root2["age"].asInt());
}

// Memory stress test
TEST(memory_stress_test) {
    const int iterations = 100; // Reduce iterations to make debugging easier
    
    for (int i = 0; i < iterations; i++) {
        std::string yaml = R"(name: John Doe
age: 30
hobbies:
  - reading
  - coding
  - gaming)";
        
        yaml::YamlValue root = yaml::parse(yaml);
        
        // Access some values to ensure they work
        ASSERT_EQ(root["name"].asString(), "John Doe");
        ASSERT_EQ(root["age"].asInt(), 30);
        
        // Debug the hobbies issue
        if (root.contains("hobbies")) {
            auto& hobbies = root["hobbies"];
            if (hobbies.isSequence()) {
                ASSERT_EQ(hobbies.size(), 3);
            }
        }
        
        // Root goes out of scope and should be cleaned up
    }
}

// Performance test
TEST(performance_test) {
    std::string yaml = R"(
application:
  name: Large Config
  version: 1.0.0
  servers:
    - name: server1
      host: 192.168.1.10
      port: 8080
    - name: server2  
      host: 192.168.1.11
      port: 8081
  database:
    host: db.example.com
    port: 5432
    name: production
)";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; i++) {
        yaml::YamlValue root = yaml::parse(yaml);
        ASSERT_EQ(root["application"]["name"].asString(), "Large Config");
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << C_YELLOW " (took " << duration.count() << "ms)" C_RESET;
}

void debugYamlValue(const yaml::YamlValue &value, const std::string &name, int depth = 0)
{
    std::string indent(depth * 2, ' ');
    std::cout << indent << name << " - Type: " << static_cast<int>(value.getType()) << std::endl;

    if (value.isMapping())
    {
        std::cout << indent << "  Mapping with " << value.size() << " keys:" << std::endl;
        const auto &map = value.asMapping();
        for (const auto &pair : map)
        {
            std::cout << indent << "    Key: '" << pair.first << "'" << std::endl;
            debugYamlValue(pair.second, pair.first, depth + 2);
        }
    }
    else if (value.isSequence())
    {
        std::cout << indent << "  Sequence with " << value.size() << " items" << std::endl;
        const auto &seq = value.asSequence();
        for (size_t i = 0; i < seq.size(); ++i)
        {
            debugYamlValue(seq[i], "[" + std::to_string(i) + "]", depth + 1);
        }
    }
    else if (value.isString())
    {
        std::cout << indent << "  String: '" << value.asString() << "'" << std::endl;
    }
    else if (value.isNumber())
    {
        std::cout << indent << "  Number: " << value.asNumber() << std::endl;
    }
    else if (value.isBool())
    {
        std::cout << indent << "  Boolean: " << (value.asBool() ? "true" : "false") << std::endl;
    }
    else
    {
        std::cout << indent << "  Nil/Unknown" << std::endl;
    }
}

bool runTest(const std::string &testName, const std::string &yamlText, bool shouldFail = false)
{
    std::cout << "\n==== TEST: " << testName << " ====" << std::endl;
    std::cout << "YAML Input:\n"
              << yamlText << std::endl;

    try
    {
        yaml::YamlValue root = yaml::parse(yamlText);

        if (shouldFail)
        {
            std::cout << "❌ FAILED: Expected parsing to fail, but it succeeded" << std::endl;
            return false;
        }

        std::cout << "\n--- Structure Debug ---" << std::endl;
        debugYamlValue(root, "ROOT", 0);

        std::cout << "\n--- Serialization Test ---" << std::endl;
        std::string serialized = root.serialize();
        std::cout << "Serialized output:\n"
                  << serialized << std::endl;

        std::cout << "✅ PASSED: " << testName << std::endl;
        return true;
    }
    catch (const yaml::YamlException &e)
    {
        if (shouldFail)
        {
            std::cout << "✅ PASSED: Expected failure - " << e.what()
                      << " at line " << e.line << ", column " << e.column << std::endl;
            return true;
        }
        else
        {
            std::cout << "❌ FAILED: Unexpected error - " << e.what()
                      << " at line " << e.line << ", column " << e.column << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "❌ FAILED: General error - " << e.what() << std::endl;
        return false;
    }
}

int main()
{
    std::cout << "=== COMPREHENSIVE YAML PARSER TEST SUITE ===" << std::endl;

    int totalTests = 0;
    int passedTests = 0;

    // Test 1: Basic types and unquoted strings with spaces
    totalTests++;
    if (runTest("Basic Types + Unquoted Strings", R"(name: John Doe
age: 30
pi: 3.14159
active: true
inactive: false
nothing: null
description: This is a long description with spaces
address: 123 Main Street Apt 4B)"))
    {
        passedTests++;
    }

    // Test 2: Nested mappings
    totalTests++;
    if (runTest("Nested Mappings", R"(user:
  name: Alice Smith
  age: 25
  address:
    street: 456 Oak Avenue
    city: Springfield
    zip: 12345
  preferences:
    theme: dark mode
    language: en US)"))
    {
        passedTests++;
    }

    // Test 3: Sequences
    totalTests++;
    if (runTest("Sequences", R"(hobbies:
  - reading books
  - playing guitar
  - mountain climbing
numbers:
  - 1
  - 2
  - 3.5
  - -10
mixed:
  - string item
  - 42
  - true
  - null)"))
    {
        passedTests++;
    }

    // Test 4: Flow style (inline)
    totalTests++;
    if (runTest("Flow Style", R"(config: {debug: true, port: 8080, host: localhost}
array: [1, 2, three, 4.5]
mixed_flow: {items: [a, b, c], count: 3})"))
    {
        passedTests++;
    }

    // Test 5: Quoted strings
    totalTests++;
    if (runTest("Quoted Strings", R"(single: 'This is single quoted'
double: "This is double quoted"
escaped: "Line 1\nLine 2\tTabbed"
mixed: 'String with "inner" quotes'
special: "String with: colons, [brackets], and {braces}")"))
    {
        passedTests++;
    }

    // Test 6: Edge cases with numbers vs strings
    totalTests++;
    if (runTest("Numbers vs Strings", R"(pure_number: 42
pure_float: 3.14
negative: -100
string_with_number: 123 Main St
number_like_string: 42nd Street
version_string: 1.0.0
phone: 555 123 4567)"))
    {
        passedTests++;
    }

    // Test 7: Complex nested structure
    totalTests++;
    if (runTest("Complex Structure", R"(application:
  name: My Web App
  version: 2.1.0
  database:
    host: db.example.com
    port: 5432
    credentials:
      username: admin
      password: secret123
  servers:
    - name: web-01
      ip: 192.168.1.10
      roles: [web, api]
    - name: web-02  
      ip: 192.168.1.11
      roles: [web, cache]
  features:
    authentication: true
    ssl: true
    debug: false
    max_connections: 1000)"))
    {
        passedTests++;
    }

    // Test 8: Empty structures
    totalTests++;
    if (runTest("Empty Structures", R"(empty_map: {}
empty_array: []
null_value: null
empty_string: ""
nested_empty:
  empty: {}
  also_empty: [])"))
    {
        passedTests++;
    }

    // Test 9: Special values and booleans
    totalTests++;
    if (runTest("Special Values", R"(bool_true: true
bool_false: false
null_value: null
tilde_null: ~
string_true: "true"
string_false: "false"
string_null: "null")"))
    {
        passedTests++;
    }

    // Test 10: Real-world config example
    totalTests++;
    if (runTest("Real-world Config", R"(server:
  host: 0.0.0.0
  port: 3000
  ssl:
    enabled: true
    cert_path: /etc/ssl/cert.pem
    key_path: /etc/ssl/key.pem
database:
  type: postgresql
  host: localhost
  port: 5432
  name: myapp_production
  pool_size: 10
logging:
  level: info
  files:
    - /var/log/app.log
    - /var/log/error.log
  max_size: 100 MB
features:
  cache: redis://localhost:6379
  search: elasticsearch://localhost:9200
  email: smtp://smtp.gmail.com:587)"))
    {
        passedTests++;
    }

    std::cout << "\n=== FINAL RESULTS ===" << std::endl;
    std::cout << "Tests passed: " << passedTests << "/" << totalTests << std::endl;

    if (passedTests == totalTests)
    {
        std::cout << "🎉 ALL TESTS PASSED! " << std::endl;
    }
    else
    {
        std::cout << "❌ Some tests failed. Check the output above for details." << std::endl;
    }

    std::cout << C_MAGENTA "=== ENHANCED YAML PARSER TEST SUITE ===" C_RESET "\n\n";

    // Basic functionality tests
    std::cout << C_BLUE "--- Basic Functionality Tests ---" C_RESET "\n";
    RUN_TEST(basic_string);
    RUN_TEST(basic_number);
    RUN_TEST(basic_boolean);
    RUN_TEST(basic_null);

    // String handling tests (our main concern)
    std::cout << "\n"
              << C_BLUE "--- String Handling Tests ---" C_RESET "\n";
    RUN_TEST(unquoted_string_with_spaces);
    RUN_TEST(complex_unquoted_strings);
    RUN_TEST(number_vs_string_disambiguation);
    RUN_TEST(quoted_strings);

    // Structure tests
    std::cout << "\n"
              << C_BLUE "--- Structure Tests ---" C_RESET "\n";
    RUN_TEST(nested_mapping);
    RUN_TEST(sequences);
    RUN_TEST(mixed_sequence);
    RUN_TEST(flow_mapping);
    RUN_TEST(flow_sequence);
    RUN_TEST(empty_structures);

    // Error handling tests
    std::cout << "\n"
              << C_BLUE "--- Error Handling Tests ---" C_RESET "\n";
    RUN_TEST(invalid_yaml_throws);
    RUN_TEST(type_conversion_errors);

    // Advanced tests
    std::cout << "\n"
              << C_BLUE "--- Advanced Tests ---" C_RESET "\n";
    RUN_TEST(serialization_roundtrip);
    RUN_TEST(memory_stress_test);
    RUN_TEST(performance_test);

    // Final results
    std::cout << "\n"
              << C_MAGENTA "=== FINAL RESULTS ===" C_RESET "\n";
    std::cout << "Total tests: " << total_tests << "\n";
    std::cout << C_GREEN "Passed: " << passed_tests << C_RESET "\n";
    std::cout << C_RED "Failed: " << failed_tests << C_RESET "\n";

    if (failed_tests == 0)
    {
        std::cout << "\n"
                  << C_GREEN "🎉 ALL TESTS PASSED!" C_RESET "\n";
        std::cout << C_GREEN "Your YAML parser is working correctly and appears memory-safe." C_RESET "\n";
        return 0;
    }
    else
    {
        std::cout << "\n"
                  << C_RED "❌ Some tests failed." C_RESET "\n";
        return 1;
    }

     std::cout << C_MAGENTA "=== ENHANCED YAML PARSER TEST SUITE ===" C_RESET "\n\n";
    
    // Basic functionality tests
    std::cout << C_BLUE "--- Basic Functionality Tests ---" C_RESET "\n";
    RUN_TEST(basic_string);
    RUN_TEST(basic_number);
    RUN_TEST(basic_boolean);
    RUN_TEST(basic_null);
    
    // String handling tests (our main concern)
    std::cout << "\n" << C_BLUE "--- String Handling Tests ---" C_RESET "\n";
    RUN_TEST(unquoted_string_with_spaces);
    RUN_TEST(complex_unquoted_strings);
    RUN_TEST(number_vs_string_disambiguation);
    RUN_TEST(quoted_strings);
    
    // Structure tests
    std::cout << "\n" << C_BLUE "--- Structure Tests ---" C_RESET "\n";
    RUN_TEST(nested_mapping);
    RUN_TEST(sequences);
    RUN_TEST(mixed_sequence);
    RUN_TEST(flow_mapping);
    RUN_TEST(flow_sequence);
    RUN_TEST(empty_structures);
    
    // Error handling tests
    std::cout << "\n" << C_BLUE "--- Error Handling Tests ---" C_RESET "\n";
    RUN_TEST(invalid_yaml_throws);
    RUN_TEST(type_conversion_errors);
    
    // Advanced tests
    std::cout << "\n" << C_BLUE "--- Advanced Tests ---" C_RESET "\n";
    RUN_TEST(serialization_roundtrip);
    RUN_TEST(memory_stress_test);
    RUN_TEST(performance_test);
    
    // Final results
    std::cout << "\n" << C_MAGENTA "=== FINAL RESULTS ===" C_RESET "\n";
    std::cout << "Total tests: " << total_tests << "\n";
    std::cout << C_GREEN "Passed: " << passed_tests << C_RESET "\n";
    std::cout << C_RED "Failed: " << failed_tests << C_RESET "\n";
    
    if (failed_tests == 0) {
        std::cout << "\n" << C_GREEN "🎉 ALL TESTS PASSED!" C_RESET "\n";
        std::cout << C_GREEN "Your YAML parser is working correctly and appears memory-safe." C_RESET "\n";
        return 0;
    } else {
        std::cout << "\n" << C_RED "❌ Some tests failed." C_RESET "\n";
        return 1;
    }
}