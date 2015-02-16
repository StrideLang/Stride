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

