#include "yaml.hpp"
#include <iomanip>

namespace yaml
{

    // ============================================================================
    // YamlValue Implementation
    // ============================================================================

    YamlValue::YamlValue() : type_(YamlType::NIL) {}

    YamlValue::YamlValue(bool value) : type_(YamlType::BOOLEAN), boolValue_(value) {}

    YamlValue::YamlValue(int value) : type_(YamlType::NUMBER), numberValue_(static_cast<double>(value)) {}

    YamlValue::YamlValue(double value) : type_(YamlType::NUMBER), numberValue_(value) {}

    YamlValue::YamlValue(const std::string &value) : type_(YamlType::STRING)
    {
        stringValue_ = new std::string(value);
    }

    YamlValue::YamlValue(const char *value) : type_(YamlType::STRING)
    {
        stringValue_ = new std::string(value);
    }

    YamlValue::YamlValue(const Sequence &seq) : type_(YamlType::SEQUENCE)
    {
        sequenceValue_ = new Sequence(seq);
    }

    YamlValue::YamlValue(const Mapping &map) : type_(YamlType::MAPPING)
    {
        mappingValue_ = new Mapping(map);
    }

    YamlValue::YamlValue(const YamlValue &other)
    {
        copyFrom(other);
    }

    YamlValue &YamlValue::operator=(const YamlValue &other)
    {
        if (this != &other)
        {
            cleanup();
            copyFrom(other);
        }
        return *this;
    }

    YamlValue::YamlValue(YamlValue &&other) noexcept
    {
        moveFrom(other);
    }

    YamlValue &YamlValue::operator=(YamlValue &&other) noexcept
    {
        if (this != &other)
        {
            cleanup();
            moveFrom(other);
        }
        return *this;
    }

    YamlValue::~YamlValue()
    {
        cleanup();
    }

    void YamlValue::cleanup()
    {
        switch (type_)
        {
        case YamlType::STRING:
            delete stringValue_;
            break;
        case YamlType::SEQUENCE:
            delete sequenceValue_;
            break;
        case YamlType::MAPPING:
            delete mappingValue_;
            break;
        default:
            break;
        }
    }

    void YamlValue::copyFrom(const YamlValue &other)
    {
        type_ = other.type_;
        switch (type_)
        {
        case YamlType::NIL:
            break;
        case YamlType::BOOLEAN:
            boolValue_ = other.boolValue_;
            break;
        case YamlType::NUMBER:
            numberValue_ = other.numberValue_;
            break;
        case YamlType::STRING:
            stringValue_ = new std::string(*other.stringValue_);
            break;
        case YamlType::SEQUENCE:
            sequenceValue_ = new Sequence(*other.sequenceValue_);
            break;
        case YamlType::MAPPING:
            mappingValue_ = new Mapping(*other.mappingValue_);
            break;
        }
    }

    void YamlValue::moveFrom(YamlValue &other)
    {
        type_ = other.type_;
        switch (type_)
        {
        case YamlType::NIL:
            break;
        case YamlType::BOOLEAN:
            boolValue_ = other.boolValue_;
            break;
        case YamlType::NUMBER:
            numberValue_ = other.numberValue_;
            break;
        case YamlType::STRING:
            stringValue_ = other.stringValue_;
            other.stringValue_ = nullptr;
            break;
        case YamlType::SEQUENCE:
            sequenceValue_ = other.sequenceValue_;
            other.sequenceValue_ = nullptr;
            break;
        case YamlType::MAPPING:
            mappingValue_ = other.mappingValue_;
            other.mappingValue_ = nullptr;
            break;
        }
        other.type_ = YamlType::NIL;
    }

    bool YamlValue::asBool() const
    {
        if (type_ != YamlType::BOOLEAN)
        {
            throw YamlException("Value is not a boolean");
        }
        return boolValue_;
    }

    double YamlValue::asNumber() const
    {
        if (type_ != YamlType::NUMBER)
        {
            throw YamlException("Value is not a number");
        }
        return numberValue_;
    }

