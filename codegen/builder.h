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
#include <QMap>
#include <QVariant>

#include "ast.h"
#include "declarationnode.h"

#define STRIDE_PLUGIN_MAX_STR_LEN 32

class Builder;

typedef Builder* (*create_object_t)(QString projectDir, QString strideRoot, QString platformPath);
typedef void (*platform_name_t)(char *name);
typedef int (*platform_version_major_t)();
typedef int (*platform_version_minor_t)();

typedef struct {
    create_object_t create;
    platform_name_t get_name;
    platform_version_major_t get_version_major;
    platform_version_minor_t get_version_minor;
} PluginInterface;

class Builder : public QObject
{
    Q_OBJECT
public:
    Builder(QString projectDir, QString strideRoot, QString platformPath)
        : m_projectDir(projectDir), m_strideRoot(strideRoot),
          m_platformPath(platformPath) {}
    virtual ~Builder() {}

    void setConfiguration(QMap<QString, QVariant> config) { m_configuration = config; }

    std::map<std::string, QVariant> getConfiguration() {
        std::map<std::string, QVariant> configMap;
        for(auto configEntry: m_configuration.keys()) {
            configMap[configEntry.toStdString()] = m_configuration[configEntry];

        }
        return configMap;
    }
    QString getPlatformPath() {return m_platformPath;}
    QString getStdErr() const {return m_stdErr;}
    QString getStdOut() const {return m_stdOut;}
    void clearBuffers() { m_stdErr = ""; m_stdOut = "";}
    void registerYieldCallback(std::function<void()> cb) {m_yieldCallback = cb;}

    std::string getPlatformDirective(std::string directive);

    // Additional information from stride system
    std::vector<std::shared_ptr<DeclarationNode>> m_connectors; // Connections between domains.

public slots:
    virtual std::map<std::string, std::string> generateCode(ASTNode tree) = 0;
    virtual bool build(std::map<std::string, std::string> domainMap) = 0;
    virtual bool deploy() = 0;
    virtual bool run(bool pressed = true) = 0;
//    // TODO this would need to send encrypted strings or remove the code sections?
//    virtual QString requestTypesJson() {return "";}
//    virtual QString requestFunctionsJson() {return "";}
//    virtual QString requestObjectsJson() {return "";}
    virtual bool isValid() {return false;}


protected:
    QString m_projectDir;
    QString m_strideRoot;
    QString m_platformPath;
    QString m_platformName;
    QString m_stdOut;
    QString m_stdErr;
    QMap<QString, QVariant> m_configuration;
    std::function<void()> m_yieldCallback = [](){};



    QString substituteTokens(QString text)
    {
        QMap<QString, QString> tokenMap;
        tokenMap["%projectDir%"] = m_projectDir;
        tokenMap["%strideRoot%"] = m_strideRoot;
        tokenMap["%platformRoot%"] = m_platformPath;
        for (auto mapEntry: tokenMap.keys()) {
            while (text.indexOf(mapEntry) >= 0) {
                text.replace(mapEntry, tokenMap[mapEntry]);
            }
        }
        return text;
    }

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



