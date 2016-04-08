#include <QVector>

#include "codevalidator.h"
#include "coderesolver.h"

CodeValidator::CodeValidator(QString platformRootDir, AST *tree):
    m_platform(platformRootDir), m_library(platformRootDir), m_tree(tree)
{
    if(tree) {
        QVector<PlatformNode *> platforms = getPlatformNodes();
        if (platforms.size() > 0) {
            PlatformNode *platformNode = platforms.at(0);
            // FIXME add error if more than one platform?
            QStringList platformRoots;
            platformRoots << platformRootDir;
            StreamPlatform platform(platformRoots,
                                    QString::fromStdString(platformNode->platformName()),
                                    QString::number(platformNode->version(),'f',  1));
            m_platform = platform;
        }
    }
    validate();
}

bool CodeValidator::isValid()
{
    return m_errors.size() == 0;
}

bool CodeValidator::platformIsValid()
{
    return m_platform.getErrors().size() == 0;
}

QVector<PlatformNode *> CodeValidator::getPlatformNodes()
{
    Q_ASSERT(m_tree);
//    if (!m_tree) {
//        return QVector<PlatformNode *>();
//    }
    QVector<PlatformNode *> platformNodes;
    vector<AST *> nodes = m_tree->getChildren();
    for(unsigned int i = 0; i < nodes.size(); i++) {
        if (nodes.at(i)->getNodeType() == AST::Platform) {
            platformNodes.push_back(static_cast<PlatformNode *>(nodes.at(i)));
        }
    }
    return platformNodes;
}

QList<LangError> CodeValidator::getErrors()
{
    return m_errors;
}

QStringList CodeValidator::getPlatformErrors()
{
    return m_platform.getErrors();
}

StreamPlatform *CodeValidator::getPlatform()
{
    return &m_platform;
}

void CodeValidator::validate()
{
    m_errors.clear();
    if(m_tree) {
        CodeResolver resolver(m_platform, m_tree);
        resolver.preProcess();
        validateTypes(m_tree, QVector<AST *>());
        validateBundleIndeces(m_tree, QVector<AST *>());
        validateBundleSizes(m_tree, QVector<AST *>());
        validateSymbolUniqueness(m_tree, QVector<AST *>());
        validateListTypeConsistency(m_tree, QVector<AST *>());
        validateStreamSizes(m_tree, QVector<AST *>());

        // TODO: validate expression type consistency
        // TODO: validate expression list operations

        // TODO: resolve constants (and store the results of the resolution (maybe replace the tree nodes?) - should this be done in the tree walker?
    }
    sortErrors();
}

