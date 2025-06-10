#include "automaton.hpp"

/* Инициализируем все НКА */
void NnfaInit(std::vector<Automaton*>& nnfa, Node* node, int number) {
    /* SYMBOL */
    if (node->type == NodeType::SYMBOL) {
        Automaton* nfa = nnfa.at(number);
        nfa->stateCount++;

        /* если предыдущей операцией было повторение с нижней границей = 0 */
        if (nfa->stateCount == nfa->transitions.size()) {
            AdjacencyList* transition = nfa->transitions.back();
            transition->next = new AdjacencyList(std::get<char>(node->data), nfa->stateCount);
            return;
        }
        AdjacencyList* transition = new AdjacencyList(std::get<char>(node->data), nfa->stateCount);
        nfa->transitions.push_back(transition);
        return;
    }
    /* CONCAT */
    if (node->type == NodeType::CONCAT) {
        Automaton* nfa = nnfa.at(number);
        int tmpStateCount;

        for (Node* children: node->childrens) {
            if (children->type == NodeType::REPEAT) 
                tmpStateCount = nfa->stateCount;

            NnfaInit(nnfa, children, number);

            if (children->type == NodeType::REPEAT) 
                nfa->finalStates.erase(tmpStateCount);
            nfa->finalStates.erase(nfa->stateCount);
        }
        nfa->finalStates.insert(nfa->stateCount);
        return;
    }
    /* OR */
    if (node->type == NodeType::OR) {
        Automaton* nfa = nnfa.at(number);
        std::vector<int> startStateCounts;
        std::vector<int> endStateCounts;
        std::vector<bool> isStartTransitionsEndStates;
        bool isStartTransition = false;
        AdjacencyList* startTransition;
        nfa->finalStates.erase(nfa->stateCount);

        /* если предыдущей операцией было повторение с нижней границей = 0 */
        if (nfa->stateCount != nfa->transitions.size())
            isStartTransition = true; 
        
        if (!isStartTransition) { 
            startTransition = new AdjacencyList();
            nfa->transitions.push_back(startTransition);
        }
        else 
            startTransition = nfa->transitions.back(); 
        
        for (Node* children: node->childrens) {
            nfa->stateCount++;
            startStateCounts.push_back(nfa->stateCount); 
            NnfaInit(nnfa, children, number);
            
            endStateCounts.push_back(nfa->stateCount);
            nfa->finalStates.erase(nfa->stateCount);

            if (nfa->stateCount == nfa->transitions.size()) { 
                AdjacencyList* endTransition = new AdjacencyList();
                nfa->transitions.push_back(endTransition);
                isStartTransitionsEndStates.push_back(false);
            }
            else
                isStartTransitionsEndStates.push_back(true);
        }
        if (!isStartTransition)
            startTransition->transition = std::make_pair('\0', startStateCounts.at(0));

        else { 
            startTransition->next = new AdjacencyList('\0', startStateCounts.at(0));
            startTransition = startTransition->next;
        } 
        for (int i = 1; i < startStateCounts.size(); ++i) {
            startTransition->next = new AdjacencyList('\0', startStateCounts.at(i));
            startTransition = startTransition->next;
        }
        nfa->stateCount++;
        int counter = 0;

        for (int count: endStateCounts) {
            if (!isStartTransitionsEndStates.at(counter))
                nfa->transitions.at(count)->transition = std::make_pair('\0', nfa->stateCount);

            else
                nfa->transitions.at(count)->next = new AdjacencyList('\0', nfa->stateCount);

            counter++;
        }
        return;     
    }
    /* REPEAT */
    if (node->type == NodeType::REPEAT) {
        Automaton* nfa = nnfa.at(number);
        std::pair<int, int> range = std::get<std::pair<int, int>>(node->data);

        /* повторение 0 раз */
        if (range.first == 0 && range.second == 0) {
            nfa->stateCount++;
            AdjacencyList* transition = new AdjacencyList('\0', nfa->stateCount);
            nfa->transitions.push_back(transition); 
            return;    
        }
        /* нижняя граница */
        for (int i = 0; i < range.first; ++i) {
            nfa->finalStates.erase(nfa->stateCount);
            NnfaInit(nnfa, node->childrens.at(0), number);
        }
        /* верхняя граница (бесконечность) */
        if (range.second == INT_MAX) {
            int tmpStateCount = nfa->stateCount;
            NnfaInit(nnfa, node->childrens.at(0), number);
            bool isStartTransition = false;

            AdjacencyList* startTransition = nfa->transitions.at(tmpStateCount);

            while (startTransition->next) {
                if (std::holds_alternative<std::pair<char, int>>(startTransition->transition)) {
                    std::pair<char, int> transition = std::get<std::pair<char, int>>(startTransition->transition) ;
                
                    if (transition.first == '\0' && transition.second == nfa->stateCount) {
                        isStartTransition = true;
                        break;
                    }
                }
                startTransition = startTransition->next;    
            }
            if (!isStartTransition) 
                startTransition->next = new AdjacencyList('\0', nfa->stateCount);

            if (nfa->stateCount == nfa->transitions.size()) {
                AdjacencyList* endTransition = new AdjacencyList('\0', tmpStateCount);
                nfa->transitions.push_back(endTransition);
            }
        }
        /* верхняя граница (конечное значение) */
        else {
            std::set<int> tmpStateCounts;

            for (int i = range.first; i < range.second; ++i) {
                tmpStateCounts.insert(nfa->stateCount);
                nfa->finalStates.erase(nfa->stateCount);
                NnfaInit(nnfa, node->childrens.at(0), number);
            }
            for (int state : tmpStateCounts) {
                AdjacencyList* newTransition = new AdjacencyList('\0', nfa->stateCount);
                AdjacencyList* transition = nfa->transitions.at(state);
                
                while (transition->next) 
                    transition = transition->next;

                transition->next = newTransition;
            } 
        }
        std::cout << nfa->stateCount << std::endl;
        return;
    }
    /* GROUP */
    if (node->type == NodeType::GROUP) {
        Automaton* nfa = nnfa.at(number);
        nfa->stateCount++;
        nfa->finalStates.insert(nfa->stateCount);

        /* если предыдущей операцией не было повторение с нижней границей = 0 */
        if (nfa->stateCount != nfa->transitions.size()) {
            AdjacencyList* transition = new AdjacencyList(std::get<int>(node->data), nfa->stateCount);
            nfa->transitions.push_back(transition); 
        }
        else 
            nfa->transitions.back()->next = new AdjacencyList(std::get<int>(node->data), nfa->stateCount);

        /* если предыдущей операцией было обязательное повторение более 1 раза */
        for (Automaton* tmpNfa: nnfa) {
            if (std::get<int>(node->data) == tmpNfa->number)
                return;
        }
        Automaton* groupNfa = new Automaton(std::get<int>(node->data));
        nnfa.push_back(groupNfa);
        NnfaInit(nnfa, node->childrens.at(0), (nnfa.size() - 1));
        return;
    }
    /* GROUP_REF */
    if (node->type == NodeType::GROUP_REF) {
        Automaton* nfa = nnfa.at(number);
        nfa->stateCount++;
        nfa->finalStates.insert(nfa->stateCount);

        /* если предыдущей операцией не было повторение с нижней границей = 0 */
        if (nfa->stateCount != nfa->transitions.size()) {
            AdjacencyList* transition = new AdjacencyList(-std::get<int>(node->data), nfa->stateCount);
            nfa->transitions.push_back(transition); 
        }
        else 
            nfa->transitions.back()->next = new AdjacencyList(-std::get<int>(node->data), nfa->stateCount);

        return;
    }
}

