#include "outputblock.h"

OutputBlock::OutputBlock(QString id, QObject *parent) :
    BaseBlock(id, parent)
{
    connect(this, SIGNAL(numInChannelsChanged(int)), this, SLOT(inChannelsChanged(int)));
    registerInput("out");
}

void OutputBlock::inChannelsChanged(int number)
{
    m_inputConnectors.clear(); // TODO: reuse previous connections
    m_outputConnectors.resize(number);
}