void CodeValidator::validateTypes(AST *node, QVector<AST *> scope)
{
    if (node->getNodeType() == AST::BlockBundle
            || node->getNodeType() == AST::Block) {
        BlockNode *block = static_cast<BlockNode *>(node);
        QString blockType = QString::fromStdString(block->getObjectType());
        BlockNode *typeBlock = NULL;
        if (m_platform.isValidType(blockType)) { // Check if node type exists
            // Validate property names and types
            vector<PropertyNode *> ports = block->getProperties();
            foreach(PropertyNode *port, ports) {
                QString portName = QString::fromStdString(port->getName());
                // Check if portname is valid
                if (!m_platform.typeHasPort(blockType, portName)) {
                    LangError error;
                    error.type = LangError::InvalidPort;
                    error.lineNumber = block->getLine();
                    error.errorTokens.push_back(blockType.toStdString());
                    error.errorTokens.push_back(portName.toStdString());
                    m_errors << error;
                } else {
                    AST *portValue = port->getValue();
                    if (portValue->getNodeType() == AST::List) {
                        ListNode *list = static_cast<ListNode *>(portValue);
                        for (int i = 0; i < list->size(); i++) {
                            QString portTypeName = getPortTypeName(resolveNodeOutType(list->getChildren().at(i), scope, m_tree));
                            if (!m_platform.isValidPortType(blockType, portName, portTypeName)) {
                                LangError error;
                                error.type = LangError::InvalidPortType;
                                error.lineNumber = block->getLine();
                                error.errorTokens.push_back(blockType.toStdString());
                                error.errorTokens.push_back(portName.toStdString());
                                error.errorTokens.push_back(portTypeName.toStdString());
                                m_errors << error;
                            }
                        }
                    } else {
                        QString portTypeName = getPortTypeName(resolveNodeOutType(portValue, scope, m_tree));

                        if (!m_platform.isValidPortType(blockType, portName, portTypeName)) {
                            LangError error;
                            error.type = LangError::InvalidPortType;
                            error.lineNumber = block->getLine();
                            error.errorTokens.push_back(blockType.toStdString());
                            error.errorTokens.push_back(portName.toStdString());
                            error.errorTokens.push_back(portTypeName.toStdString());
                            m_errors << error;
                        }
                    }
                }
            }
        } else if (typeBlock = m_library.findTypeInLibrary(blockType)) {
            foreach(PropertyNode *properties, block->getProperties()) {
//                foreach()
            }

        } else { // Not platform or library type, then error
            LangError error;
            error.type = LangError::UnknownType;
            error.lineNumber = block->getLine();
            error.errorTokens.push_back(block->getObjectType());
            m_errors << error;
        }

    }

    QVector<AST *> children = QVector<AST *>::fromStdVector(node->getChildren());
    foreach(AST *node, children) {
        validateTypes(node, scope);
    }
}

void CodeValidator::validateBundleIndeces(AST *node, QVector<AST *> scope)
{
    if (node->getNodeType() == AST::Bundle) {
        BundleNode *bundle = static_cast<BundleNode *>(node);
        PortType type = resolveNodeOutType(bundle->index(), scope, m_tree);
        if(type != ConstInt && type != ControlInt) {
            LangError error;
            error.type = LangError::IndexMustBeInteger;
            error.lineNumber = bundle->getLine();
            error.errorTokens.push_back(bundle->getName());
            error.errorTokens.push_back(getPortTypeName(type).toStdString());
            m_errors << error;
        }
    }
    QVector<AST *> children = QVector<AST *>::fromStdVector(node->getChildren());
    foreach(AST *node, children) {
        validateBundleIndeces(node, children);
    }
}

void CodeValidator::validateBundleSizes(AST *node, QVector<AST *> scope)
{
    if (node->getNodeType() == AST::BlockBundle) {
        QList<LangError> errors;
        BlockNode *block = static_cast<BlockNode *>(node);
        int size = getBlockDeclaredSize(block, scope, m_tree, errors);
        int datasize = getBlockDataSize(block, scope, errors);
        if(size != datasize && datasize > 1) {
            LangError error;
            error.type = LangError::BundleSizeMismatch;
            error.lineNumber = node->getLine();
            error.errorTokens.push_back(block->getBundle()->getName());
            error.errorTokens.push_back(QString::number(size).toStdString());
            error.errorTokens.push_back(QString::number(datasize).toStdString());
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

void CodeValidator::validateSymbolUniqueness(AST *node, QVector<AST *> scope)
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
            if (!nodeName.isEmpty() && !siblingName.isEmpty() && nodeName == siblingName) {
                LangError error;
                error.type = LangError::DuplicateSymbol;
                error.lineNumber = sibling->getLine();
                error.errorTokens.push_back(nodeName.toStdString());
                error.errorTokens.push_back(QString::number(node->getLine()).toStdString());
                m_errors << error;
            }
        }
    }

    QVector<AST *> children = QVector<AST *>::fromStdVector(node->getChildren());
    foreach(AST *node, children) {
        validateSymbolUniqueness(node, children);
    }
}

