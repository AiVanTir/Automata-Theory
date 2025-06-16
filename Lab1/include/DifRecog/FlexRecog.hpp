#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include "DifRecog/IMyRecognizer.hpp"
#include "DifRecog/FlexLexer.h"
#include <algorithm>

enum Token {
    TYPE_INT,
    TYPE_SHORT,
    TYPE_LONG,
    IDENTIFIER,
    NUMBER,
    OPEN_BRACKET,
    CLOSE_BRACKET,
    OPEN_BRACE,
    CLOSE_BRACE,
    EQUALS,
    COMMA,
    ERROR
};

extern std::vector<Token> tokens;

extern "C" {
int yylex();
}

yy_buffer_state* yy_scan_string(const char* str);

class FlexRecognizer : public IMyRecognizer{
public:
    bool CheckString(const std::string& input, std::vector<std::string>& AllNames, std::vector<std::string>& Freaks) {
        tokens.clear();
        size_t expectedSize = 0;
        std::string arrayType;

        yy_scan_string(input.c_str());
        yylex();

        if (tokens.empty()) {
            return false;
        }

        if (tokens[0] != TYPE_INT && tokens[0] != TYPE_SHORT && tokens[0] != TYPE_LONG) {
            return false;
        }

        arrayType = (tokens[0] == TYPE_INT) ? "int" :
                    (tokens[0] == TYPE_SHORT) ? "short" : "long";

        if (tokens.size() < 2 || tokens[1] != IDENTIFIER) {
            return false;
        }

        size_t identifierStart = input.find(' ', 0) + 1;
        size_t identifierEnd = input.find('[', identifierStart);
        std::string identifier = input.substr(identifierStart, identifierEnd - identifierStart);
        if (identifier.length() > 16) {
            return false;
        }

        std::string fullName = identifier + "." + arrayType;
        if(std::count(AllNames.begin(), AllNames.end(), fullName) != 0) {
            Freaks.push_back(fullName);
        }else{
            AllNames.push_back(fullName);
        }

        if (tokens.size() < 4 || tokens[2] != OPEN_BRACKET || tokens[3] != NUMBER) {
            return false;
        }

        size_t numberStart = input.find('[') + 1;
        size_t numberEnd = input.find(']');
        std::string numberStr = input.substr(numberStart, numberEnd - numberStart);
        expectedSize = std::stoul(numberStr);

        if (expectedSize < 1 || expectedSize > 999999999) {
            return false;
        }

        if (tokens.size() < 7 || tokens[4] != CLOSE_BRACKET || tokens[5] != EQUALS || tokens[6] != OPEN_BRACE) {
            return false;
        }

        size_t initializerCount = 0;
        for (size_t i = 7; i < tokens.size(); ++i) {
            if (tokens[i] == NUMBER) {
                initializerCount++;
            } else if (tokens[i] == COMMA) {
            } else if (tokens[i] == CLOSE_BRACE) {
                break;
            } else {
                return false;
            }
        }

        if (initializerCount != expectedSize || tokens.back() != CLOSE_BRACE) {
            return false;
        }

        return true;
    }
};
