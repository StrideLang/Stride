#ifndef VALUENODE_H
#define VALUENODE_H

#include <string>

#include "ast.h"

class ValueNode : public AST
{
public:
    ValueNode(const char *filename, int line);
    ValueNode(int value, const char * filename, int line);
    ValueNode(float value, const char * filename, int line);
    ValueNode(double value, const char * filename, int line);
    ValueNode(string value, const char * filename, int line);
    ValueNode(bool value, const char * filename, int line);
    ~ValueNode();

    int getIntValue() const;

    double getRealValue() const;

    double toReal() const;

    string getStringValue() const;

    bool getSwitchValue() const;

    AST *deepCopy();

private:
    int m_intValue;
    double m_floatValue;
    string m_stringValue;
    bool m_switch;
};

#endif // VALUENODE_H
