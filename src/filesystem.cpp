#include "../include/node.h"
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

// ls -> display the contents of the current directory
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