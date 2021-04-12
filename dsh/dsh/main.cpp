#include <iostream>
#include "ArgsParser.hpp"

int main(int argc, const char *argv[]) {
    try {
        ArgsParser argsParser{};
        auto asd = argsParser.parse(argc, argv);
        std::cout << asd.port << ' ' << asd.socketPath << '\n';
    } catch (const ArgsParser::Exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
