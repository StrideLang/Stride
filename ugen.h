#ifndef UGEN_H
#define UGEN_H

#include <QString>
#include <QVector>
#include <QHash>
#include <QVariant>

class Ugen
{
public:
    Ugen();
    QString name;
    QString diskname;
    QStringList inputs;
    QStringList outputs;
    QVector<QHash<QString, QVariant> > parameters;
    QString cost;
    QString shortdoc;
    QString longdoc;
};

#endif // UGEN_H