void NnfaPrepare(std::vector<Automaton*>& nnfa) {
    for (Automaton* nfa: nnfa)
        nfa->finalStates.insert(nfa->stateCount);
}

std::set<int> epsilonClosure(const Automaton& nfa, int state) {
    std::set<int> closure = {state};
    std::queue<int> queue;
    queue.push(state);

    while (!queue.empty()) {
        int current = queue.front();
        queue.pop();

        if (current >= nfa.transitions.size()) 
            break;
        AdjacencyList* transition = nfa.transitions.at(current);

        while (transition) {
            if (std::holds_alternative<std::pair<char, int>>(transition->transition)) {
                std::pair pair = std::get<std::pair<char, int>>(transition->transition);
                
                if (pair.first == '\0' && (closure.find(pair.second) == closure.end())) {
                    closure.insert(pair.second);
                    queue.push(pair.second);
                }
            }
            transition = transition->next;
        }
    }
    return closure;
}

std::vector<Automaton*> NnfaToNdfa(std::vector<Automaton*>& nnfa) {
    using KeyType = std::variant<char, int>;

    std::vector<Automaton*> ndfa;
    std::set<int> initialState; 

    for (Automaton* nfa: nnfa) {
        std::queue<std::set<int>> unmarkedStates;
        std::map<std::set<int>, int> dfaStates;
        initialState = epsilonClosure(*nfa, 0);
        unmarkedStates.push(initialState);
        Automaton* dfa;
        
        if (nfa->number == -1)
            dfa = new Automaton;
        else
            dfa = new Automaton(nfa->number);

        ndfa.push_back(dfa);
        dfaStates[initialState] = 0;
        dfa->stateCount = 1;

        for (int state : initialState) {
            if (nfa->finalStates.find(state) != nfa->finalStates.end())
                dfa->finalStates.insert(0);
        }
        while (!unmarkedStates.empty()) {
            std::set<int> currentState = unmarkedStates.front();
            unmarkedStates.pop();

            std::map<KeyType, std::set<int>> moves;

            for (int state : currentState) {
                if (state >= nfa->transitions.size()) 
                    continue;

                AdjacencyList* transition = nfa->transitions.at(state);

                while (transition) {
                    if (std::holds_alternative<std::pair<char, int>>(transition->transition)) {
                        std::pair pair = std::get<std::pair<char, int>>(transition->transition);
                    
                        if (pair.first != '\0') {
                            std::set<int> nextState = epsilonClosure(*nfa, pair.second);
                            moves[pair.first].insert(nextState.begin(), nextState.end());
                        }
                    }
                    else {
                        std::pair pair = std::get<std::pair<int, int>>(transition->transition);
                        std::set<int> nextState = epsilonClosure(*nfa, pair.second);
                        moves[pair.first].insert(nextState.begin(), nextState.end());
                    }
                    transition = transition->next;
                }
            }
            for (const auto& move : moves) {
                if (dfaStates.find(move.second) == dfaStates.end()) {
                    dfaStates[move.second] = dfa->stateCount;
                    dfa->stateCount++;
                    unmarkedStates.push(move.second);
                }
                int fromState = dfaStates[currentState];
                int toState = dfaStates[move.second];

                if (fromState >= dfa->transitions.size())
                    dfa->transitions.resize(fromState + 1, nullptr);

                AdjacencyList* newTransition;

                if (std::holds_alternative<char>(move.first))
                    newTransition = new AdjacencyList(std::get<char>(move.first), toState);
                else
                    newTransition = new AdjacencyList(std::get<int>(move.first), toState);

                newTransition->next = dfa->transitions[fromState];
                dfa->transitions[fromState] = newTransition;

                for (int state: move.second) {
                    if (nfa->finalStates.find(state) != nfa->finalStates.end())
                        dfa->finalStates.insert(toState);
                }
            }
        }
    }
    return ndfa;
}

