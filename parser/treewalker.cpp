#include "treewalker.h"

TreeWalker::TreeWalker(AST *tree) :
    m_tree(tree)
{

}

TreeWalker::~TreeWalker()
{

}

vector<AST *> TreeWalker::findPlatform()
{
    vector<AST *> platformNodes;
    vector<AST *> nodes = m_tree->getChildren();
    for(unsigned int i = 0; i < nodes.size(); i++) {
        if (nodes.at(i)->getNodeType() == AST::Platform) {
            platformNodes.push_back(nodes.at(i));
        }
    }
    return platformNodes;
}