    int YamlValue::asInt() const
    {
        return static_cast<int>(asNumber());
    }

    const std::string &YamlValue::asString() const
    {
        if (type_ != YamlType::STRING)
        {
            throw YamlException("Value is not a string");
        }
        return *stringValue_;
    }

    YamlValue::Sequence &YamlValue::asSequence()
    {
        if (type_ != YamlType::SEQUENCE)
        {
            throw YamlException("Value is not a sequence");
        }
        return *sequenceValue_;
    }

    const YamlValue::Sequence &YamlValue::asSequence() const
    {
        if (type_ != YamlType::SEQUENCE)
        {
            throw YamlException("Value is not a sequence");
        }
        return *sequenceValue_;
    }

    YamlValue::Mapping &YamlValue::asMapping()
    {
        if (type_ != YamlType::MAPPING)
        {
            throw YamlException("Value is not a mapping");
        }
        return *mappingValue_;
    }

    const YamlValue::Mapping &YamlValue::asMapping() const
    {
        if (type_ != YamlType::MAPPING)
        {
            throw YamlException("Value is not a mapping");
        }
        return *mappingValue_;
    }

    size_t YamlValue::size() const
    {
        switch (type_)
        {
        case YamlType::SEQUENCE:
            return sequenceValue_->size();
        case YamlType::MAPPING:
            return mappingValue_->size();
        case YamlType::STRING:
            return stringValue_->size();
        default:
            return 0;
        }
    }

    bool YamlValue::empty() const
    {
        return size() == 0;
    }

    bool YamlValue::contains(const std::string &key) const
    {
        if (type_ != YamlType::MAPPING)
        {
            return false;
        }
        return mappingValue_->find(key) != mappingValue_->end();
    }

    void YamlValue::clear()
    {
        cleanup();
        type_ = YamlType::NIL;
    }

    YamlValue &YamlValue::operator[](const std::string &key)
    {
        if (type_ == YamlType::NIL)
        {
            type_ = YamlType::MAPPING;
            mappingValue_ = new Mapping();
        }
        if (type_ != YamlType::MAPPING)
        {
            throw YamlException("Value is not a mapping");
        }
        return (*mappingValue_)[key];
    }

    const YamlValue &YamlValue::operator[](const std::string &key) const
    {
        if (type_ != YamlType::MAPPING)
        {
            throw YamlException("Value is not a mapping");
        }
        auto it = mappingValue_->find(key);
        if (it == mappingValue_->end())
        {
            throw YamlException("Key not found: " + key);
        }
        return it->second;
    }

    YamlValue &YamlValue::operator[](size_t index)
    {
        if (type_ == YamlType::NIL)
        {
            type_ = YamlType::SEQUENCE;
            sequenceValue_ = new Sequence();
        }
        if (type_ != YamlType::SEQUENCE)
        {
            throw YamlException("Value is not a sequence");
        }
        if (index >= sequenceValue_->size())
        {
            sequenceValue_->resize(index + 1);
        }
        return (*sequenceValue_)[index];
    }

    const YamlValue &YamlValue::operator[](size_t index) const
    {
        if (type_ != YamlType::SEQUENCE)
        {
            throw YamlException("Value is not a sequence");
        }
        if (index >= sequenceValue_->size())
        {
            throw YamlException("Index out of bounds");
        }
        return (*sequenceValue_)[index];
    }

    std::string YamlValue::serialize(int indent) const
    {
        return serializeValue(indent);
    }

