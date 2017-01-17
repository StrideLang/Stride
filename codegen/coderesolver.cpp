
#include <QDebug>

#include "coderesolver.h"
#include "codevalidator.h"

CodeResolver::CodeResolver(StridePlatform *platform, AST *tree) :
    m_platform(platform), m_tree(tree), m_connectorCounter(0)
{

}

CodeResolver::~CodeResolver()
{

}

void CodeResolver::preProcess()
{
    insertBuiltinObjects();
    fillDefaultProperties();
    declareModuleInternalBlocks();
    resolveConstants();
    expandParallel(); // Find better name this expands bundles, functions and declares undefined bundles
    resolveStreamSymbols();
    resolveRates();
    processDomains();
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
//    stream->setRate(rate);
}

void CodeResolver::fillDefaultPropertiesForNode(AST *node)
{
    if (node->getNodeType() == AST::Block || node->getNodeType() == AST::BlockBundle) {
        BlockNode *destBlock = static_cast<BlockNode *>(node);
        vector<PropertyNode *> blockProperties = destBlock->getProperties();
        QVector<AST *> typeProperties = CodeValidator::getPortsForType(
                    QString::fromStdString(destBlock->getObjectType()),
                    QVector<AST *>(), m_tree);
        if (typeProperties.isEmpty()) {
            qDebug() << "ERROR: fillDefaultProperties() No type definition for " << QString::fromStdString(destBlock->getObjectType());
            return;
        }
        for(PropertyNode *property : blockProperties) {
            fillDefaultPropertiesForNode(property->getValue());
        }

        for(AST *propertyListMember : typeProperties) {
            Q_ASSERT(propertyListMember->getNodeType() == AST::Block);
            BlockNode *portDescription = static_cast<BlockNode *>(propertyListMember);
            AST *propName = portDescription->getPropertyValue("name");
            Q_ASSERT(propName->getNodeType() == AST::String);
            string propertyName = static_cast<ValueNode *>(propName)->getStringValue();
            bool propertySet = false;
            for(PropertyNode *blockProperty : blockProperties) {
                if (blockProperty->getName() == propertyName) {
                    propertySet = true;
                    break;
                }
            }
            if (!propertySet) {
                AST *defaultValueNode = portDescription->getPropertyValue("default");
                PropertyNode *newProperty = new PropertyNode(propertyName,
                            defaultValueNode->deepCopy(),
                            portDescription->getFilename().data(), portDescription->getLine());
                destBlock->addProperty(newProperty);
            }
        }
    } else if (node->getNodeType() == AST::Function) {
//        FunctionNode *destFunc = static_cast<FunctionNode *>(node);
//        vector<PropertyNode *> blockProperties = destFunc->getProperties();
//        BlockNode *functionModule = CodeValidator::findDeclaration(
//                    QString::fromStdString(destFunc->getName()),
//                    QVector<AST *>(), m_tree);
//        vector<PropertyNode *> typeProperties = functionModule->getProperties();
//        if (typeProperties.size() < 1) {
//            qDebug() << "ERROR: fillDefaultProperties() No type definition for " << QString::fromStdString(destFunc->getName());
//            return;
//        }
//        foreach(PropertyNode *property, blockProperties) {
//            fillDefaultPropertiesForNode(property->getValue());
//        }

//        foreach(AST *propertyListMember, typeProperties) {
//            Q_ASSERT(propertyListMember->getNodeType() == AST::Property);
//            PropertyNode *property = static_cast<PropertyNode *>(propertyListMember);
//            string propertyName = property->getName();
//            bool propertySet = false;
//            foreach(PropertyNode *blockProperty, blockProperties) {
//                if (blockProperty->getName() == propertyName) {
//                    propertySet = true;
//                    break;
//                }
//            }
//            if (!propertySet) {
////                AST *defaultValueNode = property->getPropertyValue("default");
////                PropertyNode *newProperty = new PropertyNode(propertyName,
////                            defaultValueNode->deepCopy(),
////                            property->getFilename().data(), property->getLine());
////                destFunc->addProperty(newProperty);
//            }
//        }
    } else if (node->getNodeType() == AST::List) {
        ListNode *list = static_cast<ListNode *>(node);
        for(AST *listElement : list->getChildren()) {
            fillDefaultPropertiesForNode(listElement);
        }
    } else if (node->getNodeType() == AST::Stream) {
        StreamNode *stream = static_cast<StreamNode *>(node);
        for(AST *streamElement : stream->getChildren()) {
            fillDefaultPropertiesForNode(streamElement);
        }
    }
}

void CodeResolver::fillDefaultProperties()
{
    vector<AST *> nodes = m_tree->getChildren();
    for(unsigned int i = 0; i < nodes.size(); i++) {
        AST* node = nodes.at(i);
        fillDefaultPropertiesForNode(node);
    }
}


void CodeResolver::declareModuleInternalBlocks()
{
    for (AST *node : m_tree->getChildren()) {
        if (node->getNodeType() == AST::Block) {
            BlockNode *block = static_cast<BlockNode *>(node);
            AST *internalBlocks = block->getPropertyValue("blocks");
            if (block->getObjectType() == "module") {
                // First insert and resolve input and output domains for main ports. The input port takes the output domain if undefined.
                BlockNode *outputPortBlock = CodeValidator::getMainOutputPortBlock(block);
                AST *outDomain = outputPortBlock->getPropertyValue("domain");
                if (!outDomain || outDomain->getNodeType() == AST::None) {
                    NameNode *outBlockName = new NameNode("_OutputDomain", "", -1);
                    outputPortBlock->replacePropertyValue("domain", outBlockName);
                    outDomain = outBlockName;
                    AST *outDomainDeclaration = CodeValidator::findDeclaration("_OutputDomain", QVector<AST *>(), internalBlocks);
                    if (!outDomainDeclaration) {
                        BlockNode *newDomainBlock = createDomainDeclaration(QString::fromStdString("_OutputDomain"));
                        internalBlocks->addChild(newDomainBlock);
                    }
                }
                BlockNode *inputPortBlock = CodeValidator::getMainInputPortBlock(block);
                if (inputPortBlock) {
                    AST *inDomain = inputPortBlock->getPropertyValue("domain");
                    if (!inDomain || inDomain->getNodeType() == AST::None) {
                        inputPortBlock->replacePropertyValue("domain", outDomain->deepCopy());
                    }
                }

                // Then go through ports autodeclaring blocks
                ListNode *ports = static_cast<ListNode *>(block->getPropertyValue("ports"));
                if (ports->getNodeType() == AST::List) {
                    for (AST *port : ports->getChildren()) {
                        Q_ASSERT(port->getNodeType() == AST::Block);
                        BlockNode *portBlock = static_cast<BlockNode *>(port);
                        Q_ASSERT(portBlock->getObjectType() == "port");

                        // Properties that we need to auto-declare for
                        AST *ratePortValue = portBlock->getPropertyValue("rate");
                        AST *domainPortValue = portBlock->getPropertyValue("domain");
                        AST *sizePortValue = portBlock->getPropertyValue("size");
                        AST *blockPortValue = portBlock->getPropertyValue("block");
                        AST *directionPortValue = portBlock->getPropertyValue("direction");
                        AST *internalBlocks = block->getPropertyValue("blocks");

                        Q_ASSERT(ratePortValue && domainPortValue && sizePortValue && blockPortValue && directionPortValue);
                        Q_ASSERT(ratePortValue->getNodeType() == AST::Name || ratePortValue->getNodeType() == AST::None); // Catch on debug but fail gracefully on release
                        if (ratePortValue->getNodeType() == AST::Name) {
                            NameNode *nameNode = static_cast<NameNode *>(ratePortValue);
                            string name = nameNode->getName();
                            declareIfMissing(name, internalBlocks, new ValueNode(0, "", -1));

                            //                                    if (declaration->getObjectType() == "constant") {
                            //                                        // If existing declaration is not a constant then an error should be produced later when checking types
                            //                                        AST *value = declaration->getPropertyValue("value");
                            //                                        if (value)
                            //                                    }
                        } else if (ratePortValue->getNodeType() == AST::Int || ratePortValue->getNodeType() == AST::Real) {
                            // Do nothing
                        } else if (ratePortValue->getNodeType() == AST::None) {
                            // Do nothing
                        }  else {
                            qDebug() << "Rate unrecognized.";
                        }

                        Q_ASSERT(domainPortValue->getNodeType() == AST::Name || domainPortValue->getNodeType() == AST::None); // Catch on debug but fail gracefully on release
                        string domainName;

                        if (domainPortValue->getNodeType() == AST::None) { // Make default port domains match the main output/input port domain

                            AST *domainNode = nullptr;
                            QVector<AST *> subScope;
                            for (AST *node: internalBlocks->getChildren()) {
                                subScope << node;
                            }
//                            BlockNode *outputPortBlock = CodeValidator::getMainOutputPortBlock(block);
//                            if (outputPortBlock) {
//                                if (outputPortBlock->getDomain()->getNodeType() == AST::Name) {
//                                    string name = static_cast<NameNode *>(outputPortBlock->getDomain())->getName();
//                                    domainNode = CodeValidator::findDeclaration(QString::fromStdString(name), subScope, m_tree);
//                                } else {
//                                    domainNode = outputPortBlock->getDomain();
//                                }
//                            } else {
//                                BlockNode *inputPortBlock = CodeValidator::getMainInputPortBlock(block);
//                                if (inputPortBlock) { // If output port not available use input port domain
//                                    if (inputPortBlock->getDomain()->getNodeType() == AST::Name) {
//                                        string name = static_cast<NameNode *>(outputPortBlock->getDomain())->getName();
//                                        domainNode = CodeValidator::findDeclaration(QString::fromStdString(name), subScope, m_tree);
//                                    } else {
//                                        domainNode = inputPortBlock->getDomain();
//                                    }
//                                }
//                            }
                            if (domainNode && domainNode->getNodeType() != AST::None) {
                                domainName = CodeValidator::getDomainNodeString(domainNode);
                                if (domainName.size() > 0) {
                                    ValueNode *domainNameNode = new ValueNode(domainName, "", -1);
                                    portBlock->replacePropertyValue("domain", domainNameNode);
                                }
                            } else { // If no domain set and block has no domain, then make a new domain for port.
                                domainName = "_" + portBlock->getName() + "Domain";
                                // TODO check to make sure domain does not exist in scope

                                BlockNode *domainDeclaration = CodeValidator::findDeclaration(QString::fromStdString(domainName), subScope, m_tree);
                                if (!domainDeclaration) {
                                    domainDeclaration = createDomainDeclaration(QString::fromStdString(domainName));
                                    internalBlocks->addChild(domainDeclaration);
                                }
                                ValueNode *domainNameNode = new ValueNode(domainName, "", -1);
                                portBlock->replacePropertyValue("domain", domainNameNode);
                            }
                        } else if (domainPortValue->getNodeType() == AST::Name) { // Auto declare domain if not declared
                            NameNode *nameNode = static_cast<NameNode *>(domainPortValue);
                            domainName = nameNode->getName();
                            declareIfMissing(domainName, internalBlocks, new ValueNode("", -1)); // TODO should we autodeclare domains here?
                        }

                        Q_ASSERT(sizePortValue->getNodeType() == AST::Int || sizePortValue->getNodeType() == AST::Name || sizePortValue->getNodeType() == AST::None); // Catch on debug but fail gracefully on release
                        if (sizePortValue->getNodeType() == AST::Name) {
                            NameNode *nameNode = static_cast<NameNode *>(sizePortValue);
                            string name = nameNode->getName();
                            AST *internalBlocks = block->getPropertyValue("blocks");
                            declareIfMissing(name, internalBlocks, new ValueNode(0, "", -1));
                        }

                        // Now do auto declaration of IO blocks if not declared.
                        Q_ASSERT(blockPortValue->getNodeType() == AST::Name || blockPortValue->getNodeType() == AST::None); // Catch on debug but fail gracefully on release
                        if (blockPortValue->getNodeType() == AST::Name) {
                            NameNode *nameNode = static_cast<NameNode *>(blockPortValue);
                            string name = nameNode->getName();
                            AST *internalBlocks = block->getPropertyValue("blocks");
                            BlockNode *newSignal = CodeValidator::findDeclaration(QString::fromStdString(name), QVector<AST *>(), internalBlocks);
                            if (!newSignal) {
                                newSignal = createSignalDeclaration(QString::fromStdString(name));
                                internalBlocks->addChild(newSignal);
                                newSignal->setDomain(domainName);
                                //                                    if (direction == "input") {
                                //                                    } else if (direction == "output") {

                                //                                    }
                            }
                        } else if (blockPortValue->getNodeType() == AST::None) {
                            std::string directionName = static_cast<ValueNode *>(directionPortValue)->getStringValue();
                            Q_ASSERT(directionName == "input" || directionName == "output");
                            string defaultName;
                            if (directionName == "input") {
                                defaultName = "Input";
                            } else if (directionName == "output") {
                                defaultName = "Output";
                            }
                            NameNode *name = new NameNode(defaultName, "", -1);
                            portBlock->replacePropertyValue("block", name);
                            BlockNode *newSignal = CodeValidator::findDeclaration(QString::fromStdString(defaultName), QVector<AST *>(), internalBlocks);
                            if (!newSignal) {
                                newSignal = createSignalDeclaration(QString::fromStdString(defaultName));
                                internalBlocks->addChild(newSignal);
                                newSignal->setDomain(domainName);
                            }
                        }
                    }
                } else if (ports->getNodeType() == AST::None) {
                    // If port list is None, then ignore
                } else {
                    qDebug() << "ERROR! ports property must be a list or None!";
                }
            }
        }

    }
}

