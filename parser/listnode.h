#ifndef LISTNODE_H
#define LISTNODE_H

#include "ast.h"

class ListNode : public AST
{
public:
    ListNode(AST *newMember, int line);
    ~ListNode();

    void stealMembers(ListNode *list);
    Token getListType();

    int size();

    AST *deepCopy();
};

#endif // LISTNODE_H
