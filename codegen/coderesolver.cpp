/*
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
*/

#include <QDebug>

#include "coderesolver.h"
#include "codevalidator.h"

CodeResolver::CodeResolver(std::shared_ptr<StrideSystem> system, ASTNode tree,
                           SystemConfiguration systemConfig) :
    m_system(system), m_systemConfig(systemConfig), m_tree(tree), m_connectorCounter(0)
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
//    processResets();
    resolveStreamSymbols();
    processDomains();
    resolveRates();
    analyzeConnections();
    processSystem();
    storeDeclarations();
}

void CodeResolver::processSystem()
{
    for (ASTNode node: m_tree->getChildren()) {
        if (node->getNodeType() == AST::Platform) {
            SystemNode *platform = static_cast<SystemNode *>(node.get());
//            std::string systemFile = platform->platformName();
//            AST::parseFile()
        }
    }
}

void CodeResolver::resolveRates()
{
    vector<ASTNode> children = m_tree->getChildren();
    // First go through backwards to prioritize pull
    vector<ASTNode>::reverse_iterator rit = children.rbegin();
    while(rit != children.rend()) {
        ASTNode node = *rit;
        if (node->getNodeType() == AST::Stream) {
            resolveStreamRatesReverse(static_pointer_cast<StreamNode>(node));
        }
        rit++;
    }
    // Then do it again from the top to try to resolve the rest
    for(ASTNode node: children) {
        if (node->getNodeType() == AST::Stream) {
            resolveStreamRates(static_pointer_cast<StreamNode>(node));
        }
    }
}

void CodeResolver::resolveStreamRatesReverse(std::shared_ptr<StreamNode> stream)
{
    ASTNode left = stream->getLeft();
    ASTNode right = stream->getRight();
    double rate = CodeValidator::getNodeRate(left, QVector<ASTNode>(), m_tree);
    double rightRate = -1;
    if (right->getNodeType() == AST::Stream) {
        resolveStreamRatesReverse(static_pointer_cast<StreamNode>(right));
        rightRate = CodeValidator::getNodeRate(static_cast<StreamNode *>(right.get())->getLeft(), QVector<ASTNode>(), m_tree);
    } else {
        rightRate = CodeValidator::getNodeRate(right, QVector<ASTNode>(), m_tree);
    }
    if (rate < 0 && rightRate >= 0) {
        CodeValidator::setNodeRate(left, rightRate, QVector<ASTNode>(), m_tree);
    }
//    Q_ASSERT(rate != -1);
    //    stream->setRate(rate);
}

void CodeResolver::resolveStreamRates(std::shared_ptr<StreamNode> stream)
{
    ASTNode left = stream->getLeft();
    ASTNode right = stream->getRight();
    double rate = CodeValidator::getNodeRate(left, QVector<ASTNode>(), m_tree);
    if (rate < 0) { // Force node rate to platform rate
        std::shared_ptr<DeclarationNode> domainDeclaration = CodeValidator::findDomainDeclaration(
                    CodeValidator::streamMemberName(m_system->getPlatformDomain()), m_tree);
        if (domainDeclaration) {
            ASTNode rateValue = domainDeclaration->getPropertyValue("rate");
            if (rateValue->getNodeType() == AST::Int
                    || rateValue->getNodeType() == AST::Real) {
                double rate = static_cast<ValueNode *>(rateValue.get())->toReal();
                CodeValidator::setNodeRate(left, rate, QVector<ASTNode>(), m_tree);
            } else  if (rateValue->getNodeType() == AST::PortProperty) {
                if (left->getNodeType() == AST::Declaration) {
                    auto decl = static_pointer_cast<DeclarationNode>(left);
                    decl->replacePropertyValue("rate", rateValue->deepCopy());
                }
            } else {
                qDebug() << "Unexpected type for rate in domain declaration: "
                         << QString::fromStdString(CodeValidator::streamMemberName(m_system->getPlatformDomain()));
            }
        }
    }
    double rightRate = -1;
    if (right->getNodeType() == AST::Stream) {
        rightRate = CodeValidator::getNodeRate(static_cast<StreamNode *>(right.get())->getLeft(), QVector<ASTNode>(), m_tree);
        if (rightRate <= 0 && rate >= 0) {
            CodeValidator::setNodeRate(static_cast<StreamNode *>(right.get())->getLeft(), rate, QVector<ASTNode>(), m_tree);
        }
        resolveStreamRates(static_pointer_cast<StreamNode>(right));
    } else {
        rightRate = CodeValidator::getNodeRate(right, QVector<ASTNode>(), m_tree);
        if (rightRate <= 0 && rate >= 0) {
            CodeValidator::setNodeRate(right, rate, QVector<ASTNode>(), m_tree);
        }
    }
}

void CodeResolver::fillDefaultPropertiesForNode(ASTNode node)
{
    if (node->getNodeType() == AST::Declaration || node->getNodeType() == AST::BundleDeclaration) {
        std::shared_ptr<DeclarationNode> destBlock = static_pointer_cast<DeclarationNode>(node);
        vector<std::shared_ptr<PropertyNode>> blockProperties = destBlock->getProperties();
        QVector<ASTNode> typeProperties = CodeValidator::getPortsForType(
                    destBlock->getObjectType(),
                    QVector<ASTNode>(), m_tree, destBlock->getNamespaceList());
        if (typeProperties.isEmpty()) {
            qDebug() << "ERROR: fillDefaultProperties() No type definition for " << QString::fromStdString(destBlock->getObjectType());
            return;
        }
        for(std::shared_ptr<PropertyNode> property : blockProperties) {
            fillDefaultPropertiesForNode(property->getValue());
        }

        for(ASTNode propertyListMember : typeProperties) {
            Q_ASSERT(propertyListMember->getNodeType() == AST::Declaration);
            DeclarationNode *portDescription = static_cast<DeclarationNode *>(propertyListMember.get());
            ASTNode propName = portDescription->getPropertyValue("name");
            Q_ASSERT(propName->getNodeType() == AST::String);
            string propertyName = static_cast<ValueNode *>(propName.get())->getStringValue();
            bool propertySet = false;
            for(std::shared_ptr<PropertyNode> blockProperty : blockProperties) {
                if (blockProperty->getName() == propertyName) {
                    propertySet = true;
                    break;
                }
            }
            if (!propertySet) {
                ASTNode defaultValueNode = portDescription->getPropertyValue("default");
                std::shared_ptr<PropertyNode> newProperty = std::make_shared<PropertyNode>(propertyName,
                            defaultValueNode->deepCopy(),
                            portDescription->getFilename().data(), portDescription->getLine());
                destBlock->addProperty(newProperty);
            }
        }
    } else if (node->getNodeType() == AST::Function) {
        std::shared_ptr<FunctionNode> destFunc = static_pointer_cast<FunctionNode>(node);
        vector<std::shared_ptr<PropertyNode>> blockProperties = destFunc->getProperties();
        std::shared_ptr<DeclarationNode> functionModule = CodeValidator::findDeclaration(
                    QString::fromStdString(destFunc->getName()),
                    QVector<ASTNode >(), m_tree);
        if (functionModule) {
            vector<ASTNode> typeProperties = functionModule->getPropertyValue("ports")->getChildren();
            if (typeProperties.size() < 1) {
                qDebug() << "ERROR: fillDefaultProperties() No type definition for " << QString::fromStdString(destFunc->getName());
                return;
            }
            for (std::shared_ptr<PropertyNode> property : blockProperties) {
                fillDefaultPropertiesForNode(property->getValue());
            }

            for(ASTNode propertyListMember : typeProperties) {
                Q_ASSERT(propertyListMember->getNodeType() == AST::Declaration);
                DeclarationNode *propertyDecl = static_cast<DeclarationNode *>(propertyListMember.get());

                if (propertyDecl->getObjectType().substr(0,8) == "property") {
                    auto propertyNameValue = propertyDecl->getPropertyValue("name");
                    if (propertyNameValue->getNodeType() == AST::String) {
                        bool propertySet = false;
                        string propertyName = static_pointer_cast<ValueNode>(propertyNameValue)->getStringValue();
                        for(std::shared_ptr<PropertyNode> blockProperty : blockProperties) {
                            if (blockProperty->getName() == propertyName) {
                                propertySet = true;
                                break;
                            }
                        }
                        if (!propertySet) {
                            ASTNode defaultValueNode = propertyDecl->getPropertyValue("default");
                            if (defaultValueNode) {
                                std::shared_ptr<PropertyNode> newProperty
                                        = std::make_shared<PropertyNode>(propertyName,
                                                                         defaultValueNode,
                                                                         propertyDecl->getFilename().data(), propertyDecl->getLine());
                                destFunc->addProperty(newProperty);
                            }
                        }
                    }
                }

                }
        }
    } else if (node->getNodeType() == AST::List) {
        ListNode *list = static_cast<ListNode *>(node.get());
        for(ASTNode listElement : list->getChildren()) {
            fillDefaultPropertiesForNode(listElement);
        }
    } else if (node->getNodeType() == AST::Stream) {
        StreamNode *stream = static_cast<StreamNode *>(node.get());
        for(ASTNode streamElement : stream->getChildren()) {
            fillDefaultPropertiesForNode(streamElement);
        }
    }
}

void CodeResolver::analyzeChildConnections(ASTNode node, QVector<ASTNode > scopeStack)
{
    for (ASTNode object : node->getChildren()) {
//        We need to check streams on the root but also streams within modules and reactions
        if (object->getNodeType() == AST::Declaration) {
            std::shared_ptr<DeclarationNode> block = static_pointer_cast<DeclarationNode>(object);
            if (block->getObjectType() == "module" || block->getObjectType() == "reaction" || block->getObjectType() == "loop") {
                std::vector<ASTNode> streams = getModuleStreams(block);
                ASTNode blocks = block->getPropertyValue("blocks");
                QVector<ASTNode > moduleScope;
                for (ASTNode block: blocks->getChildren()) {
                    moduleScope.push_back(block);
                }
                for (ASTNode stream: streams) {
                    Q_ASSERT(stream->getNodeType() == AST::Stream);
                    if (stream->getNodeType() == AST::Stream) {
                        checkStreamConnections(static_pointer_cast<StreamNode>(stream), moduleScope + scopeStack);
                    }
                }
                // prepend current scope
                analyzeChildConnections(blocks, moduleScope + scopeStack);
            }

        } else if (object->getNodeType() == AST::Stream) {
            checkStreamConnections(static_pointer_cast<StreamNode>(object), QVector<ASTNode >());
        }
    }
}

void CodeResolver::fillDefaultProperties()
{
    vector<ASTNode> nodes = m_tree->getChildren();
    for(unsigned int i = 0; i < nodes.size(); i++) {
        ASTNode node = nodes.at(i);
        fillDefaultPropertiesForNode(node);
    }
}


void CodeResolver::declareModuleInternalBlocks()
{
    for (ASTNode node : m_tree->getChildren()) {
        declareInternalBlocksForNode(node, QVector<ASTNode>());
    }

//    populateContextDomains(m_tree->getChildren());
}

void CodeResolver::expandParallelStream(std::shared_ptr<StreamNode> stream, QVector<ASTNode > scopeStack, ASTNode tree)
{
    QList<LangError> errors;
    std::shared_ptr<StreamNode> subStream = stream;

    // Figure out stream IO sizes
    ASTNode left = stream->getLeft();
    ASTNode right = stream->getRight();
    QVector<QPair<int, int> > IOs;
    while (right) {
        if (left->getNodeType() == AST::Function) { // Expand from properties size to list
            ASTNode newFunctions = expandFunctionFromProperties(static_pointer_cast<FunctionNode>(left),
                                                             scopeStack, tree);
            if (newFunctions) {
                subStream->setLeft(newFunctions);
                left = subStream->getLeft();
            }
        }
        QPair<int, int> io;
        io.first = CodeValidator::getNodeNumInputs(left, scopeStack, tree, errors);
        io.second = CodeValidator::getNodeNumOutputs(left, scopeStack, tree, errors);
        IOs << io;
        if (right->getNodeType() == AST::Stream) {
            subStream = static_pointer_cast<StreamNode>(right);
            left = subStream->getLeft();
            right = subStream->getRight();
        } else {
            if (right->getNodeType() == AST::Function) {
                ASTNode newFunctions = expandFunctionFromProperties(static_pointer_cast<FunctionNode>(right),
                                                                 scopeStack, tree);
                if (newFunctions) {
                    subStream->setRight(newFunctions);
                    right = subStream->getRight();
                }
            }
            io.first = CodeValidator::getNodeNumInputs(right, scopeStack, tree, errors);
            io.second = CodeValidator::getNodeNumOutputs(right, scopeStack, tree, errors);
            IOs << io;
            right = nullptr;
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
            if (numCurIn > 0) {
                if (numPrevOut/(float)numCurIn == numPrevOut/numCurIn) {
                numCopies << numPrevOut/numCurIn;
                } else {
                    // Stream size mismatch. Stop expansion. The error will be reported later by
                    // CodeValidator.
                    numCopies << 1;
                    qDebug() << "Could not clone " << IOs[i - 1].second * numCopies.back()
                             << " outputs into " << IOs[i].first << " inputs.";
                }
            } else {
                // Cloning with size 1
                numCopies << 1;
            }
        } else if (numPrevOut < numCurIn && numPrevOut > 0) { // Need to clone all existing left side
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
                numCopies << 1;
            }

        } else { // Size match, no need to clone
            numCopies << 1;
        }
    }
    if (numCopies.size() == IOs.size()) { // Expansion calculation went fine, so expand
//                qDebug() << "Will expand";
        expandStreamToSizes(stream, numCopies, -1, scopeStack);
    }
}

void CodeResolver::expandParallel()
{
    std::vector<ASTNode > children = m_tree->getChildren();
    for (ASTNode node : children) {
        QVector<ASTNode > scopeStack;
        if (node->getNodeType() == AST::Stream) {
            std::shared_ptr<StreamNode> stream = static_pointer_cast<StreamNode>(node);
            expandParallelStream(stream, scopeStack, m_tree);
            // We need to process unknown symbols to make sure we can expand the following streams to the right size.
            std::vector<ASTNode > declarations = declareUnknownStreamSymbols(stream, nullptr, QVector<ASTNode >(), m_tree);
            for(ASTNode decl: declarations) {
                m_tree->addChild(decl);
            }
        }
    }
}

void CodeResolver::expandStreamToSizes(std::shared_ptr<StreamNode> stream, QVector<int> &neededCopies, int previousOutSize, QVector<ASTNode > scopeStack)
{
    ASTNode left = stream->getLeft();
//    int leftSize = CodeValidator::getNodeSize(left, scopeStack, m_tree);
    if (previousOutSize == -1) {
        previousOutSize = 1;
    }

    if (left->getNodeType() == AST::Function) {
        int numCopies = neededCopies.front();
//         if (leftSize < 0 && left->getNodeType() == AST::Block) {
//            std::vector<ASTNode > newDeclaration = declareUnknownName(static_pointer_cast<BlockNode>(left), previousOutSize, scopeStack, m_tree);
//            for(ASTNode decl:newDeclaration) {
//                m_tree->addChild(decl);
//            }
//        }
        if (numCopies > 1 && left->getNodeType() == AST::Function) {
            std::shared_ptr<ListNode> newLeft = std::make_shared<ListNode>(left, left->getFilename().data(), left->getLine());
            for (int i = 1; i < numCopies; i++) {
                newLeft->addChild(left->deepCopy());
            }
            stream->setLeft(newLeft);
        }
    }
    QList<LangError> errors;
    previousOutSize = CodeValidator::getNodeNumOutputs(stream->getLeft(), scopeStack, m_tree, errors);
    if (previousOutSize < 0) {
        previousOutSize = 1;
    }
    neededCopies.pop_front();
    ASTNode right = stream->getRight();
    if (right->getNodeType() == AST::Stream) {
        expandStreamToSizes(static_pointer_cast<StreamNode>(right), neededCopies, previousOutSize, scopeStack);
    } else {
        int rightSize = CodeValidator::getNodeSize(right, scopeStack, m_tree);
        if (right->getNodeType() == AST::Block
                || right->getNodeType() == AST::Function) {
            int numCopies = neededCopies.front();
            if (rightSize < 0 && right->getNodeType() == AST::Block) {
                std::vector<ASTNode > newDeclaration = declareUnknownName(static_pointer_cast<BlockNode>(right), previousOutSize, scopeStack, m_tree);
                for(ASTNode decl:newDeclaration) {
                    m_tree->addChild(decl);
                }
            } else if (numCopies > 1 && right->getNodeType() == AST::Function) { // Only functions should be expanded this way
                std::shared_ptr<ListNode> newRight = std::make_shared<ListNode>(right, right->getFilename().data(), right->getLine());
                for (int i = 1; i < numCopies; i++) {
                    newRight->addChild(right);
                }
                stream->setRight(newRight);
            }

        }
        neededCopies.pop_front();
        Q_ASSERT(neededCopies.size() == 0); // This is the end of the stream there should be no sizes left
    }
}

ASTNode CodeResolver::expandFunctionFromProperties(std::shared_ptr<FunctionNode> func, QVector<ASTNode > scopeStack, ASTNode tree)
{
    QList<LangError> errors;
    std::shared_ptr<ListNode> newFunctions = nullptr;
//    int dataSize = CodeValidator::getFunctionDataSize(func, scopeStack, tree, errors);
    int dataSize = CodeValidator::getFunctionNumInstances(func, scopeStack, tree);
    if (dataSize > 1) {
        vector<std::shared_ptr<PropertyNode>> props = func->getProperties();
        newFunctions = std::make_shared<ListNode>(nullptr, func->getFilename().c_str(), func->getLine());
        for (int i = 0; i < dataSize; ++i) { // FIXME this assumes each function takes a single input. Need to check the actual input size.
            newFunctions->addChild(func->deepCopy());
        }
        for (auto prop : props) {
            ASTNode value = prop->getValue();
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
                return nullptr;
            }
            if (numOuts == 1) { // Single value given, duplicate for all copies.
                for (ASTNode newFunction : newFunctions->getChildren()) {
                    newFunction->addChild(prop->deepCopy());
                }
            } else {
                if (value->getNodeType() == AST::Bundle) {
                    // FIXME write support for ranges

                } else if (value->getNodeType() == AST::Block) {
                    BlockNode *name = static_cast<BlockNode *>(value.get());
                    std::shared_ptr<DeclarationNode> block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()),
                                                                      scopeStack, tree, name->getNamespaceList());
                    int size = CodeValidator::getBlockDeclaredSize(block, scopeStack, tree, errors);
                    Q_ASSERT(size == dataSize);
                    for (int i = 0; i < size; ++i) {
                        std::shared_ptr<PropertyNode> newProp = static_pointer_cast<PropertyNode>(prop->deepCopy());
                        std::shared_ptr<ListNode> indexList = std::make_shared<ListNode>(std::make_shared<ValueNode>(i + 1,
                                                                         prop->getFilename().c_str(),
                                                                         prop->getLine()),
                                                           prop->getFilename().c_str(), prop->getLine());
                        std::shared_ptr<BundleNode> newBundle = std::make_shared<BundleNode>(name->getName(), indexList,
                                                               prop->getFilename().c_str(), prop->getLine());
                        newProp->replaceValue(newBundle);
                        static_pointer_cast<FunctionNode>(newFunctions->getChildren()[i])->addChild(newProp);
                    }

                } else if (value->getNodeType() == AST::List) {
                    // FIXME we need to split the list according to the expected size. This currently assumes size == 1
                    vector<ASTNode > values = static_pointer_cast<ListNode>(value)->getChildren();
                    vector<ASTNode > functions = newFunctions->getChildren();
                    Q_ASSERT(values.size() == functions.size());
                    for (size_t i = 0 ; i < (size_t) dataSize; ++i) {
                        std::shared_ptr<PropertyNode> newProp = static_pointer_cast<PropertyNode>(prop);
                        static_pointer_cast<FunctionNode>(functions[i])->setPropertyValue(newProp->getName(), values[i]->deepCopy());
                    }
                } else {
                    qDebug() << "Error. Don't know how to expand property.";
                }
            }

        }
    }
    return newFunctions;
}

