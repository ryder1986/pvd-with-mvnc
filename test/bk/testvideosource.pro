HEADERS += \
    ../videosource.h \
    ../tool.h

SOURCES += \
    ../videosource.cpp \
    testvideosource.cpp \
    ../tool.cpp
unix{
DEFINES +=IS_UNIX
QMAKE_CXXFLAGS+=-w
}
unix{
CVPATH="/root/repo-github/libs/third-party/opencv"
CV_PREFIX="/opencv-linux-x86"
ALGPATH="/root/repo-github/libs/alg"
}
unix{
    DEFINES += IS_UNIX
    QMAKE_CXXFLAGS+="-std=c++11"
    INCLUDEPATH +="$$CVPATH/$$CV_PREFIX/include"
    INCLUDEPATH +="$$ALGPATH/include"
    message(alg  in $$ALGPATH/include)
    LIBS+=-L$$CVPATH/$$CV_PREFIX/lib -lopencv_core -lopencv_highgui \
       -lopencv_objdetect -lopencv_imgproc -lopencv_ml   -lopencv_video  -lX11

    QMAKE_LIBS+=-lpthread
}
