#-------------------------------------------------
#
# Project created by QtCreator 2014-09-08T09:16:12
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VPNht
TEMPLATE = app

INCLUDE_DIR = . # current dir
INCLUDEPATH += $${INCLUDE_DIR}/tools/jsoncpp/include

win32 {
    LIBS += Ws2_32.lib Advapi32.lib Iphlpapi.lib

    DEFINES += NOMINMAX #qt5 error in datetime

    SOURCES +=  \
                Windows/OpenVPNConnectorQt.cpp

    HEADERS += Windows/OpenVPNConnectorQt.h
    RC_FILE += VPNht.rc
}

macx {
    LIBS += -framework CoreFoundation
    LIBS += -framework CoreServices
    LIBS += -framework Security
    LIBS += -framework SystemConfiguration
    LIBS += -framework AppKit
    LIBS += -framework ServiceManagement

    QMAKE_OBJECTIVE_CFLAGS += -fobjc-arc

    HEADERS += Mac/MacApplication.h \
        Mac/OpenVPNConnector.h \
        Mac/OpenVPNConnectorQt.h

    OBJECTIVE_SOURCES += Mac/MacApplication.mm \
                         Mac/OpenVPNConnector.mm \
                         Mac/OpenVPNConnectorQt.mm

    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7

    ICON = Mac/VPNht.icns
    QMAKE_INFO_PLIST = Mac/Info.plist

    copydata.commands = $(COPY_DIR) $$PWD/Mac/Resources $$OUT_PWD/VPNht.app/Contents
    copydata2.commands = $(COPY_DIR) $$PWD/Mac/Library $$OUT_PWD/VPNht.app/Contents
    first.depends = $(first) copydata copydata2
    export(first.depends)
    export(copydata.commands)
    export(copydata2.commands)
    QMAKE_EXTRA_TARGETS += first copydata copydata2
}

SOURCES += connectwindow.cpp \
    getservers.cpp \
    log.cpp \
    loginwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    settingswindow.cpp \
    showlog.cpp \
    downloadfile.cpp \
    proxysettingswindow.cpp \
    serverlistwindow.cpp \
    utils.cpp \
    QtSingleApplication/qtlocalpeer.cpp \
    QtSingleApplication/qtsingleapplication.cpp \
    QtSingleApplication/qtsinglecoreapplication.cpp \
    tools/jsoncpp/src/lib_json/json_value.cpp \ 
    tools/jsoncpp/src/lib_json/json_reader.cpp \ 
    tools/jsoncpp/src/lib_json/json_writer.cpp \
    pingwrapper.cpp \
    killswitch.cpp \
    getmyip.cpp \
    makeovpnfile.cpp \
    waitwindow.cpp \
    webrtcprotection.cpp \
    dnsleaks.cpp

HEADERS  += connectwindow.h \
    getservers.h \
    log.h \
    loginwindow.h \
    mainwindow.h \
    settings.h \
    settingswindow.h \
    proxysettingswindow.h \
    serverlistwindow.h \
    showlog.h \
    downloadfile.h \
    utils.h \
    subsriptionendwindow.h \
    QtSingleApplication/qtlocalpeer.h \
    QtSingleApplication/qtsingleapplication.h \
    QtSingleApplication/qtsinglecoreapplication.h \
    pingwrapper.h \
    killswitch.h \
    getmyip.h \
    makeovpnfile.h \
    waitwindow.h \
    webrtcprotection.h \
    dnsleaks.h


FORMS += connectwindow.ui \
    loginwindow.ui \
    mainwindow.ui \
    settingswindow.ui \
    proxysettingswindow.ui \
    serverlistwindow.ui \
    showlog.ui \
    waitwindow.ui

RESOURCES += \
    mainwindow.qrc

MOC_DIR = moc
OBJECTS_DIR = obj