void CodeResolver::insertBuiltinObjects()
{
    QList<std::shared_ptr<DeclarationNode>> usedDeclarations;
    map<string, vector<ASTNode>> bultinObjects;
    if (m_system) {
        bultinObjects = m_system->getBuiltinObjectsReference();
    }

    // First pass to add the fundamental types
    for (ASTNode object : bultinObjects[""]) {
        if (object->getNodeType() == AST::Declaration) {
            std::shared_ptr<DeclarationNode> block = static_pointer_cast<DeclarationNode>(object);
            if (block->getObjectType() == "type") {
                ASTNode nameNode = block->getPropertyValue("typeName");
                if (nameNode->getNodeType() == AST::String) {
                    ValueNode *typeName = static_cast<ValueNode *>(nameNode.get());
                    if (typeName->getStringValue() == "signal"
                            /*|| typeName->getStringValue() == "propertyInputPort"
                            || typeName->getStringValue() == "propertyOutputPort"
                            || typeName->getStringValue() == "reaction"
                            || typeName->getStringValue() == "signalbridge"*/) {
                        m_tree->addChild(block);
                        usedDeclarations << block;
                    }
                    if (typeName->getStringValue() == "type"
                           /* || typeName->getStringValue() == "platformModule"
                        || typeName->getStringValue() == "platformBlock"*/) {
                        m_tree->addChild(block);
                        usedDeclarations << block;
                    }
                }
            } else if (block->getObjectType() == "alias") {
                if (block->getName() == "PlatformDomain" || block->getName() == "PlatformRate") {
                    m_tree->addChild(block);
                    usedDeclarations << block;
                }
            } else if (block->getObjectType() == "_domainDefinition"
                     || block->getObjectType() == "_frameworkDescription") // Hack... this should be getting inserted when resolving symbols...)
            {
                m_tree->addChild(block);
                usedDeclarations << block;
            } else if (block->getName() == "_PlatformDomainProcessing") // Hack... this should be getting inserted when resolving symbols...)
            {
                m_tree->addChild(block);
                usedDeclarations << block;
            }
//            continue;
        }
    }
    for (ASTNode decl: usedDeclarations) {
        bultinObjects[""].erase(std::find(bultinObjects[""].begin(), bultinObjects[""].end(),decl));
        insertBuiltinObjectsForNode(decl, bultinObjects);
    }

    // Second pass to add elements that depend on the user's code
    for (ASTNode object : m_tree->getChildren()) {
        insertBuiltinObjectsForNode(object, bultinObjects);
    }

}

void CodeResolver::processDomains()
{
    // Fill missing domain information (propagate domains)
    // First we need to traverse the streams backwards to make sure we propagate the domains from the furthest point down the line
    vector<ASTNode > children = m_tree->getChildren();
    vector<ASTNode >::reverse_iterator rit = children.rbegin();
    QVector<ASTNode > scopeStack; // = QVector<AST*>::fromStdVector(children);

    ASTNode domainBlock = m_system->getPlatformDomain();
    mContextDomainStack.push_back(domainBlock);
    while(rit != children.rend()) {
        ASTNode node = *rit;
        propagateDomainsForNode(node, scopeStack);
        rit++;
    }

    // Any signals without domain are assigned to the platform domain
    // TODO: Do we need this? It seems that any signals without domain at this point are unused
//    if (domainDecl) {
//        setContextDomain(m_tree->getChildren(),domainDecl);
//    }
    // Now split streams when there is a domain change
    vector<ASTNode > new_tree;
    for (ASTNode node : m_tree->getChildren()) {
        if (node->getNodeType() == AST::Stream) {
            QVector<ASTNode > streams = sliceStreamByDomain(static_pointer_cast<StreamNode>(node), QVector<ASTNode >());
            for (ASTNode  stream: streams) {
                new_tree.push_back(stream);
            }
        } else if (node->getNodeType() == AST::Declaration) {
            std::shared_ptr<DeclarationNode> module = static_pointer_cast<DeclarationNode>(node);
            sliceDomainsInNode(module, QVector<ASTNode >());
            new_tree.push_back(module);
        } else {
            new_tree.push_back(node);
        }
    }

    m_tree->setChildren(new_tree);
}

void CodeResolver::analyzeConnections()
{
    analyzeChildConnections(m_tree);

    std::vector<std::shared_ptr<DeclarationNode>> knownDomains;
    for (ASTNode node: m_tree->getChildren()) {
        if (node->getNodeType() == AST::Declaration) {
            std::shared_ptr<DeclarationNode> decl = static_pointer_cast<DeclarationNode>(node);
            if (decl->getObjectType() == "_domainDefinition") {
                knownDomains.push_back(decl);
                Q_ASSERT(!decl->getCompilerProperty("domainReads"));
                Q_ASSERT(!decl->getCompilerProperty("domainWrites"));
                decl->setCompilerProperty("domainReads", std::make_shared<ListNode>(__FILE__, __LINE__));
                decl->setCompilerProperty("domainWrites", std::make_shared<ListNode>(__FILE__, __LINE__));
            }
        }
    }

    // Now go through the tree to match domain reads and domain writes
    for (ASTNode node : m_tree->getChildren()) {
        if (node->getNodeType() == AST::Declaration) {

            std::shared_ptr<DeclarationNode> decl;
            if (node->getCompilerProperty("reads")) {
                for (auto domain: node->getCompilerProperty("reads")->getChildren()) {
                    if (domain) {
                        if(domain->getNodeType() == AST::String) {
                            std::string domainName = static_pointer_cast<ValueNode>(domain)->getStringValue();
                            for (auto knownDomain: knownDomains) {
                                ASTNode domainNameValue = knownDomain->getPropertyValue("domainName");
                                if (domainNameValue && domainNameValue->getNodeType() == AST::String) {
                                    if (domainName == static_cast<ValueNode *>(domainNameValue.get())->getStringValue() ) {
                                        decl = knownDomain;
                                        break;
                                    }
                                }
                            }
                            if (decl) {
                                decl->getCompilerProperty("domainReads")->addChild(node);
                            }
                        } else if (domain->getNodeType() == AST::Block) {
                            decl = CodeValidator::findDeclaration(CodeValidator::streamMemberName(domain),
                                                                  vector<ASTNode>(), m_tree);
                            if (decl) {
                                decl->getCompilerProperty("domainReads")->addChild(node);
                            }
                        } else {
                            Q_ASSERT(domain->getNodeType() == AST::PortProperty);
                        }
                    }
                }
            }

            if (node->getCompilerProperty("writes")) {
                for (auto domain: node->getCompilerProperty("writes")->getChildren()) {
                    if(domain->getNodeType() == AST::String) {
                        std::string domainName = static_pointer_cast<ValueNode>(domain)->getStringValue();
                        for (auto knownDomain: knownDomains) {
                            ASTNode domainNameValue = knownDomain->getPropertyValue("domainName");
                            if (domainNameValue && domainNameValue->getNodeType() == AST::String) {
                                if (domainName == static_cast<ValueNode *>(domainNameValue.get())->getStringValue() ) {
                                    decl = knownDomain;
                                    break;
                                }
                            }
                        }
                        if (decl) {
                            decl->getCompilerProperty("domainWrites")->addChild(node);
                        }
                    } else if(domain->getNodeType() == AST::Block) {
                        decl = CodeValidator::findDeclaration(CodeValidator::streamMemberName(domain),
                                                              vector<ASTNode>(), m_tree);
                        if (decl) {
                            decl->getCompilerProperty("domainReads")->addChild(node);
                        }
                    } else {
                        Q_ASSERT(domain->getNodeType() == AST::PortProperty);
                    }
                }
            }
        }

    }
}

void CodeResolver::storeDeclarations()
{
    for(auto node: m_tree->getChildren()) {
        storeDeclarationsForNode(node, vector<ASTNode>(), m_tree);
    }
}

void CodeResolver::insertDependentTypes(std::shared_ptr<DeclarationNode> typeDeclaration, map<string, vector<ASTNode>> &objects) {
    QList<std::shared_ptr<DeclarationNode>> blockList;
//    std::shared_ptr<DeclarationNode> existingDecl = CodeValidator::findTypeDeclaration(typeDeclaration, QVector<ASTNode >(), m_tree);
    for (auto it = objects.begin(); it != objects.end(); it++)  {
            // To avoid redundant checking here we should mark nodes that have already been processed
        auto inheritedTypes = CodeValidator::getInheritedTypes(typeDeclaration, QVector<ASTNode>::fromStdVector(it->second), m_tree);

        for (auto inheritedType: inheritedTypes) {
            auto children = m_tree->getChildren();
            if (std::find(children.begin(), children.end(), inheritedType)
                    != children.end()) {
                // nothing
            } else {
                m_tree->addChild(inheritedType);
                auto findPos = std::find(it->second.begin(), it->second.end(), inheritedType);
                if (findPos != it->second.end()) {
                    it->second.erase(findPos);
                }
            }
            insertDependentTypes(inheritedType, objects);
        }
    }
}

void CodeResolver::insertBuiltinObjectsForNode(ASTNode node, map<string, vector<ASTNode>> &objects)
{
    QList<std::shared_ptr<DeclarationNode>> blockList;
    if (node->getNodeType() == AST::List) {
        for (ASTNode child : node->getChildren()) {
            insertBuiltinObjectsForNode(child, objects);
        }
    } else if (node->getNodeType() == AST::Stream) {
        StreamNode *stream = static_cast<StreamNode*>(node.get());
        insertBuiltinObjectsForNode(stream->getLeft(), objects);
        insertBuiltinObjectsForNode(stream->getRight(), objects);
    } else if (node->getNodeType() == AST::Expression) {
        ExpressionNode *expr = static_cast<ExpressionNode*>(node.get());
        if (expr->isUnary()) {
            insertBuiltinObjectsForNode(expr->getValue(), objects);
        } else {
            insertBuiltinObjectsForNode(expr->getLeft(), objects);
            insertBuiltinObjectsForNode(expr->getRight(), objects);
        }
    } else if (node->getNodeType() == AST::Function) {
        FunctionNode *func = static_cast<FunctionNode *>(node.get());
        for (auto it = objects.begin(); it != objects.end(); it++)  {
            std::shared_ptr<DeclarationNode> declaration  = CodeValidator::findDeclaration(QString::fromStdString(func->getName()),
                                                                           QVector<ASTNode >::fromStdVector(it->second), nullptr);
            if (declaration) {
                declaration->setRootScope(it->first);
                for(auto child: declaration->getChildren()) { // Check if declaration is in current namespace. If it is, set as the namespace of the child
                    std::shared_ptr<DeclarationNode> childDeclaration = nullptr;
                    if (child->getNodeType() == AST::Block) {
                        childDeclaration = CodeValidator::findDeclaration(QString::fromStdString(static_cast<BlockNode *>(child.get())->getName()),
                                                                                       QVector<ASTNode >::fromStdVector(it->second), nullptr,
                                                                                       declaration->getNamespaceList());
                    } else if (child->getNodeType() == AST::Bundle) {
                        childDeclaration = CodeValidator::findDeclaration(QString::fromStdString(static_cast<BundleNode *>(child.get())->getName()),
                                                                                       QVector<ASTNode >::fromStdVector(it->second), nullptr,
                                                                                       declaration->getNamespaceList());
                    } // FIXME need to implement for expressions, lists, etc.
                    if (childDeclaration) {
                        child->setNamespaceList(declaration->getNamespaceList());
                    }
                }
                if (!blockList.contains(declaration)) {
                    blockList << declaration;
                    break;
                }
            }
        }
        // Look for declarations of blocks present in function properties
        for(auto property :func->getProperties()) {
            insertBuiltinObjectsForNode(property->getValue(), objects);
        }
        for (std::shared_ptr<DeclarationNode> usedBlock : blockList) {
            // Add declarations to tree if not there
            if (!CodeValidator::findDeclaration(QString::fromStdString(usedBlock->getName()), QVector<ASTNode >(), m_tree, usedBlock->getNamespaceList())) {
                m_tree->addChild(usedBlock);
                for (auto it = objects.begin(); it != objects.end(); it++)  {
                    vector<ASTNode > &namespaceObjects = it->second;
                    auto position = std::find(namespaceObjects.begin(), namespaceObjects.end(), usedBlock);
                    if (position != namespaceObjects.end()) {
                        namespaceObjects.erase(position);
                        break;
                    }
                }
            }
            for (std::shared_ptr<PropertyNode> property : usedBlock->getProperties()) {
                insertBuiltinObjectsForNode(property->getValue(), objects);
            }
            insertBuiltinObjectsForNode(usedBlock, objects);
        }
    } else if (node->getNodeType() == AST::Declaration
               || node->getNodeType() == AST::BundleDeclaration) {
        std::shared_ptr<DeclarationNode> declaration = static_pointer_cast<DeclarationNode>(node);
        vector<ASTNode > namespaceObjects;
        if (declaration->getScopeLevels() > 0) {
            namespaceObjects = objects[declaration->getScopeAt(0)]; // TODO need to implement for namespaces that are more than one level deep...
        }

        for(auto child: declaration->getChildren()) { // Check if declaration is in current namespace. If it is, set as the namespace of the child
            std::shared_ptr<DeclarationNode> childDeclaration = nullptr;
            if (child->getNodeType() == AST::Block) {
                childDeclaration = CodeValidator::findDeclaration(QString::fromStdString(static_cast<BlockNode *>(child.get())->getName()),
                                                                  QVector<ASTNode >::fromStdVector(namespaceObjects), nullptr,
                                                                  declaration->getNamespaceList());
            } else if (child->getNodeType() == AST::Bundle) {
                childDeclaration = CodeValidator::findDeclaration(QString::fromStdString(static_cast<BundleNode *>(child.get())->getName()),
                                                                  QVector<ASTNode >::fromStdVector(namespaceObjects), nullptr,
                                                                  declaration->getNamespaceList());
            } // FIXME need to implement for expressions, lists, etc.
            if (childDeclaration) {
                child->setNamespaceList(declaration->getNamespaceList());
            }
        }
        auto typeDecl = CodeValidator::findTypeDeclarationByName(
                    declaration->getObjectType(), QVector<ASTNode>(), m_tree);
        if (!typeDecl) {
            for (auto &scope: objects) {
                typeDecl = CodeValidator::findTypeDeclarationByName(
                                    declaration->getObjectType(), QVector<ASTNode>::fromStdVector(scope.second), nullptr);
                if (typeDecl) {
                    m_tree->addChild(typeDecl);
                    auto position = std::find(scope.second.begin(), scope.second.end(), typeDecl);
                    if (position != scope.second.end()) {
                        scope.second.erase(position);
                        insertBuiltinObjectsForNode(typeDecl, objects);
                        break;
                    }
                    insertDependentTypes(typeDecl, objects);
                }
            }
        }

        // Insert needed objects for things in module properties
        for(std::shared_ptr<PropertyNode> property : declaration->getProperties()) {
            insertBuiltinObjectsForNode(property->getValue(), objects);
        }
        // Process index for bundle declarations
        if (node->getNodeType() == AST::BundleDeclaration) {
            insertBuiltinObjectsForNode(declaration->getBundle()->index(), objects);
        }

    } else if (node->getNodeType() == AST::Block) {
        QList<std::shared_ptr<DeclarationNode>> blockList;
        BlockNode *name = static_cast<BlockNode *>(node.get());

        for (auto it = objects.begin(); it != objects.end(); it++)  {
            auto declaration = CodeValidator::findDeclaration(QString::fromStdString(name->getName()),
                                                              QVector<ASTNode>::fromStdVector(it->second),
                                                              m_tree, name->getNamespaceList());
            if (declaration && !CodeValidator::findDeclaration(QString::fromStdString(name->getName()),
                                                                      QVector<ASTNode>(),
                                                                      m_tree, name->getNamespaceList())) {
//                declaration->setRootScope(it->first);
                if (!blockList.contains(declaration)) {
                    blockList << declaration;
                }
                break;
            }
        }
        for(auto usedBlock : blockList) {
            m_tree->addChild(usedBlock);
            for (auto it = objects.begin(); it != objects.end(); it++)  {
                vector<ASTNode > &namespaceObjects = it->second;
                auto position = std::find(namespaceObjects.begin(), namespaceObjects.end(), usedBlock);
                if (position != namespaceObjects.end()) {
                    namespaceObjects.erase(position);
                }
            }
            insertBuiltinObjectsForNode(usedBlock, objects);
        }
    } else if (node->getNodeType() == AST::Bundle) {
        QList<std::shared_ptr<DeclarationNode>> blockList;
        auto bundle = static_pointer_cast<BundleNode>(node);
        for (auto it = objects.begin(); it != objects.end(); it++)  {
            auto declaration = CodeValidator::findDeclaration(QString::fromStdString(bundle->getName()),
                                                              QVector<ASTNode>::fromStdVector(it->second),
                                                              m_tree, bundle->getNamespaceList());
            if (declaration && !CodeValidator::findDeclaration(QString::fromStdString(bundle->getName()),
                                                               QVector<ASTNode>(),
                                                               m_tree, bundle->getNamespaceList())) {
//                declaration->setRootScope(it->first);
                blockList << declaration;
                break;
            }
        }
        for(auto usedBlock : blockList) {
            ASTNode newBlock = usedBlock;
            m_tree->addChild(newBlock);
            for (auto it = objects.begin(); it != objects.end(); it++)  {
                vector<ASTNode > &namespaceObjects = it->second;
                auto position = std::find(namespaceObjects.begin(), namespaceObjects.end(), usedBlock);
                if (position != namespaceObjects.end()) {
                    namespaceObjects.erase(position);
                }
            }
            insertBuiltinObjectsForNode(newBlock, objects);
        }
    } else if (node->getNodeType() == AST::Property) {
        std::shared_ptr<PropertyNode> prop = std::static_pointer_cast<PropertyNode>(node);
        insertBuiltinObjectsForNode(prop->getValue(), objects);
    }
}

