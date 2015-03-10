#include <QDir>

#include "baseproject.h"

BaseProject::BaseProject(QString dir) :
    QObject(), m_projectDir(dir), m_ugens("xmos", this)
{
//    QDir::setCurrent(m_platform->getPlatformPath() + "/lua_scripts/");
    m_lua = lua_open();

    luaL_openlibs(m_lua); // Load standard libraries
}

BaseProject::~BaseProject()
{
    lua_close(m_lua);
}
