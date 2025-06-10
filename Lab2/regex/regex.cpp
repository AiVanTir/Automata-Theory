#include <iostream>
#include "parser.hpp"
#include "ast.hpp"
#include "automaton.hpp"
#include "regex.hpp"

struct Regex::MinNdfaImpl {
    std::vector<Automaton*> minNdfa;

    MinNdfaImpl(std::vector<Automaton*>& minNdfa) {
        this->minNdfa = minNdfa;
    }
    ~MinNdfaImpl() {
        for (int i = 0; i < minNdfa.size(); ++i)
            delete minNdfa.at(i);
    }
};

Regex::~Regex() {
    delete pMinNdfaImpl;
}
        
void Regex::Print() {
    if (!pMinNdfaImpl) {
        std::cerr << "Compile first" << std::endl;
        return;
    }
    PrintAutomata(pMinNdfaImpl->minNdfa);
}

bool Regex::Compile(std::string_view pattern) {
    Parser parser(pattern);
    Node* root = parser.ParseExpr();

    if (root == nullptr)
        return false;

    Ast ast(root);

    if (!ast.Prepare()) 
        return false;

    std::vector<Automaton*> nnfa;
    std::vector<Automaton*> ndfa;
    std::vector<Automaton*> minNdfa;

    Automaton nfa;
    nnfa.push_back(&nfa);
    NnfaInit(nnfa, root, 0);
    NnfaPrepare(nnfa);

    ndfa = NnfaToNdfa(nnfa);

    minNdfa = MinimizeNdfa(ndfa);

#if 1
    std::cout << std::endl << "NFA COUNT = " << nnfa.size() << std::endl;
    PrintAutomata(nnfa);

    std::cout << "DFA COUNT = " << ndfa.size() << std::endl;
    PrintAutomata(ndfa);

    std::cout << "MIN DFA COUNT = " << minNdfa.size() << std::endl;
    PrintAutomata(minNdfa);
#endif

    for (int i = 1; i < nnfa.size(); ++i)
        delete nnfa.at(i);
    for (int i = 0; i < ndfa.size(); ++i)
        delete ndfa.at(i);

    delete this->pMinNdfaImpl; 
    this->pMinNdfaImpl = new MinNdfaImpl(minNdfa);

    return true;
}

bool Regex::GroupMatch(int groupDfaNum, std::string_view stringToCheck, int& position, RegexData& data) {
    Automaton* groupDfa = pMinNdfaImpl->minNdfa.at(groupDfaNum);
    bool isFind = false;
    int currentState = 0;
    int positionSave = position;

    for (;position < stringToCheck.length(); ++position) {
        if (currentState >= groupDfa->transitions.size()) {
            if (groupDfa->finalStates.count(currentState) > 0) {
                for (int i = 0; i < data.capturedGroups.size(); ++i) {
                    if (data.capturedGroups.at(i).first == groupDfa->number) {
                        data.capturedGroups.at(i).second = stringToCheck.substr(positionSave, position);
                        return true;
                    }
                }
                std::pair<int, std::string> pair(groupDfa->number, stringToCheck.substr(positionSave, position));
                data.capturedGroups.push_back(pair);
                return true;
            }
            else {
                return false;
            }
        }
        AdjacencyList* currentTransition = groupDfa->transitions.at(currentState);

        while (currentTransition) {
            if (std::holds_alternative<std::pair<char, int>>(currentTransition->transition)) {
                char symbol = stringToCheck.at(position);
                std::pair<char, int> pair = std::get<std::pair<char, int>>(currentTransition->transition);
                
                if (pair.first == symbol) {
                    currentState = pair.second;
                    isFind = true;
                    break;
                }
            }
            else {
                std::pair<int, int> pair = std::get<std::pair<int, int>>(currentTransition->transition);
                int groupDfaNum;

                for (groupDfaNum = 0; groupDfaNum < pMinNdfaImpl->minNdfa.size(); ++groupDfaNum) 
                    if (pMinNdfaImpl->minNdfa.at(groupDfaNum)->number == pair.first) 
                        break;

                if (!GroupMatch(groupDfaNum, stringToCheck, position, data)) 
                    return false;

                position--;
                currentState = pair.second;
                isFind = true;
                break;
            }        
            currentTransition = currentTransition->next;
        }
        if (isFind) {
            isFind = false;
            continue;
        }
        if (groupDfa->finalStates.count(currentState) > 0) {
            for (int i = 0; i < data.capturedGroups.size(); ++i) {
                if (data.capturedGroups.at(i).first == groupDfa->number) {
                    data.capturedGroups.at(i).second = stringToCheck.substr(positionSave, position);
                    return true;
                }
            }
            std::pair<int, std::string> pair(groupDfa->number, stringToCheck.substr(positionSave, position));
            data.capturedGroups.push_back(pair);
            return true;
        }
        return false;
    }
    if (!(currentState >= groupDfa->transitions.size())) {
        AdjacencyList* currentTransition = groupDfa->transitions.at(currentState);

        while (currentTransition) {
            if (std::holds_alternative<std::pair<int, int>>(currentTransition->transition)) {
                std::pair<int, int> pair = std::get<std::pair<int, int>>(currentTransition->transition);
                int groupDfaNum;

                for (groupDfaNum = 0; groupDfaNum < pMinNdfaImpl->minNdfa.size(); ++groupDfaNum) 
                    if (pMinNdfaImpl->minNdfa.at(groupDfaNum)->number == pair.first) 
                        break;

                int strLen = stringToCheck.length();

                if (GroupMatch(groupDfaNum, stringToCheck, strLen, data)) {
                    currentState = pair.second;
                    break;
                }
            }
            currentTransition = currentTransition->next;
        }
    }
    if (groupDfa->finalStates.count(currentState) > 0) {
        for (int i = 0; i < data.capturedGroups.size(); ++i) {
            if (data.capturedGroups.at(i).first == groupDfa->number) {
                data.capturedGroups.at(i).second = stringToCheck.substr(positionSave, position);
                return true;
            }
        }
        std::pair<int, std::string> pair(groupDfa->number, stringToCheck.substr(positionSave, position));
        data.capturedGroups.push_back(pair);
        return true;
    }
    return false;
}

