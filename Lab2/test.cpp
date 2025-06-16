#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "regex.hpp"

TEST_CASE("Compilation") {
    Regex regex;

    SECTION("Базовые регулярные выражения") {
        REQUIRE(regex.Compile("a"));
        REQUIRE(regex.Compile("abc"));
        REQUIRE(regex.Compile("a|b|c"));
        REQUIRE(regex.Compile("(a|b|c)"));
        REQUIRE(regex.Compile("a+"));
        REQUIRE(regex.Compile("a{1,5}"));
        REQUIRE(regex.Compile("a{1,}"));
        REQUIRE(regex.Compile("^"));
        REQUIRE(regex.Compile("(1:123)"));
        REQUIRE(regex.Compile("#\\1"));
    }
    SECTION("Сложные регулярные выражения") {
        REQUIRE(regex.Compile("(a|b)+"));
        REQUIRE(regex.Compile("(a|b){1,5}"));
        REQUIRE(regex.Compile("(1:a)(2:b)"));
        REQUIRE(regex.Compile("(1:a)\\1"));
        REQUIRE(regex.Compile("(1:a|b)+.c"));
        REQUIRE(regex.Compile("(1:a{1,3})\\1"));
    }
    SECTION("Некорректные регулярные выражения") {
        REQUIRE(!regex.Compile("|"));
        REQUIRE(!regex.Compile("a|"));
        REQUIRE(!regex.Compile("|a"));
        REQUIRE(!regex.Compile("("));
        REQUIRE(!regex.Compile(")"));
        REQUIRE(!regex.Compile("(a"));
        REQUIRE(!regex.Compile("a)"));
        REQUIRE(!regex.Compile("{"));
        REQUIRE(!regex.Compile("}"));
        REQUIRE(!regex.Compile("a{"));
        REQUIRE(!regex.Compile("a}"));
        REQUIRE(!regex.Compile("a{,}"));
        REQUIRE(!regex.Compile("a{5,3}"));
        REQUIRE(!regex.Compile("a{-1,5}"));
        REQUIRE(!regex.Compile("\\2"));
        REQUIRE(!regex.Compile("(1:a)\\2"));
    }
}


TEST_CASE("Match") {
    Regex regex;
    RegexData data;

    SECTION("Простые совпадения") {
        REQUIRE(regex.Compile("a"));
        REQUIRE(regex.Match("a", data));
        REQUIRE(data.GetMatchedString() == "a");
        REQUIRE(!regex.Match("b", data));

        REQUIRE(regex.Compile("abc"));
        REQUIRE(regex.Match("abc", data));
        REQUIRE(data.GetMatchedString() == "abc");
        REQUIRE(!regex.Match("ab", data));
        REQUIRE(!regex.Match("abcd", data));
    }
    SECTION("Операция 'или'") {
        REQUIRE(regex.Compile("a|b|c"));
        REQUIRE(regex.Match("a", data));
        REQUIRE(regex.Match("b", data));
        REQUIRE(regex.Match("c", data));
        REQUIRE(!regex.Match("d", data));
        REQUIRE(!regex.Match("ab", data));
    }
    SECTION("Операция 'конкатенация'") {
        REQUIRE(regex.Compile("a.b"));
        REQUIRE(regex.Match("ab", data));
        REQUIRE(!regex.Match("a", data));
        REQUIRE(!regex.Match("b", data));
        REQUIRE(!regex.Match("abc", data));
    }
    SECTION("Операция 'позитивное замыкание'") {
        REQUIRE(regex.Compile("a+"));
        REQUIRE(regex.Match("a", data));
        REQUIRE(regex.Match("aa", data));
        REQUIRE(regex.Match("aaa", data));
        REQUIRE(!regex.Match("", data));
        REQUIRE(!regex.Match("b", data));
    }
    SECTION("Операция 'повтор выражения в диапазоне'") {
        REQUIRE(regex.Compile("a{1,3}"));
        REQUIRE(regex.Match("a", data));
        REQUIRE(regex.Match("aa", data));
        REQUIRE(regex.Match("aaa", data));
        REQUIRE(!regex.Match("", data));
        REQUIRE(!regex.Match("aaaa", data));
        
        REQUIRE(regex.Compile("a{2,}"));
        REQUIRE(!regex.Match("a", data));
        REQUIRE(regex.Match("aa", data));
        REQUIRE(regex.Match("aaa", data));
        REQUIRE(regex.Match("aaaa", data));
    }
    SECTION("Пустая подстрока") {
        REQUIRE(regex.Compile("^"));
        REQUIRE(regex.Match("", data));
        REQUIRE(!regex.Match("a", data));
    }
    SECTION("Скобки для приоритета") {
        REQUIRE(regex.Compile("(a|b).c"));
        REQUIRE(regex.Match("ac", data));
        REQUIRE(regex.Match("bc", data));
        REQUIRE(!regex.Match("abc", data));
        
        REQUIRE(regex.Compile("a|(b.c)"));
        REQUIRE(regex.Match("a", data));
        REQUIRE(regex.Match("bc", data));
        REQUIRE(!regex.Match("ac", data));
    }
}

