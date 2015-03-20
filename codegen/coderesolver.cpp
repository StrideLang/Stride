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
    resolveConstants();
    expandStreamBundles();
    expandTypeBundles();
}

void CodeResolver::expandBuiltinObjects()
{
    QList<PlatformObject> objects = m_platform.getBuiltinObjects();
    foreach(PlatformObject object, objects) {
        BundleNode *bundle = new BundleNode(object.getName().toStdString(),
                                            new ValueNode(object.getSize(), -1), -1);
        BlockNode *newBlock = new BlockNode(bundle, object.getType().toStdString(), NULL, -1);
        m_tree->addChild(newBlock);
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
        }
    }
    m_tree->setChildren(nodes);
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
    } else if (node->getNodeType() == AST::BundleRange) {
        assert(0==1); //TODO implement here
    } else if (node->getNodeType() == AST::Stream) {
        StreamNode *streamNode = static_cast<StreamNode *>(node);
        AST * left = expandStreamMember(streamNode->getLeft(), i);
        AST * right = expandStreamMember(streamNode->getRight(), i);
        newNode = new StreamNode(left, right, streamNode->getLine());
    } else {
        qFatal("Node type not supported in CodeResolver::expandStreamMember");
    }
    return newNode;
}