int getTransition(const Automaton& dfa, int state, std::variant<int, char> symbol) {
    if (state >= dfa.transitions.size())
        return -1;

    AdjacencyList* currentTransition = dfa.transitions.at(state);

    while (currentTransition) {
        if (std::holds_alternative<std::pair<char, int>>(currentTransition->transition)) {
            std::pair pair = std::get<std::pair<char, int>>(currentTransition->transition);
           
            if (std::holds_alternative<char>(symbol)) 
                if (pair.first == std::get<char>(symbol))
                    return pair.second;
        }
        else { 
            std::pair pair = std::get<std::pair<int, int>>(currentTransition->transition);
           
            if (std::holds_alternative<int>(symbol))
                if (pair.first == std::get<int>(symbol))
                    return pair.second;
        }
        currentTransition = currentTransition->next;
    }
    return -1;             
}


std::vector<Automaton*> MinimizeNdfa(std::vector<Automaton*>& ndfa) {
    std::vector<Automaton*> minNdfa;

    for (const Automaton* dfa: ndfa) {
        std::unordered_set<std::variant<int, char>> alphabet;
        std::vector<std::set<int>> partition;

        for (AdjacencyList* currentTransition: dfa->transitions) {
            while (currentTransition) {
                if (std::holds_alternative<std::pair<char, int>>(currentTransition->transition)) 
                    alphabet.insert(std::get<std::pair<char, int>>(currentTransition->transition).first);
                else
                    alphabet.insert(std::get<std::pair<int, int>>(currentTransition->transition).first);

                currentTransition = currentTransition->next;
            }
        }
        std::set<int> accepting, nonAccepting;

        for (int i = 0; i < dfa->stateCount; ++i) {
            if (dfa->finalStates.count(i))
                accepting.insert(i);
            else 
                nonAccepting.insert(i);
        }
        if (!nonAccepting.empty()) partition.push_back(nonAccepting);
        if (!accepting.empty()) partition.push_back(accepting);
        bool isSplited = false;
        
        for (;;) { 
            for (int i = 0; i < partition.size(); ++i) {
                std::set<int> groupStates = partition.at(i);

                for (std::variant<int, char> symbol: alphabet) { 
                    for (int currentState: groupStates) {
                        int nextState = getTransition(*dfa, currentState, symbol);

                        if (nextState != -1) {
                            int nextSet;

                            for (nextSet = 0; nextSet < partition.size(); ++nextSet) {
                                if (partition.at(nextSet).count(nextState) > 0) 
                                    break;
                            }
                            std::set<int> groupStatesTmp;
                            groupStatesTmp.insert(currentState);

                            for (int stateTmp: groupStates) {
                                int nextStateTmp = getTransition(*dfa, stateTmp, symbol);

                                if (nextStateTmp != -1) {
                                    int nextSetTmp;

                                    for (nextSetTmp = 0; nextSetTmp < partition.size(); ++nextSetTmp)
                                        if (partition.at(nextSetTmp).count(nextStateTmp) > 0) 
                                            break;

                                    if (nextSetTmp == nextSet)
                                        groupStatesTmp.insert(stateTmp);
                                }
                            }
                            if (groupStatesTmp.size() != groupStates.size()) {
                                /* избавляемся от дупликатов */
                                for (int state: groupStatesTmp)
                                    partition.at(i).erase(state);

                                partition.push_back(groupStatesTmp);
                                isSplited = true;
                                break;
                            }
                            else 
                                break;    
                        } 
                    }
                    if (isSplited)
                        break;
                }
                if (isSplited)
                    break;
            }
            if (!isSplited)
                break;

            isSplited = false;
        }
        /* создаемый новый минимальный ДКА */
        Automaton* minDfa;
        
        if (dfa->number == -1)
            minDfa = new Automaton;
        else
            minDfa = new Automaton(dfa->number);

        minNdfa.push_back(minDfa);

        /* ищем начальное состояние */
        bool isFind = false;
        int setNum;

        for (setNum = 0; setNum < partition.size(); ++setNum) {
            for (int state: partition.at(setNum)) {
                if (state == 0) {
                    isFind = true;
                    break;
                }
            }
            if (isFind)
                break;
        }
        isFind = false;
        /* первая группа -> стартовая группа */
        std::set<int> setTmp = partition.at(setNum);
        partition.erase(partition.begin() + setNum);
        partition.insert(partition.begin(), setTmp);

        for (int i = 0; i < partition.size(); ++i) {
            for (int state: partition.at(i)) {
                if (dfa->finalStates.count(state)) {
                    minDfa->finalStates.insert(i);
                    break;
                }
            }
        }
        int newTransitionsCounter = 0;

        for (int i = 0; i < partition.size(); ++i) {
            int state = *partition.at(i).begin();

            if (state >= dfa->transitions.size()) {
                if (i != (partition.size() - 1))
                    minDfa->transitions.push_back(nullptr);

                continue;
            }
            AdjacencyList* tmpTransition = dfa->transitions.at(state);

            if (!tmpTransition) {
                minDfa->transitions.push_back(nullptr);
                continue;
            }
            int setNumTmp;

            for (setNumTmp = 0; setNumTmp < partition.size(); ++setNumTmp) {
                for (int stateTmp : partition.at(setNumTmp)) {
                    if (std::holds_alternative<std::pair<char, int>>(tmpTransition->transition)) {
                        std::pair<char, int> pair = std::get<std::pair<char, int>>(tmpTransition->transition);
                        
                        if (stateTmp == pair.second) {
                            isFind = true;
                            break;
                        }
                    }
                    else {
                        std::pair<int, int> pair = std::get<std::pair<int, int>>(tmpTransition->transition);
                        
                        if (stateTmp == pair.second) {
                            isFind = true;
                            break;
                        }
                    }
                }
                if (isFind)
                    break;
            }
            isFind = false;
            AdjacencyList* newTransition;

            if (std::holds_alternative<std::pair<char, int>>(tmpTransition->transition)) {
                std::pair<char, int> pair = std::get<std::pair<char, int>>(tmpTransition->transition);
                newTransition = new AdjacencyList(pair.first, setNumTmp);
            }
            else {
                std::pair<int, int> pair = std::get<std::pair<int, int>>(tmpTransition->transition);
                newTransition = new AdjacencyList(pair.first, setNumTmp);
            }
            minDfa->transitions.push_back(newTransition);

            while (tmpTransition->next) {
                tmpTransition = tmpTransition->next;

                for (setNumTmp = 0; setNumTmp < partition.size(); ++setNumTmp) {
                    for (int stateTmp : partition.at(setNumTmp)) {
                        if (std::holds_alternative<std::pair<char, int>>(tmpTransition->transition)) {
                            std::pair<char, int> pair = std::get<std::pair<char, int>>(tmpTransition->transition);
                            
                            if (stateTmp == pair.second) {
                                isFind = true;
                                break;
                            }
                        }
                        else {
                            std::pair<int, int> pair = std::get<std::pair<int, int>>(tmpTransition->transition);
                            
                            if (stateTmp == pair.second) {
                                isFind = true;
                                break;
                            }
                        }
                    }
                    if (isFind)
                        break;
                }
                isFind = false;

                if (std::holds_alternative<std::pair<char, int>>(tmpTransition->transition)) {
                    std::pair<char, int> pair = std::get<std::pair<char, int>>(tmpTransition->transition);
                    newTransition->next = new AdjacencyList(pair.first, setNumTmp);
                }
                else {
                    std::pair<int, int> pair = std::get<std::pair<int, int>>(tmpTransition->transition);
                    newTransition->next = new AdjacencyList(pair.first, setNumTmp);
                }
                newTransition = newTransition->next; 
            }
            newTransitionsCounter++;
        }
        /* включение стартового состояния */
        minDfa->stateCount = partition.size();
    }
    return minNdfa;
}

