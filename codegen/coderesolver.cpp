#include <cassert>
#include <QDebug>

#include "coderesolver.h"
#include "codevalidator.h"

CodeResolver::CodeResolver(StreamPlatform &platform, AST *tree) :
    m_platform(platform), m_tree(tree)
{

}

CodeResolver::~CodeResolver()
{

}

void CodeResolver::process()
{
    expandBuiltinObjects();
    resolveStreamSymbols();
    resolveRates();
    resolveConstants();
    expandStreamBundles();
    expandTypeBundles();
}

void CodeResolver::resolveRates()
{
    vector<AST *> nodes = m_tree->getChildren();
    for(unsigned int i = 0; i < nodes.size(); i++) {
        AST* node = nodes.at(i);
        if (node->getNodeType() == AST::Stream) {
            resolveStreamRates(static_cast<StreamNode *>(node));
        }
    }
}

void CodeResolver::resolveStreamRates(StreamNode *stream)
{
    AST *left = stream->getLeft();
    AST *right = stream->getRight();
    double rate = getNodeRate(left, QVector<AST *>(), m_tree);
    double rightRate = -1;
    if (right->getNodeType() == AST::Stream) {
        resolveStreamRates(static_cast<StreamNode *>(right));
        rightRate = static_cast<StreamNode *>(right)->getLeft()->getRate();
    } else {
        rightRate = getNodeRate(right, QVector<AST *>(), m_tree); // TODO need to propagate rate across streams e.g. : NoRate >> NoRate2; NoRate2 >> WithRate;
    }
    if (rate < 0 && rightRate >= 0) {
        left->setRate(rightRate);
        rate = rightRate;
    }
    stream->setRate(rate);
}

void CodeResolver::fillDefaultProperties()
{
    vector<AST *> nodes = m_tree->getChildren();
    for(unsigned int i = 0; i < nodes.size(); i++) {
        AST* node = nodes.at(i);
        if (node->getNodeType() == AST::Block || node->getNodeType() == AST::Block) {
            BlockNode *block = static_cast<BlockNode *>(node);
            vector<PropertyNode *> blockProperties = block->getProperties();
            QList<Property> typeProperties = m_platform.getPortsForType(QString::fromStdString(block->getObjectType()));
            foreach(Property typeProperty, typeProperties) {
                bool propertySet = false;
                foreach(PropertyNode *blockProperty, blockProperties) {
                    if (blockProperty->getName() == typeProperty.name.toStdString()) {
                        propertySet = true;
                        break;
                    }
                }
                if (!propertySet) {
                    ValueNode *value = NULL;
                    if (typeProperty.defaultValue.type() == QVariant::Int) {
                        value = new ValueNode(typeProperty.defaultValue.toInt(), -1);
                    } else if (typeProperty.defaultValue.type() == QVariant::Double) {
                        value = new ValueNode(typeProperty.defaultValue.toDouble(), -1);
                    } else if (typeProperty.defaultValue.type() == QVariant::String) {
                        value = new ValueNode(typeProperty.defaultValue.toString().toStdString(), -1);
                    } else if (typeProperty.defaultValue.type() == QVariant::Map) {
                        qDebug() << "Property Map";
                    }
                    Q_ASSERT(value);
                    PropertyNode *newProperty = new PropertyNode(typeProperty.name.toStdString(), value, -1);
                    block->addProperty(newProperty);
                }
            }
        }
    }
}

