#pragma once
#include "node.h"
#include <string>

using namespace std;


struct ClientSession {
    int id;
    int socketFd;   // -1 when not connected or no socket
    string cwdPath = "/";
};

Node* sessionCwd(ClientSession& s,Node* root,bool &wasReset);

void sessionSetCwd(ClientSession& s,Node* newCwd);