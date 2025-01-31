#include <boost/asio.hpp>
#include <iostream>
using boost::asio::ip::tcp;

int main() {    
    
    try {
        boost::asio::io_context io_context;

        // Create a TCP acceptor on port 12345
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(),8086 ));
        std::cout << "Server is waiting for connections on port 8086..." << std::endl;

        // Wait for a client connection
        tcp::socket socket(io_context);
        acceptor.accept(socket);
        std::cout << "Client connected!" << std::endl;

        // Read a message from the client
        char share1ID[1024];
        size_t length = socket.read_some(boost::asio::buffer(share1ID));
        std::cout << "Received: " << std::string(share1ID, length) << std::endl;


    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    
}