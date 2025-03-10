#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <ctime>
#include <chrono>
#include "utility.h"

using boost::asio::ip::tcp;

std::unordered_map<int, std::pair<std::vector<int>, std::vector<std::string>>> listOfpayers;
int totalAmount = 0;

//sent total amount to NPCI
void transferMoney(){
    try{
        
        //connect to server at 8085(NPCI)
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("", "8085");
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);
        std::cout << "Connected to server on port 8085\n";

        //write amount
        boost::asio::write(socket, boost::asio::buffer(makemsg(-1,3,totalAmount,"NA","NA")));
        totalAmount = 0;
        
    }
    catch(std::exception &e){
        std::cerr << "Exeption: " <<e.what() << std::endl;
    }

}

void handle_client_from_PSP( std::pair<std::vector<int>, std::vector<std::string>> messageDetails){
    listOfpayers[messageDetails.first[0]] = messageDetails;
}

void handle_client_from_NPCI( std::pair<std::vector<int>, std::vector<std::string>> messageDetails){
    std::pair<std::vector<int>, std::vector<std::string>> messageDetails_PSP = listOfpayers[messageDetails.first[0]];
    std::vector<std::vector<int>> shares = {stringToShare(messageDetails.second[1]), stringToShare(messageDetails_PSP.second[1])};
    int amount = std::stoi(reconstructStringFromShares(shares, 127));
    
    totalAmount = totalAmount + amount;
    //return status
    boost::asio::write(socket, boost::asio::buffer(makemsg(messageDetails.first[0],2, 1, messageDetails.second[0], "")));


}

// Function to handle a single client connection
void handle_client(tcp::socket socket) {
    try {
        std::cout << "Client connected!" << std::endl;

        // Read a message from the client
        boost::asio::streambuf buffer;
        boost::asio::read_until(socket, buffer, '\n');
        std::cout << "recived message" << std::endl;
        std::istream input_stream(&buffer);
        std::string messagerevived;
        std::getline(input_stream, messagerevived);
        std::cout << "messagerevived : " << messagerevived << std::endl;
        
        std::pair<std::vector<int>,std::vector<std::string>> message = parsemsg(messagerevived);
        if(message.first[2] == 0){
            //message from NPCI
            handle_client_from_NPCI(message);

        }
        else if(message.first[2] == 1){
            //message from payerPSP
            handle_client_from_PSP(message);
        }

    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
}

int main() {
    try {
        boost::asio::io_context io_context;

        // Create a TCP acceptor on port 8086
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8086));
        std::cout << "Server is waiting for connections on port 8086..." << std::endl;

        while (true) {
            // Wait for a client connection
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            // Start a new thread to handle the client
            std::thread client_thread(handle_client, std::move(socket));
            client_thread.detach();  // Detach thread to run independently
        
            auto now = std::chrono::system_clock::now();
            std::time_t now_time = std::chrono::system_clock::to_time_t(now);
            std::tm *local_time = std::localtime(&now_time);
            if(local_time->tm_sec % 3 == 2){
                transferMoney();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    return 0;
}
