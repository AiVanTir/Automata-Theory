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
                        data.capturedGroups.at(i).second = stringToCheck.substr(positionSave, position - positionSave);
                        return true;
                    }
                }
                std::pair<int, std::string> pair(groupDfa->number, stringToCheck.substr(positionSave, position - positionSave));
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
                    data.capturedGroups.at(i).second = stringToCheck.substr(positionSave, position - positionSave);
                    return true;
                }
            }
            std::pair<int, std::string> pair(groupDfa->number, stringToCheck.substr(positionSave, position - positionSave));
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
                data.capturedGroups.at(i).second = stringToCheck.substr(positionSave, position - positionSave);
                return true;
            }
        }
        std::pair<int, std::string> pair(groupDfa->number, stringToCheck.substr(positionSave, position - positionSave));
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

                if (pair.first > 0) {
                    for (groupDfaNum = 1; groupDfaNum < pMinNdfaImpl->minNdfa.size(); ++groupDfaNum) 
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
                else {
                    std::string groupValue;

                    for (const auto& group : data.capturedGroups) {
                        if (group.first == -pair.first) {
                            groupValue = group.second;
                            break;
                        }
                    }
                    std::cout << groupValue << std::endl;
                    std::cout << i << std::endl;
                    if (stringToMatch.compare(i, groupValue.length(), groupValue) != 0) {
                        data.capturedGroups.clear();
                        return false;
                    }
                    i = i + groupValue.length() - 1;
                    currentState = pair.second;
                    isFind = true;
                    break;
                }
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
    isFind = false;

    while (currentState < minDfa->transitions.size() && minDfa->finalStates.count(currentState) <= 0) {
        AdjacencyList* currentTransition = minDfa->transitions.at(currentState);

        while (currentTransition) {
            if (std::holds_alternative<std::pair<int, int>>(currentTransition->transition)) {
                std::pair<int, int> pair = std::get<std::pair<int, int>>(currentTransition->transition);
                int groupDfaNum;

                if (pair.first > 0) {
                    for (groupDfaNum = 1; groupDfaNum < pMinNdfaImpl->minNdfa.size(); ++groupDfaNum) 
                        if (pMinNdfaImpl->minNdfa.at(groupDfaNum)->number == pair.first) 
                            break;

                    int strLen = stringToMatch.length();

                    if (GroupMatch(groupDfaNum, stringToMatch, strLen, data)) {
                        currentState = pair.second;
                        isFind = true;
                        break;
                    }
                }
                else {
                    std::string groupValue;

                    for (const auto& group : data.capturedGroups) {
                        if (group.first == -pair.first) {
                            groupValue = group.second;
                            break;
                        }
                    }
                    if (groupValue.length() == 0) {
                        currentState = pair.second;
                        isFind = true;
                        break;
                    }
                }
            }
            currentTransition = currentTransition->next;
        }
        if (isFind == true) {
            isFind = false;
            continue;
        }
        break;
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

                if (pair.first > 0) {
                    for (groupDfaNum = 1; groupDfaNum < pMinNdfaImpl->minNdfa.size(); ++groupDfaNum) 
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
                else {
                    std::string groupValue;

                    for (const auto& group : data.capturedGroups) {
                        if (group.first == -pair.first) {
                            groupValue = group.second;
                            break;
                        }
                    }
                    if (stringToMatch.compare(i, groupValue.length(), groupValue) != 0) {
                        data.capturedGroups.clear();
                        return false;
                    }
                    i = i + groupValue.length() - 1;
                    currentState = pair.second;
                    isFind = true;
                    break;
                }
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
    isFind = false;

    while (currentState < minDfa->transitions.size() && minDfa->finalStates.count(currentState) <= 0) {
        AdjacencyList* currentTransition = minDfa->transitions.at(currentState);

        while (currentTransition) {
            if (std::holds_alternative<std::pair<int, int>>(currentTransition->transition)) {
                std::pair<int, int> pair = std::get<std::pair<int, int>>(currentTransition->transition);
                int groupDfaNum;

                if (pair.first > 0) {
                    for (groupDfaNum = 1; groupDfaNum < pMinNdfaImpl->minNdfa.size(); ++groupDfaNum) 
                        if (pMinNdfaImpl->minNdfa.at(groupDfaNum)->number == pair.first) 
                            break;

                    int strLen = stringToMatch.length();

                    if (GroupMatch(groupDfaNum, stringToMatch, strLen, data)) {
                        currentState = pair.second;
                        isFind = true;
                        break;
                    }
                }
                else {
                    std::string groupValue;

                    for (const auto& group : data.capturedGroups) {
                        if (group.first == -pair.first) {
                            groupValue = group.second;
                            break;
                        }
                    }
                    if (groupValue.length() == 0) {
                        currentState = pair.second;
                        isFind = true;
                        break;
                    }
                }
            }
            currentTransition = currentTransition->next;
        }
        if (isFind == true) {
            isFind = false;
            continue;
        }
        break;
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
    if (stringToCheck.size() == 0)
        if (Match("", data))
            ndata.push_back(data);

    for (int i = 0; i <= stringToCheck.size(); ++i) {
        int length = 1;

        while ((i + length) <= stringToCheck.size()) {
            if (Match(stringToCheck.substr(i, length), data)) {
                ndata.push_back(data);
                i = i + length - 1;
                break;
            }
            length++;
        }
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
    if (stringToCheck.size() == 0)
        if (Match("", data))
            ndata.push_back(data);

    for (int i = 0; i <= stringToCheck.size(); ++i) {
        int length = 1;

        while ((i + length) <= stringToCheck.size()) {
            if (Match(stringToCheck.substr(i, length), data)) {
                ndata.push_back(data);
                i = i + length - 1;
                break;
            }
            length++;
        }
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

bool Regex::ComplementRegex() {
    if (!pMinNdfaImpl) {
        std::cerr << "Compile first" << std::endl;
        return false;
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
    return true;
}

bool Regex::ComplementRegex(std::string_view pattern) {
    if (!Compile(pattern)) {
        std::cerr << "Incorrect regular expression\n";
        return false;
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
    return true;
}
        
bool Regex::IntersectRegex(std::string_view pattern) {
    if (!pMinNdfaImpl) {
        std::cerr << "Compile first" << std::endl;
        return false;
    }
    MinNdfaImpl *pMinNdfaImpl1 = this->pMinNdfaImpl;
    this->pMinNdfaImpl = nullptr;

    if (!Compile(pattern)) {
        std::cerr << "Incorrect regular expression\n";
        this->pMinNdfaImpl = pMinNdfaImpl1;
        return false;
    }
    MinNdfaImpl *pMinNdfaImpl2 = this->pMinNdfaImpl;
    this->pMinNdfaImpl = nullptr;

    std::vector<Automaton*> minNdfaVector;

    Automaton* dfa1 = pMinNdfaImpl1->minNdfa[0];
    Automaton* dfa2 = pMinNdfaImpl2->minNdfa[0];
    
    if (!dfa1 || !dfa2) {
        std::cerr << "Invalid automata\n";
        delete pMinNdfaImpl2;
        this->pMinNdfaImpl = pMinNdfaImpl1;
        return false;
    }
    Automaton* result = new Automaton();

    std::map<std::pair<int, int>, int> stateMap;

    std::queue<std::pair<int, int>> stateQueue;

    std::pair<int, int> initialState = {0, 0};
    stateMap[initialState] = 0;
    stateQueue.push(initialState);
    result->stateCount = 1;

    if (dfa1->finalStates.count(0) > 0 && dfa2->finalStates.count(0) > 0) 
        result->finalStates.insert(0);

    while (!stateQueue.empty()) {
        auto currentPair = stateQueue.front();
        stateQueue.pop();
        
        int currentState1 = currentPair.first;
        int currentState2 = currentPair.second;
        int currentResultState = stateMap[currentPair];

        while (result->transitions.size() <= currentResultState) 
            result->transitions.push_back(nullptr);

        AdjacencyList* transitions1 = currentState1 < dfa1->transitions.size() ? dfa1->transitions[currentState1] : nullptr;
        AdjacencyList* transitions2 = currentState2 < dfa2->transitions.size() ? dfa2->transitions[currentState2] : nullptr;

        std::map<char, int> symbolToState1;

        while (transitions1) {
            if (std::holds_alternative<std::pair<char, int>>(transitions1->transition)) {
                auto [symbol, nextState] = std::get<std::pair<char, int>>(transitions1->transition);
                symbolToState1[symbol] = nextState;
            }
            transitions1 = transitions1->next;
        }
        while (transitions2) {
            if (std::holds_alternative<std::pair<char, int>>(transitions2->transition)) {
                auto [symbol, nextState2] = std::get<std::pair<char, int>>(transitions2->transition);

                if (symbolToState1.count(symbol) > 0) {
                    int nextState1 = symbolToState1[symbol];
                    std::pair<int, int> nextStatePair = {nextState1, nextState2};

                    if (stateMap.count(nextStatePair) == 0) {
                        int newState = result->stateCount++;
                        stateMap[nextStatePair] = newState;
                        stateQueue.push(nextStatePair);

                        if (dfa1->finalStates.count(nextState1) > 0 && dfa2->finalStates.count(nextState2) > 0) 
                            result->finalStates.insert(newState);
                    }
                    int targetState = stateMap[nextStatePair];
                    AdjacencyList* newTransition = new AdjacencyList(symbol, targetState);
                    
                    if (!result->transitions[currentResultState])
                        result->transitions[currentResultState] = newTransition;

                    else {
                        AdjacencyList* current = result->transitions[currentResultState];

                        while (current->next) 
                            current = current->next;

                        current->next = newTransition;
                    }
                }
            }
            transitions2 = transitions2->next;
        }
    }

    minNdfaVector.push_back(result);

    for (size_t i = 1; i < pMinNdfaImpl1->minNdfa.size(); i++) {
        Automaton* source = pMinNdfaImpl1->minNdfa[i];
        Automaton* copy = new Automaton(source->number);
        copy->stateCount = source->stateCount;
        copy->finalStates = source->finalStates;

        for (size_t j = 0; j < source->transitions.size(); j++) {
            AdjacencyList* sourceTransition = source->transitions[j];
            AdjacencyList* copyTransition = nullptr;
            AdjacencyList* current = nullptr;
            
            while (sourceTransition) {
                AdjacencyList* newTransition = nullptr;
                
                if (std::holds_alternative<std::pair<char, int>>(sourceTransition->transition)) {
                    auto [symbol, state] = std::get<std::pair<char, int>>(sourceTransition->transition);
                    newTransition = new AdjacencyList(symbol, state);
                } 
                else {
                    auto [group, state] = std::get<std::pair<int, int>>(sourceTransition->transition);
                    newTransition = new AdjacencyList(group, state);
                }
                if (!copyTransition) {
                    copyTransition = newTransition;
                    current = copyTransition;
                } 
                else {
                    current->next = newTransition;
                    current = current->next;
                }
                sourceTransition = sourceTransition->next;
            }
            copy->transitions.push_back(copyTransition);
        }
        minNdfaVector.push_back(copy);
    }
    
    for (size_t i = 1; i < pMinNdfaImpl2->minNdfa.size(); i++) {
        Automaton* source = pMinNdfaImpl2->minNdfa[i];
        Automaton* copy = new Automaton(source->number);
        copy->stateCount = source->stateCount;
        copy->finalStates = source->finalStates;

        for (size_t j = 0; j < source->transitions.size(); j++) {
            AdjacencyList* sourceTransition = source->transitions[j];
            AdjacencyList* copyTransition = nullptr;
            AdjacencyList* current = nullptr;
            
            while (sourceTransition) {
                AdjacencyList* newTransition = nullptr;
                
                if (std::holds_alternative<std::pair<char, int>>(sourceTransition->transition)) {
                    auto [symbol, state] = std::get<std::pair<char, int>>(sourceTransition->transition);
                    newTransition = new AdjacencyList(symbol, state);
                } 
                else {
                    auto [group, state] = std::get<std::pair<int, int>>(sourceTransition->transition);
                    newTransition = new AdjacencyList(group, state);
                }
                if (!copyTransition) {
                    copyTransition = newTransition;
                    current = copyTransition;
                } 
                else {
                    current->next = newTransition;
                    current = current->next;
                }
                sourceTransition = sourceTransition->next;
            }
            copy->transitions.push_back(copyTransition);
        }
        minNdfaVector.push_back(copy);
    }
    this->pMinNdfaImpl = new MinNdfaImpl(minNdfaVector);

    delete pMinNdfaImpl1;
    delete pMinNdfaImpl2;
    
    return true;
}

bool Regex::IntersectRegex(std::string_view pattern1, std::string_view pattern2) {
    MinNdfaImpl *pOriginalImpl = this->pMinNdfaImpl;
    this->pMinNdfaImpl = nullptr;

    if (!Compile(pattern1)) {
        std::cerr << "Incorrect regular expression 1\n";
        this->pMinNdfaImpl = pOriginalImpl;
        return false;
    }
    MinNdfaImpl *pMinNdfaImpl1 = this->pMinNdfaImpl;
    this->pMinNdfaImpl = nullptr;

    if (!Compile(pattern2)) {
        std::cerr << "Incorrect regular expression 2\n";
        this->pMinNdfaImpl = pOriginalImpl;
        delete pMinNdfaImpl1;
        return false;
    }
    MinNdfaImpl *pMinNdfaImpl2 = this->pMinNdfaImpl;
    this->pMinNdfaImpl = nullptr;

    std::vector<Automaton*> minNdfaVector;

    Automaton* dfa1 = pMinNdfaImpl1->minNdfa[0];
    Automaton* dfa2 = pMinNdfaImpl2->minNdfa[0];
    
    if (!dfa1 || !dfa2) {
        std::cerr << "Invalid automata\n";
        delete pMinNdfaImpl1;
        delete pMinNdfaImpl2;
        this->pMinNdfaImpl = pOriginalImpl;
        return false;
    }
    Automaton* result = new Automaton();

    std::map<std::pair<int, int>, int> stateMap;

    std::queue<std::pair<int, int>> stateQueue;

    std::pair<int, int> initialState = {0, 0};
    stateMap[initialState] = 0;
    stateQueue.push(initialState);
    result->stateCount = 1;

    if (dfa1->finalStates.count(0) > 0 && dfa2->finalStates.count(0) > 0)
        result->finalStates.insert(0);

    while (!stateQueue.empty()) {
        auto currentPair = stateQueue.front();
        stateQueue.pop();
        
        int currentState1 = currentPair.first;
        int currentState2 = currentPair.second;
        int currentResultState = stateMap[currentPair];

        while (result->transitions.size() <= currentResultState)
            result->transitions.push_back(nullptr);

        AdjacencyList* transitions1 = currentState1 < dfa1->transitions.size() ? dfa1->transitions[currentState1] : nullptr;
        AdjacencyList* transitions2 = currentState2 < dfa2->transitions.size() ? dfa2->transitions[currentState2] : nullptr;

        std::map<char, int> symbolToState1;
        while (transitions1) {
            if (std::holds_alternative<std::pair<char, int>>(transitions1->transition)) {
                auto [symbol, nextState] = std::get<std::pair<char, int>>(transitions1->transition);
                symbolToState1[symbol] = nextState;
            }
            transitions1 = transitions1->next;
        }
        while (transitions2) {
            if (std::holds_alternative<std::pair<char, int>>(transitions2->transition)) {
                auto [symbol, nextState2] = std::get<std::pair<char, int>>(transitions2->transition);

                if (symbolToState1.count(symbol) > 0) {
                    int nextState1 = symbolToState1[symbol];
                    std::pair<int, int> nextStatePair = {nextState1, nextState2};

                    if (stateMap.count(nextStatePair) == 0) {
                        int newState = result->stateCount++;
                        stateMap[nextStatePair] = newState;
                        stateQueue.push(nextStatePair);

                        if (dfa1->finalStates.count(nextState1) > 0 && dfa2->finalStates.count(nextState2) > 0)
                            result->finalStates.insert(newState);
                    }
                    int targetState = stateMap[nextStatePair];
                    AdjacencyList* newTransition = new AdjacencyList(symbol, targetState);
                    
                    if (!result->transitions[currentResultState])
                        result->transitions[currentResultState] = newTransition;

                    else {
                        AdjacencyList* current = result->transitions[currentResultState];

                        while (current->next)
                            current = current->next;

                        current->next = newTransition;
                    }
                }
            }
            transitions2 = transitions2->next;
        }
    }
    minNdfaVector.push_back(result);

    for (size_t i = 1; i < pMinNdfaImpl1->minNdfa.size(); i++) {
        Automaton* source = pMinNdfaImpl1->minNdfa[i];
        Automaton* copy = new Automaton(source->number);
        copy->stateCount = source->stateCount;
        copy->finalStates = source->finalStates;

        for (size_t j = 0; j < source->transitions.size(); j++) {
            AdjacencyList* sourceTransition = source->transitions[j];
            AdjacencyList* copyTransition = nullptr;
            AdjacencyList* current = nullptr;
            
            while (sourceTransition) {
                AdjacencyList* newTransition = nullptr;
                
                if (std::holds_alternative<std::pair<char, int>>(sourceTransition->transition)) {
                    auto [symbol, state] = std::get<std::pair<char, int>>(sourceTransition->transition);
                    newTransition = new AdjacencyList(symbol, state);
                } 
                else {
                    auto [group, state] = std::get<std::pair<int, int>>(sourceTransition->transition);
                    newTransition = new AdjacencyList(group, state);
                }
                if (!copyTransition) {
                    copyTransition = newTransition;
                    current = copyTransition;
                } 
                else {
                    current->next = newTransition;
                    current = current->next;
                }
                sourceTransition = sourceTransition->next;
            }
            copy->transitions.push_back(copyTransition);
        }
        minNdfaVector.push_back(copy);
    }
    for (size_t i = 1; i < pMinNdfaImpl2->minNdfa.size(); i++) {
        Automaton* source = pMinNdfaImpl2->minNdfa[i];
        Automaton* copy = new Automaton(source->number);
        copy->stateCount = source->stateCount;
        copy->finalStates = source->finalStates;

        for (size_t j = 0; j < source->transitions.size(); j++) {
            AdjacencyList* sourceTransition = source->transitions[j];
            AdjacencyList* copyTransition = nullptr;
            AdjacencyList* current = nullptr;
            
            while (sourceTransition) {
                AdjacencyList* newTransition = nullptr;
                
                if (std::holds_alternative<std::pair<char, int>>(sourceTransition->transition)) {
                    auto [symbol, state] = std::get<std::pair<char, int>>(sourceTransition->transition);
                    newTransition = new AdjacencyList(symbol, state);
                } 
                else {
                    auto [group, state] = std::get<std::pair<int, int>>(sourceTransition->transition);
                    newTransition = new AdjacencyList(group, state);
                }
                if (!copyTransition) {
                    copyTransition = newTransition;
                    current = copyTransition;
                } 
                else {
                    current->next = newTransition;
                    current = current->next;
                }
                sourceTransition = sourceTransition->next;
            }
            copy->transitions.push_back(copyTransition);
        }
        minNdfaVector.push_back(copy);
    }
    this->pMinNdfaImpl = new MinNdfaImpl(minNdfaVector);

    delete pMinNdfaImpl1;
    delete pMinNdfaImpl2;

    if (pOriginalImpl) 
        delete pOriginalImpl;

    return true;
}
