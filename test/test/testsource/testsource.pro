QT += core
QT -= gui

TARGET = testsource
CONFIG += console c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp
HEADERS +=../../../videosource.h ../../../tool.h
SOURCES +=../../../videosource.cpp ../../../tool.cpp
DEFINES+= IS_UNIX
LIBS+=  -lopencv_core -lopencv_highgui \
-lopencv_objdetect -lopencv_imgproc -lopencv_ml -lopencv_highgui \
 -lopencv_video
INCLUDEPATH+=../../../
