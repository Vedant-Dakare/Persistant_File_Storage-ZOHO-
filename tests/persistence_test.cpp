#include "../include/node.h"
#include "../include/filesystem.h"
#include "../include/persistence.h"
#include <iostream>
#include <memory>

int main() {
    {
        auto root = std::make_unique<Node>("/", true, nullptr);
        fsMkdir(root.get(), root.get(), "/home");
        fsMkdir(root.get(), root.get(), "/home/user");
        fsTouch(root.get(), root.get(), "/home/user/notes.txt");

        bool saved = saveFilesystem(root.get(), "test_fs.img");
        std::cout << "save -> " << (saved ? "OK" : "FAILED") << "\n";
        // root (and the whole tree) is destroyed here, at end of this block
    }

    auto loaded = loadFilesystem("test_fs.img");
    std::cout << "load -> " << (loaded.status == LoadStatus::OK ? "OK" : "FAILED") << "\n";

    if (loaded.status == LoadStatus::OK) {
        Node* r = loaded.root.get();
        std::cout << "pwd check: " << pwd(r) << "\n";
        for (auto& n : ls(r)) std::cout << "  " << n << "\n";
    }
    // loadedRoot is a unique_ptr now -- it cleans itself up automatically
    // here at the end of main(), no leak, no manual delete needed.
    return 0;
}