#include "../include/session.h"
#include "../include/filesystem.h"

using namespace std;


Node* sessionCwd(ClientSession& s,Node* root,bool &wasReset) {
    ResolveResult r = resolvePath(root,root,s.cwdPath);

    if(r.status == FsResult::OK && r.node -> isDirectory) {
        wasReset = false;
        return r.node;
    }
    s.cwdPath = "/";
    wasReset = true;
    return root;
}

void sessionSetCwd(ClientSession &s,Node* newCwd) {
    s.cwdPath = pwd(newCwd);
}