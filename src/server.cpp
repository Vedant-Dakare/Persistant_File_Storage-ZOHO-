#include "../include/node.h"
#include "../include/filesystem.h"
#include "../include/session.h"
#include "../include/protocol.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <memory>


#define SOCKET_PATH "/tmp/fs.sock"


static const char* codeFor(FsResult r) {
    switch(r) {
        case FsResult::OK : return "OK";
        case FsResult::ALREADY_EXISTS : return "EXISTS";
        case FsResult::NOT_A_DIRECTORY : return "NOT_A_DIRECTORY";
        case FsResult::NOT_FOUND : return "NOT_FOUND";
        case FsResult::NOT_EMPTY : return "NOT_EMPTY";
        case FsResult::INVALID_PATH : return "INVALID_PATH";
    }
    return "UNKNOWN";
} 

string handleRequest(const ParsedRequest &req,ClientSession &session,Node* root) {
    if(!req.valid) {
        return buildResponse(false,"BAD_RESPONSE","could not parse request");
    }

    bool reset;
    Node* cwd = sessionCwd(session,root,reset);
    string note = reset ? "(your directory was removed, so we reset you to /)" : "";

    const string &cmd = req.command;
    const string &arg = req.arg;

    if(cmd == "PWD") {
        return buildResponse(true,note+pwd(cwd)+"\n");
    }

    if(cmd == "LS") {
        if(arg == "-l") {
            ostringstream out;
            for(auto &e : lsLong(cwd)) {
                out << (e.isDirectory ? "d" : "- ") << e.size << "\t" << e.name << "\n";
            }
            return buildResponse(true,note+out.str());
        }
        ostringstream out;
        for(auto &name : ls(cwd)) out << name << "\n";
        return buildResponse(true,note+out.str());
    }

    if(cmd == "MKDIR") {
        FsResult r = fsMkdir(cwd,root,arg);
        if(r != FsResult::OK) {
            return buildResponse(false,codeFor(r),"mkdir: " + arg);
        }
        return buildResponse(true,note + "directory created\n");
    }

    if(cmd == "TOUCH") {
        FsResult r = fsTouch(cwd,root,arg);
        if(r != FsResult::OK) {
            return buildResponse(false,codeFor(r),"touch: " + arg);
        }
        return buildResponse(true,note + "file created\n");
    }

    if(cmd == "RM") {
        FsResult r = fsRm(cwd,root,arg);
        if(r != FsResult::OK) {
            return buildResponse(false,codeFor(r),"rm: " + arg);
        }
        return buildResponse(true,note + "file removed\n");
    }

    if(cmd == "RMRF") {
        FsResult r = fsRmrf(cwd,root,arg);
        if(r != FsResult::OK) {
            return buildResponse(false,codeFor(r),"rm -rf: " + arg);
        }
        return buildResponse(true,note+"file removed\n");
    }

    if(cmd == "CD") {
        CdResult r = fsCd(cwd,root,arg);
        if(r.status != FsResult::OK) {
            return buildResponse(false,codeFor(r.status),"cd: " + arg);
        }
        sessionSetCwd(session,r.newCwd);
        return buildResponse(true,note+pwd(r.newCwd)+"\n");
    }

    return buildResponse(false,"UNKNOWN_COMMAND", cmd);
}

int main() {
    auto rootOwner = std::make_unique<Node>("/",true,nullptr);
    Node* root = rootOwner.get();

    int listen_fd = socket(AF_UNIX,SOCK_STREAM,0);
    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path,SOCKET_PATH,sizeof(addr.sun_path)-1);
    unlink(SOCKET_PATH);
    bind(listen_fd,(sockaddr*)&addr,sizeof(addr));
    listen(listen_fd,5);

    cout << "Server listening on " << SOCKET_PATH << "\n";
    int nextSessionId = 1;
    while(true) {
        int client_fd = accept(listen_fd,nullptr,nullptr);
        if(client_fd == -1) continue;

        cout << "Client connected (fd=" << client_fd << ")\n";

        ClientSession session{nextSessionId++,client_fd,"/"};
        Connection conn{client_fd,""};

        while(true) {
            auto line = readLine(conn);
            if(!line) {
                cout << "Client disconnected (fd=" << client_fd << ")\n";;
                break;
            }
            ParsedRequest req = parseRequest(*line);
            string response  =  handleRequest(req,session,root);
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