void CodeResolver::expandParallelStream(StreamNode *stream, QVector<AST *> scopeStack, AST *tree)
{
    QList<LangError> errors;
    StreamNode *subStream = stream;

    // Figure out stream IO sizes
    AST *left = stream->getLeft();
    AST *right = stream->getRight();
    QVector<QPair<int, int> > IOs;
    while (right) {
        if (left->getNodeType() == AST::Function) { // Expand from properties size to list
            AST *newFunctions = expandFunctionFromProperties(static_cast<FunctionNode *>(left),
                                                             scopeStack, m_tree);
            if (newFunctions) {
                subStream->setLeft(newFunctions);
                left = subStream->getLeft();
            }
        }
        QPair<int, int> io;
        io.first = CodeValidator::getNodeNumInputs(left, scopeStack, m_tree, errors);
        io.second = CodeValidator::getNodeNumOutputs(left, scopeStack, m_tree, errors);
        IOs << io;
        if (right->getNodeType() == AST::Stream) {
            subStream = static_cast<StreamNode *>(right);
            left = subStream->getLeft();
            right = subStream->getRight();
        } else {
            if (right->getNodeType() == AST::Function) {
                AST *newFunctions = expandFunctionFromProperties(static_cast<FunctionNode *>(right),
                                                                 scopeStack, m_tree);
                if (newFunctions) {
                    subStream->setRight(newFunctions);
                    right = subStream->getRight();
                }
            }
            io.first = CodeValidator::getNodeNumInputs(right, scopeStack, m_tree, errors);
            io.second = CodeValidator::getNodeNumOutputs(right, scopeStack, m_tree, errors);
            IOs << io;
            right = NULL;
        }
    }
    // Now go through comparing number of outputs to number of inputs to figure out if we
    // need to duplicate any members
    QVector<int> numCopies;
    numCopies << 1;
    for(int i = 1; i < IOs.size(); ++i) {
        int numPrevOut = IOs[i - 1].second * numCopies.back();
        int numCurIn = IOs[i].first;
        if (numPrevOut == -1) { // Found undeclared block
            numCopies << 1;
            continue;
        }
        if (numPrevOut > numCurIn) { // Need to clone next
            if (numPrevOut/(float)numCurIn == numPrevOut/numCurIn) {
                numCopies << numPrevOut/numCurIn;
            } else {
                // Stream size mismatch. Stop expansion. The error will be reported later by
                // CodeValidator.
                qDebug() << "Could not clone " << IOs[i - 1].second * numCopies.back()
                         << " outputs into " << IOs[i].first << " inputs.";
                break;
            }
        } else if (numPrevOut < numCurIn) { // Need to clone all existing left side
            Q_ASSERT(numPrevOut > 0);
            if (numCurIn/(float)numPrevOut == numCurIn/numPrevOut) {
//                        int newNumCopies = numCurIn/numPrevOut;
//                        for(int i = 0; i < numCopies.size(); ++i) {
//                            numCopies[i] *= newNumCopies;
//                        }
                // Should not be expanded but connected recursively and interleaved (according to the rules of the language)
                numCopies << 1;
            } else {
                // Stream size mismatch. Stop expansion. The error will be reported later by
                // CodeValidator.
                qDebug() << "Could not clone " << IOs[i - 1].second
                         << " outputs into " << IOs[i].first << " inputs.";
                break;
            }

        } else { // Size match, no need to clone
            numCopies << 1;
        }
    }
    if (numCopies.size() == IOs.size()) { // Expansion calculation went fine, so expand
//                qDebug() << "Will expand";
        expandStreamToSizes(stream, numCopies);
    }
}

void CodeResolver::expandParallel()
{
    for (AST *node : m_tree->getChildren()) {
        QVector<AST *> scopeStack;
        if (node->getNodeType() == AST::Stream) {
            StreamNode *stream = static_cast<StreamNode *>(node);
            expandParallelStream(stream, scopeStack, m_tree);
        }
    }
}

void CodeResolver::expandStreamToSizes(StreamNode *stream, QVector<int> &size)
{
    QList<LangError> errors;
    QVector<AST *> scope;
//    int leftNumOuts = CodeValidator::getNodeNumOutputs(stream, *m_platform, scope, m_tree, errors);
    AST *left = stream->getLeft();
    int leftSize = CodeValidator::getNodeSize(left, m_tree);

    if (left->getNodeType() == AST::Name
            || left->getNodeType() == AST::Function) {
        int numCopies = size.front();
        if (leftSize < 0 && left->getNodeType() == AST::Name) {
            declareUnknownName(static_cast<NameNode *>(left), abs(numCopies), m_tree);
        }
        if (numCopies > 1) {
            ListNode *newLeft = new ListNode(left->deepCopy(), left->getFilename().data(), left->getLine());
            for (int i = 1; i < numCopies; i++) {
                newLeft->addChild(left->deepCopy());
            }
            stream->setLeft(newLeft); // This will take care of the deallocation internally
        }
    }
    size.pop_front();
    AST *right = stream->getRight();
    if (right->getNodeType() == AST::Stream) {
        expandStreamToSizes(static_cast<StreamNode *>(right), size);
    } else {
        int rightSize = CodeValidator::getNodeSize(right, m_tree);
        if (right->getNodeType() == AST::Name
                || right->getNodeType() == AST::Function) {
            int numCopies = size.front();
            if (rightSize < 0 && right->getNodeType() == AST::Name) {
                declareUnknownName(static_cast<NameNode *>(right), abs(numCopies), m_tree);
            } else if (numCopies > 1) {
                ListNode *newRight = new ListNode(right->deepCopy(), right->getFilename().data(), right->getLine());
                for (int i = 1; i < numCopies; i++) {
                    newRight->addChild(right->deepCopy());
                }
                stream->setRight(newRight); // This will take care of the deallocation internally
            }

        }
        size.pop_front();
        Q_ASSERT(size.size() == 0); // This is the end of the stream there should be no sizes left
    }
}

