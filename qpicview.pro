#-------------------------------------------------
#
# Project created by QtCreator 2011-01-31T01:40:46
#
#-------------------------------------------------

QT       += core gui

TARGET = qpicview
TEMPLATE = app

DEFINES += _UNICODE UNICODE

SOURCES += main.cpp\
        mainwindow.cpp \
    MemoryMappedFile.cpp \
    IffIlbm.cpp \
    IffContainer.cpp \
    FileType.cpp

HEADERS  += mainwindow.h \
    MemoryMappedFile.h \
    IffIlbm.h \
    IffContainer.h \
    FileType.h

FORMS    += mainwindow.ui
