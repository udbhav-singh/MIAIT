#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <memory>
#include <queue>
#include "utility.h"
using boost::asio::ip::tcp;

std::queue<std::string> qClientID;
int messageID = 0;
pthread_mutex_t lock; 


// Functions to find additive shares

// Function to find a random prime number greater than the maximum ASCII value
int generateRandomPrime(int minValue) {
    std::srand(std::time(0));
    while (true) {
        int candidate = minValue + std::rand() % 100; // Random value above minValue
        bool isPrime = true;
        for (int i = 2; i <= std::sqrt(candidate); ++i) {
            if (candidate % i == 0) {
                isPrime = false;
                break;
            }
        }
        if (isPrime) return candidate;
    }
}

// Function to generate additive shares for a string
std::vector<std::vector<int>> stringToAdditiveShares(const std::string &secret, int primeMod) {
    std::vector<std::vector<int>> shares(2, std::vector<int>(secret.size())); // Always 2 shares
    std::srand(std::time(0)); // Seed for randomness

    for (size_t i = 0; i < secret.size(); ++i) {
        int asciiVal = static_cast<int>(secret[i]);

        // Generate the first random share
        shares[0][i] = std::rand() % primeMod;

        // Calc

    return shares;
}

// Function to reconstruct the string from additive shares
std::string reconstructStringFromShares(const std::vector<std::vector<int>> &shares, int primeMod) {
    std::string reconstructed;
    size_t strLength = shares[0].size();

    for (size_t i = 0; i < strLength; ++i) {
        int sum = 0;
        for (const auto &share : shares) {
            sum = (sum + share[i]) % primeMod;
        }
        reconstructed += static_cast<char>(sum);
    }

    return reconstructed;
}

// Function to share to string and visa-versa


// Function to convert a single share to a string
std::string shareToString(const std::vector<int> &share) {
    std::ostringstream oss;
    for (size_t i = 0; i < share.size(); ++i) {
        oss << share[i];
        if (i != share.size() - 1) {
            oss << ","; // Separate values with a comma
        }
    }
    return oss.str();
}

// Function to convert a string back to a single share (vector)
std::vector<int> stringToShare(const std::string &shareStr) {
    std::vector<int> share;
    std::istringstream iss(shareStr);
    std::string valueStr;

    // Split the string by commas to extract values
    while (std::getline(iss, valueStr, ',')) {
        share.push_back(std::stoi(valueStr));
    }

    return share;
}




// Function to handle incoming client connections on port 8083
void handle_client(tcp::socket socket) {
    try {
        std::cout << "Client connected from: " << socket.remote_endpoint() << std::endl;

        int messageIDlocal;
        pthread_mutex_lock(&lock); 

        messageID++;
        messageIDlocal = messageID;
        pthread_mutex_unlock(&lock); 
 
        
        std::cout<<"i am here"<<std::endl;
        // Buffer to store clientID------------------------------
        boost::asio::streambuf buffer;
        boost::asio::read_until(socket, buffer, '\n');
        std::cout<<"i am here1"<<std::endl;

        // Extract clientID from the buffer----------------------------------
        std::istream input_stream(&buffer);
        std::string clientID;
        std::getline(input_stream, clientID);
        std::cout << "Client ID received: " << clientID << std::endl;
        std::cout<<"i am here2"<<std::endl;
        //encript ClientID(to be done)------------------------------------------

        //Distribute clientID to additive shares--------------------------------
        int primeMod = generateRandomPrime(127);

        std::vector<std::vector<int>> shares = stringToAdditiveShares(clientID, primeMod);
        
        std::string share1Str = shareToString(shares[0]);
        std::string share2Str = shareToString(shares[1]);

        share1Str =makemsg(messageIDlocal,0,0,share1Str,-1);// std::to_string(messageIDlocal) + ',' + share1Str + ',' + '0';
        share2Str =makemsg(messageIDlocal,1,1,share2Str,-1);// std::to_string(messageIDlocal) + ',' + share2Str + ',' + '1';
        //Senting share to payee device
        boost::asio::write(socket, boost::asio::buffer(share1Str));
        std::cout << "Message sent to client: " << share1Str << std::endl;

        qClientID.push(share2Str);


        //extras========================================

        // Print the chosen prime and the shares
        std::cout << "Prime Modulus: " << primeMod << std::endl;
        std::cout << "Shares:" << std::endl;
        for (const auto &share : shares) {
            for (int val : share) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
        // Reconstruct the string
        std::string reconstructedSecret = reconstructStringFromShares(shares, primeMod);
        std::cout << "Reconstructed Secret: " << reconstructedSecret << std::endl;



    } catch (const std::exception& e) {
        std::cerr << "Error handling client: " << e.what() << std::endl;
    }
}

// Function to start listening on port 8083 for incoming connections
void start_server(boost::asio::io_context& io_context, unsigned short port) {
    try {
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
        std::cout << "Server listening on port " << port << std::endl;

        for (;;) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            std::thread(handle_client, std::move(socket)).detach();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error starting server: " << e.what() << std::endl;
    }
} 

int main() {
    try {
        boost::asio::io_context io_context;

        // Start the server in a separate thread
        std::thread server_thread([&io_context]() {
            start_server(io_context, 8083);
        });

        // Connect to the remote server (e.g., localhost) on port 8085
        connect_to_server(io_context, "", 8085);

        server_thread.join();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
