#pragma once
#include <string>
#include <optional>

using namespace std;


struct ParsedRequest {
    bool valid;
    string command;  // ex = mkdir, cd, ls
    string arg;      // ex = path or "-l" for ls, or "" for no arg
};


struct Connection {
    int fd;
    string recvBuf;
};


ParsedRequest parseRequest(const string& line);
string buildResponse(bool ok, const string &codeOrBody, const string &body = "");


optional<string> readLine(Connection &conn);
bool sendAll(int fd,const string &data);