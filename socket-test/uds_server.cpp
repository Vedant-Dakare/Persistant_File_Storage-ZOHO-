#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
using namespace std;

#define SOCKET_PATH "/tmp/demo.sock"


int main() {
    // create the socket
    int listen_fd = socket(AF_UNIX,SOCK_STREAM,0);
    if(listen_fd < 0) {
        perror("socket");
        return 1;
    }

    // address we want to bind to
    sockaddr_un addr {};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // if any old socket it present then delete it otherwise proceed further
    unlink(SOCKET_PATH);

    // bind -> attach addr to our fd
    if(bind(listen_fd,(sockaddr*)&addr,sizeof(addr)) == -1) {
        perror("bind");
        return 1;
    }

    // listen
    if(listen(listen_fd,5) == -1) {
        perror("listen");
        return 1;
    }

    cout << "Server is listening on " << SOCKET_PATH << "...\n";
    
    // connecting to client
    int client_fd = accept(listen_fd,nullptr,nullptr);
    if(client_fd == -1) {
        perror("client");
        return 1;
    }

    cout << "Client Connected";

    // accpet the data from the client into a buffer
    char buffer[256] = {0};
    ssize_t n = recv(client_fd,buffer,sizeof(buffer)-1,0);
    cout << " Data recieved from client: " << buffer << "\n";

    // sending reponse to client
    // Socket APIs deal with bytes, so char* is commonly used directly not string 
    const char* reply = "Hello client";
    send(client_fd,reply,strlen(reply),0);

    // close connection
    close(client_fd);
    close(listen_fd);
    unlink(SOCKET_PATH);
    
    return 0;
}