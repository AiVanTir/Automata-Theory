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
        
/*void Regex::PrintDfa() {
    if (!isCompiled) {
        std::cerr << "Сначала скомпилируйте" << std::endl;
    }
    //PrintNdfa(pMinNdfaImpl->minNdfa);
}*/

bool Regex::Compile(std::string_view pattern) {
    Parser parser(pattern);
    Node* root = parser.ParseExpr();
    if (root == nullptr)
        return false;
    Ast ast(root);
    if (!ast.Prepare()) 
        return false;

    std::vector<Automaton*> nnfa;
    Automaton nfa;
    nnfa.push_back(&nfa);
    NnfaInit(nnfa, root, 0);
    NnfaPrepare(nnfa);
    //std::vector<Automaton*> minNdfa = MinimizeNdfa(ndfa);
    //std::vector<Dfa*> ndfa;
    //std::vector<Dfa*> minNdfa;

#if 1
    std::cout << std::endl << "NFA COUNT = " << nnfa.size() << std::endl;
    PrintNnfa(nnfa);

    //std::cout << "Кол-во ДКА = " << ndfa.size() << std::endl;
    //PrintNdfa(ndfa);

    //std::cout << "Кол-во минимальных ДКА = " << minNdfa.size() << std::endl;
    //PrintNdfa(minNdfa);
#endif

    /*for (int i = 0; i < ndfa.size(); ++i)
        delete ndfa.at(i);

    delete this->pMinNdfaImpl; 
    this->pMinNdfaImpl = new MinNdfaImpl(minNdfa);*/

    return true;
}