void CodeValidator::validateListTypeConsistency(AST *node, QVector<AST *> scope)
{
    if (node->getNodeType() == AST::List) {
        PortType type = resolveListType(static_cast<ListNode *>(node), scope, m_tree);
        if (type == Invalid) {
            LangError error;
            error.type = LangError::InconsistentList;
            error.lineNumber = node->getLine();
            // TODO: provide more information on inconsistent list
//            error.errorTokens <<
            m_errors << error;
        }
    }

    QVector<AST *> children = QVector<AST *>::fromStdVector(node->getChildren());
    foreach(AST *node, children) {
        validateListTypeConsistency(node, children);
    }
}

void CodeValidator::validateStreamSizes(AST *tree, QVector<AST *> scope)
{
    QVector<AST *> children = QVector<AST *>::fromStdVector(tree->getChildren());
    foreach(AST *node, children) {
        if(node->getNodeType() == AST:: Stream) {
            StreamNode *stream = static_cast<StreamNode *>(node);
            validateStreamInputSize(stream, scope, m_errors);
        }
    }
}

bool errorLineIsLower(const LangError &err1, const LangError &err2)
{
    return err1.lineNumber < err2.lineNumber;
}

void CodeValidator::sortErrors()
{
    std::sort(m_errors.begin(), m_errors.end(), errorLineIsLower);
}

void CodeValidator::validateStreamInputSize(StreamNode *stream, QVector<AST *> scope, QList<LangError> &errors)
{
    AST *left = stream->getLeft();
    AST *right = stream->getRight();

    int leftOutSize = getNodeNumOutputs(left, m_platform, scope, m_tree, errors);
    int rightInSize = getNodeNumInputs(right, m_platform, scope, m_tree, errors);

    if (leftOutSize != rightInSize
            && ((int) (rightInSize/ (double) leftOutSize)) != (rightInSize/ (double) leftOutSize) ) {
        LangError error;
        error.type = LangError::StreamMemberSizeMismatch;
        error.lineNumber = right->getLine();
        error.errorTokens.push_back(QString::number(leftOutSize).toStdString());
        error.errorTokens.push_back(getNodeText(left).toStdString());
        error.errorTokens.push_back(QString::number(rightInSize).toStdString());
        errors << error;
    }
    if (right->getNodeType() == AST::Stream) {
        validateStreamInputSize(static_cast<StreamNode *>(right), scope, errors);
    }
}

int CodeValidator::getBlockDeclaredSize(BlockNode *block, QVector<AST *> scope, AST *tree, QList<LangError> &errors)
{
    int size = -1;
    Q_ASSERT(block->getNodeType() == AST::BlockBundle);
    BundleNode *bundle = static_cast<BundleNode *>(block->getBundle());
    if (bundle->getNodeType() == AST::Bundle) {
        size = 0;
        ListNode *indexList = bundle->index();
        vector<AST *>indexExps = indexList->getChildren();
        foreach(AST* exp, indexExps) {
            if (exp->getNodeType() == AST::Range) {
                RangeNode *range = static_cast<RangeNode *>(exp);
                AST *start = range->startIndex();
                int startIndex, endIndex;
                PortType type = CodeValidator::resolveNodeOutType(start, scope, tree);
                if (type == ConstInt) {
                    startIndex = CodeValidator::evaluateConstInteger(start, scope, tree, errors);
                } else {
                    // TODO: Do something if not integer
                    continue;
                }
                AST *end = range->startIndex();
                type = CodeValidator::resolveNodeOutType(end, scope, tree);
                if (type == ConstInt) {
                    endIndex = CodeValidator::evaluateConstInteger(end, scope, tree, errors);
                } else {
                    // TODO: Do something if not integer
                    continue;
                }
                if (end > start) {
                    size += endIndex + startIndex + 1;
                }

            } else {
                PortType type = CodeValidator::resolveNodeOutType(exp, scope, tree);
                if (type == ConstInt) {
                    size += CodeValidator::evaluateConstInteger(exp, scope, tree, errors);
                }
                // TODO: Something should be done if index isn't integer
            }
        }
    }
    return size;
}

