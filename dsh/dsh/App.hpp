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
#include <fcntl.h>
#include <pwd.h>
#include <netinet/in.h>
#include <time.h>
#include <poll.h>
#include "Tokenizer.hpp"
#include "TokenParser.hpp"
#include "Help.hpp"
#include "Chdir.hpp"
#include "ExternalProgram.hpp"

struct App {
    struct Options {
        in_addr address;
        in_port_t port;
        int backlog;
        int clientTimeout;
        bool verbose;
    };
    
    struct Exception : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
    
    void handleSignal(int signal) {
        if (verbose)
            std::cerr << "[DEBUG] Handling signal " << signal << '\n';
        
        if (signal == SIGTERM) {
            if (isRoot) {
                killClients();
                cleanup();
                exit(EXIT_SUCCESS);
            } else {
                exit(EXIT_SUCCESS);
            }
        }
    }
    
    void start(const Options &options) {
        clientTimeout = options.clientTimeout;
        verbose = options.verbose;
        
        if (verbose)
            std::cerr << "[DEBUG] Starting app\n";
        
        startServerSocket(options);
        startConsoleClient();
    }
    
    void run() {
        if (verbose)
            std::cerr << "[DEBUG] Run app\n";
        
        while (true) {
            int clientFD = accept(serverSocketFD, nullptr, nullptr);
            if (clientFD < 0)
                throw Exception("Could not accept client\n" + std::string(std::strerror(errno)));
            
            startSocketClient(clientFD);
        }
    }
    
    void cleanup() {
        if (verbose)
            std::cerr << "[DEBUG] App cleanup\n";
        
        if (serverSocketFD != -1) {
            shutdown(serverSocketFD, SHUT_RDWR);
            close(serverSocketFD);
        }
    }
    
protected:
    bool isRoot{true};
    int serverSocketFD{-1};
    std::vector<pid_t> clients{};
    int clientTimeout{600000};
    bool verbose{false};
    
    Tokenizer tokenizer{};
    TokenParser tokenParser{};
    
    void killClients() {
        if (verbose)
            std::cerr << "[DEBUG] Kill all clients\n";
        
        for (const auto &clientPID : clients)
            kill(clientPID, SIGTERM);
    }
    
    void haltShell() {
        if (verbose)
            std::cerr << "[DEBUG] Execute halt command\n";
        
        kill(getppid(), SIGTERM);
    }
    
    void startServerSocket(const Options &options) {
        if (verbose)
            std::cerr << "[DEBUG] Start server socket\n";
        
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
        if (verbose)
            std::cerr << "[DEBUG] Start console client\n";
        
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
        if (verbose)
            std::cerr << "[DEBUG] Start socket client: " << sockFD << '\n';
        
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
            if (!getInput(input))
                break;
            
            auto tokens = tokenizer.tokenize(input);
            
            if (tokens.size() == 0)
                continue;
            
            if (tokens[0] == "quit")
                break;
            
            if (tokens[0] == "halt") {
                haltShell();
                break;
            }
            
            if (executeInternalProgram(tokens))
                continue;
            
            executeExternalProgram(tokens);
        }
        
        exit(EXIT_SUCCESS);
    }
    
    bool getInput(std::string &input) {
        pollfd inputFD{};
        inputFD.fd = STDIN_FILENO;
        inputFD.events = POLLIN;
        if (poll(&inputFD, 1, clientTimeout) == 0)
            return false;
        
        std::getline(std::cin, input);
        
        return true;
    }
    
    void writePrompt() {
        const tm time = getTime();
        const std::string user = getUser();
        const std::string hostname = getHostname();
        
        std::cout << std::setfill('0') << std::setw(2) << time.tm_hour << ':'
            << std::setfill('0') << std::setw(2) << time.tm_min << ' '
            << user << '@' << hostname << "# " << std::flush;
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
    
    bool executeInternalProgram(const std::vector<std::string> &tokens) {
        if (tokens[0] == "help") {
            HelpProgram().run();
            return true;
        } else if (tokens[0] == "cd") {
            ChdirProgram().run(tokens);
            return true;
        }
        
        return false;
    }
    
    void executeExternalProgram(std::vector<std::string> &tokens) {
        auto pipeline = tokenParser.parse(tokens);
        
        if (pipeline.size() == 2) executePipeline(pipeline[0], pipeline[1]);
        else executeSingleExternal(pipeline[0]);
    }
    
    void executePipeline(ExternalProgram &programA, ExternalProgram &programB) {
        int programPipe[2]{};
        
        if (pipe(programPipe) < 0) {
            std::cerr << "Could not fork external program\n" << std::string(std::strerror(errno)) << '\n';
            return;
        }
        
        const pid_t programAPID = fork();
        if (programAPID < 0) {
            std::cerr << "Could not fork external program\n" << std::string(std::strerror(errno)) << '\n';
            return;
        } else if (programAPID == 0) {
            if (programA.input != "")
                redirectIOTo(programA.input, STDIN_FILENO, O_RDONLY);
            
            close(programPipe[0]);
            dup2(programPipe[1], STDOUT_FILENO);
            
            execvp(programA.name, programA.args.data());
            std::cerr << "Could not exec external program\n" + std::string(std::strerror(errno)) << '\n';
            exit(EXIT_FAILURE);
        }
        
        const pid_t programBPID = fork();
        if (programBPID < 0) {
            std::cerr << "Could not fork external program\n" << std::string(std::strerror(errno)) << '\n';
            return;
        } else if (programBPID == 0) {
            if (programB.output != "")
                redirectIOTo(programB.output, STDOUT_FILENO, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            
            close(programPipe[1]);
            dup2(programPipe[0], STDIN_FILENO);
            
            execvp(programB.name, programB.args.data());
            std::cerr << "Could not exec external program\n" + std::string(std::strerror(errno)) << '\n';
            exit(EXIT_FAILURE);
        } else {
            close(programPipe[0]);
            close(programPipe[1]);
            
            waitForPID(programAPID);
            waitForPID(programBPID);
        }
    }
    
    void executeSingleExternal(ExternalProgram &program) {
        const pid_t programPID = fork();
        
        if (programPID < 0)
            std::cerr << "Could not fork external program\n" << std::string(std::strerror(errno)) << '\n';
        else if (programPID == 0) {
            if (program.input != "")
                redirectIOTo(program.input, STDIN_FILENO, O_RDONLY);
            
            if (program.output != "")
                redirectIOTo(program.output, STDOUT_FILENO, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            
            execvp(program.name, program.args.data());
            std::cerr << "Could not exec external program\n" + std::string(std::strerror(errno)) << '\n';
            exit(EXIT_FAILURE);
        } else {
            if (!program.runInBackground)
                waitForPID(programPID);
        }
    }
    
    void redirectIOTo(const std::string &file, int fd, int openFlags, int mode = 0) const {
        int inputFD = open(file.c_str(), openFlags, mode);
        
        if (inputFD < 0) {
            std::cerr << "Could not redirect IO\n" << std::string(std::strerror(errno)) << '\n';
            exit(EXIT_FAILURE);
        }
        
        dup2(inputFD, fd);
        close(inputFD);
    }
    
    void waitForPID(const pid_t pid) {
        int status{};
        do {
          waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
};

#endif