double CodeResolver::getNodeRate(AST *node, QVector<AST *> scope, AST *tree)
{
    if (node->getRate() != -1) {
        return node->getRate();
    }
    if (node->getNodeType() == AST::Name) {
        NameNode *name = static_cast<NameNode *>(node);
        BlockNode* declaration =  CodeValidator::findDeclaration(QString::fromStdString(name->getName()), scope, tree);
        if (!declaration) {
            return -1;
        }
        vector<PropertyNode *> properties = declaration->getProperties();
        foreach(PropertyNode *property, properties) {
            if (property->getName() == "rate") { // FIXME this assumes that a property named rate always applies to stream rate...
                AST *propertyValue = property->getValue();
                QList<LangError> errors;
                int rate = -1;
                if (propertyValue->getNodeType() == AST::Int) {
                    rate = CodeValidator::evaluateConstInteger(propertyValue, QVector<AST *>(), tree, errors);
                } else if (propertyValue->getNodeType() == AST::Real) {
                    rate = CodeValidator::evaluateConstReal(propertyValue, QVector<AST *>(), tree, errors);
                } else if (propertyValue->getNodeType() == AST::Name
                           || propertyValue->getNodeType() == AST::Bundle) {
                    rate = CodeValidator::evaluateConstReal(propertyValue, QVector<AST *>(), tree, errors);
                }
                if (errors.size() == 0) {
                    name->setRate(rate);
                    return rate;
                }
            }
        }
    } else if (node->getNodeType() == AST::Bundle) {
        BundleNode *bundle = static_cast<BundleNode *>(node);
        BlockNode* declaration =  CodeValidator::findDeclaration(QString::fromStdString(bundle->getName()), scope, tree);
        if (!declaration) {
            return -1;
        }
        vector<PropertyNode *> properties = declaration->getProperties();
        foreach(PropertyNode *property, properties) {
            if (property->getName() == "rate") { // FIXME this assumes that a property named rate always applies to stream rate...
                AST *value = property->getValue();
                QList<LangError> errors;
                double rate;
                if (value->getNodeType() == AST::Int) {
                    rate = CodeValidator::evaluateConstInteger(value, QVector<AST *>(), tree, errors); // TODO what to set here as scope?
                } else if (value->getNodeType() == AST::Real) {
                    rate = CodeValidator::evaluateConstReal(value, QVector<AST *>(), tree, errors); // TODO what to set here as scope?
                } else if (value->getNodeType() == AST::Name) {
                    BlockNode* valueDeclaration =  CodeValidator::findDeclaration(
                                QString::fromStdString(static_cast<NameNode *>(value)->getName()), scope, tree);
                    if (valueDeclaration->getObjectType() == "constant") {
                        vector<PropertyNode *> valueProperties = valueDeclaration->getProperties();
                        foreach(PropertyNode *property, valueProperties) {
                            if (property->getName() == "value") {
                                ValueNode *propertyValue = static_cast<ValueNode *>(property->getValue());
                                if (propertyValue->getNodeType() == AST::Int) {
                                    rate = CodeValidator::evaluateConstInteger(propertyValue, QVector<AST *>(), tree, errors);
                                } else if (propertyValue->getNodeType() ==AST::Real) {
                                    rate = CodeValidator::evaluateConstReal(propertyValue, QVector<AST *>(), tree, errors);
                                }
                            }
                        }
                    }
                }
                if (errors.size() == 0) {
                    bundle->setRate(rate);
                    return rate;
                }
            }
        }

    }
    return -1;
}

