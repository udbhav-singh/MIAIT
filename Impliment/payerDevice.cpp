#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

int main() {
    //scan QR for testing it is implimented in QR
    std::string QR;
    int amount;
    std::cout<<"enter amounrt:";
    std::cin>>amount;
    try {
        boost::asio::io_context io_context;

        // Create a TCP acceptor on port 12345
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(),8089 ));
        std::cout << "Server is waiting for connections on port 8089..." << std::endl;

        // Wait for a client connection
        tcp::socket socket(io_context);
        acceptor.accept(socket);
        std::cout << "Client connected!" << std::endl;

        // Read a message from the client
        char share1ID[1024];
        size_t length = socket.read_some(boost::asio::buffer(share1ID));
        std::cout << "Received: " << std::string(share1ID, length) << std::endl;
        QR = share1ID + '+' + std::to_string(amount);
        std::cout<<QR<<std::endl;

    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    try {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("", std::to_string(8084));

        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        std::cout << "Connected to server at port " << 8084 << std::endl;


        std::cout << "Message sent to client: " << QR;
        boost::asio::write(socket, boost::asio::buffer(QR));

        

    } catch (const std::exception& e) {
        std::cerr << "Error connecting to server: " << e.what() << std::endl;
    }

    return 0;
}
