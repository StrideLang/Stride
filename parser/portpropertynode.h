#ifndef PORTPROPERTYNODE_H
#define PORTPROPERTYNODE_H

#include <string>

#include "ast.h"

class PortPropertyNode : public AST
{
public:
    PortPropertyNode(string name, string port, const char *filename, int line);
    ~PortPropertyNode();

    string getName() const {return m_name;}
    string getPortName() const {return m_port;}

    AST *deepCopy();

private:
    string m_name;
    string m_port;
};


#endif // PORTPROPERTYNODE_H
