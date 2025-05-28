#pragma once

#include <string_view>
class Regex {
    private:
        struct MinNdfaImpl;
        MinNdfaImpl* pMinNdfaImpl = nullptr;

    public:
        ~Regex();

        bool Compile(std::string_view pattern);
};
