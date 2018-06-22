SOURCES += \
    testcppjson.cpp ../filedatabase.cpp ../tool.cpp

HEADERS += \
    testcppjson.h
CONFIG+=c++11
DEFINES+=IS_UNIX

INCLUDEPATH+=../cppjson/include
SOURCES += \
    ../cppjson/json_reader.cpp \
    ../cppjson/json_value.cpp \
    ../cppjson/json_writer.cpp
