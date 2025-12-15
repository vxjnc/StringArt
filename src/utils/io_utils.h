#pragma once

#include <vector>
#include <string>
#include <sstream>

std::vector<std::string> split(const std::string &s, const char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
        tokens.push_back(std::move(token));

    return tokens;
}