TEST_CASE("Группы захвата") {
    Regex regex;
    RegexData data;

    SECTION("Простые группы") {
        REQUIRE(regex.Compile("(1:a)"));
        REQUIRE(regex.Match("a", data));
        REQUIRE(data.size() == 1);
        REQUIRE(data[0].first == 1);
        REQUIRE(data[0].second == "a");
        
        REQUIRE(regex.Compile("(1:a)(2:b)"));
        REQUIRE(regex.Match("ab", data));
        REQUIRE(data.size() == 2);
        REQUIRE(data[0].first == 1);
        REQUIRE(data[0].second == "a");
        REQUIRE(data[1].first == 2);
        REQUIRE(data[1].second == "b");
    }
    SECTION("Вложенные группы") {
        REQUIRE(regex.Compile("(1:(2:a))"));
        REQUIRE(regex.Match("a", data));
        REQUIRE(data.size() == 2);
        REQUIRE(data[0].first == 2);
        REQUIRE(data[0].second == "a");
        REQUIRE(data[1].first == 1);
        REQUIRE(data[1].second == "a");
    }
    SECTION("Группы с альтернативами") {
        REQUIRE(regex.Compile("(1:a|b)"));
        REQUIRE(regex.Match("a", data));
        REQUIRE(data.size() == 1);
        REQUIRE(data[0].first == 1);
        REQUIRE(data[0].second == "a");
        
        REQUIRE(regex.Match("b", data));
        REQUIRE(data.size() == 1);
        REQUIRE(data[0].first == 1);
        REQUIRE(data[0].second == "b");
    }
    SECTION("Ссылки на группы") {
        REQUIRE(regex.Compile("(1:a|b)\\1"));
        REQUIRE(regex.Match("aa", data));
        REQUIRE(regex.Match("bb", data));
        REQUIRE(!regex.Match("ab", data));
        REQUIRE(!regex.Match("ba", data));
        
        REQUIRE(regex.Compile("(1:a|b)(2:c|d)\\1\\2"));
        REQUIRE(regex.Match("acac", data));
        REQUIRE(regex.Match("bdbd", data));
        REQUIRE(!regex.Match("acbd", data));
        REQUIRE(!regex.Match("bdac", data));
    }
}

TEST_CASE("FindAll") {
    Regex regex;

    SECTION("Простой поиск") {
        REQUIRE(regex.Compile("a"));
        auto results = regex.FindAll("ababa");
        REQUIRE(results.size() == 3);
        REQUIRE(results[0].GetMatchedString() == "a");
        REQUIRE(results[1].GetMatchedString() == "a");
        REQUIRE(results[2].GetMatchedString() == "a");
        
        REQUIRE(regex.Compile("ab"));
        results = regex.FindAll("ababab");
        REQUIRE(results.size() == 3);
        REQUIRE(results[0].GetMatchedString() == "ab");
        REQUIRE(results[1].GetMatchedString() == "ab");
        REQUIRE(results[2].GetMatchedString() == "ab");
    }
    SECTION("Поиск с группами") {
        REQUIRE(regex.Compile("(1:a)(2:b)"));
        auto results = regex.FindAll("abcabd");
        REQUIRE(results.size() == 2);
        REQUIRE(results[0].GetMatchedString() == "ab");
        REQUIRE(results[0].size() == 2);
        REQUIRE(results[0][0].first == 1);
        REQUIRE(results[0][0].second == "a");
        REQUIRE(results[0][1].first == 2);
        REQUIRE(results[0][1].second == "b");
        
        REQUIRE(results[1].GetMatchedString() == "ab");
        REQUIRE(results[1].size() == 2);
        REQUIRE(results[1][0].first == 1);
        REQUIRE(results[1][0].second == "a");
        REQUIRE(results[1][1].first == 2);
        REQUIRE(results[1][1].second == "b");
    }
    SECTION("Поиск с перекрытиями") {
        REQUIRE(regex.Compile("a+"));
        auto results = regex.FindAll("aaba");
        REQUIRE(results.size() == 3);
        REQUIRE(results[0].GetMatchedString() == "a");
        REQUIRE(results[1].GetMatchedString() == "a");
        REQUIRE(results[2].GetMatchedString() == "a");
    }
}

