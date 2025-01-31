#include <sstream>
#include <vector>
#include <string>
#include <iostream>

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

// Example usage
int main() {
    // Example shares
    std::vector<std::vector<int>> shares = {{123, 100, 94, 7, 4}, {14, 11, 3, 104, 107}};

    // Serialize each share into its own string
    std::string share1Str = shareToString(shares[0]);
    std::string share2Str = shareToString(shares[1]);

    // Print the serialized strings
    std::cout << "Serialized Share 1: " << share1Str << std::endl;
    std::cout << "Serialized Share 2: " << share2Str << std::endl;

    // Deserialize the strings back into shares
    std::vector<int> deserializedShare1 = stringToShare(share1Str);
    std::vector<int> deserializedShare2 = stringToShare(share2Str);

    // Print the deserialized shares
    std::cout << "Deserialized Share 1: ";
    for (int value : deserializedShare1) {
        std::cout << value << " ";
    }
    std::cout << std::endl;

    std::cout << "Deserialized Share 2: ";
    for (int value : deserializedShare2) {
        std::cout << value << " ";
    }
    std::cout << std::endl;

    return 0;
}
