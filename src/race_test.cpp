#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/fs.sock"

using namespace std;


string sendCommand(const string &cmd) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path,SOCKET_PATH,sizeof(addr.sun_path)-1);

    if(connect(fd,(sockaddr*)&addr,sizeof(addr)) == -1) {
        close(fd);
        return "CONNECTION_FAILED";
    }

    string line = cmd + "\n";
    send(fd,line.data(),line.size(),0);

    char buff[512] = {0};
    ssize_t n = recv(fd,buff,sizeof(buff)-1,0);
    close(fd);
    return (n > 0) ? string(buff,n) : "NO_RESPONSE";
}

int main() {
    const int N = 20;
    vector<thread> threads;
    // atomic = a way to safely share a single value between multiple threads.
    atomic<int> okCount{0};
    atomic<int> existsCount{0};
    atomic<int> otherCount{0};

    // causes race condition occurrs here
    for(int i=0;i<N;i++) {
        threads.emplace_back([&]() {
            string response = sendCommand("MKDIR race_test");
            std::cout << "[got: " << response << "]\n"; 
            if(response.rfind("OK",0) == 0) okCount++;
            else if(response.find("EXISTS") != string::npos) existsCount++;
            else otherCount++;
        });
    }

    for(auto &t : threads) t.join();

    cout << "OK: " << okCount << " EXISTS:" << existsCount << " OTHER: " << otherCount << " (total " << N <<")\n";

    string lsResult = sendCommand("LS");
    int occurrences = 0;
    size_t pos = 0;
    while((pos = lsResult.find("race_test",pos)) != string::npos) {
        occurrences++;
        pos += 1;
    }
    cout << "race_test appears in ls " << occurrences << " time(s) (expected 1)\n";
}