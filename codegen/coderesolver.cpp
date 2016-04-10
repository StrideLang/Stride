
#include <QDebug>

#include "coderesolver.h"
#include "codevalidator.h"

CodeResolver::CodeResolver(StreamPlatform &platform, AST *tree) :
    m_platform(platform), m_tree(tree), m_connectorCounter(0)
{

}

CodeResolver::~CodeResolver()
{

}

void CodeResolver::preProcess()
{
    insertBuiltinObjects();
    resolveStreamSymbols();
    resolveRates();
    resolveConstants();
    expandStreamMembers();
    sliceStreams();
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
//    Q_ASSERT(rate != -1);
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

double CodeResolver::findRateInProperties(vector<PropertyNode *> properties, QVector<AST *> scope, AST *tree)
{
    foreach(PropertyNode *property, properties) {
        if (property->getName() == "rate") { // FIXME this assumes that a property named rate always applies to stream rate...
            AST *propertyValue = property->getValue();
            QList<LangError> errors;
            int rate = -1;
            if (propertyValue->getNodeType() == AST::Int) {
                rate = CodeValidator::evaluateConstInteger(propertyValue, QVector<AST *>(), tree, errors);
                return rate;
            } else if (propertyValue->getNodeType() == AST::Real) {
                rate = CodeValidator::evaluateConstReal(propertyValue, QVector<AST *>(), tree, errors);
                return rate;
            } else if (propertyValue->getNodeType() == AST::Name) {
                BlockNode* valueDeclaration =  CodeValidator::findDeclaration(
                            QString::fromStdString(static_cast<NameNode *>(propertyValue)->getName()), scope, tree);
                if (valueDeclaration && valueDeclaration->getObjectType() == "constant") {
                    PropertyNode *property = CodeValidator::findPropertyByName(valueDeclaration->getProperties(), "value");
                    if (property) {
                        ValueNode *propertyValue = static_cast<ValueNode *>(property->getValue());
                        if (propertyValue->getNodeType() == AST::Int) {
                            rate = CodeValidator::evaluateConstInteger(propertyValue, QVector<AST *>(), tree, errors);
                        } else if (propertyValue->getNodeType() ==AST::Real) {
                            rate = CodeValidator::evaluateConstReal(propertyValue, QVector<AST *>(), tree, errors);
                        }
                    }
                }
                if (errors.size() == 0) {
                    return rate;
                }
            }
        }
    }
    return -1;
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
        double rate = findRateInProperties(declaration->getProperties(), scope, tree);
        if (rate == -1) {
            QString typeName = QString::fromStdString(declaration->getObjectType());
            rate = getDefaultForTypeAsDouble(typeName, "rate");
//            ValueNode *value = new ValueNode(rate, -1);
//            declaration->addProperty(new PropertyNode("rate", value, -1));
        }
        name->setRate(rate);
        return rate;
    } else if (node->getNodeType() == AST::Bundle) {
        BundleNode *bundle = static_cast<BundleNode *>(node);
        BlockNode* declaration =  CodeValidator::findDeclaration(QString::fromStdString(bundle->getName()), scope, tree);
        if (!declaration) {
            return -1;
        }
        double rate = findRateInProperties(declaration->getProperties(), scope, tree);
        if (rate == -1) {
            QString typeName = QString::fromStdString(declaration->getObjectType());
            rate = getDefaultForTypeAsDouble(typeName, "rate");
//            ValueNode *value = new ValueNode(rate, -1);
//            declaration->addProperty(new PropertyNode("rate", value, -1));
        }
        bundle->setRate(rate);
        return rate;
    } else if (node->getNodeType() == AST::Function) {
        return -1;
    }
    return -1;
}

void CodeResolver::insertBuiltinObjects()
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
            ListNode *indexList = new ListNode(new ValueNode(object.getSize(), -1), -1);
            BundleNode *bundle = new BundleNode(object.getName().toStdString(),indexList, -1);

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

