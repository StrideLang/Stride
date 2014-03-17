#include "oscblock.h"

OscBlock::OscBlock(QString id, QObject *parent) :
    BaseBlock(id, parent)
{
    m_tabSize = 2048; // TODO implement setting table size
    registerOutput("audio out");
}

QString OscBlock::getGlobalVariablesCode()
{
    QString code;
    QString tablelenmacro = QString("%1_TABLE_LEN").arg(m_id);
    QString tablename = QString("%1_table").arg(m_id);
    code += QString("#define %1 %2\n").arg(tablelenmacro).arg(m_tabSize);
    code += QString("S32_T %1[%2];\n").arg(tablename).arg(tablelenmacro);
    return code;
}

QString OscBlock::getUgenStructCode()
{
    QString ugenStructs = "typedef struct {\nS32_T phs;\nS32_T incr;\n} OSCDATA;\n";
    return ugenStructs;
}

QString OscBlock::getInitUgensCode()
{
    QString tablelenmacro = QString("%1_TABLE_LEN").arg(m_id);
    QString tablename = QString("%2_table").arg(m_id);
    QString code = QString("for (int i = 0; i < %1; i++) {\n"
                           "%2[i] = (S32_T) (i * 65530 / (float)%1);\n"
                           "}\n").arg(tablelenmacro, tablename);

    QString varName = m_id + "_oscdata";
    code += QString("OSCDATA %1;\n").arg(varName); // TODO there should be someway of registering struct names to avoid clashes
    code += QString("%1.phs = 0;\n%2.incr = 1;\n").arg(varName).arg(varName);
    return code;
}

QString OscBlock::getAudioProcessingCode(QStringList outvars)
{
    // TODO should processing be separated into processing and ticking (where ticking updates
    // values for next pass) so that the output can be copied as quickly as possible to the DACs?

    QString tablelenmacro = QString("%1_TABLE_LEN").arg(m_id);
    QString tablename = QString("%1_table").arg(m_id);
    Q_ASSERT(outvars.size() == 1);
    QString code;
    QString varName = m_id + "_oscdata";
    code += QString("%1 = %2[%3.phs];\n").arg(outvars[0],tablename, varName);
    code += QString("        %1.phs += %1.incr;\n"
            "while (%1.phs >= %2) {\n"
            "    %1.phs -= %2;\n"
            "}\n").arg(varName).arg(tablelenmacro);
    return code;
}

QVector<ControlDetails> OscBlock::getControlList()
{

}

