Release:TEMPLATE = app
Debug:TEMPLATE = app
Debug:CONFIG += console


CONFIG += c++17 file_copies

Release:QMAKE_CXXFLAGS += /O2i

INCLUDEPATH += ../winpcap/Include

LIBS += -L"$$_PRO_FILE_PWD_/../winpcap/Lib" wpcap.lib
LIBS += Iphlpapi.lib Ws2_32.lib Advapi32.lib Shell32.lib

DESTDIR = ../

SOURCES += \
    app_console.cpp \
    app_service.cpp \
    application.cpp \
    client.cpp \
    controller.cpp \
    module_ipc.cpp \
    module_sniffer.cpp \
    overlapped.cpp \
    overlapped_server.cpp \
    packet_values.cpp \
    pipe.cpp \
    query_parser.cpp \
    srv_utils.cpp \
    tas.cpp \
    tcp_collector.cpp


HEADERS += \
    tcp_header.h \
    types.h \
    app_console.h \
    app_service.h \
    application.h \
    client.h \
    controller.h \
    module_ipc.h \
    module_sniffer.h \
    operation.h \
    overlapped_server.h \
    packet_values.h \
    pipe.h \
    query_parser.h \
    simple_timer.h \
    srv_utils.h \
    overlapped.h \
    tcp_collector.h

COPIES += scripts

scripts.files = $$files(scripts/*.bat)
scripts.path = $$DESTDIR
