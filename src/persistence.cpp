#include "../include/persistence.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <iostream>

using namespace std;

static const uint32_t MAX_STRING_LEN = 16 * 1024* 1024;
static const uint32_t MAX_CHILD_COUNT = 1000000;


// low level write helper function

static bool writeAll(int fd,const void *buf,size_t len) {
    const char *p = (const char*)buf;
    size_t total = 0;
    while(total < len) {
        size_t n = write(fd,p+total,len-total);
        if(n <= 0) return false;
        total += (size_t)n;
    }
    return true;
}

static bool writeU32(int fd,uint32_t v) {
    return writeAll(fd,&v,sizeof(v));
}

static bool writeI64(int fd,int64_t v) {
    return writeAll(fd,&v,sizeof(v));
}

static bool writeString(int fd,const string &s) {
    uint32_t len = (uint32_t)s.size();
    if(!writeU32(fd,len)) return false;
    if(len == 0) return true;
    return writeAll(fd,s.data(),len);
}

// recursive node writing function
// take a node -> write its info -> write its children recursively
static bool writeNode(int fd,Node* node) {
    uint8_t isDir = node -> isDirectory ? 1 : 0;
    if(!writeAll(fd,&isDir,1)) return false;
    if(!writeString(fd,node -> name)) return false;
    if(!writeI64(fd,(int64_t)node -> createdAt)) return false;
    if(!writeI64(fd,(int64_t)node -> modifiedAt)) return false;
    if(!writeString(fd,node -> content)) return false;
    
    uint32_t childCount = (uint32_t)node -> children.size();
    if(!writeU32(fd,childCount)) return false;

    for(auto &child : node -> children) {
        if(!writeNode(fd,child.get())) return false;
    }
    return true;
}

bool saveFilesystem(Node* root,const string &path) {
    // O_WRONLY|O_CREAT|O_TRUNC: write-only, create if missing, overwrite if exists.
    int fd = open(path.c_str(),O_WRONLY | O_CREAT | O_TRUNC,0644);
    if(fd == -1) {
        perror("saveFilesystem: open");
        return false;
    }
    bool ok = writeNode(fd,root);
    close(fd);
    return ok;
}

// low level read helper function
static bool readAll(int fd,void *buf,size_t len) {
    char *p = (char*)buf;
    size_t total = 0;
    while(total < len) {
        size_t n = read(fd,p+total,len-total);
        if(n <= 0) return false;
        total += (size_t)n;
    }
    return true;
}

static bool readU32(int fd,uint32_t &v) {
    return readAll(fd,&v,sizeof(v));
}

static bool readI64(int fd,int64_t &v) {
    return readAll(fd,&v,sizeof(v));
}

bool static readString(int fd,string &out) {
    uint32_t len;
    if(!readU32(fd,len)) return false;
    if(len > MAX_STRING_LEN) return false;
    if(len == 0) {
        out.clear();
        return true;
    }
    out.resize(len);
    return readAll(fd,&out[0],len);
}


// recursive node reading function
// read node info -> read children recursively
static unique_ptr<Node> readNode(int fd,Node* parent,bool &ok) {
    uint8_t isDir;
    if(!readAll(fd,&isDir,1)) {
        ok = false;
        return nullptr;
    }

    string name;
    if(!readString(fd,name)) {
        ok = false;
        return nullptr;
    }

    auto node = make_unique<Node>(name,isDir != 0,parent);

    int64_t created,modified;
    if(!readI64(fd,created) || !readI64(fd,modified)) {
        ok = false;
        return nullptr;
    }

    node -> createdAt = (time_t)created;
    node -> modifiedAt = (time_t)modified;

    if(!readString(fd,node -> content)) {
        ok = false;
        return nullptr;
    }

    uint32_t childCount;
    if(!readU32(fd,childCount)) {
        ok = false;
        return nullptr;
    }

    if(childCount > MAX_CHILD_COUNT) {
        ok = false;
        return nullptr;
    }

    for(uint32_t i=0;i<childCount;i++) {
        auto child = readNode(fd,node.get(),ok);
        if(!ok) return nullptr;
        node -> children.push_back(move(child));
    }

    ok = true;
    return node;
}

LoadResult loadFilesystem(const string &path) {
    int fd = open(path.c_str(),O_RDONLY);
    if(fd == -1) {
        return {LoadStatus::NO_FILE,nullptr};
    }

    bool ok ;
    auto loadedRoot = readNode(fd,nullptr,ok);
    close(fd);
    if(!ok) {
        return {LoadStatus::CORRUPTED,nullptr};
    }

    return {LoadStatus::OK,move(loadedRoot)};
}