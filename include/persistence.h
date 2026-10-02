#pragma once
#include "node.h"
#include <string>

using namespace std;

bool saveFilesystem(Node* root, const string &path);

// bool loadFilesystem(Node* &rootOut,const string &path);

unique_ptr<Node> loadFilesystem(const string &path);