    std::string YamlValue::serializeValue(int indent, bool inArray) const
    {
        std::ostringstream oss;

        switch (type_)
        {
        case YamlType::NIL:
            oss << "null";
            break;

        case YamlType::BOOLEAN:
            oss << (boolValue_ ? "true" : "false");
            break;

        case YamlType::NUMBER:
            if (numberValue_ == static_cast<int>(numberValue_))
            {
                oss << static_cast<int>(numberValue_);
            }
            else
            {
                oss << numberValue_;
            }
            break;

        case YamlType::STRING:
        {
            const std::string &str = *stringValue_;
            bool needsQuotes = str.empty() ||
                               str.find('\n') != std::string::npos ||
                               str.find(':') != std::string::npos ||
                               str.find('#') != std::string::npos ||
                               str.find('[') != std::string::npos ||
                               str.find(']') != std::string::npos ||
                               str.find('{') != std::string::npos ||
                               str.find('}') != std::string::npos ||
                               (!str.empty() && (str.front() == ' ' || str.back() == ' '));

            if (needsQuotes)
            {
                oss << "\"";
                for (char c : str)
                {
                    switch (c)
                    {
                    case '"':
                        oss << "\\\"";
                        break;
                    case '\\':
                        oss << "\\\\";
                        break;
                    case '\n':
                        oss << "\\n";
                        break;
                    case '\r':
                        oss << "\\r";
                        break;
                    case '\t':
                        oss << "\\t";
                        break;
                    default:
                        oss << c;
                        break;
                    }
                }
                oss << "\"";
            }
            else
            {
                oss << str;
            }
            break;
        }

        case YamlType::SEQUENCE:
        {
            if (sequenceValue_->empty())
            {
                oss << "[]";
            }
            else
            {
                for (size_t i = 0; i < sequenceValue_->size(); ++i)
                {
                    if (i > 0)
                        oss << "\n";
                    oss << std::string(indent, ' ') << "- ";
                    std::string itemStr = (*sequenceValue_)[i].serializeValue(indent + 2, true);
                    oss << itemStr;
                }
            }
            break;
        }

        case YamlType::MAPPING:
        {
            if (mappingValue_->empty())
            {
                oss << "{}";
            }
            else
            {
                bool first = true;
                for (const auto &pair : *mappingValue_)
                {
                    if (!first)
                        oss << "\n";
                    first = false;

                    if (!inArray)
                    {
                        oss << std::string(indent, ' ');
                    }
                    oss << pair.first << ": ";

                    std::string valueStr = pair.second.serializeValue(indent + 2);
                    if (pair.second.isMapping() && !pair.second.empty())
                    {
                        oss << "\n"
                            << std::string(indent + 2, ' ') << valueStr;
                    }
                    else if (pair.second.isSequence() && !pair.second.empty())
                    {
                        oss << "\n"
                            << valueStr;
                    }
                    else
                    {
                        oss << valueStr;
                    }
                }
            }
            break;
        }
        }

        return oss.str();
    }

    void YamlValue::trace() const
    {
        std::cout << "YamlValue Trace:" << std::endl;
        std::cout << "Type: " << static_cast<int>(type_) << std::endl;
        std::cout << "Size: " << size() << std::endl;
        if (type_ == YamlType::STRING)
        {
            std::cout << "String Value: " << *stringValue_ << std::endl;
        }
        else if (type_ == YamlType::NUMBER)
        {
            std::cout << "Number Value: " << numberValue_ << std::endl;
        }
        else if (type_ == YamlType::BOOLEAN)
        {
            std::cout << "Boolean Value: " << (boolValue_ ? "true" : "false") << std::endl;
        }
        else if (type_ == YamlType::SEQUENCE)
        {
            std::cout << "Sequence Values:" << std::endl;
            for (size_t i = 0; i < sequenceValue_->size(); ++i)
            {
                std::cout << "  [" << i << "] ";
                (*sequenceValue_)[i].trace();
            }
        }
        else if (type_ == YamlType::MAPPING)
        {
            std::cout << "Mapping Values:" << std::endl;
            for (const auto &pair : *mappingValue_)
            {
                std::cout << "  Key: " << pair.first << std::endl;
                pair.second.trace();
            }
        }
        else
        {
            std::cout << "Nil Value" << std::endl;
        }
    }

