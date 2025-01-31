#include <boost/asio.hpp>
#include <iostream>
#include <memory>

using boost::asio::ip::tcp;

// Function to handle a single client connection
void handle_client(tcp::socket socket) {
    try {
        std::string message = "Hello from Server!";
        boost::asio::write(socket, boost::asio::buffer(message));
        std::cout << "Message sent to client: " << message << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Error handling client: " << e.what() << std::endl;
    }
}

int main() {
    try {
        boost::asio::io_context io_context;

        // Create a TCP acceptor on port 12345
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12345));
        std::cout << "Server is running on port 12345, waiting for connections..." << std::endl;

        while (true) {
            // Wait for a client connection
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            std::cout << "Client connected!" << std::endl;

            // Handle the client connection in a separate thread (optional)
            std::thread(handle_client, std::move(socket)).detach();
        }
    } catch (std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    return 0;
}
