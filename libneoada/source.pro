
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
    $$NEOADA_PATH/symboltable.h \
    $$NEOADA_PATH/functiontable.h \
    $$NEOADA_PATH/shareddata.h \
    $$NEOADA_PATH/sharedstring.h \
    $$NEOADA_PATH/numericparser.h \
    $$NEOADA_PATH/neoadaapi.h \
    $$NEOADA_PATH/exception.h


SOURCES += \
    $$NEOADA_PATH/lexer.cc \
    $$NEOADA_PATH/parser.cc \
    $$NEOADA_PATH/interpreter.cc \
    $$NEOADA_PATH/value.cc \
    $$NEOADA_PATH/state.cc \
    $$NEOADA_PATH/type.cc \
    $$NEOADA_PATH/utils.cc \
    $$NEOADA_PATH/symboltable.cc \
    $$NEOADA_PATH/functiontable.cc \
    $$NEOADA_PATH/shareddata.cc \
    $$NEOADA_PATH/sharedstring.cc \
    $$NEOADA_PATH/numericparser.cc \
    $$NEOADA_PATH/neoadaapi.cc \
    $$NEOADA_PATH/exception.cc

DISTFILES += \
    $$NEOADA_PATH/ebnf.txt
