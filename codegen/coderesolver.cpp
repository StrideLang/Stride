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
    processResets();
    resolveStreamSymbols();
    processDomains();
    resolveRates();
    analyzeConnections();
    processSystem();
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
        std::shared_ptr<DeclarationNode> domainDeclaration = CodeValidator::findDomainDeclaration(m_system->getPlatformDomain().toStdString(), m_tree);
        if (domainDeclaration) {
            ASTNode rateValue = domainDeclaration->getPropertyValue("rate");
            if (rateValue->getNodeType() == AST::Int
                    || rateValue->getNodeType() == AST::Real) {
                double rate = static_cast<ValueNode *>(rateValue.get())->toReal();
                CodeValidator::setNodeRate(left, rate, QVector<ASTNode>(), m_tree);
            } else {
                qDebug() << "Unexpected type for rate in domain declaration: " << m_system->getPlatformDomain();
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
        DeclarationNode *destBlock = static_cast<DeclarationNode *>(node.get());
        vector<std::shared_ptr<PropertyNode>> blockProperties = destBlock->getProperties();
        QVector<ASTNode> typeProperties = CodeValidator::getPortsForType(
                    destBlock->getObjectType(),
                    QVector<ASTNode>(), m_tree);
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
        // This should get taken care of by the code generator settting default values
        // according to the module's deafult values for the port blocks
//        std::shared_ptr<FunctionNode> destFunc = static_pointer_cast<FunctionNode>(node);
//        vector<std::shared_ptr<PropertyNode>> blockProperties = destFunc->getProperties();
//        std::shared_ptr<DeclarationNode> functionModule = CodeValidator::findDeclaration(
//                    QString::fromStdString(destFunc->getName()),
//                    QVector<ASTNode >(), m_tree);
//        if (functionModule) {
//            vector<std::shared_ptr<PropertyNode>> typeProperties = functionModule->getProperties();
//            if (typeProperties.size() < 1) {
//                qDebug() << "ERROR: fillDefaultProperties() No type definition for " << QString::fromStdString(destFunc->getName());
//                return;
//            }
//            for (std::shared_ptr<PropertyNode> property : blockProperties) {
//                fillDefaultPropertiesForNode(property->getValue());
//            }

//            for(ASTNode propertyListMember : typeProperties) {
//                Q_ASSERT(propertyListMember->getNodeType() == AST::Property);
//                PropertyNode *property = static_cast<PropertyNode *>(propertyListMember.get());
//                string propertyName = property->getName();
//                bool propertySet = false;
//                for(std::shared_ptr<PropertyNode> blockProperty : blockProperties) {
//                    if (blockProperty->getName() == propertyName) {
//                        propertySet = true;
//                        break;
//                    }
//                }
//                if (!propertySet) {
//                    //                ASTNode defaultValueNode = property->getPropertyValue("default");
//                    //                PropertyNode *newProperty = new PropertyNode(propertyName,
//                    //                            defaultValueNode,
//                    //                            property->getFilename().data(), property->getLine());
//                    //                destFunc->addProperty(newProperty);
//                }
//            }
//        }
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

    populateContextDomains(m_tree->getChildren());
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
    int leftSize = CodeValidator::getNodeSize(left, scopeStack, m_tree);
    if (previousOutSize == -1) {
        previousOutSize = 1;
    }

    if (left->getNodeType() == AST::Block
            || left->getNodeType() == AST::Function) {
        int numCopies = neededCopies.front();
        if (leftSize < 0 && left->getNodeType() == AST::Block) {
            std::vector<ASTNode > newDeclaration = declareUnknownName(static_pointer_cast<BlockNode>(left), previousOutSize, scopeStack, m_tree);
            for(ASTNode decl:newDeclaration) {
                m_tree->addChild(decl);
            }
        }
        if (numCopies > 1) {
            std::shared_ptr<ListNode> newLeft = std::make_shared<ListNode>(left, left->getFilename().data(), left->getLine());
            for (int i = 1; i < numCopies; i++) {
                newLeft->addChild(left);
            }
            stream->setLeft(newLeft); // This will take care of the deallocation internally
        }
    }
    previousOutSize = neededCopies.front() * leftSize;
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
            } else if (numCopies > 1) {
                std::shared_ptr<ListNode> newRight = std::make_shared<ListNode>(right, right->getFilename().data(), right->getLine());
                for (int i = 1; i < numCopies; i++) {
                    newRight->addChild(right);
                }
                stream->setRight(newRight); // This will take care of the deallocation internally
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
    int dataSize = CodeValidator::getFunctionDataSize(func, scopeStack, tree, errors);
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
    QList<std::shared_ptr<DeclarationNode>> requiredDeclarations;
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
                            || typeName->getStringValue() == "propertyInputPort"
                            || typeName->getStringValue() == "propertyOutputPort"
                            || typeName->getStringValue() == "reaction"
                            || typeName->getStringValue() == "signalbridge") {
                        requiredDeclarations << block;
                    }
                    if (typeName->getStringValue() == "type"
                            || typeName->getStringValue() == "platformType") {
                        m_tree->addChild(block); // We don't want to add dependent declarations for these as this will create an endless loop
                    }
                }
            } if (block->getObjectType() == "constant") {
                if (block->getName() == "PlatformDomain" || block->getName() == "PlatformRate"
                        || block->getName() == "_ContextDomain" || block->getName() == "_ContextRate") {
                    m_tree->addChild(block);
                }
            }
            else if (block->getObjectType() == "_domainDefinition"
                     || block->getObjectType() == "_frameworkDescription") // Hack... this should be getting inserted when resolving symbols...)
            {
                m_tree->addChild(block);
            }
            continue;
        }
    }
    for (std::shared_ptr<DeclarationNode> declaration : requiredDeclarations) {
//        insertBuiltinObjectsForNode(declaration, bultinObjects);
        insertDependentTypes(declaration->getObjectType(), bultinObjects);
        m_tree->addChild(declaration->deepCopy());
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

    while(rit != children.rend()) {
        ASTNode node = *rit;
        propagateDomainsForNode(node, scopeStack);
        rit++;
    }
    // Now go forward propagating domains
    // But this is currently messing things up... Test carefully
//    auto it = children.begin();
//    while(it != children.end()) {
//        ASTNode node = *it;
//        propagateDomainsForNode(node, scopeStack);
//        ++it;
//    }

    // TODO Do we need to propagate domains forward too

    // Any signals without domain are assigned to the platform domain
    std::shared_ptr<DeclarationNode> domainDecl = CodeValidator::findDomainDeclaration(m_system->getPlatformDomain().toStdString(), m_tree);
    if (domainDecl) {
        setContextDomain(m_tree->getChildren(),domainDecl);
    }
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
    for (ASTNode object : m_tree->getChildren()) {
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
                for (const ASTNode stream: streams) {
                    Q_ASSERT(stream->getNodeType() == AST::Stream);
                    if (stream->getNodeType() == AST::Stream) {
                        checkStreamConnections(static_pointer_cast<StreamNode>(stream), moduleScope, true);
                    }
                }
            }

        } else if (object->getNodeType() == AST::Stream) {
            checkStreamConnections(static_pointer_cast<StreamNode>(object), QVector<ASTNode >(), true);
        }
    }
}

