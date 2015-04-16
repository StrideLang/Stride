#ifndef VALUENODE_H
#define VALUENODE_H

#include <string>

#include "ast.h"

class ValueNode : public AST
{
public:
    ValueNode(int line);
    ValueNode(int value, int line);
    ValueNode(float value, int line);
    ValueNode(double value, int line);
    ValueNode(string value, int line);
    ValueNode(bool value, int line);
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
