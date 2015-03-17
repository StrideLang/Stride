#ifndef PLATFORMFUNCTION_H
#define PLATFORMFUNCTION_H

#include "platformtype.h"

class PlatformFunction : public PlatformType
{
public:
    PlatformFunction(QString name, QList<Property> &properties);
    ~PlatformFunction();
};

#endif // PLATFORMFUNCTION_H
