#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cmath>

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

// Example usage
int main() {
    std::string secret = "Hello";
    int primeMod = generateRandomPrime(127); // Choose a random prime > max ASCII value

    // Generate additive shares
    std::vector<std::vector<int>> shares = stringToAdditiveShares(secret, primeMod);

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

    return 0;
}
