#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cctype>

namespace yaml
{

    class YamlException : public std::runtime_error
    {
    public:
        int line;
        int column;

        YamlException(const std::string &msg, int ln = 0, int col = 0)
            : std::runtime_error(msg), line(ln), column(col) {}
    };

    enum class YamlType
    {
        NIL,
        BOOLEAN,
        NUMBER,
        STRING,
        SEQUENCE,
        MAPPING
    };

    class YamlValue
    {
    public:
        struct Mapping : public std::map<std::string, YamlValue>
        {
        };
        struct Sequence : public std::vector<YamlValue>
        {
        };

    private:
        YamlType type_;
        union
        {
            bool boolValue_;
            double numberValue_;
            std::string *stringValue_;
            Sequence *sequenceValue_;
            Mapping *mappingValue_;
        };

    public:
        // Constructors
        YamlValue();
        YamlValue(bool value);
        YamlValue(int value);
        YamlValue(double value);
        YamlValue(const std::string &value);
        YamlValue(const char *value);
        YamlValue(const Sequence &seq);
        YamlValue(const Mapping &map);

        // Copy constructor and assignment
        YamlValue(const YamlValue &other);
        YamlValue &operator=(const YamlValue &other);

        // Move constructor and assignment
        YamlValue(YamlValue &&other) noexcept;
        YamlValue &operator=(YamlValue &&other) noexcept;

        ~YamlValue();

        // Type checking
        YamlType getType() const { return type_; }
        bool isNil() const { return type_ == YamlType::NIL; }
        bool isBool() const { return type_ == YamlType::BOOLEAN; }
        bool isNumber() const { return type_ == YamlType::NUMBER; }
        bool isString() const { return type_ == YamlType::STRING; }
        bool isSequence() const { return type_ == YamlType::SEQUENCE; }
        bool isMapping() const { return type_ == YamlType::MAPPING; }

        // Value access
        bool asBool() const;
        double asNumber() const;
        int asInt() const;
        const std::string &asString() const;
        Sequence &asSequence();
        const Sequence &asSequence() const;
        Mapping &asMapping();
        const Mapping &asMapping() const;

        // Template getter
        template <typename T>
        T get() const;

        // Convenience methods
        size_t size() const;
        bool empty() const;
        bool contains(const std::string &key) const;
        void clear();

        void trace() const;

        // Convenience operators
        YamlValue &operator[](const std::string &key);
        const YamlValue &operator[](const std::string &key) const;
        YamlValue &operator[](size_t index);
        const YamlValue &operator[](size_t index) const;

        // Serialization
        std::string serialize(int indent = 0) const;

        // Comparison
        bool operator==(const YamlValue &other) const;
        bool operator!=(const YamlValue &other) const { return !(*this == other); }

    private:
        void cleanup();
        void copyFrom(const YamlValue &other);
        void moveFrom(YamlValue &other);
        std::string serializeValue(int indent, bool inArray = false) const;
    };

    // Token types - Fixed enum
    enum class TokenType
    {
        TOKEN_STRING,
        TOKEN_NUMBER,
        TOKEN_BOOLEAN,
        TOKEN_NULL,
        TOKEN_COLON,
        TOKEN_DASH,
        TOKEN_COMMA,
        TOKEN_NEWLINE,
        TOKEN_LBRACKET,
        TOKEN_RBRACKET,
        TOKEN_LBRACE,
        TOKEN_RBRACE,
        TOKEN_PIPE,
        TOKEN_FOLD,
        TOKEN_ANCHOR,
        TOKEN_ALIAS,
        TOKEN_EOF,
        TOKEN_ERROR,
        TOKEN_INDENT,
        TOKEN_DEDENT
    };

    struct Token
    {
        TokenType type;
        std::string value;
        int line;
        int column;
        int indent;

        Token();
        Token(TokenType t, const std::string &val = "", int ln = 0, int col = 0, int ind = 0);
    };

    class Scanner
    {
    public:
        explicit Scanner(const std::string &src);
        Token next();

    private:
        const std::string &s_;
        size_t cur_;
        int line_;
        int col_;
        bool bol_;
        std::vector<int> indents_;
        std::vector<Token> pending_;

        // helpers
        bool isAtEnd_() const;
        char peek_() const;
        char peekNext_() const;
        char advance_();
        void skipToEOL_();
        int measureIndent_();
        void emitIndentChange_(int spaces);
        static bool isSpace_(char c);
        static bool isDigit_(char c);
        static bool isAlpha_(char c);
        static bool isAlnum_(char c);
        Token make_(TokenType t, const std::string &v = "");

        static int countSpaces_(const std::string &s, size_t pos, size_t &outPos);
    };

    class Parser
    {
    public:
        explicit Parser(const std::string &src);
        YamlValue parse();

    private:
        Scanner sc_;
        Token cur_, nxt_;

        void advance_();
        bool match_(TokenType t);
        void expect_(TokenType t, const char *msg);

        YamlValue parseValue_();
        YamlValue parseFlowSeq_();
        YamlValue parseFlowMap_();
        YamlValue parseScalar_();
        YamlValue parseMapping_();
        YamlValue parseSequence_();
    };

    inline YamlValue parse(const std::string &s) { return Parser(s).parse(); }

    // Template specializations for get<T>
    template <>
    inline bool YamlValue::get<bool>() const { return asBool(); }
    template <>
    inline int YamlValue::get<int>() const { return asInt(); }
    template <>
    inline double YamlValue::get<double>() const { return asNumber(); }
    template <>
    inline std::string YamlValue::get<std::string>() const { return asString(); }

} // namespace yaml

// //------------------------------------------------------------------------------------
// // Implementation section
// //------------------------------------------------------------------------------------
// #ifdef YAML_IMPLEMENTATION

// #include <fstream>
// #include <iomanip>

// #endif // YAML_IMPLEMENTATION