#include "baseblock.h"
#include <QDebug>

BaseBlock::BaseBlock(QString id, QObject *parent) :
    QObject(parent)
{
    setId(id);
}

BaseBlock::~BaseBlock()
{
    //FIXME free m_outputCOnnectors and m_inputConnectors
}

void BaseBlock::setId(QString id)
{
    m_id = id;
    //TODO: validate id: e.g. should start with letter
}

bool BaseBlock::connectToInput(int inputIndex, BaseBlock *block, int outIndex)
{
    if (m_inputConnectors.size() > inputIndex
            && inputIndex >= 0) {
        m_inputConnectors[inputIndex]->addConnection(block, outIndex);
        return true;
    } else {
        qDebug() << "BaseBlock::connectToInput inputIndex invalid: " << inputIndex;
        return false;
    }
}

void BaseBlock::registerInput(QString name)
{
    BlockConnector *conn = new BlockConnector(name);
    m_inputConnectors.append(conn);
}

void BaseBlock::registerOutput(QString name)
{
    BlockConnector *conn = new BlockConnector(name);
    m_outputConnectors.append(conn);
}



BlockConnector::BlockConnector(QString name)
{
    m_lock = new QMutex();
    m_name = name;
}

BlockConnector::~BlockConnector()
{
    delete m_lock;
}

void BlockConnector::addConnection(BaseBlock *block, int index)
{
    QMutexLocker locker(m_lock);
    for (int i = 0; i < m_connections.size(); i++) {
        QPair<BaseBlock *,int> connection =  m_connections[i];
        if (connection.first == block && connection.second == index) {
            qDebug() << "BlockConnector::addConnection: Already connected.";
            return;
        }
    }
    m_connections.append(QPair<BaseBlock *, int>(block, index));
}

void BlockConnector::removeConnection(BaseBlock *block, int index)
{
    QMutexLocker locker(m_lock);
    for (int i = 0; i < m_connections.size(); i++) {
        QPair<BaseBlock *,int> connection =  m_connections[i];
        if (connection.first == block && connection.second == index) {
            m_connections.remove(i);
        }
    }
}

QVector<QPair<BaseBlock *, int> > BlockConnector::getConnections()
{
   QVector<QPair<BaseBlock *, int> >  connections;
   QMutexLocker locker(m_lock);
   connections = m_connections;
   return connections;
}