TEST_CASE("ComplementRegex") {
    Regex regex;
    RegexData data;

    SECTION("Дополнение простых выражений") {
        REQUIRE(regex.Compile("a"));
        REQUIRE(regex.ComplementRegex());
        REQUIRE(!regex.Match("a", data));
        REQUIRE(regex.Match("b", data));
        REQUIRE(regex.Match("ab", data));
        REQUIRE(regex.Match("", data));
        
        REQUIRE(regex.Compile("abc"));
        REQUIRE(regex.ComplementRegex());
        REQUIRE(!regex.Match("abc", data));
        REQUIRE(regex.Match("ab", data));
        REQUIRE(regex.Match("bc", data));
        REQUIRE(regex.Match("", data));
    }

    SECTION("Дополнение с группами") {
        REQUIRE(regex.Compile("(1:a)b"));
        REQUIRE(regex.ComplementRegex());
        REQUIRE(!regex.Match("ab", data));
        REQUIRE(regex.Match("a", data));
        REQUIRE(regex.Match("", data));
        
        /* Проверяем, что группы сохраняются */
        REQUIRE(regex.Match("a", data));
        REQUIRE(data.size() == 1);
        REQUIRE(data[0].first == 1);
        REQUIRE(data[0].second == "a");
    }
}

TEST_CASE("IntersectRegex") {
    Regex regex;
    RegexData data;

    SECTION("Пересечение простых выражений") {
        REQUIRE(regex.IntersectRegex("a+", "a{1,3}"));
        REQUIRE(regex.Match("a", data));
        REQUIRE(regex.Match("aa", data));
        REQUIRE(regex.Match("aaa", data));
        REQUIRE(!regex.Match("", data));
        REQUIRE(!regex.Match("aaaa", data));
        
        REQUIRE(regex.IntersectRegex("a|b", "b|c"));
        REQUIRE(!regex.Match("a", data));
        REQUIRE(regex.Match("b", data));
        REQUIRE(!regex.Match("c", data));
    }
    SECTION("Пересечение с группами") {
        REQUIRE(regex.IntersectRegex("(1:a)b", "a(2:b)"));
        REQUIRE(!regex.Match("a", data));
        REQUIRE(!regex.Match("b", data));
    }
    SECTION("Пересечение с уже скомпилированным выражением") {
        REQUIRE(regex.Compile("a+"));
        REQUIRE(regex.IntersectRegex("a{1,3}"));
        REQUIRE(regex.Match("a", data));
        REQUIRE(regex.Match("aa", data));
        REQUIRE(regex.Match("aaa", data));
        REQUIRE(!regex.Match("", data));
        REQUIRE(!regex.Match("aaaa", data));
    }
}


TEST_CASE("Комбинированные операции") {
    Regex regex;
    RegexData data;

    SECTION("Дополнение и пересечение") {
        REQUIRE(regex.Compile("a+"));
        REQUIRE(regex.ComplementRegex());
        REQUIRE(regex.IntersectRegex("b+"));
        REQUIRE(!regex.Match("a", data));
        REQUIRE(regex.Match("b", data));
        REQUIRE(regex.Match("bb", data));
        REQUIRE(!regex.Match("ab", data));
        REQUIRE(!regex.Match("ba", data));
        
        REQUIRE(regex.Compile("a|b"));
        REQUIRE(regex.ComplementRegex());
        REQUIRE(regex.IntersectRegex("c|d"));
        REQUIRE(!regex.Match("a", data));
        REQUIRE(!regex.Match("b", data));
        REQUIRE(regex.Match("c", data));
        REQUIRE(regex.Match("d", data));
        REQUIRE(!regex.Match("e", data));
    }
    SECTION("Пересечение и восстановление") {
        REQUIRE(regex.IntersectRegex("a+", "a{1,3}"));
        std::string recovered;
        REQUIRE(regex.RecoverRegex(recovered));
        
        Regex recoveredRegex;
        REQUIRE(recoveredRegex.Compile(recovered));
    }
}

