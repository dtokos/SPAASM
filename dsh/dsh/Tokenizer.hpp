#ifndef Tokenizer_hpp
#define Tokenizer_hpp

#include <vector>
#include <string>

struct Tokenizer {
    std::vector<std::string> tokenize(const std::string &input) {
        std::vector<std::string> tokens{};
        
        for (col = 0; col < input.size();) {
            skipWhiteSpace(input);
            if (col >= input.size())
                break;
            
            const auto token = parseToken(input);
            if (token.size() == 0)
                continue;
            
            tokens.push_back(token);
        }
        
        return tokens;
    }
    
private:
    std::size_t col{};
    
    void skipWhiteSpace(const std::string &input) {
        for (; col < input.size() && std::isspace(input[col]); ++col)
            ;
    }
    
    std::string parseToken(const std::string &input) {
        const char start = input[col];
        
        if (start == '#') return skipComment(input);
        else if (start == '"') return parseQuotedToken(input, '"');
        else if (start == '\'') return parseQuotedToken(input, '\'');
        else return parseNormalToken(input);
    }
    
    std::string skipComment(const std::string &input) {
        col = input.size();
        return "";
    }
    
    std::string parseQuotedToken(const std::string &input, char quote) {
        auto end = input.find(quote, col + 1);
        
        while (end != std::string::npos && input[end - 1] == '\\')
            end = input.find(quote, end + 1);
        
        if (end == std::string::npos) {
            const auto token = input.substr(col + 1);
            col = input.size();
            return token;
        } else {
            const auto token = input.substr(col + 1, end - col - 1);
            col = end + 1;
            return token;
        }
    }
    
    std::string parseNormalToken(const std::string &input) {
        std::size_t end = col;
        
        for (; end < input.size() && !std::isspace(input[end]); ++end)
            ;
        
        const auto token = input.substr(col, end - col);
        col = ++end;
        return token;
    }
};

#endif
