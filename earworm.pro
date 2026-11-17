QT += core gui qml quick quickcontrols2 dbus multimedia

CONFIG += c++17 release
TARGET = earworm
TEMPLATE = app

HEADERS += \
    src/backend.h \
    src/theme.h \
    src/translator.h \
    src/library.h \
    src/book.h \
    src/player.h \
    src/sleeptimer.h \
    src/bookmarkmodel.h \
    src/portalfilepicker.h

SOURCES += \
    src/main.cpp \
    src/backend.cpp \
    src/theme.cpp \
    src/translator.cpp \
    src/library.cpp \
    src/book.cpp \
    src/player.cpp \
    src/sleeptimer.cpp \
    src/bookmarkmodel.cpp \
    src/portalfilepicker.cpp

RESOURCES += src/resources.qrc