AST *CodeResolver::expandFunctionFromProperties(FunctionNode *func, QVector<AST *> scopeStack, AST *tree)
{
    QList<LangError> errors;
    ListNode *newFunctions = NULL;
    int dataSize = CodeValidator::getFunctionDataSize(func, scopeStack, tree, errors);
    if (dataSize > 1) {
        vector<PropertyNode *> props = func->getProperties();
        newFunctions = new ListNode(NULL, func->getFilename().c_str(), func->getLine());
        for (int i = 0; i < dataSize; ++i) { // FIXME this assumes each port takes a single input. Need to check the actual input size.
            newFunctions->addChild(func->deepCopy());
        }
        for (AST * newFunction : newFunctions->getChildren()) {
            newFunction->deleteChildren(); // Get rid of old properties. They will be put back below
        }
        for (PropertyNode *prop : props) {
            AST *value = prop->getValue();
            int numOuts = CodeValidator::getNodeNumOutputs(value, scopeStack, tree, errors);
            if (numOuts != 1 && numOuts != dataSize) {
                LangError error;
                error.type = LangError::BundleSizeMismatch;
                error.filename = func->getFilename();
                error.lineNumber = func->getLine();
                error.errorTokens.push_back(func->getName());
                error.errorTokens.push_back(QString::number(numOuts).toStdString());
                error.errorTokens.push_back(QString::number(dataSize).toStdString());
                errors << error;
                newFunctions->deleteChildren();
                delete newFunctions;
                return NULL;
            }
            if (numOuts == 1) { // Single value given, duplicate for all copies.
                for (AST * newFunction : newFunctions->getChildren()) {
                    newFunction->addChild(prop->deepCopy());
                }
            } else {
                if (value->getNodeType() == AST::Bundle) {
                    // FIXME write support for ranges

                } else if (value->getNodeType() == AST::Name) {
                    NameNode *name = static_cast<NameNode *>(value);
                    BlockNode *block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()),
                                                                      scopeStack, tree);
                    int size = CodeValidator::getBlockDeclaredSize(block, scopeStack, tree, errors);
                    Q_ASSERT(size == dataSize);
                    for (int i = 0; i < size; ++i) {
                        PropertyNode *newProp = static_cast<PropertyNode *>(prop->deepCopy());
                        ListNode *indexList = new ListNode(new ValueNode(i + 1,
                                                                         prop->getFilename().c_str(),
                                                                         prop->getLine()),
                                                           prop->getFilename().c_str(), prop->getLine());
                        BundleNode *newBundle = new BundleNode(name->getName(), indexList,
                                                               prop->getFilename().c_str(), prop->getLine());
                        newProp->replaceValue(newBundle);
                        static_cast<FunctionNode *>(newFunctions->getChildren()[i])->addChild(newProp);
                    }

                } else if (value->getNodeType() == AST::List) {
                    vector<AST *> values = static_cast<ListNode *>(value)->getChildren();
                    vector<AST *> functions = static_cast<ListNode *>(newFunctions)->getChildren();
                    Q_ASSERT(values.size() == functions.size());
                    for (int i = 0 ; i < dataSize; ++i) {
                        PropertyNode *newProp = static_cast<PropertyNode *>(prop->deepCopy());
                        newProp->replaceValue(values[i]->deepCopy());
                        static_cast<FunctionNode *>(functions[i])->addChild(newProp);
                    }
                } else {
                    qDebug() << "Error. Don't know how to expand property.";
                }
            }

        }
    }
    return newFunctions;
}

