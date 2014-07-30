#include "baseproject.h"

BaseProject::BaseProject(QString dir) :
    QObject(), m_projectDir(dir), m_ugens("xmos")
{

}

BaseProject::~BaseProject()
{

}
