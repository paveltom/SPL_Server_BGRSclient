#include "../include/ConnectionHandler.h"
#include "../include/Task.h"
#include "../include/EncoderDecoder.h"
#include <iostream>
#include <condition_variable>

int main(int argc, char** argv) {

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " host port" << std::endl << std::endl;
        return -1;
    }

    std::string host = argv[1];
    short port = atoi(argv[2]);

    EncoderDecoder* encdec = new EncoderDecoder;
    ConnectionHandler connectionHandler(host, port);
    if (!connectionHandler.connect()) {
        std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        return 1;
    }

    std::mutex mutex;//probably not needed
    //KeyboardTask keyboardTask(connectionHandler, 1, mutex);
    NetTask netTask(connectionHandler, 2, mutex);
    std::thread::id mainThread = std::this_thread::get_id();
    std::thread th2(&Task::run, &netTask);
    std::condition_variable cv;
    th2.detach();

    while (1) {
        const short bufsize = 1024;
        char buf[bufsize];
        bool write = false;
        while (!write) {
            std::lock_guard<std::mutex> lockGuard(mutex);
            std::cin.getline(buf, bufsize);
            write = true;
        }
        std::string line(buf);;
        if (!connectionHandler.sendBytes(line)) {
            std::cout << "Disconnected. Exiting...\n" << std::endl;
            break;
        }
//ADMINREG morty a123
//        std::string answer;
//
//        if (!connectionHandler.getLine(answer)) {
//            std::cout << "Disconnected. Exiting...\n" << std::endl;
//            break;
//        }

//        len=answer.length();
//

        cv.notify_all();
        if (line == "LOGOUT") {
            std::cout << "Exiting...\n" << std::endl;
            break;
        }
    }
    th2.join();
    return 0;
}