bool Regex::Match(std::string_view stringToMatch, RegexData& data) {
    if (!pMinNdfaImpl) {
        std::cerr << "Compile first" << std::endl;
        return false;
    }
    data.matchedString = "";
    data.capturedGroups.clear();
    bool isFind = false;
    int currentState = 0;
    Automaton* minDfa = pMinNdfaImpl->minNdfa.at(0);
    
    for (int i = 0; i < stringToMatch.length(); ++i) {
        if (currentState >= minDfa->transitions.size()) {
            data.capturedGroups.clear();
            return false;
        }
        AdjacencyList* currentTransition = minDfa->transitions.at(currentState);

        while (currentTransition) {
            if (std::holds_alternative<std::pair<char, int>>(currentTransition->transition)) {
                char symbol = stringToMatch.at(i);
                std::pair<char, int> pair = std::get<std::pair<char, int>>(currentTransition->transition);
                
                if (pair.first == symbol) {
                    currentState = pair.second;
                    isFind = true;
                    break;
                }
            }
            else {
                std::pair<int, int> pair = std::get<std::pair<int, int>>(currentTransition->transition);
                int groupDfaNum;

                for (groupDfaNum = 0; groupDfaNum < pMinNdfaImpl->minNdfa.size(); ++groupDfaNum) 
                    if (pMinNdfaImpl->minNdfa.at(groupDfaNum)->number == pair.first) 
                        break;

                if (!GroupMatch(groupDfaNum, stringToMatch, i, data)) {
                    data.capturedGroups.clear();
                    return false;
                }
                i--;
                currentState = pair.second;
                isFind = true;
                break;
            }        
            currentTransition = currentTransition->next;
        }
        if (isFind) {
            isFind = false;
            continue;
        }
        data.capturedGroups.clear();
        return false;
    }
    if (!(currentState >= minDfa->transitions.size())) {
        AdjacencyList* currentTransition = minDfa->transitions.at(currentState);

        while (currentTransition) {
            if (std::holds_alternative<std::pair<int, int>>(currentTransition->transition)) {
                std::pair<int, int> pair = std::get<std::pair<int, int>>(currentTransition->transition);
                int groupDfaNum;

                for (groupDfaNum = 0; groupDfaNum < pMinNdfaImpl->minNdfa.size(); ++groupDfaNum) 
                    if (pMinNdfaImpl->minNdfa.at(groupDfaNum)->number == pair.first) 
                        break;

                int strLen = stringToMatch.length();

                if (GroupMatch(groupDfaNum, stringToMatch, strLen, data)) {
                    currentState = pair.second;
                    break;
                }
            }
            currentTransition = currentTransition->next;
        }
    }
    if (minDfa->finalStates.count(currentState) > 0) {
        data.matchedString = stringToMatch;
        return true;
    }
    data.capturedGroups.clear();
    return false;
}

