#ifndef App_hpp
#define App_hpp

#include <string>

struct App {
    struct Options {
        int port;
        std::string socketPath;
    };
};

#endif
