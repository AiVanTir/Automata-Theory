#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

std::string generateType() {
    const std::vector<std::string> types = {"int", "short", "long"};
    return types[rand() % types.size()];
}

std::string generateArrayName() {
    const std::string letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string alphanum = letters + "0123456789";
    std::string name;
    name += letters[rand() % letters.size()]; // Первым символом всегда буква
    int length = rand() % 16 + 1; // Длина до 16 символов
    for (int i = 1; i < length; ++i) {
        name += alphanum[rand() % alphanum.size()];
    }
    return name;
}

std::string generateElementCount() {
    int length = rand() % 9 + 1; // Длина до 9 символов
    std::string count;
    count += '1' + rand() % 9; // Первое число не может быть '0'
    for (int i = 1; i < length; ++i) {
        count += '0' + rand() % 10;
    }
    return count;
}

std::string generateLiteral() {
    std::string literal;
    if (rand() % 2) literal += '-'; // Возможен символ '-' в начале
    literal += std::to_string(rand()); // Числовой литерал без ограничения на 1000
    return literal;
}

std::string generateInitializationList(int count) {
    std::string initList;
    for (int i = 0; i < count; ++i) {
        initList += generateLiteral();
        if (i != count - 1) initList += ";";
    }
    return initList;
}

std::string generateRandomArrayString() {
    srand(static_cast<unsigned int>(time(nullptr)));

    std::string type = generateType();
    std::string arrayName = generateArrayName();
    std::string elementCount = generateElementCount();
    int count = std::stoi(elementCount);
    std::string initList = generateInitializationList(count);

    return type + " " + arrayName + "[" + elementCount + "]={" + initList + "};";
}
