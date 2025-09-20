
#include <chrono>
#include <sstream>
#include <cassert>
#define YAML_IMPLEMENTATION
#include "yaml_single.hpp"

 

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

    return 0;

     
}