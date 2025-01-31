#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io_context;

        // Connect to the server at localhost:12345
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("", "12345");
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        // Send a message to the server
        std::string message = "Hello from Client!"; 
        boost::asio::write(socket, boost::asio::buffer(message));
        std::cout << "Message sent: " << message << std::endl;

    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
