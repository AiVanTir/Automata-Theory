#pragma once
#include <iostream>
#include <string>
#include <regex>
#include "DifRecog/IMyRecognizer.hpp"

class RegexRecognizer : public IMyRecognizer {
public:
    bool CheckString(const std::string& input, std::vector<std::string>& AllNames, std::vector<std::string>& Freaks) override {
        std::regex pattern(R"(^(int|short|long)\s([a-zA-Z][a-zA-Z0-9]{0,15})\[([1-9][0-9]{0,8})\]\=\{(-?[0-9]+(?:,-?[0-9]+)*)?\}$)");

        std::smatch matches;
        if (!std::regex_match(input, matches, pattern)) {
            return false;
        }

        if (matches.size() >= 4) {
            std::string type = matches[1].str();
            std::string name = matches[2].str();
            int declaredSize = std::stoi(matches[3].str());

            if (matches.size() > 4 && !matches[4].str().empty()) {
                std::string values = matches[4].str();
                int actualCount = 1 + std::count(values.begin(), values.end(), ',');
                if (actualCount != declaredSize) {
                    return false;
                }
            }

            std::string fullName = name + "." + type;
            if (std::find(AllNames.begin(), AllNames.end(), fullName) != AllNames.end()) {
                Freaks.push_back(fullName);
            } else {
                AllNames.push_back(fullName);
            }
        }
        return true;
    }
};