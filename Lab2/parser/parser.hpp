#pragma once

#include <string>
#include "ast.hpp"

class Parser {
    private:
        std::string pattern;

        bool GetSymbol(char* symbol) {
            if (position == pattern.length())
                return false;

            *symbol = pattern.at(position);
            position++;
            return true;
        }
        bool Match(const char symbol) {
            if (position < pattern.size()) 
                if (pattern[position] == symbol)
                    return true;

            return false;
        }
        bool MatchAndConsume(const char symbol) {
            if (Match(symbol)) {
                position++;
                return true;
            }
            return false;
        }
        std::string ParseUntilSymbol(const char symbol) {
            int oldPosition = position;
            position = pattern.find(symbol, position);

            if (position != std::string::npos) 
                return pattern.substr(oldPosition, position - oldPosition);

            position = pattern.size();
            return pattern.substr(oldPosition); 
        }
        Node* ParseOr();
        Node* ParseConcat();
        Node* ParseRepeat();
        Node* ParseAtom();
        Node* ParseGroup();
        Node* ParseGroupedExpr();
        Node* ParseSymbol();
        Node* ParseGroupRef();

    public:
        int position = 0;
        Parser(std::string_view pattern) : pattern(pattern) {}
        Node* ParseExpr();
};
