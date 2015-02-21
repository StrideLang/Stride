#include "codegen.h"

#include <QVector>

Codegen::Codegen(StreamPlatform &platform, AST *tree) :
    m_platform(platform), m_tree(tree)
{
    validate();
}

QList<LangError> Codegen::getErrors()
{
    return m_errors;
}

void Codegen::validate()
{
    checkTypeNames(m_tree);
}

void Codegen::checkTypeNames(AST *node)
{
    QVector<AST *> children = QVector<AST *>::fromStdVector(node->getChildren());
    if (node->getNodeType() == AST::BlockBundle
            || node->getNodeType() == AST::Block) {
        BlockNode *block = static_cast<BlockNode *>(node);
        if (m_platform.isValidType(QString::fromStdString(block->getObjectType()))) {
            LangError error;
            error.type = LangError::UnknownType;
            error.lineNumber = block->getLine();
            error.errorToken = QString::fromStdString(block->getObjectType());
            m_errors << error;
        }
    }

    foreach(AST *node, children) {
        checkTypeNames(node);
    }
}
