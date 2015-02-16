#include "codegen.h"


Codegen::Codegen(StreamPlatform &platform, AST *tree) :
    m_platform(platform), m_tree(tree)
{
}
