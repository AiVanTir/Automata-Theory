#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

std::string generateInvalidType() {
    const std::vector<std::string> invalidTypes = {"integer", "shrt", "lng", "int "}; // Ошибки в типе данных
    return invalidTypes[rand() % invalidTypes.size()];
}

std::string generateInvalidArrayName() {
    const std::string letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string alphanum = letters + "0123456789";
    std::string name;
    if (rand() % 2) name += '_'; // Недопустимый символ в начале
    else name += letters[rand() % letters.size()];

    int length = rand() % 17; // Длина может превысить 16 символов
    for (int i = 1; i < length; ++i) {
        name += alphanum[rand() % alphanum.size()];
    }
    return name;
}

std::string generateInvalidElementCount() {
    if (rand() % 2) return "0"; // Неверное значение — начинается с '0'
    if (rand() % 2) return "";  // Пропуск значения
    return std::to_string(rand() % 1000000000); // Чрезмерно длинное значение
}

std::string generateInvalidLiteral() {
    std::string literal;
    if (rand() % 2) literal += '-';
    literal += std::to_string(rand() % 100000);
    if (rand() % 3 == 0) literal += 'x'; // Недопустимый символ в конце
    return literal;
}

std::string generateInvalidInitializationList(int count) {
    std::string initList;
    for (int i = 0; i < count; ++i) {
        initList += generateInvalidLiteral();
        if (rand() % 3 == 0) initList += ','; // Неверный разделитель
        else if (i != count - 1) initList += ";";
    }
    return initList;
}

std::string generateInvalidArrayString() {
    srand(static_cast<unsigned int>(time(nullptr)));

    std::string type = generateInvalidType();
    std::string arrayName = generateInvalidArrayName();
    std::string elementCount = generateInvalidElementCount();
    int count = (elementCount.empty() || elementCount == "0") ? 1 : std::stoi(elementCount);
    std::string initList = generateInvalidInitializationList(count);

    return type + " " + arrayName + "[" + elementCount + "]={" + initList + "};";
}
