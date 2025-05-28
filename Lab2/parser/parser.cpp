#include <cctype>
#include "parser.hpp"

Node* Parser::ParseGroupRef() {
    int oldPosition = position;

    if (!MatchAndConsume('\\'))
        return nullptr;

    long long result = 0;
    bool isNumber = false;
    int number;
    while (position < pattern.size() && isdigit(pattern[position])) {
        isNumber = true;
        number = pattern[position] - '0';

        if (result > (INT_MAX - number) / 10) {
            result = INT_MAX;
            break;
        }
        result = result * 10 + number;
        position++;
    }
    if (isNumber)
        return new Node(NodeType::GROUP_REF, static_cast<int>(result));

    position = oldPosition;
    return nullptr;
}

Node* Parser::ParseSymbol() {
    int oldPosition = position;
    char symbol;
    if (MatchAndConsume('#')) {
        if (!GetSymbol(&symbol)) {
            position = oldPosition;
            return nullptr;
        }
        else {
            return new Node(NodeType::SYMBOL, symbol);
        }
    }
    if (Match('|') ||
        Match('.') ||
        Match('+') ||
        Match('{') ||
        Match(',') ||
        Match('}') ||
        Match('(') ||
        Match(':') ||
        Match(')') ||
        Match('\\')) {
        return nullptr;
    }
    if (!GetSymbol(&symbol))
        return nullptr;

    if (symbol == '^')
        return new Node(NodeType::SYMBOL, '\0');

    return new Node(NodeType::SYMBOL, symbol);

}

Node* Parser::ParseGroupedExpr() {
    int oldPosition = position;

    if (!MatchAndConsume('('))
        return nullptr;

    if (MatchAndConsume(')')) 
        return new Node(NodeType::SYMBOL, '\0');

    Node* newNode = ParseExpr();
    
    if (!MatchAndConsume(')') || newNode == nullptr) {
        position = oldPosition;
        return nullptr;
    }
    return newNode;
}

Node* Parser::ParseGroup() {
    int oldPosition = position;

    if (!MatchAndConsume('('))
        return nullptr;

    std::string string = ParseUntilSymbol(':');

    if (string.size() == 0) {
        position = oldPosition;
        return nullptr;
    }
    if (!MatchAndConsume(':')) {
        position = oldPosition;
        return nullptr;
    }
    int groupNumber;
    long long result = 0;

    for (int i = 0; i < string.size(); ++i) {
        if (!isdigit(string[i])) {
            position = oldPosition;
            return nullptr;
        }
        groupNumber = string[i] - '0';
        if (result > (INT_MAX - groupNumber) / 10) {
            position = oldPosition;
            return nullptr;
        }
        result = result * 10 + groupNumber;
    }
    groupNumber = static_cast<int>(result);
    int tmpPosition = position;
    Node* newNode = ParseExpr();

    if (!MatchAndConsume(')') || newNode == nullptr) {
        position = oldPosition;
        return nullptr;
    }
    return new Node(NodeType::GROUP, groupNumber, newNode);
}

Node* Parser::ParseAtom() {
    Node* newNode = ParseGroup();

    if (newNode != nullptr)
        return newNode;
    newNode = ParseGroupedExpr();

    if (newNode != nullptr)
        return newNode;
    newNode = ParseSymbol();

    if (newNode != nullptr)
        return newNode;
    newNode = ParseGroupRef();

    if (newNode != nullptr)
        return newNode;

    return nullptr;
}

Node* Parser::ParseRepeat() {
    int oldPosition = position;
    Node* newNode = ParseAtom();
    Node* nodeToReturn;

    if (newNode == nullptr)
        return nullptr;

    for (;;) {
        if (MatchAndConsume('+')) {
            std::pair<int, int> range = {1, INT_MAX};
            nodeToReturn = new Node(NodeType::REPEAT, range, newNode);
            newNode = nodeToReturn;
            continue;
        }
        if (MatchAndConsume('{')) {
            std::string string = ParseUntilSymbol(',');
            if (string.size() == 0) {
                position = oldPosition;
                delete nodeToReturn;
                return nullptr;
            }
            if (!MatchAndConsume(',')) {
                position = oldPosition;
                delete nodeToReturn;
                return nullptr;
            }
            int lowerBound, upperBound;
            long long result;

            for (int i = 0; i < string.size(); ++i) {
                if (!isdigit(string[i])) {
                    position = oldPosition;
                    delete nodeToReturn;
                    return nullptr;
                }
                lowerBound = string[i] - '0';
                if (result > (INT_MAX - lowerBound) / 10) {
                    result = INT_MAX;
                    continue;
                }
                result = result * 10 + lowerBound;
            }
            lowerBound = static_cast<int>(result);
            string = ParseUntilSymbol('}');
            
            if (!MatchAndConsume('}')) {
                position = oldPosition;
                delete nodeToReturn;
                return nullptr;
            }
            if (string.size() == 0) 
                upperBound = INT_MAX;

            for (int i = 0; i < string.size(); ++i) {
                if (!isdigit(string[i])) {
                    position = oldPosition;
                    delete nodeToReturn;
                    return nullptr;
                }
                upperBound = string[i] - '0';
                if (result > (INT_MAX - upperBound) / 10) {
                    result = INT_MAX;
                    continue;
                }
                result = result * 10 + upperBound;
            }
            std::pair<int, int> range = {lowerBound, upperBound};
            nodeToReturn = new Node(NodeType::REPEAT, range, newNode);
            newNode = nodeToReturn;
            continue;
        }
        return newNode;
    }
}

Node* Parser::ParseConcat() {
    int oldPosition = position;
    Node* firstNode = ParseRepeat();

    if (firstNode == nullptr)
        return nullptr;

    std::vector<Node*> childrens;
    childrens.push_back(firstNode);
    Node* nextNode;

    for (;;) {
        if (Match('|') || position == pattern.size())
            break;
        if (MatchAndConsume('.')) {
            nextNode = ParseRepeat();

            if (nextNode == nullptr) {
                for (auto children : childrens)
                    delete children;

                position = oldPosition;
                return nullptr;
            }
            childrens.push_back(nextNode);
            continue;
        }
        if (Match(')'))
            break;
        nextNode = ParseRepeat();

        if (nextNode == nullptr) {
            for (auto children : childrens)
                delete children;

            position = oldPosition;
            return nullptr;
        } 
        childrens.push_back(nextNode);
    }
    if (childrens.size() == 1)
        return firstNode;

    return new Node(NodeType::CONCAT, childrens);
}

Node* Parser::ParseOr() {
    int oldPosition = position;
    Node* firstNode = ParseConcat();

    if (firstNode == nullptr) 
        return nullptr;
    
    std::vector<Node*> childrens;
    childrens.push_back(firstNode);
    Node* nextNode;

    while (MatchAndConsume('|')) {
        nextNode = ParseConcat();
        
        if (nextNode == nullptr) {
            for (auto children : childrens)
                delete children;

            oldPosition = position;
            return nullptr;
        }
        childrens.push_back(nextNode);
    }
    if (childrens.size() == 1)
        return firstNode; 

    return new Node(NodeType::OR, childrens);
}

Node* Parser::ParseExpr() {
    if (pattern.size() == 0)
        return new Node(NodeType::SYMBOL, '\0');

    return ParseOr();
}
