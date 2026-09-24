#include "../include/node.h"
#include "../include/filesystem.h"
#include <string>
#include <vector>

using namespace std;

// print working directory path
string pwd(Node* current) {
    // already at the root --> "/"
    if(current -> parent == nullptr) {
        return "/";
    }

    vector<string> parts;
    Node* node = current;

    while(node -> parent != NULL) {
        parts.push_back(node -> name);
        node = node -> parent;
    }

    string result;
    for(auto it= parts.rbegin(); it != parts.rend();it++) {
        result += "/" + *it;
    }
    
    return result;
}

vector<string> ls(Node* current) {
    vector<string> names;

    if(!current -> isDirectory) {
        names.push_back(current -> name);
        return names;
    }

    for(auto &child : current -> children) {
        names.push_back(child -> name + (child -> isDirectory ? "/" : ""));
    }

    return names;
}

FsResult mkdir(Node* cwd, const string &name) {
    if(!cwd -> isDirectory) {
        return FsResult::NOT_A_DIRECTORY;
    }

    for(auto &child : cwd -> children) {
        if(child -> name == name) {
            return FsResult::ALREADY_EXISTS;
        }
    }

    // createing a new dir node and adding it to cwd's children ,true -> means it is a dir
    auto newDir = make_unique<Node> (name,true,cwd);
    cwd -> children.push_back(move(newDir));

    return FsResult::OK;
}

FsResult touch(Node* cwd,const string &name) {
    if(!cwd -> isDirectory) {
        return FsResult::NOT_A_DIRECTORY;
    }

    for(auto &child : cwd -> children) {
        if(child -> name == name) {
            return FsResult::ALREADY_EXISTS;
        }
        // file already exists, just update its timestamps
        child -> modifiedAt = time(nullptr);
        return FsResult::OK;
    }
    
    // false-> means it is a file
    auto newFile = make_unique<Node> (name,false,cwd);
    cwd -> children.push_back(move(newFile));
    return FsResult::OK;
}

CdResult cd(Node* cwd,const string &name) {
    if(name == ".") {
        return {FsResult::OK,cwd};
    }
    if(name == "..") {
        Node* target = (cwd -> parent != nullptr) ? cwd -> parent : cwd;
        return {FsResult::OK,target};
    }

    if(!cwd -> isDirectory) {
        return {FsResult::NOT_A_DIRECTORY,nullptr};
    }

    for(auto &child : cwd -> children) {
        if(child -> name == name) {
            if(!child -> isDirectory) {
                return {FsResult::NOT_A_DIRECTORY,nullptr};
            }
            return {FsResult::OK,child.get()};
        }
    }
    return {FsResult::NOT_FOUND,nullptr};
}