#ifndef TokensParser_hpp
#define TokensParser_hpp

#include <vector>
#include <string>
#include "ExternalProgram.hpp"

struct TokenParser {
    std::vector<ExternalProgram> parse(std::vector<std::string> &tokens) {
        std::vector<ExternalProgram> result{};
        
        auto iter = tokens.begin();
        const auto end = tokens.end();
        
        while (iter != end) {
            ExternalProgram program{};
            
            if (parseProgram(program, iter, end))
                result.push_back(program);
        }
        
        return result;
    }
    
    bool parseProgram(ExternalProgram &program, std::vector<std::string>::iterator &iter, const std::vector<std::string>::iterator &end) {
        if (iter == end)
            return false;
        
        program.name = (*iter).data();
        program.args.push_back((*iter).data());
        
        for (++iter; iter != end; ++iter) {
            if (*iter == "&") {
                program.runInBackground = true;
            } else if (*iter == "<" && (iter + 1) != end) {
                program.input = *(++iter);
            } else if (*iter == ">" && (iter + 1) != end) {
                program.output = *(++iter);
            } else if (*iter == "|") {
                ++iter;
                break;
            } else {
                program.args.push_back((*iter).data());
            }
        }
        
        program.args.push_back(nullptr);
        
        return true;
    }
};

#endif