double CodeResolver::createSignalDeclaration(QString name, int size, AST *tree)
{
    BlockNode *newBlock;
    double nameRate = -1;
    Q_ASSERT(size > 0);
    if (size == 1) {
        newBlock = new BlockNode(name.toStdString(), "signal", NULL, -1);
    } else if (size > 1) {
        ListNode *indexList = new ListNode(new ValueNode(size, -1), -1);
        BundleNode *bundle = new BundleNode(name.toStdString(),indexList, -1);
        newBlock = new BlockNode(bundle, "signal", NULL, -1);
    }

    nameRate = getDefaultForTypeAsDouble("signal", "rate");
//    ValueNode *value = new ValueNode(defaultRate, -1);
//    newBlock->addProperty(new PropertyNode("rate", value, -1));

    tree->addChild(newBlock);
    return nameRate;
}

void CodeResolver::declareUnknownStreamSymbols(StreamNode *stream, AST *previousStreamMember, AST * tree)
{
    AST *left = stream->getLeft();
    AST *right = stream->getRight();

    AST * nextStreamMember;
    if (right->getNodeType() != AST::Stream) {
        nextStreamMember = right;
    } else {
        nextStreamMember = static_cast<StreamNode *>(right)->getLeft();
    }

    if (left->getNodeType() == AST::Name) {
        NameNode *name = static_cast<NameNode *>(left);
        BlockNode *block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()), QVector<AST *>(), tree);
        if (!block) { // Not declared, so make declaration
            QList<LangError> errors;
            QVector<AST *> scope;
            int size = -1;
            if (previousStreamMember) {
                size = CodeValidator::getNodeNumOutputs(previousStreamMember, m_platform, scope, m_tree, errors);
            }
            if (size <= 0) { // Look to the right if can't resolve from the left
                size = CodeValidator::getNodeNumInputs(nextStreamMember, m_platform, scope, m_tree, errors);
            }
            if (size <= 0) { // None of the elements in the stream have size
                size = 1;
            }
            double rate = createSignalDeclaration(QString::fromStdString(name->getName()), size, tree);
            name->setRate(rate);
        }
    }

    if (right->getNodeType() == AST::Stream) {
        declareUnknownStreamSymbols(static_cast<StreamNode *>(right), left, tree);
    } else if (right->getNodeType() == AST::Name) {
        NameNode *name = static_cast<NameNode *>(right);
        BlockNode *block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()), QVector<AST *>(), tree);
        if (!block) { // Not declared, so make declaration
            QList<LangError> errors;
            QVector<AST *> scope;
            int size = CodeValidator::getNodeNumOutputs(left, m_platform, scope, m_tree, errors);
            if (size <= 0) { // None of the elements in the stream have size
                size = 1;
            }
            double rate = createSignalDeclaration(QString::fromStdString(name->getName()), size, tree);
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
            declareUnknownStreamSymbols(stream, NULL, m_tree);
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

void CodeResolver::expandStreamMembers()
{
    vector<AST *> nodes = m_tree->getChildren();
    QVector<AST *> newReversedNodes;
    QVector<AST *> newNodes;

    // Need to traverse streams in reverse order to resolve later streams first.
    vector<AST *>::reverse_iterator rit = nodes.rbegin();
    for (; rit!= nodes.rend(); ++rit) {
        AST *node = *rit;
        if (node->getNodeType() == AST::Stream) {
            StreamNode *oldNode = static_cast<StreamNode *>(node);
            QVector<AST *> newStreams = expandStreamNode(oldNode);
            for(int i = 0; i < newStreams.size(); i++) {
                newReversedNodes << newStreams[newStreams.size() - i - 1];
            }
        } else {
            newReversedNodes << node->deepCopy();
        }
    }
    // Now reverse vectors to put them back.
    for(int k = 0; k < newReversedNodes.size(); k++) {
        newNodes << newReversedNodes[newReversedNodes.size() - k -1];
    }
    m_tree->deleteChildren();
    vector<AST *> newNodesStl = newNodes.toStdVector();
    m_tree->setChildren(newNodesStl);
}

void CodeResolver::sliceStreams()
{
    vector<AST *> nodes = m_tree->getChildren();
    QVector<AST *> newNodes;

    // Need to traverse streams in reverse order to resolve later streams first.
    vector<AST *>::reverse_iterator rit = nodes.rbegin();
    for (; rit!= nodes.rend(); ++rit) {
        AST *node = *rit;
        if (node->getNodeType() == AST::Stream) {
            StreamNode *oldNode = static_cast<StreamNode *>(node);
            QVector<AST *> newStreams = sliceStream(oldNode);
            newNodes << newStreams;
        } else {
            newNodes << node->deepCopy();
        }
    }
    m_tree->deleteChildren();
    vector<AST *> newNodesStl;
    QVector<AST *>::iterator i = newNodes.end();
    while (i != newNodes.begin()) { // Now reverse again while putting in STL vector
        --i;
        newNodesStl.push_back(*i);
    }
    m_tree->setChildren(newNodesStl);
}

ValueNode *CodeResolver::reduceConstExpression(ExpressionNode *expr, QVector<AST *> scope, AST *tree)
{
    AST *left, *right;
    bool isConstant;

    if (!expr->isUnary()) {
        left = expr->getLeft();
    } else {
        left = expr->getValue();
    }

    ValueNode *newValue = resolveConstant(left, scope);
    if (newValue) {
        if (expr->isUnary()) {
            expr->replaceValue(newValue);
        } else {
            expr->replaceLeft(newValue);
        }
        left = newValue;
    }
    if (!expr->isUnary()) {
        right = expr->getRight();
        newValue = resolveConstant(right, scope);
        if (newValue) {
            expr->replaceRight(newValue);
            right = newValue;
        }
        isConstant = (left->getNodeType() == AST::Int || left->getNodeType() == AST::Real) && (right->getNodeType() == AST::Int || right->getNodeType() == AST::Real);
    } else {
        isConstant = (left->getNodeType() == AST::Int || left->getNodeType() == AST::Real);
    }

    if (isConstant) {
        ValueNode *result = NULL;
        switch (expr->getExpressionType()) {
        case ExpressionNode::Multiply:
            result = multiply(static_cast<ValueNode *>(left), static_cast<ValueNode *>(right));
            break;
        case ExpressionNode::Divide:
            result = divide(static_cast<ValueNode *>(left), static_cast<ValueNode *>(right));
            break;
        case ExpressionNode::Add:
            result = add(static_cast<ValueNode *>(left), static_cast<ValueNode *>(right));
            break;
        case ExpressionNode::Subtract:
            result = subtract(static_cast<ValueNode *>(left), static_cast<ValueNode *>(right));
            break;
        case ExpressionNode::And:
            result = logicalAnd(static_cast<ValueNode *>(left), static_cast<ValueNode *>(right));
            break;
        case ExpressionNode::Or:
            result = logicalOr(static_cast<ValueNode *>(left), static_cast<ValueNode *>(right));
            break;
        case ExpressionNode::UnaryMinus:
            result = unaryMinus(static_cast<ValueNode *>(left));
            break;
        case ExpressionNode::LogicalNot:
            result = logicalNot(static_cast<ValueNode *>(left));
            break;
        default:
            Q_ASSERT(0 == 1); // Should never get here
            break;
        }
        if(result) {
            return result;
        }
    }
    return NULL;
}

ValueNode *CodeResolver::resolveConstant(AST* value, QVector<AST *> scope)
{
    ValueNode *newValue = NULL;
    if(value->getNodeType() == AST::Expression) {
        ExpressionNode *expr = static_cast<ExpressionNode *>(value);
        newValue = reduceConstExpression(expr, scope, m_tree);
        return newValue;
    } else if(value->getNodeType() == AST::Name) {
        NameNode *name = static_cast<NameNode *>(value);
        BlockNode *block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()), QVector<AST *>(), m_tree);
        if (block && block->getNodeType() == AST::Block && block->getObjectType() == "constant") { // Size == 1
            AST *blockValue = block->getPropertyValue("value");
            if (blockValue->getNodeType() == AST::Int || blockValue->getNodeType() == AST::Real ) {
                return static_cast<ValueNode *>(blockValue->deepCopy());
            }
            newValue = resolveConstant(block->getPropertyValue("value"), scope);
            return newValue;
        }
    } else if (value->getNodeType() == AST::Bundle) {

    }
    return NULL;
}

