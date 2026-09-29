#include "../include/protocol.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/protocol_test.sock"

int main() {
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    connect(sock_fd, (sockaddr*)&addr, sizeof(addr));

    // Deliberately send TWO lines in a single send() call, to prove
    // readLine() correctly splits them instead of merging them.
    sendAll(sock_fd, "MKDIR /home/user\nLS -l\n");
    sendAll(sock_fd, "PWD\n");

    Connection conn{sock_fd, ""};
    for (int i = 0; i < 3; i++) {
        auto line = readLine(conn);   // reading the OK/ERR header line
        if (!line) break;
        std::cout << "Client got header: [" << *line << "]\n";

        // header looks like "OK <len>" — parse the length and read the body.
        size_t sp = line->find(' ');
        int len = std::stoi(line->substr(sp + 1));
        std::string body(len, '\0');
        size_t got = 0;
        while ((int)got < len) {
            ssize_t n = recv(sock_fd, &body[got], len - got, 0);
            if (n <= 0) break;
            got += n;
        }
        std::cout << "Client got body: [" << body << "]\n";
    }

    close(sock_fd);
}