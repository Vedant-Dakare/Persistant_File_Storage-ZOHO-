#include "../include/server.h"
#include "../include/persistence.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <memory>

#define SOCKET_PATH "/tmp/fs.sock"
#define DATA_PATH "data/filesystem.img"

using namespace std;

int main() {

    auto rootOwner = loadFilesystem(DATA_PATH);

    if(!rootOwner) {
        cout << "No existing filesystem found, creating new one\n";
        rootOwner = make_unique<Node>("/",true,nullptr);
    }
    else {
        cout << "Loaded existing filesystem from " << DATA_PATH << "\n";
    }

    Node* root = rootOwner.get();

    int listen_fd = socket(AF_UNIX,SOCK_STREAM,0);
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path,SOCKET_PATH,sizeof(addr.sun_path)-1);
    unlink(SOCKET_PATH);  // remove old socket file if exists
    bind(listen_fd,(sockaddr*)&addr,sizeof(addr));
    listen(listen_fd,5);

    cout << "fsserver is listening on " << SOCKET_PATH << "\n";

    int nextSessionId = 1;

    while(true) {
        int client_fd = accept(listen_fd,nullptr,nullptr);
        if(client_fd == -1) continue;
        
        cout << "Client connected (fd=" << client_fd << ")\n";

        // create a new session for this client and handle requests in a loop until the client disconnects
        ClientSession session{nextSessionId++,client_fd,"/"};
        Connection conn{client_fd,""};

        while(true) {
            auto line = readLine(conn);
            if(!line) {
                cout << "Client disconnected";
                break;
            }
            ParsedRequest req = parseRequest(*line);
            string response = handleRequest(req,session,root);
            if(!sendAll(client_fd,response)) {
                break;
            }
        }
        close(client_fd);
    }
    close(listen_fd);
    unlink(SOCKET_PATH);
    return 0;
}