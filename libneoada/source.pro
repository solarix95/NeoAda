
INCLUDEPATH += $$NEOADA_PATH
CONFIG += c++11
QT     += core 

# Input
HEADERS += \
    $$NEOADA_PATH/lexer.h \
    $$NEOADA_PATH/parser.h


SOURCES += \
    $$NEOADA_PATH/lexer.cc \
    $$NEOADA_PATH/parser.cc
