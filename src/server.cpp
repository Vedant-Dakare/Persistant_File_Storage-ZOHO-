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
#include "../include/server.h"
#include "../include/persistence.h"
#include <mutex>
#include <thread>

using namespace std;

mutex fsMutex;   // one global lock protecting the whole filesystem tree

#define SOCKET_PATH "/tmp/fs.sock"
#define DATA_PATH "data/filesystem.img"


// CD and PWD/LS never call persist() at all, since they don't mutate the tree
static bool persist(Node* root) {
    if(!saveFilesystem(root,DATA_PATH)) {
        cerr << "WARNING: failed to persist filesystem to disk\n";
        return false;
    }
    return true;
}

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

    lock_guard<mutex> lock(fsMutex);    // lock the whole filesystem tree

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
        if(!arg.empty()) {
            return buildResponse(false,"BAD_REQUEST","ls: unknown option " + arg);
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
        bool saved = persist(root);
        string msg = "directory created\n";
        if(!saved) msg += "WARNING: failed to persist filesystem to disk\n";
        return buildResponse(true,note + "directory created\n");
    }

    if(cmd == "TOUCH") {
        FsResult r = fsTouch(cwd,root,arg);
        if(r != FsResult::OK) {
            return buildResponse(false,codeFor(r),"touch: " + arg);
        }
        bool saved =persist(root);
        string msg = "file created\n";
        if(!saved) msg += "WARNING: failed to persist filesystem to disk\n";
        return buildResponse(true,note + "file created\n");
    }

    if(cmd == "RM") {
        FsResult r = fsRm(cwd,root,arg);
        if(r != FsResult::OK) {
            return buildResponse(false,codeFor(r),"rm: " + arg);
        }
        bool saved = persist(root);
        string msg = "file removed\n";
        if(!saved) msg += "WARNING: failed to persist filesystem to disk\n";
        return buildResponse(true,note + "file removed\n");
    }

    if(cmd == "RMRF") {
        FsResult r = fsRmrf(cwd,root,arg);
        if(r != FsResult::OK) {
            return buildResponse(false,codeFor(r),"rm -rf: " + arg);
        }
        bool saved = persist(root);
        string msg = "file removed\n";
        if(!saved) msg += "WARNING: failed to persist filesystem to disk\n";
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


void clientHandler(int client_fd, int sessionId, Node* root) {
    cout << "client connected (fd=" << client_fd << ")\n";

    ClientSession session{sessionId, client_fd, "/"};
    Connection conn{client_fd, ""};

    while (true) {
        auto line = readLine(conn);
        if (!line) {
            cout << "client disconnected (fd=" << client_fd << ")\n";
            break;
        }
        ParsedRequest req = parseRequest(*line);
        string response = handleRequest(req, session, root);
        if (!sendAll(client_fd, response)) break;
    }

    close(client_fd);
}