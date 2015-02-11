#ifndef AST_H
#define AST_H

#include <vector>

using namespace std;

class AST
{
public:
    AST();

    typedef enum {
        None,
        Platform,
        Bundle,
        BlockBundle,
        Block,
        Stream,
        Property,
        List,
        Value,

        // Built-in types (leaf nodes)
        Int,
        Float,
        Name,
        String,
        Expression,
        Function,
        Switch
    } Token;

    AST(Token token);
    ~AST();

    Token getNodeType() const { return m_token; }
    void addChild(AST *t);
    void giveChildren(AST *p); // Move all children nodes to be children of "parent" and make parent a child of this class
    bool isNil() { return m_token == AST::None; }

    vector<AST *> getChildren() const {return m_children;}

    void deleteChildren();

protected:
    Token m_token; // From which token did we create node?
    vector<AST *> m_children; // normalized list of children
};

#endif // AST_H