double CodeResolver::findRateInProperties(vector<PropertyNode *> properties, QVector<AST *> scope, AST *tree)
{
    for (PropertyNode *property : properties) {
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
    QList<AST *> requiredDeclarations;
    QList<AST *> bultinObjects = m_platform->getBuiltinObjects();

    // First pass to add required elements.
    // We need to add the fundamental types: "type" "platformType" "signal" "signalbridge"
    for (AST *object : bultinObjects) {
        if (object->getNodeType() == AST::Block) {
            BlockNode *block = static_cast<BlockNode *>(object);
            if (block->getObjectType() == "type") {
                AST *nameNode = block->getPropertyValue("typeName");
                if (nameNode->getNodeType() == AST::String) {
                    ValueNode *typeName = static_cast<ValueNode *>(nameNode);
                    if (typeName->getStringValue() == "type"
                            || typeName->getStringValue() == "platformType"
                            || typeName->getStringValue() == "signal"
                            || typeName->getStringValue() == "_domain"
                            || typeName->getStringValue() == "signalbridge") {
                        requiredDeclarations << object;
                    }
                }
            }
            continue;
        }
    }
    for (AST *declaration : requiredDeclarations) {
//        insertBuiltinObjectsForNode(declaration, bultinObjects);
        m_tree->addChild(declaration->deepCopy());
    }

    // Second pass to add elements that depend on the user's code
    for (AST *object : m_tree->getChildren()) {
        insertBuiltinObjectsForNode(object, bultinObjects);
    }

}

void CodeResolver::processDomains()
{
    // Fill missing domain information (propagate domains)
    // First we need to traverse the streams backwards to make sure we propagate the streams from the furthest point down the line
    vector<AST *> children = m_tree->getChildren();
    vector<AST *>::reverse_iterator rit = children.rbegin();
    QVector<AST *> scopeStack = QVector<AST*>::fromStdVector(children);
    while(rit != children.rend()) {
        AST *node = *rit;
        if (node->getNodeType() == AST::Stream) {
            resolveDomainsForStream(static_cast<StreamNode *>(node), scopeStack);
        } else if (node->getNodeType() == AST::Block) {
            BlockNode *module = static_cast<BlockNode *>(node);
            if (module->getObjectType() == "module") {
                AST *streamsNode = module->getPropertyValue("streams");
                if (streamsNode->getNodeType() == AST::Stream) {
                    ListNode *blocks = static_cast<ListNode *>(module->getPropertyValue("blocks"));
                    Q_ASSERT(blocks->getNodeType() == AST::List);
                    scopeStack = QVector<AST*>::fromStdVector(blocks->getChildren()) + scopeStack; // Prepend internal scope
                    BlockNode *domainBlock = CodeValidator::getMainOutputPortBlock(module);
                    AST *domainNode = domainBlock->getPropertyValue("domain");
                    QString domainName;
                    if (domainNode->getNodeType() == AST::Name) {
                        domainName = QString::fromStdString(static_cast<NameNode *>(domainNode)->getName());
                    } else if (domainNode->getNodeType() == AST::String) {
                        domainName = QString::fromStdString(static_cast<ValueNode *>(domainNode)->getStringValue());
                    }
                    resolveDomainsForStream(static_cast<StreamNode *>(streamsNode), scopeStack, domainName);
                } else if (streamsNode->getNodeType() == AST::List) {
                    vector<AST *> streamChild = streamsNode->getChildren();
                    vector<AST *>::reverse_iterator streamIt = streamChild.rbegin();
                    while(streamIt != streamChild.rend()) {
                        AST *streamNode = *streamIt;
                        if (streamNode->getNodeType() == AST::Stream) {
                            ListNode *blocks = static_cast<ListNode *>(module->getPropertyValue("blocks"));
                            Q_ASSERT(blocks->getNodeType() == AST::List);
                            scopeStack = QVector<AST*>::fromStdVector(blocks->getChildren()) + scopeStack; // Prepend internal scope
                            BlockNode *domainBlock = CodeValidator::getMainOutputPortBlock(module);
                            AST *domainNode = domainBlock->getPropertyValue("domain");
                            QString domainName;
                            if (domainNode->getNodeType() == AST::Name) {
                                domainName = QString::fromStdString(static_cast<NameNode *>(domainNode)->getName());
                            } else if (domainNode->getNodeType() == AST::String) {
                                domainName = QString::fromStdString(static_cast<ValueNode *>(domainNode)->getStringValue());
                            }
                            resolveDomainsForStream(static_cast<StreamNode *>(streamNode), scopeStack, domainName);
                        } else {
                            qDebug() << "ERROR: Expecting stream.";
                        }
                        streamIt++;
                    }
                }
            }
        }
        rit++;
    }

    // Now split streams when there is a domain change
    vector<AST *> new_tree;
    for (AST *node : m_tree->getChildren()) {
        if (node->getNodeType() == AST::Stream) {
            QVector<AST *> streams = sliceStreamByDomain(static_cast<StreamNode *>(node), QVector<AST *>());
            for (AST * stream: streams) {
                new_tree.push_back(stream);
            }
        } else if (node->getNodeType() == AST::Block) {
            BlockNode *module = static_cast<BlockNode *>(node);
            if (module->getObjectType() == "module") {
                AST *streamsNode = module->getPropertyValue("streams");
                AST *blocksNode = module->getPropertyValue("blocks");
                ListNode *newStreamsList = new ListNode(nullptr, "", -1);
                QVector<AST *> scopeStack;
                scopeStack << CodeValidator::getBlocksInScope(module, QVector<AST *>(), m_tree);
                if (streamsNode->getNodeType() == AST::List) {
                    for (AST * stream: streamsNode->getChildren()) {
                        if (stream->getNodeType() == AST::Stream) {
                            QVector<AST *> streams = sliceStreamByDomain(static_cast<StreamNode *>(stream), scopeStack);
                            for (AST * node: streams) {
                                if (node->getNodeType() == AST::Stream) {
                                    newStreamsList->addChild(node->deepCopy());
                                } else if (node->getNodeType() == AST::Block || node->getNodeType() == AST::BlockBundle) {
                                    blocksNode->addChild(node->deepCopy());
                                } else {
                                    qDebug() << "Stream slicing must result in streams or blocks.";
                                }
                            }
                        }
                    }
                } else if (streamsNode->getNodeType() == AST::Stream) {
                    QVector<AST *> streams = sliceStreamByDomain(static_cast<StreamNode *>(streamsNode), scopeStack);
                    for (AST * newStream: streams) {
                        newStreamsList->addChild(newStream->deepCopy());
                    }
                }
                module->replacePropertyValue("streams", newStreamsList);
            }
            new_tree.push_back(node);
        } else {
            new_tree.push_back(node);
        }
    }

    m_tree->setChildren(new_tree);
}

void CodeResolver::insertBuiltinObjectsForNode(AST *node, QList<AST *> &objects)
{
    QList<BlockNode *> blockList;
    if (node->getNodeType() == AST::List) {
        for (AST *child : node->getChildren()) {
            insertBuiltinObjectsForNode(child, objects);
        }
    } else if (node->getNodeType() == AST::Stream) {
        StreamNode *stream = static_cast<StreamNode*>(node);
        insertBuiltinObjectsForNode(stream->getLeft(), objects);
        insertBuiltinObjectsForNode(stream->getRight(), objects);
    } else if (node->getNodeType() == AST::Expression) {
        ExpressionNode *expr = static_cast<ExpressionNode*>(node);
        if (expr->isUnary()) {
            insertBuiltinObjectsForNode(expr->getValue(), objects);
        } else {
            insertBuiltinObjectsForNode(expr->getLeft(), objects);
            insertBuiltinObjectsForNode(expr->getRight(), objects);
        }
    }else if (node->getNodeType() == AST::Function) {
        for (AST *object : objects) {
            if (object->getNodeType() == AST::Block) {
                BlockNode *block = static_cast<BlockNode *>(object);
                if (block->getObjectType() == "module"
                        || block->getObjectType() == "reaction") {
                    FunctionNode *func = static_cast<FunctionNode *>(node);
                    if (block->getName() == func->getName()
                            && !blockList.contains(block)) {
//                        AST *usedObject = object->deepCopy();
////                        fillDefaultPropertiesForNode(usedObject);
//                        m_tree->addChild(usedObject);
//                        objects.removeOne(object);
//                        blockList << object;
                        blockList << block;
                        break;
                    }
                }
            }
        }
        for (BlockNode *usedBlock : blockList) {
            if (!CodeValidator::findDeclaration(QString::fromStdString(usedBlock->getName()), QVector<AST *>(), m_tree)) {
                AST *newBlock = usedBlock->deepCopy();
                m_tree->addChild(newBlock);
                insertBuiltinObjectsForNode(usedBlock, objects);
                for (PropertyNode *property : usedBlock->getProperties()) {
                    insertBuiltinObjectsForNode(property->getValue(), objects);
                }
            }
        }
    } else if (node->getNodeType() == AST::Block
               || node->getNodeType() == AST::BlockBundle) {
        QList<BlockNode *> blockList;
        BlockNode *userBlock = static_cast<BlockNode *>(node);
        // Find type declaration and insert it if needed
        for (AST *object : objects) {
            BlockNode *block = static_cast<BlockNode *>(object);
            if (object->getNodeType() == AST::Block) {
                if (block->getObjectType() == "type"
                        || block->getObjectType() == "platformType") {
                    ValueNode *typeName = static_cast<ValueNode *>(block->getPropertyValue("typeName"));
                    Q_ASSERT(typeName->getNodeType() == AST::String);
                    // Type declaration and current object type match
                    if (typeName->getStringValue() == userBlock->getObjectType()
                            && !blockList.contains(block)) {
                        blockList << block;

                        // Insert declaration for inherited types
                        QStringList inheritedTypes = CodeValidator::getInheritedTypeNames(block, QVector<AST *>::fromList(objects), m_tree);

                        for(QString typeName : inheritedTypes) {
                            QList<LangError> errors;
                            BlockNode *typeDeclaration = CodeValidator::findTypeDeclarationByName(typeName, QVector<AST *>::fromList(objects), nullptr, errors);
                            if (typeDeclaration && !blockList.contains(typeDeclaration)) {
                                blockList << typeDeclaration;
                            }
                        }
                        break; // Stop looking for matching type, type found
                    }
                }
            }
        }
        for(PropertyNode *property : userBlock->getProperties()) {
            insertBuiltinObjectsForNode(property->getValue(), objects);
        }
        for(BlockNode *usedBlock : blockList) {
            QVector<AST *> subscope;
            AST *blocks = usedBlock->getPropertyValue("blocks");
            if (blocks) {
                for(AST *blockNode : blocks->getChildren()) {
                    if (blockNode->getNodeType() == AST::Block
                            || blockNode->getNodeType() == AST::BlockBundle) {
                        subscope << blockNode;
                    }
                }
            }
            if (!CodeValidator::findDeclaration(QString::fromStdString(usedBlock->getName()), subscope, m_tree)) {
                AST *newBlock = usedBlock->deepCopy();
                m_tree->addChild(newBlock);
                objects.removeOne(usedBlock);
                //            insertBuiltinObjectsForNode(newBlock, objects);
                for(PropertyNode *property : usedBlock->getProperties()) {
                    insertBuiltinObjectsForNode(property->getValue(), objects);
                }
            }
        }
    } else if (node->getNodeType() == AST::Name) {
        QList<BlockNode *> blockList;
        for(AST *object : objects) {
            if (object->getNodeType() == AST::Block
                    || object->getNodeType() == AST::BlockBundle) {
                NameNode *name = static_cast<NameNode *>(node);
                BlockNode *block = static_cast<BlockNode *>(object);
                if (block->getName() == name->getName()
                        && !blockList.contains(block)) {
                    if (!CodeValidator::findDeclaration(QString::fromStdString(block->getName()), QVector<AST *>(), m_tree)) {
//                        AST *usedObject = object->deepCopy();
//                        fillDefaultPropertiesForNode(usedObject);
//                        m_tree->addChild(usedObject);
                        blockList << block;
                    }
                    break;

                }
            }
        }
        for(BlockNode *usedBlock : blockList) {
            AST *newBlock = usedBlock->deepCopy();
            m_tree->addChild(newBlock);
            objects.removeOne(usedBlock);
            insertBuiltinObjectsForNode(newBlock, objects);
        }
    } else if (node->getNodeType() == AST::Bundle) {
        QList<BlockNode *> blockList;
        for(AST *object : objects) {
            if (object->getNodeType() == AST::Block
                    || object->getNodeType() == AST::BlockBundle) {
                BundleNode *bundle = static_cast<BundleNode *>(node);
                BlockNode *block = static_cast<BlockNode *>(object);
                if (block->getName() == bundle->getName()
                        && !blockList.contains(block)) {
                    if (!CodeValidator::findDeclaration(QString::fromStdString(block->getName()), QVector<AST *>(), m_tree)) {
//                        AST *usedObject = object->deepCopy();
//                        fillDefaultPropertiesForNode(usedObject);
//                        m_tree->addChild(usedObject);
                        blockList << block;
                    }
                    break;

                }
            }
        }
        for(BlockNode *usedBlock : blockList) {
            AST *newBlock = usedBlock->deepCopy();
            m_tree->addChild(newBlock);
            objects.removeOne(usedBlock);
            insertBuiltinObjectsForNode(newBlock, objects);
        }
    }
}

void CodeResolver::resolveDomainsForStream(StreamNode *stream, QVector<AST *> scopeStack, QString contextDomain)
{
    AST *left = stream->getLeft();
    AST *right = stream->getRight();
    QList<AST *> domainStack;
    string previousDomainName;
    string domainName;
    while (right) {
        domainName = processDomainsForNode(left, scopeStack, domainStack);
        if (left == right && domainName.size() == 0) { // If this is is the last pass then set the unknown domains to platform domain
            // This is needed in cases where signals with no set domain are connected
            // to input ports on other objects. Domains are not
            // propagated across secondary ports
            if (contextDomain.isEmpty()) {
                domainName = m_platform->getPlatformDomain().toStdString();
            } else {
                domainName = contextDomain.toStdString();
            }
        }

        if (domainName.size() > 0 && previousDomainName != domainName) {
            setDomainForStack(domainStack, domainName, scopeStack);
            domainStack.clear();
        } else if (domainName.size() == 0) { // Domain needs to be resolved
            domainStack << left;
        }
        previousDomainName = domainName;


        if (left == right) {
            right = left = NULL; // End
        } else if(right->getNodeType() == AST::Stream) {
            stream = static_cast<StreamNode *>(right);
            left = stream->getLeft();
            right = stream->getRight();
        } else {
            left = right; // Last pass (process right, call it left)
        }
    }
}

std::string CodeResolver::processDomainsForNode(AST *node, QVector<AST *> scopeStack, QList<AST *> &domainStack)
{
    string domainName;
    resolveDomainForStreamNode(node, scopeStack);
    if (node->getNodeType() == AST::Name
            || node->getNodeType() == AST::Bundle) {
        BlockNode *declaration = CodeValidator::findDeclaration(CodeValidator::streamMemberName(node, scopeStack, m_tree), scopeStack, m_tree);
        if(declaration) {
            AST *domain = declaration->getDomain();
            if (!domain) {
                // Put declaration in stack to set domain once domain is resolved
                domainStack << declaration;
            } else {
                if (domain->getNodeType() == AST::String) {
                    domainName = static_cast<ValueNode *>(domain)->getStringValue();
                } else if (domain->getNodeType() == AST::Name) {
                    QList<LangError> errors;
                    domainName = CodeValidator::evaluateConstString(domain, scopeStack, m_tree, errors);
                } else if (domain->getNodeType() == AST::None) { // domain is streamDomain
                    domainStack << declaration;
                } else {
                    qDebug() << "WARNING: Unrecognized domain type"; // Should this trigger an error?
                    domainStack << declaration;
                }
            }
        }
    } else if (node->getNodeType() == AST::Function) {
        FunctionNode *func = static_cast<FunctionNode *>(node);
        AST *domain = func->getDomain();
        if (!domain) {
            // Put declaration in stack to set domain once domain is resolved
            domainStack << node;
        } else {
            if (domain->getNodeType() == AST::String) {
                domainName = static_cast<ValueNode *>(domain)->getStringValue();
            } else if (domain->getNodeType() == AST::None) { // domain is streamDomain
                domainStack << node;
            } else {
                qDebug() << "WARNING: Unrecognized domain type"; // Should this trigger an error?
                domainStack << node;
            }
        }
    } else if (node->getNodeType() == AST::List) {
        for (AST * member : node->getChildren()) {
            domainName = processDomainsForNode(member, scopeStack, domainStack);
            if (domainName.size() == 0) {
                // Put declaration in stack to set domain once domain is resolved
                domainStack << member;
                // FIMXE: This is very simplistic (or plain wrong....)
                // It assumes that the next found domain affects all elements
                // in the list that don't have domains. This is likely a
                // common case but list elements should inherit domains from
                // the port to which they are connected.
            }
        }
    } else if (node->getNodeType() == AST::Expression) {
        QList<AST *> expressionDomainStack;
        for(AST * member : node->getChildren()) {
            domainName = processDomainsForNode(member, scopeStack, domainStack);
            if (domainName.size() == 0) {
                // Put declaration in stack to set domain once domain is resolved
                expressionDomainStack << member;
            } else {
                setDomainForStack(expressionDomainStack, domainName, scopeStack);
            }
        }

    }
    return domainName;
}

void CodeResolver::setDomainForStack(QList<AST *> domainStack, string domainName,  QVector<AST *> scopeStack)
{
    for (AST *relatedNode : domainStack) {
        if (relatedNode->getNodeType() == AST::Block
                || relatedNode->getNodeType() == AST::BlockBundle ) {
            BlockNode *block = static_cast<BlockNode *>(relatedNode);
            block->setDomain(domainName);
        } else if (relatedNode->getNodeType() == AST::Name) {
            BlockNode *declaration = CodeValidator::findDeclaration(CodeValidator::streamMemberName(relatedNode, scopeStack, m_tree), scopeStack, m_tree);
            declaration->setDomain(domainName);
        } else if (relatedNode->getNodeType() == AST::Function) {
            FunctionNode *func = static_cast<FunctionNode *>(relatedNode);
            func->setDomain(domainName);
        }  else if (relatedNode->getNodeType() == AST::List
                    || relatedNode->getNodeType() == AST::Expression) {
            for(AST * member : relatedNode->getChildren()) {
                if (member->getNodeType() == AST::Block
                        || member->getNodeType() == AST::BlockBundle ) {
                    BlockNode *block = static_cast<BlockNode *>(member);
                    block->setDomain(domainName);
                } else if (member->getNodeType() == AST::Function) {
                    FunctionNode *func = static_cast<FunctionNode *>(member);
                    func->setDomain(domainName);
                }
            }
        }
    }
}

BlockNode *CodeResolver::createDomainDeclaration(QString name)
{
    BlockNode *newBlock = nullptr;
    newBlock = new BlockNode(name.toStdString(), "_domain", NULL, "", -1);
    newBlock->addProperty(new PropertyNode("domainName", new ValueNode(name.toStdString(), "", -1), "", -1));
    fillDefaultPropertiesForNode(newBlock);
    return newBlock;
}

BlockNode *CodeResolver::createSignalDeclaration(QString name, int size)
{
    BlockNode *newBlock = nullptr;
    Q_ASSERT(size > 0);
    if (size == 1) {
        newBlock = new BlockNode(name.toStdString(), "signal", NULL, "", -1);
    } else if (size > 1) {
        ListNode *indexList = new ListNode(new ValueNode(size, "",-1), "", -1);
        BundleNode *bundle = new BundleNode(name.toStdString(),indexList, "",-1);
        newBlock = new BlockNode(bundle, "signal", NULL, "",-1);
    }
    Q_ASSERT(newBlock);
    fillDefaultPropertiesForNode(newBlock);
    return newBlock;
}
void CodeResolver::declareUnknownName(NameNode *name, int size, AST *tree)
{
    BlockNode *block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()), QVector<AST *>(), tree);
    if (!block) { // Not declared, so make declaration
        BlockNode *newSignal = createSignalDeclaration(QString::fromStdString(name->getName()), size);
        tree->addChild(newSignal);
        double rate = getNodeRate(newSignal, QVector<AST *>(), tree);
        name->setRate(rate);
    }
}

