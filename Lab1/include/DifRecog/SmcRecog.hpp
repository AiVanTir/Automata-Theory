#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include "DifRecog/IMyRecognizer.hpp"
#include "DifRecog/SmcRecog_sm.h"

class SmcRecog : public IMyRecognizer {
protected:
    SmcRecogContext _fsm;
    std::string currentType;
    std::string currentArray;
    int expectedSize = 0;
    int currentCount = 0;
    bool valid = true;

public:
    SmcRecog() : _fsm(*this) {}

    bool CheckString(const std::string& input,
                     std::vector<std::string>& AllNames,
                     std::vector<std::string>& Freaks) override
    {
        currentType.clear();
        currentArray.clear();
        expectedSize = 0;
        currentCount = 0;
        valid = true;

        _fsm.enterStartState();

        std::string buffer;
        for (size_t i = 0; i < input.size() && valid; ++i) {
            char c = input[i];

            if (isspace(c)) continue;

            if (isalpha(c)) {
                buffer += c;
                while (i+1 < input.size() && isalnum(input[i+1])) {
                    buffer += input[++i];
                }

                try {
                    if (buffer == "int") {
                        _fsm.INT();
                        currentType = "int";
                    } else if (buffer == "short") {
                        _fsm.SHORT();
                        currentType = "short";
                    } else if (buffer == "long") {
                        _fsm.LONG();
                        currentType = "long";
                    } else {
                        // Проверка имени массива
                        if (buffer.empty() || !isalpha(buffer[0]) || buffer.length() > 16) {
                            valid = false;
                        } else {
                            _fsm.IDENT();
                            currentArray = buffer;
                        }
                    }
                } catch (const statemap::TransitionUndefinedException&) {
                    valid = false;
                }

                buffer.clear();
            } else if (isdigit(c) || c == '-') {
                // Обработка чисел (включая отрицательные)
                buffer += c;
                while (i+1 < input.size() && (isdigit(input[i+1]) || input[i+1] == '-')) {
                    if (input[i+1] == '-' && buffer.find('-') != std::string::npos) {
                        valid = false; // Несколько '-' в числе
                        break;
                    }
                    buffer += input[++i];
                }

                if (valid) {
                    try {
                        _fsm.NUMBER();
                        if (expectedSize == 0) {
                            expectedSize = std::stoi(buffer);
                            if (expectedSize == 0) valid = false;
                        } else {
                            currentCount++;
                        }
                    } catch (const statemap::TransitionUndefinedException&) {
                        valid = false;
                    } catch (const std::invalid_argument&) {
                        valid = false;
                    }
                }

                buffer.clear();
            } else {
                try {
                    switch (c) {
                        case '[': _fsm.LBRACKET(); break;
                        case ']': _fsm.RBRACKET(); break;
                        case '=': _fsm.EQUAL_SIGN(); break;
                        case '{': _fsm.OPEN_BRACE(); break;
                        case '}': _fsm.CLOSE_BRACE(); break;
                        case ',': _fsm.COMMA(); break;
                        default: valid = false;
                    }
                } catch (const statemap::TransitionUndefinedException&) {
                    valid = false;
                }
            }
        }

        // Дополнительные проверки после обработки всей строки
        if (valid) {

            if (currentCount != expectedSize) {
                valid = false;
            }

            if (valid && !currentType.empty() && !currentArray.empty()) {
                std::string fullName = currentArray + "." + currentType;
                if (std::find(AllNames.begin(), AllNames.end(), fullName) != AllNames.end()) {
                    Freaks.push_back(fullName);
                } else {
                    AllNames.push_back(fullName);
                }
            } else {
                valid = false;
            }
        }

        return valid;
    }
};