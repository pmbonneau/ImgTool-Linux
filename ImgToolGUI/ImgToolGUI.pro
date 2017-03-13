TEMPLATE = app
TARGET = ImgToolGUI 

QT        += core gui 

HEADERS   += imgtoolgui.h
SOURCES   += main.cpp \
    imgtoolgui.cpp
FORMS     += imgtoolgui.ui    
LIBS += "../Source/Debug/libSource.a"
INCLUDEPATH += "../Source/."