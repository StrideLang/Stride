#include <QString>
#include <QtTest>

#include "ast.h"

extern AST *parse(const char* fileName);

class ParserTest : public QObject
{
    Q_OBJECT

public:
    ParserTest();

private Q_SLOTS:
    void testCase1();
};

ParserTest::ParserTest()
{
}

void ParserTest::testCase1()
{
    AST *tree;
    QString path = "../../StreamStack/tests/data/";
    QStringList files;
    files << "simple.stream" << "array.stream" << "list.stream"
          << "introBlock.stream"
          << "introConverter.stream" << "introFeedback.stream"
          << "introGenerator.stream";
    foreach (QString file, files) {
        tree = parse(QString(path + file).toStdString().c_str());
        QVERIFY2(tree != NULL, "Failure");
    }
}

QTEST_APPLESS_MAIN(ParserTest)

#include "tst_parsertest.moc"
