#-------------------------------------------------
#
# Project created by QtCreator 2014-02-21T06:00:39
#
#-------------------------------------------------

QT       += core gui qml quick

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

lessThan(QT_MAJOR_VERSION, 5): message("Qt 5 required!")

TARGET = OdoEdit
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    projectwindow.cpp \
    baseproject.cpp \
    simpleproject.cpp

HEADERS  += mainwindow.h \
    projectwindow.h \
    baseproject.h \
    simpleproject.h

FORMS    += mainwindow.ui \
    projectwindow.ui

OTHER_FILES += \
    qml/Editor.qml \
    templates/simple/Makefile \
    templates/simple/src/app_conf.h \
    templates/simple/src/app_global.h \
    templates/simple/src/audio_io.h \
    templates/simple/src/audio_io.xc \
    templates/simple/src/codec.h \
    templates/simple/src/codec.xc \
    templates/simple/src/dsp_biquad.h \
    templates/simple/src/dsp_biquad.xc \
    templates/simple/src/i2s_master_conf.h \
    templates/simple/src/lfo.h \
    templates/simple/src/lfo.xc \
    templates/simple/src/main.h \
    templates/simple/src/main.xc \
    templates/simple/src/module_dsp_biquad_conf.h \
    templates/simple/src/xa_sk_audio_1v0.h

folder_01.source = qml
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

RESOURCES += \
    qmlfiles.qrc
