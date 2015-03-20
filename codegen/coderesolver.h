#ifndef CODERESOLVER_H
#define CODERESOLVER_H

#include <QVector>

#include "streamplatform.h"
#include "ast.h"
#include "streamnode.h"
#include "bundlenode.h"

class CodeResolver
{
public:
    CodeResolver(StreamPlatform &platform, AST *tree);
    ~CodeResolver();

    void process();

private:
    void expandBuiltinObjects();
    void resolveConstants();
    void expandStreamBundles();
    void expandTypeBundles();

    QVector<StreamNode *> expandBundleStream(StreamNode *stream, int size = -1);

    AST *expandStreamMember(AST *node, int i);
    StreamPlatform m_platform;
    AST *m_tree;
};

#endif // CODERESOLVER_H
