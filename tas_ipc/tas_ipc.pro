TEMPLATE = lib

CONFIG += c++17
DEFINES += TAS_IPC_BUILD_DLL

DESTDIR = ../

SOURCES += \
    dllmain.cpp \
    query.cpp \
    request.cpp \
    tas_ipc.cpp \

HEADERS += \
    tas_ipc.h \
    exceptions.h \
    request.h \
    query.h \
