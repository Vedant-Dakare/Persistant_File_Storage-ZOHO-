#pragma once
#include "node.h"
#include <string>
#include <vector>

using namespace std;

string pwd(Node* current);

// ls -> display the contents of the current directory
vector<string> ls(Node* current);


enum class FsResult {
    OK,
    ALREADY_EXISTS,
    NOT_A_DIRECTORY,
    NOT_FOUND
};

// mkdir -> create a new directory in the current working directory
FsResult mkdir(Node* cwd, const string &name);


// used to update file timestamps and create empty files
FsResult touch(Node* cwd, const string &name);

// cd -> change the current working directory
struct CdResult {
    FsResult status;
    Node* newCwd;  // valid only if result == FsResult::OK
};

CdResult cd(Node* cwd,const string &name);