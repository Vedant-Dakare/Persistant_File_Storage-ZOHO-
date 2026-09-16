#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
using namespace std;

#define SOCKET_PATH "/tmp/demo.sock"


int main() {
    // create socket
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        return 1;
    }

    // server address we want to connect to
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // connect -> reach to the server's listening socket
    if(connect(sock_fd,(sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        return 1;
    }

    // send message
    // Socket APIs deal with bytes, so char* is commonly used directly not string 
    const char* msg = "Hello Server!!!";
    send(sock_fd,msg,strlen(msg),0);

    // Receive server reply
    char buffer[256] = {0};
    ssize_t n = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        cout << "Data received from client: " << buffer << "\n";
    }

    close(sock_fd);
    return 0;
}