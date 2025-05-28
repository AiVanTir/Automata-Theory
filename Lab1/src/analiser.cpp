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
        int name_len = std::min(16, static_cast<int>(length / 4));
        name_len = std::max(1, name_len);
        result += "a";
        for (int i = 1; i < name_len; ++i) {
            result += alphanum[char_dist(gen)];
        }

        // Количество элементов
        int num_elements = num_dist(gen);
        result += "[" + std::to_string(num_elements) + "]={";

        // Список инициализации
        for (int i = 0; i < num_elements; ++i) {
            if (i != 0) result += ",";
            if (neg_dist(gen)) {
                result += "-" + std::to_string(val_dist(gen));
            } else {
                result += std::to_string(val_dist(gen));
            }
        }

        result += "}";

        // Дополнение до нужной длины
        while (result.length() < length) {
            result += " ";
        }

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

// Точный таймер с наносекундным разрешением
class PreciseTimer {
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Nanoseconds = std::chrono::nanoseconds;

public:
    void start() { start_time = Clock::now(); }
    void stop() { end_time = Clock::now(); }
    double elapsed() const {
        return std::chrono::duration_cast<Nanoseconds>(end_time - start_time).count() / 1e6;
    }

private:
    TimePoint start_time, end_time;
};

// Функция для прогрева кэша и стабилизации измерений
template<typename Func>
void warmup(Func&& func, const std::string& input,
            std::vector<std::string>& names, std::vector<std::string>& freaks) {
    for (int i = 0; i < 100; ++i) {
        func(input, names, freaks);
        names.clear();
        freaks.clear();
    }
}

struct TimingResult {
    double min_time;
    double avg_time;
    double max_time;
    double std_dev;
};

// Измерение времени с высокой точностью
template<typename Func>
TimingResult precise_measure(Func&& func, const std::string& input,
                             std::vector<std::string>& names, std::vector<std::string>& freaks,
                             int iterations = 100) {
    std::vector<double> timings;
    timings.reserve(iterations);

    PreciseTimer timer;

    // Прогрев и стабилизация
    warmup(func, input, names, freaks);

    // Основные измерения
    for (int i = 0; i < iterations; ++i) {
        names.clear();
        freaks.clear();

        timer.start();
        func(input, names, freaks);
        timer.stop();

        timings.push_back(timer.elapsed());
    }

    // Статистическая обработка
    TimingResult result;
    result.min_time = *std::min_element(timings.begin(), timings.end());
    result.max_time = *std::max_element(timings.begin(), timings.end());
    result.avg_time = std::accumulate(timings.begin(), timings.end(), 0.0) / iterations;

    // Стандартное отклонение
    double sq_sum = std::inner_product(timings.begin(), timings.end(), timings.begin(), 0.0);
    result.std_dev = std::sqrt(sq_sum / iterations - result.avg_time * result.avg_time);

    return result;
}

void measurePerformance(const std::vector<std::unique_ptr<IMyRecognizer>>& recognizers,
                        const std::vector<std::string>& testStrings,
                        const std::string& outputFilename) {
    std::ofstream outFile(outputFilename);
    if (!outFile) {
        std::cerr << "Failed to open output file: " << outputFilename << std::endl;
        return;
    }

    // Заголовок CSV с расширенной статистикой
    outFile << "StringLength,"
            << "SmcRecogMin,SmcRecogAvg,SmcRecogMax,SmcRecogStdDev,"
            << "FlexMin,FlexAvg,FlexMax,FlexStdDev,"
            << "RegexMin,RegexAvg,RegexMax,RegexStdDev\n";

    std::vector<std::string> dummyNames;
    std::vector<std::string> dummyFreaks;

    for (const auto& testStr : testStrings) {
        outFile << testStr.length();

        for (const auto& recognizer : recognizers) {
            auto result = precise_measure(
                    [&](const std::string& s, std::vector<std::string>& n, std::vector<std::string>& f) {
                        return recognizer->CheckString(s, n, f);
                    },
                    testStr, dummyNames, dummyFreaks, 1000
            );

            outFile << "," << result.min_time
                    << "," << result.avg_time
                    << "," << result.max_time
                    << "," << result.std_dev;

            dummyNames.clear();
            dummyFreaks.clear();
        }

        outFile << "\n";
    }

    outFile.close();
}

int main() {
    // Создаем распознаватели
    std::vector<std::unique_ptr<IMyRecognizer>> recognizers;
    recognizers.push_back(std::make_unique<SmcRecog>());
    recognizers.push_back(std::make_unique<FlexRecognizer>());
    recognizers.push_back(std::make_unique<RegexRecognizer>());

    // Генерируем тестовые строки (степени двойки от 2^4 до 2^12)
    std::vector<std::string> validStrings;
    std::vector<std::string> invalidStrings;

    for (int exp = 4; exp <= 12; ++exp) {
        size_t length = 1 << exp; // 2^exp

        // Генерируем 10 валидных строк этой длины
        for (int i = 0; i < 10; ++i) {
            validStrings.push_back(TestStringGenerator::generateValidString(length));
        }

        // Генерируем 10 невалидных строк этой длины
        for (int i = 0; i < 10; ++i) {
            invalidStrings.push_back(TestStringGenerator::generateInvalidString(length));
        }
    }

    // Измеряем производительность для валидных строк
    measurePerformance(recognizers, validStrings, "valid_strings_performance.csv");

    // Измеряем производительность для невалидных строк
    measurePerformance(recognizers, invalidStrings, "invalid_strings_performance.csv");

    std::cout << "Performance measurement completed. Results saved to CSV files." << std::endl;

    return 0;
}