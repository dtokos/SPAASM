#ifndef ArgsParser_hpp
#define ArgsParser_hpp

#include <arpa/inet.h>
#include <docopt/docopt.h>
#include "App.hpp"

static const char USAGE[] =
R"(dsh
This program is a simple shell. You can interact with it trough terminal or local UNIX socket.
Usage:
  dsh [options]
  dsh (-h | --help)
Options:
  -h --help                       Show this screen.
  
  -a ADDRESS --address=ADDRESS    Configures address that the shell will be listening on.
                                  Accepts valid ipv4 address.
                                  [default: 0.0.0.0]
  
  -p PORT --port=PORT             Configures port that the shell will be listening on.
                                  Accepts values from <1, 65535> (OS may block listening on reserved ports < 1024).
                                  [default: 48184]
)";

struct ArgsParser {
    struct Exception : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
    
    App::Options parse(int argc, const char *argv[]) {
        std::map<std::string, docopt::value> args = docopt::docopt(USAGE, {argv + 1, argv + argc}, true);
        
        return App::Options{
            parseSocketAddress(args["--address"]),
            parseSocketPort(args["--port"]),
        };
    }
    
private:
    in_addr parseSocketAddress(const docopt::value &value) {
        const auto ipString = value.asString();
        in_addr ip{};
        
        if (inet_aton(ipString.c_str(), &ip) == 0)
            throw ArgsParser::Exception("Option 'address' is invalid");
        
        return ip;
    }
    
    in_port_t parseSocketPort(const docopt::value &value) {
        try {
            int port = std::stoi(value.asString());
            if (port < 0 || port > 65535) throw std::out_of_range("Option 'port' is out of range");
            
            return htons(port);
        } catch (const std::invalid_argument &e) {
            throw ArgsParser::Exception("Option 'port' is invalid");
        } catch (const std::out_of_range &e) {
            throw ArgsParser::Exception("Option 'port' is out of range");
        }
    }
};

#endif
