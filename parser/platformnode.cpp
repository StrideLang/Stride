#include <sstream>
#include <iostream>
#include <vector>
#include <cstdlib>

#include "platformnode.h"

PlatformNode::PlatformNode(string platformName, float version, int line) :
    AST(AST::Platform, line)
{
    m_platformName = platformName;
    m_version = version;
}

PlatformNode::~PlatformNode()
{

}

float PlatformNode::version() const
{
    return m_version;
}

void PlatformNode::setVersion(float version)
{
    m_version = version;
}

string PlatformNode::platformName() const
{
    return m_platformName;
}

void PlatformNode::setPlatformName(const string &platformName)
{
    m_platformName = platformName;
}