void CodeResolver::insertDependentTypes(string typeName, map<string, vector<ASTNode>> &objects) {
    QList<std::shared_ptr<DeclarationNode>> blockList;
    QList<LangError> errors;
//    auto is_declared = CodeValidator::findTypeDeclaration(decl, QVector<ASTNode >(), m_tree, errors);
    std::shared_ptr<DeclarationNode> existingDecl = CodeValidator::findTypeDeclarationByName(typeName, QVector<ASTNode >(), m_tree, errors);
    for (auto it = objects.begin(); it != objects.end(); it++)  {
        // Look for type declaration in object lists
        if(!existingDecl) {
            auto newTypeDeclaration = CodeValidator::findTypeDeclarationByName(typeName, QVector<ASTNode >::fromStdVector(it->second), nullptr, errors);
            if (newTypeDeclaration) { // type declaration found
                newTypeDeclaration->setRootScope(it->first);
                m_tree->addChild(newTypeDeclaration);
                existingDecl = newTypeDeclaration;
                for (ASTNode child : existingDecl->getChildren()) {
                    insertBuiltinObjectsForNode(child, objects);
                }
            }
        }
        if (existingDecl) {
            vector<string> inheritedTypes = CodeValidator::getInheritedTypeNames(existingDecl, QVector<ASTNode>::fromStdVector(it->second), m_tree);
            for (auto inheritedType: inheritedTypes) {
                insertDependentTypes(inheritedType, objects);
            }
        }

        //            if (typeDeclaration && !blockList.contains(typeDeclaration)) {
        //                typeDeclaration->setRootScope(it->first);
        //                blockList << typeDeclaration;

        //                for(string &typeName : inheritedTypes) {
        //                    ASTNode existingDeclaration = CodeValidator::findTypeDeclarationByName(typeName, QVector<ASTNode >(), m_tree, errors);
        //                    if (!existingDeclaration) {
        //                        auto newTypeDeclaration = CodeValidator::findTypeDeclarationByName(typeName, QVector<ASTNode >::fromStdVector(rootNamespace), nullptr, errors);
        //                        if (newTypeDeclaration && !blockList.contains(newTypeDeclaration)) {
        //                            newTypeDeclaration->setRootScope(it->first);
        //                            blockList << newTypeDeclaration;
        //                        }
        //                    }
        //                }
        //            }
    }

    // Insert declaration for inherited types

//    // Add types that need to be added
//    for(auto usedBlock : blockList) {
////        QVector<ASTNode > subscope;
////        // Make subscope in case this is a module declaration
////        ASTNode blocks = usedBlock->getPropertyValue("blocks");
////        if (blocks) {
////            for(ASTNode declarationNode : blocks->getChildren()) {
////                if (declarationNode->getNodeType() == AST::Declaration
////                        || declarationNode->getNodeType() == AST::BundleDeclaration) {
////                    subscope << declarationNode;
////                }
////            }
////        }
//        // Add type. It shouldn't be there as this has been checked above
//        if (!CodeValidator::findDeclaration(QString::fromStdString(usedBlock->getName()),
//                                            QVector<ASTNode>(), m_tree)) {
//            m_tree->addChild(usedBlock);
//            for (auto it = objects.begin(); it != objects.end(); it++)  {
//                vector<ASTNode > &namespaceObjects = it->second;
//                auto position = std::find(namespaceObjects.begin(), namespaceObjects.end(), usedBlock);
//                if (position != namespaceObjects.end()) {
//                    namespaceObjects.erase(position);
//                }
//            }
//            insertDependentTypes(usedBlock, objects);
//            //            insertBuiltinObjectsForNode(newBlock, objects);
//        }

//    }
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
                blockList << declaration; // TODO this currently inserts the object from all available namespaces. This should be optimized, and only the needed ones should be inserted.
            }
        }
        // Look for declarations of blocks present in function properties
        for(auto property :func->getProperties()) {
            insertBuiltinObjectsForNode(property->getValue(), objects);
        }
        for (std::shared_ptr<DeclarationNode> usedBlock : blockList) {
            // Add declarations to tree if not there
            if (!CodeValidator::findDeclaration(QString::fromStdString(usedBlock->getName()), QVector<ASTNode >(), m_tree, usedBlock->getNamespaceList())) {
                insertBuiltinObjectsForNode(usedBlock, objects);
                for (std::shared_ptr<PropertyNode> property : usedBlock->getProperties()) {
                    insertBuiltinObjectsForNode(property->getValue(), objects);
                }
                ASTNode newBlock = usedBlock;
                m_tree->addChild(newBlock);
            }
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
        // Find type declaration and insert it if needed
        QList<LangError> errors;
        // Check if type is already declared

        insertDependentTypes(declaration->getObjectType(), objects);

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
            for(ASTNode object : it->second) {
                if (object->getNodeType() == AST::Declaration
                        || object->getNodeType() == AST::BundleDeclaration) {
                    std::shared_ptr<DeclarationNode> block = static_pointer_cast<DeclarationNode>(object);
                    if (block->getName() == name->getName()
                            && !blockList.contains(block)) {
                        if (!CodeValidator::findDeclaration(QString::fromStdString(block->getName()), QVector<ASTNode >(), m_tree, block->getNamespaceList())) {
                            //                        ASTNode usedObject = object;
                            //                        fillDefaultPropertiesForNode(usedObject);
                            //                        m_tree->addChild(usedObject);
                            block->setRootScope(it->first);
                            blockList << block;
                        }
                        break;

                    }
                }
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
    } else if (node->getNodeType() == AST::Bundle) {
        QList<std::shared_ptr<DeclarationNode>> blockList;
        for (auto it = objects.begin(); it != objects.end(); it++)  {
            for(ASTNode object : it->second) {
                if (object->getNodeType() == AST::Declaration
                        || object->getNodeType() == AST::BundleDeclaration) {
                    BundleNode *bundle = static_cast<BundleNode *>(node.get());
                    std::shared_ptr<DeclarationNode> block = static_pointer_cast<DeclarationNode>(object);
                    if (block->getName() == bundle->getName()
                            && !blockList.contains(block)) {
                        if (!CodeValidator::findDeclaration(QString::fromStdString(block->getName()), QVector<ASTNode >(), m_tree, block->getNamespaceList())) {
                            //                        ASTNode usedObject = object;
                            //                        fillDefaultPropertiesForNode(usedObject);
                            //                        m_tree->addChild(usedObject);

                            block->setRootScope(it->first);
                            blockList << block;
                        }
                        break;

                    }
                }
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

void CodeResolver::resolveDomainsForStream(std::shared_ptr<StreamNode> stream, QVector<ASTNode > scopeStack, QString contextDomain)
{
    ASTNode left = stream->getLeft();
    ASTNode right = stream->getRight();
    QList<ASTNode > domainStack;
    string previousDomainName = CodeValidator::getDomainNodeString(CodeValidator::getNodeDomain(left, scopeStack, m_tree));

    if (previousDomainName.size() == 0) {
        previousDomainName = contextDomain.toStdString();
    }
    string domainName;
    while (right) {
        domainName = processDomainsForNode(left, scopeStack, domainStack);
        if (left == right && domainName.size() == 0) { // If this is is the last pass then set the unknown domains to platform domain
            if (left->getNodeType() == AST::Function && domainName == "") {
                // Have functions/loops/reactions be assigned to whatever they are connected to
                // if they are final in a stream.
                if (previousDomainName.size() != 0) {
                    domainName = previousDomainName;
                } else {
                    domainName = contextDomain.toStdString();
                }
            } else {
                // This is needed in cases where signals with no set domain are connected
                // to input ports on other objects. Domains are not
                // propagated across secondary ports
                domainName = contextDomain.toStdString();
            }
            domainStack << left;
        }

        if (domainName.size() > 0 && previousDomainName != domainName) {
            setDomainForStack(domainStack, domainName, scopeStack);
            domainStack.clear();
        }
        previousDomainName = domainName;

        if (left == right) {
            if (domainName.size() > 0) {
                setDomainForStack(domainStack, domainName, scopeStack);
                domainStack.clear();
            }
            right = left = nullptr; // End
        } else if(right->getNodeType() == AST::Stream) {
            stream = static_pointer_cast<StreamNode>(right);
            left = stream->getLeft();
            right = stream->getRight();
        } else {
            left = right; // Last pass (process right, call it left)
        }
    }
}

std::string CodeResolver::processDomainsForNode(ASTNode node, QVector<ASTNode > scopeStack, QList<ASTNode > &domainStack)
{
    string domainName;
    resolveDomainForStreamNode(node, scopeStack);
    if (node->getNodeType() == AST::Block
            || node->getNodeType() == AST::Bundle) {
        std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(CodeValidator::streamMemberName(node, scopeStack, m_tree), scopeStack, m_tree, node->getNamespaceList());
        if(declaration) {
            ASTNode domain = declaration->getDomain();
            if (!domain) {
                // Put declaration in stack to set domain once domain is resolved
                domainStack << declaration;
            } else {
                if (domain->getNodeType() == AST::String) {
                    domainName = static_cast<ValueNode *>(domain.get())->getStringValue();
                } else if (domain->getNodeType() == AST::Block) {
                    QList<LangError> errors;
                    domainName = CodeValidator::evaluateConstString(domain, scopeStack, m_tree, errors);
                } else if (domain->getNodeType() == AST::None) {
                    // TODO does this need to be handled?
                    domainStack << declaration;
                } else if (domain->getNodeType() == AST::PortProperty) {
                    ASTNode resolvedDomain = resolvePortProperty(static_pointer_cast<PortPropertyNode>(domain),
                                                         scopeStack);
                    if (resolvedDomain) {
                        if (resolvedDomain->getNodeType() == AST::String) {
                            domainName = std::static_pointer_cast<ValueNode>(resolvedDomain)->getStringValue();
                        } else {
                            domainName = CodeValidator::getNodeDomainName(resolvedDomain, scopeStack, m_tree);
                        }
                        declaration->setDomainString(domainName);
                    }
                    //                    domainStack << declaration;
                } else {
                    domainStack << declaration;
                }
            }
        }
    } else if (node->getNodeType() == AST::Function) {
        FunctionNode *func = static_cast<FunctionNode *>(node.get());
        ASTNode domain = func->getDomain();
        if (!domain) {
            // Put declaration in stack to set domain once domain is resolved
            domainStack << node;
        } else {
            if (domain->getNodeType() == AST::String) {
                domainName = static_cast<ValueNode *>(domain.get())->getStringValue();
            } else if (domain->getNodeType() == AST::None) { // domain is streamDomain
                domainStack << node;
            } else {
                qDebug() << "WARNING: Unrecognized domain type"; // Should this trigger an error?
                domainStack << node;
            }
        }
        for(auto member : func->getProperties()) {
            string propertyDomainName = processDomainsForNode(member->getValue(), scopeStack, domainStack);
            if (propertyDomainName.size() == 0) {
                 domainStack << member->getValue();
            }
        }
    } else if (node->getNodeType() == AST::List) {
        QList<ASTNode > listDomainStack;
        for (ASTNode  member : node->getChildren()) {
            string memberDomainName = processDomainsForNode(member, scopeStack, domainStack);
            if (domainName.size() == 0 && memberDomainName.size() > 0) {
                domainName = memberDomainName; // list takes domain from first element that has a domain
            }
            if (memberDomainName.size() == 0) {
                // Put declaration in stack to set domain once domain is resolved

                listDomainStack << member;
                // FIMXE: This is very simplistic (or plain wrong....)
                // It assumes that the next found domain affects all elements
                // in the list that don't have domains. This is likely a
                // common case but list elements should inherit domains from
                // the port to which they are connected.
            }
        }
        if (domainName.size() > 0) {
            setDomainForStack(listDomainStack, domainName, scopeStack);
        } else {
            domainStack << listDomainStack; // If list has no defined domain, pass all elements to be set later
        }
    } else if (node->getNodeType() == AST::Expression) {
        for(ASTNode  member : node->getChildren()) {
            string newDomainName = processDomainsForNode(member, scopeStack, domainStack);
            // if member doesn't have a domain, put in the stack to resolve later.
            if (newDomainName.size() == 0) {
                // Put declaration in stack to set domain once domain is resolved
                domainStack << member;
            }
        }
    }
    return domainName;
}

void CodeResolver::setDomainForStack(QList<ASTNode > domainStack, string domainName,  QVector<ASTNode > scopeStack)
{
    // Add domain declaration if not already present (This might happen if
    // the domain is resolved after and it has not been explicitly declared,
    // so the insertBuiltinObjects() function will not insert it)
    bool domainFound = false;
    for(ASTNode node: m_tree->getChildren()) {
        if (node->getNodeType() == AST::Declaration) {
            std::shared_ptr<DeclarationNode> decl = static_pointer_cast<DeclarationNode>(node);
            if (decl->getObjectType() == "_domainDefinition") {
                ASTNode domainNameNode = decl->getPropertyValue("domainName");
                if (domainNameNode->getNodeType() == AST::String) {
                    if (static_cast<ValueNode *>(domainNameNode.get())->getStringValue() == domainName) {
                        // FIXME We need to check the namespace of the domain!
                        domainFound = true;
                        break;
                    }
                }
            }
        }
    }
    if (!domainFound) {
        map<string, vector<ASTNode >> bultinObjects;
        if (m_system) {
            bultinObjects = m_system->getBuiltinObjectsReference();
        }

        // FIXME shouldnt this have happened in the insert built-in objects function
        for (auto it = bultinObjects.begin(); it != bultinObjects.end(); it++)  {
            for (ASTNode object : it->second) {
                if (object->getNodeType() == AST::Declaration) {
                    std::shared_ptr<DeclarationNode>block = static_pointer_cast<DeclarationNode>(object);
                    if (block->getObjectType() == "_domainDefinition") {
                        ASTNode nameNode = block->getPropertyValue("domainName");
                        if (nameNode->getNodeType() == AST::String) {
                            ValueNode *typeName = static_cast<ValueNode *>(nameNode.get());
                            if (typeName->getStringValue() == domainName) {
                                // FIXME We need to check the namespace of the domain!
                                block->setRootScope(it->first);
                                m_tree->addChild(block);
                                fillDefaultPropertiesForNode(m_tree->getChildren().back());
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    for (ASTNode relatedNode : domainStack) {
        if (relatedNode->getNodeType() == AST::Declaration
                || relatedNode->getNodeType() == AST::BundleDeclaration ) {
            std::shared_ptr<DeclarationNode> block = static_pointer_cast<DeclarationNode>(relatedNode);
            if (block) {
                block->setDomainString(domainName);
            }
        } else if (relatedNode->getNodeType() == AST::Block
                   || relatedNode->getNodeType() == AST::Bundle) {
            std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(CodeValidator::streamMemberName(relatedNode, scopeStack, m_tree), scopeStack, m_tree);
            if(declaration) {
                declaration->setDomainString(domainName);
            }
        } else if (relatedNode->getNodeType() == AST::Function) {
            std::shared_ptr<FunctionNode> func = static_pointer_cast<FunctionNode>(relatedNode);
            if (func) {
                func->setDomainString(domainName);
            }
        }  else if (relatedNode->getNodeType() == AST::List
                    || relatedNode->getNodeType() == AST::Expression) {
            for(ASTNode  member : relatedNode->getChildren()) {
                if (member->getNodeType() == AST::Block) {
                    string name = static_cast<BlockNode *>(member.get())->getName();
                    std::shared_ptr<DeclarationNode> block =  CodeValidator::findDeclaration(QString::fromStdString(name), scopeStack, m_tree);
                    if (block) {
                        if (!block->getDomain() || block->getDomain()->getNodeType() == AST::None) {
                            // Don't override expression member domains. Only set if domain is not there.
                            block->setDomainString(domainName);
                        }
                    }
                } else if (member->getNodeType() == AST::Bundle) {
                    string name = static_cast<BundleNode *>(member.get())->getName();
                    std::shared_ptr<DeclarationNode> block =  CodeValidator::findDeclaration(QString::fromStdString(name), scopeStack, m_tree);
                    if (block) {
                        if (!block->getDomain() || block->getDomain()->getNodeType() == AST::None) {
                            // Don't override expression member domains. Only set if domain is not there.
                            block->setDomainString(domainName);
                        }
                    }
                } else if (member->getNodeType() == AST::Function) {
                    std::shared_ptr<FunctionNode> func = static_pointer_cast<FunctionNode>(member);
                    func->setDomainString(domainName);
                }
            }
        }
    }
}

std::shared_ptr<DeclarationNode>CodeResolver::createDomainDeclaration(QString name)
{
    std::shared_ptr<DeclarationNode>newBlock = nullptr;
    newBlock = std::make_shared<DeclarationNode>(name.toStdString(), "_domainDefinition", nullptr, "", -1);
    newBlock->addProperty(std::make_shared<PropertyNode>("domainName", std::make_shared<ValueNode>(name.toStdString(), "", -1), "", -1));
    fillDefaultPropertiesForNode(newBlock);
    return newBlock;
}

std::shared_ptr<DeclarationNode> CodeResolver::createSignalDeclaration(QString name, int size, QVector<ASTNode> &scope)
{
    std::shared_ptr<DeclarationNode> newBlock = nullptr;
    Q_ASSERT(size > 0);
    if (size == 1) {
        newBlock = std::make_shared<DeclarationNode>(name.toStdString(), "signal", nullptr, "", -1);
    } else if (size > 1) {
        std::shared_ptr<ListNode> indexList = std::make_shared<ListNode>(std::make_shared<ValueNode>(size, "",-1), "", -1);
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
    std::shared_ptr<DeclarationNode> constant = std::make_shared<DeclarationNode>(name, "constant", nullptr, "", -1);
    std::shared_ptr<PropertyNode> valueProperty = std::make_shared<PropertyNode>("value", value, "", -1);
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
        newBridge = std::make_shared<DeclarationNode>(bridgeName, "signalbridge", nullptr, "", -1);
    } else { // A BlockBundle
        newBridge = std::make_shared<DeclarationNode>(std::make_shared<BundleNode>(bridgeName, std::make_shared<ListNode>(std::make_shared<ValueNode>(size, "", -1), "", -1), "", -1),
                                                              "signalbridge", nullptr, "", -1);
    }
    newBridge->addProperty(std::make_shared<PropertyNode>("default", defaultValue, filename.c_str(), line));
    newBridge->addProperty(std::make_shared<PropertyNode>("signal", std::make_shared<ValueNode>(originalName, filename.c_str(), line),
                                                          filename.c_str(), line));
    if (inDomain) {
        newBridge->addProperty(std::make_shared<PropertyNode>("inputDomain", inDomain, filename.c_str(), line));
        newBridge->addProperty(std::make_shared<PropertyNode>("domain", inDomain, filename.c_str(), line));
    } else {
        newBridge->addProperty(std::make_shared<PropertyNode>("inputDomain", std::make_shared<ValueNode>("", -1), filename.c_str(), line));
        newBridge->addProperty(std::make_shared<PropertyNode>("domain", std::make_shared<ValueNode>("", -1), filename.c_str(), line));
    }
    if (outDomain) {
        newBridge->addProperty(std::make_shared<PropertyNode>("outputDomain", outDomain, filename.c_str(), line));
    } else {
        newBridge->addProperty(std::make_shared<PropertyNode>("outputDomain", std::make_shared<ValueNode>("", -1), filename.c_str(), line));
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
        if (size <= 0) { // Look to the right if can't resolve from the left
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
        int size = CodeValidator::getNodeNumOutputs(left, localScope, m_tree, errors);
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
    if (blocksNode->getNodeType() == AST::Declaration) {
        blocks.push_back(blocksNode);
    } else if (blocksNode->getNodeType() == AST::List) {
        for(ASTNode  node: blocksNode->getChildren()) {
            if (node->getNodeType() == AST::Declaration) {
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
            std::shared_ptr<DeclarationNode> reactionInput = std::make_shared<DeclarationNode>("_TriggerInput", "propertyInputPort", nullptr,"", -1);
            reactionInput->setPropertyValue("block", std::make_shared<BlockNode>("_Trigger", "", -1));

            ListNode *ports = static_cast<ListNode *>(block->getPropertyValue("ports").get());
            if (ports && ports->getNodeType() == AST::None) {
                block->replacePropertyValue("ports", std::make_shared<ListNode>(nullptr, "", -1));
                ports = static_cast<ListNode *>(block->getPropertyValue("ports").get());
            }
            ports->addChild(reactionInput);
        }
        if (block->getObjectType() == "loop") { // We need to define an internal domain for loops
            ListNode *ports = static_cast<ListNode *>(block->getPropertyValue("ports").get());
            if (!ports) {
                block->setPropertyValue("ports", std::make_shared<ValueNode>("", -1)); // Make a None node to trigger next branch
            }
            if (ports && ports->getNodeType() == AST::None) {
                block->replacePropertyValue("ports", std::make_shared<ListNode>(nullptr, "", -1));
                ports = static_cast<ListNode *>(block->getPropertyValue("ports").get());
            }
            ASTNode internalBlocks = block->getPropertyValue("blocks");
//            internalBlocks->addChild(createDomainDeclaration(QString::fromStdString("_OutputDomain")));
        }
        if (block->getObjectType() == "module" || block->getObjectType() == "reaction" || block->getObjectType() == "loop" ) {
            // First insert and resolve input and output domains for main ports. The input port takes the output domain if undefined.

            ASTNode internalBlocks = block->getPropertyValue("blocks");
            if (!internalBlocks || internalBlocks->getNodeType() != AST::List) { // Turn blocks property into list
                block->setPropertyValue("blocks", std::make_shared<ListNode>(internalBlocks, "", -1));
            }
            for (ASTNode node: internalBlocks->getChildren()) {
                subScope << node;
            }
            std::shared_ptr<DeclarationNode> outputPortBlock = CodeValidator::getMainOutputPortBlock(block);

            if (outputPortBlock) {
                ASTNode outDomain = outputPortBlock->getPropertyValue("domain");
                if (!outDomain || outDomain->getNodeType() == AST::None) {
                    ASTNode portBlock = outputPortBlock->getPropertyValue("block");
                    bool gotDomainFromBlock = false;
                    if (portBlock) { // If Output port has no domain set, try getting it from its block
                        std::string domainName
                                = CodeValidator::getNodeDomainName(portBlock,
                                                                   QVector<ASTNode>::fromStdVector(internalBlocks->getChildren()),
                                                                   m_tree);
                        if (domainName.size() > 0) {
                            outputPortBlock->replacePropertyValue("domain",
                                                                  std::make_shared<ValueNode>(domainName, "", -1));
                            gotDomainFromBlock = true;
                        }
                    }
                    if (!gotDomainFromBlock) {
                        int counter = 0;
                        string newDomainName;
                        bool domainAvailable;
                        do {
                            newDomainName = "_OutputDomain_" + std::to_string(counter);
                            domainAvailable = true;
                            for (ASTNode node: internalBlocks->getChildren()) {
                                if (node->getNodeType() == AST::Declaration) {
                                    auto decl = static_pointer_cast<DeclarationNode>(node);
                                    if (decl->getName() == newDomainName) {
                                        domainAvailable = false;
                                        counter++;
                                        break;
                                    }
                                }
                            }
                        } while (!domainAvailable);

                        std::shared_ptr<BlockNode> outBlockName = std::make_shared<BlockNode>(newDomainName, "", -1);
                        outputPortBlock->replacePropertyValue("domain", outBlockName); // FIXME We should we issue a warning that we are overwriting declared domain
                        outDomain = outBlockName;
                        ASTNode outDomainDeclaration = CodeValidator::findDeclaration(QString::fromStdString(newDomainName), QVector<ASTNode>(), internalBlocks);
                        if (!outDomainDeclaration) {
                            std::shared_ptr<DeclarationNode> newDomainBlock = createDomainDeclaration(QString::fromStdString(newDomainName));
                            subScope << newDomainBlock;
                            internalBlocks->addChild(newDomainBlock);
                        }
                    }
                }
                std::shared_ptr<DeclarationNode> inputPortBlock = CodeValidator::getMainInputPortBlock(block);
                if (inputPortBlock) {
                    ASTNode inDomain = inputPortBlock->getPropertyValue("domain");
                    if (!inDomain || inDomain->getNodeType() == AST::None) {

                        ASTNode portBlock = inputPortBlock->getPropertyValue("block");
                        bool gotDomainFromBlock = false;
                        if (portBlock) { // If Output port has no domain set, try getting it from its block
                            std::string domainName
                                    = CodeValidator::getNodeDomainName(portBlock,
                                                                       QVector<ASTNode>::fromStdVector(internalBlocks->getChildren()),
                                                                       m_tree);
                            if (domainName.size() > 0) {
                                inputPortBlock->replacePropertyValue("domain",
                                                                      std::make_shared<ValueNode>(domainName, "", -1));
                                gotDomainFromBlock = true;
                            }
                        }
                        if (!gotDomainFromBlock) {
                            int counter = 0;
                            string newDomainName;
                            bool domainAvailable;
                            do {
                                newDomainName = "_InputDomain_" + std::to_string(counter);
                                domainAvailable = true;
                                for (ASTNode node: internalBlocks->getChildren()) {
                                    if (node->getNodeType() == AST::Declaration) {
                                        auto decl = static_pointer_cast<DeclarationNode>(node);
                                        if (decl->getName() == newDomainName) {
                                            domainAvailable = false;
                                            counter++;
                                            break;
                                        }
                                    }
                                }
                            } while (!domainAvailable);



                            std::shared_ptr<BlockNode> inBlockName = std::make_shared<BlockNode>(newDomainName, "", -1);
                            inputPortBlock->replacePropertyValue("domain", inBlockName); // FIXME We should we issue a warning that we are overwriting declared domain
//                            outDomain = outBlockName;
                            ASTNode inDomainDeclaration = CodeValidator::findDeclaration(QString::fromStdString(newDomainName), QVector<ASTNode>(), internalBlocks);
                            if (!inDomainDeclaration) {
                                std::shared_ptr<DeclarationNode> newDomainBlock = createDomainDeclaration(QString::fromStdString(newDomainName));
                                subScope << newDomainBlock;
                                internalBlocks->addChild(newDomainBlock);
                            }


//                        BlockNode *domainNameNode = static_cast<BlockNode *>(outDomain.get());
//                        std::shared_ptr<DeclarationNode> domainDeclaration = CodeValidator::findDeclaration(
//                                    //FIXME use scope stack, not only this module's blocks
//                                    QString::fromStdString(domainNameNode->getName()), QVector<ASTNode>::fromStdVector(internalBlocks->getChildren()), m_tree);
//                        if (domainDeclaration) {
//                            ASTNode domainValue = domainDeclaration->getPropertyValue("domainName");
//                            std::shared_ptr<BlockNode> outBlockName = std::make_shared<BlockNode>(static_pointer_cast<ValueNode>(domainValue)->getStringValue() , "", -1);
//                            inputPortBlock->replacePropertyValue("domain", outBlockName);
//                        }
                        }
                    }
                }
            } else {
                // TODO This assumes that the output block determines the context domain, should we look at the input port too if output port not set?
                ASTNode outDomainDeclaration = CodeValidator::findDeclaration("_OutputDomain_0", QVector<ASTNode>(), internalBlocks);
                if (!outDomainDeclaration) {
                    std::shared_ptr<DeclarationNode> newDomainBlock = createDomainDeclaration(QString::fromStdString("_OutputDomain_0"));
                    internalBlocks->addChild(newDomainBlock);
                    subScope << newDomainBlock;
                }
            }

            // Then go through ports autodeclaring blocks
            ListNode *ports = static_cast<ListNode *>(block->getPropertyValue("ports").get());
            if (ports->getNodeType() == AST::List) {
                for (ASTNode port : ports->getChildren()) {
                    Q_ASSERT(port->getNodeType() == AST::Declaration);
                    DeclarationNode *portDeclaration = static_cast<DeclarationNode *>(port.get());

                    // Properties that we need to auto-declare for
                    ASTNode ratePortValue = portDeclaration->getPropertyValue("rate");
                    ASTNode domainPortValue = portDeclaration->getPropertyValue("domain");
                    ASTNode blockPortValue = portDeclaration->getPropertyValue("block");
                    ASTNode internalBlocks = block->getPropertyValue("blocks");

                    // Declare domain block if undeclared
                    std::string domainName = "_" + portDeclaration->getName() + "Domain";
                    if (domainPortValue) {
                        if (domainPortValue->getNodeType() == AST::None) {
                            std::shared_ptr<DeclarationNode> domainDeclaration = CodeValidator::findDeclaration(QString::fromStdString(domainName), subScope, m_tree);
                            if (!domainDeclaration) {
                                domainDeclaration = createDomainDeclaration(QString::fromStdString(domainName));
                                internalBlocks->addChild(domainDeclaration);
                            }
                            std::shared_ptr<ValueNode> domainNameNode = std::make_shared<ValueNode>(domainName, "", -1);
                            portDeclaration->replacePropertyValue("domain", domainNameNode);
                            domainPortValue = portDeclaration->getPropertyValue("domain");
                        } else if (domainPortValue->getNodeType() == AST::Block) { // Auto declare domain if not declared
                            std::shared_ptr<BlockNode> nameNode = static_pointer_cast<BlockNode>(domainPortValue);
                            // FIXME hack to simplify domain name resolution.
                            // There should be some lookup here from the block name to domain name
                            domainName = nameNode->getName();
                            std::shared_ptr<DeclarationNode> domainDeclaration = CodeValidator::findDeclaration(QString::fromStdString(domainName), subScope, m_tree);
                            if (!domainDeclaration) {
                                std::shared_ptr<DeclarationNode> domainDeclaration = createDomainDeclaration(QString::fromStdString(domainName));
                                internalBlocks->addChild(domainDeclaration);
                                subScope << domainDeclaration;
                                // Give assigned block this domain if it doesn't have one.
                                auto assignedBlock = portDeclaration->getPropertyValue("block");
                                if (assignedBlock) {
                                    std::string blockDomain = CodeValidator::getNodeDomainName(assignedBlock,
                                                                                               subScope,
                                                                                               m_tree);
                                    if (blockDomain.size() == 0 && assignedBlock->getNodeType() == AST::Block) { // TODO implement for bundles
                                        std::string blockName = std::static_pointer_cast<BlockNode>(assignedBlock)->getName();
                                        auto decl = CodeValidator::findDeclaration(QString::fromStdString(blockName), subScope, m_tree);
                                        if (!decl) {
                                            decl = createSignalDeclaration(QString::fromStdString(blockName), 1, subScope);
                                            fillDefaultPropertiesForNode(decl);
                                            internalBlocks->addChild(decl);
                                        }
                                        decl->setDomainString(domainName);
                                    }
                                }
                            }
                        }
                    }

                    // Declare block in rate port if not declared
                    if (ratePortValue) {
                        if (ratePortValue->getNodeType() == AST::Block) {
                            BlockNode *nameNode = static_cast<BlockNode *>(ratePortValue.get());
                            string name = nameNode->getName();
                            // TODO should this declaraion default to platform rate or something else?
                            declareIfMissing(name, internalBlocks, std::make_shared<ValueNode>(0, "", -1));

                            //                                    if (declaration->getObjectType() == "constant") {
                            //                                        // If existing declaration is not a constant then an error should be produced later when checking types
                            //                                        ASTNode value = declaration->getPropertyValue("value");
                            //                                        if (value)
                            //                                    }
                        } else if (ratePortValue->getNodeType() == AST::Int || ratePortValue->getNodeType() == AST::Real) {
                            // Do nothing
                        } else if (ratePortValue->getNodeType() == AST::None) {
                            // Do nothing
                        }  else {
                            qDebug() << "Rate unrecognized.";
                        }
                    }
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
                                blockDecl->replacePropertyValue("rate", std::make_shared<ValueNode>("", -1));
                                blockDecl->setDomainString(domainName);
                                internalBlocks->addChild(blockDecl);
                                // TODO This default needs to be done per instance
                                ASTNode portDefault = portDeclaration->getPropertyValue("default");
                                if (portDefault && portDefault->getNodeType() != AST::None) {
                                    Q_ASSERT(blockDecl->getPropertyValue("default"));
                                    blockDecl->replacePropertyValue("default", portDefault);
                                }
                                //                                    if (direction == "input") {
                                //                                    } else if (direction == "output") {

                                //                                    }
                            } else { // If port block declared then set its domain to the port domain if not set
                                // TODO there should be an error check to verify that port blocks have the port domain
                                ASTNode blockDomain = blockDecl->getDomain();
                                //                                Q_ASSERT(blockDomain); // Constants don't have domain...
                                if (!blockDomain || blockDomain->getNodeType() == AST::None) {
                                    blockDecl->setDomainString(domainName);
                                }
                            }
                        } else if (blockPortValue->getNodeType() == AST::None) {
                            string defaultName;
                            if (portDeclaration->getObjectType() == "mainInputBlock") {
                                defaultName = "Input";
                           } else if (portDeclaration->getObjectType() == "mainOutputBlock") {
                                defaultName = "Output";
                            }
                            std::shared_ptr<BlockNode> name = std::make_shared<BlockNode>(defaultName, "", -1);
                            portDeclaration->replacePropertyValue("block", name);
                            std::shared_ptr<DeclarationNode> newSignal = CodeValidator::findDeclaration(QString::fromStdString(defaultName), QVector<ASTNode >(), internalBlocks);
                            if (!newSignal) {
                                newSignal = createSignalDeclaration(QString::fromStdString(defaultName), 1, subScope);
                                internalBlocks->addChild(newSignal);
                                fillDefaultPropertiesForNode(newSignal);
                                newSignal->setDomainString(domainName);
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
                   decl->replacePropertyValue("value", std::make_shared<ValueNode>(override.value().toString().toStdString(), "", -1));
               } else if (override.value().type() == QVariant::Int) {
                   decl->replacePropertyValue("value", std::make_shared<ValueNode>(override.value().toInt(), "", -1));
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
    processResetForNode(m_tree, m_tree, std::make_shared<ListNode>(nullptr, "", -1));
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
                                                    "", -1);
        std::shared_ptr<ListNode> streamList = std::make_shared<ListNode>(nullptr, "", -1);

        std::shared_ptr<StreamNode> stream
                = std::make_shared<StreamNode>(pair.first->getPropertyValue("default"),
                                               std::make_shared<BlockNode>(pair.first->getName(), "", -1), "", -1);
        fillDefaultPropertiesForNode(stream);
        streamList->addChild(stream);
        newReaction->setPropertyValue("streams", streamList);
        newReaction->setPropertyValue("blocks", std::make_shared<ListNode>(nullptr, "", -1));
        newReaction->setPropertyValue("ports", std::make_shared<ListNode>(nullptr, "", -1));
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
                            std::make_shared<StreamNode>(std::make_shared<BlockNode>(pair.second, "", -1),
                                                         std::make_shared<FunctionNode>(reactionName,
                                                                                        std::make_shared<ListNode>(nullptr, "", -1),
                                                                                        "", -1), "", -1)
                    );
        }
        streamScope->setChildren(childStreams);

    }
}

void CodeResolver::propagateDomainsForNode(ASTNode node, QVector<ASTNode > scopeStack)
{
    if (node->getNodeType() == AST::Stream) {
        string contextDomain = getContextDomainName(scopeStack);
        resolveDomainsForStream(static_pointer_cast<StreamNode>(node), scopeStack, QString::fromStdString(contextDomain));
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

            scopeStack << CodeValidator::getBlockSubScope(module);

            string contextDomainName = getContextDomainName(scopeStack);
            if (contextDomainName.size() == 0) {
                std::shared_ptr<DeclarationNode> outputPortDecl = CodeValidator::getMainOutputPortBlock(module);
                if (outputPortDecl) {
                    ASTNode domainNode = outputPortDecl->getPropertyValue("domain");
                    if (domainNode->getNodeType() == AST::Block || domainNode->getNodeType() == AST::Bundle ) {
                        contextDomainName = static_cast<BlockNode *>(domainNode.get())->getName();
                    } else if (domainNode->getNodeType() == AST::String) {
                        contextDomainName = static_cast<ValueNode *>(domainNode.get())->getStringValue();
                    }
                } else {
                    std::shared_ptr<DeclarationNode> inputPortDecl = CodeValidator::getMainInputPortBlock(module);
                    if (inputPortDecl) {
                        ASTNode domainNode = inputPortDecl->getPropertyValue("domain");
                        if (domainNode->getNodeType() == AST::Block || domainNode->getNodeType() == AST::Bundle ) {
                            contextDomainName = static_cast<BlockNode *>(domainNode.get())->getName();
                        } else if (domainNode->getNodeType() == AST::String) {
                            contextDomainName = static_cast<ValueNode *>(domainNode.get())->getStringValue();
                        }
                    }
                }
            }

            std::shared_ptr<DeclarationNode> contextDomainDecl = CodeValidator::findDeclaration(QString::fromStdString(contextDomainName),
                                                                                                scopeStack,
                                                                                                m_tree);

            while(streamIt != streamsNode.rend()) {
                const ASTNode streamNode = *streamIt;
                if (streamNode->getNodeType() == AST::Stream) {
                    resolveDomainsForStream(static_pointer_cast<StreamNode>(streamNode), scopeStack, QString::fromStdString(contextDomainName));
                } else {
                    qDebug() << "ERROR: Expecting stream.";
                }
                streamIt++;
            }
//            scopeStack << QVector<ASTNode>::fromStdVector(moduleBlocks);
            for (auto block: blocks) {
                propagateDomainsForNode(block, scopeStack);
            }
            if (contextDomainDecl) {
                setContextDomain(blocks, contextDomainDecl);
            }
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

void CodeResolver::terminateStackWithBridge(ASTNode node, QVector<ASTNode > &streams, QVector<ASTNode > &stack, QVector<ASTNode > &scopeStack)
{
    int size = CodeValidator::getNodeSize(node, scopeStack, m_tree);
    std::string connectorName = "_BridgeSig_" + std::to_string(m_connectorCounter++);
    ASTNode closingName = nullptr;
    std::shared_ptr<StreamNode> newStream = nullptr;
    ASTNode newStart = nullptr;
    vector<ASTNode > newDeclarations;
    if (stack.size() > 0) {
        ASTNode &stackBack = stack.back();
        if  (stackBack->getNodeType() == AST::List) { // Slice list

            bool listIsConst = true;
            for (auto child: stackBack->getChildren()) {
                if (!(child->getNodeType() == AST::Int
                      || child->getNodeType() == AST::Real
                      || child->getNodeType() == AST::String
                      || child->getNodeType() == AST::Switch)) {
                    listIsConst = false;
                    break;
                }
            }
            if (listIsConst) {
                stack << node; // Make direct connection.
                return;
            }
            closingName = std::make_shared<ListNode>(nullptr, "", node->getLine());
            newStart = std::make_shared<ListNode>(nullptr, "", node->getLine());
            // Set the domain for all members of list
            for (unsigned int i = 0; i < stackBack->getChildren().size(); i++) {
                string listConnectorName = connectorName + "_" + std::to_string(i);
                std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(CodeValidator::streamMemberName(stack.back()->getChildren()[i], scopeStack, m_tree),
                                                                                              scopeStack, m_tree);
                if (declaration) {
                    string type = declaration->getObjectType();
                    ASTNode valueNode = declaration->getPropertyValue("default");
                    if (!valueNode) {
                        valueNode =  std::make_shared<ValueNode>(0, "", -1);
                    }
                    std::string nodeDomainName = CodeValidator::getNodeDomainName(stackBack->getChildren()[i], scopeStack, m_tree);
                    if (nodeDomainName.size() > 0) {
                        newDeclarations.push_back(createSignalBridge(listConnectorName, nodeDomainName,
                                                                     valueNode,
                                                                     std::make_shared<BlockNode>(nodeDomainName, "", -1), std::make_shared<ValueNode>("", -1),
                                                                     stackBack->getChildren()[i]->getFilename(), stackBack->getChildren()[i]->getLine(),
                                                                     1, type));
                        closingName->addChild(std::make_shared<BlockNode>(listConnectorName, "", -1));
                        newStart->addChild(std::make_shared<BlockNode>(listConnectorName, "", -1));
                        std::shared_ptr<StreamNode> newStream = std::make_shared<StreamNode>(stack.back()->getChildren()[i],
                                                                                             std::make_shared<BlockNode>(listConnectorName, "", -1),
                                                                                             declaration->getFilename().c_str(), declaration->getLine());
                        newDeclarations.push_back(newStream);

                    } else if (stackBack->getChildren()[i]->getNodeType() == AST::Int
                               || stackBack->getChildren()[i]->getNodeType() == AST::Real
                               || stackBack->getChildren()[i]->getNodeType() == AST::String
                               || stackBack->getChildren()[i]->getNodeType() == AST::Switch ){
                        std::shared_ptr<DeclarationNode> constDeclaration = createConstantDeclaration(listConnectorName, stackBack->getChildren()[i]);
                        newDeclarations.push_back(constDeclaration);
                        closingName->addChild(std::make_shared<BlockNode>(listConnectorName, "", -1));
                        newStart->addChild(std::make_shared<BlockNode>(listConnectorName, "", -1));
                    }
//                    ASTNode valueNode = declaration->getPropertyValue("default");
//                    if (!valueNode) {
//                        valueNode =  std::make_shared<ValueNode>(0, "", -1);
//                    }
//                    QString memberName = CodeValidator::streamMemberName(node, scopeStack, m_tree);
//                    std::shared_ptr<DeclarationNode> nextDecl = CodeValidator::findDeclaration(memberName, scopeStack, m_tree);
//                    if (nextDecl) {
//                        newDeclarations.push_back(createSignalBridge(listConnectorName, memberName.toStdString(), valueNode,
//                                                                     declaration->getDomain(), nextDecl->getDomain(),
//                                                                     declaration->getFilename(), declaration->getLine()));
//                    } else {
//                        std::shared_ptr<ValueNode> noneValue = std::make_shared<ValueNode>("", -1);
//                        newDeclarations.push_back(createSignalBridge(listConnectorName, memberName.toStdString(), valueNode,
//                                                                     declaration->getDomain(), noneValue,
//                                                                     declaration->getFilename(), declaration->getLine()));
//                    }
//                    std::shared_ptr<StreamNode> newStream = std::make_shared<StreamNode>(stack.back()->getChildren()[i],
//                                                                                         std::make_shared<BlockNode>(listConnectorName, "", -1),
//                                                                                         declaration->getFilename().c_str(), declaration->getLine());
//                    newDeclarations.push_back(newStream);
//                    closingName->addChild(std::make_shared<BlockNode>(listConnectorName, "", -1));
//                    newStart->addChild(std::make_shared<BlockNode>(listConnectorName, "", -1));
                } else {
                    std::string nodeDomainName = CodeValidator::getNodeDomainName(stackBack->getChildren()[i], scopeStack, m_tree);
                    if (nodeDomainName.size() > 0) {
                        newDeclarations.push_back(createSignalBridge(listConnectorName, nodeDomainName,
                                                                     std::make_shared<ValueNode>("", -1),
                                                                     std::make_shared<BlockNode>(nodeDomainName, "", -1), std::make_shared<ValueNode>("", -1),
                                                                     stackBack->getChildren()[i]->getFilename(), stackBack->getChildren()[i]->getLine(), 1, "signal"));
                        closingName->addChild(std::make_shared<BlockNode>(listConnectorName, "", -1));
                        newStart->addChild(std::make_shared<BlockNode>(listConnectorName, "", -1));

                    } else if (stackBack->getChildren()[i]->getNodeType() == AST::Int
                               || stackBack->getChildren()[i]->getNodeType() == AST::Real
                               || stackBack->getChildren()[i]->getNodeType() == AST::String
                               || stackBack->getChildren()[i]->getNodeType() == AST::Switch ){
                        std::shared_ptr<DeclarationNode> constDeclaration = createConstantDeclaration(listConnectorName, stackBack->getChildren()[i]);
                        newDeclarations.push_back(constDeclaration);
                        closingName->addChild(std::make_shared<BlockNode>(listConnectorName, "", -1));
                        newStart->addChild(std::make_shared<BlockNode>(listConnectorName, "", -1));
                    }
                }
            }
        } else { // Slice everything else that is not a list
            QString memberName = CodeValidator::streamMemberName(stackBack, scopeStack, m_tree);
            std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(memberName, scopeStack, m_tree);
            if (declaration
                    && (declaration->getObjectType() == "signal"
                        || declaration->getObjectType() == "switch"
                        || declaration->getObjectType() == "trigger")
                    ) {
                string type = declaration->getObjectType();
                newDeclarations.push_back(createSignalBridge(connectorName, memberName.toStdString(),
                                                             declaration->getPropertyValue("default"),
                                                             declaration->getDomain(), std::make_shared<ValueNode>("", -1),
                                                             declaration->getFilename(), declaration->getLine(),
                                                             size, type));
            } else if (stackBack->getNodeType() == AST::Expression
                       || stackBack->getNodeType() == AST::Function){
//                        ASTNode domain = CodeValidator::
                // TODO set in/out domains correctly
                // FIXME set default value correctly
                string type = "signal"; // FIXME assumes that expressions and functions output signals...
                newDeclarations.push_back(createSignalBridge(connectorName, memberName.toStdString(),
                                                             std::make_shared<ValueNode>(0.0,"", -1),
                                                             std::make_shared<ValueNode>("", -1), std::make_shared<ValueNode>("", -1),
                                                             stackBack->getFilename(), stackBack->getLine(),
                                                             size, type));

            } else {
                newDeclarations.push_back(createSignalBridge(connectorName, memberName.toStdString(),
                                                             std::make_shared<ValueNode>(0.0,"", -1),
                                                             std::make_shared<ValueNode>("", -1), std::make_shared<ValueNode>("", -1),
                                                             stackBack->getFilename(), stackBack->getLine(),
                                                             size, "signal"));
            }
            closingName = std::make_shared<BlockNode>(connectorName, "", -1);
            newStart = std::make_shared<BlockNode>(connectorName, "", -1);
            newStream = std::make_shared<StreamNode>(stack.back(), closingName, node->getFilename().c_str(), node->getLine());
        }
//        stack.pop_back();
//        while (stack.size() > 0) {
//            ASTNode lastNode = stack.back();
//            stack.pop_back();
//        }
        // FIXME this looks very wrong... Shouldn't we create a stream from the stack???
        stack.clear();
    } else if (stack.size() == 0) {
        newStream = std::make_shared<StreamNode>(closingName, node, node->getFilename().c_str(), node->getLine());
    }
    for (ASTNode declaration: newDeclarations) {
        streams << declaration;
    }
    if (newStream) {
        streams << newStream;
    }
    stack << newStart << node;
}

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
        }

        if (left->getNodeType() == AST::Expression) {
            // We might need to add bridge signals if expression members belong to different domains
            // Resolve domain from the right member in stream (if domain in expression members is not set
            ASTNode outDomain;
            if (right) {
                outDomain = CodeValidator::getNodeDomain(right, scopeStack, m_tree);
                std::shared_ptr<ExpressionNode> expr = static_pointer_cast<ExpressionNode>(left);
                QVector<ASTNode > newStreams = processExpression(expr, scopeStack, outDomain);
                streams << newStreams;
            }
        } else if (left->getNodeType() == AST::Function) {
            std::shared_ptr<FunctionNode> func = static_pointer_cast<FunctionNode>(left);
            vector<std::shared_ptr<PropertyNode>> properties = func->getProperties();
            for(auto prop: properties) {
                ASTNode value = prop->getValue();
                if (value->getNodeType() == AST::Block) {
                    std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(CodeValidator::streamMemberName(value, scopeStack, m_tree),
                                                                                                  scopeStack, m_tree);
                    if (declaration) {
                        std::string connectorName = "_BridgeSig_" + std::to_string(m_connectorCounter++);
                        int size = 1;
                        if (declaration->getNodeType() == AST::BundleDeclaration) {
                            Q_ASSERT(declaration->getBundle()->index()->getChildren()[0]->getNodeType() == AST::Int);
                            size = static_cast<ValueNode *>(declaration->getBundle()->index()->getChildren()[0].get())->getIntValue();
                        }
                        ASTNode defaultProperty = declaration->getPropertyValue("default");
                        ASTNode bridgeDomain = declaration->getDomain();
                        if (defaultProperty && bridgeDomain) {
                            std::shared_ptr<BlockNode> block = static_pointer_cast<BlockNode>(value);
                            std::shared_ptr<ValueNode> noneValue = std::make_shared<ValueNode>("", -1);
                            // FIXME this assumes the input to a function is a signal
                            streams.push_back(createSignalBridge(connectorName, block->getName(), defaultProperty,
                                                                 bridgeDomain, noneValue,
                                                                 declaration->getFilename(), declaration->getLine(),
                                                                 size, "signal")); // Add definition to stream
                            std::shared_ptr<BlockNode> connectorNameNode = std::make_shared<BlockNode>(connectorName, "", -1);
                            std::shared_ptr<StreamNode> newStream = std::make_shared<StreamNode>(value, connectorNameNode, left->getFilename().c_str(), left->getLine());
                            prop->replaceValue(connectorNameNode);
                            streams.push_back(newStream);
                        }
                    }
                } /*else if (value->getNodeType() == AST::Bundle) {
                    std::string connectorName = "_BridgeSig_" + std::to_string(m_connectorCounter++);
                    //                    newDeclarations.push_back(createSignalBridge(listMemberName, declaration, new ValueNode("", -1)));

                }*/
            }
        }
        if (previousDomainName != domainName
                && left != stream->getLeft() // domain change and not the first node in the stream
                ) {
            bool skipSlice = false;
            if (left->getNodeType() == AST::Function) {
                std::shared_ptr<FunctionNode> func = static_pointer_cast<FunctionNode>(left);
                std::shared_ptr<DeclarationNode> decl = CodeValidator::findDeclaration(QString::fromStdString(func->getName()), scopeStack, m_tree);
                if (decl->getObjectType() == "reaction"
                        || decl->getObjectType() == "loop") {
                    // TODO hack to avoid slice. The right thing should be to have reactions and loops take the right domain...
                    skipSlice = true;
                }
            }
            if (!skipSlice) {
                terminateStackWithBridge(left, streams, stack, scopeStack);
            }
        } else {
//            if (left->getNodeType() == AST::Block) {
//                std::shared_ptr<BlockNode> block = static_pointer_cast<BlockNode>(left);
//                for(auto alias: m_bridgeAliases) {
//                    if (alias[1] == block->getName()) {
//                        // FIXME check domains match!
//                        left = std::make_shared<BlockNode>(alias[0], nullptr, block->getFilename().c_str(), block->getLine());
//                    }
//                }
//            } else if (left->getNodeType() == AST::Bundle) {
//                // FIXME implement for bundles
//                std::shared_ptr<BlockNode> block = static_pointer_cast<BlockNode>(left);
//                for(auto alias: m_bridgeAliases) {
//                    if (alias[1] == block->getName()) {
//                        // FIXME check domains match!
//                        left = std::make_shared<BlockNode>(alias[0], nullptr, block->getFilename().c_str(), block->getLine());
//                    }
//                }
//            }
            stack << left;
        }
        if(right) { // Is this check needed?
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
                        if (decl->getObjectType() == "reaction"
                                || decl->getObjectType() == "loop") {
                            // TODO hack to avoid slice. The right thing should be to have reactions and loops take the right domain...
                            skipSlice = true;
                        }
                    }
                    if (!skipSlice) {
                        lastNode = stack.back();
                        stack.pop_back();
                        terminateStackWithBridge(lastNode, streams, stack, scopeStack);
                    }
                }

                if (stack.size() != 0) {
                    streams << makeStreamFromStack(stack);
                }
                left = right = nullptr; // Mark last pass;
            }
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
            module->replacePropertyValue("blocks", std::make_shared<ListNode>(nullptr, "", -1));
            blocksNode = module->getPropertyValue("blocks");
        }

//        if (!streamsNode) {
//           module->setPropertyValue("streams", std::make_shared<ListNode>(nullptr, "", -1));
//           streamsNode = module->getPropertyValue("streams");
//        }

        std::shared_ptr<ListNode> newStreamsList = std::make_shared<ListNode>(nullptr, "", -1);
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
        QString memberName = CodeValidator::streamMemberName(exprLeft, scopeStack, m_tree);
        std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(memberName, scopeStack, m_tree);
        if (declaration) {
            if ( CodeValidator::getDomainNodeString(declaration->getDomain()) !=
                 CodeValidator::getDomainNodeString(outDomain)) {
                std::string connectorName = "_BridgeSig_" + std::to_string(m_connectorCounter++);
                string type = declaration->getObjectType();
                if (type == "switch" || type == "signal" || type == "trigger") { // This keeps constants away
                    streams.push_back(createSignalBridge(connectorName, memberName.toStdString(),
                                                         declaration->getPropertyValue("default"),
                                                         declaration->getDomain(), outDomain,
                                                         declaration->getFilename(), declaration->getLine(), 1, type)); // Add definition to stream
                    std::shared_ptr<BlockNode> connectorNameNode = std::make_shared<BlockNode>(connectorName, "", -1);
                    std::shared_ptr<StreamNode> newStream = std::make_shared<StreamNode>(exprLeft, connectorNameNode, exprLeft->getFilename().c_str(), exprLeft->getLine());
                    expr->replaceLeft(std::make_shared<BlockNode>(connectorName, "", -1));
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
                if ( CodeValidator::getDomainNodeString(declaration->getDomain()) !=
                     CodeValidator::getDomainNodeString(outDomain)) {
                    std::string connectorName = "_BridgeSig_" + std::to_string(m_connectorCounter++);
                    string type = declaration->getObjectType();
                    streams.push_back(createSignalBridge(connectorName, exprName->getName(),
                                                         declaration->getPropertyValue("default"),
                                                         declaration->getDomain(), outDomain,
                                                         declaration->getFilename(), declaration->getLine(),1 , type)); // Add definition to stream
                    std::shared_ptr<BlockNode> connectorNameNode = std::make_shared<BlockNode>(connectorName, "", -1);
                    std::shared_ptr<StreamNode> newStream = std::make_shared<StreamNode>(exprRight, connectorNameNode, exprRight->getFilename().c_str(), exprRight->getLine());
                    expr->replaceRight(std::make_shared<BlockNode>(connectorName, "", -1));
                    streams.push_back(newStream);
                }
            }
        } else if (exprRight->getNodeType() == AST::Bundle) {

            // FIXME need to implement for bundles
        }
    }
    return streams;
}

string CodeResolver::getContextDomainName(QVector<ASTNode> &scopeStack)
{
    string name;
    auto contextDomainNode = CodeValidator::findDeclaration("_ContextDomain", scopeStack, m_tree);

    if (contextDomainNode) {
        if (contextDomainNode->getObjectType() == "constant") {
            ASTNode node = contextDomainNode->getPropertyValue("value");
            if (node && node->getNodeType() == AST::String) {
                name = std::static_pointer_cast<ValueNode>(node)->getStringValue();
            }
        }
    }
    return name;
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
                        } else {
                            qDebug() << "Unexpected type for rate in domain declaration: " << QString::fromStdString(domainDeclaration->getName());
                        }
                    }
                }
            }
        }
    }
}

void CodeResolver::populateContextDomains(vector<ASTNode> nodes)
{
    for (auto node: nodes) {
        if (node->getNodeType() == AST::Declaration) {
            std::shared_ptr<DeclarationNode> decl = std::static_pointer_cast<DeclarationNode>(node);
            if (decl->getObjectType() == "module" || decl->getObjectType() == "reaction" || decl->getObjectType() == "loop") {
                auto blocks = decl->getPropertyValue("blocks");
                bool hasContextDomain = false;
                for (auto block: blocks->getChildren()) {
                    if (block->getNodeType() == AST::Declaration) {
                        auto internalDecl = std::static_pointer_cast<DeclarationNode>(block);
                        if (internalDecl->getName() == "_ContextDomain") {
                            hasContextDomain = true;
                            break;
                        }
                    }
                }
                if (!hasContextDomain) {
                    // This is currently hard coded to try output first, then input. This order should somehow be specified within Stride itself
                    auto outputPortBlock = CodeValidator::getMainOutputPortBlock(decl);
                    bool contextDomainSet = false;
                    if (outputPortBlock) {
                        QVector<ASTNode> scopeStack;
                        if (blocks) {
                            scopeStack = QVector<ASTNode>::fromStdVector(blocks->getChildren());
                            string domainName = CodeValidator::getNodeDomainName(outputPortBlock->getPropertyValue("block"), scopeStack, m_tree);
                            if (domainName.size() > 0) {
                                ASTNode propertiesList = std::make_shared<ListNode>(
                                            std::make_shared<PropertyNode>("value",
                                                                           std::make_shared<ValueNode>(domainName, "", -1),
                                                                           "", -1), "", -1);
                                blocks->addChild(std::make_shared<DeclarationNode>("_ContextDomain", "constant", propertiesList, "", -1));
                                propertiesList = std::make_shared<ListNode>(
                                            std::make_shared<PropertyNode>("value",
                                                                           std::make_shared<ValueNode>(0, "", -1),
                                                                           "", -1), "", -1);
                                blocks->addChild(std::make_shared<DeclarationNode>("_ContextRate", "constant", propertiesList, "", -1));
                                contextDomainSet = true;
                            }
                        }
                    }
                    auto inputPortBlock = CodeValidator::getMainInputPortBlock(decl);
                    if (inputPortBlock && !contextDomainSet) {
                        QVector<ASTNode> scopeStack;
                        if (blocks) {
                            scopeStack = QVector<ASTNode>::fromStdVector(blocks->getChildren());
                            string domainName = CodeValidator::getNodeDomainName(inputPortBlock->getPropertyValue("block"), scopeStack, m_tree);
                            if (domainName.size() > 0) {
                                ASTNode propertiesList = std::make_shared<ListNode>(
                                            std::make_shared<PropertyNode>("value",
                                                                           std::make_shared<ValueNode>(domainName, "", -1),
                                                                           "", -1), "", -1);
                                blocks->addChild(std::make_shared<DeclarationNode>("_ContextDomain", "constant", propertiesList, "", -1));
                                propertiesList = std::make_shared<ListNode>(
                                            std::make_shared<PropertyNode>("value",
                                                                           std::make_shared<ValueNode>(0, "", -1),
                                                                           "", -1), "", -1);
                                blocks->addChild(std::make_shared<DeclarationNode>("_ContextRate", "constant", propertiesList, "", -1));
                            }
                        }
                    }
                }
                populateContextDomains(decl->getPropertyValue("blocks")->getChildren()) ; // Do it recursively
            }
        }
    }
}

void CodeResolver::resolveDomainForStreamNode(ASTNode node, QVector<ASTNode > scopeStack)
{
    ASTNode domain = nullptr;
    if (node->getNodeType() == AST::Block
            || node->getNodeType() == AST::Bundle) {
        std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(CodeValidator::streamMemberName(node, scopeStack, m_tree), scopeStack, m_tree, node->getNamespaceList());
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
        while (domain->getNodeType() == AST::PortProperty) {
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
                    std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(CodeValidator::streamMemberName(node, scopeStack, m_tree), scopeStack, m_tree);
                    declaration->setDomainString(domainName);
                } else if (node->getNodeType() == AST::Function) {
                     static_cast<FunctionNode *>(node.get())->setDomainString(domainName);
                }
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

void CodeResolver::checkStreamConnections(std::shared_ptr<StreamNode> stream, QVector<ASTNode > scopeStack, bool start)
{
    ASTNode left = stream->getLeft();
    ASTNode right = stream->getRight();
    markConnectionForNode(left, scopeStack, start);

    if (right->getNodeType() == AST::Stream) {
        checkStreamConnections(static_pointer_cast<StreamNode>(right), scopeStack, false);
    } else {
        markConnectionForNode(right, scopeStack, false);
    }
}

void CodeResolver::markConnectionForNode(ASTNode node, QVector<ASTNode > scopeStack, bool start)
{
    if (node->getNodeType() == AST::Block) {
        QString name = QString::fromStdString(static_cast<BlockNode *>(node.get())->getName());
        std::shared_ptr<DeclarationNode> decl = CodeValidator::findDeclaration(name, scopeStack, m_tree);
        if (decl && (decl->getObjectType() == "signal"
                     || decl->getObjectType() == "switch"
                     || decl->getObjectType() == "trigger")) {
            if (start) { // not first element in stream, so it is being written to
                std::shared_ptr<PropertyNode> readsProperty;
                std::shared_ptr<ListNode> readsProperties;
                if (!decl->getPropertyValue("_reads")) {
                    readsProperties = std::make_shared<ListNode>(nullptr, node->getFilename().c_str(), node->getLine());
                    readsProperty = std::make_shared<PropertyNode>("_reads", readsProperties, node->getFilename().c_str(), node->getLine());
                    decl->addProperty(readsProperty);
                } else {
                    readsProperties = static_pointer_cast<ListNode>(decl->getPropertyValue("_reads"));
                    Q_ASSERT(readsProperties->getNodeType() == AST::List);
                }
                std::string domainName = CodeValidator::getDomainNodeString(decl->getDomain());
                readsProperties->addChild(std::make_shared<ValueNode>(domainName, node->getFilename().c_str(), node->getLine()));
            } else {
                std::shared_ptr<PropertyNode> writesProperty;
                std::shared_ptr<ListNode> writesProperties;
                if (!decl->getPropertyValue("_writes")) {
                    writesProperties = std::make_shared<ListNode>(nullptr, node->getFilename().c_str(), node->getLine());
                    writesProperty = std::make_shared<PropertyNode>("_writes", writesProperties, node->getFilename().c_str(), node->getLine());
                    decl->addProperty(writesProperty);
                } else {
                    writesProperties = static_pointer_cast<ListNode>(decl->getPropertyValue("_writes"));
                    Q_ASSERT(writesProperties->getNodeType() == AST::List);
                }
                std::string domainName = CodeValidator::getDomainNodeString(decl->getDomain());
                writesProperties->addChild(std::make_shared<ValueNode>(domainName.c_str(), node->getFilename().c_str(), node->getLine()));
            }
        }
    } else if (node->getNodeType() == AST::Bundle) {
        //TODO implement for bundles
    }
}

