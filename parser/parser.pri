
win32: BISON_BINARY = $$BISON_BIN_PATH\bison
macx: BISON_BINARY = $$BISON_BIN_PATH/bison
else: BISON_BINARY = bison

win32: FLEX_BINARY = $$FLEX_BIN_PATH\flex
macx: FLEX_BINARY = $$FLEX_BIN_PATH/flex
else: FLEX_BINARY = flex

bison.name = Bison ${QMAKE_FILE_IN}
bison.input = BISONSOURCES
bison.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.parser.cpp
bison.commands = $$BISON_BINARY  -d -v -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_IN}
#lex_compile.depend_command = g++ -E -M ${QMAKE_FILE_NAME} | sed "s,^.*: ,,"
bison.CONFIG += target_predeps
bison.variable_out = GENERATED_SOURCES
QMAKE_EXTRA_COMPILERS += bison
#bison_header.input = BISONSOURCES
#bison_header.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.parser2.hpp
#bison_header.commands = bison -d -o${QMAKE_FILE_OUT} ${QMAKE_FILE_IN}
#bison_header.CONFIG += target_predeps no_link
#silent:bison_header.commands = @echo Bison ${QMAKE_FILE_IN} && $$bison.commands
#QMAKE_EXTRA_COMPILERS += bison_header

flex.name = Flex ${QMAKE_FILE_IN}
flex.input = FLEXSOURCES
flex.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.lexer.cpp
flex.commands = $$FLEX_BINARY -o${QMAKE_FILE_OUT} ${QMAKE_FILE_IN}
flex.CONFIG += target_predeps
flex.variable_out = GENERATED_SOURCES
silent:flex.commands = @echo Lex ${QMAKE_FILE_IN} && $$flex.commands
QMAKE_EXTRA_COMPILERS += flex

