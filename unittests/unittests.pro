QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
INCLUDEPATH += ../

TEMPLATE = app

NEOADA_PATH = ../libneoada
include(../libneoada/source.pro)
SOURCES +=  tst_parser.cpp
