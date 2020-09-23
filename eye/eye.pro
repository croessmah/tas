TEMPLATE = app

CONFIG += c++17 console file_copies

INCLUDEPATH += ../tas_ipc
LIBS += -L"../" -ltas_ipc

DESTDIR = ../

SOURCES += \
    Controller.cpp \
    ControllersHoarder.cpp \
    main.cpp

HEADERS += \
    Controller.h \
    ControllersHoarder.h

COPIES += qml_files

qml_files.files = $$files(qml/*.qml)
qml_files.path = $$OUT_PWD