int CodeValidator::getBlockDataSize(BlockNode *block, QVector<AST *> scope, QList<LangError> &errors)
{
    QVector<PropertyNode *> ports = QVector<PropertyNode *>::fromStdVector(block->getProperties());
    if (ports.size() == 0) {
        return 0;
    }
    int size = getNodeNumOutputs(ports.at(0)->getValue(), m_platform, scope, m_tree, errors);
    foreach(PropertyNode *port, ports) {
        AST *value = port->getValue();
        int newSize = getNodeNumOutputs(value, m_platform, scope, m_tree, errors);
        if (size != newSize) {
            if (size == 1) {
                size = newSize;
            } if (newSize == 1) {
                // just ignore
            } else {
                size = -1;
            }
        }
    }
    return size;
}

int CodeValidator::getBundleSize(BundleNode *bundle, QVector<AST *> scope, AST * tree, QList<LangError> &errors)
{
    ListNode *indexList = bundle->index();
    int size = 0;
    vector<AST *> listExprs = indexList->getChildren();
    PortType type;
    foreach(AST *expr, listExprs) {
        switch (expr->getNodeType()) {
        case AST::Int:
//            size += evaluateConstInteger(expr, scope, tree, errors);
            size += 1;
            break;
        case AST::Range:
            size += evaluateConstInteger(static_cast<RangeNode *>(expr)->endIndex(), scope, tree, errors)
                    - evaluateConstInteger(static_cast<RangeNode *>(expr)->startIndex(), scope, tree, errors) + 1;
            break;
        case AST::Expression:
            type = resolveExpressionType(static_cast<ExpressionNode *>(expr), scope, tree);
            if (type == ConstInt) {
//                size += evaluateConstInteger(expr, scope, tree, errors);
                size += 1;
            }
            break;
        default:
            break;
        }
    }

    return size;
}

QString CodeValidator::getNodeText(AST *node)
{
    QString outText;
    if(node->getNodeType() == AST::Name) {
        outText = QString::fromStdString(static_cast<NameNode *>(node)->getName());
    } else if(node->getNodeType() == AST::Bundle) {
        outText = QString::fromStdString(static_cast<BundleNode *>(node)->getName());
    } else if(node->getNodeType() == AST::Function) {
         outText = QString::fromStdString(static_cast<FunctionNode *>(node)->getName());
     } else if(node->getNodeType() == AST::List) {
        outText = "[ List ]";
    } else {
        qFatal("Unsupported type in getNodeText(AST *node)");
    }
    return outText;
}

int CodeValidator::getLargestPropertySize(vector<PropertyNode *> &properties, QVector<AST *> scope, AST *tree, QList<LangError> &errors)
{
    int maxSize = 1;
    foreach(PropertyNode *property, properties) {
        AST *value = property->getValue();
        if (value->getNodeType() == AST::Name) {
            NameNode *name = static_cast<NameNode *>(value);
            BlockNode *block = findDeclaration(QString::fromStdString(name->getName()), scope, tree);
            if (block) {
                if (block->getNodeType() == AST::Block) {
                    if (maxSize < 1) {
                        maxSize = 1;
                    }
                } else if (block->getNodeType() == AST::BlockBundle) {
                    AST *index = block->getBundle()->index();
                    int newSize = evaluateConstInteger(index, QVector<AST *>(), tree, errors);
                    if (newSize > maxSize) {
                        maxSize = newSize;
                    }
                }
            }
        } else if (value->getNodeType() == AST::List) {
            ListNode *list = static_cast<ListNode *>(value);
            int newSize = list->getChildren().size();
            if (newSize > maxSize) {
                maxSize = newSize;
            }

        } else if (value->getNodeType() == AST::Int
                   || value->getNodeType() == AST::Real
                   || value->getNodeType() == AST::String
                   ) {
            if (maxSize < 1) {
                maxSize = 1;
            }
        }
    }
    return maxSize;
}

