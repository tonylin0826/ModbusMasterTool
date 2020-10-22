QT       += core gui network serialbus serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG += app_bundle

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
    addmodbusregisterdialog.cpp \
    hexvalidator.cpp \
    main.cpp \
    mainwindow.cpp \
    modbusconnectdialog.cpp \
    modbusregistertablewidget.cpp \
    modbussubwindow.cpp \
    writemultiplecoilsdialog.cpp \
    writemutipleregistersdialog.cpp \
    writesinglecoildialog.cpp \
    writesingleregisterdialog.cpp

HEADERS += \
    addmodbusregisterdialog.hpp \
    hexvalidator.hpp \
    mainwindow.hpp \
    modbusconnectdialog.hpp \
    modbusregistertablewidget.hpp \
    modbussubwindow.hpp \
    writemultiplecoilsdialog.hpp \
    writemutipleregistersdialog.hpp \
    writesinglecoildialog.hpp \
    writesingleregisterdialog.hpp

FORMS += \
    addmodbusregisterdialog.ui \
    mainwindow.ui \
    modbusconnectdialog.ui \
    modbussubwindow.ui \
    writemultiplecoilsdialog.ui \
    writemutipleregistersdialog.ui \
    writesinglecoildialog.ui \
    writesingleregisterdialog.ui

TRANSLATIONS += \
    ModbusMasterTool_en_US.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    style.qss

RESOURCES += \
    resource.qrc