BlockNode *CodeResolver::createConstantDeclaration(string name, AST *value)
{
    BlockNode *constant = new BlockNode(name, "constant", NULL, "", -1);
    PropertyNode *valueProperty = new PropertyNode("value", value, "", -1);
    constant->addProperty(valueProperty);
    return constant;
}

void CodeResolver::declareIfMissing(string name, AST *blocks, AST * value)
{
    BlockNode *declaration = NULL;
    if (blocks->getNodeType() == AST::List) {
        ListNode *blockList = static_cast<ListNode *>(blocks);
        // First check if block has been declared
        for(AST *block : blockList->getChildren()) {
            if (block->getNodeType() == AST::Block || block->getNodeType() == AST::BlockBundle) {
                BlockNode *declaredBlock = static_cast<BlockNode *>(block);
                if (declaredBlock->getName() == name) {
                    declaration = declaredBlock;
                    break;
                }
            }
        }
        if (!declaration) {
            declaration = createConstantDeclaration(name, value);
            fillDefaultPropertiesForNode(declaration);
            blockList->addChild(declaration);
        } else {
            delete value;
        }
    } else {
        delete value;
        qDebug() << "CodeResolver::declareIfMissing() blocks is not list";
    }
}


void CodeResolver::declareUnknownExpressionSymbols(ExpressionNode *expr, int size, AST * tree)
{
    if (expr->isUnary()) {
        if (expr->getValue()->getNodeType() == AST::Name) {
            NameNode *name = static_cast<NameNode *>(expr->getValue());
            declareUnknownName(name, size, tree);
        } else if (expr->getValue()->getNodeType() == AST::Expression) {
            ExpressionNode *name = static_cast<ExpressionNode *>(expr->getValue());
            declareUnknownExpressionSymbols(name, size, tree);
        }
    } else {
        if (expr->getLeft()->getNodeType() == AST::Name) {
            NameNode *name = static_cast<NameNode *>(expr->getLeft());
            declareUnknownName(name, size, tree);
        } else if (expr->getLeft()->getNodeType() == AST::Expression) {
            ExpressionNode *inner_expr = static_cast<ExpressionNode *>(expr->getLeft());
            declareUnknownExpressionSymbols(inner_expr, size, tree);
        }
        if (expr->getRight()->getNodeType() == AST::Name) {
            NameNode *name = static_cast<NameNode *>(expr->getRight());
            declareUnknownName(name, size, tree);
        } else if (expr->getRight()->getNodeType() == AST::Expression) {
            ExpressionNode *inner_expr = static_cast<ExpressionNode *>(expr->getRight());
            declareUnknownExpressionSymbols(inner_expr, size, tree);
        }
    }
}

ListNode *CodeResolver::expandNameToList(NameNode *name, int size)
{
    ListNode *list = new ListNode(NULL, name->getFilename().data(), name->getLine());
    for (int i = 0; i < size; i++) {
        ListNode *indexList = new ListNode(new ValueNode(i + 1, name->getFilename().data(), name->getLine()),
                                           name->getFilename().data(), name->getLine());
        BundleNode *bundle = new BundleNode(name->getName(), indexList, name->getFilename().data(), name->getLine());
        list->addChild(bundle);
    }
    return list;
}

