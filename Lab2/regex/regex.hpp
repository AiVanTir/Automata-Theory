#pragma once

#include <string>
#include <string_view>
#include <vector>

class RegexData;

class Regex {
    private:
        struct MinNdfaImpl;
        MinNdfaImpl* pMinNdfaImpl = nullptr;
        bool GroupMatch(int groupDfaNum, std::string_view stringToCheck, int& position, RegexData& data);

    public:
        ~Regex();

        bool Compile(std::string_view pattern);
        
        bool Match(std::string_view stringToMatch, RegexData& data);
        bool Match(std::string_view pattern, std::string_view stringToMatch, RegexData& data);
        std::vector<RegexData> FindAll(std::string_view stringToCheck);
        std::vector<RegexData> FindAll(std::string_view pattern, std::string_view stringToCheck);

        bool RecoverRegex(std::string& pattern);
        void ComplementRegex();
        void IntersectRegex(std::string_view pattern);

        void Print();
};

class RegexData {
    private:
        std::string matchedString;
        std::vector<std::pair<int, std::string>> capturedGroups;
        
        void AddCapturedGroup(int number, std::string value) {
            capturedGroups.emplace_back(number, value);
        }

    public:
        std::string GetMatchedString() const {
            return matchedString;
        }
        std::pair<int, std::string> operator[](size_t index) const {
            if (index < capturedGroups.size()) 
                return capturedGroups[index];
            else 
                throw std::out_of_range("");
        }
        auto begin() { return capturedGroups.begin(); }
        auto end()   { return capturedGroups.end();   }
        auto begin() const { return capturedGroups.begin(); }
        auto end()   const { return capturedGroups.end();   }

        size_t size() const {
            return capturedGroups.size();
        }
        friend class Regex;
};
