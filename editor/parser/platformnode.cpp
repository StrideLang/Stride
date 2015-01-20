#include "platformnode.h"

PlatformNode::PlatformNode()
{

}

PlatformNode::PlatformNode(int majorVersion, int minorVersion, string platformName)
{
    m_majorVersion = majorVersion;
    m_minorVersion = minorVersion;
    m_platformName = platformName;
}

PlatformNode::~PlatformNode()
{

}

int PlatformNode::majorVersion() const
{
    return m_majorVersion;
}

void PlatformNode::setMajorVersion(int majorVersion)
{
    m_majorVersion = majorVersion;
}

int PlatformNode::minorVersion() const
{
    return m_minorVersion;
}

void PlatformNode::setMinorVersion(int minorVersion)
{
    m_minorVersion = minorVersion;
}

string PlatformNode::platformName() const
{
    return m_platformName;
}

void PlatformNode::setPlatformName(const string &platformName)
{
    m_platformName = platformName;
}