void CodeResolver::resolveDomainsForStream(std::shared_ptr<StreamNode> stream, QVector<ASTNode > scopeStack, ASTNode contextDomainNode)
{
    ASTNode left = stream->getLeft();
    ASTNode right = stream->getRight();
    QList<ASTNode > domainStack;
    ASTNode previousDomain = CodeValidator::getNodeDomain(left, scopeStack, m_tree);
    std::string previousDomainId = CodeValidator::getDomainIdentifier(previousDomain, scopeStack.toStdVector(), m_tree);

//    std::string contextDomain = CodeValidator::getDomainIdentifier(contextDomainNode, scopeStack.toStdVector(), m_tree);

    if (!previousDomain || previousDomain->getNodeType() == AST::None) {
        previousDomain = contextDomainNode;
    }
    ASTNode domainNode;
    while (right) {
        domainNode = processDomainsForNode(left, scopeStack, domainStack);
        std::string domainId;
        if (domainNode) {
            domainId = CodeValidator::getDomainIdentifier(domainNode, scopeStack.toStdVector(), m_tree);
        }
        if (left == right && (!domainNode || domainNode->getNodeType() == AST::None)) { // If this is is the last pass then set the unknown domains to context domain
            if (left->getNodeType() == AST::Function && (!domainNode || domainNode->getNodeType() == AST::None)) {
                // Have functions/loops/reactions be assigned to whatever they are connected to
                // if they are final in a stream.
                // FIXME we should look inside the modules themselves to see if we can infer the domain better
                if (previousDomain && previousDomain->getNodeType() != AST::None) {
                    domainNode = previousDomain;
                } else {
                    previousDomain = contextDomainNode;
                }
            } else {
                // FIXME This is needed in cases where signals with no set domain are connected
                // to input ports on other objects. Domains are not
                // propagated across secondary ports, but they should be

                    previousDomain = contextDomainNode;
            }
            // Unneeded as it gets added in the above call to processDomainsForNode
//            domainStack << left;
        }

        if (domainNode && domainNode->getNodeType() != AST::None && previousDomainId != domainId) {
            setDomainForStack(domainStack, domainNode, scopeStack);
            domainStack.clear();
        }
        if (left->getNodeType() == AST::Expression) {

            auto samplingDomain = CodeValidator::getNodeDomain(stream->getRight(), scopeStack, m_tree);
            function<void(ASTNode node, ASTNode samplingDomain)> func = [&func](ASTNode node, ASTNode samplingDomain){
                for (auto child: node->getChildren()) {
                    if (child->getNodeType() == AST::Expression) {
                        child->setCompilerProperty("samplingDomain", samplingDomain);
                        func(child, samplingDomain);
                    }
                }
            };
            func(left, samplingDomain);
            left->setCompilerProperty("samplingDomain", samplingDomain);
        } else if (left->getNodeType() == AST::Function) {
            auto decl = CodeValidator::findDeclaration(CodeValidator::streamMemberName(left), scopeStack.toStdVector(), m_tree);

            if (decl) {
                domainNode = processDomainsForNode(decl, scopeStack, domainStack);
                auto internalStreams = decl->getPropertyValue("streams");
                auto internalBlocks = decl->getPropertyValue("blocks");
                auto internalScopeStack = scopeStack;

                if (internalStreams && internalBlocks) {
                    for (auto internalStream: internalStreams->getChildren()) {
                        Q_ASSERT(internalStream->getNodeType() == AST::Stream);
                        internalScopeStack << QVector<ASTNode>::fromStdVector(internalBlocks->getChildren());
                        if (internalStream->getNodeType() == AST::Stream) {
                            // No context domain as all domains within modules must be resolvable internally
                            resolveDomainsForStream(
                                        std::static_pointer_cast<StreamNode>(internalStream), internalScopeStack, nullptr);
                        }
                    }
                }

            }
        }

        if (left == right) {
            if (previousDomain && previousDomain->getNodeType() != AST::None) {
                setDomainForStack(domainStack, previousDomain, scopeStack);
                domainStack.clear();
            }
            right = left = nullptr; // End
        } else if(right->getNodeType() == AST::Stream) {
            stream = static_pointer_cast<StreamNode>(right);
            left = stream->getLeft();
            right = stream->getRight();
        } else {
            if (left->getNodeType() == AST::Expression) {
                left->setCompilerProperty("samplingDomain",
                                          CodeValidator::getNodeDomain(right, scopeStack, m_tree));
            }
            left = right; // Last pass (process right, call it left)
        }
        previousDomain = domainNode;
        previousDomainId = domainId;
    }
}

ASTNode CodeResolver::processDomainsForNode(ASTNode node, QVector<ASTNode > scopeStack, QList<ASTNode > &domainStack)
{
    ASTNode currentDomain;
    resolveDomainForStreamNode(node, scopeStack);
    if (node->getNodeType() == AST::Block
            || node->getNodeType() == AST::Bundle) {
        std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(CodeValidator::streamMemberName(node),
                                                                                      scopeStack.toStdVector(), m_tree, node->getNamespaceList());
        if(declaration) {
            ASTNode domain = declaration->getDomain();
            if (!domain || domain->getNodeType() == AST::None) {
                // Put declaration in stack to set domain once domain is resolved
                domainStack << declaration;
            } else if (domain->getNodeType() == AST::String
                        || domain->getNodeType() == AST::Block
                        || domain->getNodeType() == AST::PortProperty) {
                    currentDomain = domain;
            } else {
                qDebug() << "unrecognized domain type";
            }
        }
    } else if (node->getNodeType() == AST::Function) {
        FunctionNode *func = static_cast<FunctionNode *>(node.get());
        ASTNode domain = func->getDomain();
        if (!domain) {
            // Put declaration in stack to set domain once domain is resolved
            domainStack << node;
        } else {
            if (domain->getNodeType() == AST::String
                    || domain->getNodeType() == AST::Block
                    || domain->getNodeType() == AST::PortProperty) {
                currentDomain = domain;
            } else {
                domainStack << node;
            }
        }
        for(auto member : func->getProperties()) {
            ASTNode propertyDomainName = processDomainsForNode(member->getValue(), scopeStack, domainStack);
            if (!propertyDomainName || propertyDomainName->getNodeType() == AST::None) { // Include property block in domain stack because domain undefined
                 domainStack << member->getValue();
            }
        }
    } else if (node->getNodeType() == AST::List) {
        QList<ASTNode > listDomainStack;
        for (ASTNode  member : node->getChildren()) {
            ASTNode memberDomainName = processDomainsForNode(member, scopeStack, domainStack);

            if (!currentDomain || currentDomain->getNodeType() == AST::None) {
                // list takes domain from first element that has a domain
                if (memberDomainName && (memberDomainName->getNodeType() == AST::String
                                         || memberDomainName->getNodeType() == AST::Block
                                         || memberDomainName->getNodeType() == AST::PortProperty)) {
                    currentDomain = memberDomainName;
                } else {
                    domainStack << node;
                }
            }
            if (!memberDomainName || memberDomainName->getNodeType() == AST::None) {
                // Put declaration in stack to set domain once domain is resolved

                listDomainStack << member;
                // FIMXE: This is very simplistic (or plain wrong....)
                // It assumes that the next found domain affects all elements
                // in the list that don't have domains. This is likely a
                // common case but list elements should inherit domains from
                // the port to which they are connected.
            }
        }
        if (currentDomain && currentDomain->getNodeType() != AST::None) {
            setDomainForStack(listDomainStack, currentDomain, scopeStack);
        } else {
            domainStack << listDomainStack; // If list has no defined domain, pass all elements to be set later
        }
    } else if (node->getNodeType() == AST::Expression) {
        for(ASTNode  member : node->getChildren()) {
            ASTNode newDomainName = processDomainsForNode(member, scopeStack, domainStack);
            // if member doesn't have a domain, put in the stack to resolve later.
            if (!newDomainName || newDomainName->getNodeType() == AST::None) {
                // Put declaration in stack to set domain once domain is resolved
                domainStack << member;
            } else {
                currentDomain = newDomainName;
            }
        }
    } else if (node->getNodeType() == AST::Int || node->getNodeType() == AST::Real
               || node->getNodeType() == AST::String || node->getNodeType() == AST::Switch) {
        domainStack << node;
    }
    return currentDomain;
}

void CodeResolver::setDomainForStack(QList<ASTNode > domainStack, ASTNode domainName,  QVector<ASTNode > scopeStack)
{
    for (ASTNode relatedNode : domainStack) {
        if (relatedNode->getNodeType() == AST::Declaration
                || relatedNode->getNodeType() == AST::BundleDeclaration ) {
            std::shared_ptr<DeclarationNode> block = static_pointer_cast<DeclarationNode>(relatedNode);
            if (block) {
                block->replacePropertyValue("domain", domainName);
            }
        } else if (relatedNode->getNodeType() == AST::Block
                   || relatedNode->getNodeType() == AST::Bundle) {
            std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(CodeValidator::streamMemberName(relatedNode),
                                                                                          scopeStack.toStdVector(), m_tree);
            if(declaration) {
                declaration->replacePropertyValue("domain", domainName);
            }
        } else if (relatedNode->getNodeType() == AST::Function) {
            std::shared_ptr<FunctionNode> func = static_pointer_cast<FunctionNode>(relatedNode);
//            if (func) {
//                func->setPropertyValue("domain", domainName);
//            }
        } else if (relatedNode->getNodeType() == AST::Real
                   || relatedNode->getNodeType() == AST::Int
                   || relatedNode->getNodeType() == AST::String
                   || relatedNode->getNodeType() == AST::Switch) {
            std::shared_ptr<ValueNode> val = static_pointer_cast<ValueNode>(relatedNode);
            if (val) {
                val->setDomain(domainName);
            }
        } else if (relatedNode->getNodeType() == AST::List
                    || relatedNode->getNodeType() == AST::Expression) {
            QList<ASTNode> children;
            for(ASTNode  member : relatedNode->getChildren()) {
                children.push_back(member);
            }
            setDomainForStack(children, domainName, scopeStack);
        }
    }
}

std::shared_ptr<DeclarationNode>CodeResolver::createDomainDeclaration(QString name)
{
    std::shared_ptr<DeclarationNode>newBlock = nullptr;
    newBlock = std::make_shared<DeclarationNode>(name.toStdString(), "_domainDefinition", nullptr, __FILE__, __LINE__);
    newBlock->addProperty(std::make_shared<PropertyNode>("domainName", std::make_shared<ValueNode>(name.toStdString(), __FILE__, __LINE__), __FILE__, __LINE__));
    fillDefaultPropertiesForNode(newBlock);
    return newBlock;
}

std::shared_ptr<DeclarationNode> CodeResolver::createSignalDeclaration(QString name, int size, QVector<ASTNode> &scope)
{
    std::shared_ptr<DeclarationNode> newBlock = nullptr;
    Q_ASSERT(size > 0);
    if (size == 1) {
        newBlock = std::make_shared<DeclarationNode>(name.toStdString(), "signal", nullptr, __FILE__, __LINE__);
    } else if (size > 1) {
        std::shared_ptr<ListNode> indexList = std::make_shared<ListNode>(std::make_shared<ValueNode>(size, "",-1), __FILE__, __LINE__);
        std::shared_ptr<BundleNode> bundle = std::make_shared<BundleNode>(name.toStdString(),indexList, "",-1);
        newBlock = std::make_shared<DeclarationNode>(bundle, "signal", nullptr, "",-1);
    }
    Q_ASSERT(newBlock);
    fillDefaultPropertiesForNode(newBlock);

    resolveConstantsInNode(newBlock, scope);
    return newBlock;
}

std::vector<ASTNode> CodeResolver::declareUnknownName(std::shared_ptr<BlockNode> block, int size, QVector<ASTNode> localScope, ASTNode tree)
{
    std::vector<ASTNode > declarations;
    std::shared_ptr<DeclarationNode> decl = CodeValidator::findDeclaration(QString::fromStdString(block->getName()), localScope, tree, block->getNamespaceList());
    if (!decl) { // Not declared, so make declaration
        std::shared_ptr<DeclarationNode> newSignal = createSignalDeclaration(QString::fromStdString(block->getName()), size, localScope);
        double rate = CodeValidator::getNodeRate(newSignal, QVector<ASTNode >(), tree);

        CodeValidator::setNodeRate(block, rate, localScope, tree);
        declarations.push_back(newSignal);
    }
	return declarations;
}

std::vector<ASTNode> CodeResolver::declareUnknownBundle(std::shared_ptr<BundleNode> bundle, int size, QVector<ASTNode > localScope, ASTNode tree)
{
    std::vector<ASTNode > declarations;
    std::shared_ptr<DeclarationNode> block = CodeValidator::findDeclaration(QString::fromStdString(bundle->getName()), localScope, tree, bundle->getNamespaceList());
    if (!block) { // Not declared, so make declaration
        std::shared_ptr<DeclarationNode> newSignal = createSignalDeclaration(QString::fromStdString(bundle->getName()), size, localScope);
        double rate = CodeValidator::getNodeRate(newSignal, QVector<ASTNode >(), tree);
        CodeValidator::setNodeRate(bundle, rate, localScope, tree);
        declarations.push_back(newSignal);
    }
    return declarations;
}

std::shared_ptr<DeclarationNode> CodeResolver::createConstantDeclaration(string name, ASTNode value)
{
    std::shared_ptr<DeclarationNode> constant = std::make_shared<DeclarationNode>(name, "constant", nullptr, __FILE__, __LINE__);
    std::shared_ptr<PropertyNode> valueProperty = std::make_shared<PropertyNode>("value", value, __FILE__, __LINE__);
    constant->addProperty(valueProperty);
    return constant;
}

void CodeResolver::declareIfMissing(string name, ASTNode blocks, ASTNode value)
{
    std::shared_ptr<DeclarationNode> declaration = nullptr;
    if (blocks->getNodeType() == AST::List) {
        ListNode *blockList = static_cast<ListNode *>(blocks.get());
        // First check if block has been declared
        for(ASTNode block : blockList->getChildren()) {
            if (block->getNodeType() == AST::Declaration || block->getNodeType() == AST::BundleDeclaration) {
                std::shared_ptr<DeclarationNode> declaredBlock = static_pointer_cast<DeclarationNode>(block);
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
//            delete value;
        }
    } else {
//        delete value;
        qDebug() << "CodeResolver::declareIfMissing() blocks is not list";
    }
}

std::shared_ptr<DeclarationNode> CodeResolver::createSignalBridge(string bridgeName, string originalName,
                                                                  ASTNode defaultValue, ASTNode inDomain, ASTNode outDomain,
                                                                  const string filename, int line, int size,
                                                                  string type)
{
    std::shared_ptr<DeclarationNode> newBridge;
    if (size == 1) {
        newBridge = std::make_shared<DeclarationNode>(bridgeName, "signalbridge", nullptr, __FILE__, __LINE__);
    } else { // A BlockBundle
        newBridge = std::make_shared<DeclarationNode>(std::make_shared<BundleNode>(bridgeName, std::make_shared<ListNode>(std::make_shared<ValueNode>(size, __FILE__, __LINE__), __FILE__, __LINE__), __FILE__, __LINE__),
                                                              "signalbridge", nullptr, __FILE__, __LINE__);
    }
    newBridge->addProperty(std::make_shared<PropertyNode>("default", defaultValue, filename.c_str(), line));
    newBridge->addProperty(std::make_shared<PropertyNode>("signal", std::make_shared<ValueNode>(std::string(originalName), filename.c_str(), line),
                                                          filename.c_str(), line));
    if (inDomain) {
        newBridge->addProperty(std::make_shared<PropertyNode>("inputDomain", inDomain, filename.c_str(), line));
        newBridge->addProperty(std::make_shared<PropertyNode>("domain", inDomain, filename.c_str(), line));
    } else {
        newBridge->addProperty(std::make_shared<PropertyNode>("inputDomain", std::make_shared<ValueNode>(__FILE__, __LINE__), filename.c_str(), line));
        newBridge->addProperty(std::make_shared<PropertyNode>("domain", std::make_shared<ValueNode>(__FILE__, __LINE__), filename.c_str(), line));
    }
    if (outDomain) {
        newBridge->addProperty(std::make_shared<PropertyNode>("outputDomain", outDomain, filename.c_str(), line));
    } else {
        newBridge->addProperty(std::make_shared<PropertyNode>("outputDomain", std::make_shared<ValueNode>(__FILE__, __LINE__), filename.c_str(), line));
    }
    string domainName = CodeValidator::getDomainNodeString(outDomain);
    m_bridgeAliases.push_back({bridgeName, originalName, domainName});
    newBridge->addProperty(std::make_shared<PropertyNode>("bridgeType",
                                                          std::make_shared<ValueNode>(type, filename.c_str(), line),
                                                          filename.c_str(), line));
    return newBridge;
}


std::vector<ASTNode > CodeResolver::declareUnknownExpressionSymbols(std::shared_ptr<ExpressionNode> expr, int size, QVector<ASTNode > scopeStack, ASTNode  tree)
{
    std::vector<ASTNode > newDeclarations;
    if (expr->isUnary()) {
        if (expr->getValue()->getNodeType() == AST::Block) {
            std::shared_ptr<BlockNode> name = static_pointer_cast<BlockNode>(expr->getValue());
            std::vector<ASTNode> decls = declareUnknownName(name, size, scopeStack, tree);
            newDeclarations.insert(newDeclarations.end(), decls.begin(), decls.end());
        } else if (expr->getValue()->getNodeType() == AST::Expression) {
            std::shared_ptr<ExpressionNode> name = static_pointer_cast<ExpressionNode>(expr->getValue());
            std::vector<ASTNode > decls = declareUnknownExpressionSymbols(name, size, scopeStack, tree);
            newDeclarations.insert(newDeclarations.end(), decls.begin(), decls.end());
        }
    } else {
        if (expr->getLeft()->getNodeType() == AST::Block) {
            std::shared_ptr<BlockNode> name = static_pointer_cast<BlockNode>(expr->getLeft());
            std::vector<ASTNode > decls = declareUnknownName(name, size, scopeStack, tree);
            newDeclarations.insert(newDeclarations.end(), decls.begin(), decls.end());
        } else if (expr->getLeft()->getNodeType() == AST::Expression) {
            std::shared_ptr<ExpressionNode> inner_expr = static_pointer_cast<ExpressionNode>(expr->getLeft());
            std::vector<ASTNode > decls = declareUnknownExpressionSymbols(inner_expr, size, scopeStack, tree);
            newDeclarations.insert(newDeclarations.end(), decls.begin(), decls.end());
        }
        if (expr->getRight()->getNodeType() == AST::Block) {
            std::shared_ptr<BlockNode> name = static_pointer_cast<BlockNode>(expr->getRight());
            std::vector<ASTNode > decls = declareUnknownName(name, size, scopeStack, tree);
            newDeclarations.insert(newDeclarations.end(), decls.begin(), decls.end());
        } else if (expr->getRight()->getNodeType() == AST::Expression) {
            std::shared_ptr<ExpressionNode> inner_expr = static_pointer_cast<ExpressionNode>(expr->getRight());
            std::vector<ASTNode > decls = declareUnknownExpressionSymbols(inner_expr, size, scopeStack, tree);
            newDeclarations.insert(newDeclarations.end(), decls.begin(), decls.end());
        }
    }
    return newDeclarations;
}

std::vector<ASTNode > CodeResolver::declareUnknownFunctionSymbols(std::shared_ptr<FunctionNode> func, QVector<ASTNode > scopeStack, ASTNode tree)
{
    std::vector<ASTNode > newDeclarations;
    vector<std::shared_ptr<PropertyNode>> properties = func->getProperties();

    for (auto property:properties) {
        ASTNode value = property->getValue();
        if (value->getNodeType() == AST::Block) {
            std::shared_ptr<BlockNode> block = static_pointer_cast<BlockNode>(value);
            std::vector<ASTNode > declarations = declareUnknownName(block, 1, scopeStack, tree);
            for (ASTNode  declaration: declarations) {
                tree->addChild(declaration);
            }
        } else  if (value->getNodeType() == AST::Bundle) {
            // Can't autodeclare bundles...
        }
    }

    return newDeclarations;
}

std::shared_ptr<ListNode> CodeResolver::expandNameToList(BlockNode *name, int size)
{
    std::shared_ptr<ListNode> list = std::make_shared<ListNode>(nullptr, name->getFilename().data(), name->getLine());
    for (int i = 0; i < size; i++) {
        std::shared_ptr<ListNode> indexList = std::make_shared<ListNode>(std::make_shared<ValueNode>(i + 1, name->getFilename().data(), name->getLine()),
                                                                         name->getFilename().data(), name->getLine());
        std::shared_ptr<BundleNode> bundle = std::make_shared<BundleNode>(name->getName(), indexList, name->getFilename().data(), name->getLine());
        list->addChild(bundle);
    }
    return list;
}

void CodeResolver::expandNamesToBundles(std::shared_ptr<StreamNode> stream, ASTNode tree)
{
    ASTNode left = stream->getLeft();
    ASTNode right = stream->getRight();
//    ASTNode nextStreamMember;
//    if (right->getNodeType() != AST::Stream) {
//        nextStreamMember = right;
//    } else {
//        nextStreamMember = static_cast<StreamNode *>(right)->getLeft();
//    }

    if (left->getNodeType() == AST::Block) {
        BlockNode *name = static_cast<BlockNode *>(left.get());
        QVector<ASTNode > scope;
        std::shared_ptr<DeclarationNode> block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()), scope, tree);
        int size = 0;
        if (block) {
            if (block->getNodeType() == AST::BundleDeclaration) {
                QList<LangError> errors;
                size = CodeValidator::getBlockDeclaredSize(block, scope, tree, errors);
            } else if (block->getNodeType() == AST::Declaration ) {
                size = 1;
            }
        }
        if (size > 1) {
            std::shared_ptr<ListNode> list = expandNameToList(name, size);
            stream->setLeft(list);
        }
    }
    if (right->getNodeType() == AST::Block) {
        BlockNode *name = static_cast<BlockNode *>(right.get());
        QVector<ASTNode > scope;
        std::shared_ptr<DeclarationNode> block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()), scope, tree, name->getNamespaceList());
        int size = 0;
        if (block) {
            if (block->getNodeType() == AST::BundleDeclaration) {
                QList<LangError> errors;
                size = CodeValidator::getBlockDeclaredSize(block, scope, tree, errors);
            } else if (block->getNodeType() == AST::Declaration ) {
                size = 1;
            }
        }
        if (size > 1) {
            std::shared_ptr<ListNode> list = expandNameToList(name, size);
            stream->setRight(list);
        }
    } else if (right->getNodeType() == AST::Stream) {
        expandNamesToBundles(static_pointer_cast<StreamNode>(right), tree);
    }
}

