#include "../include/node.h"
#include "../include/filesystem.h"
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

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
            if(child -> isDirectory) {
                return FsResult::ALREADY_EXISTS;
            }
            // file already exists, just update its timestamps
            child -> modifiedAt = time(nullptr);
            return FsResult::OK;
        }
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

FsResult rm(Node* cwd,const string &name) {
    if(!cwd -> isDirectory) {
        return FsResult::NOT_A_DIRECTORY;
    }

    // find_if(start,end,condition)
    auto it = find_if(cwd -> children.begin(),cwd -> children.end(),
        [&](const unique_ptr<Node> &child) {
            return child -> name == name;
    });

    if(it == cwd -> children.end()) {
        return FsResult::NOT_FOUND;
    }

    if((*it) -> isDirectory && !(*it) -> children.empty()) {
        return FsResult::NOT_EMPTY;
    }

    cwd -> children.erase(it);
    return FsResult::OK;
}


FsResult rmrf(Node* cwd,const string &name) {
    if(!cwd -> isDirectory) {
        return FsResult::NOT_A_DIRECTORY;
    }

    auto it = find_if(cwd -> children.begin(),cwd -> children.end(),
        [&](const unique_ptr<Node> &child) {
            return child -> name == name;
    });
    
    if(it == cwd -> children.end()) {
        return FsResult::NOT_FOUND;
    }

    cwd -> children.erase(it);
    return FsResult::OK;
}

vector<LsResult>  lsLong(Node* current) {
    vector<LsResult> entries;
    if(!current -> isDirectory) {
        entries.push_back({current -> name, false, current -> content.size(), current -> modifiedAt});
        return entries;
    }

    for(auto &child : current -> children) {
        size_t size = child -> isDirectory ? child -> children.size() : child -> content.size();
        entries.push_back({child -> name, child -> isDirectory, size, child -> modifiedAt});
    }
    return entries;
}


vector<string> tokenizePath(const string &path) {
    vector<string> tokens;
    string current;

    for(char c : path) {
        if(c == '/') {
            if(!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        }
        else {
            current += c;
        }
    }
    if(!current.empty()) {
        tokens.push_back(current);
    }
    return tokens;
}


ResolveResult resolvePath(Node* cwd, Node* root, const string& path) {
    if(path.empty()) {
        return {FsResult::NOT_FOUND,nullptr};
    }

    Node* current = (path[0] == '/') ? root : cwd;
    vector<string> tokens = tokenizePath(path);

    for(const string &token : tokens) {
        if(token == ".") {
            continue;
        }
        if(token == "..") {
            current = (current -> parent != NULL) ? current -> parent : current;
            continue; 
        }
        if(!current -> isDirectory) {
            return {FsResult::NOT_A_DIRECTORY,nullptr};
        }

        Node* next = nullptr;
        for(auto &child : current -> children) {
            if(child -> name == token) {
                next = child.get();
                break;
            }
        }

        if(next == nullptr) {
            return {FsResult::NOT_FOUND,nullptr};
        }
        current = next;
    }
    return {FsResult::OK,current};
}
