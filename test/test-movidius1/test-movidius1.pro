QT += core
QT -= gui

TARGET = test-movidius1
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp  \
    conversion.cpp \
    movidiusprocessor.cpp

HEADERS += \
    conversion.h \
    movidiusprocessor.h

DISTFILES += \
    movidius.py

LIBS+= -lopencv_core -lopencv_highgui \
-lopencv_objdetect -lopencv_imgproc -lopencv_ml -lopencv_highgui\
 -lopencv_video -lpthread


INCLUDEPATH+=/usr/include/python2.7
LIBS+=-lpython2.7
CONFIG +=c++11
install_files.files+=movidius.py
install_files.path=$$OUT_PWD/

INSTALLS+=install_files