std::vector<ASTNode > CodeResolver::declareUnknownStreamSymbols(std::shared_ptr<StreamNode> stream, ASTNode previousStreamMember, QVector<ASTNode > localScope, ASTNode tree)
{
    std::vector<ASTNode > newDeclarations;
    ASTNode left = stream->getLeft();
    ASTNode right = stream->getRight();

    ASTNode  nextStreamMember;
    if (right->getNodeType() != AST::Stream) {
        nextStreamMember = right;
    } else {
        nextStreamMember = static_pointer_cast<StreamNode>(right)->getLeft();
    }

    if (left->getNodeType() == AST::Block) {
        std::shared_ptr<BlockNode> name = static_pointer_cast<BlockNode>(left);
        QList<LangError> errors;
        int size = -1;
        if (previousStreamMember) {
            size = CodeValidator::getNodeNumOutputs(previousStreamMember, localScope, m_tree, errors);
        }
        if (size <= 0 && previousStreamMember) { // Look to the right if can't resolve from the left
            // Size for first member should always be 1
            size = CodeValidator::getNodeNumInputs(nextStreamMember, localScope, m_tree, errors);
        }
        if (size <= 0) { // None of the elements in the stream have size
            size = 1;
        }
        std::vector<ASTNode > declarations = declareUnknownName(name, size, localScope, tree);
        newDeclarations.insert(newDeclarations.end(), declarations.begin(), declarations.end());
    } else if (left->getNodeType() == AST::Expression) {
        int size = 1; // FIXME implement size detection for expressions
        std::shared_ptr<ExpressionNode> expr = static_pointer_cast<ExpressionNode>(left);
        std::vector<ASTNode > declarations = declareUnknownExpressionSymbols(expr, size, localScope, tree);
        newDeclarations.insert(newDeclarations.end(), declarations.begin(), declarations.end());
    } else if (left->getNodeType() == AST::Function) {
        std::shared_ptr<FunctionNode> func = static_pointer_cast<FunctionNode>(left);
        std::vector<ASTNode > declarations = declareUnknownFunctionSymbols(func, localScope, tree);
        newDeclarations.insert(newDeclarations.end(), declarations.begin(), declarations.end());
    }

    if (right->getNodeType() == AST::Stream) {
        std::vector<ASTNode > declarations = declareUnknownStreamSymbols(static_pointer_cast<StreamNode>(right), left, localScope, tree);
        newDeclarations.insert(newDeclarations.end(), declarations.begin(), declarations.end());
    } else if (right->getNodeType() == AST::Block) {
        std::shared_ptr<BlockNode> name = static_pointer_cast<BlockNode>(right);
        QList<LangError> errors;
        int size = 0;
        if (left->getNodeType() == AST::Function) {
            auto funcDecl = CodeValidator::findDeclaration(
                        CodeValidator::streamMemberName(left), localScope.toStdVector(), tree);
            size = CodeValidator::getTypeNumOutputs(funcDecl, localScope, m_tree, errors);
        } else {
           size = CodeValidator::getNodeNumOutputs(left, localScope, m_tree, errors);
        }
        if (size <= 0) { // None of the elements in the stream have size
            size = 1;
        }
        std::vector<ASTNode > declarations = declareUnknownName(name, size, localScope, tree);
        newDeclarations.insert(newDeclarations.end(), declarations.begin(), declarations.end());
    } else if (right->getNodeType() == AST::Function) {
        std::shared_ptr<FunctionNode> func = static_pointer_cast<FunctionNode>(right);
        std::vector<ASTNode > declarations = declareUnknownFunctionSymbols(func, localScope, tree);
        newDeclarations.insert(newDeclarations.end(), declarations.begin(), declarations.end());
    }
    return newDeclarations;
}

std::vector<ASTNode > CodeResolver::getModuleStreams(std::shared_ptr<DeclarationNode> module)
{
    std::vector<ASTNode> streams;

    ASTNode streamsNode = module->getPropertyValue("streams");
    Q_ASSERT(streamsNode);
    if (streamsNode->getNodeType() == AST::Stream) {
        streams.push_back(streamsNode);
    } else if (streamsNode->getNodeType() == AST::List) {
        for(ASTNode  node: streamsNode->getChildren()) {
            if (node->getNodeType() == AST::Stream) {
                streams.push_back(node);
            }
        }
    }
    return streams;
}

std::vector<ASTNode> CodeResolver::getModuleBlocks(std::shared_ptr<DeclarationNode> module)
{
    std::vector<ASTNode> blocks;

    ASTNode blocksNode = module->getPropertyValue("blocks");
    Q_ASSERT(blocksNode);
    if (blocksNode->getNodeType() == AST::Declaration
            || blocksNode->getNodeType() == AST::BundleDeclaration) {
        blocks.push_back(blocksNode);
    } else if (blocksNode->getNodeType() == AST::List) {
        for(ASTNode  node: blocksNode->getChildren()) {
            if (node->getNodeType() == AST::Declaration
                    || node->getNodeType() == AST::BundleDeclaration) {
                blocks.push_back(node);
            }
        }
    }
    return blocks;
}

void CodeResolver::declareInternalBlocksForNode(ASTNode node, QVector<ASTNode> subScope)
{
    if (node->getNodeType() == AST::Declaration) {
        std::shared_ptr<DeclarationNode> block = static_pointer_cast<DeclarationNode>(node);
        if (block->getObjectType() == "reaction") {
            std::shared_ptr<DeclarationNode> reactionInput = std::make_shared<DeclarationNode>("_TriggerInput", "mainInputPort", nullptr,__FILE__, __LINE__);
            reactionInput->setPropertyValue("block", std::make_shared<BlockNode>("_Trigger", __FILE__, __LINE__));
            reactionInput->setPropertyValue("name", std::make_shared<ValueNode>(std::string("_trigger"), __FILE__, __LINE__));

            ListNode *ports = static_cast<ListNode *>(block->getPropertyValue("ports").get());
            if (ports && ports->getNodeType() == AST::None) {
                block->replacePropertyValue("ports", std::make_shared<ListNode>(nullptr, __FILE__, __LINE__));
                ports = static_cast<ListNode *>(block->getPropertyValue("ports").get());
            }
            ports->addChild(reactionInput);
        }
        if (block->getObjectType() == "loop") { // We need to define an internal domain for loops
            ListNode *ports = static_cast<ListNode *>(block->getPropertyValue("ports").get());
            if (!ports) {
                block->setPropertyValue("ports", std::make_shared<ValueNode>(__FILE__, __LINE__)); // Make a None node to trigger next branch
            }
            if (ports && ports->getNodeType() == AST::None) {
                block->replacePropertyValue("ports", std::make_shared<ListNode>(nullptr, __FILE__, __LINE__));
                ports = static_cast<ListNode *>(block->getPropertyValue("ports").get());
            }
            ASTNode internalBlocks = block->getPropertyValue("blocks");
//            internalBlocks->addChild(createDomainDeclaration(QString::fromStdString("_OutputDomain")));
        }
        if (block->getObjectType() == "module" || block->getObjectType() == "reaction" || block->getObjectType() == "loop" ) {
            // First insert and resolve input and output domains for main ports. The input port takes the output domain if undefined.

            ASTNode internalBlocks = block->getPropertyValue("blocks");
            if (!internalBlocks || internalBlocks->getNodeType() != AST::List) { // Turn blocks property into list
                block->setPropertyValue("blocks", std::make_shared<ListNode>(internalBlocks, __FILE__, __LINE__));
                internalBlocks = block->getPropertyValue("blocks");
            }
            for (ASTNode node: internalBlocks->getChildren()) {
                subScope << node;
            }

            // Check Output port and declare block if not declared
            std::shared_ptr<DeclarationNode> outputPortBlock = CodeValidator::getMainOutputPortBlock(block);
            ASTNode mainPortsDefaultDomain;
            if (outputPortBlock) {
                ASTNode portBlock = outputPortBlock->getPropertyValue("block");
                auto outputDomain = std::make_shared<PortPropertyNode>(outputPortBlock->getName(),
                                                                       "domain", __FUNCTION__, __LINE__);
                mainPortsDefaultDomain = outputDomain;
                // First give port a block and its declaration if it doesn't have one.
                if (!portBlock || portBlock->getNodeType() == AST::None) {
                    // FIXME  check if there is a block called Output already
                    portBlock = std::make_shared<BlockNode>("Output", __FILE__, __LINE__);
                    outputPortBlock->setPropertyValue("block", portBlock);
                }

                // Add declaration if not there
                // We need to find declaration only in the internal blocks, as port blocks can only
                // be internal.
                auto portBlockDecl = CodeValidator::findDeclaration(
                            CodeValidator::streamMemberName(portBlock), subScope.toStdVector(), nullptr);
                if (!portBlockDecl) {
                    // FIXME this has to check wether block is a bundle and create declaration accordingly
                    portBlockDecl= createSignalDeclaration(QString::fromStdString(CodeValidator::streamMemberName(portBlock)),
                                                           1, subScope);
                    internalBlocks->addChild(portBlockDecl);
                    fillDefaultPropertiesForNode(portBlockDecl);
                }

                // Now check if domain set, if not, set to port domain
                ASTNode domainNode
                        = CodeValidator::getNodeDomain(portBlock,
                                                       QVector<ASTNode>::fromStdVector(internalBlocks->getChildren()),
                                                       m_tree);
                if (!domainNode || domainNode->getNodeType() == AST::None) {
                    if (portBlockDecl) {
                        portBlockDecl->setPropertyValue("domain", mainPortsDefaultDomain);
                    }
                }

            }
            // Check Input port and declare block if not declared, pass default domain if available from output port
            // or set default mode from input port
            std::shared_ptr<DeclarationNode> inputPortBlock = CodeValidator::getMainInputPortBlock(block);
            if (inputPortBlock) {
                ASTNode portBlock = inputPortBlock->getPropertyValue("block");
                auto inputDomain = std::make_shared<PortPropertyNode>(inputPortBlock->getName(),
                                                                       "domain", __FUNCTION__, __LINE__);

                if (!mainPortsDefaultDomain) { // If not set from output port, set here
                    mainPortsDefaultDomain = inputDomain;
                }
                // First give port a block and its declaration if it doesn't have one.
                if (!portBlock || portBlock->getNodeType() == AST::None) {
                    // FIXME  check if there is a block called Output already
                    portBlock = std::make_shared<BlockNode>("Input", __FILE__, __LINE__);
                    outputPortBlock->setPropertyValue("block", portBlock);
                }

                // Add declaration if not there
                // Port blocks can only be internal, so look for declaration only in internal blocks
                auto portBlockDecl = CodeValidator::findDeclaration(
                            CodeValidator::streamMemberName(portBlock), subScope.toStdVector(), nullptr);
                if (!portBlockDecl) {
                    // FIXME this has to check wether block is a bundle and create declaration accordingly
                    portBlockDecl= createSignalDeclaration(QString::fromStdString(CodeValidator::streamMemberName(portBlock)),
                                                           1, subScope);
                    internalBlocks->addChild(portBlockDecl);
                    fillDefaultPropertiesForNode(portBlockDecl);
                }

                // Now check if domain set, if not, set to port domain
                ASTNode domainNode
                        = CodeValidator::getNodeDomain(portBlock,
                                                       QVector<ASTNode>::fromStdVector(internalBlocks->getChildren()),
                                                       m_tree);
                if (!domainNode || domainNode->getNodeType() == AST::None) {
                    if (portBlockDecl) {
                        portBlockDecl->setPropertyValue("domain", mainPortsDefaultDomain);
                    }
                }
            }
//            if (!mainPortsDefaultDomain) { // Fallback
//                mainPortsDefaultDomain = std::make_shared<AST>(AST::None, __FILE__, __LINE__);
//            }
//            Q_ASSERT(moduleDefaultDomain);

            // Find OutputPort node or give a default Block and domain
            ListNode *ports = static_cast<ListNode *>(block->getPropertyValue("ports").get());

            internalBlocks = block->getPropertyValue("blocks");

            // Then go through ports autodeclaring blocks
            if (ports->getNodeType() == AST::List) {
                for (ASTNode port : ports->getChildren()) {
                    Q_ASSERT(port->getNodeType() == AST::Declaration);
                    auto portDeclaration = static_pointer_cast<DeclarationNode>(port);

                    // Properties that we need to auto-declare for
                    ASTNode blockPortValue = portDeclaration->getPropertyValue("block");
                    auto portDomain = std::make_shared<PortPropertyNode>(portDeclaration->getName(),
                                                                         "domain", __FUNCTION__, __LINE__);

                    if (blockPortValue) {
//                        // Now do auto declaration of IO blocks if not declared.
                        Q_ASSERT(blockPortValue->getNodeType() == AST::Block || blockPortValue->getNodeType() == AST::None); // Catch on debug but fail gracefully on release
                        if (blockPortValue->getNodeType() == AST::Block) {
                            std::shared_ptr<BlockNode> nameNode = static_pointer_cast<BlockNode>(blockPortValue);
                            string name = nameNode->getName();
                            std::shared_ptr<DeclarationNode> blockDecl = CodeValidator::findDeclaration(QString::fromStdString(name), QVector<ASTNode >(), internalBlocks);
                            if (!blockDecl) { // If block is given but not declared, declare and assing port domain
                                int size = 1;
//                                if (sizePortValue->getNodeType() == AST::Int) {
//                                    size = static_pointer_cast<ValueNode>(sizePortValue)->getIntValue();
//                                }
                                blockDecl = createSignalDeclaration(QString::fromStdString(name), size, subScope);
                                blockDecl->replacePropertyValue("rate", std::make_shared<ValueNode>(__FILE__, __LINE__));
                                internalBlocks->addChild(blockDecl);
                                // TODO This default needs to be done per instance
                                ASTNode portDefault = portDeclaration->getPropertyValue("default");
                                if (portDefault && portDefault->getNodeType() != AST::None) {
                                    Q_ASSERT(blockDecl->getPropertyValue("default"));
                                    blockDecl->replacePropertyValue("default", portDefault);
                                }
                            }
                            // set its domain to the port domain if not set
                            auto declType = CodeValidator::findTypeDeclaration(blockDecl, subScope, m_tree);
                            if (declType) {
                                auto inheritList = CodeValidator::getInheritedTypes(declType,subScope, m_tree);
                                bool inheritsDomainType = false;
                                for (auto inherits: inheritList) {
                                    auto typeName = inherits->getPropertyValue("typeName");
                                    if (typeName->getNodeType() == AST::String) {
                                        if (static_pointer_cast<ValueNode>(typeName)->getStringValue() == "domainMember") {
                                            inheritsDomainType = true;
                                            break;
                                        }
                                    }
                                }
                                if (inheritsDomainType) {
                                    ASTNode blockDomain = blockDecl->getDomain();
                                    if (!blockDomain || blockDomain->getNodeType() == AST::None) {
                                        blockDecl->setPropertyValue("domain",portDomain);
                                    }
                                }
                            }
                        } else if (blockPortValue->getNodeType() == AST::None) {
                            // TODO Make sure block name not being used
                            string defaultName = portDeclaration->getName() + "_Block";
                            std::shared_ptr<BlockNode> name = std::make_shared<BlockNode>(defaultName, __FILE__, __LINE__);
                            portDeclaration->replacePropertyValue("block", name);
                            std::shared_ptr<DeclarationNode> newSignal = CodeValidator::findDeclaration(QString::fromStdString(defaultName), QVector<ASTNode >(), internalBlocks);
                            if (!newSignal) {
                                newSignal = createSignalDeclaration(QString::fromStdString(defaultName), 1, subScope);
                                internalBlocks->addChild(newSignal);
                                fillDefaultPropertiesForNode(newSignal);
                                ASTNode blockDomain = newSignal->getDomain();
                                if (!blockDomain || blockDomain->getNodeType() == AST::None) {
                                    newSignal->setPropertyValue("domain",portDomain);
                                }
                            }
                        }
                    }


                }
            } else if (ports->getNodeType() == AST::None) {
                // If port list is None, then ignore
            } else {
                qDebug() << "ERROR! ports property must be a list or None!";
            }

            // Go through child blocks and autodeclare for internal modules and reactions
//            ASTNode internalBlocks = block->getPropertyValue("blocks");
            for(ASTNode node : internalBlocks->getChildren()) {
                declareInternalBlocksForNode(node, subScope);
            }
        }
    }

}

void CodeResolver::resolveStreamSymbols()
{
    // FIMXE we need to resolve the streams in the root tree in reverse order as we do for streams within modules.
    for(ASTNode node : m_tree->getChildren()) {
        if(node->getNodeType() == AST::Stream) {
            std::shared_ptr<StreamNode> stream = static_pointer_cast<StreamNode>(node);
            std::vector<ASTNode > declarations = declareUnknownStreamSymbols(stream, nullptr, QVector<ASTNode >(), m_tree); // FIXME Is this already done in expandParallelFunctions?
            for(ASTNode decl: declarations) {
                m_tree->addChild(decl);
            }
            expandNamesToBundles(stream, m_tree);
        } else if(node->getNodeType() == AST::Declaration) {
            std::shared_ptr<DeclarationNode> block = static_pointer_cast<DeclarationNode>(node);
            if (block->getObjectType() == "module" || block->getObjectType() == "reaction" || block->getObjectType() == "loop") {
                std::vector<ASTNode > streams = getModuleStreams(block);
                QVector<ASTNode > scopeStack;
                ASTNode blocks = block->getPropertyValue("blocks");
                if (blocks && blocks->getNodeType() == AST::List) {
                    std::shared_ptr<ListNode> blockList = static_pointer_cast<ListNode>(blocks);
                    for (ASTNode node: blockList->getChildren()) {
                        scopeStack.push_back(node);
                    }
                }
                auto rit = streams.rbegin();
                while (rit != streams.rend()) {
                    const ASTNode streamNode = *rit;
                    if (streamNode->getNodeType() == AST::Stream) {
                        std::shared_ptr<StreamNode> stream = static_pointer_cast<StreamNode>(streamNode);
                        std::vector<ASTNode > declarations = declareUnknownStreamSymbols(stream, nullptr, scopeStack, m_tree);
                        std::shared_ptr<ListNode> blockList = static_pointer_cast<ListNode>(block->getPropertyValue("blocks"));
                        Q_ASSERT(blockList && blockList->getNodeType() == AST::List);
                        for(ASTNode decl: declarations) {
                            blockList->addChild(decl);
                            scopeStack.push_back(decl);
                        }
                    }
                    rit++;
                }
            }
        }
    }
}

