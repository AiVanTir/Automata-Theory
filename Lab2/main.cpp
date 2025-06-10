#include <iostream>
#include <sstream>
#include "regex.hpp"

int main() {
    std::string string, stringToCheck; 
    std::istringstream stream;
    RegexData data;
    Regex regex;
    int variant;
    
    std::cout << "Menu:\n"
              << "0.Exit\n"
              << "1.Compile\n"
              << "2.Match without compile\n"
              << "3.Match with compile\n"
              << "4.Findall without compile\n"
              << "5.Findall with compile\n"
              << "6.Complement regex\n"
              << "7.Print DFA\n"
              << "8.Print menu\n";

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

            case 2:
                std::cout << "Enter string to match:\n>";
                std::getline(std::cin, stringToCheck);

                if (regex.Match(stringToCheck, data)) {
                    std::cout << "match (" << data.GetMatchedString() << ")\n";
                    std::cout << "Named groups:\n";
                    
                    for (const auto pair: data)
                        std::cout << pair.first << " (" << pair.second << ")\n";
                }
                else 
                    std::cout << "nomatch\n";

                break;

            case 3:
                std::cout << "Enter a regular expression:\n>";
                std::getline(std::cin, string);
                
                std::cout << "Enter string to match:\n>";
                std::getline(std::cin, stringToCheck);

                if (regex.Match(string, stringToCheck, data)) {
                    std::cout << "match (" << data.GetMatchedString() << ")\n";
                    std::cout << "Named groups:\n";

                    for (const auto pair: data)
                        std::cout << pair.first << " (" << pair.second << ")\n";
                }
                else 
                    std::cout << "nomatch\n";

                break;

            case 4: {
                std::cout << "Enter string to findall:\n>";
                std::getline(std::cin, string);

                std::vector<RegexData> ndata = regex.FindAll(string);

                for (RegexData findAllData : ndata) {
                    std::cout << "match (" << findAllData.GetMatchedString() << ")\n";
                    std::cout << "named groups:\n";

                    for (const auto pair: findAllData)
                        std::cout << pair.first << " (" << pair.second << ")\n";
                }
                if (ndata.size() == 0)
                    std::cout << "nomatch\n";

                break;
            }
            case 5: {
                std::cout << "Enter a regular expression:\n>";
                std::getline(std::cin, string);

                std::cout << "Enter string to findall:\n>";
                std::getline(std::cin, stringToCheck);

                std::vector<RegexData> ndata = regex.FindAll(string, stringToCheck);

                for (RegexData findAllData : ndata) {
                    std::cout << "match (" << findAllData.GetMatchedString() << ")\n";
                    std::cout << "named groups:\n";

                    for (const auto pair: findAllData)
                        std::cout << pair.first << " (" << pair.second << ")\n";
                }
                if (ndata.size() == 0)
                    std::cout << "nomatch\n";

                break;
            }
            case 6:
                regex.ComplementRegex();
                std::cout << "Successful complement\n";
                break;

            case 7:
                regex.Print();
                break;

            case 8:
                std::cout << "Menu:\n"
                          << "0.Exit\n"
                          << "1.Compile\n"
                          << "2.Match without compile\n"
                          << "3.Match with compile\n"
                          << "4.Findall without compile\n"
                          << "5.Findall with compile\n"
                          << "6.Complement regex\n"
                          << "7.Print DFA\n"
                          << "8.Print menu\n";
                break;

            default:
                std::cerr << "Invalid input" << std::endl;
        }
    }
} 