void PrintAutomata(const std::vector<Automaton*>& automata) {
    int counter;

    for (Automaton* automaton: automata) {
        counter = 0;
        std::cout << std::endl;

        if (automaton->number == -1) 
            std::cout << "Main automata" << std::endl;

        else 
            std::cout << "\"" << automaton->number << "\"" << " automata" << std::endl;    

        for (AdjacencyList* transition: automaton->transitions) {
            while (transition) {
                /* переход по символу */
                if (std::holds_alternative<std::pair<char, int>>(transition->transition)) {
                    std::cout << counter << " -> " << std::get<std::pair<char, int>>(transition->transition).second;
                    
                    if (std::get<std::pair<char, int>>(transition->transition).first != '\0')
                        std::cout << " '" << std::get<std::pair<char, int>>(transition->transition).first << "'" << std::endl;
                    
                    else 
                        std::cout << " epsilon" << std::endl;
                }
                /* переход по группе */
                else {
                    std::cout << counter << " -> " << std::get<std::pair<int, int>>(transition->transition).second;
                    std::cout << " \"" << std::get<std::pair<int, int>>(transition->transition).first << "\"" << std::endl;    
                }
                transition = transition->next;
            }
            counter++;
        }
        std::cout << "Final states:";

        for (int finalState: automaton->finalStates)
            std::cout << " " << finalState;

        std::cout << std::endl;
    }
    std::cout << std::endl;
}
