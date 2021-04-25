#include <iostream>
#include "ArgsParser.hpp"
#include "App.hpp"

App app{};

void registerHandler(int signal);
void handleSignal(int signal);

int main(int argc, const char *argv[]) {
    try {
        ArgsParser argsParser{};
        auto options = argsParser.parse(argc, argv);
        
        registerHandler(SIGTERM);
        
        app.start(options);
        app.run();
        app.cleanup();
    } catch (const ArgsParser::Exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    } catch (const App::Exception &e) {
        app.cleanup();
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

void registerHandler(int signal) {
    struct sigaction action{};
    action.sa_handler = &handleSignal;
    sigaction(SIGTERM, &action, nullptr);
}

void handleSignal(int signal) {
    app.handleSignal(signal);
}
