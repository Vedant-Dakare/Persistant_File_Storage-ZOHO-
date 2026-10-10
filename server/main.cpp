#include "../include/server.h"
#include "../include/persistence.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <memory>
#include <thread>

#define SOCKET_PATH "/tmp/fs.sock"
#define DATA_PATH "data/filesystem.img"

using namespace std;

int main() {

    auto loadResult = loadFilesystem(DATA_PATH);
    unique_ptr<Node> rootOwner;


    switch(loadResult.status) {
        case LoadStatus::OK:
            cout << "Loaded filesystem from " << DATA_PATH << "\n";
            rootOwner = move(loadResult.root);
            break;
        case LoadStatus::NO_FILE:
            cout << "No existing filesystem found, creating new one\n";
            rootOwner = make_unique<Node>("/",true,nullptr);
            break;
        case LoadStatus::CORRUPTED:
            cerr << "WARNING: " << DATA_PATH << " exists but is corrupted/unreadable. "
            << "Starting with a fresh filesystem. The corrupted file has been left "
            << "in place for inspection.\n";
            rootOwner = make_unique<Node>("/",true,nullptr);
            break;
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
        
        int sessionId = nextSessionId++;

        // creates a new thread to handle the client connection, which allows server accept the new connection while serving the previous one
        thread t(clientHandler,client_fd,sessionId,root);
        
        // detach the thread from main thread, so that it can run independently and resources will be released when it finishes
        t.detach(); 
    }
    close(listen_fd);
    unlink(SOCKET_PATH);
    return 0;
}