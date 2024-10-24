#include <chrono>
#include <iostream>
#include <thread>

#include "services/PermissionManager.h"

int main() {
    services::PermissionManager::Locate();
    int dotCount = 0;
    while (true) {
        std::cout << "Permission Manager working";
        for (int i = 0; i < 4; i++) {
            if (i < dotCount) {
                std::cout << '.';
            } else {
                std::cout << ' ';
            }
        }
        std::cout << std::endl;
        std::cout << "\e[1A";  // writing cursor up on 1 line
        ++dotCount %= 4;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    return 0;
}