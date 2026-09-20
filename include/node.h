#pragma once

#include <string>
#include <vector>
#include <memory>
#include <ctime>
#include <utility>

using namespace std;

struct Node {
    string name;
    bool isDirectory;

    Node* parent;
    vector<unique_ptr<Node>> children;

    string content;

    time_t createdAt;
    time_t modifiedAt;

    Node(string name, bool isDir, Node* parent)
        : name(move(name)),
          isDirectory(isDir),
          parent(parent)
    {
        createdAt = modifiedAt = time(nullptr);
    }
};
