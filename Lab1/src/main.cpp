#include "DifRecog/FlexRecog.hpp"
#include "DifRecog/RegexRecog.hpp"
#include "DifRecog/SmcRecog.hpp"
#include <iostream>
#include <vector>

std::vector<Token> tokens;

int main() {
    FlexRecognizer flexRecognizer;
    std::string input = "int myArray[3]={1,2,3}";
    std::vector<std::string> AllNames;
    std::vector<std::string> Freaks;


    if (flexRecognizer.CheckString(input, AllNames, Freaks)) {
        std::cout << "FlexRecognizer: Строка корректна!" << std::endl;
    } else {
        std::cout << "FlexRecognizer: Строка некорректна!" << std::endl;
    }

    RegexRecognizer regexRecognizer;

    if (regexRecognizer.CheckString(input, AllNames, Freaks)) {
        std::cout << "RegexRecognizer: Строка корректна!" << std::endl;
    } else {
        std::cout << "RegexRecognizer: Строка некорректна!" << std::endl;
    }

    SmcRecog smcRecognizer;

    if (smcRecognizer.CheckString(input, AllNames, Freaks)) {
        std::cout << "SmcRecognizer: Строка корректна!" << std::endl;
    } else {
        std::cout << "SmcRecognizer: Строка некорректна!" << std::endl;
    }

    return 0;
}