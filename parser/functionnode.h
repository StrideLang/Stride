#ifndef FUNCTIONNODE_H
#define FUNCTIONNODE_H

#include <string>

#include "ast.h"
#include "propertynode.h"

class FunctionNode : public AST
{
public:
    typedef enum {
        BuiltIn,
        UserDefined
    } FunctionType;

    FunctionNode(string name, AST *propertiesList, FunctionType type, int line);
    FunctionNode(string name, string namespace_, AST *propertiesList, FunctionType type, int line);
    ~FunctionNode();

    string getName() const { return m_name; }
    string getNamespace() const { return m_namespace; }
    vector<PropertyNode *> getProperties() const;
    int getParallelInstances() { return m_parallelInstances; }
    void setParallelInstances(int number) { m_parallelInstances = number; }

    AST *deepCopy();

private:
    string m_name;
    string m_namespace;
    int m_parallelInstances; // number of parallel instances according to stream neighbors
    FunctionType m_type;
    vector<PropertyNode *> m_properties;
};

#endif // FUNCTIONNODE_H
