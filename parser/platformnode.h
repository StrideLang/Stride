#ifndef PLATFORMNODE_H
#define PLATFORMNODE_H

#include <string>

#include "ast.h"

using namespace std;

class PlatformNode : public AST
{
public:
    PlatformNode(string platformName, double version, int line);

    ~PlatformNode();

    string platformName() const;
    void setPlatformName(const string &platformName);

    double version() const;
    void setVersion(double version);

    AST *deepCopy();

private:
    double m_version;
    string m_platformName;
};

#endif // PLATFORMNODE_H