    bool YamlValue::operator==(const YamlValue &other) const
    {
        if (type_ != other.type_)
            return false;

        switch (type_)
        {
        case YamlType::NIL:
            return true;
        case YamlType::BOOLEAN:
            return boolValue_ == other.boolValue_;
        case YamlType::NUMBER:
            return numberValue_ == other.numberValue_;
        case YamlType::STRING:
            return *stringValue_ == *other.stringValue_;
        case YamlType::SEQUENCE:
            return *sequenceValue_ == *other.sequenceValue_;
        case YamlType::MAPPING:
            return *mappingValue_ == *other.mappingValue_;
        }
        return false;
    }

    // ============================================================================
    // Token Implementation
    // ============================================================================

    Token::Token() : type(TokenType::TOKEN_EOF), line(0), column(0), indent(0) {}

    Token::Token(TokenType t, const std::string &val, int ln, int col, int ind)
        : type(t), value(val), line(ln), column(col), indent(ind) {}

    // ============================================================================
    // Scanner Implementation
    // ============================================================================

    Scanner::Scanner(const std::string &src)
        : s_(src), cur_(0), line_(1), col_(1), bol_(true)
    {
        indents_.push_back(0); // Base indentation level
    }

    Token Scanner::next()
    {
        if (!pending_.empty())
        {
            Token t = pending_.front();
            pending_.erase(pending_.begin());
            return t;
        }

        while (!isAtEnd_())
        {
            if (bol_)
            {
                int spaces = measureIndent_();
                if (spaces >= 0)
                {
                    emitIndentChange_(spaces);
                    bol_ = false;

                    // Se temos tokens pendentes, retornar o primeiro
                    if (!pending_.empty())
                    {
                        Token t = pending_.front();
                        pending_.erase(pending_.begin());
                        return t;
                    }
                }
                else
                {
                    bol_ = false; // Linha vazia/comentário, continuar
                }
            }

            char c = peek_();

            // Skip whitespace (except newlines)
            if (isSpace_(c) && c != '\n')
            {
                advance_();
                continue;
            }

            // Comments
            if (c == '#')
            {
                skipToEOL_();
                continue;
            }

            // Newlines
            if (c == '\n')
            {
                advance_();
                bol_ = true;
                return make_(TokenType::TOKEN_NEWLINE);
            }

            // Structural characters
            switch (c)
            {
            case ':':
                advance_();
                return make_(TokenType::TOKEN_COLON);
            case '-':
                if (peekNext_() == ' ' || peekNext_() == '\n' || peekNext_() == '\0')
                {
                    advance_();
                    return make_(TokenType::TOKEN_DASH);
                }
                // Fall through to string parsing
                break;
            case '[':
                advance_();
                return make_(TokenType::TOKEN_LBRACKET);
            case ']':
                advance_();
                return make_(TokenType::TOKEN_RBRACKET);
            case '{':
                advance_();
                return make_(TokenType::TOKEN_LBRACE);
            case '}':
                advance_();
                return make_(TokenType::TOKEN_RBRACE);
            case ',':
                advance_();
                return make_(TokenType::TOKEN_COMMA);
            }

            // Quoted strings
            if (c == '"' || c == '\'')
            {
                char quote = advance_();
                std::string str;
                while (!isAtEnd_() && peek_() != quote)
                {
                    if (peek_() == '\\')
                    {
                        advance_(); // Skip backslash
                        if (!isAtEnd_())
                        {
                            char escaped = advance_();
                            switch (escaped)
                            {
                            case 'n':
                                str += '\n';
                                break;
                            case 't':
                                str += '\t';
                                break;
                            case 'r':
                                str += '\r';
                                break;
                            case '\\':
                                str += '\\';
                                break;
                            case '"':
                                str += '"';
                                break;
                            case '\'':
                                str += '\'';
                                break;
                            default:
                                str += escaped;
                                break;
                            }
                        }
                    }
                    else
                    {
                        str += advance_();
                    }
                }
                if (!isAtEnd_())
                    advance_(); // Skip closing quote
                return make_(TokenType::TOKEN_STRING, str);
            }

            // Everything else: read until delimiter
            std::string str;

            // Read until we hit a structural character
            while (!isAtEnd_())
            {
                char ch = peek_();

                // Stop at YAML structural characters
                if (ch == ':' || ch == '\n' || ch == '#' ||
                    ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == ',')
                {
                    break;
                }

                // Stop at dash if it's a list item marker (dash followed by space)
                if (ch == '-' && (cur_ + 1 >= s_.size() || s_[cur_ + 1] == ' ' || s_[cur_ + 1] == '\n'))
                {
                    break;
                }

                str += advance_();
            }

            // Trim trailing spaces
            while (!str.empty() && str.back() == ' ')
            {
                str.pop_back();
            }

            if (!str.empty())
            {
                // Check for special values
                if (str == "true" || str == "false")
                {
                    return make_(TokenType::TOKEN_BOOLEAN, str);
                }
                if (str == "null" || str == "~")
                {
                    return make_(TokenType::TOKEN_NULL, str);
                }

                // Check if it's a pure number (improved validation)
                bool isNumber = true;
                size_t pos = 0;
                bool hasDecimal = false;

                // Skip optional negative sign
                if (pos < str.size() && str[pos] == '-')
                    pos++;

                // Must have at least one digit
                if (pos >= str.size() || !isDigit_(str[pos]))
                {
                    isNumber = false;
                }
                else
                {
                    // Read digits and optional single decimal point
                    while (pos < str.size() && isDigit_(str[pos]))
                        pos++;

                    if (pos < str.size() && str[pos] == '.' && !hasDecimal)
                    {
                        hasDecimal = true;
                        pos++;
                        // Must have digits after decimal point
                        if (pos >= str.size() || !isDigit_(str[pos]))
                        {
                            isNumber = false;
                        }
                        else
                        {
                            while (pos < str.size() && isDigit_(str[pos]))
                                pos++;
                        }
                    }

                    // Should have consumed entire string, no extra characters allowed
                    if (pos != str.size())
                        isNumber = false;
                }

                if (isNumber)
                {
                    return make_(TokenType::TOKEN_NUMBER, str);
                }
                else
                {
                    return make_(TokenType::TOKEN_STRING, str);
                }
            }

            // Skip unknown single characters
            advance_();
        }

        // Handle final dedents
        while (indents_.size() > 1)
        {
            indents_.pop_back();
            pending_.push_back(make_(TokenType::TOKEN_DEDENT));
        }

        if (!pending_.empty())
        {
            Token t = pending_.front();
            pending_.erase(pending_.begin());
            return t;
        }

        return make_(TokenType::TOKEN_EOF);
    }

