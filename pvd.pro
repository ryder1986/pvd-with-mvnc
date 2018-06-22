QT+=network
QT       -= gui
TARGET = server
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
CONFIG +=c++11
QMAKE_CXXFLAGS+=-w
unix{
DEFINES +=IS_UNIX
QMAKE_CXXFLAGS+=-w
}

SOURCES += main.cpp \
    videoprocessor.cpp \
    server.cpp \
    processeddatasender.cpp \
    clientsession.cpp \
    cameramanager.cpp \
    camera.cpp C4Common.cpp pvdprocessor.cpp tool.cpp videosource.cpp filedatabase.cpp pvd.cpp \
    locationservice.cpp \
    pvdhogprocessor.cpp \
    movidiusprocessor.cpp \
    conversion.cpp
HEADERS += \
    videoprocessor.h \
    server.h \
    processeddatasender.h \
    clientsession.h \
    cameramanager.h \
    camera.h C4Common.h  pvdprocessor.h tool.h pvd.h videosource.h filedatabase.h \
    locationservice.h \
    pvdhogprocessor.h \
    movidiusprocessor.h \
    conversion.h

INCLUDEPATH+=cppjson/include
SOURCES += \
    cppjson/json_reader.cpp \
    cppjson/json_value.cpp \
    cppjson/json_writer.cpp


INCLUDEPATH+=track
SOURCES +=track/Ctracker.cpp track/HungarianAlg.cpp  track/Kalman.cpp
HEADERS += track/defines.h track/HungarianAlg.h track/Kalman.h track/Ctracker.h



install_files.files+=res
install_files.path=$$OUT_PWD/
#LIBS+=-L$$CVPATH/$$CV_PREFIX/lib -lopencv_core -lopencv_highgui \
#-lopencv_objdetect -lopencv_imgproc -lopencv_ml -lopencv_highgui \
# -lopencv_video
INSTALLS +=install_files
#LIBS+=-L/root/source/opencv/build/__install/lib -lopencv_core -lopencv_highgui \
#-lopencv_objdetect -lopencv_imgproc -lopencv_ml -lopencv_highgui\
# -lopencv_video -lpthread   -lopencv_videoio# -L/root/source/opencv/build/__install/lib



#INCLUDEPATH+=/root/source/opencv/build/__install/include



LIBS+= -lopencv_core -lopencv_highgui \
-lopencv_objdetect -lopencv_imgproc -lopencv_ml -lopencv_highgui\
 -lopencv_video -lpthread


INCLUDEPATH+=/usr/include/python2.7
LIBS+=-lpython2.7
