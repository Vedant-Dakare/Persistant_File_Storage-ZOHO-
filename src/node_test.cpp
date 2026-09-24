#include "../include/node.h"
#include "../include/filesystem.h"
#include <iostream>
#include <memory>

void printTree(Node* node, int depth = 0) {
    std::string label =
        (node->name == "/")
        ? "/"
        : node->name + (node->isDirectory ? "/" : "");

    std::cout << std::string(depth * 2, ' ') << label << "\n";

    for (auto& child : node->children) {
        printTree(child.get(), depth + 1);
    }
}

int main() {

    // Build root
    auto root = std::make_unique<Node>("/", true, nullptr);

    // mkdir /home
    auto home = std::make_unique<Node>("home", true, root.get());
    Node* homePtr = home.get();
    root->children.push_back(std::move(home));

    // mkdir /home/user
    auto user = std::make_unique<Node>("user", true, homePtr);
    Node* userPtr = user.get();
    homePtr->children.push_back(std::move(user));

    // touch /home/user/notes.txt
    auto notes = std::make_unique<Node>("notes.txt", false, userPtr);
    notes->content = "hello world";

    Node* notesPtr = notes.get();
    userPtr->children.push_back(std::move(notes));

    // Test pwd()
    std::cout << "pwd(root) = " << pwd(root.get()) << "\n";
    std::cout << "pwd(home) = " << pwd(homePtr) << "\n";
    std::cout << "pwd(user) = " << pwd(userPtr) << "\n";
    std::cout << "pwd(notes) = " << pwd(notesPtr) << "\n";

    printTree(root.get());


    // Test ls()
    std::cout << "\nls(root):\n";
    for (auto& name : ls(root.get())) {
        std::cout << "  " << name << "\n";
    }

    std::cout << "ls(user):\n";
    for (auto& name : ls(userPtr)) {
        std::cout << "  " << name << "\n";
    }

    return 0;
}