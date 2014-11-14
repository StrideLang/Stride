#include <QString>
#include <QtTest>

extern int parse(const char* fileName);

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
    parse("../../StreamStack/tests/data/lexar.txt");
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(ParserTest)

#include "tst_parsertest.moc"
