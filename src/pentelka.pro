TARGET = Pentelka
QMAKE_PROJECT_NAME = $${TARGET}
TEMPLATE = app

QT += quick core

CONFIG += c++17
CONFIG += lrelease warn_on

# Include translations
TRANSLATIONS += pentelka_en_US.ts

SOURCES += \
    main.cpp \
    canvas/item/canvasitem.cpp

HEADERS += \
    canvas/item/canvasitem.h

# Platform-specific settings
macx {
    QMAKE_INFO_PLIST = Info.plist
    QMAKE_MACOSX_BUNDLE_IDENTIFIER = "dev.hrubos.pentelka"
}

INCLUDEPATH += \
    canvas/item

# QML settings
RESOURCES += \
	qml.qrc

qml.files = qml
qml.path = .
QML_IMPORT_NAME = $${TARGET}
QML_IMPORT_MAJOR_VERSION = 1
INSTALLS += qml

# Deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
