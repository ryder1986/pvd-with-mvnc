#-------------------------------------------------
#
# Project created by QtCreator 2018-05-08T15:40:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = client1
TEMPLATE = app
PREFIX=../../../

SOURCES += main.cpp\
        mainwindow.cpp serverinfosearcher.cpp \
client.cpp \
    playerwidget.cpp $$PREFIX/tool.cpp $$PREFIX/videosource.cpp \
$$PREFIX/pvd.cpp $$PREFIX/cppjson/json_reader.cpp \
    $$PREFIX/cppjson/json_value.cpp \
    $$PREFIX/cppjson/json_writer.cpp \
    datamanager.cpp


HEADERS  += mainwindow.h serverinfosearcher.h  client.h\
    playerwidget.h $$PREFIX/tool.h   $$PREFIX/videosource.h  $$PREFIX/pvd.h \
    datamanager.h

FORMS    += mainwindow.ui



INCLUDEPATH+=$$PREFIX
INCLUDEPATH+=$$PREFIX/cppjson/include
CONFIG+=c++11
DEFINES+=IS_UNIX
    LIBS+= -lopencv_core -lopencv_highgui \
       -lopencv_objdetect -lopencv_imgproc -lopencv_ml -lopencv_highgui  -lopencv_video  -lX11

QT+=network