void CodeResolver::resolveConstantsInNode(AST *node, QVector<AST *> scope)
{
    if (node->getNodeType() == AST::Stream) {
        StreamNode *stream = static_cast<StreamNode *>(node);
        resolveConstantsInNode(stream->getLeft(), scope);
        resolveConstantsInNode(stream->getRight(), scope);
    } else if (node->getNodeType() == AST::Function) {
        FunctionNode *func = static_cast<FunctionNode *>(node);
        vector<PropertyNode *> properties = func->getProperties();
        foreach(PropertyNode *property, properties) {
            ValueNode *newValue = resolveConstant(property->getValue(), scope);
            if (newValue) {
                property->replaceValue(newValue);
            }
        }
    } else if(node->getNodeType() == AST::Block) {
        BlockNode *block = static_cast<BlockNode *>(node);
        vector<PropertyNode *> properties = block->getProperties();
        foreach(PropertyNode *property, properties) {
            ValueNode *newValue = resolveConstant(property->getValue(), scope);
            if (newValue) {
                property->replaceValue(newValue);
            }
        }
    } else if(node->getNodeType() == AST::BlockBundle) {
        BlockNode *block = static_cast<BlockNode *>(node);
        vector<PropertyNode *> properties = block->getProperties();
        foreach(PropertyNode *property, properties) {
            ValueNode *newValue = resolveConstant(property->getValue(), scope);
            if (newValue) {
                property->replaceValue(newValue);
            }
        }
        BundleNode *bundle = block->getBundle();
        ListNode *indexList = bundle->index();
        vector<AST *> elements = indexList->getChildren();
        foreach(AST *element, elements) {
            if (element->getNodeType() == AST::Expression) {
                ExpressionNode *expr = static_cast<ExpressionNode *>(element);
                ValueNode *newValue = reduceConstExpression(expr, scope, m_tree);
                if (newValue) {
                    indexList->replaceMember(newValue, element);
                    element->deleteChildren();
                    delete element;
                }
            }
        }
    }
}

