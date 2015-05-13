#include <QDir>

#include "baseproject.h"

//extern int lua_build(const char* template_path, const char* destination_path,
//                     const char* ugen_graph, const char* configuration);

BaseProject::BaseProject(QString projectDir) :
    QObject(), m_projectDir(projectDir)
{
    m_runProcess = new QProcess(this);
    QDir::setCurrent("/home/andres/Documents/src/XMOS/Odo/Streamstack/platforms/Gamma/1.0/scripts");
}

BaseProject::~BaseProject()
{
    delete m_runProcess;
}

void BaseProject::build()
{


}
