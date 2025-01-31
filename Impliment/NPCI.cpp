#include <iostream>
#include <boost/asio.hpp>
#include <optional>
#include <vector>
#include <array>
#include <sstream>
#include <thread>
#include <mutex>
#include <algorithm>

using boost::asio::ip::tcp;

// Global storage for quadruples and a mutex for thread safety
std::vector<std::array<int, 4>> quadruples;
std::mutex quadruples_mutex;

// Function to sort quadruples by the first element (x)
void sort_quadruples() {
    std::sort(quadruples.begin(), quadruples.end(), [](const std::array<int, 4>& a, const std::array<int, 4>& b) {
        return a[0] < b[0];
    });
}

// Function to perform binary search to find a quadruple with a certain x
std::optional<std::array<int, 4>> binary_search_quadruple(int x) {
    std::lock_guard<std::mutex> lock(quadruples_mutex); // Ensure thread safety

    int left = 0, right = quadruples.size() - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2; // Avoid overflow
        int mid_x = quadruples[mid][0];

        if (mid_x == x) {
            return quadruples[mid]; // Found the quadruple
        } else if (mid_x < x) {
            left = mid + 1; // Search in the right half
        } else {
            right = mid - 1; // Search in the left half
        }
    }

    return std::nullopt; // Return empty if no quadruple is found
}

// Function to parse and store the received message
void process_message(const std::string& message) {
    try {
        std::istringstream iss(message);
        int x, y, z, w;
        char comma1, comma2, comma3;

        // Parse the message in the form of "int,int,int,int"
        if (iss >> x >> comma1 >> y >> comma2 >> z >> comma3 >> w && comma1 == ',' && comma2 == ',' && comma3 == ',') {
            std::cout << "Parsed quadruple: (" << x << ", " << y << ", " << z << ", " << w << ")" << std::endl;

            if (w == 0) { // Store the quadruple only if w == 0
                // Lock the mutex and store the quadruple
                std::lock_guard<std::mutex> lock(quadruples_mutex);
                quadruples.push_back({x, y, z, w});
                sort_quadruples();
                std::cout << "Stored quadruple: (" << x << ", " << y << ", " << z << ", " << w << ")" << std::endl;
            } else { // Handle the search case if w != 0
                auto result = binary_search_quadruple(x);
                if (result) {
                    const auto& [rx, ry, rz, rw] = *result;
                    std::cout << "Found quadruple: (" << rx << ", " << ry << ", " << rz << ", " << rw << ")" << std::endl;

                    
                } else {
                    std::cout << "No quadruple found with x = " << x << std::endl;
                }
            }
        } else {
            std::cerr << "Invalid message format: " << message << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error processing message: " << e.what() << std::endl;
    }
}

// Function to handle a client connection
void handle_client(tcp::socket socket) {
    try {
        std::cout << "Client connected from: " << socket.remote_endpoint() << std::endl;

        for (;;) {
            char data[1024] = {0};

            // Read data from the client
            size_t length = socket.read_some(boost::asio::buffer(data));
            std::string message(data, length);

            std::cout << "Received message: " << message << std::endl;

            // Process and store the message
            process_message(message);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling client: " << e.what() << std::endl;
    }
}

// Function to start the server
void start_server(boost::asio::io_context& io_context, unsigned short port) {
    try {
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
        std::cout << "Server listening on port " << port << std::endl;

        for (;;) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            std::thread(handle_client, std::move(socket)).detach();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error starting server: " << e.what() << std::endl;
    }
}

int main() {
    try {
        boost::asio::io_context io_context;

        // Start the server on port 8085
        start_server(io_context, 8085);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
