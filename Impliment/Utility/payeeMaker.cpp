#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <bits/stdc++.h>

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
std::vector<int> parsemsg(const std::string& input) {
    std::vector<int> numbers;
    std::stringstream ss(input);
    std::string token;
    
    while (std::getline(ss, token, ',')&& numbers.size() < 5) {
        numbers.push_back(std::stoi(token));
    }

    return numbers;
}
std::string makemsg(std::string a, int b, int c, int d, int e) {
    std::stringstream ss;
    ss << a << "," << b << "," << c << "," << d << "," << e<<",";
    return ss.str();
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
