/*
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
*/

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
    virtual bool build(AST *tree) = 0;
    virtual bool flash() = 0;
    virtual bool run(bool pressed = true) = 0;
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

