QT += testlib
QT -= gui
CONFIG += debug

CONFIG += qt console warn_on depend_includepath testcase c++11
CONFIG -= app_bundle
INCLUDEPATH += ../

TEMPLATE = app

NEOADA_PATH = ../libneoada
include(../libneoada/libneoada.pro)
SOURCES +=  tst_parser.cpp \
            tst_AdaString.cpp \
            tst_AdaMath.cpp \
            tst_AdaTextEncoding.cpp \
            tst_AdaIoFile.cpp \
            tst_AdaDateTime.cpp
