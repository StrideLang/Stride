#include <sstream>
#include <iostream>
#include <vector>
#include <cstdlib>

#include "platformnode.h"

PlatformNode::PlatformNode(string platformName, double version, int line) :
    AST(AST::Platform, line)
{
    m_platformName = platformName;
    m_version = version;
}

PlatformNode::~PlatformNode()
{

}

double PlatformNode::version() const
{
    return m_version;
}

void PlatformNode::setVersion(double version)
{
    m_version = version;
}

AST *PlatformNode::deepCopy()
{
    return new PlatformNode(m_platformName, m_version, m_line);
}

string PlatformNode::platformName() const
{
    return m_platformName;
}

void PlatformNode::setPlatformName(const string &platformName)
{
    m_platformName = platformName;
}

