#ifndef OUTPUTBLOCK_H
#define OUTPUTBLOCK_H

#include <QStringList>

#include "baseblock.h"


class OutputBlock : public BaseBlock
{
    Q_OBJECT

    Q_PROPERTY(int numInChannels MEMBER m_numInChannels NOTIFY numInChannelsChanged)
public:
    explicit OutputBlock(QString id, QObject *parent = 0);
    virtual QString getGlobalVariablesCode() {return "";}
    virtual QString getUgenStructCode() {return "";}
    virtual QString getInitUgensCode() {return "";}
    virtual QString getAudioProcessingCode(QStringList outvars) {return "";}

    virtual QVector<ControlDetails> getControlList() {return QVector<ControlDetails>();}

public slots:
    void inChannelsChanged(int number);

private:
    int m_numInChannels;

signals:
    void numInChannelsChanged(int number);
};

#endif // OUTPUTBLOCK_H