void CodeResolver::expandNamesToBundles(StreamNode *stream, AST *tree)
{
    AST *left = stream->getLeft();
    AST *right = stream->getRight();
//    AST *nextStreamMember;
//    if (right->getNodeType() != AST::Stream) {
//        nextStreamMember = right;
//    } else {
//        nextStreamMember = static_cast<StreamNode *>(right)->getLeft();
//    }

    if (left->getNodeType() == AST::Name) {
        NameNode *name = static_cast<NameNode *>(left);
        QVector<AST *> scope;
        BlockNode *block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()), scope, tree);
        int size = 0;
        if (block) {
            if (block->getNodeType() == AST::BlockBundle) {
                QList<LangError> errors;
                size = CodeValidator::getBlockDeclaredSize(block, scope, tree, errors);
            } else if (block->getNodeType() == AST::Block ) {
                size = 1;
            }
        }
        if (size > 1) {
            ListNode *list = expandNameToList(name, size);
            stream->setLeft(list);
        }
    }
    if (right->getNodeType() == AST::Name) {
        NameNode *name = static_cast<NameNode *>(right);
        QVector<AST *> scope;
        BlockNode *block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()), scope, tree);
        int size = 0;
        if (block) {
            if (block->getNodeType() == AST::BlockBundle) {
                QList<LangError> errors;
                size = CodeValidator::getBlockDeclaredSize(block, scope, tree, errors);
            } else if (block->getNodeType() == AST::Block ) {
                size = 1;
            }
        }
        if (size > 1) {
            ListNode *list = expandNameToList(name, size);
            stream->setRight(list);
        }
    } else if (right->getNodeType() == AST::Stream) {
        expandNamesToBundles(static_cast<StreamNode *>(right), tree);
    }
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
        QList<LangError> errors;
        QVector<AST *> scope;
        int size = -1;
        if (previousStreamMember) {
            size = CodeValidator::getNodeNumOutputs(previousStreamMember, scope, m_tree, errors);
        }
        if (size <= 0) { // Look to the right if can't resolve from the left
            size = CodeValidator::getNodeNumInputs(nextStreamMember, scope, m_tree, errors);
        }
        if (size <= 0) { // None of the elements in the stream have size
            size = 1;
        }
        declareUnknownName(name, size, tree);
    } else if (left->getNodeType() == AST::Expression) {
        int size = 1; // FIXME implement size detection for expressions
        ExpressionNode *expr = static_cast<ExpressionNode *>(left);
        declareUnknownExpressionSymbols(expr, size, tree);
    }

    if (right->getNodeType() == AST::Stream) {
        declareUnknownStreamSymbols(static_cast<StreamNode *>(right), left, tree);
    } else if (right->getNodeType() == AST::Name) {
        NameNode *name = static_cast<NameNode *>(right);
        QList<LangError> errors;
        QVector<AST *> scope;
        int size = CodeValidator::getNodeNumOutputs(left, scope, m_tree, errors);
        if (size <= 0) { // None of the elements in the stream have size
            size = 1;
        }
        declareUnknownName(name, size, tree);
    } else if (left->getNodeType() == AST::Expression) {
        int size = 1; // FIXME implement size detection for expressions
        ExpressionNode *expr = static_cast<ExpressionNode *>(left);
        declareUnknownExpressionSymbols(expr, size, tree);
    }
}

void CodeResolver::resolveStreamSymbols()
{
    QVector<AST *> children = QVector<AST *>::fromStdVector(m_tree->getChildren());
    for(AST *node : children) {
        if(node->getNodeType() == AST:: Stream) {
            StreamNode *stream = static_cast<StreamNode *>(node);
            declareUnknownStreamSymbols(stream, NULL, m_tree); // FIXME Is this already done in expandParallelFunctions?
            expandNamesToBundles(stream, m_tree);
        }
    }
}

void CodeResolver::resolveConstants()
{
    QVector<AST *> children = QVector<AST *>::fromStdVector(m_tree->getChildren());
    for(AST *node : children) {
        resolveConstantsInNode(node, children);
    }
}

ValueNode *CodeResolver::reduceConstExpression(ExpressionNode *expr, QVector<AST *> scope, AST *tree)
{
    AST *left = nullptr, *right = nullptr;
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
        case ExpressionNode::Equal:
            result = equal(static_cast<ValueNode *>(left), static_cast<ValueNode *>(right));
            break;
        case ExpressionNode::NotEqual:
            result = notEqual(static_cast<ValueNode *>(left), static_cast<ValueNode *>(right));
            break;
        case ExpressionNode::Greater:
            result = greaterThan(static_cast<ValueNode *>(left), static_cast<ValueNode *>(right));
            break;
        case ExpressionNode::Lesser:
            result = lesser(static_cast<ValueNode *>(left), static_cast<ValueNode *>(right));
            break;
        case ExpressionNode::GreaterEqual:
            result = greaterEqual(static_cast<ValueNode *>(left), static_cast<ValueNode *>(right));
            break;
        case ExpressionNode::LesserEqual:
            result = lesserEqual(static_cast<ValueNode *>(left), static_cast<ValueNode *>(right));
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
        BlockNode *block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()), scope, m_tree);
        if (block && block->getNodeType() == AST::Block && block->getObjectType() == "constant") { // Size == 1
            AST *blockValue = block->getPropertyValue("value");
            if (blockValue->getNodeType() == AST::Int || blockValue->getNodeType() == AST::Real
                     || blockValue->getNodeType() == AST::String ) {
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
        if (stream->getLeft()->getNodeType() == AST::Expression) {
            ExpressionNode *expr = static_cast<ExpressionNode *>(stream->getLeft());
            ValueNode *newValue = reduceConstExpression(expr, scope, m_tree);
            if (newValue) {
                stream->setLeft(newValue);
            }
        }
        resolveConstantsInNode(stream->getRight(), scope);
    } else if (node->getNodeType() == AST::Function) {
        FunctionNode *func = static_cast<FunctionNode *>(node);
        vector<PropertyNode *> properties = func->getProperties();
        for(PropertyNode *property : properties) {
            ValueNode *newValue = resolveConstant(property->getValue(), scope);
            if (newValue) {
                property->replaceValue(newValue);
            }
        }
    } else if(node->getNodeType() == AST::Block) {
        BlockNode *block = static_cast<BlockNode *>(node);
        vector<PropertyNode *> properties = block->getProperties();
        ListNode *internalBlocks = static_cast<ListNode *>(block->getPropertyValue("blocks"));
        if (internalBlocks) {
            if (internalBlocks->getNodeType() == AST::List) {
                scope << QVector<AST *>::fromStdVector(internalBlocks->getChildren());
            }
        }
        for(PropertyNode *property : properties) {
            resolveConstantsInNode(property->getValue(), scope);
            ValueNode *newValue = resolveConstant(property->getValue(), scope);
            if (newValue) {
                property->replaceValue(newValue);
            }
        }
    } else if(node->getNodeType() == AST::BlockBundle) {
        BlockNode *block = static_cast<BlockNode *>(node);
        vector<PropertyNode *> properties = block->getProperties();
        ListNode *internalBlocks = static_cast<ListNode *>(block->getPropertyValue("blocks"));
        if (internalBlocks) {
            if (internalBlocks->getNodeType() == AST::List) {
                scope << QVector<AST *>::fromStdVector(internalBlocks->getChildren());
            }
        }
        for (PropertyNode *property : properties) {
            resolveConstantsInNode(property->getValue(), scope);
            ValueNode *newValue = resolveConstant(property->getValue(), scope);
            if (newValue) {
                property->replaceValue(newValue);
            }
        }
        BundleNode *bundle = block->getBundle();
        ListNode *indexList = bundle->index();
        vector<AST *> elements = indexList->getChildren();
        for (AST *element : elements) {
            if (element->getNodeType() == AST::Expression) {
                ExpressionNode *expr = static_cast<ExpressionNode *>(element);
                resolveConstantsInNode(expr, scope);
                ValueNode *newValue = reduceConstExpression(expr, scope, m_tree);
                if (newValue) {
                    indexList->replaceMember(newValue, element);
//                    element->deleteChildren();
//                    delete element;
                }
            } else if (element->getNodeType() == AST::Name) {
//                NameNode *name = static_cast<NameNode *>(element);
                ValueNode *newValue = resolveConstant(element, scope);
                if (newValue) {
                    indexList->replaceMember(newValue, element);
//                    element->deleteChildren();
//                    delete element;
                }
            }
        }
    } else if(node->getNodeType() == AST::Expression) {
        ExpressionNode *expr = static_cast<ExpressionNode *>(node);
        if (expr->isUnary()) {
            resolveConstantsInNode(expr->getValue(), scope);
            if (expr->getValue()->getNodeType() == AST::Expression) {
                ExpressionNode *exprValue = static_cast<ExpressionNode *>(expr->getValue());
                ValueNode *newValue = reduceConstExpression(exprValue, scope, m_tree);
                if (newValue) {
                    exprValue->replaceValue(newValue);
                }
            }
        } else {
            resolveConstantsInNode(expr->getLeft(), scope);
            resolveConstantsInNode(expr->getRight(), scope);
            if (expr->getLeft()->getNodeType() == AST::Expression) {
                ExpressionNode *exprValue = static_cast<ExpressionNode *>(expr->getLeft());
                ValueNode *newValue = reduceConstExpression(exprValue, scope, m_tree);
                if (newValue) {
                    expr->replaceLeft(newValue);
                }
            }
            if (expr->getRight()->getNodeType() == AST::Expression) {
                ExpressionNode *exprValue = static_cast<ExpressionNode *>(expr->getRight());
                ValueNode *newValue = reduceConstExpression(exprValue, scope, m_tree);
                if (newValue) {
                    expr->replaceRight(newValue);
                }
            }
        }
    } else if(node->getNodeType() == AST::List) {
        for (AST *element : node->getChildren()) {
            resolveConstantsInNode(element, scope);
        }
    }
}

