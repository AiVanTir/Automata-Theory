#include <iostream>
#include <sstream>
#include "regex.hpp"

int main() {
    std::istringstream stream;
    std::string string;
    Regex regex;
    int variant;
    
    std::cout << "Menu:\n"
              << "0.Exit\n"
              << "1.Compile\n"
              /* << "2.Match without compile" << std::endl;
                 << "3.Match with compile" << std::endl;
                 << "4.Search without compile" << std::endl;
                 << "5.Search with compile" << std::endl;
                 << "6.Inverse regex" << std::endl;
                 << "7.Complement regex" << std::endl;
                 << "8.Print DFA" << std::endl; */
              << "9.Print menu\n";

    for (;;) {
        std::cout << ">";
        std::getline(std::cin, string);

        if (string.empty())
            continue;
        stream.str(string);
        stream.clear();
        if (!(stream >> variant) || stream.peek() != EOF) {
            std::cerr << "Invalid input" << std::endl;
            continue;
        }
        switch (variant) {
            case 0:
                return 0;

            case 1:
                std::cout << "Enter a regular expression:\n>";
                std::getline(std::cin, string);

                if (!regex.Compile(string)) {
                    std::cerr << "Incorrect regular expression\n";
                    break;
                }
                std::cout << "Successful compilation\n";
                break;

            case 9:
                std::cout << "Menu:\n"
                          << "0.Exit\n"
                          << "1.Compile\n"
                          /* << "2.Match without compile" << std::endl;
                             << "3.Match with compile" << std::endl;
                             << "4.Search without compile" << std::endl;
                             << "5.Search with compile" << std::endl;
                             << "6.Inverse regex" << std::endl;
                             << "7.Complement regex" << std::endl;
                             << "8.Print DFA" << std::endl; */
                          << "9.Print menu\n";
                break;

            default:
                std::cerr << "Invalid input" << std::endl;
        }
    }
} 
