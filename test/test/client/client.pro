#-------------------------------------------------
#
# Project created by QtCreator 2018-03-20T22:17:01
#
#-------------------------------------------------

QT       += core gui network
include("../test.pri")
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = client
TEMPLATE = app

PREFIX=../../../
SOURCES += main.cpp\
        mainwindow.cpp client.cpp serverinfosearcher.cpp \
    player.cpp \
    playerwidget.cpp $$PREFIX/tool.cpp $$PREFIX/videosource.cpp $$PREFIX/pvd.cpp $$PREFIX/cppjson/json_reader.cpp \
    $$PREFIX/cppjson/json_value.cpp \
    $$PREFIX/cppjson/json_writer.cpp

HEADERS  += mainwindow.h client.h serverinfosearcher.h \
    player.h \
    playerwidget.h $$PREFIX/tool.h   $$PREFIX/videosource.h  $$PREFIX/pvd.h

FORMS    += mainwindow.ui
#include(../common/common.pri)


CONFIG+=c++11
DEFINES+=IS_UNIX
    LIBS+= -lopencv_core -lopencv_highgui \
       -lopencv_objdetect -lopencv_imgproc -lopencv_ml -lopencv_highgui  -lopencv_video  -lX11

