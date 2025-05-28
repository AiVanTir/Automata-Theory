#pragma once
#include <iostream>
#include <string>
#include <vector>

class IMyRecognizer {
public:
    virtual bool CheckString(const std::string& input, std::vector<std::string>& AllNames, std::vector<std::string>& Freaks) = 0;
    virtual ~IMyRecognizer() = default;
};