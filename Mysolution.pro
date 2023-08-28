QT       += core gui
QT       += sql axcontainer
QT       += printsupport
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11


TARGET      =  Mysolution
TEMPLATE    =  app

RC_ICONS = resource\logo.ico

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AppStyle/DarkStyle.cpp \
    AppStyle/lightstyle.cpp \
    AppStyle/stylemanager.cpp \
    Camera/GetImageThread.cpp \
    Camera/MvCamera.cpp \
    FramelessWindow/framelesswindow.cpp \
    FramelessWindow/titlebar.cpp \
    MySQL/database.cpp \
    MySQL/databasewidget.cpp \
    MySQL/doublespinboxdelegate.cpp \
    MySQL/login.cpp \
    MySQL/managerwidget.cpp \
    MySQL/operatordatabasewidget.cpp \
    MySQL/qprintermanager.cpp \
    MySQL/registerwidget.cpp \
    displayLabel/mylabel.cpp \
    main.cpp \
    mainwindow.cpp \
    progressBar/customprogressbar.cpp \
    udpSocketThread/udpsocketthread.cpp \
    visionAlgorithm/imagepro.cpp \
    visionAlgorithm/lsdlines.cpp

HEADERS += \
    AppStyle/DarkStyle.h \
    AppStyle/lightstyle.h \
    AppStyle/stylemanager.h \
    Camera/GetImageThread.h \
    Camera/MvCamera.h \
    FramelessWindow/framelesswindow.h \
    FramelessWindow/titlebar.h \
    MySQL/database.h \
    MySQL/databasewidget.h \
    MySQL/doublespinboxdelegate.h \
    MySQL/login.h \
    MySQL/managerwidget.h \
    MySQL/operatordatabasewidget.h \
    MySQL/qprintermanager.h \
    MySQL/registerwidget.h \
    displayLabel/mylabel.h \
    mainwindow.h \
    progressBar/customprogressbar.h \
    udpSocketThread/udpsocketthread.h \
    visionAlgorithm/imagepro.h \
    visionAlgorithm/lsdlines.h

FORMS += \
    FramelessWindow/framelesswindow.ui \
    FramelessWindow/titlebar.ui \
    MySQL/databasewidget.ui \
    MySQL/login.ui \
    MySQL/managerwidget.ui \
    MySQL/operatordatabasewidget.ui \
    MySQL/registerwidget.ui \
    mainwindow.ui

#INCLUDEPATH += F:\opencv\opencv\opencv-build-32\install\include
#LIBS += F:\opencv\opencv\opencv-build-32\lib\libopencv_*.a
INCLUDEPATH += $$PWD/include
LIBS += $$PWD/lib/libopencv_*.a
LIBS += -L$$PWD/lib/ -llibopencv_gapi451 -llibopencv_video451 -llibopencv_stitching451 \
                   -llibopencv_objdetect451 -llibopencv_calib3d451 -llibopencv_features2d451 \
                   -llibopencv_dnn451 -llibopencv_photo451 -llibopencv_ml451 -llibopencv_flann451 \
                   -llibopencv_highgui451 -llibopencv_highgui451 -llibopencv_highgui451 \
                   -llibopencv_videoio451 -llibopencv_imgcodecs451 -llibopencv_imgproc451 \
                   -llibopencv_core451 -lopencv_videoio_ffmpeg451
INCLUDEPATH += $$PWD/Camera/include
DEPENDPATH += $$PWD/Camera/include
CONFIG(debug,debug|release)
{
    LIBS += -L$$PWD/Camera/include -lMvCameraControl
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += AppStyle/appstyle.qrc \
    FramelessWindow/framelesswindow.qrc \
    MySQL.qrc



