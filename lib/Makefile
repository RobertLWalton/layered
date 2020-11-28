# Layered Lib Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@acm.org)
# Date:		Sat Nov 28 00:58:57 EST 2020
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.

# The following locates MIN.
#
MIN = ~/min

MAKE = make --no-print-directory

all:
	@${MAKE} -C ${MIN}/lib
	@${MAKE} liblayered.a

D = $$

# The following turns on MIN debugging.
#
GFLAGS = -DMIN_PROTECT=1

# O3 is needed to inline functions.
#
OPTIMIZE = -O3

WFLAGS = -Wall -Wextra \
         -Wno-unused-parameter \
         -Wno-char-subscripts

.SUFFIXES:


MIN_H =	${MIN}/lib/libmin.a

LEXEME_H =	${MIN_H} \
		../include/ll_lexeme_standard.h \
		../include/ll_lexeme.h

LEXEME_O =	ll_lexeme.o \
		ll_lexeme_ndl.o \
		ll_lexeme_standard.o

PARSER_O =	ll_parser.o \
		ll_parser_table.o \
		ll_parser_command.o \
		ll_parser_bracketed.o \
		ll_parser_oper.o \
		ll_parser_standard.o \
		ll_parser_standard_input.o \
		ll_parser_standard_brackets.o \
		ll_parser_standard_oper.o

O_FILES =       ${LEXEME_O} ${PARSER_O}

%.cc:	../src/%.lexcc Makefile
	sed -n \
		-e '/^cc/!s/.*/\/\//p' \
		-e '/^cc	/s///p' \
		-e '/^cc$$/s///p' \
		< ../src/$*.lexcc \
		> $*.cc

ll_lexeme.o:	\
	${MIN_H} \
	../include/ll_lexeme.h \
	../include/ll_lexeme_program_data.h \
	../src/ll_lexeme.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_lexeme.cc

ll_lexeme_test.o:	\
	${MIN_H} \
	../include/ll_lexeme.h \
	../include/ll_lexeme_program_data.h \
	../src/ll_lexeme_test.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_lexeme_test.cc

ll_lexeme_ndl.o:	\
		${MIN_H} \
		../src/ll_lexeme_ndl.cc \
		../include/ll_lexeme.h \
		../include/ll_lexeme_ndl.h \
		../include/ll_lexeme_ndl_data.h \
		Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_lexeme_ndl.cc

ll_lexeme_standard.cc:	\
		../src/ll_lexeme_standard.lexcc

ll_lexeme_standard.o:	\
		ll_lexeme_standard.cc \
		${LEXEME_H} \
		../include/ll_lexeme_ndl.h \
		Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ll_lexeme_standard.cc

ll_parser.o:	\
	${LEXEME_H} \
	../include/ll_parser.h \
	../include/ll_parser_table.h \
	../include/ll_parser_bracketed.h \
	../include/ll_parser_command.h \
	../include/ll_parser_standard.h \
	../src/ll_parser.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_parser.cc

ll_parser_standard_input.o:	\
	${LEXEME_H} \
	../include/ll_parser.h \
	../include/ll_parser_table.h \
	../include/ll_parser_standard.h \
	../src/ll_parser_standard_input.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_parser_standard_input.cc

ll_parser_table.o:	\
	${LEXEME_H} \
	../include/ll_parser_table.h \
	../src/ll_parser_table.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_parser_table.cc

ll_parser_standard.o:	\
	${LEXEME_H} \
	../include/ll_parser.h \
	../include/ll_parser_table.h \
	../include/ll_parser_standard.h \
	../src/ll_parser_standard.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_parser_standard.cc

ll_parser_bracketed.o:	\
	${LEXEME_H} \
	../include/ll_parser.h \
	../include/ll_parser_table.h \
	../include/ll_parser_bracketed.h \
	../include/ll_parser_command.h \
	../include/ll_parser_standard.h \
	../src/ll_parser_bracketed.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c \
	    ../src/ll_parser_bracketed.cc

ll_parser_standard_brackets.o:	\
	${LEXEME_H} \
	../include/ll_parser.h \
	../include/ll_parser_table.h \
	../include/ll_parser_bracketed.h \
	../include/ll_parser_standard.h \
	../src/ll_parser_standard_brackets.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_parser_standard_brackets.cc

ll_parser_oper.o:	\
	${LEXEME_H} \
	../include/ll_parser.h \
	../include/ll_parser_table.h \
	../include/ll_parser_oper.h \
	../src/ll_parser_oper.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_parser_oper.cc

ll_parser_standard_oper.o:	\
	${LEXEME_H} \
	../include/ll_parser.h \
	../include/ll_parser_table.h \
	../include/ll_parser_oper.h \
	../include/ll_parser_standard.h \
	../src/ll_parser_standard_oper.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_parser_standard_oper.cc

ll_parser_command.o:	\
	${LEXEME_H} \
	../include/ll_parser.h \
	../include/ll_parser_table.h \
	../include/ll_parser_standard.h \
	../include/ll_parser_command.h \
	../src/ll_parser_command.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_parser_command.cc

liblayered.a:       ${O_FILES}
	rm -f liblayered.a
	ar rs liblayered.a ${O_FILES}

clean:
	rm -f *.cc *.o *.a


