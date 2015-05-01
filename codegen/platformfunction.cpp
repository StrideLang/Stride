#include "platformfunction.h"

PlatformFunction::PlatformFunction(QString name, QList<Property> &properties, int numInputs, int numOutputs) :
    PlatformType(name, properties), m_numInputs(numInputs), m_numOutputs(numOutputs)
{

}

PlatformFunction::~PlatformFunction()
{

}
int PlatformFunction::numInputs() const
{
    return m_numInputs;
}

int PlatformFunction::numOutputs() const
{
    return m_numOutputs;
}

