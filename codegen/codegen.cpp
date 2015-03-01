#include <QVector>

#include "codegen.h"
#include "treewalker.h"

// FIXME check what here should be moved to the parser/TreeWalker class
// Little can be moved as tree walker doesn't know about the platform...
// Maybe move treewalker here instead?

Codegen::Codegen(StreamPlatform &platform, AST *tree) :
    m_platform(platform), m_tree(tree)
{
    validate();
}

Codegen::Codegen(QString platformRootDir, AST *tree):
    m_platform(platformRootDir), m_tree(tree)
{
    TreeWalker walker(tree);
    QVector<AST *> platforms = QVector<AST *>::fromStdVector(walker.findPlatform());
    if (platforms.size() > 0) {
        PlatformNode *platformNode = static_cast<PlatformNode *>(platforms.at(0));
        // FIXME add error if more than one platform?
        StreamPlatform platform(platformRootDir,
                                QString::fromStdString(platformNode->platformName()),
                                QString::number(platformNode->version(),'f',  1));
        m_platform = platform;
    }
    validate();
}

bool Codegen::isValid()
{
    return m_errors.size() == 0;
}

bool Codegen::platformIsValid()
{
    return m_platform.getErrors().size() == 0;
}

QList<LangError> Codegen::getErrors()
{
    return m_errors;
}

QStringList Codegen::getPlatformErrors()
{
    return m_platform.getErrors();
}

void Codegen::validate()
{
    m_errors.clear();
    validateTypeNames(m_tree);
    validateProperties(m_tree, QVector<AST *>());
    validateBundleIndeces(m_tree, QVector<AST *>());
    validateBundleSizes(m_tree, QVector<AST *>());
    validateSymbolUniqueness(m_tree, QVector<AST *>());
    // TODO: Check for duplicate symbols
    // TODO: Validate list consistency
    // TODO: validate expression type consistency
    // TODO: validate expression list operations

    // TODO: resolve constants (and store the results of the resolution (maybe replace the tree nodes?) - should this be done in the tree walker?
    sortErrors();
}

void Codegen::validateTypeNames(AST *node)
{
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

    QVector<AST *> children = QVector<AST *>::fromStdVector(node->getChildren());
    foreach(AST *node, children) {
        validateTypeNames(node);
    }
}

void Codegen::validateProperties(AST *node, QVector<AST *> scope)
{
    if (node->getNodeType() == AST::BlockBundle
            || node->getNodeType() == AST::Block) {
        BlockNode *block = static_cast<BlockNode *>(node);
        QString blockType = QString::fromStdString(block->getObjectType());
        vector<PropertyNode *> ports = block->getProperties();
        foreach(PropertyNode *port, ports) {
            QString portName = QString::fromStdString(port->getName());
            // Check if portname is valid
            if (!m_platform.typeHasPort(blockType, portName)) {
                LangError error;
                error.type = LangError::InvalidPort;
                error.lineNumber = block->getLine();
                error.errorTokens << blockType << portName;
                m_errors << error;
            } else {
                AST *portValue = port->getValue();
                QString portTypeName = getPortTypeName(resolveNodeOutType(portValue, scope));

                if (!m_platform.isValidPortType(blockType, portName, portTypeName)) {
                    LangError error;
                    error.type = LangError::InvalidPortType;
                    error.lineNumber = block->getLine();
                    error.errorTokens << blockType << portName << portTypeName;
                    m_errors << error;
                }
            }
        }
    }

    QVector<AST *> children = QVector<AST *>::fromStdVector(node->getChildren());
    foreach(AST *node, children) {
        validateProperties(node, children);
    }
}

void Codegen::validateBundleIndeces(AST *node, QVector<AST *> scope)
{
    if (node->getNodeType() == AST::Bundle) {
        BundleNode *bundle = static_cast<BundleNode *>(node);
        PortType type = resolveNodeOutType(bundle->index(), scope);
        if(type != ConstInt && type != ControlInt) {
            LangError error;
            error.type = LangError::IndexMustBeInteger;
            error.lineNumber = bundle->getLine();
            error.errorTokens << QString::fromStdString(bundle->getName()) << getPortTypeName(type);
            m_errors << error;
        }
    } else if (node->getNodeType() == AST::BundleRange) {

    }
    QVector<AST *> children = QVector<AST *>::fromStdVector(node->getChildren());
    foreach(AST *node, children) {
        validateBundleIndeces(node, children);
    }
}

void Codegen::validateBundleSizes(AST *node, QVector<AST *> scope)
{
    if (node->getNodeType() == AST::BlockBundle) {
        QList<LangError> errors;
        BlockNode *block = static_cast<BlockNode *>(node);
        int size = getBlockBundleDeclaredSize(block, scope, errors);
        int datasize = getConstBlockDataSize(block, scope, errors);
        if(size != datasize) {
            LangError error;
            error.type = LangError::BundleSizeMismatch;
            error.lineNumber = node->getLine();
            error.errorTokens << QString::fromStdString(block->getBundle()->getName())
                              << QString::number(size) << QString::number(datasize);
            m_errors << error;
        }

        // TODO : use this pass to store the computed value of constant int?
        m_errors << errors;
    }

    QVector<AST *> children = QVector<AST *>::fromStdVector(node->getChildren());
    foreach(AST *node, children) {
        validateBundleSizes(node, children);
    }
}