double CodeResolver::getDefaultForTypeAsDouble(QString type, QString port)
{
    double outValue = -1;
    QVariant defaultValue = m_platform.getDefaultPortValueForType(type, port);
    if (defaultValue.type() != QVariant::String) {
        outValue = defaultValue.toDouble();
    } else {
        BlockNode *block = CodeValidator::findDeclaration(defaultValue.toString(),
                                                          QVector<AST *>(), m_tree);
        if (block && block->getObjectType() == "constant") {
            AST *propValue = block->getPropertyValue("value");
            QList<LangError> errors;
            double rate = CodeValidator::evaluateConstReal(propValue, QVector<AST *>(),
                                                           m_tree, errors);
            if (errors.size() == 0) {
                outValue = rate;
            }
        }
    }
    return outValue;
}

ValueNode *CodeResolver::multiply(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() * right->getIntValue(), left->getLine());
    } else { // Automatic casting from int to real
        Q_ASSERT((left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Int)
                 || (left->getNodeType() == AST::Int &&  right->getNodeType() == AST::Real)
                 || (left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Real));
        return new ValueNode(left->toReal() * right->toReal(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::divide(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() / right->getIntValue(), left->getLine());
    } else { // Automatic casting from int to real
        Q_ASSERT((left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Int)
                 || (left->getNodeType() == AST::Int &&  right->getNodeType() == AST::Real)
                 || (left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Real));
        return new ValueNode(left->toReal() / right->toReal(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::add(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() + right->getIntValue(), left->getLine());
    } else { // Automatic casting from int to real
        Q_ASSERT((left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Int)
                 || (left->getNodeType() == AST::Int &&  right->getNodeType() == AST::Real)
                 || (left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Real));
        return new ValueNode(left->toReal() + right->toReal(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::subtract(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() - right->getIntValue(), left->getLine());
    } else { // Automatic casting from int to real
        Q_ASSERT((left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Int)
                 || (left->getNodeType() == AST::Int &&  right->getNodeType() == AST::Real)
                 || (left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Real));
        return new ValueNode(left->toReal() - right->toReal(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::unaryMinus(ValueNode *value)
{
    if (value->getNodeType() == AST::Int) {
        return new ValueNode(- value->getIntValue(), value->getLine());
    } else if (value->getNodeType() == AST::Real){
        return new ValueNode(- value->getRealValue(), value->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::logicalAnd(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() & right->getIntValue(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::logicalOr(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() | right->getIntValue(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::logicalNot(ValueNode *value)
{
    if (value->getNodeType() == AST::Int) {
        return new ValueNode(~ (value->getIntValue()), value->getLine());
    }
    return NULL;
}

QVector<AST *> CodeResolver::expandStreamNode(StreamNode *stream)
{
    QList<LangError> errors;
    QVector<AST *> scope;
    int size = CodeValidator::numParallelStreams(stream, m_platform, scope, m_tree, errors);
    QVector<AST *> streams;
    if (size <= 1) {
        streams << stream->deepCopy();
        return streams;
    }
    AST *left = stream->getLeft();
    AST *right = stream->getRight();
    for (int i = 0; i < size; i++) {
        AST *newLeft,*newRight;
        int rightNumInputs = CodeValidator::getNodeNumInputs(right, m_platform, scope, m_tree, errors);
        newLeft = expandStream(left, i, rightNumInputs, 0);
        newRight = expandStream(right, i, 1, CodeValidator::getNodeNumOutputs(left, m_platform, scope, m_tree, errors));
        StreamNode *newStream = new StreamNode(newLeft, newRight, stream->getLine());
        streams << newStream;
    }
    return streams;
}

QVector<AST *> CodeResolver::sliceStream(StreamNode *stream)
{
    QList<LangError> errors;
    QVector<AST *> scope;
    int size = CodeValidator::numParallelStreams(stream, m_platform, scope, m_tree, errors);

    QVector<AST *> streams;
    if (size == 1 || size == -1) {
        streams << stream->deepCopy();
        return streams;
    }
    AST *left = stream->getLeft();
//    AST *marker = stream->getLeft(); // To mark where last split occured
    int numOutputs = CodeValidator::getNodeNumOutputs(left, m_platform, scope, m_tree, errors);
    Q_ASSERT(numOutputs >= 0);
    AST *nextNode = stream->getRight();
    while (nextNode) {
        if (nextNode->getNodeType() == AST::Stream) {
            StreamNode *rightStream = static_cast<StreamNode *>(nextNode);
            int nextSize = CodeValidator::getNodeSize(rightStream->getLeft(), m_tree);
            Q_ASSERT(nextSize != -1);
//            Q_ASSERT(nextSize >=)
            if (nextSize > size) {
                QVector<AST *> nextStreams = expandStreamNode(rightStream);
                foreach(AST * stream, nextStreams) {
                    // Now prepend the new connection symbol
                    QString nodeName = QString("__Connector%1").arg(m_connectorCounter++, 2);
                    NameNode *newConnectionName = new NameNode(nodeName.toStdString(), -1);
                    streams << new StreamNode(newConnectionName, stream->deepCopy(), -1);
                    stream->deleteChildren();
                    delete stream;
                }
                nextNode = NULL;
                continue;
            } else if (nextSize == size) {
                // Just keep going if size hasn't changed
            } else {
//                qFatal("Error, stream size shouldn't have decreased");
            }
            nextNode = rightStream->getRight();
        } else { // Last member of the stream (any node that is not StreamNode *)

            int nextSize = CodeValidator::getNodeNumInputs(nextNode, m_platform, scope, m_tree, errors)
                    * CodeValidator::getNodeSize(nextNode, m_tree);
            if (nextSize > size) {
                // TODO expand up to here and add new signal to connect
//                QVector<AST *> nextStreams = expandStreamFromMemberSizes(rightStream);
//                foreach(AST * stream, nextStreams) {
//                    // Now prepend the new connection symbol
//                    QString nodeName = QString("__Connector%1").arg(m_connectorCounter++, 2);
//                    NameNode *newConnectionName = new NameNode(nodeName.toStdString(), -1);
//                    streams << new StreamNode(newConnectionName, stream, -1);
//                }
                nextNode = NULL;
                continue;
            } else if (nextSize == size) { // All streams are parallel and of the same size

                AST *left = stream->getLeft();
                AST *right = stream->getRight();

                for (int i = 0; i < size; i++) {
                    AST *newLeft, *newRight;
                    newRight = expandStream(right, i);
                    newLeft = expandStream(left, i);
                    StreamNode *newStream = new StreamNode(newLeft, newRight, stream->getLine());
                    streams << newStream;
                }
            } else {
//                qFatal("Error, stream size shouldn't have decreased");
            }
            nextNode = NULL;
        }
    }
    return streams;
}

StreamNode *CodeResolver::splitStream(StreamNode *stream, AST *closingNode, AST *endNode)
{
    StreamNode *outStream;
    AST * left = stream->getLeft();
    AST * right = stream->getRight();
    if (right == endNode) { // endNode is the last node in the stream, just append closingNode
        StreamNode *lastStream = new StreamNode(right->deepCopy(), closingNode, -1);
        StreamNode *outStream = new StreamNode(left->deepCopy(), lastStream, -1);
        return outStream;
    } else if (left == endNode) { // There is more stream, but we have reached the split point
        StreamNode *outStream = new StreamNode(left->deepCopy(), closingNode, -1);
        return outStream;
    }
    if  (right != endNode) {
        Q_ASSERT(right->getNodeType() == AST::Stream); // Shouldn't reach the end of the stream before reaching endNode
        outStream = splitStream(static_cast<StreamNode *>(right), closingNode, endNode);
        StreamNode *finalStream = new StreamNode(left, outStream->deepCopy(), -1);
        outStream->deleteChildren();
        delete outStream;
        return finalStream;
    }
    qFatal("Shouldn't get here");
    return outStream;
}

AST *CodeResolver::expandStream(AST *node, int index, int rightNumInputs, int leftNumOutputs)
{
    AST *newNode;
    QList<LangError> errors;
    QVector<AST *> scope;
    if (node->getNodeType() == AST::Function) {
        FunctionNode *func = static_cast<FunctionNode *>(node);
        int numOutputs = CodeValidator::getNodeNumOutputs(node, m_platform, scope, m_tree, errors);
        if (numOutputs != rightNumInputs) {
            int numInstances = (numOutputs == 0? rightNumInputs:(int) (rightNumInputs/ numOutputs));
            func->setParallelInstances(numInstances);
        }
        func->setParallelInstances(func->getParallelInstances() + 1);
        newNode = static_cast<FunctionNode *>(node)->deepCopy();
    } else if (node->getNodeType() == AST::Name) {
        NameNode *nameNode = static_cast<NameNode *>(node);
        if (rightNumInputs == 1) {
            int numOutputs = CodeValidator::getNodeNumOutputs(node, m_platform, scope, m_tree, errors);
            Q_ASSERT(numOutputs > 0);
            if (numOutputs == 1) {
                newNode = node->deepCopy();
            } else {
                ListNode *indexList = new ListNode(new ValueNode(index + 1, nameNode->getLine()), -1);
                newNode = new BundleNode(nameNode->getName(), indexList ,nameNode->getLine());
                newNode->setRate(node->getRate());
            }
        } else {
            int startIndex = index * leftNumOutputs;
            int endIndex = (index + 1) * leftNumOutputs ;
            ValueNode *startIndexNode = new ValueNode(startIndex + 1, nameNode->getLine());
            ValueNode *endIndexNode = new ValueNode(endIndex + 1, nameNode->getLine());
            RangeNode *range = new RangeNode(startIndexNode, endIndexNode, nameNode->getLine());
            ListNode *list = new ListNode(range, nameNode->getLine());
            newNode = new BundleNode(nameNode->getName(), list, nameNode->getLine());
            newNode->setRate(node->getRate());
        }
    } else if (node->getNodeType() == AST::Stream) {
        QList<LangError> errors;
        QVector<AST *> scope;
        StreamNode *streamNode = static_cast<StreamNode *>(node);
        int rightNumInputsStream = CodeValidator::getNodeNumInputs(streamNode->getRight(), m_platform, scope, m_tree, errors);
        int leftNumOutputsStream = CodeValidator::getNodeNumOutputs(streamNode->getLeft(), m_platform, scope, m_tree, errors);
        AST * newRight = expandStream(streamNode->getRight(), index, 1, leftNumOutputsStream);
        Q_ASSERT(rightNumInputsStream > 0);
        AST * newLeft = expandStream(streamNode->getLeft(), index, rightNumInputsStream, leftNumOutputs);
        newNode = new StreamNode(newLeft, newRight, streamNode->getLine());
        newNode->setRate(node->getRate());
    } else if (node->getNodeType() == AST::Bundle) {
        newNode = node->deepCopy(); // TODO must check how many connections the type supports
    } else if (node->getNodeType() == AST::List) {
        QList<LangError> errors;
        CodeValidator::getMemberFromList(static_cast<ListNode *>(node),
                                         index + 1, errors);
        Q_ASSERT(errors.size() == 0);
    } else {
        qFatal("Node type not supported in CodeResolver::expandStreamMember");
    }
    return newNode;
}

