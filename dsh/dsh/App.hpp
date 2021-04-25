#ifndef App_hpp
#define App_hpp

#include <vector>
#include <string>
#include <iomanip>
#include <cerrno>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <pwd.h>
#include <netinet/in.h>
#include <time.h>
#include "Tokenizer.hpp"

struct App {
    struct Options {
        in_addr address;
        in_port_t port;
        int backlog;
    };
    
    struct Exception : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
    
    void handleSignal(int signal) {
        if (signal == SIGTERM && isRoot) {
            killClients();
            cleanup();
            exit(EXIT_SUCCESS);
        }
    }
    
    void start(const Options &options) {
        startServerSocket(options);
        startConsoleClient();
    }
    
    void run() {
        while (true) {
            int clientFD = accept(serverSocketFD, nullptr, nullptr);
            if (clientFD < 0)
                throw Exception("Could not accept client\n" + std::string(std::strerror(errno)));
            
            startSocketClient(clientFD);
        }
    }
    
    void cleanup() {
        if (serverSocketFD != -1) {
            shutdown(serverSocketFD, SHUT_RDWR);
            close(serverSocketFD);
        }
    }
    
private:
    bool isRoot{true};
    Tokenizer tokenizer{};
    int serverSocketFD{-1};
    std::vector<pid_t> clients{};
    
    void killClients() {
        for (const auto &clientPID : clients)
            kill(clientPID, SIGTERM);
    }
    
    void startServerSocket(const Options &options) {
        int socketFD = socket(PF_INET, SOCK_STREAM, 0);
        if (socketFD < 0)
            throw Exception("Could not open server socket\n" + std::string(std::strerror(errno)));
        
        serverSocketFD = socketFD;
        
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr = options.address;
        address.sin_port = options.port;
        
        if (bind(serverSocketFD, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0)
            throw Exception("Could not bind address to socket\n" + std::string(std::strerror(errno)));
        
        if (listen(serverSocketFD, options.backlog) < 0)
            throw Exception("Could not start listening on socket\n" + std::string(std::strerror(errno)));
    }
    
    void startConsoleClient() {
        const pid_t clientPID = fork();
        
        if (clientPID < 0)
            throw new Exception("Could not start console client\n" + std::string(std::strerror(errno)));
        else if (clientPID == 0) {
            isRoot = false;
            runClientLoop();
        } else {
            clients.push_back(clientPID);
            close(STDIN_FILENO);
        }
    }
    
    void startSocketClient(int sockFD) {
        const pid_t clientPID = fork();
        
        if (clientPID < 0)
            throw new Exception("Could not start socket client\n" + std::string(std::strerror(errno)));
        else if (clientPID == 0) {
            isRoot = false;
            dup2(sockFD, STDIN_FILENO);
            dup2(sockFD, STDOUT_FILENO);
            dup2(sockFD, STDERR_FILENO);
            runClientLoop();
        } else {
            clients.push_back(clientPID);
            close(sockFD);
        }
    }
    
    void runClientLoop() {
        std::string input;
        
        while (true) {
            writePrompt();
            std::getline(std::cin, input);
            
            const auto tokens = tokenizer.tokenize(input);
            
            if (tokens.size() == 0)
                continue;
            
            if (tokens[0] == "quit")
                break;
            
            if (tokens[0] == "halt") {
                haltShell();
                break;
            }
            
            for (const auto &t : tokens)
                std::cout << t << '\n';
            
            //std::cout << "Echo: " << input << '\n';
        }
        
        exit(EXIT_SUCCESS);
    }
    
    void writePrompt() {
        const tm time = getTime();
        const std::string user = getUser();
        const std::string hostname = getHostname();
        
        std::cout << std::setfill('0') << std::setw(2) << time.tm_hour << ':'
            << std::setfill('0') << std::setw(2) << time.tm_min << ' '
            << user << '@' << hostname << "# ";
    }
    
    tm getTime() const {
        const time_t timeSinceEpoch = time(nullptr);
        const tm *parts = localtime(&timeSinceEpoch);
        return *parts;
    }
    
    std::string getUser() const {
        const auto userRecord = getpwuid(getuid());
        return std::string(userRecord->pw_name);
    }
    
    std::string getHostname() const {
        char hostnameBuffer[256];
        gethostname(hostnameBuffer, 256);
        return std::string(hostnameBuffer);
    }
    
    void haltShell() {
        kill(getppid(), SIGTERM);
    }
};

#endif
