#pragma once

#include <iostream>
#include <algorithm>
#include <queue>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include "ast.hpp"

typedef struct AdjacencyList {
    std::variant<std::pair<char, int>, std::pair<int, int>> transition;
    AdjacencyList* next = nullptr;

    AdjacencyList() {}
    
    /* EPSILON | SYMBOL */
    AdjacencyList(char symbol, int stateNum) {
        transition = std::make_pair(symbol, stateNum);
    }
    /* GROUP */
    AdjacencyList(int groupNumber, int stateNum) {
        transition = std::make_pair(groupNumber, stateNum);
    }
} AdjacencyList;

typedef struct Automaton {
    /* номер основого автомата = -1, остальных - номер группы */
    int number = -1;
    int stateCount = 0;
    std::set<int> finalStates;
    std::vector<AdjacencyList*> transitions;

    /* основной автомат */
    Automaton() {}
    /* автомат для групп */
    Automaton(int number) {
        this->number = number;
    }
    ~Automaton() {
        AdjacencyList* tmpTransition;

        for (AdjacencyList* transition: transitions) {
            while (transition != nullptr) {
                tmpTransition = transition->next;
                delete transition;
                transition = tmpTransition;
            }
        }
    }
} Automaton;

void NnfaInit(std::vector<Automaton*>& nnfa, Node* node, int number);
void NnfaPrepare(std::vector<Automaton*>& nnfa);

std::vector<Automaton*> NnfaToNdfa(std::vector<Automaton*>& nnfa);

void PrintAutomata(const std::vector<Automaton*>& automata);

std::vector<Automaton*> MinimizeNdfa(std::vector<Automaton*>& ndfa);
