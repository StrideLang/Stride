QT       += testlib

QT       -= gui

TARGET = tst_parsertest
CONFIG   += console
CONFIG   -= app_bundle

INCLUDEPATH += ../parser

TEMPLATE = app

SOURCES += tst_parsertest.cpp \
    buildtester.cpp
DEFINES += BUILDPATH=\\\"$$OUT_PWD/\\\"
CONFIG += c++11

include("../config.pri")

unix {
    !macx {
        LIBS += -lfl

        code_coverage {
            QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
            QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0
            LIBS += \
                -lgcov
        }

    }
}


folder_01.source = data/
folder_01.target = data/
DEPLOYMENTFOLDERS += folder_01

DISTFILES += \
    data/introFeedback.stride \
    data/introGenerator.stride \
    data/introBlock.stride \
    data/introProcessor.stride \
    data/introConverter.stride \
    data/introRemote.stride \
    data/platfromBasic.stride \
    data/defaultProperties.stride \
    data/shorthand.stride \
    data/introComplex.stride \
    data/introControlFlow.stride \
    data/introEnvelope.stride \
    data/introFIR.stride \
    data/introGranular.stride \
    data/introRandom.stride \
    data/introSequencer.stride \
    data/introStreamRate.stride \
    data/introVocoder.stride \
    data/introModulation.stride \
    data/01_header.stride \
    data/02_basic_blocks.stride \
    data/03_basic_bundle.stride \
    data/04_basic_stream.stride \
    data/05_basic_functions.stride \
    data/06_basic_noneswitch.stride \
    data/07_bundle_indeces.stride \
    data/09_lists.stride \
    data/10_expressions.stride \
    data/11_modules.stride \
    data/12_modules_domains.stride \
    data/13_connection_count.stride \
    data/14_members.stride \
    data/15_connection_errors.stride \
    data/16_buffer.stride \
    data/17_loop.stride \
    data/P01_platform_objects.stride \
    data/P02_check_duplicates.stride \
    data/P03_bundle_resolution.stride \
    data/P04_import.stride \
    data/P05_import_fail.stride \
    data/P06_domains.stride \
    data/P07_type_validation.stride \
    data/P08_scope.stride \
    data/P09_port_property.stride \
    data/P10_platform_validity.stride \
    data/P11_port_name_validation.stride \
    data/E01_constant_res.stride \
    data/E02_stream_expansions.stride \
    data/E03_multichn_streams.stride \
    data/E04_rates.stride \
    data/E05_library_objects.stride \
    data/E06_context_domain.stride \
    data/L01_library_types_validation.stride \
    data/E07_namespaces.stride

# Link to codegen library

win32-msvc2015:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../codegen/release/ -lcodegen
else:win32-msvc2015:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../codegen/debug/ -lcodegen
else:unix: LIBS += -L$$OUT_PWD/../codegen/ -lcodegen

INCLUDEPATH += $$PWD/../codegen
DEPENDPATH += $$PWD/../codegen

win32-msvc2015:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../codegen/release/codegen.lib
else:win32-msvc2015:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../codegen/debug/codegen.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../codegen/libcodegen.a

# Link to parser library
win32-msvc2015:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../parser/release/ -lStrideParser
else:win32-msvc2015:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../parser/debug/ -lStrideParser
else:unix: LIBS += -L$$OUT_PWD/../parser/ -lStrideParser

INCLUDEPATH += $$PWD/../parser
DEPENDPATH += $$PWD/../parser

win32-msvc2015:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../parser/release/StrideParser.lib
else:win32-msvc2015:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../parser/debug/StrideParser.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../parser/libStrideParser.a

HEADERS += \
    buildtester.hpp

