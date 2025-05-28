#include "automaton.hpp"


void NnfaInit(std::vector<Automaton*>& nnfa, Node* node, int number) {
    if (node->type == NodeType::SYMBOL) {
        Automaton* nfa = nnfa.at(number);
        nfa->stateCount++;

        if (nfa->stateCount == nfa->transitions.size()) {
            AdjacencyList* transition = nfa->transitions.back();
            transition->next = new AdjacencyList(std::get<char>(node->data), nfa->stateCount);
            return;
        }
        AdjacencyList* transition = new AdjacencyList(std::get<char>(node->data), nfa->stateCount);
        nfa->transitions.push_back(transition);
        return;
    }
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
    if (node->type == NodeType::OR) {
        Automaton* nfa = nnfa.at(number);
        std::vector<int> startStateCounts;
        std::vector<int> endStateCounts;
        std::vector<bool> isStartTransitionsEndStates;
        bool isStartTransition = false;
        AdjacencyList* startTransition;
        nfa->finalStates.erase(nfa->stateCount);

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
    /*if (node->type == NodeType::REPEAT) {
        Automaton* nfa = nnfa.at(number);
        std::vector<int> range = std::get<std::vector<int>>(node->data);

        // '{}' 
        if (range.at(0) == range.at(1)) {
            Automaton* nfa = nnfa.at(number);

            for (int i = 0; i < range.at(0); ++i) {
                nfa->finalStates.erase(nfa->stateCount);
                NnfaInit(nnfa, node->childrens.at(0), number);
            }
            nfa->finalStates.insert(nfa->stateCount);
        }
        // '?'
        else if (range.at(1) == 1) {
            int tmpStateCount = nfa->stateCount;
            NnfaInit(nnfa, node->childrens.at(0), number);
            AdjacencyList* startTransition = nfa->transitions.at(tmpStateCount);
            AdjacencyList* tmpStartTransition;
            nfa->finalStates.insert(tmpStateCount);

            do {
                tmpStartTransition = startTransition;

                if (std::holds_alternative<std::pair<char, int>>(startTransition->transition)) {
                    std::pair<char, int> transition = std::get<std::pair<char, int>>(startTransition->transition) ;
                
                    if (transition.first == '\0' && transition.second == nfa->stateCount) {
                        return;
                    }
                }
                startTransition = startTransition->next;    
            } while (startTransition != nullptr);

            tmpStartTransition->next = new AdjacencyList('\0', nfa->stateCount);
        }
        // '...'
        else {
            int tmpStateCount = nfa->stateCount;
            NnfaInit(nnfa, node->childrens.at(0), number);
            AdjacencyList* startTransition = nfa->transitions.at(tmpStateCount);
            AdjacencyList* tmpStartTransition;
            bool isStartTransition = false;
            nfa->finalStates.insert(tmpStateCount);

            do {
                tmpStartTransition = startTransition;

                if (std::holds_alternative<std::pair<char, int>>(startTransition->transition)) {
                    std::pair<char, int> transition = std::get<std::pair<char, int>>(startTransition->transition) ;
                
                    if (transition.first == '\0' && transition.second == nfa->stateCount) {
                        isStartTransition = true;
                        break;
                    }
                }
                startTransition = startTransition->next;    
            } while (startTransition != nullptr);

            if (!isStartTransition) 
                tmpStartTransition->next = new AdjacencyList('\0', nfa->stateCount);

            if (nfa->stateCount != nfa->transitions.size()) 
                return;

            else {
                AdjacencyList* endTransition = new AdjacencyList('\0', tmpStateCount);
                nfa->transitions.push_back(endTransition);
            }
        }
    }*/
    /* GROUP */
    if (node->type == NodeType::GROUP) {
        Automaton* nfa = nnfa.at(number);
        nfa->stateCount++;
        nfa->finalStates.insert(nfa->stateCount);

        if (nfa->stateCount != nfa->transitions.size()) {
            AdjacencyList* transition = new AdjacencyList(std::get<int>(node->data), nfa->stateCount);
            nfa->transitions.push_back(transition); 
        }
        else 
            nfa->transitions.back()->next = new AdjacencyList(std::get<int>(node->data), nfa->stateCount);

        for (Automaton* tmpNfa: nnfa) {
            if (std::get<int>(node->data) == tmpNfa->number)
                return;
        }
        Automaton* groupNfa = new Automaton(std::get<int>(node->data));
        nnfa.push_back(groupNfa);
        NnfaInit(nnfa, node->childrens.at(0), (nnfa.size() - 1));
        return;
    }
}

void NnfaPrepare(std::vector<Automaton*>& nnfa) {
    for (Automaton* nfa: nnfa)
        nfa->finalStates.insert(nfa->stateCount);
}

void PrintNnfa(const std::vector<Automaton*>& nnfa) {
    int counter;

    for (Automaton* nfa: nnfa) {
        counter = 0;
        std::cout << std::endl;

        if (nfa->number == -1) 
            std::cout << "Main automata" << std::endl;

        else 
            std::cout << "\"" << nfa->number << "\"" << " automata" << std::endl;    

        for (AdjacencyList* transition: nfa->transitions) {
            while (transition != nullptr) {
                if (std::holds_alternative<std::pair<char, int>>(transition->transition)) {
                    std::cout << counter << " -> " << std::get<std::pair<char, int>>(transition->transition).second;
                    
                    if (std::get<std::pair<char, int>>(transition->transition).first != '\0')
                        std::cout << " '" << std::get<std::pair<char, int>>(transition->transition).first << "'" << std::endl;
                    
                    else 
                        std::cout << " epsilon" << std::endl;
                }
                else {
                    std::cout << counter << " -> " << std::get<std::pair<int, int>>(transition->transition).second;
                    std::cout << " \"" << std::get<std::pair<int, int>>(transition->transition).first << "\"" << std::endl;    
                }
                transition = transition->next;
            }
            counter++;
        }
        std::cout << "Final states:";

        for (int finalState: nfa->finalStates)
            std::cout << " " << finalState;

        std::cout << std::endl;
    }
    std::cout << std::endl;
}

/*std::set<int> epsilon_closure(const Automaton& nfa, int state) {
    std::set<int> closure = {state};
    std::queue<int> queue;
    queue.push(state);

    while (!queue.empty()) {
        int current = queue.front();
        queue.pop();

        if (current >= nfa.transitions.size()) 
            break;
        AdjacencyList* transition = nfa.transitions.at(current);

        while (transition != nullptr) {
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

std::vector<Dfa*> nfa_to_dfa(std::vector<Automaton*>& nnfa) {
    using KeyType = std::variant<char, std::string>;

    std::vector<Dfa*> ndfa;
    std::set<int> initial_state; 

    for (Automaton* nfa: nnfa) {
        std::queue<std::set<int>> unmarked_states;
        std::map<std::set<int>, int> dfa_states;
        initial_state = epsilon_closure(*nfa, 0);
        unmarked_states.push(initial_state);
        Dfa* dfa;
        
        if (nfa->name == "")
            dfa = new Dfa;
        else
            dfa = new Dfa(nfa->name);
        ndfa.push_back(dfa);
        dfa_states[initial_state] = 0;
        dfa->stateCount = 1;

        for (int state : initial_state) {
            if (nfa->finalStates.find(state) != nfa->finalStates.end())
                dfa->finalStates.insert(0);
        }
        while (!unmarked_states.empty()) {
            std::set<int> current_state = unmarked_states.front();
            unmarked_states.pop();

            std::map<KeyType, std::set<int>> moves;

            for (int state : current_state) {
                if (state >= nfa->transitions.size()) 
                    continue;
                AdjacencyList* transition = nfa->transitions.at(state);

                while (transition != nullptr) {
                    if (std::holds_alternative<std::pair<char, int>>(transition->transition)) {
                        std::pair pair = std::get<std::pair<char, int>>(transition->transition);
                    
                        if (pair.first != '\0') {
                            std::set<int> next_state = epsilon_closure(*nfa, pair.second);
                            moves[pair.first].insert(next_state.begin(), next_state.end());
                        }
                    }
                    else {
                        std::pair pair = std::get<std::pair<std::string, int>>(transition->transition);
                        std::set<int> next_state = epsilon_closure(*nfa, pair.second);
                        moves[pair.first].insert(next_state.begin(), next_state.end());
                    }
                    transition = transition->next;
                }
            }
            for (const auto& move : moves) {
                if (dfa_states.find(move.second) == dfa_states.end()) {
                    dfa_states[move.second] = dfa->stateCount;
                    dfa->stateCount++;
                    unmarked_states.push(move.second);
                }
                int from_state = dfa_states[current_state];
                int to_state = dfa_states[move.second];

                if (from_state >= dfa->transitions.size()) {
                    dfa->transitions.resize(from_state + 1, nullptr);
                }
                AdjacencyList* new_transition;

                if (std::holds_alternative<char>(move.first))
                    new_transition = new AdjacencyList(std::get<char>(move.first), to_state);
                else
                    new_transition = new AdjacencyList(std::get<std::string>(move.first), to_state);

                new_transition->next = dfa->transitions[from_state];
                dfa->transitions[from_state] = new_transition;

                for (int state: move.second) {
                    if (nfa->finalStates.find(state) != nfa->finalStates.end())
                        dfa->finalStates.insert(to_state);
                }
            }
        }
    }
    return ndfa;
}

void print_ndfa(const std::vector<Dfa*>& ndfa) {
    int counter;

    for (Dfa* dfa: ndfa) {
        counter = 0;
        std::cout << std::endl;

        if (dfa->name == "") 
            std::cout << "Main automata" << std::endl;
        else 
            std::cout << "\"" << dfa->name << "\"" << " automata" << std::endl;    

        for (AdjacencyList* transition: dfa->transitions) {
            while (transition != nullptr) {
                if (std::holds_alternative<std::pair<char, int>>(transition->transition)) {
                    std::cout << counter << " -> " << std::get<std::pair<char, int>>(transition->transition).second;
                    std::cout << " '" << std::get<std::pair<char, int>>(transition->transition).first << "'" << std::endl;
                }
                else {
                    std::cout << counter << " -> " << std::get<std::pair<std::string, int>>(transition->transition).second;
                    std::cout << " \"" << std::get<std::pair<std::string, int>>(transition->transition).first << "\"" << std::endl;    
                }
                transition = transition->next;
            }
            counter++;
        }
        std::cout << "Final states:";

        for (int final_state: dfa->finalStates)
            std::cout << " " << final_state; 
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

int get_transition(const Dfa& dfa, int state, std::variant<std::string, char> symbol) {
    if (state >= dfa.transitions.size())
        return -1;
    AdjacencyList* current_transition = dfa.transitions.at(state);

    while (current_transition != nullptr) {
        if (std::holds_alternative<std::pair<char, int>>(current_transition->transition)) {
            std::pair pair = std::get<std::pair<char, int>>(current_transition->transition);
           
            if (std::holds_alternative<char>(symbol)) { 
                if (pair.first == std::get<char>(symbol))
                    return pair.second;
            }
        }
        else { 
            std::pair pair = std::get<std::pair<std::string, int>>(current_transition->transition);
           
            if (std::holds_alternative<std::string>(symbol)) {
                if (pair.first == std::get<std::string>(symbol))
                    return pair.second;
            }
        }
        current_transition = current_transition->next;
    }
    return -1;             
}


std::vector<Dfa*> minimize_dfa(std::vector<Dfa*>& ndfa) {
    std::vector<Dfa*> min_ndfa;

    for (const Dfa* dfa: ndfa) {
        std::unordered_set<std::variant<std::string, char>> alphabet;
        std::vector<std::set<int>> partition;

        for (AdjacencyList* current_transition: dfa->transitions) {
            while (current_transition) {
                if (std::holds_alternative<std::pair<char, int>>(current_transition->transition)) 
                    alphabet.insert(std::get<std::pair<char, int>>(current_transition->transition).first);
                else
                    alphabet.insert(std::get<std::pair<std::string, int>>(current_transition->transition).first);
                current_transition = current_transition->next;
            }
        }
        std::set<int> accepting, non_accepting;

        for (int i = 0; i < dfa->stateCount; ++i) {
            if (dfa->finalStates.count(i))
                accepting.insert(i);
            else 
                non_accepting.insert(i);
        }
        if (!non_accepting.empty()) partition.push_back(non_accepting);
        if (!accepting.empty()) partition.push_back(accepting);
        bool is_splited = false;
        
        for (;;) { 
            for (int i = 0; i < partition.size(); ++i) {
                std::set<int> group_states = partition.at(i);

                for (std::variant<std::string, char> symbol: alphabet) { 
                    for (int current_state: group_states) {
                        int next_state = get_transition(*dfa, current_state, symbol);

                        if (next_state != -1) {
                            int next_set;

                            for (next_set = 0; next_set < partition.size(); ++next_set) {
                                if (partition.at(next_set).count(next_state) > 0) 
                                    break;
                            }
                            std::set<int> group_states_tmp;
                            group_states_tmp.insert(current_state);

                            for (int state_tmp: group_states) {
                                int next_state_tmp = get_transition(*dfa, state_tmp, symbol);

                                if (next_state_tmp != -1) {
                                    int next_set_tmp;

                                    for (next_set_tmp = 0; next_set_tmp < partition.size(); ++next_set_tmp) {
                                        if (partition.at(next_set_tmp).count(next_state_tmp) > 0) 
                                            break;
                                    }
                                    if (next_set_tmp == next_set)
                                        group_states_tmp.insert(state_tmp);
                                }
                            }
                            if (group_states_tmp.size() != group_states.size()) {
                                // removing duplicates
                                for (int state: group_states_tmp)
                                    partition.at(i).erase(state);
                                partition.push_back(group_states_tmp);
                                is_splited = true;
                                break;
                            }
                            else {
                                break;    
                            }
                        } 
                    }
                    if (is_splited)
                        break;
                }
                if (is_splited)
                    break;
            }
            if (!is_splited)
                break;
            is_splited = false;
        }
        // Creating a new minimized DFA
        Dfa* min_dfa;
        
        if (dfa->name == "")
            min_dfa = new Dfa;
        else
            min_dfa = new Dfa(dfa->name);
        min_ndfa.push_back(min_dfa);

        // Looking for the initial state
        bool is_find = false;
        int set_num;

        for (set_num = 0; set_num < partition.size(); ++set_num) {
            for (int state: partition.at(set_num)) {
                if (state == 0) {
                    is_find = true;
                    break;
                }
            }
            if (is_find)
                break;
        }
        is_find = false;
        // first group -> start group
        std::set<int> set_tmp = partition.at(set_num);
        partition.erase(partition.begin() + set_num);
        partition.insert(partition.begin(), set_tmp);

        for (int i = 0; i < partition.size(); ++i) {
            for (int state: partition.at(i)) {
                if (dfa->finalStates.count(state)) {
                    min_dfa->finalStates.insert(i);
                    break;
                }
            }
        }
        //min_dfa->transitions.resize(partition.size(), nullptr); 
        int new_transitions_counter = 0;

        for (int i = 0; i < partition.size(); ++i) {
            int state = *partition.at(i).begin();

            if (state >= dfa->transitions.size()) {
                if (i != (partition.size() - 1))
                    min_dfa->transitions.push_back(nullptr);
                continue;
            }
            AdjacencyList* tmp_transition = dfa->transitions.at(state);

            if (tmp_transition == nullptr) {
                min_dfa->transitions.push_back(nullptr);
                continue;
            }
            int set_num_tmp;

            for (set_num_tmp = 0; set_num_tmp < partition.size(); ++set_num_tmp) {
                for (int state_tmp : partition.at(set_num_tmp)) {
                    if (std::holds_alternative<std::pair<char, int>>(tmp_transition->transition)) {
                        std::pair<char, int> pair = std::get<std::pair<char, int>>(tmp_transition->transition);
                        
                        if (state_tmp == pair.second) {
                            is_find = true;
                            break;
                        }
                    }
                    else {
                        std::pair<std::string, int> pair = std::get<std::pair<std::string, int>>(tmp_transition->transition);
                        
                        if (state_tmp == pair.second) {
                            is_find = true;
                            break;
                        }
                    }
                }
                if (is_find)
                    break;
            }
            is_find = false;
            AdjacencyList* new_transition;

            if (std::holds_alternative<std::pair<char, int>>(tmp_transition->transition)) {
                std::pair<char, int> pair = std::get<std::pair<char, int>>(tmp_transition->transition);
                new_transition = new AdjacencyList(pair.first, set_num_tmp);
            }
            else {
                std::pair<std::string, int> pair = std::get<std::pair<std::string, int>>(tmp_transition->transition);
                new_transition = new AdjacencyList(pair.first, set_num_tmp);
            }
            min_dfa->transitions.push_back(new_transition);

            while (tmp_transition->next != nullptr) {
                tmp_transition = tmp_transition->next;

                for (set_num_tmp = 0; set_num_tmp < partition.size(); ++set_num_tmp) {
                    for (int state_tmp : partition.at(set_num_tmp)) {
                        if (std::holds_alternative<std::pair<char, int>>(tmp_transition->transition)) {
                            std::pair<char, int> pair = std::get<std::pair<char, int>>(tmp_transition->transition);
                            
                            if (state_tmp == pair.second) {
                                is_find = true;
                                break;
                            }
                        }
                        else {
                            std::pair<std::string, int> pair = std::get<std::pair<std::string, int>>(tmp_transition->transition);
                            
                            if (state_tmp == pair.second) {
                                is_find = true;
                                break;
                            }
                        }
                    }
                    if (is_find)
                        break;
                }
                is_find = false;

                if (std::holds_alternative<std::pair<char, int>>(tmp_transition->transition)) {
                    std::pair<char, int> pair = std::get<std::pair<char, int>>(tmp_transition->transition);
                    new_transition->next = new AdjacencyList(pair.first, set_num_tmp);
                }
                else {
                    std::pair<std::string, int> pair = std::get<std::pair<std::string, int>>(tmp_transition->transition);
                    new_transition->next = new AdjacencyList(pair.first, set_num_tmp);
                }
                new_transition = new_transition->next; 
            }
            new_transitions_counter++;
        }
        min_dfa->stateCount = partition.size();
    }
    return min_ndfa;
}*/
