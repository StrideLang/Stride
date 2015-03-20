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
        BundleRange,
        BlockBundle,
        Block,
        Stream,
        Property,
        List,

        // Built-in types (leaf nodes)
        Int,
        Real,
        Name,
        String,
        Expression,
        Function,
        Switch,

        // Invalid
        Invalid
    } Token;

    AST(Token token, int line = -1);
    virtual ~AST();

    Token getNodeType() const { return m_token; }
    void addChild(AST *t);
    void giveChildren(AST *p); // Move all children nodes to be children of "parent" and make parent a child of this class
    bool isNil() { return m_token == AST::None; }

    vector<AST *> getChildren() const {return m_children;}
    void setChildren(vector<AST *> &newChildren);

    int getLine() const {return m_line;}

    void deleteChildren();

    virtual AST *deepCopy();

    static AST * parseFile(const char *fileName);

protected:
    Token m_token; // From which token did we create node?
    vector<AST *> m_children; // normalized list of children
    int m_line;
};

#endif // AST_H
