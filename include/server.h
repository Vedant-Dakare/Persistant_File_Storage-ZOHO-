#pragma once
#include "node.h"
#include "session.h"
#include "protocol.h"
#include <string>
#include <mutex>

using namespace std;

extern mutex fsMutex;   // one global lock protecting the whole filesystem tree

string handleRequest(const ParsedRequest &req,ClientSession &session,Node* root);

void clientHandler(int client_fd,int sessionId,Node* root);