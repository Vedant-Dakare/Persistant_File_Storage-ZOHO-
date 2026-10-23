#include "../include/node.h"
#include "../include/filesystem.h"
#include <iostream>
#include <memory>

void printTree(Node *node, int depth = 0)
{
        std::string label =
            (node->name == "/")
                ? "/"
                : node->name + (node->isDirectory ? "/" : "");

        std::cout << std::string(depth * 2, ' ') << label << "\n";

        for (auto &child : node->children)
        {
                printTree(child.get(), depth + 1);
        }
}

int main()
{

        // Build root
        auto root = std::make_unique<Node>("/", true, nullptr);

        // mkdir /home
        auto home = std::make_unique<Node>("home", true, root.get());
        Node *homePtr = home.get();
        root->children.push_back(std::move(home));

        // mkdir /home/user
        auto user = std::make_unique<Node>("user", true, homePtr);
        Node *userPtr = user.get();
        homePtr->children.push_back(std::move(user));

        // touch /home/user/notes.txt
        auto notes = std::make_unique<Node>("notes.txt", false, userPtr);
        notes->content = "hello world";

        Node *notesPtr = notes.get();
        userPtr->children.push_back(std::move(notes));

        // Test pwd()
        std::cout << "pwd(root) = " << pwd(root.get()) << "\n";
        std::cout << "pwd(home) = " << pwd(homePtr) << "\n";
        std::cout << "pwd(user) = " << pwd(userPtr) << "\n";
        std::cout << "pwd(notes) = " << pwd(notesPtr) << "\n";

        printTree(root.get());

        // Test ls()
        std::cout << "\nls(root):\n";
        for (auto &name : ls(root.get()))
        {
                std::cout << "  " << name << "\n";
        }

        std::cout << "ls(user):\n";
        for (auto &name : ls(userPtr))
        {
                std::cout << "  " << name << "\n";
        }

        std::cout << "\n--- mkdir tests ---\n";

        FsResult r1 = mkdir(userPtr, "projects");
        std::cout << "mkdir /home/user/projects -> "
                  << (r1 == FsResult::OK ? "OK" : "FAILED") << "\n";

        FsResult r2 = mkdir(userPtr, "projects"); // duplicate, should fail
        std::cout << "mkdir /home/user/projects (again) -> "
                  << (r2 == FsResult::ALREADY_EXISTS ? "ALREADY_EXISTS (expected)" : "UNEXPECTED") << "\n";

        std::cout << "\nls(user) after mkdir:\n";
        for (auto &name : ls(userPtr))
        {
                std::cout << "  " << name << "\n";
        }

        std::cout << "\n--- touch tests ---\n";

        FsResult t1 = touch(userPtr, "readme.txt");
        std::cout << "touch readme.txt -> " << (t1 == FsResult::OK ? "OK" : "FAILED") << "\n";

        FsResult t2 = touch(userPtr, "readme.txt"); // touch again, should still be OK (timestamp update)
        std::cout << "touch readme.txt (again) -> " << (t2 == FsResult::OK ? "OK (timestamp updated)" : "FAILED") << "\n";

        FsResult t3 = touch(userPtr, "projects"); // name collision with existing directory
        std::cout << "touch projects (dir exists) -> "
                  << (t3 == FsResult::ALREADY_EXISTS ? "ALREADY_EXISTS (expected)" : "UNEXPECTED") << "\n";

        std::cout << "\nls(user) after touch:\n";
        for (auto &name : ls(userPtr))
        {
                std::cout << "  " << name << "\n";
        }

        std::cout << "\n--- cd tests ---\n";

        CdResult c1 = cd(userPtr, "projects");
        std::cout << "cd projects -> "
                  << (c1.status == FsResult::OK ? "OK, now at " + pwd(c1.newCwd) : "FAILED") << "\n";

        CdResult c2 = cd(c1.newCwd, "..");
        std::cout << "cd .. -> "
                  << (c2.status == FsResult::OK ? "OK, now at " + pwd(c2.newCwd) : "FAILED") << "\n";

        CdResult c3 = cd(userPtr, "notes.txt"); // trying to cd into a file
        std::cout << "cd notes.txt -> "
                  << (c3.status == FsResult::NOT_A_DIRECTORY ? "NOT_A_DIRECTORY (expected)" : "UNEXPECTED") << "\n";

        CdResult c4 = cd(userPtr, "doesnotexist");
        std::cout << "cd doesnotexist -> "
                  << (c4.status == FsResult::NOT_FOUND ? "NOT_FOUND (expected)" : "UNEXPECTED") << "\n";

        std::cout << "\n--- rm tests ---\n";

        FsResult rm1 = rm(userPtr, "readme.txt");
        std::cout << "rm readme.txt -> " << (rm1 == FsResult::OK ? "OK" : "FAILED") << "\n";

        FsResult rm2 = rm(userPtr, "projects"); // non-empty dir, should be rejected
        std::cout << "rm projects (non-empty) -> "
                  << (rm2 == FsResult::NOT_EMPTY ? "NOT_EMPTY (expected)" : "UNEXPECTED") << "\n";

        FsResult rm3 = rm(userPtr, "doesnotexist");
        std::cout << "rm doesnotexist -> "
                  << (rm3 == FsResult::NOT_FOUND ? "NOT_FOUND (expected)" : "UNEXPECTED") << "\n";

        std::cout << "\nls(user) after rm:\n";
        for (auto &name : ls(userPtr))
        {
                std::cout << "  " << name << "\n";
        }

        std::cout << "\n--- rm -rf tests ---\n";

        // Put something inside projects first, so we're actually testing recursive delete
        mkdir(userPtr, "projects2");
        CdResult intoProjects2 = cd(userPtr, "projects2");
        touch(intoProjects2.newCwd, "a.txt");
        mkdir(intoProjects2.newCwd, "nested");

        std::cout << "ls(user) before rm -rf:\n";
        for (auto &name : ls(userPtr))
                std::cout << "  " << name << "\n";

        FsResult rrf = rmrf(userPtr, "projects2");
        std::cout << "rm -rf projects2 -> " << (rrf == FsResult::OK ? "OK" : "FAILED") << "\n";

        std::cout << "ls(user) after rm -rf:\n";
        for (auto &name : ls(userPtr))
                std::cout << "  " << name << "\n";

        std::cout << "\n--- ls -l test ---\n";
        for (auto &entry : lsLong(userPtr))
        {
                std::cout << (entry.isDirectory ? "d" : "-") << "  "
                          << "size=" << entry.size << "  "
                          << "modified=" << ctime(&entry.modifiedAt);
        }

        std::cout << "\n--- resolvePath tests ---\n";

        auto path1 = resolvePath(userPtr, root.get(), "/home/user/notes.txt");
        std::cout << "/home/user/notes.txt -> "
                  << (path1.status == FsResult::OK ? pwd(path1.node) : "FAILED")
                  << "\n";

        auto path2 = resolvePath(userPtr, root.get(), "../../home");
        std::cout << "../../home (from /home/user) -> "
                  << (path2.status == FsResult::OK ? pwd(path2.node) : "FAILED")
                  << "\n";

        auto path3 = resolvePath(userPtr, root.get(), "./notes.txt");
        std::cout << "./notes.txt -> "
                  << (path3.status == FsResult::OK ? pwd(path3.node) : "FAILED")
                  << "\n";

        auto path4 = resolvePath(userPtr, root.get(), "notes.txt/something");
        std::cout << "notes.txt/something -> "
                  << (path4.status == FsResult::NOT_A_DIRECTORY
                          ? "NOT_A_DIRECTORY (expected)"
                          : "UNEXPECTED")
                  << "\n";

        auto path5 = resolvePath(userPtr, root.get(), "/nowhere/at/all");
        std::cout << "/nowhere/at/all -> "
                  << (path5.status == FsResult::NOT_FOUND
                          ? "NOT_FOUND (expected)"
                          : "UNEXPECTED")
                  << "\n";

        std::cout << "\n--- path-based command tests ---\n";
        auto show = [](const char *label, FsResult r)
        {
                static const char *names[] = {"OK", "ALREADY_EXISTS", "NOT_A_DIRECTORY",
                                              "NOT_FOUND", "NOT_EMPTY", "INVALID_PATH"};
                std::cout << label << " -> " << names[(int)r] << "\n";
        };

        Node *R = root.get();
        show("mkdir /home/user/a", fsMkdir(userPtr, R, "/home/user/a"));
        show("mkdir a/b   (relative)", fsMkdir(userPtr, R, "a/b"));
        show("touch a/b/f.txt", fsTouch(userPtr, R, "a/b/f.txt"));
        show("mkdir nope/x (parent missing)", fsMkdir(userPtr, R, "nope/x"));
        show("mkdir notes.txt/x (parent is file)", fsMkdir(userPtr, R, "notes.txt/x"));
        show("mkdir /   ", fsMkdir(userPtr, R, "/"));
        show("rm a (non-empty)", fsRm(userPtr, R, "a"));
        show("rm -rf /home/user/a", fsRmrf(userPtr, R, "/home/user/a"));
        show("rm ..", fsRm(userPtr, R, ".."));

        CdResult c = fsCd(userPtr, R, "../..");
        std::cout << "cd ../.. -> " << pwd(c.newCwd) << "\n";

        return 0;
}