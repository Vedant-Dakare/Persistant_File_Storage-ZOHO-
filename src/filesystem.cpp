#include "../include/node.h"
#include <string>
#include <vector>

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