void Codegen::validateSymbolUniqueness(AST *node, QVector<AST *> scope)
{
    // TODO: This only checks symbol uniqueness within its scope...

    while (!scope.isEmpty() && scope.takeFirst() != node) { ; }

    foreach(AST *sibling, scope) {
        if(node != sibling) {
            QString nodeName, siblingName;
            if (sibling->getNodeType() == AST::Block
                    || sibling->getNodeType() == AST::BlockBundle) {
                siblingName = QString::fromStdString(static_cast<BlockNode *>(sibling)->getName());
            }
            if (node->getNodeType() == AST::Block
                    || node->getNodeType() == AST::BlockBundle) {
                nodeName = QString::fromStdString(static_cast<BlockNode *>(node)->getName());
            }
            if (!nodeName.isEmpty() && nodeName == siblingName) {
                LangError error;
                error.type = LangError::DuplicateSymbol;
                error.lineNumber = node->getLine();
                error.errorTokens << nodeName
                                  << QString::number(sibling->getLine());
                m_errors << error;
            }
        }
    }



    QVector<AST *> children = QVector<AST *>::fromStdVector(node->getChildren());
    foreach(AST *node, children) {
        validateSymbolUniqueness(node, children);
    }
}

bool errorLineIsLower(const LangError &err1, const LangError &err2)
{
    return err1.lineNumber < err2.lineNumber;
}

void Codegen::sortErrors()
{
    std::sort(m_errors.begin(), m_errors.end(), errorLineIsLower);
}

int Codegen::getBlockBundleDeclaredSize(BlockNode *block, QVector<AST *> scope, QList<LangError> &errors)
{
    Q_ASSERT(block->getNodeType() == AST::BlockBundle);
    BundleNode *bundle = static_cast<BundleNode *>(block->getBundle());
    if (bundle->getNodeType() == AST::Bundle) { // BundleRange not acceptable here (in declaration)
        PortType type = resolveNodeOutType(bundle->index(), scope);
        if (type == ConstInt) {
            int size = evaluateConstInteger(bundle->index(), scope, errors);
            return size;
        }
    }
    return -1;
}

int Codegen::getConstBlockDataSize(BlockNode *block, QVector<AST *> scope, QList<LangError> &errors)
{
    if (block->getObjectType() == "constant") {
        QVector<PropertyNode *> ports = QVector<PropertyNode *>::fromStdVector(block->getProperties());
        foreach(PropertyNode *port, ports) {
            if(port->getName() == "value") {
                AST *value = port->getValue();
                if (value->getNodeType() == AST::List) {
                    return value->getChildren().size();
                } else if (value->getNodeType() == AST::Bundle) {
                    return 1;
                } else if (value->getNodeType() == AST::Int
                           || value->getNodeType() == AST::Real
                           || value->getNodeType() == AST::String) {
                    return 1;
                } else if (value->getNodeType() == AST::Expression) {
                    // TODO: evaluate
                } else if (value->getNodeType() == AST::Name) {
                    NameNode *name = static_cast<NameNode *>(value);
                    BlockNode *block = findDeclaration(QString::fromStdString(name->getName()), scope);
                    return getBlockBundleDeclaredSize(block, scope, errors);
                } else if (value->getNodeType() == AST::BundleRange) {
                    BundleNode *bundle = static_cast<BundleNode *>(value);
                    if (resolveNodeOutType(bundle->endIndex(), scope) == ConstInt
                            && resolveNodeOutType(bundle->endIndex(), scope) == ConstInt) {
                        return evaluateConstInteger(bundle->endIndex(), scope, errors)
                                - evaluateConstInteger(bundle->startIndex(), scope, errors) + 1;
                    }
                }
            }
        }
    }
    return -1;
}

BlockNode *Codegen::findDeclaration(QString bundleName, QVector<AST *>scope)
{
    QVector<AST *> globalAndLocal;
    globalAndLocal << scope << QVector<AST *>::fromStdVector(m_tree->getChildren());
    foreach(AST *node, globalAndLocal) {
        if (node->getNodeType() == AST::BlockBundle) {
            BlockNode *block = static_cast<BlockNode *>(node);
            BundleNode *bundle = block->getBundle();
            QString name = QString::fromStdString(bundle->getName());
            if (name == bundleName) {
                return block;
            }
        } else if (node->getNodeType() == AST::Block) {
            BlockNode *block = static_cast<BlockNode *>(node);
            QString name = QString::fromStdString(block->getName());
            if (name == bundleName) {
                return block;
            }
        }
    }
    return NULL;
}

Codegen::PortType Codegen::resolveBundleType(BundleNode *bundle, QVector<AST *>scope)
{
    QString bundleName = QString::fromStdString(bundle->getName());
    BlockNode *declaration = findDeclaration(bundleName, scope);
    if(declaration) {
        if (declaration->getObjectType() == "constant") {
            vector<PropertyNode *> properties = declaration->getProperties();
            foreach(PropertyNode *property, properties)  {
                if(property->getName() == "value") {
                    return resolveNodeOutType(property->getValue(), scope);
                }
            }
        } else {
//            return QString::fromStdString(declaration->getObjectType());
        }
    }
    return None;
}

