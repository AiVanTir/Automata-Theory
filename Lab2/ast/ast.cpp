#include <algorithm>
#include "ast.hpp"

bool Ast::CollectNumbers(std::vector<int>& groupNumbers, std::vector<int>& groupRefNumbers, Node* node) {
    for (Node* children: node->childrens) 
        if (!CollectNumbers(groupNumbers, groupRefNumbers, children))
            return false;

    if (node->type == NodeType::GROUP) {
        int groupNumber = std::get<int>(node->data);
        groupNumbers.push_back(groupNumber);
    }
    if (node->type == NodeType::GROUP_REF) {
        if (groupNumbers.empty()) 
            return false;

        int groupRefNumber = std::get<int>(node->data);
        groupRefNumbers.push_back(groupRefNumber);

        return std::find(groupNumbers.begin(), groupNumbers.end(), groupRefNumber) != groupNumbers.end();
    }
    return true;
}

bool Ast::Prepare() {
    if (root == nullptr) 
        return true;

    std::vector<int> groupNumbers;
    std::vector<int> groupRefNumbers;

    if (!CollectNumbers(groupNumbers, groupRefNumbers, root)) 
        return false;  

    std::set<int> uniqueNumbers;
    
    for (const auto& groupNumber : groupNumbers) {
        if (!uniqueNumbers.insert(groupNumber).second)
            return false;
    }
    return true;
}