int CodeValidator::getNodeNumOutputs(AST *node, StreamPlatform &platform, QVector<AST *> &scope, AST *tree, QList<LangError> &errors)
{
    Q_ASSERT(node->getNodeType() != AST::Stream); // Stream nodes should not be on the left...
    if (node->getNodeType() == AST::List) {
        return node->getChildren().size();
    } else if (node->getNodeType() == AST::Bundle) {
        return getBundleSize(static_cast<BundleNode *>(node), scope, tree, errors);
    } else if (node->getNodeType() == AST::Int
               || node->getNodeType() == AST::Real
               || node->getNodeType() == AST::String) {
        return 1;
    } else if (node->getNodeType() == AST::Expression) {
        // TODO: evaluate
    } else if (node->getNodeType() == AST::Name) {
        NameNode *name = static_cast<NameNode *>(node);
        BlockNode *block = findDeclaration(QString::fromStdString(name->getName()), scope, tree);
        if (block) {
            if (block->getNodeType() == AST::BlockBundle) {
                return getBlockDeclaredSize(block, scope, tree, errors);
            } else if (block->getNodeType() == AST::Block ) {
                return 1;
            }
        } else {
            return -1; // Not a bundle or block
        }
    } else if (node->getNodeType() == AST::Function) {
        FunctionNode *func = static_cast<FunctionNode *>(node);
        PlatformFunction platformFunc = platform.getFunction(QString::fromStdString(func->getName()));
        return platformFunc.numOutputs();
    }
    return -1;
}

int CodeValidator::getNodeNumInputs(AST *node, StreamPlatform &platform, QVector<AST *> &scope, AST *tree, QList<LangError> &errors)
{
    if (node->getNodeType() == AST::Function) {
        FunctionNode *func = static_cast<FunctionNode *>(node);
        PlatformFunction platformFunc = platform.getFunction(QString::fromStdString(func->getName()));
        return platformFunc.numInputs();
    } else if (node->getNodeType() == AST::Stream) {
        StreamNode *stream = static_cast<StreamNode *>(node);
        AST *left = stream->getLeft();
//        AST *right = stream->getRight();
        int leftSize = CodeValidator::getNodeNumInputs(left, platform, scope, tree, errors);
        return leftSize;
    } else if (node->getNodeType() == AST::Name) {
        NameNode *name = static_cast<NameNode *>(node);
        BlockNode *block = findDeclaration(QString::fromStdString(name->getName()), scope, tree);
        if (block && block->getNodeType() == AST::BlockBundle) {
            return getBlockDeclaredSize(block, scope, tree, errors);
        } else {
            return 1;
        }
    } else if (node->getNodeType() == AST::Bundle) {
        return getBundleSize(static_cast<BundleNode *>(node), scope, tree, errors);
    } else {
        return 1;
//        return CodeValidator::getNodeNumOutputs(node, platform, scope, tree, errors);
    }
    return -1;
}

BlockNode *CodeValidator::findDeclaration(QString objectName, QVector<AST *> scope, AST *tree)
{
    QVector<AST *> globalAndLocal;
    globalAndLocal << scope << QVector<AST *>::fromStdVector(tree->getChildren());
    foreach(AST *node, globalAndLocal) {
        if (node->getNodeType() == AST::BlockBundle) {
            BlockNode *block = static_cast<BlockNode *>(node);
            BundleNode *bundle = block->getBundle();
            QString name = QString::fromStdString(bundle->getName());
            if (name == objectName) {
                return block;
            }
        } else if (node->getNodeType() == AST::Block) {
            BlockNode *block = static_cast<BlockNode *>(node);
            QString name = QString::fromStdString(block->getName());
            if (name == objectName) {
                return block;
            }
        }
    }
    return NULL;
}

CodeValidator::PortType CodeValidator::resolveBundleType(BundleNode *bundle, QVector<AST *>scope, AST *tree)
{
    QString bundleName = QString::fromStdString(bundle->getName());
    BlockNode *declaration = findDeclaration(bundleName, scope, tree);
    if(declaration) {
        if (declaration->getObjectType() == "constant") {
            vector<PropertyNode *> properties = declaration->getProperties();
            foreach(PropertyNode *property, properties)  {
                if(property->getName() == "value") {
                    return resolveNodeOutType(property->getValue(), scope, tree);
                }
            }
        } else {
//            return QString::fromStdString(declaration->getObjectType());
        }
    }
    return None;
}

