QT += core gui widgets testlib

TARGET = CalculatorTests
TEMPLATE = app
CONFIG += c++11 console
CONFIG -= app_bundle

SOURCES += \
    test_calculator.cpp \
    ../calculator.cpp

HEADERS += \
    ../calculator.h

FORMS += \
    ../calculator.ui

INCLUDEPATH += ..
