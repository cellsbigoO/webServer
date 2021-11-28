


#include <iostream>
#include "EventLoop.h"
#include "HttpServer.h"
#include "EchoServer.h"

EventLoop *mainlp;

int main(int argc, char *argv[]) {

    int port = 1234;
    int iothreadnum = 4;
    int workerthreadnum = 4;
    if(argc == 4) {
        port = atoi(argv[1]);
        iothreadnum = atoi(argv[2]);
        workerthreadnum = atoi(argv[3]);
    }
    // printf("%d %d %d", port, iothreadnum, workerthreadnum);
    EVentLoop loop;
    mainlp = &loop;
    HttpServer httpserver(&loop, port, iothreadnum, workerthreadnum);
    httpserver.Start();
    try{
        loop.loop();
    } catch(std::bad_alloc &ba) {
        std::cerr << "bad_alloc caught in ThreadPool::ThreadFunc task: " << ba.what() << '\n';
    }
}