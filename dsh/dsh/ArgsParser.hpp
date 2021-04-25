#ifndef ArgsParser_hpp
#define ArgsParser_hpp

#include <arpa/inet.h>
#include <docopt/docopt.h>
#include "App.hpp"
#include "Help.hpp"

struct ArgsParser {
    struct Exception : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
    
    App::Options parse(int argc, const char *argv[]) {
        std::map<std::string, docopt::value> args = docopt::docopt(USAGE, {argv + 1, argv + argc}, true);
        
        return App::Options{
            parseSocketAddress(args["--address"]),
            parseSocketPort(args["--port"]),
            parseBacklog(args["--backlog"]),
            parseClientTimeout(args["--timeout"]),
            parseVerbose(args["--verbose"])
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
    
    int parseBacklog(const docopt::value &value) {
        try {
            int backlog = std::stoi(value.asString());
            if (backlog < 1 || backlog > 10) throw std::out_of_range("Option 'backlog' is out of range");
            
            return backlog;
        } catch (const std::invalid_argument &e) {
            throw ArgsParser::Exception("Option 'backlog' is invalid");
        } catch (const std::out_of_range &e) {
            throw ArgsParser::Exception("Option 'backlog' is out of range");
        }
    }
    
    int parseClientTimeout(const docopt::value &value) {
        try {
            int timeout = std::stoi(value.asString());
            if (timeout < 5 || timeout > 3600) throw std::out_of_range("Option 'timeout' is out of range");
            
            return timeout * 1000;
        } catch (const std::invalid_argument &e) {
            throw ArgsParser::Exception("Option 'timeout' is invalid");
        } catch (const std::out_of_range &e) {
            throw ArgsParser::Exception("Option 'timeout' is out of range");
        }
    }
    
    bool parseVerbose(const docopt::value &value) {
        return value.asBool();
    }
};

#endif
