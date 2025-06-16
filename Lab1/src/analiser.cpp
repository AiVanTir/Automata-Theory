#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <memory>
#include <random>
#include <algorithm>
#include <cctype>
#include <numeric>
#include <cmath>
#include <iomanip>
#include "DifRecog/IMyRecognizer.hpp"
#include "DifRecog/RegexRecog.hpp"
#include "DifRecog/SmcRecog.hpp"
#include "DifRecog/FlexRecog.hpp"
std::vector<Token> tokens;
// Генератор тестовых строк
class TestStringGenerator {
public:
    static std::string generateValidString(size_t length) {
        static const std::string types[] = {"int", "short", "long"};
        static const char alphanum[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> type_dist(0, 2);
        std::uniform_int_distribution<> len_dist(1, 16);
        std::uniform_int_distribution<> num_dist(1, 9);
        std::uniform_int_distribution<> char_dist(0, sizeof(alphanum) - 2);
        std::uniform_int_distribution<> val_dist(0, 999);
        std::uniform_int_distribution<> neg_dist(0, 1);

        // Тип
        std::string result = types[type_dist(gen)] + " ";

        // Имя массива
        result += "Name";

        // Количество элементов
        result += "[" + std::to_string(length) + "]={";

        // Список инициализации
        for (int i = 0; i < length; ++i) {
            if (i != 0) result += ",";
            if (neg_dist(gen)) {
                result += "-" + std::to_string(val_dist(gen));
            } else {
                result += std::to_string(val_dist(gen));
            }
        }

        result += "}";

        return result;
    }

    static std::string generateInvalidString(size_t length) {
        static const char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+-=[]{};':\",./<>?\\|`~";

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> char_dist(0, sizeof(chars) - 2);

        std::string result;
        for (size_t i = 0; i < length; ++i) {
            result += chars[char_dist(gen)];
        }

        return result;
    }
};

void BenchmarkRecognizers(const std::string& outputFile) {
    std::ofstream out(outputFile);
    if (!out.is_open()) {
        std::cerr << "Не удалось открыть файл для записи\n";
        return;
    }

    // Шапка таблицы
    out << "StringLength SmcRecog(ms) FlexRecognizer(ms) RegexRecognizer(ms)\n";
    // Длины строк от 2^7 до 2^20
    for (size_t length = 64; length <= 6064; length += 100) { // 2^7 = 128 ... 2^14 = 16384
        std::vector<std::unique_ptr<IMyRecognizer>> recognizers;
        recognizers.emplace_back(std::make_unique<SmcRecog>());
        recognizers.emplace_back(std::make_unique<FlexRecognizer>());
        recognizers.emplace_back(std::make_unique<RegexRecognizer>());

        std::vector<double> avgTimes(3, 0.0); // среднее время для каждого распознавателя

        for (int r = 0; r < 3; ++r) {
            double totalTime = 0.0;

            for (int i = 0; i < 10; ++i) {
                std::string testStr = TestStringGenerator::generateValidString(length);
                std::vector<std::string> dummyAllNames, dummyFreaks;

                auto start = std::chrono::high_resolution_clock::now();
                recognizers[r]->CheckString(testStr, dummyAllNames, dummyFreaks);
                auto end = std::chrono::high_resolution_clock::now();

                std::chrono::duration<double, std::milli> duration = end - start;
                totalTime += duration.count();
            }

            avgTimes[r] = totalTime / 10.0;
        }

        // Запись строки в файл
        out << length * 5  +  13 << " "
            << std::fixed << std::setprecision(3)
            << avgTimes[0] << " "
            << avgTimes[1] << " "
            << avgTimes[2] << "\n";

        std::cout << "Обработано: длина " << length << " символов\n";
    }

    out.close();
    std::cout << "Результаты сохранены в файл: " << outputFile << "\n";
}

int main() {
    BenchmarkRecognizers("benchmark_results.txt");
    return 0;
}