bool Regex::Match(std::string_view pattern, std::string_view stringToMatch, RegexData& data) {
    if (!Compile(pattern)) {
        std::cerr << "Incorrect regular expression\n";
        return false;
    }
    data.matchedString = "";
    data.capturedGroups.clear();
    bool isFind = false;
    int currentState = 0;
    Automaton* minDfa = pMinNdfaImpl->minNdfa.at(0);
    
    for (int i = 0; i < stringToMatch.length(); ++i) {
        if (currentState >= minDfa->transitions.size()) {
            data.capturedGroups.clear();
            return false;
        }
        AdjacencyList* currentTransition = minDfa->transitions.at(currentState);

        while (currentTransition) {
            if (std::holds_alternative<std::pair<char, int>>(currentTransition->transition)) {
                char symbol = stringToMatch.at(i);
                std::pair<char, int> pair = std::get<std::pair<char, int>>(currentTransition->transition);
                
                if (pair.first == symbol) {
                    currentState = pair.second;
                    isFind = true;
                    break;
                }
            }
            else {
                std::pair<int, int> pair = std::get<std::pair<int, int>>(currentTransition->transition);
                int groupDfaNum;

                for (groupDfaNum = 0; groupDfaNum < pMinNdfaImpl->minNdfa.size(); ++groupDfaNum) 
                    if (pMinNdfaImpl->minNdfa.at(groupDfaNum)->number == pair.first) 
                        break;

                if (!GroupMatch(groupDfaNum, stringToMatch, i, data)) {
                    data.capturedGroups.clear();
                    return false;
                }
                i--;
                currentState = pair.second;
                isFind = true;
                break;
            }        
            currentTransition = currentTransition->next;
        }
        if (isFind) {
            isFind = false;
            continue;
        }
        data.capturedGroups.clear();
        return false;
    }
    if (!(currentState >= minDfa->transitions.size())) {
        AdjacencyList* currentTransition = minDfa->transitions.at(currentState);

        while (currentTransition) {
            if (std::holds_alternative<std::pair<int, int>>(currentTransition->transition)) {
                std::pair<int, int> pair = std::get<std::pair<int, int>>(currentTransition->transition);
                int groupDfaNum;

                for (groupDfaNum = 0; groupDfaNum < pMinNdfaImpl->minNdfa.size(); ++groupDfaNum) 
                    if (pMinNdfaImpl->minNdfa.at(groupDfaNum)->number == pair.first) 
                        break;

                int strLen = stringToMatch.length();

                if (GroupMatch(groupDfaNum, stringToMatch, strLen, data)) {
                    currentState = pair.second;
                    break;
                }
            }
            currentTransition = currentTransition->next;
        }
    }
    if (minDfa->finalStates.count(currentState) > 0) {
        data.matchedString = stringToMatch;
        return true;
    }
    data.capturedGroups.clear();
    return false;
}

std::vector<RegexData> Regex::FindAll(std::string_view stringToCheck) {
    std::vector<RegexData> ndata;
    RegexData data;

    if (!pMinNdfaImpl) {
        std::cerr << "Compile first" << std::endl;
        return ndata;
    }
    if (Match("", data))
        ndata.push_back(data);

    int length = 0;

    for (int i = 0; i <= stringToCheck.size(); ++i) {
        if (Match(stringToCheck.substr(i, length), data)) {
            ndata.push_back(data);
            length = 0;
        }
        length++;
    }
    return ndata;
}

std::vector<RegexData> Regex::FindAll(std::string_view pattern, std::string_view stringToCheck) {
    std::vector<RegexData> ndata;
    RegexData data;

    if (!Compile(pattern)) {
        std::cerr << "Incorrect regular expression\n";
        return ndata;
    }
    if (Match("", data))
        ndata.push_back(data);
    
    int length = 0;

    for (int i = 0; i <= stringToCheck.size(); ++i) {
        if (Match(stringToCheck.substr(i, length), data)) {
            ndata.push_back(data);
            length = 0;
        }
        length++;
    }
    return ndata;
}

