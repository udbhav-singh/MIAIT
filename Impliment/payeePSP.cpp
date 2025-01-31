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
// #include <aes.h>
// #include <modes.h>
// #include <filters.h>
// #include <secblock.h>
// #include <osrng.h> 


using boost::asio::ip::tcp;


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


// Function to handle communication with a single client
void handle_client(tcp::socket socket) {
    try {
        int messageIDlocal;
        pthread_mutex_lock(&lock); 

        messageID++;
        messageIDlocal = messageID;
        pthread_mutex_unlock(&lock); 
 
        
        std::cout<<"i am here"<<std::endl;
        // Buffer to store clientID
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

        share1Str = std::to_string(messageIDlocal) + ',' + share1Str;
        share2Str = std::to_string(messageIDlocal) + ',' + share2Str;
        //Senting share to payee device
        boost::asio::write(socket, boost::asio::buffer(share1Str));
        std::cout << "Message sent to client: " << share1Str << std::endl;


        //connecting to NPCI servers to NPCI-------------------------------------------
        
        try {
        boost::asio::io_context io_context;

        // Connect to the server
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "8085");
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        std::cout << "Connected to server!8085" << std::endl;

        // Sent share2Str
        boost::asio::write(socket, boost::asio::buffer(share2Str));
        std::cout<<"sent to NPCI:"<< share2Str<< std::endl;

        

    } catch (std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }









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


int main() {


    try {
        boost::asio::io_context io_context;

        // Create a TCP acceptor on port 8083
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8083));
        std::cout << "Server is running and waiting for connections on port 8083..." << std::endl;

        while (true) {
            // Wait for a client to connect
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            std::cout << "New client connected!" << std::endl;

            // Handle the client connection in a separate thread
            std::thread(handle_client, std::move(socket)).detach();
        }
    } catch (std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    return 0;
}


