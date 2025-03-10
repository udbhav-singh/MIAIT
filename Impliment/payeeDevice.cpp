#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include "utility.h"
using boost::asio::ip::tcp;

int main() {

    std::string clientID;
    // Getting client ID
    try {
        boost::asio::io_context io_context;

        // Connect to the server
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "8081");
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        std::cout << "Connected to server!" << std::endl;

        // Buffer to store the server's message
        boost::asio::streambuf buffer;
        boost::asio::read_until(socket, buffer, "\n");

        // Convert buffer to string and print the message
        std::istream input_stream(&buffer);
        std::string message;
        std::getline(input_stream, message);

        std::cout << "Message received from server: " << message << std::endl;

        clientID = message;
    } catch (std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }

    // Asking PSP to genrate QR(in this case just asking for an addetive share)
    try {
        boost::asio::io_context io_context;

        // Connect to the server
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "8083");
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        std::cout << "Connected to server!" << std::endl;

        // Sent ClientID
        std::string message1 = clientID;
        boost::asio::write(socket, boost::asio::buffer(message1));
        std::cout<<"sent:"<< message1<< std::endl;

        // Read a message from the server
        boost::asio::streambuf buffer;
        boost::asio::read_until(socket, buffer, '\n');
        std::cout<<"recived clientID share from PSP"<<std::endl;
        std::istream input_stream(&buffer);
        std::string QR;
        std::getline(input_stream, QR);
        std::cout << "QR received: " << QR << std::endl;

        try {
            boost::asio::io_context io_context;

            // Connect to the server at localhost:8089
            tcp::resolver resolver(io_context);
            auto endpoints = resolver.resolve("", "8089");
            tcp::socket socket(io_context);
            boost::asio::connect(socket, endpoints);

            // Send a message to the server
            boost::asio::write(socket, boost::asio::buffer(QR));
            std::cout << "Message sent: " << QR << std::endl;

        } catch (std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }

    } catch (std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }



    return 0;
}