TEST_CASE("Экранирование и метасимволы") {
    Regex regex;
    RegexData data;

    SECTION("Экранирование метасимволов") {
        REQUIRE(regex.Compile("#|"));
        REQUIRE(regex.Match("|", data));
        REQUIRE(!regex.Match("a", data));
        
        REQUIRE(regex.Compile("#+"));
        REQUIRE(regex.Match("+", data));
        REQUIRE(!regex.Match("a", data));
        
        REQUIRE(regex.Compile("#{"));
        REQUIRE(regex.Match("{", data));
        REQUIRE(!regex.Match("a", data));
        
        REQUIRE(regex.Compile("#}"));
        REQUIRE(regex.Match("}", data));
        REQUIRE(!regex.Match("a", data));
        
        REQUIRE(regex.Compile("#("));
        REQUIRE(regex.Match("(", data));
        REQUIRE(!regex.Match("a", data));
        
        REQUIRE(regex.Compile("#)"));
        REQUIRE(regex.Match(")", data));
        REQUIRE(!regex.Match("a", data));
        
        REQUIRE(regex.Compile("#:"));
        REQUIRE(regex.Match(":", data));
        REQUIRE(!regex.Match("a", data));
        
        REQUIRE(regex.Compile("#\\"));
        REQUIRE(regex.Match("\\", data));
        REQUIRE(!regex.Match("a", data));
        
        REQUIRE(regex.Compile("#."));
        REQUIRE(regex.Match(".", data));
        REQUIRE(!regex.Match("a", data));
    }
    SECTION("Пустая подстрока") {
        REQUIRE(regex.Compile("^"));
        REQUIRE(regex.Match("", data));
        REQUIRE(!regex.Match("a", data));
        
        REQUIRE(regex.Compile("a^"));
        REQUIRE(regex.Match("a", data));
        REQUIRE(!regex.Match("", data));
        REQUIRE(!regex.Match("aa", data));
        
        REQUIRE(regex.Compile("^a"));
        REQUIRE(regex.Match("a", data));
        REQUIRE(!regex.Match("", data));
        REQUIRE(!regex.Match("aa", data));
    }
}

TEST_CASE("Сложные регулярные выражения") {
    Regex regex;
    RegexData data;

    SECTION("Сложные выражения с группами") {
        REQUIRE(regex.Compile("(1:a|b)+.(2:c|d)+"));
        REQUIRE(regex.Match("ac", data));
        REQUIRE(data.size() == 2);
        REQUIRE(data[0].first == 1);
        REQUIRE(data[1].first == 2);

        REQUIRE(regex.Match("bd", data));
        REQUIRE(data.size() == 2);
        REQUIRE(data[0].first == 1);
        REQUIRE(data[1].first == 2);

        REQUIRE(!regex.Match("c", data));
        REQUIRE(!regex.Match("ab", data));
    }
}

TEST_CASE("Граничные случаи") {
    Regex regex;
    RegexData data;

    SECTION("Пустые выражения") {
        REQUIRE(regex.Compile(""));
        REQUIRE(regex.Match("", data));
        REQUIRE(!regex.Match("a", data));
        
        REQUIRE(regex.Compile("^"));
        REQUIRE(regex.Match("", data));
        REQUIRE(!regex.Match("a", data));
    }
    SECTION("Большие повторы") {
        REQUIRE(regex.Compile("a{1,100}"));
        std::string longA(100, 'a');
        REQUIRE(regex.Match("a", data));
        REQUIRE(regex.Match(longA, data));
        REQUIRE(!regex.Match(longA + "a", data));
        
        REQUIRE(regex.Compile("(1:a){10,20}"));
        std::string a15(15, 'a');
        REQUIRE(regex.Match(a15, data));
        REQUIRE(data.size() == 1);
        REQUIRE(data[0].first == 1);
        REQUIRE(data[0].second == "a");
    }
    SECTION("Много групп") {
        std::string pattern = "";

        for (int i = 1; i <= 10; i++)
            pattern += "(" + std::to_string(i) + ":a)";

        REQUIRE(regex.Compile(pattern));
        REQUIRE(regex.Match("aaaaaaaaaa", data));
        REQUIRE(data.size() == 10);

        for (int i = 0; i < 10; i++) {
            REQUIRE(data[i].first == i + 1);
            REQUIRE(data[i].second == "a");
        }
    }
}

TEST_CASE("Производительность") {
    Regex regex;
    RegexData data;

    SECTION("Компиляция сложных выражений") {
        std::string pattern = "";
        for (int i = 0; i < 100; i++) {
            if (i > 0) 
                pattern += "|";

            pattern += std::string(1, 'a' + (i % 26));
        }
        REQUIRE(regex.Compile(pattern));

        for (int i = 0; i < 26; i++)
            REQUIRE(regex.Match(std::string(1, 'a' + i), data));

        REQUIRE(!regex.Match("aa", data));
    }
    SECTION("Поиск в длинных строках") {
        REQUIRE(regex.Compile("a+"));

        std::string longText(1000, 'b');
        longText[100] = 'a';
        longText[500] = 'a';
        longText[900] = 'a';
        
        auto results = regex.FindAll(longText);
        REQUIRE(results.size() == 3);

        for (const auto& result : results) 
            REQUIRE(result.GetMatchedString() == "a");
    }
}
