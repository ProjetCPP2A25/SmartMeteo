QT       += core gui widgets sql

TARGET   = SmartWeatherIntervention
TEMPLATE = app

CONFIG   += c++17

# Les sources contiennent des accents (UTF-8)
gcc:  QMAKE_CXXFLAGS += -finput-charset=UTF-8
msvc: QMAKE_CXXFLAGS += /utf-8

INCLUDEPATH += Headers

SOURCES += \
    Sources/connection.cpp \
    Sources/interventiondialog.cpp \
    Sources/interventionservice.cpp \
    Sources/interventionwidget.cpp \
    Sources/main.cpp \
    Sources/statschartwidget.cpp

HEADERS += \
    Headers/connection.h \
    Headers/intervention.h \
    Headers/interventiondialog.h \
    Headers/interventionservice.h \
    Headers/interventionwidget.h \
    Headers/statschartwidget.h \
    Headers/theme.h

FORMS += \
    Forms/interventiondialog.ui
