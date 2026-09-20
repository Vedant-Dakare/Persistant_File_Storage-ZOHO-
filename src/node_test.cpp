#include "../include/node.h"
#include <iostream>
#include <memory>

void printTree(Node* node, int depth = 0) {
    std::string label = (node->name == "/") ? "/" : node->name + (node->isDirectory ? "/" : "");
    std::cout << std::string(depth * 2, ' ') << label << "\n";
    for (auto& child : node->children) {
        printTree(child.get(), depth + 1);
    }
}

int main() { 
    // Build root manually: root has no parent, so parent = nullptr.
    auto root = std::make_unique<Node>("/", true, nullptr);

    // mkdir /home
    auto home = std::make_unique<Node>("home", true, root.get());
    Node* homePtr = home.get();          // keep a raw pointer before moving ownership away
    root->children.push_back(std::move(home));

    // mkdir /home/user
    auto user = std::make_unique<Node>("user", true, homePtr);
    Node* userPtr = user.get();
    homePtr->children.push_back(std::move(user));

    // touch /home/user/notes.txt
    auto notes = std::make_unique<Node>("notes.txt", false, userPtr);
    notes->content = "hello world";
    userPtr->children.push_back(std::move(notes));

    printTree(root.get());

    return 0;
}