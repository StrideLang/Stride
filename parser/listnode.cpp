#include "listnode.h"

ListNode::ListNode(AST *newMember) :
    AST(AST::List)
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

