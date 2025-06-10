#pragma once

#include <vector>
#include <variant>
#include <set>
#include <climits>

enum class NodeType {
    OR,
    CONCAT,
    REPEAT,
    GROUP,
    SYMBOL,
    GROUP_REF
};

typedef struct Node {
    std::variant<std::pair<int, int>, int, char> data;
    std::vector<Node*> childrens;
    NodeType type;
    
    /* OR | CONCAT */ 
    Node(NodeType type, std::vector<Node*> childrens) {
        this->type = type;
        this->childrens = childrens;
    } 
    /* REPEAT */
    Node(NodeType type, std::pair<int, int> range, Node* node) {
        this->type = type;
        data = range;
        childrens.push_back(node);
    }
    /* GROUP */
    Node(NodeType type, int groupNumber, Node* node) {
        this->type = type;
        data = groupNumber;
        childrens.push_back(node);
    }
    /* SYMBOL */
    Node(NodeType type, const char symbol) {
        this->type = type;
        data = symbol;
    }
    /* GROUP_REF */
    Node(NodeType type, int groupNumber) {
        this->type = type;
        data = groupNumber;
    }
    ~Node() {
        for (Node* children: childrens)
            delete children;
    }
} Node;

typedef struct Ast {
    Node* root;

    Ast(Node* root) {
        this->root = root;
    }
    ~Ast() {
        delete root;
    }
    bool Prepare();
    bool CollectNumbers(std::vector<int>& groupNumbers, std::vector<int>& groupRefNumbers, Node* node);
} Ast; 
