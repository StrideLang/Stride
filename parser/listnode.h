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

    void replaceMember(AST *replacement, AST *member);

    AST *deepCopy();
};

#endif // LISTNODE_H
