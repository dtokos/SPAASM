#ifndef Help_hpp
#define Help_hpp

#include <iostream>

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
  
  -b BACKLOG --backlog=BACKLOG    Configures length of socket connection queue.
                                  Accepts values from <1, 10>.
                                  [default: 5]
  
  -t TIMEOUT --timeout=TIMEOUT    Configures timeout after which inactive clients will be disconnected.
                                  Accepts values from <5, 3600> as seconds.
                                  [default: 600]
  
  -v --verbose                    Increases output verbosity.
)";


struct HelpProgram {
    void run() {
        std::cout << USAGE << '\n';
    }
};

#endif
