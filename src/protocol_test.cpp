#include "../include/protocol.h"
#include <iostream>

void show(const std::string& line) {
    ParsedRequest r = parseRequest(line);
    std::cout << "\"" << line << "\" -> valid=" << r.valid
              << " cmd=[" << r.command << "] arg=[" << r.arg << "]\n";
}

int main() {
    show("MKDIR /home/user/newdir");
    show("mkdir my folder");      // lowercase command, multi-word arg -> arg should be "my folder"
    show("LS -l");
    show("PWD");
    show("");                     // invalid

    std::cout << "\n" << buildResponse(true, "home/\nprojects/\n");
    std::cout << "\n" << buildResponse(false, "NOT_FOUND", "cd: no such directory: /nowhere");
}