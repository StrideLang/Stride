#include "keywordnode.hpp"

KeywordNode::KeywordNode(string keyword, const char *filename, int line)
    :AST(AST::Keyword, filename, line)
{
    m_kw = keyword;
}

AST *KeywordNode::deepCopy() {
    return new KeywordNode(keyword(), m_filename.data(), getLine());
}
