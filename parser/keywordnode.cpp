#include "keywordnode.hpp"

KeywordNode::KeywordNode(string keyword, int line)
    :AST(AST::Keyword, line)
{
    m_kw = keyword;
}
