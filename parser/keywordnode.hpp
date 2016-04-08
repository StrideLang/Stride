#ifndef KEYWORDNODE_HPP
#define KEYWORDNODE_HPP

#include "ast.h"

class KeywordNode : public AST
{
public:
    KeywordNode(std::string keyword, int line);

    std::string keyword() {return m_kw;}

    virtual AST *deepCopy() {
        return new KeywordNode(keyword(), getLine());
    }

private:
    std::string m_kw;
};

#endif // KEYWORDNODE_HPP
