#include "../include/protocol.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sstream>
#include <algorithm>

#define SOCKET_PATH "/tmp/fs.sock"

using namespace std;

struct Response {
    bool ok;
    string body;
    string errCode;
};


static Response readResponse(Connection  &conn) {
    auto headerLine = readLine(conn);

    if(!headerLine) {
        return {false,"","DISCONNECTED"};
    }

    istringstream hs(*headerLine);
    string status;
    hs >> status;

    int len = 0;
    string errCode;
    if(status == "OK") {
        hs >> len;
    }
    else {
        hs >> errCode >> len;
    }
    
    string body;
    while((int)body.size() < len) {
        if(!conn.recvBuf.empty()) {
            size_t take = min(conn.recvBuf.size(),(size_t)(len - body.size()));
            body += conn.recvBuf.substr(0, take);
            conn.recvBuf.erase(0, take);
        }
        else {
            char chunk[512];
            ssize_t n = recv(conn.fd,chunk,sizeof(chunk),0);
            if(n <= 0) {
                return {false,"","DISCONNECTED"};
            }
            conn.recvBuf.append(chunk,n);
        }
    }
    return {status == "OK",body,errCode};
}

int main() {
    int sock_fd = socket(AF_UNIX,SOCK_STREAM,0);
    if(sock_fd == -1) {
        perror("socket");
        return 1;
    } 

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path,SOCKET_PATH,sizeof(addr.sun_path)-1);

    if(connect(sock_fd,(sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        cerr << "Is fsserver running?\n";
        return 1;
    }

    Connection conn{sock_fd,""};
    string promptPath = "/";

    sendAll(sock_fd,"PWD\n");
    Response initial = readResponse(conn);
    
    if(initial.ok) {
        promptPath = initial.body;
        if(!promptPath.empty() && promptPath.back() == '\n') promptPath.pop_back();
    }

    string line;
    while(true) {
        cout << promptPath << " $ ";
        if(!getline(cin,line)) break;  // EOF(ctrl + D) --> quit
        if(line.empty()) continue;

        if(line == "exit" || line == "quit") break;

        ParsedRequest req = parseRequest(line);
        string wire = line;
        if(req.command == "RM" && (req.arg == "-rf" || req.arg.rfind("-rf",0) == 0)) {
            string rest = req.arg.substr(3);
            size_t p = rest.find_first_not_of(' ');
            rest = (p == string::npos) ? "" : rest.substr(p);
            wire = "RMRF " + rest;
        }

        if(!sendAll(sock_fd, wire + "\n")) {
            cerr << "connection lost\n";
            break;
        }
        
        Response r = readResponse(conn);

        if(r.errCode == "DISCONNECTED") {
            cerr << "Server Disconnected\n";
            break;
        } 

        if(r.ok) {
            cout << r.body;
            ParsedRequest req = parseRequest(line);
            if(req.command == "PWD" || req.command == "CD") {
                istringstream ss(r.body);
                string firstLine;
                getline(ss,firstLine);
                if(firstLine.size() > 0 && firstLine[0] == '/') {
                    promptPath = firstLine;
                }
                else {
                    getline(ss,firstLine);
                    if(!firstLine.empty()) {
                        promptPath = firstLine;
                    }
                }
            }
        }
        else {
            cout << "Error [" << r.errCode << "]: " << r.body << "\n";
        }
    }
    close(sock_fd);
    return 0;
}