void CodeResolver::resolveConstants()
{
    QVector<ASTNode > children = QVector<ASTNode >::fromStdVector(m_tree->getChildren());
    for(auto override = m_systemConfig.overrides["all"].constBegin(); override != m_systemConfig.overrides["all"].constEnd() ; ++override) {
//        qDebug() << override.key();
       std::shared_ptr<DeclarationNode> decl = CodeValidator::findDeclaration(override.key(), QVector<ASTNode>(), m_tree);
       if (decl) {
           if (decl->getObjectType() == "constant") {
               if (override.value().type() == QVariant::String) {
                   decl->replacePropertyValue("value", std::make_shared<ValueNode>(override.value().toString().toStdString(), __FILE__, __LINE__));
               } else if (override.value().type() == QVariant::Int) {
                   decl->replacePropertyValue("value", std::make_shared<ValueNode>(override.value().toInt(), __FILE__, __LINE__));
               }
           } else {
               qDebug() << "WARNING: Ignoring configuration override '" + override.key() + "'. Not constant.";
           }
       } else {
           qDebug() << "WARNING: Configuration override not found: " + override.key();
       }
    }
    for(ASTNode node : children) {
        resolveConstantsInNode(node, children);
    }
}

void CodeResolver::processResets()
{
    processResetForNode(m_tree, m_tree, std::make_shared<ListNode>(nullptr, __FILE__, __LINE__));
}

std::shared_ptr<ValueNode> CodeResolver::reduceConstExpression(std::shared_ptr<ExpressionNode> expr, QVector<ASTNode > scope, ASTNode tree)
{
    ASTNode left = nullptr, right = nullptr;
    bool isConstant;

    if (!expr->isUnary()) {
        left = expr->getLeft();
    } else {
        left = expr->getValue();
    }

    std::shared_ptr<ValueNode> newValue = resolveConstant(left, scope);
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
        std::shared_ptr<ValueNode> result = nullptr;
        switch (expr->getExpressionType()) {
        case ExpressionNode::Multiply:
            result = multiply(static_pointer_cast<ValueNode>(left), static_pointer_cast<ValueNode>(right));
            break;
        case ExpressionNode::Divide:
            result = divide(static_pointer_cast<ValueNode>(left), static_pointer_cast<ValueNode>(right));
            break;
        case ExpressionNode::Add:
            result = add(static_pointer_cast<ValueNode>(left), static_pointer_cast<ValueNode>(right));
            break;
        case ExpressionNode::Subtract:
            result = subtract(static_pointer_cast<ValueNode>(left), static_pointer_cast<ValueNode>(right));
            break;
        case ExpressionNode::And:
            result = logicalAnd(static_pointer_cast<ValueNode>(left), static_pointer_cast<ValueNode>(right));
            break;
        case ExpressionNode::Or:
            result = logicalOr(static_pointer_cast<ValueNode>(left), static_pointer_cast<ValueNode>(right));
            break;
        case ExpressionNode::UnaryMinus:
            result = unaryMinus(static_pointer_cast<ValueNode>(left));
            break;
        case ExpressionNode::LogicalNot:
            result = logicalNot(static_pointer_cast<ValueNode>(left));
            break;
        default:
            Q_ASSERT(0 == 1); // Should never get here
            break;
        }
        if(result) {
            return result;
        }
    }
    return nullptr;
}

std::shared_ptr<ValueNode> CodeResolver::resolveConstant(ASTNode value, QVector<ASTNode > scope)
{
    std::shared_ptr<ValueNode> newValue = nullptr;
    if(value->getNodeType() == AST::Expression) {
        std::shared_ptr<ExpressionNode> expr = static_pointer_cast<ExpressionNode>(value);
        newValue = reduceConstExpression(expr, scope, m_tree);
        return newValue;
    } else if(value->getNodeType() == AST::Block) {
        BlockNode *name = static_cast<BlockNode *>(value.get());
        std::shared_ptr<DeclarationNode> block = CodeValidator::findDeclaration(QString::fromStdString(name->getName()), scope, m_tree);
        if (block && block->getNodeType() == AST::Declaration && block->getObjectType() == "constant") { // Size == 1
//            string namespaceValue = name->getScopeAt(0);
            ASTNode declarationNamespace = block->getPropertyValue("namespace");
//            if (namespaceValue.size() == 0 || namespaceValue)
            ASTNode blockValue = block->getPropertyValue("value");
            if (blockValue->getNodeType() == AST::Int || blockValue->getNodeType() == AST::Real
                     || blockValue->getNodeType() == AST::String ) {
                return static_pointer_cast<ValueNode>(blockValue);
            }
            newValue = resolveConstant(block->getPropertyValue("value"), scope);
            return newValue;
        }
    } else if (value->getNodeType() == AST::Bundle) {

    } else if(value->getNodeType() == AST::PortProperty) {
        PortPropertyNode *propertyNode = static_cast<PortPropertyNode *>(value.get());
        std::shared_ptr<DeclarationNode> block = CodeValidator::findDeclaration(QString::fromStdString(propertyNode->getPortName()), scope, m_tree);
        if (block) {
            ASTNode propertyValue = block->getPropertyValue(propertyNode->getName());
            if (propertyValue) {
//                || propertyValue->getNodeType() == AST::Block || propertyValue->getNodeType() == AST::Bundle
                if (propertyValue->getNodeType() == AST::Int || propertyValue->getNodeType() == AST::Real
                        || propertyValue->getNodeType() == AST::String ) {
                    return static_pointer_cast<ValueNode>(propertyValue);
                }
            }
        }
    }
    return nullptr;
}

void CodeResolver::resolveConstantsInNode(ASTNode node, QVector<ASTNode > scope)
{
    if (node->getNodeType() == AST::Stream) {
        std::shared_ptr<StreamNode> stream = static_pointer_cast<StreamNode>(node);
        resolveConstantsInNode(stream->getLeft(), scope);
        if (stream->getLeft()->getNodeType() == AST::Expression) {
            std::shared_ptr<ExpressionNode> expr = static_pointer_cast<ExpressionNode>(stream->getLeft());
            std::shared_ptr<ValueNode> newValue = reduceConstExpression(expr, scope, m_tree);
            if (newValue) {
                stream->setLeft(newValue);
            }
        } else if(stream->getLeft()->getNodeType() == AST::PortProperty) {
            std::shared_ptr<PortPropertyNode> propertyNode = static_pointer_cast<PortPropertyNode>(stream->getLeft());
            std::shared_ptr<DeclarationNode> block = CodeValidator::findDeclaration(QString::fromStdString(propertyNode->getPortName()), scope, m_tree);
            if (block) {
                ASTNode property = block->getPropertyValue(propertyNode->getName());
                if (property) { // First replace if pointing to a name
                    if (property->getNodeType() == AST::Block || property->getNodeType() == AST::Bundle) {
                        stream->setLeft(property);
                    }
                    std::shared_ptr<ValueNode> newValue = resolveConstant(stream->getLeft(), scope);
                    if (newValue) {
                        stream->setLeft(newValue);
                    }
                }
            }
        }
        resolveConstantsInNode(stream->getRight(), scope);
    } else if (node->getNodeType() == AST::Function) {
        std::shared_ptr<FunctionNode> func = static_pointer_cast<FunctionNode>(node);
        vector<std::shared_ptr<PropertyNode> > properties = func->getProperties();
        for(auto property : properties) {
            std::shared_ptr<ValueNode> newValue = resolveConstant(property->getValue(), scope);
            if (newValue) {
                property->replaceValue(newValue);
            }
        }
    } else if(node->getNodeType() == AST::Declaration) {
        std::shared_ptr<DeclarationNode> decl = static_pointer_cast<DeclarationNode>(node);
        vector<std::shared_ptr<PropertyNode>> properties = decl->getProperties();
        std::shared_ptr<ListNode> internalBlocks = static_pointer_cast<ListNode>(decl->getPropertyValue("blocks"));
        if ((decl->getObjectType() == "module" || decl->getObjectType() == "reaction" || decl->getObjectType() == "loop") && internalBlocks) {
            if (internalBlocks->getNodeType() == AST::List) {
                scope = QVector<ASTNode >::fromStdVector(internalBlocks->getChildren()) + scope;
            }
        }
        if (decl->getObjectType() != "type") {
            // FIXME This is a hack to protect constants that are context dependent. Should the language mark this?
            for(std::shared_ptr<PropertyNode> property : properties) {
                resolveConstantsInNode(property->getValue(), scope);
                std::shared_ptr<ValueNode> newValue = resolveConstant(property->getValue(), scope);
                if (newValue) {
                    property->replaceValue(newValue);
                }
            }
        }
    } else if(node->getNodeType() == AST::BundleDeclaration) {
        std::shared_ptr<DeclarationNode> block = static_pointer_cast<DeclarationNode>(node);
        vector<std::shared_ptr<PropertyNode>> properties = block->getProperties();
        std::shared_ptr<ListNode> internalBlocks = static_pointer_cast<ListNode>(block->getPropertyValue("blocks"));
        if (internalBlocks) {
            if (internalBlocks->getNodeType() == AST::List) {
                scope << QVector<ASTNode >::fromStdVector(internalBlocks->getChildren());
            }
        }
        for (std::shared_ptr<PropertyNode> property : properties) {
            resolveConstantsInNode(property->getValue(), scope);
            std::shared_ptr<ValueNode> newValue = resolveConstant(property->getValue(), scope);
            if (newValue) {
                property->replaceValue(newValue);
            }
        }
        std::shared_ptr<BundleNode> bundle = block->getBundle();
//        std::shared_ptr<ListNode> indexList = bundle->index();
//        vector<ASTNode > elements = indexList->getChildren();
//        for (ASTNode element : elements) {
//            if (element->getNodeType() == AST::Expression) {
//                std::shared_ptr<ExpressionNode> expr = static_pointer_cast<ExpressionNode>(element);
//                resolveConstantsInNode(expr, scope);
//                std::shared_ptr<ValueNode> newValue = reduceConstExpression(expr, scope, m_tree);
//                if (newValue) {
//                    indexList->replaceMember(newValue, element);
//                }
//            } else if (element->getNodeType() == AST::Block) {
//                std::shared_ptr<ValueNode> newValue = resolveConstant(element, scope);
//                if (newValue) {
//                    indexList->replaceMember(newValue, element);
//                }
//            }
//        }
        resolveConstantsInNode(bundle->index(), scope);
    } else if(node->getNodeType() == AST::Expression) {
        std::shared_ptr<ExpressionNode> expr = static_pointer_cast<ExpressionNode>(node);
        if (expr->isUnary()) {
            resolveConstantsInNode(expr->getValue(), scope);
            if (expr->getValue()->getNodeType() == AST::Expression) {
                std::shared_ptr<ExpressionNode> exprValue = static_pointer_cast<ExpressionNode>(expr->getValue());
                std::shared_ptr<ValueNode>newValue = reduceConstExpression(exprValue, scope, m_tree);
                if (newValue) {
                    exprValue->replaceValue(newValue);
                }
            }
        } else {
            resolveConstantsInNode(expr->getLeft(), scope);
            resolveConstantsInNode(expr->getRight(), scope);
            if (expr->getLeft()->getNodeType() == AST::Expression) {
                std::shared_ptr<ExpressionNode> exprValue = static_pointer_cast<ExpressionNode>(expr->getLeft());
                std::shared_ptr<ValueNode> newValue = reduceConstExpression(exprValue, scope, m_tree);
                if (newValue) {
                    expr->replaceLeft(newValue);
                }
            }
            if (expr->getRight()->getNodeType() == AST::Expression) {
                std::shared_ptr<ExpressionNode> exprValue = static_pointer_cast<ExpressionNode>(expr->getRight());
                std::shared_ptr<ValueNode> newValue = reduceConstExpression(exprValue, scope, m_tree);
                if (newValue) {
                    expr->replaceRight(newValue);
                }
            }
        }
    } else if(node->getNodeType() == AST::List) {
        std::map<ASTNode , ASTNode> replaceMap;
        for (ASTNode element : node->getChildren()) {
            resolveConstantsInNode(element, scope);
            std::shared_ptr<ValueNode> newValue;
            if (element->getNodeType() == AST::Expression) {
                newValue = reduceConstExpression(std::static_pointer_cast<ExpressionNode>(element), scope, m_tree);

            } else {
                newValue = resolveConstant(element, scope);
            }
            if (newValue) {
                replaceMap[element] = newValue;
            }
        }
        std::shared_ptr<ListNode> list = static_pointer_cast<ListNode>(node);
        for (auto& values: replaceMap) {
            list->replaceMember(values.second, values.first);
        }

    } else if(node->getNodeType() == AST::Bundle) {
        std::shared_ptr<BundleNode> bundle = static_pointer_cast<BundleNode>(node);
        std::shared_ptr<ListNode> index = bundle->index();
        resolveConstantsInNode(index, scope);
    }
}

void CodeResolver::processResetForNode(ASTNode thisScope, ASTNode streamScope, ASTNode upperScope)
{
    map<std::shared_ptr<DeclarationNode>, string> resetMap; // Key is variable name, value is reset symbol
    for(ASTNode node : thisScope->getChildren()) {
        if (node->getNodeType() == AST::Declaration) {
            shared_ptr<DeclarationNode> decl = static_pointer_cast<DeclarationNode>(node);
            if (decl->getObjectType() == "signal") {
                ASTNode resetValue = decl->getPropertyValue("reset");
                if (resetValue && resetValue->getNodeType() == AST::Block) {
                    resetMap[decl] = static_pointer_cast<BlockNode>(resetValue)->getName();
                }
            } else if (decl->getObjectType() == "module"
                       || decl->getObjectType() == "reaction") {
                ASTNode blocks = decl->getPropertyValue("blocks");
                ASTNode streamScope = decl->getPropertyValue("streams");
                ASTNode newScope = thisScope->deepCopy();
                for (ASTNode node : upperScope->getChildren()) { // Append upper scope to this scope
                    newScope->addChild(node);
                }
                processResetForNode(blocks, streamScope, newScope);
            }
        }
    }
    for(const auto& pair : resetMap ) {
        std::string reactionName = "_" + pair.first->getName() + "Reset";
        std::shared_ptr<DeclarationNode> newReaction
                = std::make_shared<DeclarationNode>(reactionName,
                                                    "reaction",
                                                    nullptr,
                                                    __FILE__, __LINE__);
        std::shared_ptr<ListNode> streamList = std::make_shared<ListNode>(nullptr, __FILE__, __LINE__);

        std::shared_ptr<StreamNode> stream
                = std::make_shared<StreamNode>(pair.first->getPropertyValue("default"),
                                               std::make_shared<BlockNode>(pair.first->getName(), __FILE__, __LINE__), __FILE__, __LINE__);
        fillDefaultPropertiesForNode(stream);
        streamList->addChild(stream);
        newReaction->setPropertyValue("streams", streamList);
        newReaction->setPropertyValue("blocks", std::make_shared<ListNode>(nullptr, __FILE__, __LINE__));
        newReaction->setPropertyValue("ports", std::make_shared<ListNode>(nullptr, __FILE__, __LINE__));
        newReaction->setPropertyValue("domain", pair.first->getDomain());
        thisScope->addChild(newReaction);

        // Find where to put the triggering stream
        vector<int> positions;
        vector<ASTNode> childStreams = streamScope->getChildren();
        for(size_t i = 0; i < childStreams.size(); i++) {
            ASTNode &node = childStreams[i];
            if (node->getNodeType() == AST::Stream) {
                bool triggerInStream = false;
                std::shared_ptr<StreamNode> stream = static_pointer_cast<StreamNode>(node);
                // We need to skip the first stream member as we need to check when the trigger
                // is written to.
                ASTNode left, right;
                right = stream->getRight();
                if (right->getNodeType() == AST::Stream) {
                    left = static_pointer_cast<StreamNode>(right)->getLeft();
                    right = static_pointer_cast<StreamNode>(right)->getRight();
                } else {
                    left = right;
                    right = nullptr;
                }
                while(left) {
                    if (left->getNodeType() == AST::Block) {
                        std::shared_ptr<BlockNode> block = static_pointer_cast<BlockNode>(left);
                        if (block->getName() == pair.second) {
                            triggerInStream = true;
                            break;
                        }
                    }
                    if (right) {
                        if (right->getNodeType() == AST::Stream) {
                            left = static_pointer_cast<StreamNode>(right)->getLeft();
                            right = static_pointer_cast<StreamNode>(right)->getRight();
                        } else {
                            left = right;
                            right = nullptr;
                        }
                    } else {
                        left = nullptr; // End of stream
                    }
                }

                if(triggerInStream) {
                    positions.push_back(i + 1);
                }
            }
        }

        // Insert triggering streams in right place
        int numInsertions = 0;
        for (int pos : positions) {
            childStreams.insert(childStreams.begin() + pos + numInsertions++,
                            std::make_shared<StreamNode>(std::make_shared<BlockNode>(pair.second, __FILE__, __LINE__),
                                                         std::make_shared<FunctionNode>(reactionName,
                                                                                        std::make_shared<ListNode>(nullptr, __FILE__, __LINE__),
                                                                                        __FILE__, __LINE__), __FILE__, __LINE__)
                    );
        }
        streamScope->setChildren(childStreams);

    }
}

void CodeResolver::propagateDomainsForNode(ASTNode node, QVector<ASTNode > scopeStack)
{
    if (node->getNodeType() == AST::Stream) {
        ASTNode contextDomain = mContextDomainStack.back();
        resolveDomainsForStream(static_pointer_cast<StreamNode>(node), scopeStack, contextDomain);
    } else if (node->getNodeType() == AST::Declaration) {
        std::shared_ptr<DeclarationNode> module = static_pointer_cast<DeclarationNode>(node);
        if (module->getObjectType() == "module" || module->getObjectType() == "reaction" || module->getObjectType() == "loop") {
            vector<ASTNode > streamsNode = getModuleStreams(module);
            vector<ASTNode >::reverse_iterator streamIt = streamsNode.rbegin();

            vector<ASTNode > blocks = getModuleBlocks(module);
            ASTNode ports = module->getPropertyValue("ports");
            scopeStack = QVector<ASTNode>::fromStdVector(blocks) + scopeStack; // Prepend internal scope
            if (ports && ports->getNodeType() == AST::List) { // We need to add ports to stack because users might need to query their properties e.g. Port.domain
                scopeStack = QVector<ASTNode>::fromStdVector(ports->getChildren()) + scopeStack;
            }

//            scopeStack << CodeValidator::getBlockSubScope(module);

            auto contextDomainNode = getModuleContextDomain(module);

//            std::shared_ptr<DeclarationNode> contextDomainDecl = CodeValidator::findDeclaration(QString::fromStdString(contextDomainName),
//                                                                                                scopeStack,
//                                                                                                m_tree);

            while(streamIt != streamsNode.rend()) {
                const ASTNode streamNode = *streamIt;
                if (streamNode->getNodeType() == AST::Stream) {
                    resolveDomainsForStream(static_pointer_cast<StreamNode>(streamNode), scopeStack, contextDomainNode);
                } else {
                    qDebug() << "ERROR: Expecting stream.";
                }
                streamIt++;
            }

            // Now go forward to process anything that was missed
            vector<ASTNode >::iterator streamIt2 = streamsNode.begin();
            while(streamIt2 != streamsNode.end()) {
                const ASTNode streamNode = *streamIt2;
                if (streamNode->getNodeType() == AST::Stream) {
                    resolveDomainsForStream(static_pointer_cast<StreamNode>(streamNode), scopeStack, contextDomainNode);
                } else {
                    qDebug() << "ERROR: Expecting stream.";
                }
                streamIt2++;
            }
//            scopeStack << QVector<ASTNode>::fromStdVector(moduleBlocks);
            for (auto block: blocks) {
                propagateDomainsForNode(block, scopeStack);
            }
//            if (contextDomainDecl) {
//                setContextDomain(blocks, contextDomainDecl);
//            }
        }
    }
}

