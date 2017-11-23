TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

win32:INCLUDEPATH += "C:/Program Files (x86)/IVI Foundation/VISA/WinNT/Include"
win32:LIBS += "c:/Program Files (x86)/IVI Foundation/VISA/WinNT/lib/msc/visa32.lib"

SOURCES += main.cpp \
    generator.cpp \
    analyzer.cpp

HEADERS += \
    generator.h \
    analyzer.h
