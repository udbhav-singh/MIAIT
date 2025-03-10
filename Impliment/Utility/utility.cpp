#include"utility.h"
#include<bits/stdc++.h>
#include<sstream>
std::vector<int> parsemsg(const std::string& input) {
    std::vector<int> numbers;
    std::stringstream ss(input);
    std::string token;
    
    while (std::getline(ss, token, ',')&& numbers.size() < 5) {
        numbers.push_back(std::stoi(token));
    }

    return numbers;
}
std::string makemsg(int a, int b, int c, std::string d, int e) {
    std::stringstream ss;
    ss << a << "," << b << "," << c << "," << d << "," << e<<",";
    return ss.str();
}