#pragma once
#include "node.h"
#include "session.h"
#include "protocol.h"
#include <string>

using namespace std;

string handleRequest(const ParsedRequest &req,ClientSession &session,Node* root);