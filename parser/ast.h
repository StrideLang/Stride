#ifndef AST_H
#define AST_H

#include <vector>

#include "lang_stream.parser.hpp"

typedef enum yytokentype Token;

using namespace std;

class AST
{
public:
    AST();
    AST(Token token);
    AST(int tokenType);
    ~AST();

    int getNodeType() { return m_token; }
    void addChild(AST t);
    bool isNil() { return m_token == NONE; }

private:
    Token m_token; // From which token did we create node?
    vector<AST> m_children; // normalized list of children
};

#endif // AST_H