double CodeResolver::getDefaultForTypeAsDouble(QString type, QString port)
{
    double outValue = 0.0;
    AST *value = getDefaultPortValueForType(type, port);
    QList<LangError> errors;
    if (value) {
        outValue = CodeValidator::evaluateConstReal(value, QVector<AST *>(), m_tree, errors);
    }
    return outValue;
}

AST *CodeResolver::getDefaultPortValueForType(QString type, QString portName)
{
    QVector<AST *> ports = CodeValidator::getPortsForType(type, QVector<AST *>(), m_tree);
    if (!ports.isEmpty()) {
        for (AST *port : ports) {
            BlockNode *block = static_cast<BlockNode *>(port);
            Q_ASSERT(block->getNodeType() == AST::Block);
            Q_ASSERT(block->getObjectType() == "typeProperty");
            AST *platPortNameNode = block->getPropertyValue("name");
            ValueNode *platPortName = static_cast<ValueNode *>(platPortNameNode);
            Q_ASSERT(platPortName->getNodeType() == AST::String);
            if (platPortName->getStringValue() == portName.toStdString()) {
                AST *platPortDefault = block->getPropertyValue("default");
                if (platPortDefault) {
                    return platPortDefault;
                }
            }
        }
    }
    return NULL;
}

ValueNode *CodeResolver::multiply(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() * right->getIntValue(), left->getFilename().data(),left->getLine());
    } else { // Automatic casting from int to real
        Q_ASSERT((left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Int)
                 || (left->getNodeType() == AST::Int &&  right->getNodeType() == AST::Real)
                 || (left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Real));
        return new ValueNode(left->toReal() * right->toReal(), left->getFilename().data(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::divide(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() / right->getIntValue(), left->getFilename().data(), left->getLine());
    } else { // Automatic casting from int to real
        Q_ASSERT((left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Int)
                 || (left->getNodeType() == AST::Int &&  right->getNodeType() == AST::Real)
                 || (left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Real));
        return new ValueNode(left->toReal() / right->toReal(), left->getFilename().data(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::add(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() + right->getIntValue(), left->getFilename().data(), left->getLine());
    } else { // Automatic casting from int to real
        Q_ASSERT((left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Int)
                 || (left->getNodeType() == AST::Int &&  right->getNodeType() == AST::Real)
                 || (left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Real));
        return new ValueNode(left->toReal() + right->toReal(), left->getFilename().data(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::subtract(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() - right->getIntValue(), left->getFilename().data(), left->getLine());
    } else { // Automatic casting from int to real
        Q_ASSERT((left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Int)
                 || (left->getNodeType() == AST::Int &&  right->getNodeType() == AST::Real)
                 || (left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Real));
        return new ValueNode(left->toReal() - right->toReal(), left->getFilename().data(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::unaryMinus(ValueNode *value)
{
    if (value->getNodeType() == AST::Int) {
        return new ValueNode(- value->getIntValue(), value->getFilename().data(), value->getLine());
    } else if (value->getNodeType() == AST::Real){
        return new ValueNode(- value->getRealValue(), value->getFilename().data(), value->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::logicalAnd(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() & right->getIntValue(), left->getFilename().data(), left->getLine());
    } else if (left->getNodeType() == AST::Switch && right->getNodeType() == AST::Switch) {
        return new ValueNode(left->getSwitchValue() == right->getSwitchValue(), left->getFilename().data(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::logicalOr(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() | right->getIntValue(), left->getFilename().data(), left->getLine());
    } else if (left->getNodeType() == AST::Switch && right->getNodeType() == AST::Switch) {
        return new ValueNode(left->getSwitchValue() == right->getSwitchValue(), left->getFilename().data(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::logicalNot(ValueNode *value)
{
    if (value->getNodeType() == AST::Int) {
        return new ValueNode(~ (value->getIntValue()), value->getFilename().data(), value->getLine());
    } else if (value->getNodeType() == AST::Switch) {
        return new ValueNode(!value->getSwitchValue(), value->getFilename().data(), value->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::equal(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() == right->getIntValue(), left->getFilename().data(), left->getLine());
    } else if ((left->getNodeType() == AST::Int || left->getNodeType() == AST::Real) &&
               (right->getNodeType() == AST::Int || right->getNodeType() == AST::Real)) {
        return new ValueNode(left->toReal() == right->toReal(), left->getFilename().data(), left->getLine());
    } else if (left->getNodeType() == AST::Switch && right->getNodeType() == AST::Switch) {
        return new ValueNode(left->getSwitchValue() == right->getSwitchValue(), left->getFilename().data(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::notEqual(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() != right->getIntValue(), left->getFilename().data(), left->getLine());
    } else if ((left->getNodeType() == AST::Int || left->getNodeType() == AST::Real) &&
               (right->getNodeType() == AST::Int || right->getNodeType() == AST::Real)) {
        return new ValueNode(left->toReal() != right->toReal(), left->getFilename().data(), left->getLine());
    } else if (left->getNodeType() == AST::Switch && right->getNodeType() == AST::Switch) {
        return new ValueNode(left->getSwitchValue() != right->getSwitchValue(), left->getFilename().data(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::greaterThan(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() > right->getIntValue(), left->getFilename().data(), left->getLine());
    } else if ((left->getNodeType() == AST::Int || left->getNodeType() == AST::Real) &&
               (right->getNodeType() == AST::Int || right->getNodeType() == AST::Real)) {
        return new ValueNode(left->toReal() > right->toReal(), left->getFilename().data(), left->getLine());
    } else if (left->getNodeType() == AST::Switch && right->getNodeType() == AST::Switch) {
        return new ValueNode(left->getSwitchValue() > right->getSwitchValue(), left->getFilename().data(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::lesser(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() < right->getIntValue(), left->getFilename().data(), left->getLine());
    } else if ((left->getNodeType() == AST::Int || left->getNodeType() == AST::Real) &&
               (right->getNodeType() == AST::Int || right->getNodeType() == AST::Real)) {
        return new ValueNode(left->toReal() < right->toReal(), left->getFilename().data(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::greaterEqual(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() >= right->getIntValue(), left->getFilename().data(), left->getLine());
    } else if ((left->getNodeType() == AST::Int || left->getNodeType() == AST::Real) &&
               (right->getNodeType() == AST::Int || right->getNodeType() == AST::Real)) {
        return new ValueNode(left->toReal() >= right->toReal(), left->getFilename().data(), left->getLine());
    }
    return NULL;
}

ValueNode *CodeResolver::lesserEqual(ValueNode *left, ValueNode *right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return new ValueNode(left->getIntValue() <= right->getIntValue(), left->getFilename().data(), left->getLine());
    } else if ((left->getNodeType() == AST::Int || left->getNodeType() == AST::Real) &&
               (right->getNodeType() == AST::Int || right->getNodeType() == AST::Real)) {
        return new ValueNode(left->toReal() <= right->toReal(), left->getFilename().data(), left->getLine());
    }
    return NULL;
}


//QVector<AST *> CodeResolver::expandStream(StreamNode *stream)
//{
//    QList<LangError> errors;
//    QVector<AST *> scope;
//    int size = CodeValidator::numParallelStreams(stream, m_platform, scope, m_tree, errors);
//    QVector<AST *> streams;
//    if (size <= 1) { // Just copy the existing stream, no need to expand
//        streams << stream->deepCopy();
//        return streams;
//    }
//    streams << stream->deepCopy();
//    return streams;

//    //    QVector<AST *> slicedStreams;
//    //    return slicedStreams;


//}
//QVector<AST *> CodeResolver::expandStreamNode(StreamNode *stream)
//{

//}

QVector<AST *> CodeResolver::sliceStreamByDomain(StreamNode *stream, QVector<AST *> scopeStack)
{
    QVector<AST *> streams;
    QVector<AST *> stack;

    AST *left = stream->getLeft();
    AST *right = stream->getRight();
    std::string domainName = CodeValidator::getNodeDomainName(left, CodeValidator::getBlocksInScope(left, scopeStack, m_tree), m_tree);
    std::string previousDomainName = CodeValidator::getNodeDomainName(left, CodeValidator::getBlocksInScope(left, scopeStack, m_tree), m_tree);
    while (right) {
        if (left == right) { // If this is is the last pass then make last slice
            StreamNode *newStream;
            AST *lastNode = stack.back();
            stack.pop_back();
            newStream = new StreamNode(lastNode, left, lastNode->getFilename().c_str(), lastNode->getLine());
            while (stack.size() > 0) {
                lastNode = stack.back();
                newStream = new StreamNode(lastNode, newStream, lastNode->getFilename().c_str(), lastNode->getLine());
                stack.pop_back();
            }
            streams << newStream;
            right = left = NULL; // End
            continue;
        }
        domainName = CodeValidator::getNodeDomainName(left, CodeValidator::getBlocksInScope(left, scopeStack, m_tree), m_tree);

        if (previousDomainName != domainName && left != stream->getLeft()) { // domain change and not the first node in the stream
            int size = CodeValidator::getNodeSize(left, m_tree);
            // FIXME make sure connectorName is not declared in current scope
            std::string connectorName = "_BridgeSig_" + std::to_string(m_connectorCounter++);
            AST *closingName = nullptr;
            StreamNode *newStream = nullptr;
            AST *newStart = nullptr;
            vector<AST *> newDeclarations;
            if (stack.size() > 0 &&  stack.back()->getNodeType() == AST::List) {
                closingName = new ListNode(nullptr, "", -1);
                newStart = new ListNode(nullptr, "", -1);
                for (unsigned int i = 0; i < stack.back()->getChildren().size(); i++) {
                    string listMemberName = connectorName + "_" + std::to_string(i);
                    newDeclarations.push_back(new BlockNode(listMemberName, "signalbridge", nullptr, "", -1));
                    closingName->addChild(new NameNode(listMemberName, "", -1));
                    newStart->addChild(new NameNode(listMemberName, "", -1));
                }
                // Slice
            } else {
                if (size == 1) {
                     newDeclarations.push_back(new BlockNode(connectorName, "signalbridge", nullptr, "", -1));
                } else {
                     newDeclarations.push_back(new BlockNode(
                                                   new BundleNode(connectorName, new ListNode(new ValueNode(size, "", -1), "", -1), "", -1),
                                                   "signalbridge", nullptr, "", -1));
                }
                closingName = new NameNode(connectorName, "", -1);
                newStart = new NameNode(connectorName, "", -1);
            }
            if (stack.size() == 0) {
                newStream = new StreamNode(closingName, left, left->getFilename().c_str(), left->getLine());
            } else {
                newStream = new StreamNode(stack.back(), closingName, left->getFilename().c_str(), left->getLine());
                stack.pop_back();
            }
            while (stack.size() > 0) {
                AST *lastNode = stack.back();
                newStream = new StreamNode(lastNode, newStream, lastNode->getFilename().c_str(), lastNode->getLine());
                stack.pop_back();
            }
            for (AST *declaration: newDeclarations) {
                streams << declaration;
            }
            streams << newStream;
            stack << newStart << left;
        } else {
            stack << left;
        }
        previousDomainName = domainName;

        if(right->getNodeType() == AST::Stream) {
            StreamNode *subStream = static_cast<StreamNode *>(right);
            left = subStream->getLeft();
            right = subStream->getRight();
//            delete stream;
        } else {
            left = right; // Last pass (process right, call it left)
        }
    }
//    delete stream;
    return streams;
}

//StreamNode *CodeResolver::splitStream(StreamNode *stream, AST *closingNode, AST *endNode)
//{
//    StreamNode *outStream;
//    AST * left = stream->getLeft();
//    AST * right = stream->getRight();
//    if (right == endNode) { // endNode is the last node in the stream, just append closingNode
//        StreamNode *lastStream = new StreamNode(right->deepCopy(), closingNode, "", -1);
//        outStream = new StreamNode(left->deepCopy(), lastStream, "", -1);
//        return outStream;
//    } else if (left == endNode) { // There is more stream, but we have reached the split point
//        StreamNode *outStream = new StreamNode(left->deepCopy(), closingNode, "", -1);
//        return outStream;
//    }
//    if  (right != endNode) {
//        Q_ASSERT(right->getNodeType() == AST::Stream); // Shouldn't reach the end of the stream before reaching endNode
//        outStream = splitStream(static_cast<StreamNode *>(right), closingNode, endNode);
//        StreamNode *finalStream = new StreamNode(left, outStream->deepCopy(), "", -1);
//        outStream->deleteChildren();
//        delete outStream;
//        return finalStream;
//    }
//    qFatal("Shouldn't get here");
//}

void CodeResolver::resolveDomainForStreamNode(AST *node, QVector<AST *> scopeStack)
{
    AST *domain = NULL;
    if (node->getNodeType() == AST::Name
            || node->getNodeType() == AST::Bundle) {
        BlockNode *declaration = CodeValidator::findDeclaration(CodeValidator::streamMemberName(node, scopeStack, m_tree), scopeStack, m_tree);
        if (declaration) {
            domain = static_cast<BlockNode *>(declaration)->getDomain();
        }
    } else if (node->getNodeType() == AST::Function) {
        domain = static_cast<FunctionNode *>(node)->getDomain();
    } else if (node->getNodeType() == AST::List) {
        for (AST *member : node->getChildren()) {
            resolveDomainForStreamNode(member, scopeStack);
        }
        return;
    } else if (node->getNodeType() == AST::Expression) {
        ExpressionNode *expr = static_cast<ExpressionNode *>(node);
        if (expr->isUnary()) {
            resolveDomainForStreamNode(expr->getValue(), scopeStack);
        } else {
            resolveDomainForStreamNode(expr->getLeft(), scopeStack);
            resolveDomainForStreamNode(expr->getRight(), scopeStack);
        }
        return;
    }
    if (domain) {
        if (domain->getNodeType() == AST::Name) { // Resolve domain name
            NameNode *domainNameNode = static_cast<NameNode *>(domain);
            BlockNode *domainDeclaration = CodeValidator::findDeclaration(
                        QString::fromStdString(domainNameNode->getName()), scopeStack, m_tree);
            if (domainDeclaration) {
                AST *domainValue = domainDeclaration->getPropertyValue("domainName");
                while (domainValue && domainValue->getNodeType() == AST::Name) {
                    NameNode *recurseDomain = static_cast<NameNode *>(domainValue);
                    domainDeclaration = CodeValidator::findDeclaration(
                                QString::fromStdString(recurseDomain->getName()), scopeStack, m_tree);
                    domainValue = domainDeclaration->getPropertyValue("name");
                }
                if (domainValue && domainValue->getNodeType() == AST::String) {
                    string domainName = static_cast<ValueNode *>(domainValue)->getStringValue();
                    if (node->getNodeType() == AST::Name
                            || node->getNodeType() == AST::Bundle) {
                        BlockNode *declaration = CodeValidator::findDeclaration(CodeValidator::streamMemberName(node, scopeStack, m_tree), scopeStack, m_tree);
                        declaration->setDomain(domainName);
                    } else if (node->getNodeType() == AST::Function) {
                         static_cast<FunctionNode *>(node)->setDomain(domainName);
                    }
                }
            }
        }
    }
}

//AST *CodeResolver::expandStream(AST *node, int index, int rightNumInputs, int leftNumOutputs)
//{
//    AST *newNode;
//    QList<LangError> errors;
//    QVector<AST *> scope;
//    if (node->getNodeType() == AST::Function) {
//        FunctionNode *func = static_cast<FunctionNode *>(node);
//        int numOutputs = CodeValidator::getNodeNumOutputs(node, m_platform, scope, m_tree, errors);
//        if (numOutputs != rightNumInputs) {
//            int numInstances = (numOutputs == 0? rightNumInputs:(int) (rightNumInputs/ numOutputs));
//            func->setParallelInstances(numInstances);
//        }
//        func->setParallelInstances(func->getParallelInstances() + 1);
//        newNode = static_cast<FunctionNode *>(node)->deepCopy();
//    } else if (node->getNodeType() == AST::Name) {
//        NameNode *nameNode = static_cast<NameNode *>(node);
//        if (rightNumInputs == 1) {
//            int numOutputs = CodeValidator::getNodeNumOutputs(node, m_platform, scope, m_tree, errors);
//            Q_ASSERT(numOutputs > 0);
//            if (numOutputs == 1) {
//                newNode = node->deepCopy();
//            } else {
//                ListNode *indexList = new ListNode(new ValueNode(index + 1, nameNode->getLine()), -1);
//                newNode = new BundleNode(nameNode->getName(), indexList ,nameNode->getLine());
//                newNode->setRate(node->getRate());
//            }
//        } else {
//            int startIndex = index * leftNumOutputs;
//            int endIndex = (index + 1) * leftNumOutputs ;
//            ValueNode *startIndexNode = new ValueNode(startIndex + 1, nameNode->getLine());
//            ValueNode *endIndexNode = new ValueNode(endIndex + 1, nameNode->getLine());
//            RangeNode *range = new RangeNode(startIndexNode, endIndexNode, nameNode->getLine());
//            ListNode *list = new ListNode(range, nameNode->getLine());
//            newNode = new BundleNode(nameNode->getName(), list, nameNode->getLine());
//            newNode->setRate(node->getRate());
//        }
//    } else if (node->getNodeType() == AST::Stream) {
//        QList<LangError> errors;
//        QVector<AST *> scope;
//        StreamNode *streamNode = static_cast<StreamNode *>(node);
//        int rightNumInputsStream = CodeValidator::getNodeNumInputs(streamNode->getRight(), m_platform, scope, m_tree, errors);
//        int leftNumOutputsStream = CodeValidator::getNodeNumOutputs(streamNode->getLeft(), m_platform, scope, m_tree, errors);
//        AST * newRight = expandStream(streamNode->getRight(), index, 1, leftNumOutputsStream);
//        Q_ASSERT(rightNumInputsStream > 0);
//        AST * newLeft = expandStream(streamNode->getLeft(), index, rightNumInputsStream, leftNumOutputs);
//        newNode = new StreamNode(newLeft, newRight, streamNode->getLine());
//        newNode->setRate(node->getRate());
//    } else if (node->getNodeType() == AST::Bundle) {
//        newNode = node->deepCopy(); // TODO must check how many connections the type supports
//    } else if (node->getNodeType() == AST::List) {
//        QList<LangError> errors;
//        CodeValidator::getMemberFromList(static_cast<ListNode *>(node),
//                                         index + 1, errors);
//        Q_ASSERT(errors.size() == 0);
//    } else {
//        qFatal("Node type not supported in CodeResolver::expandStreamMember");
//    }
//    return newNode;
//}

