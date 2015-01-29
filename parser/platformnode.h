#ifndef PLATFORMNODE_H
#define PLATFORMNODE_H

#include <string>

#include "ast.h"

using namespace std;

class PlatformNode : public AST
{
public:
    PlatformNode();
    PlatformNode(int majorVersion, int minorVersion, string platformName);
    ~PlatformNode();

    int majorVersion() const;
    void setMajorVersion(int majorVersion);

    int minorVersion() const;
    void setMinorVersion(int minorVersion);

    string platformName() const;
    void setPlatformName(const string &platformName);

private:
    int m_majorVersion;
    int m_minorVersion;
    string m_platformName;
};

#endif // PLATFORMNODE_H
