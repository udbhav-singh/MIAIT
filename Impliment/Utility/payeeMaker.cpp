#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <thread>

using boost::asio::ip::tcp;

// Function to handle a single client connection
void handle_client(tcp::socket socket, int clientID) {
    try {
        // Convert the clientID to a string to send it over the network
        std::string message = std::to_string(clientID) + "\n";
        boost::asio::write(socket, boost::asio::buffer(message));
        std::cout << "Message sent to client: " << message;
    } catch (std::exception& e) {
        std::cerr << "Error handling client: " << e.what() << std::endl;
    }
}

int main() {
    int clientID = 0;

    try {
        boost::asio::io_context io_context;

        // Create a TCP acceptor on port 8081
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8081));
        std::cout << "Server is running on port 8081, waiting for connections..." << std::endl;

        while (true) {
            // Wait for a client connection
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            std::cout << "Client connected!" << std::endl;

            // Handle the client connection in a separate thread
            std::thread(handle_client, std::move(socket), clientID).detach();
            clientID++;
        }
    } catch (std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    return 0;
}
