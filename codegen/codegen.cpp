#include "codegen.h"

#include <QVector>

Codegen::Codegen(StreamPlatform &platform, AST *tree) :
    m_platform(platform), m_tree(tree)
{
    validate();
}

bool Codegen::isValid()
{
    return m_errors.size() == 0;
}

QList<LangError> Codegen::getErrors()
{
    return m_errors;
}

void Codegen::validate()
{
    m_errors.clear();
    checkTypeNames(m_tree);
    checkProperties(m_tree);
}

void Codegen::checkTypeNames(AST *node)
{
    QVector<AST *> children = QVector<AST *>::fromStdVector(node->getChildren());
    if (node->getNodeType() == AST::BlockBundle
            || node->getNodeType() == AST::Block) {
        BlockNode *block = static_cast<BlockNode *>(node);
        if (!m_platform.isValidType(QString::fromStdString(block->getObjectType()))) {
            LangError error;
            error.type = LangError::UnknownType;
            error.lineNumber = block->getLine();
            error.errorTokens << QString::fromStdString(block->getObjectType());
            m_errors << error;
        }
    }

    foreach(AST *node, children) {
        checkTypeNames(node);
    }
}

void Codegen::checkProperties(AST *node)
{
    QVector<AST *> children = QVector<AST *>::fromStdVector(node->getChildren());
    if (node->getNodeType() == AST::BlockBundle
            || node->getNodeType() == AST::Block) {
        BlockNode *block = static_cast<BlockNode *>(node);
        QString blockType = QString::fromStdString(block->getObjectType());
        vector<PropertyNode *> properties = block->getProperties();
        foreach(PropertyNode *property, properties) {
            QString propertyName = QString::fromStdString(property->getName());
            if (!m_platform.typeHasProperty(blockType, propertyName)) {
                LangError error;
                error.type = LangError::InvalidProperty;
                error.lineNumber = block->getLine();
                error.errorTokens << blockType << propertyName;
                m_errors << error;
            } else {
                AST *propertyValue = property->getValue();
                QString propertyType;
                // TODO validate non basic types
                switch (propertyValue->getNodeType()) {
                case AST::None:
                    propertyType = "none";
                    break;
                case AST::Int:
                    propertyType = "int";
                    break;
                case AST::Real:
                    propertyType = "real";
                    break;
                case AST::Name:
                    propertyType = "name";
                    break;
                case AST::String:
                    propertyType = "string";
                    break;
                    // TODO must evaluate expressions and functions to check their output types
                    //            case AST::Expression:
                    //                propertyType = "expression";
                    //                break;
                    //            case AST::Function:
                    //                propertyType = "string";
                    //                break;
                case AST::Switch:
                    propertyType = "switch";
                    break;
                }

                if (!m_platform.isValidPropertyType(blockType, propertyName, propertyType)) {
                    LangError error;
                    error.type = LangError::InvalidPropertyType;
                    error.lineNumber = block->getLine();
                    error.errorTokens << blockType << propertyName << propertyType;
                    m_errors << error;
                }
            }
        }
    }

    foreach(AST *node, children) {
        checkProperties(node);
    }
}
