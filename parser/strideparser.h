#ifndef STREAMPARSER_H
#define STREAMPARSER_H

#include "ast.h"
#include "blocknode.h"
#include "bundlenode.h"
#include "expressionnode.h"
#include "functionnode.h"
#include "listnode.h"
#include "namenode.h"
#include "platformnode.h"
#include "propertynode.h"
#include "streamnode.h"
#include "valuenode.h"
#include "importnode.h"
#include "fornode.h"
#include "rangenode.h"


typedef enum {
    Signal,
    ConstReal,
    ConstInt,
    ConstBoolean,
    ConstString,
    None,
    Invalid
} PortType;


#endif // STREAMPARSER_H

