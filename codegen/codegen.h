#ifndef CODEGEN_H
#define CODEGEN_H

#include "streamparser.h"
#include "streamplatform.h"

class Codegen
{
public:
    Codegen(StreamPlatform &platform, AST * tree);

private:
    StreamPlatform m_platform;
    AST *m_tree;
};

#endif // CODEGEN_H
