#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>

//#include "ast.h"
#include "codevalidator.h"
#include "pythonproject.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("stridecc");
    QCoreApplication::setApplicationVersion("0.1-alpha");

    QCommandLineParser parser;
    parser.setApplicationDescription("Stride command line compiler");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("source", QCoreApplication::translate("main", "Source file to build."));
//    parser.addPositionalArgument("destination", QCoreApplication::translate("main", "Destination directory."));

    QCommandLineOption targetDirectoryOption(QStringList() << "p" << "platform-root",
                                             QCoreApplication::translate("main", "Path to Stride Platforms directory"),
                                             QCoreApplication::translate("main", "directory"));
    parser.addOption(targetDirectoryOption);
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    QString platformRootPath = parser.value(targetDirectoryOption);
    QString fileName = args.at(0);

//    qDebug() << args.at(0);
//    qDebug() << platformRootPath;

    AST *tree;
    tree = AST::parseFile(fileName.toLocal8Bit().constData());

    if (tree) {
        CodeValidator validator(platformRootPath, tree);
        validator.validate();
        //            QStringList m_types = validator.getPlatform()->getPlatformTypeNames();
        //            m_funcs = validator.getPlatform()->getFunctionNames();
        //            QList<AST *> objects = validator.getPlatform()->getBuiltinObjects();
        //            m_objectNames.clear();
        //            foreach (AST *platObject, objects) {
        //                if (platObject->getNodeType() == AST::Block) {
        //                    m_objectNames << QString::fromStdString(static_cast<BlockNode *>(platObject)->getName());
        //                }
        //            }
        //            m_errors = validator.getErrors();
        StridePlatform *platform = validator.getPlatform();

        QFileInfo info(fileName);
        QString dirName = info.absolutePath() + QDir::separator()
                + info.fileName() + "_Products";
        if (!QFile::exists(dirName)) {
            if (!QDir().mkpath(dirName)) {
                qDebug() << "Error creating project path";
                return -1;
            }
        }
        Builder * builder = platform->createBuilder(dirName);

        builder->build(tree);
        qDebug() << "Built in directory:" << dirName;

        //            if(m_lastValidTree) {
        //                m_lastValidTree->deleteChildren();
        //                delete m_lastValidTree;
        //            }
        //            m_lastValidTree = tree;
        tree->deleteChildren();
        delete tree;
    } else {
        qDebug() << "Syntax error";
    }
    return 0;
}