std::shared_ptr<ValueNode> CodeResolver::multiply(std::shared_ptr<ValueNode>  left, std::shared_ptr<ValueNode>  right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return std::make_shared<ValueNode>(left->getIntValue() * right->getIntValue(), left->getFilename().data(),left->getLine());
    } else { // Automatic casting from int to real
        Q_ASSERT((left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Int)
                 || (left->getNodeType() == AST::Int &&  right->getNodeType() == AST::Real)
                 || (left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Real));
        return std::make_shared<ValueNode>(left->toReal() * right->toReal(), left->getFilename().data(), left->getLine());
    }
}

std::shared_ptr<ValueNode>  CodeResolver::divide(std::shared_ptr<ValueNode>  left, std::shared_ptr<ValueNode>  right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return std::make_shared<ValueNode>(left->getIntValue() / right->getIntValue(), left->getFilename().data(), left->getLine());
    } else { // Automatic casting from int to real
        Q_ASSERT((left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Int)
                 || (left->getNodeType() == AST::Int &&  right->getNodeType() == AST::Real)
                 || (left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Real));
        return std::make_shared<ValueNode>(left->toReal() / right->toReal(), left->getFilename().data(), left->getLine());
    }
}

std::shared_ptr<ValueNode>  CodeResolver::add(std::shared_ptr<ValueNode>  left, std::shared_ptr<ValueNode>  right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return std::make_shared<ValueNode>(left->getIntValue() + right->getIntValue(), left->getFilename().data(), left->getLine());
    } else { // Automatic casting from int to real
        Q_ASSERT((left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Int)
                 || (left->getNodeType() == AST::Int &&  right->getNodeType() == AST::Real)
                 || (left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Real));
        return std::make_shared<ValueNode>(left->toReal() + right->toReal(), left->getFilename().data(), left->getLine());
    }
}

std::shared_ptr<ValueNode>  CodeResolver::subtract(std::shared_ptr<ValueNode>  left, std::shared_ptr<ValueNode>  right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return std::make_shared<ValueNode>(left->getIntValue() - right->getIntValue(), left->getFilename().data(), left->getLine());
    } else { // Automatic casting from int to real
        Q_ASSERT((left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Int)
                 || (left->getNodeType() == AST::Int &&  right->getNodeType() == AST::Real)
                 || (left->getNodeType() == AST::Real &&  right->getNodeType() == AST::Real));
        return std::make_shared<ValueNode>(left->toReal() - right->toReal(), left->getFilename().data(), left->getLine());
    }
}

std::shared_ptr<ValueNode>  CodeResolver::unaryMinus(std::shared_ptr<ValueNode>  value)
{
    if (value->getNodeType() == AST::Int) {
        return std::make_shared<ValueNode>(- value->getIntValue(), value->getFilename().data(), value->getLine());
    } else if (value->getNodeType() == AST::Real){
        return std::make_shared<ValueNode>(- value->getRealValue(), value->getFilename().data(), value->getLine());
    }
    return nullptr;
}

std::shared_ptr<ValueNode>  CodeResolver::logicalAnd(std::shared_ptr<ValueNode>  left, std::shared_ptr<ValueNode>  right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return std::make_shared<ValueNode>(left->getIntValue() & right->getIntValue(), left->getFilename().data(), left->getLine());
    } else if (left->getNodeType() == AST::Switch && right->getNodeType() == AST::Switch) {
        return std::make_shared<ValueNode>(left->getSwitchValue() == right->getSwitchValue(), left->getFilename().data(), left->getLine());
    }
    return nullptr;
}

std::shared_ptr<ValueNode>  CodeResolver::logicalOr(std::shared_ptr<ValueNode>  left, std::shared_ptr<ValueNode>  right)
{
    if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
        return std::make_shared<ValueNode>(left->getIntValue() | right->getIntValue(), left->getFilename().data(), left->getLine());
    } else if (left->getNodeType() == AST::Switch && right->getNodeType() == AST::Switch) {
        return std::make_shared<ValueNode>(left->getSwitchValue() == right->getSwitchValue(), left->getFilename().data(), left->getLine());
    }
    return nullptr;
}

std::shared_ptr<ValueNode>  CodeResolver::logicalNot(std::shared_ptr<ValueNode>  value)
{
    if (value->getNodeType() == AST::Int) {
        return std::make_shared<ValueNode>(~ (value->getIntValue()), value->getFilename().data(), value->getLine());
    } else if (value->getNodeType() == AST::Switch) {
        return std::make_shared<ValueNode>(!value->getSwitchValue(), value->getFilename().data(), value->getLine());
    }
    return nullptr;
}

//void CodeResolver::terminateStackWithBridge(ASTNode node, QVector<ASTNode > &streams, QVector<ASTNode > &stack, QVector<ASTNode > &scopeStack)
//{
//    int size = CodeValidator::getNodeSize(node, scopeStack, m_tree);
//    std::string connectorName = "_B_" + std::to_string(m_connectorCounter++);
//    ASTNode closingName = nullptr;
//    std::shared_ptr<StreamNode> newStream = nullptr;
//    ASTNode newStart = nullptr;
//    vector<ASTNode > newDeclarations;
//    ASTNode outDomain = CodeValidator::getNodeDomain(node, scopeStack, m_tree);
//    if (stack.size() > 0) {
//        ASTNode &stackBack = stack.back();
//        if  (stackBack->getNodeType() == AST::List) { // Slice list

//            bool listIsConst = true;
//            for (auto child: stackBack->getChildren()) {
//                if (!(child->getNodeType() == AST::Int
//                      || child->getNodeType() == AST::Real
//                      || child->getNodeType() == AST::String
//                      || child->getNodeType() == AST::Switch)) {
//                    listIsConst = false;
//                    break;
//                }
//            }
//            if (listIsConst) {
//                stack << node; // Make direct connection.
//                return;
//            }
//            closingName = std::make_shared<ListNode>(nullptr, "", node->getLine());
//            newStart = std::make_shared<ListNode>(nullptr, "", node->getLine());
//            // Set the domain for all members of list
//            for (unsigned int i = 0; i < stackBack->getChildren().size(); i++) {
//                string listConnectorName = connectorName + "_" + std::to_string(i);
//                std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(CodeValidator::streamMemberName(stack.back()->getChildren()[i], scopeStack, m_tree),
//                                                                                              scopeStack, m_tree);
//                if (declaration) {
//                    string type = declaration->getObjectType();
//                    ASTNode valueNode = declaration->getPropertyValue("default");
//                    if (!valueNode) {
//                        valueNode =  std::make_shared<ValueNode>(0, __FILE__, __LINE__);
//                    }
//                    std::string nodeDomainName = CodeValidator::getNodeDomainName(stackBack->getChildren()[i], scopeStack, m_tree);
//                    if (nodeDomainName.size() > 0) {
//                        newDeclarations.push_back(createSignalBridge(listConnectorName, nodeDomainName,
//                                                                     valueNode,
//                                                                     std::make_shared<BlockNode>(nodeDomainName, __FILE__, __LINE__), outDomain,
//                                                                     stackBack->getChildren()[i]->getFilename(), stackBack->getChildren()[i]->getLine(),
//                                                                     1, type));
//                        closingName->addChild(std::make_shared<BlockNode>(listConnectorName, __FILE__, __LINE__));
//                        newStart->addChild(std::make_shared<BlockNode>(listConnectorName, __FILE__, __LINE__));
//                        std::shared_ptr<StreamNode> newStream = std::make_shared<StreamNode>(stack.back()->getChildren()[i],
//                                                                                             std::make_shared<BlockNode>(listConnectorName, __FILE__, __LINE__),
//                                                                                             declaration->getFilename().c_str(), declaration->getLine());
//                        newDeclarations.push_back(newStream);

//                    } else if (stackBack->getChildren()[i]->getNodeType() == AST::Int
//                               || stackBack->getChildren()[i]->getNodeType() == AST::Real
//                               || stackBack->getChildren()[i]->getNodeType() == AST::String
//                               || stackBack->getChildren()[i]->getNodeType() == AST::Switch ){
//                        std::shared_ptr<DeclarationNode> constDeclaration = createConstantDeclaration(listConnectorName, stackBack->getChildren()[i]);
//                        newDeclarations.push_back(constDeclaration);
//                        closingName->addChild(std::make_shared<BlockNode>(listConnectorName, __FILE__, __LINE__));
//                        newStart->addChild(std::make_shared<BlockNode>(listConnectorName, __FILE__, __LINE__));
//                    }
////                    ASTNode valueNode = declaration->getPropertyValue("default");
////                    if (!valueNode) {
////                        valueNode =  std::make_shared<ValueNode>(0, __FILE__, __LINE__);
////                    }
////                    QString memberName = CodeValidator::streamMemberName(node, scopeStack, m_tree);
////                    std::shared_ptr<DeclarationNode> nextDecl = CodeValidator::findDeclaration(memberName, scopeStack, m_tree);
////                    if (nextDecl) {
////                        newDeclarations.push_back(createSignalBridge(listConnectorName, memberName.toStdString(), valueNode,
////                                                                     declaration->getDomain(), nextDecl->getDomain(),
////                                                                     declaration->getFilename(), declaration->getLine()));
////                    } else {
////                        std::shared_ptr<ValueNode> noneValue = std::make_shared<ValueNode>(__FILE__, __LINE__);
////                        newDeclarations.push_back(createSignalBridge(listConnectorName, memberName.toStdString(), valueNode,
////                                                                     declaration->getDomain(), noneValue,
////                                                                     declaration->getFilename(), declaration->getLine()));
////                    }
////                    std::shared_ptr<StreamNode> newStream = std::make_shared<StreamNode>(stack.back()->getChildren()[i],
////                                                                                         std::make_shared<BlockNode>(listConnectorName, __FILE__, __LINE__),
////                                                                                         declaration->getFilename().c_str(), declaration->getLine());
////                    newDeclarations.push_back(newStream);
////                    closingName->addChild(std::make_shared<BlockNode>(listConnectorName, __FILE__, __LINE__));
////                    newStart->addChild(std::make_shared<BlockNode>(listConnectorName, __FILE__, __LINE__));
//                } else {
//                    std::string nodeDomainName = CodeValidator::getNodeDomainName(stackBack->getChildren()[i], scopeStack, m_tree);
//                    if (nodeDomainName.size() > 0) {
//                        newDeclarations.push_back(createSignalBridge(listConnectorName, nodeDomainName,
//                                                                     std::make_shared<ValueNode>(__FILE__, __LINE__),
//                                                                     std::make_shared<BlockNode>(nodeDomainName, __FILE__, __LINE__), outDomain,
//                                                                     stackBack->getChildren()[i]->getFilename(), stackBack->getChildren()[i]->getLine(), 1, "signal"));
//                        closingName->addChild(std::make_shared<BlockNode>(listConnectorName, __FILE__, __LINE__));
//                        newStart->addChild(std::make_shared<BlockNode>(listConnectorName, __FILE__, __LINE__));

//                    } else if (stackBack->getChildren()[i]->getNodeType() == AST::Int
//                               || stackBack->getChildren()[i]->getNodeType() == AST::Real
//                               || stackBack->getChildren()[i]->getNodeType() == AST::String
//                               || stackBack->getChildren()[i]->getNodeType() == AST::Switch ){
//                        std::shared_ptr<DeclarationNode> constDeclaration = createConstantDeclaration(listConnectorName, stackBack->getChildren()[i]);
//                        newDeclarations.push_back(constDeclaration);
//                        closingName->addChild(std::make_shared<BlockNode>(listConnectorName, __FILE__, __LINE__));
//                        newStart->addChild(std::make_shared<BlockNode>(listConnectorName, __FILE__, __LINE__));
//                    }
//                }
//            }
//        } else { // Slice everything else that is not a list
//            QString memberName = CodeValidator::streamMemberName(stackBack, scopeStack, m_tree);
//            connectorName += "_" + memberName.toStdString();
//            std::shared_ptr<DeclarationNode> prevDeclaration = CodeValidator::findDeclaration(memberName, scopeStack, m_tree);
//            if (prevDeclaration
//                    && (prevDeclaration->getObjectType() == "signal"
//                        || prevDeclaration->getObjectType() == "switch"
//                        || prevDeclaration->getObjectType() == "trigger")
//                    ) {
//                string type = prevDeclaration->getObjectType();
//                newDeclarations.push_back(createSignalBridge(connectorName, memberName.toStdString(),
//                                                             prevDeclaration->getPropertyValue("default"),
//                                                             prevDeclaration->getDomain(), outDomain,
//                                                             prevDeclaration->getFilename(), prevDeclaration->getLine(),
//                                                             size, type));
//            } else if (stackBack->getNodeType() == AST::Expression
//                       || stackBack->getNodeType() == AST::Function){
////                        ASTNode domain = CodeValidator::
//                // TODO set in/out domains correctly
//                // FIXME set default value correctly
//                string type = "signal"; // FIXME assumes that expressions and functions output signals...
//                newDeclarations.push_back(createSignalBridge(connectorName, memberName.toStdString(),
//                                                             std::make_shared<ValueNode>(0.0,__FILE__, __LINE__),
//                                                             std::make_shared<ValueNode>(__FILE__, __LINE__), outDomain,
//                                                             stackBack->getFilename(), stackBack->getLine(),
//                                                             size, type));

//            } else {
//                newDeclarations.push_back(createSignalBridge(connectorName, memberName.toStdString(),
//                                                             std::make_shared<ValueNode>(0.0,__FILE__, __LINE__),
//                                                             std::make_shared<ValueNode>(__FILE__, __LINE__), outDomain,
//                                                             stackBack->getFilename(), stackBack->getLine(),
//                                                             size, "signal"));
//            }
//            closingName = std::make_shared<BlockNode>(connectorName, __FILE__, __LINE__);
//            newStart = std::make_shared<BlockNode>(connectorName, __FILE__, __LINE__);
//            newStream = std::make_shared<StreamNode>(stack.back(), closingName, node->getFilename().c_str(), node->getLine());
//        }
////        stack.pop_back();
////        while (stack.size() > 0) {
////            ASTNode lastNode = stack.back();
////            stack.pop_back();
////        }
//        // FIXME this looks very wrong... Shouldn't we create a stream from the stack???
//        stack.clear();
//    } else if (stack.size() == 0) {
//        newStream = std::make_shared<StreamNode>(closingName, node, node->getFilename().c_str(), node->getLine());
//    }
//    for (ASTNode declaration: newDeclarations) {
//        streams << declaration;
//    }
//    if (newStream) {
//        streams << newStream;
//    }
//    stack << newStart << node;
//}

std::shared_ptr<StreamNode> CodeResolver::makeStreamFromStack(QVector<ASTNode> &stack)
{
    Q_ASSERT(stack.size()> 1);
    ASTNode lastNode = stack.back();
    stack.pop_back();
    std::shared_ptr<StreamNode> newStream =
            std::make_shared<StreamNode>(stack.back(), lastNode, lastNode->getFilename().c_str(), lastNode->getLine());
    stack.pop_back();
    while (stack.size() > 0) {
        lastNode = stack.back();
        newStream = std::make_shared<StreamNode>(lastNode, newStream, lastNode->getFilename().c_str(), lastNode->getLine());
        stack.pop_back();
    }
    return newStream;
}


QVector<ASTNode > CodeResolver::sliceStreamByDomain(std::shared_ptr<StreamNode> stream, QVector<ASTNode > scopeStack)
{
    QVector<ASTNode > streams;
    QVector<ASTNode > stack;

    ASTNode left = stream->getLeft();
    ASTNode right = stream->getRight();
    std::string domainName;
    std::string previousDomainName = CodeValidator::getNodeDomainName(left, scopeStack, m_tree);
    while (left) {
        domainName = CodeValidator::getNodeDomainName(left, scopeStack, m_tree);
        if (left->getNodeType() == AST::Int || left->getNodeType() == AST::Real  || left->getNodeType() == AST::Switch) {
            // If constant, ignore domain and go to next member
            stack << left;
            if(right->getNodeType() == AST::Stream) {
                //            stream = static_pointer_cast<StreamNode>(right);
                StreamNode *subStream = static_cast<StreamNode *>(right.get());
                left = subStream->getLeft();
                right = subStream->getRight();
            } else {
                left = right; // Last pass (process right, call it left)
                stack << right;
                if (stack.size() != 0) {
                    streams << makeStreamFromStack(stack);
                }
                break;
            }
            // Force it to take the domain from the left to avoid domain slicing
            domainName = CodeValidator::getNodeDomainName(left, scopeStack, m_tree);
            if (previousDomainName.size() == 0) {
                previousDomainName = domainName;
            }
        } else if (left->getNodeType() == AST::Expression) {
            // We might need to add bridge signals if expression members belong to different domains
            // Resolve domain from the right member in stream (if domain in expression members is not set)
            ASTNode outDomain;
//            if (right) {
//                outDomain = CodeValidator::getNodeDomain(right, scopeStack, m_tree);
//                std::shared_ptr<ExpressionNode> expr = static_pointer_cast<ExpressionNode>(left);
//                QVector<ASTNode > newStreams = processExpression(expr, scopeStack, outDomain);
//                streams << newStreams;
//                domainName = CodeValidator::getDomainNodeString(outDomain);
//            }
        } else if (left->getNodeType() == AST::Function) {
            std::shared_ptr<FunctionNode> func = static_pointer_cast<FunctionNode>(left);
            vector<std::shared_ptr<PropertyNode>> properties = func->getProperties();
//            for(auto prop: properties) {
//                ASTNode value = prop->getValue();
//                if (value->getNodeType() == AST::Block) {
//                    std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(CodeValidator::streamMemberName(value),
//                                                                                                  scopeStack.toStdVector(), m_tree);
//                    if (declaration) {
//                        std::string connectorName = "_B_" + std::to_string(m_connectorCounter++);
//                        connectorName += "_" + declaration->getName();
//                        int size = 1;
//                        if (declaration->getNodeType() == AST::BundleDeclaration) {
//                            Q_ASSERT(declaration->getBundle()->index()->getChildren()[0]->getNodeType() == AST::Int);
//                            size = static_cast<ValueNode *>(declaration->getBundle()->index()->getChildren()[0].get())->getIntValue();
//                        }
//                        ASTNode defaultProperty = declaration->getPropertyValue("default");
//                        ASTNode bridgeDomain = declaration->getDomain();
//                        if (defaultProperty && bridgeDomain) {
//                            std::shared_ptr<BlockNode> block = static_pointer_cast<BlockNode>(value);
//                            std::shared_ptr<ValueNode> noneValue = std::make_shared<ValueNode>(__FILE__, __LINE__);
//                            // FIXME this assumes the input to a function is a signal
//                            streams.push_back(createSignalBridge(connectorName, block->getName(), defaultProperty,
//                                                                 bridgeDomain, noneValue,
//                                                                 declaration->getFilename(), declaration->getLine(),
//                                                                 size, "signal")); // Add definition to stream
//                            std::shared_ptr<BlockNode> connectorNameNode = std::make_shared<BlockNode>(connectorName, __FILE__, __LINE__);
//                            std::shared_ptr<StreamNode> newStream = std::make_shared<StreamNode>(value, connectorNameNode, left->getFilename().c_str(), left->getLine());
//                            prop->replaceValue(connectorNameNode);
//                            streams.push_back(newStream);
//                        }
//                    }
//                } /*else if (value->getNodeType() == AST::Bundle) {
//                    std::string connectorName = "_BridgeSig_" + std::to_string(m_connectorCounter++);
//                    //                    newDeclarations.push_back(createSignalBridge(listMemberName, declaration, new ValueNode(__FILE__, __LINE__)));

//                }*/
//            }
        }
        stack << left;
        if (right->getNodeType() == AST::Stream) {
                //            stream = static_pointer_cast<StreamNode>(right);
            StreamNode *subStream = static_cast<StreamNode *>(right.get());
            left = subStream->getLeft();
            right = subStream->getRight();
        } else {
            stack << right;
            ASTNode lastNode = nullptr;
            previousDomainName = domainName;
            domainName = CodeValidator::getNodeDomainName(right, scopeStack, m_tree);

            if (domainName != previousDomainName) {
                bool skipSlice = false;
                if (right->getNodeType() == AST::Function) {
                    std::shared_ptr<FunctionNode> func = static_pointer_cast<FunctionNode>(right);
                    std::shared_ptr<DeclarationNode> decl = CodeValidator::findDeclaration(QString::fromStdString(func->getName()), scopeStack, m_tree);
                    //                        if (decl->getObjectType() == "reaction"
                    //                                || decl->getObjectType() == "loop") {
                    // TODO hack to avoid slice. The right thing should be to have reactions and loops take the right domain...
                    skipSlice = true;
                    //                        }
                }
                if (stack.size() > 0) {
                    if (left->getNodeType() == AST::Function) {
                        skipSlice = true;
                    } else if (left->getNodeType() == AST::List) {
                        bool allFuncs = true;
                        for (auto elem: left->getChildren()) {
                            if (elem->getNodeType() != AST::Function) {
                                allFuncs = false;
                                break;
                            }
                        }
                        if (allFuncs) {
                            skipSlice = true;
                        }
                    }
                }

                if (!skipSlice && stack.size() > 2) {
                    lastNode = stack.back();
                    stack.pop_back();
                    auto oneFromLastNode = stack.back();
                    streams << makeStreamFromStack(stack);
                    stack << oneFromLastNode << lastNode;
                    //                        terminateStackWithBridge(lastNode, streams, stack, scopeStack);
                }
            }
            if (stack.size() != 0) {
                streams << makeStreamFromStack(stack);
            }


            left = right = nullptr; // Mark last pass;
        }
        previousDomainName = domainName;
    }
    return streams;
}

