
INCLUDEPATH += $$NEOADA_PATH
CONFIG += c++11
QT     += core 

# Input
HEADERS += \
    $$NEOADA_PATH/private/type.h \
    $$NEOADA_PATH/private/utils.h \
    $$NEOADA_PATH/private/symboltable.h \
    $$NEOADA_PATH/private/functiontable.h \
    $$NEOADA_PATH/private/shareddata.h \
    $$NEOADA_PATH/private/sharedstring.h \
    $$NEOADA_PATH/private/sharedlist.h \
    $$NEOADA_PATH/private/numericparser.h \
    $$NEOADA_PATH/lexer.h \
    $$NEOADA_PATH/parser.h \
    $$NEOADA_PATH/interpreter.h \
    $$NEOADA_PATH/variant.h \
    $$NEOADA_PATH/state.h \
    $$NEOADA_PATH/neoadaapi.h \
    $$NEOADA_PATH/exception.h \
    $$NEOADA_PATH/runtime.h \
    $$NEOADA_PATH/addons/AdaList.h

SOURCES += \
    $$NEOADA_PATH/private/type.cc \
    $$NEOADA_PATH/private/utils.cc \
    $$NEOADA_PATH/private/symboltable.cc \
    $$NEOADA_PATH/private/functiontable.cc \
    $$NEOADA_PATH/private/shareddata.cc \
    $$NEOADA_PATH/private/sharedstring.cc \
    $$NEOADA_PATH/private/sharedlist.cc \
    $$NEOADA_PATH/private/numericparser.cc \
    $$NEOADA_PATH/lexer.cc \
    $$NEOADA_PATH/parser.cc \
    $$NEOADA_PATH/interpreter.cc \
    $$NEOADA_PATH/variant.cc \
    $$NEOADA_PATH/state.cc \
    $$NEOADA_PATH/neoadaapi.cc \
    $$NEOADA_PATH/exception.cc \
    $$NEOADA_PATH/runtime.cc \
    $$NEOADA_PATH/addons/AdaList.cc

DISTFILES += \
    $$NEOADA_PATH/ebnf.txt
