#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <stdexcept>
#include <iomanip>
#include <crypto++/cryptlib.h>
#include <cstdlib>
#include <ctime>
#include<string>
#include <cmath>
#include <queue>
#include "utility.h"
// #include <aes.h>
// #include <modes.h>
// #include <filters.h>
// #include <secblock.h>
// #include <osrng.h> 


using boost::asio::ip::tcp;

std::queue<std::string> qToNPCI;
int messageID = 0;
pthread_mutex_t lock; 

// using namespace CryptoPP;

// // Encryption functions
// std::string encryptAES(const std::string& plaintext, const SecByteBlock& key, const byte iv[AES::BLOCKSIZE]) {
//     std::string ciphertext;

//     try {
//         CBC_Mode<AES>::Encryption encryption;
//         encryption.SetKeyWithIV(key, key.size(), iv);

//         StringSource(plaintext, true,
//             new StreamTransformationFilter(encryption,
//                 new StringSink(ciphertext)
//             )
//         );
//     } catch (const CryptoPP::Exception& e) {
//         std::cerr << "Encryption Error: " << e.what() << std::endl;
//         throw;
//     }

//     return ciphertext;
// }

// // Function to decrypt a ciphertext using AES-CBC
// std::string decryptAES(const std::string& ciphertext, const SecByteBlock& key, const byte iv[AES::BLOCKSIZE]) {
//     std::string decryptedtext;

//     try {
//         CBC_Mode<AES>::Decryption decryption;
//         decryption.SetKeyWithIV(key, key.size(), iv);

//         StringSource(ciphertext, true,
//             new StreamTransformationFilter(decryption,
//                 new StringSink(decryptedtext)
//             )
//         );
//     } catch (const CryptoPP::Exception& e) {
//         std::cerr << "Decryption Error: " << e.what() << std::endl;
//         throw;
//     }

//     return decryptedtext;
// }

// // Utility function to generate a random key and IV
// void generateKeyAndIV(SecByteBlock& key, byte iv[AES::BLOCKSIZE]) {
//     AutoSeededRandomPool rng;

//     // Generate a random key
//     key = SecByteBlock(AES::DEFAULT_KEYLENGTH);
//     rng.GenerateBlock(key, key.size());

//     // Generate a random IV
//     rng.GenerateBlock(iv, AES::BLOCKSIZE);
// }


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

        // Calculate the second share
        shares[1][i] = (asciiVal - shares[0][i] + primeMod) % primeMod;
    }

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

// Function to handle communication with a single client
void handle_client(tcp::socket socket) {
    try {
        //allocating messageID and storing it in a local variable
        int messageIDlocal;
        pthread_mutex_lock(&lock); 
        messageID++;
        messageIDlocal = messageID;
        pthread_mutex_unlock(&lock); 
        std::cout<<"messageID is:"<<messageIDlocal<<std::endl;
        
        // Buffer to store clientID
        boost::asio::streambuf buffer;
        boost::asio::read_until(socket, buffer, '\n');
        std::cout<<"recived clientID"<<std::endl;

        // Extract clientID from the buffer----------------------------------
        std::istream input_stream(&buffer);
        std::string clientID;
        std::getline(input_stream, clientID);
        std::cout << "Client ID received: " << clientID << std::endl;
        //encript ClientID(to be done)------------------------------------------

        //Distribute clientID to additive shares to-> shares[0], shares[1]--------------------------------
        int primeMod = generateRandomPrime(127);
        std::vector<std::vector<int>> shares = stringToAdditiveShares(clientID, primeMod);

        //Adding other share to queue to send it to NPCI-------------------------------------------
        std::string shareToNPCI = shareToString(shares[1]);
        std::string toNPCI = makemsg(messageIDlocal,1,-1,shareToNPCI,"-1");
        qToNPCI.push(toNPCI);
        
        //Make and senting share to payee device--------------
        std::string shareToPayeeDevice = shareToString(shares[0]);
        std::string toPayeeDevice = makemsg(messageIDlocal,0,-1,shareToPayeeDevice,"-1");
        boost::asio::write(socket, boost::asio::buffer(toPayeeDevice));
        std::cout << "Message sent to payeeDevice: " << toPayeeDevice << std::endl;


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






    } catch (std::exception& e) {
        std::cerr << "Client connection error: " << e.what() << std::endl;
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

// Function to handle the connection to the server on port 8085
void connect_to_server(boost::asio::io_context& io_context, const std::string& server_address, unsigned short server_port) {
    try {
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve(server_address, std::to_string(server_port));

        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        std::cout << "Connected to server at port " << server_port << std::endl;

        while(true){
            if(qToNPCI.empty()) continue;
            else{
                std::string message = qToNPCI.front() + "/n";
                qToNPCI.pop();
                boost::asio::write(socket, boost::asio::buffer(message));
                std::cout << "Message sent to client: " << message;
                
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error connecting to server: " << e.what() << std::endl;
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