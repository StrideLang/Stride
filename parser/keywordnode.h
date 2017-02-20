#ifndef KEYWORDNODE_H
#define KEYWORDNODE_H

#include "ast.h"

class KeywordNode : public AST
{
public:
    KeywordNode(std::string keyword, const char *filename, int line);

    std::string keyword() {return m_kw;}

    virtual AST *deepCopy();

private:
    std::string m_kw;
};

#endif // KEYWORDNODE_H