    bool Scanner::isAtEnd_() const
    {
        return cur_ >= s_.size();
    }

    char Scanner::peek_() const
    {
        if (isAtEnd_())
            return '\0';
        return s_[cur_];
    }

    char Scanner::peekNext_() const
    {
        if (cur_ + 1 >= s_.size())
            return '\0';
        return s_[cur_ + 1];
    }

    char Scanner::advance_()
    {
        if (isAtEnd_())
            return '\0';
        char c = s_[cur_++];
        if (c == '\n')
        {
            line_++;
            col_ = 1;
        }
        else
        {
            col_++;
        }
        return c;
    }

    void Scanner::skipToEOL_()
    {
        while (!isAtEnd_() && peek_() != '\n')
        {
            advance_();
        }
    }

    int Scanner::measureIndent_()
    {
        size_t start = cur_;
        int spaces = 0;

        while (cur_ < s_.size() && (s_[cur_] == ' ' || s_[cur_] == '\t'))
        {
            if (s_[cur_] == ' ')
            {
                spaces++;
            }
            else
            {
                spaces += 8; // Tab = 8 spaces
            }
            cur_++;
            col_++;
        }

        // Check if line is empty or comment
        if (cur_ >= s_.size() || s_[cur_] == '\n' || s_[cur_] == '#')
        {
            cur_ = start;   // Reset position
            col_ -= spaces; // Reset column
            return -1;      // Signal empty/comment line
        }

        return spaces;
    }