void CodeResolver::sliceDomainsInNode(std::shared_ptr<DeclarationNode> module, QVector<ASTNode> scopeStack)
{
    if (module->getObjectType() == "module" || module->getObjectType() == "reaction" || module->getObjectType() == "loop") { // TODO how to handle different domains within loops and reactions?
        ASTNode streamsNode = module->getPropertyValue("streams");
        ASTNode blocksNode = module->getPropertyValue("blocks");

        Q_ASSERT(blocksNode);
        if (blocksNode->getNodeType() == AST::None) {
            module->replacePropertyValue("blocks", std::make_shared<ListNode>(nullptr, __FILE__, __LINE__));
            blocksNode = module->getPropertyValue("blocks");
        }

//        if (!streamsNode) {
//           module->setPropertyValue("streams", std::make_shared<ListNode>(nullptr, __FILE__, __LINE__));
//           streamsNode = module->getPropertyValue("streams");
//        }

        std::shared_ptr<ListNode> newStreamsList = std::make_shared<ListNode>(nullptr, __FILE__, __LINE__);
        scopeStack << CodeValidator::getBlocksInScope(module, scopeStack, m_tree);
        if (streamsNode->getNodeType() == AST::List) {
            for (ASTNode  stream: streamsNode->getChildren()) {
                if (stream->getNodeType() == AST::Stream) {
                    QVector<ASTNode > streams = sliceStreamByDomain(static_pointer_cast<StreamNode>(stream), scopeStack);
                    for (ASTNode  streamNode: streams) {
                        if (streamNode->getNodeType() == AST::Stream) {
                            newStreamsList->addChild(streamNode);
                        } else if (streamNode->getNodeType() == AST::Declaration || streamNode->getNodeType() == AST::BundleDeclaration) {
                            blocksNode->addChild(streamNode);
                        } else {
                            qDebug() << "Stream slicing must result in streams or blocks.";
                        }
                    }
                }
            }
        } else if (streamsNode->getNodeType() == AST::Stream) {
            QVector<ASTNode > streams = sliceStreamByDomain(static_pointer_cast<StreamNode>(streamsNode), scopeStack);
            for (ASTNode  streamNode: streams) {
                if (streamNode->getNodeType() == AST::Stream) {
                    newStreamsList->addChild(streamNode);
                } else if (streamNode->getNodeType() == AST::Declaration || streamNode->getNodeType() == AST::BundleDeclaration) {
                    blocksNode->addChild(streamNode);
                } else {
                    qDebug() << "Stream slicing must result in streams or blocks.";
                }
            }
        }
        module->replacePropertyValue("streams", newStreamsList);
        for (auto block : blocksNode->getChildren()) {
            if (block->getNodeType() == AST::Declaration) {
                // TODO this is untested and likely not completely working...
                std::shared_ptr<DeclarationNode> decl = static_pointer_cast<DeclarationNode>(block);
                scopeStack << QVector<ASTNode>::fromStdVector(blocksNode->getChildren());
                sliceDomainsInNode(decl, scopeStack);
            }
        }
    }
}

QVector<ASTNode> CodeResolver::processExpression(std::shared_ptr<ExpressionNode> expr, QVector<ASTNode> scopeStack, ASTNode outDomain)
{
    QVector<ASTNode > streams;

    ASTNode exprLeft;
    if (expr->isUnary()) {
        exprLeft = expr->getValue();
    } else {
        exprLeft = expr->getLeft();
    }
    if (exprLeft->getNodeType() == AST::Expression) {
        streams << processExpression(static_pointer_cast<ExpressionNode>(exprLeft), scopeStack, outDomain);
    } else if (exprLeft->getNodeType() == AST::Block || exprLeft->getNodeType() == AST::Bundle) {
        std::string memberName = CodeValidator::streamMemberName(exprLeft);
        std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(memberName, scopeStack.toStdVector(), m_tree);
        if (declaration) {
            if ( CodeValidator::getDomainIdentifier(declaration->getDomain(), scopeStack.toStdVector(), m_tree) !=
                 CodeValidator::getDomainIdentifier(outDomain, scopeStack.toStdVector(), m_tree)) {
                std::string connectorName = "_B_" + std::to_string(m_connectorCounter++);
                connectorName += "_" + memberName;
                string type = declaration->getObjectType();
                if (type == "switch" || type == "signal" || type == "trigger") { // This keeps constants away
                    streams.push_back(createSignalBridge(connectorName, memberName,
                                                         declaration->getPropertyValue("default"),
                                                         declaration->getDomain(), outDomain,
                                                         declaration->getFilename(), declaration->getLine(), 1, type)); // Add definition to stream
                    std::shared_ptr<BlockNode> connectorNameNode = std::make_shared<BlockNode>(connectorName, __FILE__, __LINE__);
                    std::shared_ptr<StreamNode> newStream = std::make_shared<StreamNode>(exprLeft, connectorNameNode, exprLeft->getFilename().c_str(), exprLeft->getLine());
                    expr->replaceLeft(std::make_shared<BlockNode>(connectorName, __FILE__, __LINE__));
                    streams.push_back(newStream);
                }
                // FIXME need to implement for bundles
            }
        }
    }
    if (!expr->isUnary()) {
        ASTNode exprRight = expr->getRight();

        if (exprRight->getNodeType() == AST::Expression) {
            streams << processExpression(static_pointer_cast<ExpressionNode>(exprRight), scopeStack, outDomain);
        } else if (exprRight->getNodeType() == AST::Block) {
            BlockNode *exprName = static_cast<BlockNode *>(exprRight.get());
            std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(QString::fromStdString(exprName->getName()),
                                                                    scopeStack, m_tree);
            if (declaration) {

                if ( CodeValidator::getDomainIdentifier(declaration->getDomain(), scopeStack.toStdVector(), m_tree) !=
                     CodeValidator::getDomainIdentifier(outDomain, scopeStack.toStdVector(), m_tree)) {
                    std::string connectorName = "_B_" + std::to_string(m_connectorCounter++);
                    string type = declaration->getObjectType();
                    streams.push_back(createSignalBridge(connectorName, exprName->getName(),
                                                         declaration->getPropertyValue("default"),
                                                         declaration->getDomain(), outDomain,
                                                         declaration->getFilename(), declaration->getLine(),1 , type)); // Add definition to stream
                    std::shared_ptr<BlockNode> connectorNameNode = std::make_shared<BlockNode>(connectorName, __FILE__, __LINE__);
                    std::shared_ptr<StreamNode> newStream = std::make_shared<StreamNode>(exprRight, connectorNameNode, exprRight->getFilename().c_str(), exprRight->getLine());
                    expr->replaceRight(std::make_shared<BlockNode>(connectorName, __FILE__, __LINE__));
                    streams.push_back(newStream);
                }
            }
        } else if (exprRight->getNodeType() == AST::Bundle) {

            // FIXME need to implement for bundles
        }
    }
    return streams;
}

ASTNode CodeResolver::getModuleContextDomain(std::shared_ptr<DeclarationNode> moduleDecl)
{
    auto blocks = moduleDecl->getPropertyValue("blocks");
    auto outputPortBlock = CodeValidator::getMainOutputPortBlock(moduleDecl);
    ASTNode contextDomainBlock;
    bool contextDomainSet = false;
    if (outputPortBlock) {
        QVector<ASTNode> scopeStack;
        if (blocks) {
            scopeStack = QVector<ASTNode>::fromStdVector(blocks->getChildren());
            contextDomainBlock = CodeValidator::getNodeDomain(outputPortBlock->getPropertyValue("block"), scopeStack, m_tree);
        }
    }
    auto inputPortBlock = CodeValidator::getMainInputPortBlock(moduleDecl);
    if (inputPortBlock && !contextDomainSet) {
        QVector<ASTNode> scopeStack;
        if (blocks) {
            scopeStack = QVector<ASTNode>::fromStdVector(blocks->getChildren());
            contextDomainBlock = CodeValidator::getNodeDomain(inputPortBlock->getPropertyValue("block"), scopeStack, m_tree);
        }
    }
    return contextDomainBlock;
}

void CodeResolver::setContextDomain(vector<ASTNode> nodes, std::shared_ptr<DeclarationNode> domainDeclaration)
{
    Q_ASSERT(domainDeclaration);
    for(ASTNode node: nodes) {
        if (node->getNodeType() == AST::Declaration
                || node->getNodeType() == AST::BundleDeclaration) {
            std::shared_ptr<DeclarationNode> decl = static_pointer_cast<DeclarationNode>(node);
            if (decl->getObjectType() == "signal" || decl->getObjectType() == "switch") {
                // Check if signal has domain
                ASTNode  nodeDomain = CodeValidator::getNodeDomain(decl, QVector<ASTNode >(), m_tree);
                if (!nodeDomain || nodeDomain->getNodeType() == AST::None) {
                    decl->setDomainString(domainDeclaration->getName());
                    if (CodeValidator::getNodeRate(decl, QVector<ASTNode >(), m_tree) < 0) {
                        ASTNode rateValue = domainDeclaration->getPropertyValue("rate");
                        if (rateValue->getNodeType() == AST::Int
                                || rateValue->getNodeType() == AST::Real) {
                            double rate = static_cast<ValueNode *>(rateValue.get())->toReal();
                            CodeValidator::setNodeRate(decl, rate, QVector<ASTNode >(), m_tree);
                        } else  if (rateValue->getNodeType() == AST::PortProperty) {
                            decl->replacePropertyValue("rate", rateValue->deepCopy());
                        } else {
                            qDebug() << "Unexpected type for rate in domain declaration: " << QString::fromStdString(domainDeclaration->getName());
                        }
                    }
                }
            }
        } else if (node->getNodeType() == AST::Stream) {
            // We need to go through setting the domain for functions in stream as they are the instances
//            setContextDomainForStreamNode(node, domainDeclaration);
        }
    }
}

//void CodeResolver::setContextDomainForStreamNode(ASTNode node, std::shared_ptr<DeclarationNode> domainDeclaration)
//{
//    if (node->getNodeType() == AST::Stream) {
//        auto stream = static_pointer_cast<StreamNode>(node);
//        setContextDomainForStreamNode(stream->getLeft(), domainDeclaration);
//        setContextDomainForStreamNode(stream->getRight(), domainDeclaration);
//    } else if (node->getNodeType() == AST::Function) {
//        auto func = static_pointer_cast<FunctionNode>(node);
//        auto domainProperty = func->getPropertyValue("domain");
//        if (!domainProperty || domainProperty->getNodeType() ==AST::None) {
//            func->setPropertyValue("domain", make_shared<ValueNode>(domainDeclaration->getName(), __FILE__, __LINE__));
//        }
//    }
//}

//void CodeResolver::populateContextDomains(vector<ASTNode> nodes)
//{
//    for (auto node: nodes) {
//        if (node->getNodeType() == AST::Declaration) {
//            std::shared_ptr<DeclarationNode> decl = std::static_pointer_cast<DeclarationNode>(node);
//            if (decl->getObjectType() == "module" || decl->getObjectType() == "reaction" || decl->getObjectType() == "loop") {
//                auto blocks = decl->getPropertyValue("blocks");
//                bool hasContextDomain = false;
//                for (auto block: blocks->getChildren()) {
//                    if (block->getNodeType() == AST::Declaration) {
//                        auto internalDecl = std::static_pointer_cast<DeclarationNode>(block);
//                        if (internalDecl->getName() == "_ContextDomain") {
//                            hasContextDomain = true;
//                            break;
//                        }
//                    }
//                }
//                if (!hasContextDomain) {
//                    // This is currently hard coded to try output first, then input. This order should somehow be specified within Stride itself
//                    auto outputPortBlock = CodeValidator::getMainOutputPortBlock(decl);
//                    bool contextDomainSet = false;
//                    if (outputPortBlock) {
//                        QVector<ASTNode> scopeStack;
//                        if (blocks) {
//                            scopeStack = QVector<ASTNode>::fromStdVector(blocks->getChildren());
//                            auto domainBlock = CodeValidator::getNodeDomain(outputPortBlock->getPropertyValue("block"), scopeStack, m_tree);
//                            if (domainBlock) {
//                                ASTNode propertiesList = std::make_shared<ListNode>(
//                                            std::make_shared<PropertyNode>("value",
//                                                                           domainBlock,
//                                                                           __FILE__, __LINE__), __FILE__, __LINE__);
//                                blocks->addChild(std::make_shared<DeclarationNode>("_ContextDomain", "constant", propertiesList, __FILE__, __LINE__));
//                                contextDomainSet = true;
//                            }
//                        }
//                    }
//                    auto inputPortBlock = CodeValidator::getMainInputPortBlock(decl);
//                    if (inputPortBlock && !contextDomainSet) {
//                        QVector<ASTNode> scopeStack;
//                        if (blocks) {
//                            scopeStack = QVector<ASTNode>::fromStdVector(blocks->getChildren());
//                            auto domainBlock = CodeValidator::getNodeDomain(inputPortBlock->getPropertyValue("block"), scopeStack, m_tree);
//                            if (domainBlock) {
//                                ASTNode propertiesList = std::make_shared<ListNode>(
//                                            std::make_shared<PropertyNode>("value",
//                                                                           domainBlock,
//                                                                           __FILE__, __LINE__), __FILE__, __LINE__);
//                                blocks->addChild(std::make_shared<DeclarationNode>("_ContextDomain", "constant", propertiesList, __FILE__, __LINE__));
//                                contextDomainSet = true;
//                            }
//                        }
//                    }
//                }
////                populateContextDomains(decl->getPropertyValue("blocks")->getChildren()) ; // Do it recursively
//            }
//        }
//    }
//}

void CodeResolver::resolveDomainForStreamNode(ASTNode node, QVector<ASTNode > scopeStack)
{
    ASTNode domain = nullptr;
    if (node->getNodeType() == AST::Block
            || node->getNodeType() == AST::Bundle) {
        std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(CodeValidator::streamMemberName(node),
                                                                                      scopeStack.toStdVector(), m_tree, node->getNamespaceList());
        if (declaration) {
            domain = static_cast<DeclarationNode *>(declaration.get())->getDomain();
        }
    } else if (node->getNodeType() == AST::Function) {
        domain = static_cast<FunctionNode *>(node.get())->getDomain();
    } else if (node->getNodeType() == AST::List) {
        for (ASTNode member : node->getChildren()) {
            resolveDomainForStreamNode(member, scopeStack);
        }
        return;
    } else if (node->getNodeType() == AST::Expression) {
        ExpressionNode *expr = static_cast<ExpressionNode *>(node.get());
        if (expr->isUnary()) {
            resolveDomainForStreamNode(expr->getValue(), scopeStack);
        } else {
            resolveDomainForStreamNode(expr->getLeft(), scopeStack);
            resolveDomainForStreamNode(expr->getRight(), scopeStack);
        }
        return;
    }
    if (domain) {
        std::shared_ptr<DeclarationNode> domainDeclaration;
        if (domain->getNodeType() == AST::PortProperty) {
            auto domainNameNode = static_pointer_cast<PortPropertyNode>(domain);
            domain = resolvePortProperty(domainNameNode, scopeStack);
            if (!domain) {
                domain = domainNameNode;
            }
        }
        if (domain->getNodeType() == AST::Block) { // Resolve domain name
            auto domainNameNode = static_pointer_cast<BlockNode>(domain);
            domainDeclaration = CodeValidator::findDeclaration(
                        QString::fromStdString(domainNameNode->getName()), scopeStack, m_tree);
        }
        if (domainDeclaration) {
            ASTNode domainValue = domainDeclaration->getPropertyValue("domainName");
            while (domainValue && domainValue->getNodeType() == AST::Block) {
                auto recurseDomain = static_pointer_cast<BlockNode>(domainValue);
                domainDeclaration = CodeValidator::findDeclaration(
                            QString::fromStdString(recurseDomain->getName()), scopeStack, m_tree);
                domainValue = domainDeclaration->getPropertyValue("name");
            }
            if (domainValue && domainValue->getNodeType() == AST::String) {
                string domainName = static_cast<ValueNode *>(domainValue.get())->getStringValue();
                if (node->getNodeType() == AST::Block
                        || node->getNodeType() == AST::Bundle) {
                    std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(CodeValidator::streamMemberName(node), scopeStack.toStdVector(), m_tree);
//                    declaration->setDomainString(domainName);
                }
//                else if (node->getNodeType() == AST::Function) {
//                    // FIXME we need to find the domain declaration that matches this domain name
//                    // This is currently assuming the domain name and the domain block delcaration name are the same
//                     static_cast<FunctionNode *>(node.get())->setDomainString(domainName);
//                }
            }
        } else {

        }
    }
}

ASTNode CodeResolver::resolvePortProperty(std::shared_ptr<PortPropertyNode> portProperty, QVector<ASTNode> scopeStack)
{
    ASTNode resolved;
    auto decl
            = CodeValidator::findDeclaration(QString::fromStdString(portProperty->getPortName()),
                                             scopeStack, m_tree);
    if (decl) {
        resolved = decl->getPropertyValue(portProperty->getName());
        if (resolved && resolved->getNodeType() == AST::PortProperty) {
            auto resolvedPortProperty = std::static_pointer_cast<PortPropertyNode>(resolved);
            if (decl->getName() != resolvedPortProperty->getPortName()) { // To avoid infinite recursion
                resolved = resolvePortProperty(resolvedPortProperty, scopeStack);
            }
        }
    }
    return resolved;
}

void CodeResolver::setInputBlockForFunction(std::shared_ptr<FunctionNode> func, QVector<ASTNode > scopeStack, ASTNode previous)
{
    if (previous) {
        func->setCompilerProperty("inputBlock", previous);
    }
    QVector<ASTNode> blocks;
    auto funcDecl = CodeValidator::findDeclaration(QString::fromStdString(func->getName()), scopeStack, m_tree);
    if (!funcDecl) return;
    auto blocksNode = funcDecl->getPropertyValue("blocks");
    if (blocksNode) {
        if (blocksNode->getNodeType() == AST::List) {
            blocks = QVector<ASTNode>::fromStdVector(blocksNode->getChildren());
        } else if (blocksNode->getNodeType() == AST::Declaration
                   || blocksNode->getNodeType() == AST::BundleDeclaration) {
            blocks.push_back(blocksNode);
        }
    }
    auto streamsNode = funcDecl->getPropertyValue("streams");
    if (streamsNode) {
        for (auto stream: streamsNode->getChildren()) {
            if (stream->getNodeType() ==AST::Stream) {
                checkStreamConnections(static_pointer_cast<StreamNode>(stream), blocks + scopeStack, nullptr);
            }
        }

    }
}

