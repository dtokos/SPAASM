#ifndef ArgsParser_hpp
#define ArgsParser_hpp

#include <docopt/docopt.h>
#include "App.hpp"

static const char USAGE[] =
R"(dsh
This program is a simple shell. You can interact with it trough terminal or local UNIX socket.
Usage:
  dsh [options]
  dsh (-h | --help)
Options:
  -h --help                    Show this screen.
  
  -p PORT --port=PORT          Configures port that the shell will be listening on.
                               Accepts values from <1, 65535> (OS may block listening on reserved ports < 1024).
                               [default: 48184]
  
  -u PATH --unix_path=PATH    Configures path that the shell will be listening on.
                               Accepts valid absolute or relative path.
                               [default: ./dsh-sock]
)";

struct ArgsParser {
    struct Exception : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
    
    App::Options parse(int argc, const char *argv[]) {
        std::map<std::string, docopt::value> args = docopt::docopt(USAGE, {argv + 1, argv + argc}, true);
        
        return App::Options{
            parseSocketPort(args["--port"]),
            parseSocketPath(args["--unix_path"]),
        };
    }
    
private:
    int parseSocketPort(const docopt::value &value) {
        try {
            int port = std::stoi(value.asString());
            if (port < 0 || port > 65535) throw std::out_of_range("Option 'port' is out of range");
            return port;
        } catch (const std::invalid_argument &e) {
            throw ArgsParser::Exception("Option 'port' is invalid");
        } catch (const std::out_of_range &e) {
            throw ArgsParser::Exception("Option 'port' is out of range");
        }
    }
    
    std::string parseSocketPath(const docopt::value &value) {
        return value.asString();
    }
};

#endif
