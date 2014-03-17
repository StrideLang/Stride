#ifndef SINBLOCK_H
#define SINBLOCK_H

#include "baseblock.h"
#include <QStringList>

class OscBlock : public BaseBlock
{
    Q_OBJECT
    
public:
    explicit OscBlock(QString id, QObject *parent);

    virtual QString getGlobalVariablesCode();
    virtual QString getUgenStructCode();
    virtual QString getInitUgensCode();
    virtual QString getAudioProcessingCode(QStringList outvars);

    virtual QVector<ControlDetails> getControlList();


signals:

public slots:

protected:
    int m_tabSize;
};

#endif // SINBLOCK_H
