#include "listnode.h"

ListNode::ListNode(AST *newMember, int line) :
    AST(AST::List, line)
{
    if (newMember) {
        addChild(newMember);
    }
}

ListNode::~ListNode()
{

}

void ListNode::stealMembers(ListNode *list)
{
    list->giveChildren(this);
}

AST::Token ListNode::getListType()
{
    vector<AST *> children = getChildren();
    if (children.size() == 0) {
        return AST::Invalid;
    }
    Token type = children.at(0)->getNodeType();

    for(unsigned int i = 1; i < children.size(); i++) {
        Token nextType = children.at(i)->getNodeType();
        if (type == AST::Int && nextType == AST::Real) {
            type = AST::Real;
        } else if (type == AST::Real && nextType == AST::Int) {
            // Consider int as real
        } else if(nextType != type) {
            return AST::Invalid;
        }
    }
    return type;
}

int ListNode::size()
{
    return m_children.size();
}

void ListNode::replaceMember(AST *replacement, AST *member)
{
    vector<AST *> children = getChildren();
    for(unsigned int i = 1; i < children.size(); i++) {
        if (children.at(i) == member) {
            children.at(i) = replacement;
            member->deleteChildren();
            delete member;
            return;
        }
    }
}

AST *ListNode::deepCopy()
{
    vector<AST *> children = getChildren();
    ListNode *newList = new ListNode(children.at(0)->deepCopy(), m_line);
    for(unsigned int i = 1; i < children.size(); i++) {
        newList->addChild(children.at(i)->deepCopy());
    }
    return newList;
}

