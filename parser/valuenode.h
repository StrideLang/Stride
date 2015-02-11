#ifndef VALUENODE_H
#define VALUENODE_H

#include <string>

#include "ast.h"

class ValueNode : public AST
{
public:
    ValueNode();
    ValueNode(int value);
    ValueNode(float value);
    ValueNode(string value);
    ValueNode(bool value);
    ~ValueNode();

    int getIntValue() const;

    float getFloatValue() const;

    string getStringValue() const;

    bool getSwitchValue() const;

private:
    int m_intValue;
    float m_floatValue;
    string m_stringValue;
    bool m_switch;
};

#endif // VALUENODE_H
