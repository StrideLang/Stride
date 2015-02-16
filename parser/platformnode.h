#ifndef PLATFORMNODE_H
#define PLATFORMNODE_H

#include <string>

#include "ast.h"

using namespace std;

class PlatformNode : public AST
{
public:
    PlatformNode(string platformName, float version, int line);

    ~PlatformNode();

    string platformName() const;
    void setPlatformName(const string &platformName);

    float version() const;
    void setVersion(float version);

private:
    float m_version;
    string m_platformName;
};

#endif // PLATFORMNODE_H
