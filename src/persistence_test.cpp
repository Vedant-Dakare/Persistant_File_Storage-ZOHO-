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

    auto loadedRoot = loadFilesystem("test_fs.img");
    std::cout << "load -> " << (loadedRoot ? "OK" : "FAILED") << "\n";

    if (loadedRoot) {
        std::cout << "pwd check: " << pwd(loadedRoot.get()) << "\n";
        std::cout << "ls /:\n";
        for (auto& n : ls(loadedRoot.get())) std::cout << "  " << n << "\n";

        auto r = resolvePath(loadedRoot.get(), loadedRoot.get(), "/home/user");
        std::cout << "ls /home/user:\n";
        for (auto& n : ls(r.node)) std::cout << "  " << n << "\n";
    }
    // loadedRoot is a unique_ptr now -- it cleans itself up automatically
    // here at the end of main(), no leak, no manual delete needed.
    return 0;
}