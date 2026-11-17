QT += core gui quick quickcontrols2 testlib dbus multimedia
CONFIG += c++17 testcase
TARGET = earworm_tests
TEMPLATE = app

INCLUDEPATH += ../src

HEADERS += ../src/backend.h ../src/theme.h ../src/translator.h ../src/book.h \
    ../src/library.h ../src/player.h ../src/sleeptimer.h ../src/bookmarkmodel.h \
    ../src/portalfilepicker.h

SOURCES += earworm_tests.cpp ../src/backend.cpp ../src/theme.cpp ../src/translator.cpp \
    ../src/book.cpp ../src/library.cpp ../src/player.cpp ../src/sleeptimer.cpp \
    ../src/bookmarkmodel.cpp ../src/portalfilepicker.cpp

RESOURCES += ../src/resources.qrc
