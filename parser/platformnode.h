#ifndef PLATFORMNODE_H
#define PLATFORMNODE_H

#include <string>

#include "ast.h"

using namespace std;

typedef struct {
  string name;
  double version;
} HwPlatform;

class PlatformNode : public AST
{
public:
    PlatformNode(string platformName, double version, int line, string hwPlatform = string(), double hwVersion = -1);

    ~PlatformNode();

    string platformName() const;
    void setPlatformName(const string &platformName);

    double version() const;
    void setVersion(double version);

    string hwPlatform() const;
    void setHwPlatform(const string &hwPlatform);

    double hwVersion() const;
    void setHwVersion(double hwVersion);

    AST *deepCopy();

private:
    double m_version;
    string m_platformName;
    HwPlatform m_targetPlatform;
};

#endif // PLATFORMNODE_H
