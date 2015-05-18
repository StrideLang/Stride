#ifndef BASEPROJECT_H
#define BASEPROJECT_H

#include <QObject>
#include <QString>
#include <QLibrary>

class AST;
class Builder;

typedef Builder* (*create_object_t)(QString platformPath, const char *projectDir, const char *xmosToolchainRoot);
typedef void (*platform_name_t)(char *name);
typedef double (*platform_version_t)();

typedef struct {
    create_object_t create;
    platform_name_t get_name;
    platform_version_t get_version;
} PluginInterface;

class Builder : public QObject
{
    Q_OBJECT
public:
    Builder(QString projectDir, QString platformPath)
        : m_projectDir(projectDir), m_platformPath(platformPath) {}
    virtual ~Builder() {}

public slots:
    virtual void build(AST *tree) = 0;
    virtual void flash() = 0;
    virtual void run(bool pressed = true) = 0;
    // TODO this would need to send encrypted strings or remove the code sections?
    virtual QString requestTypesJson() {return "";}
    virtual QString requestFunctionsJson() {return "";}
    virtual QString requestObjectsJson() {return "";}
    virtual bool isValid() {return false;}

protected:
    QString m_projectDir;
    QString m_platformPath;

private:

    PluginInterface m_interface;
    QLibrary *m_pluginLibrary;
    Builder *m_project;

signals:
    void outputText(QString text);
    void errorText(QString text);
    void programStopped();
};


#endif // BASEPROJECT_H