CodeValidator::PortType CodeValidator::resolveNameType(NameNode *name, QVector<AST *>scope, AST *tree)
{
    QString nodeName = QString::fromStdString(name->getName());
    BlockNode *declaration = findDeclaration(nodeName, scope, tree);
    if(declaration) {
        if (declaration->getObjectType() == "constant") {
            vector<PropertyNode *> properties = declaration->getProperties();
            foreach(PropertyNode *property, properties)  {
                if(property->getName() == "value") {
                    return resolveNodeOutType(property->getValue(), scope, tree);
                }
            }
        } else {
//            return QString::fromStdString(declaration->getObjectType());
        }
    }
    return None;
}

CodeValidator::PortType CodeValidator::resolveNodeOutType(AST *node, QVector<AST *> scope, AST *tree)
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
        return resolveListType(static_cast<ListNode *>(node), scope, tree);
    }  else if(node->getNodeType() == AST::Bundle) {
        return resolveBundleType(static_cast<BundleNode *>(node), scope, tree);
    } else if (node->getNodeType() == AST::Expression) {
        return resolveExpressionType(static_cast<ExpressionNode *>(node), scope, tree);
    } else if (node->getNodeType() == AST::Name) {
        return resolveNameType(static_cast<NameNode *>(node), scope, tree);
    } else if (node->getNodeType() == AST::Range) {
        return resolveRangeType(static_cast<RangeNode *>(node), scope, tree);
    }
    return None;
}

