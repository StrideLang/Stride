#include <cassert>

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
    Q_ASSERT(node->getRate() == -1); // Make sure rate hasn't been set
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
                } else if (propertyValue->getNodeType() ==AST::Real) {
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

void CodeResolver::declareUnknownStreamSymbols(StreamNode *stream, AST * tree)
{
    AST *left = stream->getLeft();

    if (left->getNodeType() == AST::Name) {
        NameNode *name = static_cast<NameNode *>(left);
        BlockNode *block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()), QVector<AST *>(), tree);
        if (!block) {
            int size = CodeValidator::largestBundleSize(stream, tree);
            if (size == 1) {
                BlockNode *block = new BlockNode(name->getName(), "signal", NULL, -1);
                tree->addChild(block);
            } else if (size > 1) {
                BundleNode *bundle = new BundleNode(name->getName(),
                                                    new ValueNode(size, -1), -1);
                BlockNode *block = new BlockNode(bundle, "signal", NULL, -1);
                tree->addChild(block);

            }
        }
    }

    AST *right = stream->getRight();
    if (right->getNodeType() == AST::Stream) {
        declareUnknownStreamSymbols(static_cast<StreamNode *>(right), tree);
    } else if (right->getNodeType() == AST::Name) {
        NameNode *name = static_cast<NameNode *>(right);
        BlockNode *block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()), QVector<AST *>(), tree);
        if (!block) {
            int size = CodeValidator::largestBundleSize(stream, tree);
            if (size == 1) {
                BlockNode *block = new BlockNode(name->getName(), "signal", NULL, -1);
                tree->addChild(block);
            } else if (size > 1) {
                BundleNode *bundle = new BundleNode(name->getName(),
                                                    new ValueNode(size, -1), -1);
                BlockNode *block = new BlockNode(bundle, "signal", NULL, -1);
                tree->addChild(block);

            }
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

}

void CodeResolver::expandStreamBundles()
{
    vector<AST *> nodes = m_tree->getChildren();
    for(unsigned int i = 0; i < nodes.size(); i++) {
        AST* node = nodes.at(i);
        if (node->getNodeType() == AST::Stream) {
            QVector<StreamNode *> streams = expandBundleStream(static_cast<StreamNode *>(node));
            node->deleteChildren();
            StreamNode *oldNode = static_cast<StreamNode *>(node);
            nodes.erase(nodes.begin() + i);
            delete oldNode;
            foreach(StreamNode *stream, streams) {
                nodes.insert(nodes.begin() + i, stream);
                i++;
            }
            i--;
        }
    }
    m_tree->setChildren(nodes); // FIXME This probably leaks
}

void CodeResolver::expandTypeBundles()
{

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





