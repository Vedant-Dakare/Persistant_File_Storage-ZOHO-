#include "../include/node.h"
#include "../include/filesystem.h"
#include "../include/session.h"
#include <iostream>
#include <memory>


int main() {
    auto rootOwner = std::make_unique<Node>("/", true, nullptr);
    Node* R = rootOwner.get();
    
    fsMkdir(R, R, "/tmp");
    fsMkdir(R, R, "/tmp/work");
    fsMkdir(R, R, "/home");

    ClientSession A{1, -1};
    ClientSession B{2, -1};
    bool reset;


    // A: cd /tmp/work
    Node* cwdA = sessionCwd(A, R, reset);
    sessionSetCwd(A, fsCd(cwdA, R, "/tmp/work").newCwd);

    // B: cd /home
    Node* cwdB = sessionCwd(B, R, reset);
    sessionSetCwd(B, fsCd(cwdB, R, "/home").newCwd);

    std::cout << "A pwd: " << A.cwdPath << "\n";   // /tmp/work
    std::cout << "B pwd: " << B.cwdPath << "\n";   // /home

      // B deletes A's directory
    cwdB = sessionCwd(B, R, reset);
    std::cout << "B: rm -rf /tmp/work -> "
            << (fsRmrf(cwdB, R, "/tmp/work") == FsResult::OK ? "OK" : "FAILED") << "\n";

    // A's next request
    cwdA = sessionCwd(A, R, reset);
    std::cout << "A next request: reset=" << (reset ? "yes" : "no")
            << ", now at " << pwd(cwdA) << "\n";   // reset=yes, now at /
}