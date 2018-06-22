QT += core
QT -= gui

TARGET = ipinfo
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ipinfo.cpp

HEADERS += \
    ipinfo.h

QT+=network
