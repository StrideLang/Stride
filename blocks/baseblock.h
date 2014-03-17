#ifndef BASEBLOCK_H
#define BASEBLOCK_H

#include <QObject>
#include <QVector>
#include <QMutex>
#include <QPair>

class BaseBlock;

typedef enum {
    integer,
    float32,
    string
} VarTypes;

typedef struct {
    QString name;
    VarTypes type;
    double max; // TODO use right types. With union?
    double min;
    double default_val;
} ControlDetails;

class BlockConnector : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name)
public:
    BlockConnector(QString name);
    ~BlockConnector();
    void addConnection(BaseBlock *block, int index);
    void removeConnection(BaseBlock *block, int index);
    QVector<QPair<BaseBlock *, int> > getConnections();
private:
    QString m_name;
    QVector<QPair<BaseBlock *, int> > m_connections;
    QMutex *m_lock;
};


class BaseBlock : public QObject
{
    Q_OBJECT
public:
    explicit BaseBlock(QString id, QObject *parent = 0);
    ~BaseBlock();
    void setId(QString id);

    virtual QString getGlobalVariablesCode() = 0;
    virtual QString getUgenStructCode() = 0;
    virtual QString getInitUgensCode() = 0;
    virtual QString getAudioProcessingCode(QStringList outvars) = 0;

    virtual QVector<ControlDetails> getControlList() = 0;

    bool connectToInput(int inputIndex, BaseBlock* block, int outIndex);
    void registerInput(QString name);
    void registerOutput(QString name);
    QVector<BlockConnector *> getInputConnectors() {return m_inputConnectors;}
    QVector<BlockConnector *> getOutputConnectors() {return m_outputConnectors;}

signals:

public slots:
    
protected:
    QString m_id;
    QVector<BlockConnector *> m_inputConnectors;
    QVector<BlockConnector *> m_outputConnectors;
};

#endif // BASEBLOCK_H
