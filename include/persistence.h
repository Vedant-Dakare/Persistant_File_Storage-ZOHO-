#pragma once
#include "node.h"
#include <string>

using namespace std;

bool saveFilesystem(Node* root, const string &path);

// bool loadFilesystem(Node* &rootOut,const string &path);

enum class LoadStatus {NO_FILE, CORRUPTED,OK};

struct LoadResult {
    LoadStatus status;
    unique_ptr<Node> root;
};

LoadResult loadFilesystem(const string &path);