bool Regex::RecoverRegex(std::string& pattern) {
    if (!pMinNdfaImpl) {
        std::cerr << "Compile first" << std::endl;
        return false;
    }
    Automaton* dfa = pMinNdfaImpl->minNdfa.at(0);
    std::map<int, std::string> regexMap;

    for (int state : dfa->finalStates) {
        regexMap[state] = ""; 
    }
    for (int i = 0; i < dfa->stateCount; ++i) {
        if (regexMap.find(i) == regexMap.end()) {
            regexMap[i] = ""; 
        }
    }
    for (int stateToRemove = 1; stateToRemove < dfa->stateCount; ++stateToRemove) {
        if (dfa->finalStates.count(stateToRemove) > 0 || stateToRemove == 0) 
            continue;

        for (int p = 0; p < dfa->stateCount; ++p) {
            AdjacencyList* transition = dfa->transitions.at(p); 

            while (transition) {
                if (std::get_if<std::pair<char, int>>(&transition->transition) &&
                    std::get<std::pair<char, int>>(transition->transition).second == stateToRemove) {
                    char symbol = std::get<std::pair<char, int>>(transition->transition).first;

                    if (!regexMap[p].empty()) 
                        regexMap[p] += "|"; 

                    regexMap[p] += symbol + (regexMap[stateToRemove].empty() ? "" : regexMap[stateToRemove]);
                }
                transition = transition->next; 
            }
        }
    }
    pattern = regexMap[0];
    return true; 
}

void Regex::ComplementRegex() {
    if (!pMinNdfaImpl) {
        std::cerr << "Compile first" << std::endl;
        return;
    }
    int start = -1, end, counter = 0;

    for (int i = 0; i < 256; ++i) {
        if (isprint(i)) {
            if (start == -1) {
                start = i;
            }
            end = i;
            counter++;
        }
    }
    for (int i = 0; i < pMinNdfaImpl->minNdfa.size(); ++i) {
        Automaton* dfa = pMinNdfaImpl->minNdfa.at(i);

        /* случай пустого регулярного выражения */
        if (dfa->transitions.size() == 0) {
            dfa->stateCount++;
            dfa->finalStates.erase(0);
            dfa->finalStates.insert(1);

            AdjacencyList* firstTransition = new AdjacencyList(static_cast<char>(start), 1);
            dfa->transitions.push_back(firstTransition);

            for (int j = start + 1; j <= end; ++j) {
                firstTransition->next = new AdjacencyList(static_cast<char>(j), 1);
                firstTransition = firstTransition->next;
            }
            AdjacencyList* secondTransition = new AdjacencyList(static_cast<char>(start), 1);
            dfa->transitions.push_back(secondTransition);
            
            for (int j = start + 1; j <= end; ++j) {
                secondTransition->next = new AdjacencyList(static_cast<char>(j), 1);
                secondTransition = secondTransition->next;
            }
            continue;
        }
        if (i >= 1) {
            for (int j = 0; j < dfa->stateCount; ++j) 
                dfa->finalStates.insert(j);

            continue;
        }
        dfa->stateCount++;
        std::set<int> newFinalStates;
        
        for (int j = 0; j < dfa->stateCount; ++j) {
            if (dfa->finalStates.find(j) == dfa->finalStates.end()) 
                newFinalStates.insert(j);
        }
        dfa->finalStates = newFinalStates;
        AdjacencyList* endTransition = new AdjacencyList(static_cast<char>(start), dfa->stateCount - 1);

        if (dfa->transitions.size() == (dfa->stateCount - 2))
            dfa->transitions.push_back(nullptr);

        dfa->transitions.push_back(endTransition); 

        for (int j = start + 1; j <= end; ++j) {
            endTransition->next = new AdjacencyList(static_cast<char>(j), dfa->stateCount - 1);
            endTransition = endTransition->next;
        }
        for (int j = 0; j < dfa->transitions.size() - 1; ++j) {
            AdjacencyList* currentTransition = dfa->transitions.at(j);

            if (!currentTransition) {
                currentTransition = new AdjacencyList(static_cast<char>(start), dfa->stateCount - 1);
                dfa->transitions.at(j) = currentTransition;
            }
            else {
                while (currentTransition->next) 
                    currentTransition = currentTransition->next;

                currentTransition->next = new AdjacencyList(static_cast<char>(start), dfa->stateCount - 1);
                currentTransition = currentTransition->next;
            }
            for (int k = start + 1; k <= end; ++k) {
                currentTransition->next = new AdjacencyList(static_cast<char>(k), dfa->stateCount - 1);
                currentTransition = currentTransition->next;
            }
        }
    }
}
        
void Regex::IntersectRegex(std::string_view pattern) {

}
