#ifndef Chdir_hpp
#define Chdir_hpp

#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>

struct ChdirProgram {
    void run(const std::vector<std::string> tokens) {
        if (tokens.size() >= 2) {
            if (chdir(tokens[1].c_str()) < 0) {
                std::cerr << "Could not change working directory\n" << std::string(std::strerror(errno)) << '\n';
            }
        }
    }
};

#endif