void CodeResolver::expandBuiltinObjects()
{
    QList<PlatformObject> objects = m_platform.getBuiltinObjects();
    foreach(PlatformObject object, objects) {
        int size = object.getSize();
        QVariantMap properties = object.getProperties();
        AST *propertiesTree = new AST;
        foreach(QString key, properties.keys()) {
            QVariant value = properties[key];
            if (value.type() == QVariant::Int) {
                ValueNode *valueNode = new ValueNode(value.toInt(), -1);
                propertiesTree->addChild(new PropertyNode(key.toStdString(), valueNode, -1));
            } else if (value.type() == QVariant::Double)  {
                ValueNode *valueNode = new ValueNode(value.toDouble(), -1);
                propertiesTree->addChild(new PropertyNode(key.toStdString(), valueNode, -1));
            } else if (value.type() == QVariant::String)  {
                ValueNode *valueNode = new ValueNode(value.toString().toStdString(), -1);
                propertiesTree->addChild(new PropertyNode(key.toStdString(), valueNode, -1));
            } else if (value.type() == QVariant::Map)  {
                QString name = value.toMap()["name"].toString();
                NameNode *nameNode = new NameNode(name.toStdString(), -1);
                propertiesTree->addChild(new PropertyNode(key.toStdString(), nameNode, -1));
            }
            else {
                qFatal("Unsupported key type for builtin object.");
            }
        }
        if (size >= 0) {
            BundleNode *bundle = new BundleNode(object.getName().toStdString(),
                                                new ValueNode(object.getSize(), -1), -1);

            BlockNode *child = new BlockNode(bundle, object.getType().toStdString(),
                                  propertiesTree, -1);
            m_tree->addChild(child);
        } else { //Not a bundle
            BlockNode *child = new BlockNode(object.getName().toStdString(), object.getType().toStdString(),
                                  propertiesTree, -1);
            m_tree->addChild(child);
        }
        delete propertiesTree;
    }
}

int CodeResolver::createSignalDeclaration(QString name, StreamNode *parentStream, AST *tree)
{
    BlockNode *newBlock;
    double nameRate = -1;
    int size = CodeValidator::largestBundleSize(parentStream, tree);
    if (size <= 0) { // None of the elements in the stream have size
        size = 1;
    }
    if (size == 1) {
        newBlock = new BlockNode(name.toStdString(), "signal", NULL, -1);
    } else if (size > 1) {
        BundleNode *bundle = new BundleNode(name.toStdString(),
                                            new ValueNode(size, -1), -1);
        newBlock = new BlockNode(bundle, "signal", NULL, -1);
    }
    QVariant defaultRate = m_platform.getDefaultPortValueForType("signal", "rate");
    if (defaultRate.type() != QVariant::String) {
        ValueNode *value = new ValueNode(defaultRate.toDouble(), -1);
        newBlock->addProperty(new PropertyNode("rate", value, -1));
        nameRate = defaultRate.toDouble();
    } else {
        BlockNode *block = CodeValidator::findDeclaration(defaultRate.toString(),
                                                          QVector<AST *>(), tree);
        if (block && block->getObjectType() == "constant") {
            AST *propValue = block->getPropertyValue("value");
            QList<LangError> errors;
            double rate = CodeValidator::evaluateConstReal(propValue, QVector<AST *>(),
                                                           tree, errors);
            if (errors.size() == 0) {
                ValueNode *valueNode = new ValueNode(rate, -1);
                PropertyNode *propNode = new PropertyNode("rate", valueNode, -1);
                newBlock->addProperty(propNode);
            }
            nameRate = rate;
        }
    }
    tree->addChild(newBlock);
    return nameRate;
}

void CodeResolver::declareUnknownStreamSymbols(StreamNode *stream, AST * tree)
{
    AST *left = stream->getLeft();

    if (left->getNodeType() == AST::Name) {
        NameNode *name = static_cast<NameNode *>(left);
        BlockNode *block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()), QVector<AST *>(), tree);
        if (!block) {
            int rate = createSignalDeclaration(QString::fromStdString(name->getName()), stream, tree);
            name->setRate(rate);
        }
    }

    AST *right = stream->getRight();
    if (right->getNodeType() == AST::Stream) {
        declareUnknownStreamSymbols(static_cast<StreamNode *>(right), tree);
    } else if (right->getNodeType() == AST::Name) {
        NameNode *name = static_cast<NameNode *>(right);
        BlockNode *block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()), QVector<AST *>(), tree);
        if (!block) {
            int rate = createSignalDeclaration(QString::fromStdString(name->getName()), stream, tree);
            name->setRate(rate);
        }
    }
}

