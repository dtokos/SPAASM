#ifndef ExternalProgram_hpp
#define ExternalProgram_hpp

#include <vector>
#include <string>

struct ExternalProgram {
    char *name{};
    std::vector<char *> args{};
    bool runInBackground{false};
    std::string input{};
    std::string output{};
};

#endif
