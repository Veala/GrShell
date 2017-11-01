TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

win32:INCLUDEPATH += "C:/Program Files (x86)/IVI Foundation/VISA/WinNT/Include"
win32:LIBS += "c:/Program Files (x86)/IVI Foundation/VISA/WinNT/lib/msc/visa32.lib"

#win32:INCLUDEPATH += "C:/Program Files/IVI Foundation/VISA/Win64/Include"
#win32:LIBS += "c:/Program Files/IVI Foundation/VISA/Win64/Lib_x64/msc/visa32.lib"

#win32:INCLUDEPATH += "c:/Program Files (x86)/IVI Foundation/VISA/WinNT/agvisa/include"
#win32:LIBS += "c:/Program Files (x86)/IVI Foundation/VISA/WinNT/agvisa/lib/msc/visa32.lib"
#win32:DLLDESTDIR+= "c:/Windows/system32"

#win32:LIBS += "C:/Windows/System32/visa32.dll"
SOURCES += main.cpp
