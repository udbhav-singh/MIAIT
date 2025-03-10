#include <boost/asio.hpp>
#include <iostream>
#include <random>
#include "utility.h"

using boost::asio::ip::tcp;

int main() {
    //scan QR for testing it is implimented in QR
    std::string QRreceived;
    //amount is a random number genrated between 1 to 100
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<int> dist(1, 100); 

    int amount = dist(gen);

    try {
        boost::asio::io_context io_context;

        // Create a TCP acceptor on port 8089
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(),8089 ));
        std::cout << "Server is waiting for connections on port 8089..." << std::endl;

        // Wait for a client connection
        tcp::socket socket(io_context);
        acceptor.accept(socket);
        std::cout << "Client connected!" << std::endl;

        // Read a message from the client
        boost::asio::streambuf buffer;
        boost::asio::read_until(socket, buffer, '\n');
        std::cout<<"recived QR share from payee Device"<<std::endl;
        std::istream input_stream(&buffer);
        std::getline(input_stream, QRreceived);
        std::cout << "QRreceived received: " << QRreceived << std::endl;


    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    //add amount to QR
    std::pair<std::vector<int>,std::vector<std::string>>  QRtoSent = parsemsg(QRreceived);
    QRtoSent.second[1] = std::to_string(amount);
    std::string QRsent = makemsg(QRtoSent.first[0],QRtoSent.first[1],QRtoSent.first[2],QRtoSent.second[0],QRtoSent.second[1]);

    try {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("", std::to_string(8084));
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);
        std::cout << "Connected to server at port " << 8084 << std::endl;

        boost::asio::write(socket, boost::asio::buffer(QRsent));
        std::cout << "Message sent to PSP: " << QRsent<<std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error connecting to server: " << e.what() << std::endl;
    }

    return 0;
}
