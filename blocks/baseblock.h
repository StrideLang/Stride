#ifndef BASEBLOCK_H
#define BASEBLOCK_H

#include <QObject>
#include <QVector>

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


class BaseBlock : public QObject
{
    Q_OBJECT
public:
    explicit BaseBlock(QObject *parent = 0);

    void setId(QString id) {m_id = id;}

    virtual QString getGlobalVariablesCode() = 0;
    virtual QString getInitGlobalsCode() = 0;
    virtual QVector<ControlDetails> getControlList() = 0;
    virtual QString getInitCode(QStringList varNames) = 0;
    virtual QString getProcessCode() = 0;

signals:

public slots:
    
private:
    QString m_id;
    QVector<BaseBlock *> inputs;
    QVector<BaseBlock *> outputs;
};

#endif // BASEBLOCK_H
