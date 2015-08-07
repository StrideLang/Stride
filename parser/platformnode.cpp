#include <sstream>
#include <iostream>
#include <vector>
#include <cstdlib>

#include "platformnode.h"

PlatformNode::PlatformNode(string platformName, double version, int line, string hwPlatform, double hwVersion) :
    AST(AST::Platform, line)
{
    m_platformName = platformName;
    m_version = version;
    m_targetPlatform.name = hwPlatform;
    m_targetPlatform.version = hwVersion;
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
string PlatformNode::hwPlatform() const
{
    return m_targetPlatform.name;
}

void PlatformNode::setHwPlatform(const string &hwPlatform)
{
    m_targetPlatform.name = hwPlatform;
}
double PlatformNode::hwVersion() const
{
    return m_targetPlatform.version;
}

void PlatformNode::setHwVersion(double hwVersion)
{
    m_targetPlatform.version = hwVersion;
}

string PlatformNode::platformName() const
{
    return m_platformName;
}

void PlatformNode::setPlatformName(const string &platformName)
{
    m_platformName = platformName;
}

