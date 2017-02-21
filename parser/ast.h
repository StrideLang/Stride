#ifndef AST_H
#define AST_H

#include "langerror.h"

using namespace std;

class AST
{
public:
    AST();

    typedef enum {
        None,
        Platform,
        Bundle,
        Declaration,
        BundleDeclaration,
        Stream,
        Property,
        Range,
        List,
        Import,
        For,
        Scope,
        PortProperty,

        // Built-in types (leaf nodes)
        Int = 0x80,
        Real = 0x81,
        String = 0x82,
        Block = 0x20,
        Expression = 0x21,
        Function = 0x22,
        Switch = 0x23,
        Keyword = 0x24,

        // Invalid
        Invalid
    } Token;

    AST(Token token, const char *filename, int line = -1);
    virtual ~AST();

    Token getNodeType() const { return m_token; }
    virtual void addChild(AST *t);
    void giveChildren(AST *p); // Move all children nodes to be children of "parent" and make parent a child of this class
    bool isNil() { return m_token == AST::None; }

    vector<AST *> getChildren() const {return m_children;}
    virtual void setChildren(vector<AST *> &newChildren);

    int getLine() const {return m_line;}

    virtual void deleteChildren();

    virtual AST *deepCopy();


    static AST * parseFile(const char *fileName);
    static vector<LangError> getParseErrors();

    double getRate() const;
    void setRate(double rate);

    string getFilename() const;
    void setFilename(const string &filename);

    virtual void resolveScope(AST* scope);

    void addScope(string newScope);
    unsigned int getScopeLevels ();
    string getScopeAt(unsigned int scopeLevel);

protected:
    Token m_token; // From which token did we create node?
    vector<AST *> m_children; // normalized list of children
    string m_filename; // file where the node was generated
    int m_line;
    double m_rate;
    vector<string> m_scope;
};

#endif // AST_H
