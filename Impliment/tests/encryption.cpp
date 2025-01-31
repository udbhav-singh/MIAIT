#include <iostream>
#include <string>
#include <iomanip>
#include <cryptlib.h>
#include <aes.h>
#include <modes.h>
#include <filters.h>
#include <secblock.h>
#include <osrng.h> 

// For AutoSeededRandomPool

using namespace CryptoPP;

// Function to encrypt a plaintext using AES-CBC
std::string encryptAES(const std::string& plaintext, const SecByteBlock& key, const byte iv[AES::BLOCKSIZE]) {
    std::string ciphertext;

    try {
        CBC_Mode<AES>::Encryption encryption;
        encryption.SetKeyWithIV(key, key.size(), iv);

        StringSource(plaintext, true,
            new StreamTransformationFilter(encryption,
                new StringSink(ciphertext)
            )
        );
    } catch (const CryptoPP::Exception& e) {
        std::cerr << "Encryption Error: " << e.what() << std::endl;
        throw;
    }

    return ciphertext;
}

// Function to decrypt a ciphertext using AES-CBC
std::string decryptAES(const std::string& ciphertext, const SecByteBlock& key, const byte iv[AES::BLOCKSIZE]) {
    std::string decryptedtext;

    try {
        CBC_Mode<AES>::Decryption decryption;
        decryption.SetKeyWithIV(key, key.size(), iv);

        StringSource(ciphertext, true,
            new StreamTransformationFilter(decryption,
                new StringSink(decryptedtext)
            )
        );
    } catch (const CryptoPP::Exception& e) {
        std::cerr << "Decryption Error: " << e.what() << std::endl;
        throw;
    }

    return decryptedtext;
}

// Utility function to generate a random key and IV
void generateKeyAndIV(SecByteBlock& key, byte iv[AES::BLOCKSIZE]) {
    AutoSeededRandomPool rng;

    // Generate a random key
    key = SecByteBlock(AES::DEFAULT_KEYLENGTH);
    rng.GenerateBlock(key, key.size());

    // Generate a random IV
    rng.GenerateBlock(iv, AES::BLOCKSIZE);
}

// Main function to demonstrate usage
int main() {
    // Original plaintext
    std::string plaintext = "Hello, AES encryption with functions!";

    // Key and IV
    SecByteBlock key;
    byte iv[AES::BLOCKSIZE];

    // Generate random key and IV
    generateKeyAndIV(key, iv);

    // Encrypt the plaintext
    std::string ciphertext = encryptAES(plaintext, key, iv);
    std::cout << "Ciphertext (Hex): ";
    for (unsigned char c : ciphertext)
        std::cout << 
        std::hex << 
        std::setw(2) << std::setfill('0') << (int)c;
    std::cout << std::endl;

    // Decrypt the ciphertext
    std::string decryptedtext = decryptAES(ciphertext, key, iv);
    std::cout << "Decrypted Text: " << decryptedtext << std::endl;

    return 0;
}