Codegen::PortType Codegen::resolveNodeOutType(AST *node, QVector<AST *> scope)
{
    if (node->getNodeType() == AST::Int) {
        return ConstInt;
    } else if (node->getNodeType() == AST::Real) {
        return ConstReal;
    } else if (node->getNodeType() == AST::Switch) {
        return ConstBoolean;
    } else if (node->getNodeType() == AST::String) {
        return ConstString;
    } else if(node->getNodeType() == AST::List) {
        return resolveListType(static_cast<ListNode *>(node), scope);
    }  else if(node->getNodeType() == AST::Bundle) {
        return resolveBundleType(static_cast<BundleNode *>(node), scope);
    } else if (node->getNodeType() == AST::Expression) {
        return resolveExpressionType(static_cast<ExpressionNode *>(node), scope);
    }

    return None;
}

Codegen::PortType Codegen::resolveListType(ListNode *listnode, QVector<AST *> scope)
{
    QVector<AST *> members = QVector<AST *>::fromStdVector(listnode->getChildren());
    if (members.isEmpty()) {
        return None;
    }
    AST *firstMember = members.takeFirst();
    PortType type = resolveNodeOutType(firstMember, scope);

    foreach(AST *member, members) {
        PortType nextPortType = resolveNodeOutType(member, scope);
        if (type != nextPortType) {
            if (type == ConstInt && nextPortType == ConstReal) { // List becomes Real if Real found
                type = ConstReal;
            } else if (type == ConstReal && nextPortType == ConstInt) { // Int in Real list
                // Nothing here for now
            } else { // Invalid combination
                return Invalid;
            }
        }
    }

    return type;
}

Codegen::PortType Codegen::resolveExpressionType(ExpressionNode *exprnode, QVector<AST *> scope)
{
    // TODO: implement expression node type resolution
    return None;
}

int Codegen::evaluateConstInteger(AST *node, QVector<AST *> scope, QList<LangError> &errors)
{
    int result = 0;
    if (node->getNodeType() == AST::Int) {
        return static_cast<ValueNode *>(node)->getIntValue();
    } else if (node->getNodeType() == AST::Bundle) {
        BundleNode *bundle = static_cast<BundleNode *>(node);
        QString bundleName = QString::fromStdString(bundle->getName());
        BlockNode *declaration = findDeclaration(bundleName, scope);
        int index = evaluateConstInteger(bundle->index(), scope, errors);
        if(declaration && declaration->getNodeType() == AST::BlockBundle) {
            AST *member = getMemberfromBlockBundle(declaration, index, errors);
            return evaluateConstInteger(member, scope, errors);
        }
    } else if (node->getNodeType() == AST::Expression) {
        // TODO: check expression out
    } else {
        LangError error;
        error.type = LangError::InvalidType;
        error.lineNumber = node->getLine();
        error.errorTokens << getPortTypeName(resolveNodeOutType(node, scope));
        errors << error;
    }
    return result;
}

AST *Codegen::getMemberfromBlockBundle(BlockNode *node, int index, QList<LangError> &errors)
{
    AST *out = NULL;
    if (node->getObjectType() == "constant") {
        BlockNode *block = static_cast<BlockNode *>(node);
        QVector<PropertyNode *> ports = QVector<PropertyNode *>::fromStdVector(block->getProperties());
        foreach(PropertyNode *port, ports) {
            if(port->getName() == "value") {
                AST *value = port->getValue();
                if (value->getNodeType() == AST::List) {
                    return getMemberFromList(static_cast<ListNode *>(value), index, errors);
                } else if (value->getNodeType() == AST::Bundle) {
                    // TODO: do something here
                }
            }
        }
    } else {
        // TODO: What to do with other cases?
    }
    return out;
}

AST *Codegen::getMemberFromList(ListNode *node, int index, QList<LangError> &errors)
{
    if (index < 1 || index > (int) node->getChildren().size()) {
        LangError error;
        error.type = LangError::ArrayIndexOutOfRange;
        error.lineNumber = node->getLine();
        error.errorTokens << QString::number(index);
        errors << error;
        return NULL;
    }
    return node->getChildren()[index - 1];
}

QString Codegen::getPortTypeName(Codegen::PortType type)
{
    switch (type) {
    case Audio:
        return "ASP";
        break;
    case ControlReal:
        return "CSRP";
        break;
    case ControlInt:
        return "CSRP";
        break;
    case ControlBoolean:
        return "CSBP";
        break;
    case ControlString:
        return "CSSP";
        break;
    case ConstReal:
        return "CRP";
        break;
    case ConstInt:
        return "CIP";
        break;
    case ConstBoolean:
        return "CBP";
        break;
    case ConstString:
        return "CSP";
        break;
    case None:
        return "none";
        break;
    case Invalid:
        return "";
        break;
    default:
        break;
    }
    return "";
}

