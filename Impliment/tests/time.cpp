#include <iostream>
#include <chrono>
#include <ctime>

int main() {
    // Get current time
    auto now = std::chrono::system_clock::now();
    
    // Convert to time_t
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // Convert to local time
    std::tm* local_time = std::localtime(&now_time);

    // Print only seconds
    std::cout << local_time->tm_sec << std::endl;

    return 0;
}
