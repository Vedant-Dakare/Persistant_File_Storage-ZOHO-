#include "../include/protocol.h"
#include <sstream>
#include <algorithm>
#include <sys/socket.h>
#include <optional>


using namespace std;

static string toUpper(string &s) {
    // for(int i=0;i<s.size();i++) {
    //     s[i] = toupper(s[i]);
    // }
    // stl ->
    transform(s.begin(),s.end(),s.begin(), ::toupper);
    
    return s;
} 

static string trim(const string &s) {
    size_t b = s.find_first_not_of(" \t\r");
    if(b == string::npos) return "";
    size_t e = s.find_last_not_of(" \t\r");
    return s.substr(b,e-b+1);
}

ParsedRequest parseRequest(const string& line) {
    string s = trim(line);
    
    if(!s.empty() && s.back() == '\r') {
        s.pop_back();
    }

    // return first occ or the space
    size_t sp = s.find(' ');
    string cmd = (sp == string::npos) ? s : s.substr(0,sp);
    string arg = (sp == string :: npos) ? "" : s.substr(sp+1);
    
    if(cmd.empty()) {
        return {false,"",""};
    }

    return {true,toUpper(cmd),arg};
}

string buildResponse(bool ok,const string &codeOrBody,const string &body) {

    std::ostringstream out;

    if(ok) {
        out << "OK " << codeOrBody.size()<< "\n" << codeOrBody;
    }
    else {
        out << "ERR " << codeOrBody << " " << body.size()<< "\n" << body;
    }

    return out.str();
}


optional<string> readLine(Connection &conn) {
    while(true) {
        // Do we already have a full line buffered from a previous recv()?
        size_t nl = conn.recvBuf.find('\n');
        if(nl != string::npos) {
            string line = conn.recvBuf.substr(0,nl);
            conn.recvBuf.erase(0,nl+1);
            return line;
        }

        char chunk[512];
        ssize_t n = recv(conn.fd,chunk,sizeof(chunk),0);
        if(n < 0) {
            return nullopt;
        }
        if(n == 0) {
            return nullopt;
        }

        conn.recvBuf.append(chunk,n);

        if(conn.recvBuf.size() > 4096) {
            return nullopt;
        }
    }
}

bool sendAll(int fd,const string &data) {
    size_t total = 0;
    while(total < data.size()) {
        ssize_t n = send(fd,data.data() + total,data.size() - total,0);
        if(n <= 0) {
            return false;
        }
        total += (size_t)n;
    }
    return true;
}