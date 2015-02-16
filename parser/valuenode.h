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
    ValueNode(string value, int line);
    ValueNode(bool value, int line);
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
