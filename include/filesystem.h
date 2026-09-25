#pragma once
#include "node.h"
#include <string>
#include <vector>

using namespace std;

// print working directory path
string pwd(Node* current);

// ls -> display the contents of the current directory
vector<string> ls(Node* current);


enum class FsResult {
    OK,
    ALREADY_EXISTS,
    NOT_A_DIRECTORY,
    NOT_FOUND,
    NOT_EMPTY
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

// rm -> remove a file or directory 
FsResult rm(Node* cwd,const string &name);

// rmrf -> remove a file or directory (recursively)
FsResult rmrf(Node* cwd, const string &name);

// ls -l -> display the contents of the current directory with details
struct LsResult {
    string name;
    bool isDirectory;
    size_t size;
    time_t modifiedAt;
};

vector<LsResult> lsLong(Node* current);