void CodeResolver::resolveStreamSymbols()
{
    QVector<AST *> children = QVector<AST *>::fromStdVector(m_tree->getChildren());
    foreach(AST *node, children) {
        if(node->getNodeType() == AST:: Stream) {
            StreamNode *stream = static_cast<StreamNode *>(node);
            declareUnknownStreamSymbols(stream, m_tree);
        }
    }
}

void CodeResolver::resolveConstants()
{
    QVector<AST *> children = QVector<AST *>::fromStdVector(m_tree->getChildren());
    foreach(AST *node, children) {
        resolveConstantsInNode(node, children);
    }
}

void CodeResolver::expandStreamBundles()
{
    vector<AST *> nodes = m_tree->getChildren();
    vector<AST *> newNodes;
    for(unsigned int i = 0; i < nodes.size(); i++) {
        AST* node = nodes.at(i);
        if (node->getNodeType() == AST::Stream) {
            StreamNode *oldNode = static_cast<StreamNode *>(node);
            QVector<StreamNode *> newStreams = expandBundleStream(oldNode);
            oldNode->deleteChildren();
            delete oldNode;
            newNodes.insert(newNodes.end(), newStreams.begin(), newStreams.end());
        } else {
            newNodes.push_back(node);
        }
    }
    m_tree->setChildren(newNodes); // FIXME This probably leaks
}

void CodeResolver::expandTypeBundles()
{

}

bool CodeResolver::reduceConstExpression(ExpressionNode *expr, QVector<AST *> scope, AST *tree, double &expressionResult)
{
    bool isConstant = false;
    if (!expr->isUnary()) {
        AST *left = expr->getLeft();
        AST *right = expr->getRight();

        if (left->getNodeType() == AST::Expression) {
            double expressionValue = 0;
            bool isConstant =
                    reduceConstExpression(static_cast<ExpressionNode *>(left), scope, m_tree, expressionValue);
            if (isConstant) {
                ValueNode *newValue = new ValueNode(expressionValue, left->getLine());
                expr->replaceLeft(newValue);
            }
        }
        if (right->getNodeType() == AST::Expression) {
            double expressionValue = 0;
            bool isConstant =
                    reduceConstExpression(static_cast<ExpressionNode *>(right), scope, tree, expressionValue);
            if (isConstant) {
                ValueNode *newValue = new ValueNode(expressionValue, right->getLine());
                expr->replaceRight(newValue);
            }
        }
        QList<LangError> errorsLeft;
        QList<LangError> errorsRight;
        double leftValue = CodeValidator::evaluateConstReal(expr->getLeft(), scope, tree, errorsLeft);
        double rightValue = CodeValidator::evaluateConstReal(expr->getRight(), scope, tree, errorsRight);
        if ( errorsLeft.size() == 0 && errorsRight.size() == 0 ) {
            switch (expr->getExpressionType()) {
            case ExpressionNode::Multiply:
                expressionResult = leftValue * rightValue;
                break;
            case ExpressionNode::Divide:
                expressionResult = leftValue / rightValue;
                break;
            case ExpressionNode::Add:
                expressionResult = leftValue + rightValue;
                break;
            case ExpressionNode::Subtract:
                expressionResult = leftValue - rightValue;
                break;
            case ExpressionNode::And:
                expressionResult = (int) leftValue & (int) rightValue;
                break;
            case ExpressionNode::Or:
                expressionResult = (int) leftValue | (int) rightValue;
                break;
            case ExpressionNode::UnaryMinus:
            case ExpressionNode::LogicalNot:
                assert(0 == 1); // Should never get here
                break;
            }
            return true;
        } else {
            // Not a constant expression, can't resolve;
        }
    } else {
// TODO implement for unary
    }
    return isConstant;
}