void CodeResolver::setOutputBlockForFunction(std::shared_ptr<FunctionNode> func, QVector<ASTNode > scopeStack, ASTNode next)
{
    if (next) {
        func->setCompilerProperty("outputBlock", next);
    }
    QVector<ASTNode> blocks;
    auto funcDecl = CodeValidator::findDeclaration(QString::fromStdString(func->getName()), scopeStack, m_tree);
    if (!funcDecl) return;
    auto blocksNode = funcDecl->getPropertyValue("blocks");
    if (blocksNode) {
        if (blocksNode->getNodeType() == AST::List) {
            blocks = QVector<ASTNode>::fromStdVector(blocksNode->getChildren());
        } else if (blocksNode->getNodeType() == AST::Declaration
                   || blocksNode->getNodeType() == AST::BundleDeclaration) {
            blocks.push_back(blocksNode);
        }
    }
    auto streamsNode = funcDecl->getPropertyValue("streams");
    if (streamsNode) {
        for (auto stream: streamsNode->getChildren()) {
            if (stream->getNodeType() ==AST::Stream) {
                checkStreamConnections(static_pointer_cast<StreamNode>(stream), blocks + scopeStack, nullptr);
            }
        }

    }
}

void CodeResolver::checkStreamConnections(std::shared_ptr<StreamNode> stream, QVector<ASTNode > scopeStack, ASTNode previous)
{
    ASTNode left = stream->getLeft();
    ASTNode right = stream->getRight();

    markConnectionForNode(left, scopeStack, previous);

    if (left->getNodeType() == AST::Function) {
        auto func = static_pointer_cast<FunctionNode>(left);
        setInputBlockForFunction(func, scopeStack, previous);
    } else if (left->getNodeType() == AST::List) {
        if (previous) {
            if (previous->getNodeType() == AST::List && previous->getChildren().size() > 0) {
                auto previousChildren = previous->getChildren();
                std::vector<ASTNode>::iterator connection = previousChildren.begin();
                for (auto child: left->getChildren()) {
                    if (connection == previousChildren.end()) {
                        connection = previousChildren.begin();
                    }
                    if (child->getNodeType() == AST::Function) {
                        auto func = static_pointer_cast<FunctionNode>(child);
                        setInputBlockForFunction(func, scopeStack, *connection);
                    }
                    connection++;
                }
            } else {
                for (auto child: left->getChildren()) {
                    if (child->getNodeType() == AST::Function) {
                        auto func = static_pointer_cast<FunctionNode>(child);
                        setInputBlockForFunction(func, scopeStack, previous);
                    }
                }
            }
        }
    }

    if (right->getNodeType() == AST::Function) {
        previous = left;
        auto func = static_pointer_cast<FunctionNode>(right);
        if (left->getNodeType() == AST::Function) {
            // FIXME this is a hack assuming that the input is always connected to the output inside modules.
            ASTNode newPrevious = left->getCompilerProperty("inputBlock");
            if (newPrevious) {
                previous = newPrevious; // Otherwise keep old previous
            }
        }
        setInputBlockForFunction(func, scopeStack, previous);
    } else if (right->getNodeType() == AST::List) {
        previous = left;
        for (auto child: right->getChildren()) {
            if (child->getNodeType() == AST::Function) {
                auto func = static_pointer_cast<FunctionNode>(child);
                // FIXME this is a hack assuming that the input is always connected to the output inside modules.
                ASTNode newPrevious = left->getCompilerProperty("inputBlock");
                if (newPrevious) {
                    previous = newPrevious; // Otherwise keep old previous
                }
                setInputBlockForFunction(func, scopeStack, previous);
            }
        }
    } else {
        previous = left;
    }

    if (right->getNodeType() == AST::Stream) {
        checkStreamConnections(static_pointer_cast<StreamNode>(right), scopeStack, previous);
        if (left->getNodeType() == AST::Function) {
            auto next = static_pointer_cast<StreamNode>(right)->getLeft();
            if (next) {
                if (left->getNodeType() == AST::List) {
                    for (auto child: left->getChildren()) {
                        if (child->getNodeType() == AST::Function) {
                            // FIXME simplistic. We are connecting the input of a module to the output. This might
                            // not be the case always.
                            auto func = static_pointer_cast<FunctionNode>(child);
                            setOutputBlockForFunction(func, scopeStack, next->getCompilerProperty("outputBlock"));
                            //                    left->setCompilerProperty("outputBlock", next->getCompilerProperty("outputBlock"));
                            if (child->getCompilerProperty("outputBlock")) {
                                next->setCompilerProperty("inputBlock", child->getCompilerProperty("outputBlock"));
                            }
                        } /*else {
                            auto func = static_pointer_cast<FunctionNode>(child);
                            setOutputBlockForFunction(func, scopeStack, next);
                        }*/
                    }
                } else if (left->getNodeType() == AST::Function) {
                    // FIXME simplistic. We are connecting the input of a module to the output. This might
                    // not be the case always.
                    auto func = static_pointer_cast<FunctionNode>(left);
                    setOutputBlockForFunction(func, scopeStack, next->getCompilerProperty("outputBlock"));
                    //                    left->setCompilerProperty("outputBlock", next->getCompilerProperty("outputBlock"));
                    if (left->getCompilerProperty("outputBlock")) {
                        // FIXME we should go recursively to get the rightmost outputBlock
                        next->setCompilerProperty("inputBlock", left->getCompilerProperty("outputBlock"));
                    }
                } else {
                    auto func = static_pointer_cast<FunctionNode>(left);
                    setOutputBlockForFunction(func, scopeStack, next);
                }
            }
        }
    } else {
        markConnectionForNode(right, scopeStack, previous);
        if (left->getNodeType() == AST::List) {
            for (auto child: left->getChildren()) {
                ASTNode nextBlock = right;
//                CodeValidator::getNodeNumInputs()
                if (child->getNodeType() == AST::Function) {
//                    if (right->getNodeType() == AST::Function) {
//                        right->getCompilerProperty("inputBlock");
//                    }
                    auto func = static_pointer_cast<FunctionNode>(child);
                    setOutputBlockForFunction(func, scopeStack, nextBlock);
                    //            if (nextBlock) {
                    //                left->setCompilerProperty("outputBlock", nextBlock);
                    //            }
                }
            }
        } else if (left->getNodeType() == AST::Function) {
            ASTNode nextBlock = right;
//            if (right->getNodeType() == AST::Function) {
//                right->getCompilerProperty("inputBlock");
//            }
            auto func = static_pointer_cast<FunctionNode>(left);
            setOutputBlockForFunction(func, scopeStack, nextBlock);
//            if (nextBlock) {
//                left->setCompilerProperty("outputBlock", nextBlock);
//            }
        }
    }
}

void CodeResolver::setReadsWrites(ASTNode node, ASTNode previous, QVector<ASTNode > scopeStack) {
    if (previous) {
        if (previous->getNodeType() == AST::Block
                || previous->getNodeType() == AST::Bundle) {
            QString name = QString::fromStdString(CodeValidator::streamMemberName(node));
            std::shared_ptr<DeclarationNode> decl = CodeValidator::findDeclaration(name, scopeStack, m_tree);
    //        std::shared_ptr<ListNode> readsProperties = static_pointer_cast<ListNode>(decl->getCompilerProperty("reads"));
            std::shared_ptr<ListNode> writesProperties = static_pointer_cast<ListNode>(decl->getCompilerProperty("writes"));

            std::string previousName = CodeValidator::streamMemberName(previous);
            auto previousDecl = CodeValidator::findDeclaration(QString::fromStdString(previousName), scopeStack, m_tree);
            if (previousDecl) {
                auto nodeTypeName = decl->getObjectType();

                // TODO we need to support lists here too
                std::shared_ptr<ListNode> previousReads = static_pointer_cast<ListNode>(previous->getCompilerProperty("reads"));
                std::shared_ptr<ListNode> previousDeclReads = static_pointer_cast<ListNode>(previousDecl->getCompilerProperty("reads"));
                if (nodeTypeName == "module" || nodeTypeName == "reaction"|| nodeTypeName == "loop") {
                    // Modules, reactions and loops, the domain that matters is the domain of the output block.
                    if (node->getNodeType() ==AST::Function) {
                        auto func = std::static_pointer_cast<FunctionNode>(node);
                        if (func->getDomain() != nullptr) {
                            previousReads->addChild(func->getDomain());
                            if (previousDeclReads) {
                                previousDeclReads->addChild(func->getDomain());
                            }
                        }
                    }
                } else {
                    // platformBlock and platformModule can be ignored as they are tied to a domain.
                    // However, we do need to figure out how to support buffer access across domains.
                    bool alreadyInReads = false;
                    auto newReadDomain = decl->getDomain();
                    for (auto read: previousReads->getChildren()) {
                        if (read->getNodeType() == AST::PortProperty
                                && newReadDomain->getNodeType() == AST::PortProperty) {
                            std::shared_ptr<PortPropertyNode> newReadDomainNode =
                                    std::static_pointer_cast<PortPropertyNode>(newReadDomain);
                            std::shared_ptr<PortPropertyNode> existingReadDomain =
                                    std::static_pointer_cast<PortPropertyNode>(read);
                            if ( (newReadDomainNode->getName() == existingReadDomain->getName())
                                 && (newReadDomainNode->getPortName() == existingReadDomain->getPortName()) ) {
                                alreadyInReads = true;
                                break;
                            }
                        }
                    }

                    if (!alreadyInReads) {
                        previousReads->addChild(newReadDomain);
                    }
                    alreadyInReads = false;

                    if (previousDeclReads) {
                        for (auto read: previousDeclReads->getChildren()) {
                            if (read->getNodeType() == AST::PortProperty
                                    && newReadDomain->getNodeType() == AST::PortProperty) {
                                std::shared_ptr<PortPropertyNode> newReadDomainNode =
                                        std::static_pointer_cast<PortPropertyNode>(newReadDomain);
                                std::shared_ptr<PortPropertyNode> existingReadDomain =
                                        std::static_pointer_cast<PortPropertyNode>(read);
                                if ( (newReadDomainNode->getName() == existingReadDomain->getName())
                                     && (newReadDomainNode->getPortName() == existingReadDomain->getPortName()) ) {
                                    alreadyInReads = true;
                                    break;
                                }
                            }
                        }
                        if (!alreadyInReads) {
                            previousDeclReads->addChild(newReadDomain);
                        }
                    }

                    // FIXME we need to get previousReads for functions from the function itsefl, not the module decl.
                }
        //                    Q_ASSERT(previousDecl);
                Q_ASSERT(writesProperties->getNodeType() == AST::List);
                //                std::string domainName = CodeValidator::getDomainNodeString(decl->getDomain());
                bool alreadyInWrites = false;
                auto newWriteDomain = previousDecl->getDomain();
                for (auto write: writesProperties->getChildren()) {
                    if (write->getNodeType() == AST::PortProperty
                            && newWriteDomain->getNodeType() == AST::PortProperty) {
                        std::shared_ptr<PortPropertyNode> newReadDomainNode =
                                std::static_pointer_cast<PortPropertyNode>(newWriteDomain);
                        std::shared_ptr<PortPropertyNode> existingReadDomain =
                                std::static_pointer_cast<PortPropertyNode>(write);
                        if ( (newReadDomainNode->getName() == existingReadDomain->getName())
                             && (newReadDomainNode->getPortName() == existingReadDomain->getPortName()) ) {
                            alreadyInWrites = true;
                            break;
                        }
                    }
                }

                if (!alreadyInWrites) {
                    writesProperties->addChild(newWriteDomain);
                }


            }
        } else if (previous->getNodeType() == AST::Expression) {
            for(auto child: previous->getChildren()) {
                setReadsWrites(node, child, scopeStack);
            }
        }
    }
}

void CodeResolver::markConnectionForNode(ASTNode node, QVector<ASTNode > scopeStack, ASTNode previous)
{
    if (node->getNodeType() == AST::Block || node->getNodeType() == AST::Bundle || node->getNodeType() == AST::Function) {
        std::string name = CodeValidator::streamMemberName(node);

        // First create read and write compiler properties for the node if not there
        std::shared_ptr<ListNode> nodeReadsProperties = static_pointer_cast<ListNode>(node->getCompilerProperty("reads"));
        if (!nodeReadsProperties) {
            nodeReadsProperties = std::make_shared<ListNode>(nullptr, node->getFilename().c_str(), node->getLine());
            node->setCompilerProperty("reads", nodeReadsProperties);
        }
        std::shared_ptr<ListNode> nodeWritesProperties = static_pointer_cast<ListNode>(node->getCompilerProperty("writes"));
        if (!nodeWritesProperties) {
            nodeWritesProperties = std::make_shared<ListNode>(nullptr, node->getFilename().c_str(), node->getLine());
            node->setCompilerProperty("writes", nodeWritesProperties);
        }
        std::shared_ptr<DeclarationNode> decl = CodeValidator::findDeclaration(name, scopeStack.toStdVector(), m_tree);
        if (decl) {
            auto typeDecl = CodeValidator::findTypeDeclaration(decl, scopeStack, m_tree);
            if (typeDecl) {
                // For the declaration too
                std::shared_ptr<ListNode> readsProperties = static_pointer_cast<ListNode>(decl->getCompilerProperty("reads"));
                if (!readsProperties) {
                    readsProperties = std::make_shared<ListNode>(nullptr, node->getFilename().c_str(), node->getLine());
                    decl->setCompilerProperty("reads", readsProperties);
                }
                std::shared_ptr<ListNode> writesProperties = static_pointer_cast<ListNode>(decl->getCompilerProperty("writes"));
                if (!writesProperties) {
                    writesProperties = std::make_shared<ListNode>(nullptr, node->getFilename().c_str(), node->getLine());
                    decl->setCompilerProperty("writes", writesProperties);
                }
                if (previous) {
                    QString previousName;
                    std::shared_ptr<ListNode> previousReads;
                    std::shared_ptr<DeclarationNode> previousDecl;
                    if (previous->getNodeType() == AST::Block || previous->getNodeType() == AST::Bundle) {
                        setReadsWrites(node, previous, scopeStack);
                    } else if (previous->getNodeType() == AST::Expression) {
                        auto expr = static_pointer_cast<ExpressionNode>(previous);
                        for (auto child: expr->getChildren()) {
                            markConnectionForNode(child, scopeStack, nullptr);
                            setReadsWrites(node, child, scopeStack);
    //                        previousReads = static_pointer_cast<ListNode>(expr->getCompilerProperty("reads"));
    //                        Q_ASSERT(previousReads);
                        }
    //                    previousReads->addChild(decl->getDomain());
                    } else if (previous->getNodeType() == AST::List) {
                        // How should lists be handled?
                        for (auto child: previous->getChildren()) {
                            markConnectionForNode(child, scopeStack, nullptr);
                            setReadsWrites(node, child, scopeStack);
                        }
                    }
                }
            }
        }
    } else if (node->getNodeType() == AST::Int
               || node->getNodeType() == AST::Real
               || node->getNodeType() == AST::Switch
               || node->getNodeType() == AST::String) {
        // First create read and write compiler properties for the node if not there
        std::shared_ptr<ListNode> nodeReadsProperties = static_pointer_cast<ListNode>(node->getCompilerProperty("reads"));
        if (!nodeReadsProperties) {
            nodeReadsProperties = std::make_shared<ListNode>(nullptr, node->getFilename().c_str(), node->getLine());
            node->setCompilerProperty("reads", nodeReadsProperties);
        }
        // Writes property is created but should be empty for ValueNode
        std::shared_ptr<ListNode> nodeWritesProperties = static_pointer_cast<ListNode>(node->getCompilerProperty("writes"));
        if (!nodeWritesProperties) {
            nodeWritesProperties = std::make_shared<ListNode>(nullptr, node->getFilename().c_str(), node->getLine());
            node->setCompilerProperty("writes", nodeWritesProperties);
        }

        if (previous) {
            QString previousName;
            std::shared_ptr<ListNode> previousReads;
            std::shared_ptr<DeclarationNode> previousDecl;
            if (previous->getNodeType() == AST::Block || previous->getNodeType() == AST::Bundle) {
                setReadsWrites(node, previous, scopeStack);
            } else if (previous->getNodeType() == AST::Expression) {
                auto expr = static_pointer_cast<ExpressionNode>(previous);
                for (auto child: expr->getChildren()) {
                    markConnectionForNode(child, scopeStack, nullptr);
                    setReadsWrites(node, child, scopeStack);
//                        previousReads = static_pointer_cast<ListNode>(expr->getCompilerProperty("reads"));
//                        Q_ASSERT(previousReads);
                }
//                    previousReads->addChild(decl->getDomain());
            } else if (previous->getNodeType() == AST::List) {
                // How should lists be handled?
            }
        }
    } else if (node->getNodeType() == AST::Expression) {
        for (auto child: node->getChildren()) {
            markConnectionForNode(child, scopeStack, previous);
        }
        previous = node;
    } else if (node->getNodeType() == AST::List) {
        for (auto child: node->getChildren()) {
            markConnectionForNode(child, scopeStack, previous);
        }
        previous = node;
    }
}

void CodeResolver::storeDeclarationsForNode(ASTNode node, vector<ASTNode> scopeStack, ASTNode tree)
{
    if (node->getNodeType() == AST::Block || node->getNodeType() == AST::Bundle
            || node->getNodeType() == AST::Function) {
        auto decl = CodeValidator::findDeclaration(CodeValidator::streamMemberName(node),
                                                  scopeStack, tree);
        if (decl) {
            node->setCompilerProperty("declaration", decl);
        }
    } else if (node->getNodeType() == AST::Expression
               || node->getNodeType() == AST::List) {
        for(auto child: node->getChildren()) {
            storeDeclarationsForNode(child, scopeStack, tree);
        }
    } else if (node->getNodeType() ==AST::Stream ) {
        auto stream = static_pointer_cast<StreamNode>(node);
        auto streamNode = stream->getLeft();
        while (streamNode) {
            storeDeclarationsForNode(streamNode, scopeStack, m_tree);
            if (stream) {
                if (stream->getRight()->getNodeType() == AST::Stream) {
                    stream = static_pointer_cast<StreamNode>(stream->getRight());
                    streamNode = stream->getLeft();
                } else {
                    streamNode = stream->getRight();
                    stream = nullptr;
                }
            } else {
                streamNode = nullptr;
            }
        }
    } else if (node->getNodeType() == AST::Declaration) {
        // TODO support recursive modules (modules declared in modules)
        auto decl = static_pointer_cast<DeclarationNode>(node);
        if (decl->getObjectType() == "module"
                || decl->getObjectType() == "reaction"
                || decl->getObjectType() == "loop") {
            auto internalStreams = decl->getPropertyValue("streams");
            auto internalBlocks = decl->getPropertyValue("blocks");
            vector<ASTNode> blocksList;
            if (internalBlocks) {
                if (internalBlocks->getNodeType() == AST::List) {
                    for (auto child: internalBlocks->getChildren()) {
                        if (child->getNodeType() == AST::Declaration
                                || child->getNodeType() == AST::BundleDeclaration) {
                            blocksList.push_back(static_pointer_cast<DeclarationNode>(child));
                        } else {
                            qDebug() << "Unexpected node type in internal blocks";
                        }
                    }
                } else if (internalBlocks->getNodeType() == AST::Declaration
                           || internalBlocks->getNodeType() == AST::BundleDeclaration) {
                    blocksList.push_back(static_pointer_cast<DeclarationNode>(internalBlocks));
                }
            }
            if (internalStreams) {
                if (internalStreams->getNodeType() == AST::List) {
                    for(auto node: internalStreams->getChildren()) {
                        storeDeclarationsForNode(node, blocksList, m_tree);
                    }
                }
            }
        }
    }
}

