
INCLUDEPATH += $$NEOADA_PATH
CONFIG += c++11
QT     += core 

# Input
HEADERS += \
    $$NEOADA_PATH/lexer.h \
    $$NEOADA_PATH/parser.h \
    $$NEOADA_PATH/interpreter.h \
    $$NEOADA_PATH/value.h \
    $$NEOADA_PATH/state.h \
    $$NEOADA_PATH/type.h \
    $$NEOADA_PATH/utils.h \
    $$NEOADA_PATH/symboltable.h


SOURCES += \
    $$NEOADA_PATH/lexer.cc \
    $$NEOADA_PATH/parser.cc \
    $$NEOADA_PATH/interpreter.cc \
    $$NEOADA_PATH/value.cc \
    $$NEOADA_PATH/state.cc \
    $$NEOADA_PATH/type.cc \
    $$NEOADA_PATH/utils.cc \
    $$NEOADA_PATH/symboltable.cc