void CodeResolver::resolveConstantInProperty(PropertyNode *property, QVector<AST *> scope)
{
    AST *value = property->getValue();
    if(value->getNodeType() == AST::Expression) {
        ExpressionNode *expr = static_cast<ExpressionNode *>(value);
        double expressionValue = 0;
        bool isConstant =
                reduceConstExpression(expr, scope, m_tree, expressionValue);
        if (isConstant) {
            ValueNode *newValue = new ValueNode(expressionValue, expr->getLine());
            property->replaceValue(newValue);
        }
    } else if(value->getNodeType() == AST::Name) {
        QList<LangError> errors;
        NameNode *name = static_cast<NameNode *>(value);
        BlockNode *block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()), QVector<AST *>(), m_tree);
        if (block && block->getNodeType() == AST::Block && block->getObjectType() == "constant") { // Size == 1
            double resolvedValue = CodeValidator::evaluateConstReal(value, QVector<AST *>(), m_tree, errors);
            if (errors.size() == 0) {
                ValueNode *newValue = new ValueNode(resolvedValue, value->getLine());
                property->replaceValue(newValue);
            }
        }
    } else if (value->getNodeType() == AST::Bundle) {

    }
}

void CodeResolver::resolveConstantsInNode(AST *node, QVector<AST *> scope)
{
    if (node->getNodeType() == AST::Stream) {
        StreamNode *stream = static_cast<StreamNode *>(node);
        resolveConstantsInNode(stream->getLeft(), scope);
        resolveConstantsInNode(stream->getRight(), scope);
    } if (node->getNodeType() == AST::Function) {
        FunctionNode *func = static_cast<FunctionNode *>(node);
        vector<PropertyNode *> properties = func->getProperties();
        foreach(PropertyNode *property, properties) {
            resolveConstantInProperty(property, scope);
        }

    } else if(node->getNodeType() == AST::Block
              || node->getNodeType() == AST::BlockBundle) {
        BlockNode *block = static_cast<BlockNode *>(node);
        vector<PropertyNode *> properties = block->getProperties();
        foreach(PropertyNode *property, properties) {
            resolveConstantInProperty(property, scope);
        }
    }
}

QVector<StreamNode *> CodeResolver::expandBundleStream(StreamNode *stream, int size)
{
    QVector<StreamNode *> streams;
    if (size == -1) {
        size = CodeValidator::largestBundleSize(stream, m_tree);
        Q_ASSERT(size > 0);
    }
    if (size == 1) {
        streams << static_cast<StreamNode *>(stream->deepCopy());
        return streams;
    }
    for (int i = 0; i<size; i++) {
        AST *left = stream->getLeft();
        AST *right = stream->getRight();
        AST *newLeft,*newRight;
        newLeft = expandStreamMember(left, i);
        newRight = expandStreamMember(right, i);
        StreamNode *newStream = new StreamNode(newLeft, newRight, stream->getLine());
        streams << newStream;
    }
    return streams;
}

AST *CodeResolver::expandStreamMember(AST *node, int i)
{
    AST *newNode;
    if (node->getNodeType() == AST::Function) {
        newNode = static_cast<FunctionNode *>(node)->deepCopy();
    } else if (node->getNodeType() == AST::Name) {
        NameNode *nameNode = static_cast<NameNode *>(node);
        ValueNode *indexNode = new ValueNode(i + 1, nameNode->getLine());
        newNode = new BundleNode(nameNode->getName(), indexNode ,nameNode->getLine());
        newNode->setRate(node->getRate());
    } else if (node->getNodeType() == AST::BundleRange) {
        assert(0==1); //TODO implement here
    } else if (node->getNodeType() == AST::Stream) {
        StreamNode *streamNode = static_cast<StreamNode *>(node);
        AST * left = expandStreamMember(streamNode->getLeft(), i);
        AST * right = expandStreamMember(streamNode->getRight(), i);
        newNode = new StreamNode(left, right, streamNode->getLine());
        newNode->setRate(node->getRate());
    } else if (node->getNodeType() == AST::Bundle) {
        newNode = static_cast<BundleNode *>(node)->deepCopy(); // TODO must check how many connections the type supports
    } else {
        qFatal("Node type not supported in CodeResolver::expandStreamMember");
    }
    return newNode;
}