/*bool Regex::group_match(int group_dfa_num, std::string_view string_to_check, int& position, RegexData& data) {
    Dfa* group_dfa = pMinNdfaImpl->min_ndfa.at(group_dfa_num);
    bool is_find = false;
    int current_state = 0;
    int position_save = position;

    for (;position < string_to_check.length(); ++position) {
        if (current_state >= group_dfa->transitions.size()) {
            if (group_dfa->final_states.count(current_state) > 0) {
                for (int i = 0; i < data.captured_groups.size(); ++i) {
                    if (data.captured_groups.at(i).first == group_dfa->name) {
                        data.captured_groups.at(i).second = string_to_check.substr(position_save, position);
                        return true;
                    }
                }
                std::pair<std::string, std::string> pair(group_dfa->name, string_to_check.substr(position_save, position));
                data.captured_groups.push_back(pair);
                return true;
            }
            else {
                return false;
            }
        }
        Adjacency_list* current_transition = group_dfa->transitions.at(current_state);

        while (current_transition) {
            if (std::holds_alternative<std::pair<char, int>>(current_transition->transition)) {
                char symbol = string_to_check.at(position);
                std::pair<char, int> pair = std::get<std::pair<char, int>>(current_transition->transition);
                
                if (pair.first == symbol) {
                    current_state = pair.second;
                    is_find = true;
                    break;
                }
            }
            else {
                std::pair<std::string, int> pair = std::get<std::pair<std::string, int>>(current_transition->transition);
                int group_dfa_num;

                for (group_dfa_num = 0; group_dfa_num < pMinNdfaImpl->min_ndfa.size(); ++group_dfa_num) {
                    if (pMinNdfaImpl->min_ndfa.at(group_dfa_num)->name == pair.first) 
                        break;
                }
                if (!group_match(group_dfa_num, string_to_check, position, data)) 
                    return false;
                position--;
                current_state = pair.second;
                is_find = true;
                break;
            }        
            current_transition = current_transition->next;
        }
        if (is_find) {
            is_find = false;
            continue;
        }
        if (group_dfa->final_states.count(current_state) > 0) {
            for (int i = 0; i < data.captured_groups.size(); ++i) {
                if (data.captured_groups.at(i).first == group_dfa->name) {
                    data.captured_groups.at(i).second = string_to_check.substr(position_save, position);
                    return true;
                }
            }
            std::pair<std::string, std::string> pair(group_dfa->name, string_to_check.substr(position_save, position));
            data.captured_groups.push_back(pair);
            return true;
        }
        return false;
    }
    if (!(current_state >= group_dfa->transitions.size())) {
        Adjacency_list* current_transition = group_dfa->transitions.at(current_state);

        while (current_transition) {
            if (std::holds_alternative<std::pair<std::string, int>>(current_transition->transition)) {
                std::pair<std::string, int> pair = std::get<std::pair<std::string, int>>(current_transition->transition);
                int group_dfa_num;

                for (group_dfa_num = 0; group_dfa_num < pMinNdfaImpl->min_ndfa.size(); ++group_dfa_num) {
                    if (pMinNdfaImpl->min_ndfa.at(group_dfa_num)->name == pair.first) 
                        break;
                }
                int str_len = string_to_check.length();

                if (group_match(group_dfa_num, string_to_check, str_len, data)) {
                    current_state = pair.second;
                    break;
                }
            }
            current_transition = current_transition->next;
        }
    }
    if (group_dfa->final_states.count(current_state) > 0) {
        for (int i = 0; i < data.captured_groups.size(); ++i) {
            if (data.captured_groups.at(i).first == group_dfa->name) {
                data.captured_groups.at(i).second = string_to_check.substr(position_save, position);
                return true;
            }
        }
        std::pair<std::string, std::string> pair(group_dfa->name, string_to_check.substr(position_save, position));
        data.captured_groups.push_back(pair);
        return true;
    }
    return false;
}

bool Regex::match(std::string_view string_to_match, RegexData& data) {
    if (!is_compiled) {
        std::cerr << "Error: The regular expression is not compiled. Please call the compile() method first." << std::endl;
        throw std::logic_error("Error");
    }
    data.matched_string = "";
    data.captured_groups.clear();
    bool is_find = false;
    int current_state = 0;
    Dfa* min_dfa = pMinNdfaImpl->min_ndfa.at(0);
    
    for (int i = 0; i < string_to_match.length(); ++i) {
        if (current_state >= min_dfa->transitions.size()) {
            data.captured_groups.clear();
            return false;
        }
        Adjacency_list* current_transition = min_dfa->transitions.at(current_state);

        while (current_transition) {
            if (std::holds_alternative<std::pair<char, int>>(current_transition->transition)) {
                char symbol = string_to_match.at(i);
                std::pair<char, int> pair = std::get<std::pair<char, int>>(current_transition->transition);
                
                if (pair.first == symbol) {
                    current_state = pair.second;
                    is_find = true;
                    break;
                }
            }
            else {
                std::pair<std::string, int> pair = std::get<std::pair<std::string, int>>(current_transition->transition);
                int group_dfa_num;

                for (group_dfa_num = 0; group_dfa_num < pMinNdfaImpl->min_ndfa.size(); ++group_dfa_num) {
                    if (pMinNdfaImpl->min_ndfa.at(group_dfa_num)->name == pair.first) 
                        break;
                }
                if (!group_match(group_dfa_num, string_to_match, i, data)) {
                    data.captured_groups.clear();
                    return false;
                }
                i--;
                current_state = pair.second;
                is_find = true;
                break;
            }        
            current_transition = current_transition->next;
        }
        if (is_find) {
            is_find = false;
            continue;
        }
        data.captured_groups.clear();
        return false;
    }
    if (!(current_state >= min_dfa->transitions.size())) {
        Adjacency_list* current_transition = min_dfa->transitions.at(current_state);

        while (current_transition) {
            if (std::holds_alternative<std::pair<std::string, int>>(current_transition->transition)) {
                std::pair<std::string, int> pair = std::get<std::pair<std::string, int>>(current_transition->transition);
                int group_dfa_num;

                for (group_dfa_num = 0; group_dfa_num < pMinNdfaImpl->min_ndfa.size(); ++group_dfa_num) {
                    if (pMinNdfaImpl->min_ndfa.at(group_dfa_num)->name == pair.first) 
                        break;
                }
                int str_len = string_to_match.length();

                if (group_match(group_dfa_num, string_to_match, str_len, data)) {
                    current_state = pair.second;
                    break;
                }
            }
            current_transition = current_transition->next;
        }
    }
    if (min_dfa->final_states.count(current_state) > 0) {
        data.matched_string = string_to_match;
        return true;
    }
    data.captured_groups.clear();
    return false;
}

bool Regex::match(std::string_view cregex, std::string_view string_to_match, RegexData& data) {
    compile(cregex);

    data.matched_string = "";
    data.captured_groups.clear();
    bool is_find = false;
    int current_state = 0;
    Dfa* min_dfa = pMinNdfaImpl->min_ndfa.at(0);
    
    for (int i = 0; i < string_to_match.length(); ++i) {
        if (current_state >= min_dfa->transitions.size()) {
            data.captured_groups.clear();
            return false;
        }
        Adjacency_list* current_transition = min_dfa->transitions.at(current_state);

        while (current_transition) {
            if (std::holds_alternative<std::pair<char, int>>(current_transition->transition)) {
                char symbol = string_to_match.at(i);
                std::pair<char, int> pair = std::get<std::pair<char, int>>(current_transition->transition);
                
                if (pair.first == symbol) {
                    current_state = pair.second;
                    is_find = true;
                    break;
                }
            }
            else {
                std::pair<std::string, int> pair = std::get<std::pair<std::string, int>>(current_transition->transition);
                int group_dfa_num;

                for (group_dfa_num = 0; group_dfa_num < pMinNdfaImpl->min_ndfa.size(); ++group_dfa_num) {
                    if (pMinNdfaImpl->min_ndfa.at(group_dfa_num)->name == pair.first) 
                        break;
                }
                if (!group_match(group_dfa_num, string_to_match, i, data)) {
                    data.captured_groups.clear();
                    return false;
                }
                i--;
                current_state = pair.second;
                is_find = true;
                break;
            }        
            current_transition = current_transition->next;
        }
        if (is_find) {
            is_find = false;
            continue;
        }
        data.captured_groups.clear();
        return false;
    }
    if (!(current_state >= min_dfa->transitions.size())) {
        Adjacency_list* current_transition = min_dfa->transitions.at(current_state);

        while (current_transition) {
            if (std::holds_alternative<std::pair<std::string, int>>(current_transition->transition)) {
                std::pair<std::string, int> pair = std::get<std::pair<std::string, int>>(current_transition->transition);
                int group_dfa_num;

                for (group_dfa_num = 0; group_dfa_num < pMinNdfaImpl->min_ndfa.size(); ++group_dfa_num) {
                    if (pMinNdfaImpl->min_ndfa.at(group_dfa_num)->name == pair.first) 
                        break;
                }
                int str_len = string_to_match.length();

                if (group_match(group_dfa_num, string_to_match, str_len, data)) {
                    current_state = pair.second;
                    break;
                }
            }
            current_transition = current_transition->next;
        }
    }
    if (min_dfa->final_states.count(current_state) > 0) {
        data.matched_string = string_to_match;
        return true;
    }
    data.captured_groups.clear();
    return false;
}

bool Regex::search(std::string_view string_to_search, RegexData& data) {
    if (!is_compiled) {
        std::cerr << "Error: The regular expression is not compiled. Please call the compile() method first." << std::endl;
        throw std::logic_error("Error");
    }
    if (match("", data))
        return true;
    for (int i = 0; i <= string_to_search.size(); ++i) {
        if (match(string_to_search.substr(0, i), data))
            return true;
    }
    return false;
}

bool Regex::search(std::string_view cregex, std::string_view string_to_search, RegexData& data) {
    compile(cregex);
    
    if (match("", data))
        return true;
    for (int i = 0; i <= string_to_search.size(); ++i) {
        if (match(string_to_search.substr(0, i), data))
            return true;
    }
    return false;
}

std::string Regex::recover_regex() {
    if (!is_compiled) {
        std::cerr << "Error: The regular expression is not compiled. Please call the compile() method first." << std::endl;
        throw std::logic_error("Error");
    }
    Dfa* dfa = pMinNdfaImpl->min_ndfa.at(0);
    std::map<int, std::string> regexMap;

    for (int state : dfa->final_states) {
        regexMap[state] = ""; 
    }
    for (int i = 0; i < dfa->state_count; ++i) {
        if (regexMap.find(i) == regexMap.end()) {
            regexMap[i] = ""; 
        }
    }
    for (int stateToRemove = 1; stateToRemove < dfa->state_count; ++stateToRemove) {
        if (dfa->final_states.count(stateToRemove) > 0 || stateToRemove == 0) {
            continue;
        }
        for (int p = 0; p < dfa->state_count; ++p) {
            Adjacency_list* transition = dfa->transitions.at(p); 
            while (transition != nullptr) {
                if (std::get_if<std::pair<char, int>>(&transition->transition) &&
                    std::get<std::pair<char, int>>(transition->transition).second == stateToRemove) {
                    char symbol = std::get<std::pair<char, int>>(transition->transition).first;

                    if (!regexMap[p].empty()) {
                        regexMap[p] += "|"; 
                    }
                    regexMap[p] += symbol + (regexMap[stateToRemove].empty() ? "" : regexMap[stateToRemove]);
                }
                transition = transition->next; 
            }
        }
    }
    return regexMap[0]; 
}

std::vector<Adjacency_list*> invert_transitions(const std::vector<Adjacency_list*>& transitions) {
    std::vector<Adjacency_list*> inverted_transitions(transitions.size(), nullptr);

    for (size_t from_state = 0; from_state < transitions.size(); ++from_state) {
        Adjacency_list* current_transition = transitions[from_state];

        while (current_transition) {
            int to_state;

            if (std::holds_alternative<std::pair<char, int>>(current_transition->transition)) 
                to_state = std::get<std::pair<char, int>>(current_transition->transition).second;
            else 
                to_state = std::get<std::pair<std::string, int>>(current_transition->transition).second;

            Adjacency_list* new_node;

            if (std::holds_alternative<std::pair<char, int>>(current_transition->transition)) {
                char symbol = std::get<std::pair<char, int>>(current_transition->transition).first;
                new_node = new Adjacency_list(symbol, from_state);
            } 
            else {
                std::string group_name = std::get<std::pair<std::string, int>>(current_transition->transition).first;
                new_node = new Adjacency_list(group_name, from_state);
            }
            if (to_state >= inverted_transitions.size()) {
                inverted_transitions.push_back(nullptr);
            }
            if (inverted_transitions[to_state] == nullptr)
                inverted_transitions[to_state] = new_node;
            else {
                Adjacency_list* node = inverted_transitions[to_state];

                while (node->next != nullptr) {
                    node = node->next;
                }
                node->next = new_node;
            }
            current_transition = current_transition->next;
        }
    }
    return inverted_transitions;
}

void Regex::inverse_regex() {
    if (!is_compiled) {
        std::cerr << "Error: The regular expression is not compiled. Please call the compile() method first." << std::endl;
        throw std::logic_error("Error");
    }
    std::vector<Nfa*> nnfa;

    for (Dfa* dfa: pMinNdfaImpl->min_ndfa) {
        if (dfa->state_count == dfa->final_states.size()) {
            Nfa* nfa = new Nfa(dfa->name);
            nfa->final_states = dfa->final_states;
            nfa->transitions = dfa->transitions;
            nfa->state_count = dfa->state_count - 1;

            if (dfa->transitions.size() == dfa->state_count) 
                nfa->state_count++;
            for (int i = 0; i < dfa->transitions.size(); ++i)
                dfa->transitions.at(i) = nullptr;

            delete dfa;
            nnfa.push_back(nfa);
            continue;
        }
        Nfa* nfa = new Nfa(dfa->name);
        nfa->state_count = dfa->state_count;
        nfa->final_states.insert(1);
        std::vector<Adjacency_list*> inverted_transitions = invert_transitions(dfa->transitions);
        nfa->transitions = inverted_transitions;

        std::vector<int> old_final_states(dfa->final_states.begin(), dfa->final_states.end());
        Adjacency_list* new_transition = new Adjacency_list('\0', (old_final_states.at(0) + 1));
        nfa->transitions.insert(nfa->transitions.begin(), new_transition); 

        for (int i = 1; i < old_final_states.size(); ++i) {
            new_transition->next = new Adjacency_list('\0', (old_final_states.at(i) + 1));
            new_transition = new_transition->next;
        }
        for (int i = 1; i < nfa->transitions.size(); ++i) {
            Adjacency_list* transition = nfa->transitions.at(i);

            while (transition) {
                if (std::holds_alternative<std::pair<char, int>>(transition->transition)) {
                    std::pair<char, int>* pair = std::get_if<std::pair<char, int>>(&transition->transition);
                    pair->second++;
                }
                else {
                    std::pair<std::string, int>* pair = std::get_if<std::pair<std::string, int>>(&transition->transition);
                    pair->second++;
                }
                transition = transition->next;
            }
        }
        delete dfa;
        nnfa.push_back(nfa); 
    }     
    std::vector<Dfa*> ndfa = nfa_to_dfa(nnfa);
    std::vector<Dfa*> min_ndfa = minimize_dfa(ndfa);

#if 0
    std::cout << std::endl << "NFA COUNT = " << nnfa.size() << std::endl;
    print_nnfa(nnfa);

    std::cout << "DFA COUNT = " << ndfa.size() << std::endl;
    print_ndfa(ndfa);

    std::cout << "MIN DFA COUNT = " << min_ndfa.size() << std::endl;
    print_ndfa(min_ndfa);
#endif
    
    for (int i = 0; i < nnfa.size(); ++i)
        delete nnfa.at(i);
    for (int i = 0; i < ndfa.size(); ++i)
        delete ndfa.at(i);

    this->pMinNdfaImpl->min_ndfa = min_ndfa;
}
        
void Regex::complement_regex() {
    if (!is_compiled) {
        std::cerr << "Error: The regular expression is not compiled. Please call the compile() method first." << std::endl;
        throw std::logic_error("Error");
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
    for (int i = 0; i < pMinNdfaImpl->min_ndfa.size(); ++i) {
        Dfa* dfa = pMinNdfaImpl->min_ndfa.at(i);

        // Empty regex case
        if (dfa->transitions.size() == 0) {
            dfa->state_count++;
            dfa->final_states.erase(0);
            dfa->final_states.insert(1);

            Adjacency_list* first_transition = new Adjacency_list(static_cast<char>(start), 1);
            dfa->transitions.push_back(first_transition);

            for (int j = start + 1; j <= end; ++j) {
                first_transition->next = new Adjacency_list(static_cast<char>(j), 1);
                first_transition = first_transition->next;
            }
            Adjacency_list* second_transition = new Adjacency_list(static_cast<char>(start), 1);
            dfa->transitions.push_back(second_transition);
            
            for (int j = start + 1; j <= end; ++j) {
                second_transition->next = new Adjacency_list(static_cast<char>(j), 1);
                second_transition = second_transition->next;
            }
            continue;
        }
        // Group dfa case
        if (i >= 1) {
            for (int j = 0; j < dfa->state_count; ++j) 
                dfa->final_states.insert(j);
            continue;
        }
        // Main dfa case
        dfa->state_count++;
        std::set<int> new_final_states;
        
        for (int j = 0; j < dfa->state_count; ++j) {
            if (dfa->final_states.find(j) == dfa->final_states.end()) 
                new_final_states.insert(j);
        }
        dfa->final_states = new_final_states;
        Adjacency_list* end_transition = new Adjacency_list(static_cast<char>(start), dfa->state_count - 1);
        if (dfa->transitions.size() == (dfa->state_count - 2))
            dfa->transitions.push_back(nullptr);
        dfa->transitions.push_back(end_transition); 

        for (int j = start + 1; j <= end; ++j) {
            end_transition->next = new Adjacency_list(static_cast<char>(j), dfa->state_count - 1);
            end_transition = end_transition->next;
        }
        for (int j = 0; j < dfa->transitions.size() - 1; ++j) {
            Adjacency_list* current_transition = dfa->transitions.at(j);

            if (current_transition == nullptr) {
                current_transition = new Adjacency_list(static_cast<char>(start), dfa->state_count - 1);
                dfa->transitions.at(j) = current_transition;
            }
            else {
                while (current_transition->next != nullptr) 
                current_transition = current_transition->next;
                current_transition->next = new Adjacency_list(static_cast<char>(start), dfa->state_count - 1);
                current_transition = current_transition->next;
            }
            for (int k = start + 1; k <= end; ++k) {
                current_transition->next = new Adjacency_list(static_cast<char>(k), dfa->state_count - 1);
                current_transition = current_transition->next;
            }
        }
    }
}*/