    void Scanner::emitIndentChange_(int spaces)
    {
        int currentIndent = indents_.back();

        if (spaces > currentIndent)
        {
            indents_.push_back(spaces);
            pending_.push_back(make_(TokenType::TOKEN_INDENT));
        }
        else if (spaces < currentIndent)
        {
            while (indents_.size() > 1 && indents_.back() > spaces)
            {
                indents_.pop_back();
                pending_.push_back(make_(TokenType::TOKEN_DEDENT));
            }
            // Verificar se o nível de indentação é válido
            if (indents_.back() != spaces)
            {
                throw YamlException("Invalid indentation level", line_, col_);
            }
        }
    }

    bool Scanner::isSpace_(char c)
    {
        return c == ' ' || c == '\t' || c == '\r';
    }

    bool Scanner::isDigit_(char c)
    {
        return c >= '0' && c <= '9';
    }

    bool Scanner::isAlpha_(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    bool Scanner::isAlnum_(char c)
    {
        return isAlpha_(c) || isDigit_(c);
    }

    Token Scanner::make_(TokenType t, const std::string &v)
    {
        return Token(t, v, line_, col_, 0);
    }

    int Scanner::countSpaces_(const std::string &s, size_t pos, size_t &outPos)
    {
        int spaces = 0;
        while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t'))
        {
            spaces += (s[pos] == ' ') ? 1 : 8;
            pos++;
        }
        outPos = pos;
        return spaces;
    }

    // ============================================================================
    // Parser Implementation
    // ============================================================================

    Parser::Parser(const std::string &src) : sc_(src)
    {
        advance_(); // Load first token
        advance_(); // Load second token (lookahead)

        // Debug: imprimir os primeiros tokens
        /*
        std::cout << "First token: " << static_cast<int>(cur_.type) << " = '" << cur_.value << "'" << std::endl;
        std::cout << "Second token: " << static_cast<int>(nxt_.type) << " = '" << nxt_.value << "'" << std::endl;
        */
    }

    YamlValue Parser::parse()
    {
        while (cur_.type == TokenType::TOKEN_NEWLINE || cur_.type == TokenType::TOKEN_INDENT)
        {
            advance_();
        }

        if (cur_.type == TokenType::TOKEN_EOF)
        {
            return YamlValue(); // Documento vazio
        }

        return parseValue_();
    }

    void Parser::advance_()
    {
        cur_ = nxt_;
        nxt_ = sc_.next();
    }

    bool Parser::match_(TokenType t)
    {
        if (cur_.type == t)
        {
            advance_();
            return true;
        }
        return false;
    }

    void Parser::expect_(TokenType t, const char *msg)
    {
        if (!match_(t))
        {
            throw YamlException(msg, cur_.line, cur_.column);
        }
    }

    YamlValue Parser::parseValue_()
    {
        // Skip newlines
        while (cur_.type == TokenType::TOKEN_NEWLINE)
        {
            advance_();
        }

        switch (cur_.type)
        {
        case TokenType::TOKEN_LBRACE:
            return parseFlowMap_();
        case TokenType::TOKEN_LBRACKET:
            return parseFlowSeq_();
        case TokenType::TOKEN_DASH:
            return parseSequence_();
        case TokenType::TOKEN_INDENT:
            advance_(); // consume indent
            return parseValue_();
        default:
            if (cur_.type == TokenType::TOKEN_STRING && nxt_.type == TokenType::TOKEN_COLON)
            {
                return parseMapping_();
            }
            return parseScalar_();
        }
    }

    YamlValue Parser::parseFlowSeq_()
    {
        expect_(TokenType::TOKEN_LBRACKET, "Expected '['");

        YamlValue::Sequence seq;

        while (cur_.type != TokenType::TOKEN_RBRACKET && cur_.type != TokenType::TOKEN_EOF)
        {
            seq.push_back(parseValue_());

            if (cur_.type == TokenType::TOKEN_COMMA)
            {
                advance_();
            }
            else if (cur_.type != TokenType::TOKEN_RBRACKET)
            {
                break;
            }
        }

        expect_(TokenType::TOKEN_RBRACKET, "Expected ']'");
        return YamlValue(seq);
    }

    YamlValue Parser::parseFlowMap_()
    {
        expect_(TokenType::TOKEN_LBRACE, "Expected '{'");

        YamlValue::Mapping map;

        while (cur_.type != TokenType::TOKEN_RBRACE && cur_.type != TokenType::TOKEN_EOF)
        {
            if (cur_.type != TokenType::TOKEN_STRING)
            {
                throw YamlException("Expected string key in mapping", cur_.line, cur_.column);
            }

            std::string key = cur_.value;
            advance_();

            expect_(TokenType::TOKEN_COLON, "Expected ':' after key");

            map[key] = parseValue_();

            if (cur_.type == TokenType::TOKEN_COMMA)
            {
                advance_();
            }
            else if (cur_.type != TokenType::TOKEN_RBRACE)
            {
                break;
            }
        }

        expect_(TokenType::TOKEN_RBRACE, "Expected '}'");
        return YamlValue(map);
    }

    YamlValue Parser::parseScalar_()
    {
        switch (cur_.type)
        {
        case TokenType::TOKEN_NULL:
            advance_();
            return YamlValue();
        case TokenType::TOKEN_BOOLEAN:
        {
            bool val = (cur_.value == "true");
            advance_();
            return YamlValue(val);
        }
        case TokenType::TOKEN_NUMBER:
        {
            double val = std::stod(cur_.value);
            advance_();
            return YamlValue(val);
        }
        case TokenType::TOKEN_STRING:
        {
            std::string val = cur_.value;
            advance_();
            return YamlValue(val);
        }
        case TokenType::TOKEN_DEDENT:
            // Se chegamos aqui com DEDENT, significa que não há valor
            throw YamlException("Missing value after key", cur_.line, cur_.column);
        default:
            throw YamlException("Expected scalar value", cur_.line, cur_.column);
        }
    }
    YamlValue Parser::parseMapping_()
    {
        YamlValue::Mapping map;

        while (cur_.type == TokenType::TOKEN_STRING && nxt_.type == TokenType::TOKEN_COLON)
        {
            std::string key = cur_.value;
            advance_(); // consume key
            advance_(); // consume colon

            // Skip newlines após colon
            while (cur_.type == TokenType::TOKEN_NEWLINE)
            {
                advance_();
            }

            map[key] = parseValue_();

            // Skip newlines entre entries
            while (cur_.type == TokenType::TOKEN_NEWLINE)
            {
                advance_();
            }

            // Consumir DEDENTs
            while (cur_.type == TokenType::TOKEN_DEDENT)
            {
                advance_();
            }

            // Check se ainda há mais pares key:value
            if (!(cur_.type == TokenType::TOKEN_STRING && nxt_.type == TokenType::TOKEN_COLON))
            {
                break;
            }
        }

        return YamlValue(map);
    }
    YamlValue Parser::parseSequence_()
    {
        YamlValue::Sequence seq;

        while (cur_.type == TokenType::TOKEN_DASH)
        {
            advance_(); // consume dash
            seq.push_back(parseValue_());

            // Skip newlines
            while (cur_.type == TokenType::TOKEN_NEWLINE)
            {
                advance_();
            }

            // Se não há mais dashes, parar
            if (cur_.type != TokenType::TOKEN_DASH)
            {
                break;
            }
        }

        return YamlValue(seq);
    }

} // namespace yaml