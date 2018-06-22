TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp catcharp.cpp    sendarp.cpp  zenlog.cpp

LIBS+=-lpthread
