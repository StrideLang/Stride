#ifndef PORTTYPES_H
#define PORTTYPES_H

typedef enum {
    Signal,
    ConstReal,
    ConstInt,
    ConstBoolean,
    ConstString,
    None,
    Invalid
} PortType;

#endif // PORTTYPES_H
