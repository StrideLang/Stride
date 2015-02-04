#include "functionnode.h"

FunctionNode::FunctionNode(string name, AST *propertiesList, FunctionType type) :
    AST(AST::Function)
{
    m_name = name;
    if (propertiesList) {
        propertiesList->giveChildren(this);
    }
    m_type = type;
}

FunctionNode::~FunctionNode()
{

}

