#include <iostream>
#include <boost/asio.hpp>
#include <optional>
#include <vector>
#include <array>
#include <sstream>
#include <thread>
#include <mutex>
#include <algorithm>
#include <unordered_map>
#include <ctime>
#include <chrono>
#include "utility.h"

using boost::asio::ip::tcp;

std::unordered_map<int, std::pair<std::vector<int>, std::vector<std::string>>> listOfpayees;
std::vector<std::pair<std::vector<int>, std::vector<std::string>>> in_buffer;
int first_time = 0;

//to handle payer bank
void handle_payer_bank_client(std::pair<std::vector<int>, std::vector<std::string>> messagedetails){
    //this should be used to add all amount from different banks and sent credit requeset to all bank
    //but since we are using only one bank total amount will be sent to it

    //connenting to payer bank
    boost::asio::io_context io_context;
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve("", "8087");
    tcp::socket socket(io_context);
    boost::asio::connect(socket, endpoints);
    std::cout << "Connected to server on port 8085\n";

    //recive amount from bank
    boost::asio::streambuf buffer;
    boost::asio::read_until(socket, buffer, '\n');
    std::cout << "recived message" << std::endl;
    std::istream input_stream(&buffer);
    std::string messagerevived;
    std::getline(input_stream, messagerevived);
    std::cout << "messagerevived : " << messagerevived << std::endl;

    std::pair<std::vector<int>,std::vector<std::string>> message = parsemsg(messagerevived);

    


    
    
}

// to handle payee client
void handle_payee_client(std::pair<std::vector<int>, std::vector<std::string>> messagedetails)
{
    listOfpayees[messagedetails.first[0]] = messagedetails;
}

// to handle payer client
void handle_payer_client(std::pair<std::vector<int>, std::vector<std::string>> messagedetails)
{
    std::pair<std::vector<int>, std::vector<std::string>> messagedetails_fromPayee = listOfpayees[messagedetails.first[0]];
    // might possible we need to switch positions if do not work
    std::vector<std::vector<int>> shares = {stringToShare(messagedetails.second[0]), stringToShare(messagedetails_fromPayee.second[0])};
    std::string encryptedID = reconstructStringFromShares(shares, 127);
    std::pair<std::vector<int>, std::vector<std::string>> new_message = messagedetails;
    new_message.second[0] = encryptedID;
    // check if ID exist.......................................................(to be done)

    

    // step 10 time-stemps
    //  Get current time
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm *local_time = std::localtime(&now_time);
    std::cout << "time:" << local_time->tm_sec << std::endl;

    // this part can be used for parallel computing
    if (local_time->tm_sec % 3 == 0 || local_time->tm_sec % 3 == 1)
    {
        // green light, sent messagesIDs to bank
        //////////////////////////////////////////
        try
        {
            boost::asio::io_context io_context;
            tcp::resolver resolver(io_context);
            tcp::resolver::results_type endpoints = resolver.resolve("", "8089");
            tcp::socket socket(io_context);
            boost::asio::connect(socket, endpoints);
            std::cout << "Connected to server on port 8089\n";

            // for first time empty the buffer
            if (first_time == 0)
            {
                // empty in_buffer
                std::vector<std::pair<std::vector<int>, std::vector<std::string>>> statusOfBuffer;
                for (int i = 0; i < in_buffer.size(); i++)
                {
                    // write all IDs which were left in buffer
                    boost::asio::write(socket, boost::asio::buffer(makemsg(in_buffer[i].first[0], in_buffer[i].first[1], in_buffer[i].first[2], in_buffer[i].second[0], in_buffer[i].second[1])));

                    // recive status back
                    boost::asio::streambuf buffer;
                    boost::asio::read_until(socket, buffer, '\n');
                    std::istream input_stream(&buffer);
                    std::string status;
                    std::getline(input_stream, status);
                    std::cout << "status recived: " << status << std::endl;
                    statusOfBuffer.push_back(parsemsg(status));

                    // return status to PSP(connection not made yet)
                    try
                    {
                        boost::asio::io_context io_context;
                        tcp::resolver resolver(io_context);
                        tcp::resolver::results_type endpoints = resolver.resolve("", "8084");
                        tcp::socket socket(io_context);
                        boost::asio::connect(socket, endpoints);
                        std::cout << "Connected to server on port 8084\n";

                        // sent to PSP(connection not made in PSP)
                        boost::asio::write(socket, boost::asio::buffer(status));
                    }
                    catch (std::exception &e)
                    {
                        std::cerr << "Exception: " << e.what() << "\n";
                    }
                }
                first_time = 1;
                // empty vector buffer(to be done)
            }
            //write current stuff
            boost::asio::write(socket, boost::asio::buffer(makemsg(new_message.first[0], new_message.first[1], new_message.first[2], new_message.second[0], new_message.second[1])));
            
            // recive status back
            boost::asio::streambuf buffer;
            boost::asio::read_until(socket, buffer, '\n');
            std::istream input_stream(&buffer);
            std::string status;
            std::getline(input_stream, status);
            std::cout << "status recived: " << status << std::endl;

            // return status to PSP(connection not made yet)(to be done)
            try
            {
                boost::asio::io_context io_context;
                tcp::resolver resolver(io_context);
                tcp::resolver::results_type endpoints = resolver.resolve("", "8084");
                tcp::socket socket(io_context);
                boost::asio::connect(socket, endpoints);
                std::cout << "Connected to server on port 8084\n";
                // sent to PSP(connection not made in PSP)
                boost::asio::write(socket, boost::asio::buffer(status));
            }
            catch (std::exception &e)
            {
                std::cerr << "Exception: " << e.what() << "\n";
            }
        }
        catch (std::exception &e)
        {
            std::cerr << "Exception: " << e.what() << "\n";
        }
    }
    else
    {
        // buffer stop sending messageIDs,store it
        in_buffer.push_back(new_message);
        if (first_time == 1)
        {
            // steps 13 to 19 transfer money and sent status


            first_time = 0;
        }
    }
}

// Function to handle a client connection
void handle_client(tcp::socket socket)
{
    try
    {
        std::cout << "Client connected from: " << socket.remote_endpoint() << std::endl;

        while (1)
        {
            std::string messagerevived;

            // Read a message from the client
            boost::asio::streambuf buffer;
            boost::asio::read_until(socket, buffer, '\n');
            std::cout << "recived message" << std::endl;
            std::istream input_stream(&buffer);
            std::getline(input_stream, messagerevived);
            std::cout << "messagerevived : " << messagerevived << std::endl;

            std::pair<std::vector<int>, std::vector<std::string>> messageDetails = parsemsg(messagerevived);
            
            if (messageDetails.first[1] == 1)
            {
                // message is from psyee
                handle_payee_client(messageDetails);
            }
            else if(messageDetails.first[1] == 0)
            {
                // message is from payer
                handle_payer_client(messageDetails);
            }
            else if(messageDetails.first[1] == 3){
                //recive total amount from payer bank
                handle_payer_bank_client(messageDetails);
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error handling client: " << e.what() << std::endl;
    }
}

// Function to start the server
void start_server(boost::asio::io_context &io_context, unsigned short port)
{
    try
    {
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
        std::cout << "Server listening on port " << port << std::endl;

        while (1)
        {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            std::thread(handle_client, std::move(socket)).detach();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error starting server: " << e.what() << std::endl;
    }
}

int main()
{
    try
    {
        boost::asio::io_context io_context;
        // Start the server on port 8085
        start_server(io_context, 8085);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