CodeValidator::PortType CodeValidator::resolveListType(ListNode *listnode, QVector<AST *> scope, AST *tree)
{
    QVector<AST *> members = QVector<AST *>::fromStdVector(listnode->getChildren());
    if (members.isEmpty()) {
        return None;
    }
    AST *firstMember = members.takeFirst();
    PortType type = resolveNodeOutType(firstMember, scope, tree);

    foreach(AST *member, members) {
        PortType nextPortType = resolveNodeOutType(member, scope, tree);
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

CodeValidator::PortType CodeValidator::resolveExpressionType(ExpressionNode *exprnode, QVector<AST *> scope, AST *tree)
{
    if (!exprnode->isUnary()) {
        AST *left = exprnode->getLeft();
        AST *right = exprnode->getRight();
        PortType leftType = resolveNodeOutType(left, scope, tree);
        PortType rightType = resolveNodeOutType(right, scope, tree);
        if (leftType == rightType) {
            return leftType;
        }
        // TODO implement toleraces between ints and reals

    } else {
        // TODO implement for unary
    }
    return None;
}

CodeValidator::PortType CodeValidator::resolveRangeType(RangeNode *rangenode, QVector<AST *> scope, AST *tree)
{
    PortType leftType = resolveNodeOutType(rangenode->startIndex(), scope, tree);
    PortType rightType = resolveNodeOutType(rangenode->endIndex(), scope, tree);
    if (leftType == rightType) {
        return leftType;
    }
    return None;
}

int CodeValidator::evaluateConstInteger(AST *node, QVector<AST *> scope, AST *tree, QList<LangError> &errors)
{
    int result = 0;
    if (node->getNodeType() == AST::Int) {
        return static_cast<ValueNode *>(node)->getIntValue();
    } else if (node->getNodeType() == AST::Bundle) {
        BundleNode *bundle = static_cast<BundleNode *>(node);
        ListNode *indexList = bundle->index();
        if (indexList->size() == 1) {
            int index = evaluateConstInteger(indexList->getChildren().at(0), scope, tree, errors);

            QString bundleName = QString::fromStdString(bundle->getName());
            BlockNode *declaration = findDeclaration(bundleName, scope, tree);
            if(declaration && declaration->getNodeType() == AST::BlockBundle) {
                AST *member = getMemberfromBlockBundle(declaration, index, errors);
                return evaluateConstInteger(member, scope, tree, errors);
            }
        }
        LangError error;
        error.type = LangError::InvalidType;
        error.lineNumber = bundle->getLine();
        error.errorTokens.push_back(bundle->getName());
        errors << error;
    } else if (node->getNodeType() == AST::Expression) {
        // TODO: check expression out
    } else {
        LangError error;
        error.type = LangError::InvalidType;
        error.lineNumber = node->getLine();
        error.errorTokens.push_back(getPortTypeName(resolveNodeOutType(node, scope, tree)).toStdString());
        errors << error;
    }
    return result;
}

double CodeValidator::evaluateConstReal(AST *node, QVector<AST *> scope, AST *tree, QList<LangError> &errors)
{
    double result = 0;
    if (node->getNodeType() == AST::Real) {
        return static_cast<ValueNode *>(node)->getRealValue();
    } else if (node->getNodeType() == AST::Int) {
        return static_cast<ValueNode *>(node)->getIntValue();
    } else if (node->getNodeType() == AST::Bundle) {
        BundleNode *bundle = static_cast<BundleNode *>(node);
        QString bundleName = QString::fromStdString(bundle->getName());
        BlockNode *declaration = findDeclaration(bundleName, scope, tree);
        int index = evaluateConstInteger(bundle->index(), scope, tree, errors);
        if(declaration && declaration->getNodeType() == AST::BlockBundle) {
            AST *member = getMemberfromBlockBundle(declaration, index, errors);
            return evaluateConstReal(member, scope, tree, errors);
        }
    } else if (node->getNodeType() == AST::Name) {
        NameNode *nameNode = static_cast<NameNode *>(node);
        QString name = QString::fromStdString(nameNode->getName());
        BlockNode *declaration = findDeclaration(name, scope, tree);
        if (!declaration) {
            LangError error;
            error.type = LangError::UndeclaredSymbol;
            error.lineNumber = node->getLine();
            error.errorTokens.push_back(name.toStdString());
            errors << error;
        }
        if(declaration && declaration->getNodeType() == AST::Block) {
            AST *value = getValueFromConstBlock(declaration);
            if(value->getNodeType() == AST::Int || value->getNodeType() == AST::Real) {
                return static_cast<ValueNode *>(value)->toReal();
            } else {
                // Do something?
            }
        }
    } else {
        LangError error;
        error.type = LangError::InvalidType;
        error.lineNumber = node->getLine();
        error.errorTokens.push_back(getPortTypeName(resolveNodeOutType(node, scope, tree)).toStdString());
        errors << error;
    }
    return result;
}

AST *CodeValidator::getMemberfromBlockBundle(BlockNode *block, int index, QList<LangError> &errors)
{
    AST *out = NULL;
    if (block->getObjectType() == "constant") {
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

AST *CodeValidator::getValueFromConstBlock(BlockNode *block)
{
    AST *out = NULL;
    if (block->getObjectType() == "constant") {
        QVector<PropertyNode *> ports = QVector<PropertyNode *>::fromStdVector(block->getProperties());
        foreach(PropertyNode *port, ports) {
            if(port->getName() == "value") {
                return port->getValue();
            }
        }
    } else {
        // Should something else be done?
    }
    return out;
}

AST *CodeValidator::getMemberFromList(ListNode *node, int index, QList<LangError> &errors)
{
    if (index < 1 || index > (int) node->getChildren().size()) {
        LangError error;
        error.type = LangError::ArrayIndexOutOfRange;
        error.lineNumber = node->getLine();
        error.errorTokens.push_back(QString::number(index).toStdString());
        errors << error;
        return NULL;
    }
    return node->getChildren()[index - 1];
}

int CodeValidator::numParallelStreams(StreamNode *stream, StreamPlatform &platform, QVector<AST *> &scope, AST *tree, QList<LangError> &errors)
{
    AST *left = stream->getLeft();
    AST *right = stream->getRight();
    int numParallel = 0;

    int leftSize;
    int rightSize;

    if (left->getNodeType() == AST::Name) {
        leftSize = getNodeSize(left, tree);
    } else if (left->getNodeType() == AST::List) {
        leftSize = getNodeSize(left, tree);
    } else {
        leftSize = getNodeNumOutputs(left, platform, scope, tree, errors);
    }
    if (right->getNodeType() == AST::Name
            || right->getNodeType() == AST::List) {
        rightSize = getNodeSize(right, tree);
    } else if (right->getNodeType() == AST::Function) {
        int functionNodeSize = getNodeSize(right, tree);
        if (functionNodeSize == 1) {
            rightSize = leftSize;
        }
    } else if (right->getNodeType() == AST::Stream) {
        StreamNode *rightStream = static_cast<StreamNode *>(right);
        numParallel = numParallelStreams(rightStream, platform, scope, tree, errors);

        AST *firstMember = rightStream->getLeft();
        if (firstMember->getNodeType() == AST::Name
                || firstMember->getNodeType() == AST::List) {
            rightSize = getNodeSize(firstMember, tree);
        } else {
            rightSize = getNodeNumInputs(firstMember, platform, scope, tree, errors);
        }
        if (firstMember->getNodeType() == AST::Function) {
            int functionNodeSize = getNodeSize(firstMember, tree);
            if (functionNodeSize == 1) {
                rightSize = getNodeNumInputs(firstMember, platform, scope, tree, errors);;
            } else {

            }
        }
    } else {
        rightSize = getNodeNumInputs(right, platform, scope, tree, errors);
    }
    int thisParallel = -1;

    if (leftSize == rightSize ||
            (rightSize/(float)leftSize) == (int)(rightSize/(float)leftSize)){
        if (leftSize == 0) {
          thisParallel = -1;
        } else {
          thisParallel = rightSize/leftSize;
        }
    }
    if (leftSize == 1) {
        thisParallel = rightSize;
    } else if (rightSize == 1) {
        thisParallel = leftSize;
    }
    if (thisParallel != numParallel  && numParallel > 0) {
        if (rightSize == 1)
        numParallel = -1;
    } else {
        numParallel = thisParallel;
    }

    return numParallel;
}

int CodeValidator::getNodeSize(AST *node, AST *tree)
{
    int size = 1;
    if (node->getNodeType() == AST::Bundle) {
        BundleNode *bundle = static_cast<BundleNode *>(node);
        QList<LangError> errors;
        size = getBundleSize(bundle, QVector<AST *>::fromStdVector(tree->getChildren()),
                             tree, errors);
        if (errors.size() > 0) {
            return -1;
        }
        Q_ASSERT(errors.size() ==0);
        return size;
    } else if (node->getNodeType() == AST::Expression) {

        qFatal("implement Expression parsing in getNodeSize");
    } else if (node->getNodeType() == AST::Name) {
        NameNode *nameNode = static_cast<NameNode *>(node);
        BlockNode *block = findDeclaration(QString::fromStdString(nameNode->getName()), QVector<AST *>(), tree);
        if (!block) {
            size = -1; // Block not declared
        } else if (block->getNodeType() == AST::BlockBundle) {
            QList<LangError> errors;
            size = getBlockDeclaredSize(block, QVector<AST *>(), tree, errors);
        } else  if (block->getNodeType() == AST::Block) {
            size = 1;
        } else {
            Q_ASSERT(0 == 1);
        }
    } else if (node->getNodeType() == AST::Function) {
        vector<PropertyNode *> properties = static_cast<FunctionNode *>(node)->getProperties();
        QList<LangError> errors;
        size = getLargestPropertySize(properties, QVector<AST *>(), tree, errors);
    } else if (node->getNodeType() == AST::List) {
        size = node->getChildren().size();
    } else if (node->getNodeType() == AST::Stream) {
        StreamNode *st = static_cast<StreamNode *>(node);
        size = getNodeSize(st->getLeft(), tree);
    }

    return size;
}

QString CodeValidator::getPortTypeName(CodeValidator::PortType type)
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

