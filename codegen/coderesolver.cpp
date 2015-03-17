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
    resolveConstants();
    expandStreamBundles();
    expandTypeBundles();
}

void CodeResolver::resolveConstants()
{

}

void CodeResolver::expandStreamBundles()
{
    vector<AST *> nodes = m_tree->getChildren();
    foreach(AST* node, nodes) {
        if (node->getNodeType() == AST::Stream) {
            QVector<StreamNode *> streams = expandBundleStream(static_cast<StreamNode *>(node));
            // TODO Replace stream by new streams
        }
    }
}

void CodeResolver::expandTypeBundles()
{

}

QVector<StreamNode *> CodeResolver::expandBundleStream(StreamNode *stream)
{
    QVector<StreamNode *> streams;
    int size = largestBundleSize(stream);
    Q_ASSERT(size > 0);
    if (size == 1) {
        streams << stream;
        return streams;
    }
    for (int i = 0; i<size; i++) {

    }
    return streams;
}

int CodeResolver::largestBundleSize(StreamNode *stream)
{
    AST *left = stream->getLeft();
    int maxleft = getBundleSize(left);

    AST *right = stream->getRight();
    int maxright = 1;
    if (right->getNodeType() == AST::Stream) {
        maxright = largestBundleSize(static_cast<StreamNode *>(right));
    } else {
        maxright = getBundleSize(right);
    }
    return (maxleft > maxright? maxleft : maxright);
}

int CodeResolver::getBundleSize(AST *node)
{
    int size = 1;
    if (node->getNodeType() == AST::Bundle) {
        size = 1;
    } else if (node->getNodeType() == AST::BundleRange) {
        BundleNode *bundle = static_cast<BundleNode *>(node);
        AST *startIndex = bundle->startIndex();
        AST *endIndex = bundle->endIndex();
        QList<LangError> errors;
        int start = CodeValidator::evaluateConstInteger(startIndex, QVector<AST *>(), m_tree, errors);
        if (errors.size() > 0) {
            return -1;
        }
        int end = CodeValidator::evaluateConstInteger(endIndex, QVector<AST *>(), m_tree, errors);
        if (errors.size() > 0) {
            return -1;
        }
        return end - start;
    } else if (node->getNodeType() == AST::Bundle) {

    } else if (node->getNodeType() == AST::Expression) {

    }
    return size;
}



