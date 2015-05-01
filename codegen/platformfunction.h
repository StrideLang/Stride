#ifndef PLATFORMFUNCTION_H
#define PLATFORMFUNCTION_H

#include "platformtype.h"

class PlatformFunction : public PlatformType
{
public:
    PlatformFunction(QString name, QList<Property> &properties, int numInputs, int numOutputs);
    ~PlatformFunction();

    int numInputs() const;
    int numOutputs() const;

private:
    int m_numInputs;
    int m_numOutputs;
};

#endif // PLATFORMFUNCTION_H
