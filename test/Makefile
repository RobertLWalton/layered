# Layered Language Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@acm.org)
# Date:		Mon May 26 08:32:57 PM EDT 2025
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.

D = $$

# The following locates the MIN and MEX source code.
#
MIN = ~/min
MEX = ~/mex

# The following turns on MIN debugging.
#
GFLAGS = -DMIN_PROTECT=1
WFLAGS = -Wall -Wextra \
         -Wno-unused-parameter \
	 -Wno-char-subscripts

.SUFFIXES:
.SUFFIXES: .out .test .diff

# O3 is needed to inline functions.
#
OPTIMIZE = -O3

DIFFOPT ?= -u

LEXEME_TESTS = ll_lexeme_basic_test \
               ll_lexeme_standard_basic_test \
               ll_lexeme_standard_special_test \
               ll_lexeme_standard_test \
               ll_lexeme_alternate_basic_test \
               ll_lexeme_alternate_test \
               ll_lexeme_name_string_test \
	       ll_lexeme_c++_test

PARSER_TESTS = ll_parser_input_test \
	       ll_parser_table_test \
	       ll_parser_command_test \
	       ll_parser_lexeme_map_standard_test \
	       ll_parser_bracketed_standard_test \
	       ll_parser_bracketed_command_test \
	       ll_parser_prefix_command_test \
	       ll_parser_oper_standard_test \
	       ll_parser_oper_command_test \
	       ll_parser_primary_command_test \
	       ll_parser_standard_test \
	       ll_parser_standard_errors_test \
	       ll_parser_standard_print_test

TESTS = ${LEXEME_TESTS} ${PARSER_TESTS}

MIN_H =	${MIN}/lib/libmin.a

MIN_O =	${MIN}/lib/libmin.a

MEX_H = ${MEX}/lib/libmex.a

MEX_O = ${MEX}/lib/libmex.a

LEXEME_H =	${MIN_H} \
		../include/ll_lexeme_standard.h \
		../include/ll_lexeme.h

LEXEME_O =	${MIN_O} \
		ll_lexeme.o \
		ll_lexeme_ndl.o \
		ll_lexeme_standard.o

LEXEME_ALTERNATE_H = \
		${MIN_H} \
		../include/ll_lexeme_alternate.h \
		../include/ll_lexeme.h

LEXEME_ALTERNATE_O = \
		${MIN_O} \
		ll_lexeme.o \
		ll_lexeme_ndl.o \
		ll_lexeme_alternate.o

PARSER_O =	${LEXEME_O} \
		ll_parser.o \
		ll_parser_table.o \
		ll_parser_command.o \
		ll_parser_bracketed.o \
		ll_parser_oper.o \
		ll_parser_primary.o \
		ll_parser_standard.o \
		ll_parser_standard_input.o \
		ll_parser_standard_brackets.o \
		ll_parser_standard_oper.o \
		ll_parser_standard_primary.o \
		${MEX_O}  # must be last

.SECONDARY:	${TESTS:=.out}

all:		diff \
		ll_lexeme_standard.lex \
		ll_lexeme_alternate.lex \
	        ll_lexeme_c++.lex

lexeme_diff:	${LEXEME_TESTS:=.diff}
parser_diff:	${PARSER_TESTS:=.diff}
diff:		${TESTS:=.diff}

out:		${TESTS:=.out}

test:		${TESTS:=.test}


%.test:	%.out Makefile
	rm -f $*.test
	sed \
	    -e '/\.cc:[0-9]* assert/s//.cc:XXXX assert/' \
	    -e '/\.h:[0-9]* assert/s//.h:XXXX assert/' \
	    -e '/\.cc:[0-9]* desire/s//.cc:XXXX desire/' \
	    -e '/\.L/s/\(\.L[A-Z][A-Z]*\)[0-9][0-9]*/\1XXX/g' \
	    -e '/line [0-9]/s//line #/g' \
	    -e '/\(lines [0-9][0-9]*-\)[0-9]/s//\1#/g' \
	    -e '/lines [0-9]\([0-9]*-#\)/s//lines #\1/g' \
	    -e '/#[0-9]/s//##/g' \
	    -e '/#[0-9]/s//##/g' \
	    -e '/#[0-9]/s//##/g' \
	    -e '/#[0-9]/s//##/g' \
	    -e '/#[0-9]/s//##/g' \
	    -e '/[0-9]#/s//##/g' \
	    -e '/[0-9]#/s//##/g' \
	    -e '/[0-9]#/s//##/g' \
	    -e '/[0-9]#/s//##/g' \
	    -e '/[0-9]#/s//##/g' \
	    -e '/@[0-9]/s//@@/g' \
	    -e '/@[0-9]/s//@@/g' \
	    -e '/@[0-9]/s//@@/g' \
	    -e '/@[0-9]/s//@@/g' \
	    -e '/@[0-9]/s//@@/g' \
	    -e '/^[ \t]*<..*>  *<..*>  *<..*>$D/d' \
            < $*.out > $*.test

%.diff:	%.out
	@echo DIFFING $*.test $*.out
	-@sed \
	    -e '/\.cc:[0-9]* assert/s//.cc:XXXX assert/' \
	    -e '/\.h:[0-9]* assert/s//.h:XXXX assert/' \
	    -e '/\.cc:[0-9]* desire/s//.cc:XXXX desire/' \
	    -e '/\.L/s/\(\.L[A-Z][A-Z]*\)[0-9][0-9]*/\1XXX/g' \
	    -e '/line [0-9]/s//line #/g' \
	    -e '/\(lines [0-9][0-9]*-\)[0-9]/s//\1#/g' \
	    -e '/lines [0-9]\([0-9]*-#\)/s//lines #\1/g' \
	    -e '/#[0-9]/s//##/g' \
	    -e '/#[0-9]/s//##/g' \
	    -e '/#[0-9]/s//##/g' \
	    -e '/#[0-9]/s//##/g' \
	    -e '/#[0-9]/s//##/g' \
	    -e '/[0-9]#/s//##/g' \
	    -e '/[0-9]#/s//##/g' \
	    -e '/[0-9]#/s//##/g' \
	    -e '/[0-9]#/s//##/g' \
	    -e '/[0-9]#/s//##/g' \
	    -e '/@[0-9]/s//@@/g' \
	    -e '/@[0-9]/s//@@/g' \
	    -e '/@[0-9]/s//@@/g' \
	    -e '/@[0-9]/s//@@/g' \
	    -e '/@[0-9]/s//@@/g' \
	    -e '/^[ \t]*<..*>  *<..*>  *<..*>$D/d' \
            < $*.out | diff ${DIFFOPT} $*.test -

%.out:	%
	rm -f $*.out
	${RUNENV} ./$* > $*.out

%.lex:	../src/%.lexcc Makefile
	sed -n \
		-e '/^lex	/s///p' \
		-e '/^lex$$/s///p' \
		< ../src/$*.lexcc \
		> $*.lex

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

ll_lexeme_basic_test:		\
		ll_lexeme_basic_test.cc \
		ll_lexeme.o \
		${MIN_O} \
		Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_lexeme.o \
	    ll_lexeme_basic_test.cc \
	    ${MIN}/lib/libmin.a

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

ll_lexeme_standard.lex:	\
		../src/ll_lexeme_standard.lexcc

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

ll_lexeme_standard_basic_test:		\
		ll_lexeme_standard_basic_test.cc \
		${LEXEME_O} \
		ll_lexeme_test.o \
		Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_lexeme_test.o \
	    ll_lexeme_standard_basic_test.cc \
	    ${LEXEME_O}

ll_lexeme_standard_basic_test.out:		\
	    ll_lexeme_standard_basic_test \
	    ll_lexeme_standard_test.in
	rm -f $*.out
	${RUNENV} ./$* -p < ll_lexeme_standard_test.in \
	               > $*.out

ll_lexeme_standard_special_test.out:		\
	    ll_lexeme_standard_basic_test \
	    ll_lexeme_standard_special_test.in
	rm -f $*.out
	${RUNENV} ./ll_lexeme_standard_basic_test \
	    < ll_lexeme_standard_special_test.in \
	    > $*.out

ll_lexeme_standard_test:		\
		ll_lexeme_standard_test.cc \
		ll_lexeme_test.o \
		${LEXEME_O} \
		Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_lexeme_test.o \
	    ll_lexeme_standard_test.cc \
	    ${LEXEME_O}

ll_lexeme_standard_test.out:		\
	    ll_lexeme_standard_test \
	    ll_lexeme_standard_test.in
	rm -f $*.out
	${RUNENV} ./$* < ll_lexeme_standard_test.in \
	               > $*.out

ll_lexeme_alternate.lex:	\
		../src/ll_lexeme_alternate.lexcc

ll_lexeme_alternate.cc:	\
		../src/ll_lexeme_alternate.lexcc

ll_lexeme_alternate.o:	\
		ll_lexeme_alternate.cc \
		${LEXEME_ALTERNATE_H} \
		../include/ll_lexeme_ndl.h \
		Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ll_lexeme_alternate.cc

ll_lexeme_alternate_basic_test:		\
		ll_lexeme_alternate_basic_test.cc \
		${LEXEME_ALTERNATE_O} \
		ll_lexeme_test.o \
		Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_lexeme_test.o \
	    ll_lexeme_alternate_basic_test.cc \
	    ${LEXEME_ALTERNATE_O}

ll_lexeme_alternate_basic_test.out:		\
	    ll_lexeme_alternate_basic_test \
	    ll_lexeme_alternate_test.in
	rm -f $*.out
	${RUNENV} ./$* < ll_lexeme_alternate_test.in \
	               > $*.out

ll_lexeme_alternate_test:		\
		ll_lexeme_alternate_test.cc \
		ll_lexeme_test.o \
		${LEXEME_ALTERNATE_O} \
		Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_lexeme_test.o \
	    ll_lexeme_alternate_test.cc \
	    ${LEXEME_ALTERNATE_O}

ll_lexeme_alternate_test.out:		\
	    ll_lexeme_alternate_test \
	    ll_lexeme_alternate_test.in
	rm -f $*.out
	${RUNENV} ./$* < ll_lexeme_alternate_test.in \
	               > $*.out

ll_lexeme_name_string_test:		\
		ll_lexeme_name_string_test.cc \
		ll_lexeme_test.o \
		${LEXEME_O} \
		Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_lexeme_test.o \
	    ll_lexeme_name_string_test.cc \
	    ${LEXEME_O}

ll_lexeme_name_string_test.out:		\
	    ll_lexeme_name_string_test
	rm -f $*.out
	${RUNENV} ./$* > $*.out

ll_lexeme_c++.lex:	\
		../src/ll_lexeme_c++.lexcc

ll_lexeme_c++.cc:	\
		../src/ll_lexeme_c++.lexcc

ll_lexeme_c++.o:	\
		ll_lexeme_c++.cc \
		${MIN_H} \
		../include/ll_lexeme.h \
		../include/ll_lexeme_ndl.h \
		../include/ll_lexeme_c++.h \
		Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ll_lexeme_c++.cc

ll_lexeme_c++_basic_test:		\
		ll_lexeme_c++_basic_test.cc \
		ll_lexeme.o \
		ll_lexeme_test.o \
		ll_lexeme_ndl.o \
		ll_lexeme_c++.o \
		${MIN_O} \
		Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_lexeme.o \
	    ll_lexeme_test.o \
	    ll_lexeme_ndl.o \
	    ll_lexeme_c++.o \
	    ll_lexeme_c++_basic_test.cc \
	    ${MIN_O}

ll_lexeme_c++_basic_test.out:		\
	    ll_lexeme_c++_basic_test \
	    ll_lexeme_c++_test.in
	rm -f $*.out
	${RUNENV} ./$* < ll_lexeme_c++_test.in \
	               > $*.out

ll_lexeme_c++_test:		\
		ll_lexeme_c++_test.cc \
		ll_lexeme.o \
		ll_lexeme_test.o \
		ll_lexeme_ndl.o \
		ll_lexeme_c++.o \
		${MIN_O} \
		Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_lexeme.o \
	    ll_lexeme_test.o \
	    ll_lexeme_ndl.o \
	    ll_lexeme_c++.o \
	    ll_lexeme_c++_test.cc \
	    ${MIN_O}

ll_lexeme_c++_test.out:		\
	    ll_lexeme_c++_test \
	    ll_lexeme_c++_test.in
	rm -f $*.out
	${RUNENV} ./$* < ll_lexeme_c++_test.in \
	               > $*.out

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

ll_parser_primary.o:	\
	${LEXEME_H} \
	../include/ll_parser.h \
	../include/ll_parser_table.h \
	../include/ll_parser_primary.h \
	../include/ll_parser_command.h \
	../src/ll_parser_primary.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_parser_primary.cc

ll_parser_standard_primary.o:	\
	${LEXEME_H} \
	${MEX_H} \
	../include/ll_parser.h \
	../include/ll_parser_table.h \
	../include/ll_parser_standard.h \
	../include/ll_parser_primary.h \
	../src/ll_parser_standard_primary.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -I ${MEX}/include \
	    -c ../src/ll_parser_standard_primary.cc

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

ll_parser_input_test:		\
		${PARSER_O} \
		ll_parser_input_test.cc \
		Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_parser_input_test.cc \
	    ${PARSER_O}

ll_parser_input_test.out:		\
	    ll_parser_input_test \
	    ll_lexeme_standard_test.in
	rm -f $*.out
	${RUNENV} ./$* < ll_lexeme_standard_test.in \
	               > $*.out

ll_parser_lexeme_map_standard_test.out:		\
	    ll_parser_input_test \
	    ll_parser_lexeme_map_standard_test.in
	rm -f $*.out
	${RUNENV} ./ll_parser_input_test -m \
	    < ll_parser_lexeme_map_standard_test.in \
	    > $*.out

ll_parser_table_test:		\
		ll_parser_table_test.cc \
		${PARSER_O} \
		Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_parser_table_test.cc \
	    ${PARSER_O}

ll_parser_table_test.out:		\
	    ll_parser_table_test
	rm -f $*.out
	${RUNENV} ./$* > $*.out

ll_parser_bracketed_standard_test:		\
	    ll_parser_bracketed_standard_test.cc \
	    ${PARSER_O} \
	    Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_parser_bracketed_standard_test.cc \
	    ${PARSER_O}

ll_parser_bracketed_standard_test.out:	\
	    ll_parser_bracketed_standard_test \
	    ll_parser_bracketed_standard_test.in
	rm -f $*.out
	${RUNENV} ./$* < \
	    ll_parser_bracketed_standard_test.in \
	    > $*.out

ll_parser_command_test:		\
	    ll_parser_command_test.cc \
	    ${PARSER_O} \
	    Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_parser_command_test.cc \
	    ${PARSER_O}

ll_parser_command_test.out:	\
	    ll_parser_command_test \
	    ll_parser_command_test.in
	rm -f $*.out
	${RUNENV} \
	    ./ll_parser_command_test < $*.in > $*.out

ll_parser_bracketed_command_test.out:	\
	    ll_parser_command_test \
	    ll_parser_bracketed_command_test.in
	rm -f $*.out
	${RUNENV} \
	    ./ll_parser_command_test \
		< $*.in > $*.out

ll_parser_prefix_command_test.out:	\
	    ll_parser_command_test \
	    ll_parser_prefix_command_test.in
	rm -f $*.out
	${RUNENV} \
	    ./ll_parser_command_test < $*.in > $*.out

ll_parser_oper_standard_test:		\
	    ll_parser_oper_standard_test.cc \
	    ${PARSER_O} \
	    Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_parser_oper_standard_test.cc \
	    ${PARSER_O}

ll_parser_oper_standard_test.out:	\
	    ll_parser_oper_standard_test \
	    ll_parser_oper_standard_test.in
	rm -f $*.out
	${RUNENV} \
	    ./ll_parser_oper_standard_test \
		< $*.in > $*.out

ll_parser_oper_command_test:		\
	    ll_parser_oper_command_test.cc \
	    ${PARSER_O} \
	    Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_parser_oper_command_test.cc \
	    ${PARSER_O}

ll_parser_oper_command_test.out:	\
	    ll_parser_oper_command_test \
	    ll_parser_oper_command_test.in
	rm -f $*.out
	${RUNENV} \
	    ./ll_parser_oper_command_test \
                < $*.in > $*.out

ll_parser_standard_test:		\
	    ll_parser_standard_test.cc \
	    ${PARSER_O} \
	    Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -I ${MEX}/include \
	    -o $@ \
	    ll_parser_standard_test.cc \
	    ${PARSER_O}

ll_parser_standard_test.out:	\
	    ll_parser_standard_test \
	    ll_parser_standard_test.in
	rm -f $*.out
	${RUNENV} ./ll_parser_standard_test < \
	    ll_parser_standard_test.in \
	    > $*.out

ll_parser_primary_command_test.out:	\
	    ll_parser_standard_test \
	    ll_parser_primary_command_test.in
	rm -f $*.out
	${RUNENV} \
	    ./ll_parser_standard_test < $*.in > $*.out

ll_parser_standard_errors_test.out:	\
	    ll_parser_standard_test \
	    ll_parser_standard_errors_test.in
	rm -f $*.out
	${RUNENV} ./ll_parser_standard_test < \
	    ll_parser_standard_errors_test.in \
	    > $*.out

ll_parser_standard_print_test.out:	\
	    ll_parser_standard_test \
	    ll_parser_standard_print_test.in
	rm -f $*.out
	${RUNENV} ./ll_parser_standard_test < \
	    ll_parser_standard_print_test.in \
	    > $*.out

clean:
	rm -f ${TESTS} ${TESTS:=.out} *.o \
	      ll_lexeme_standard.cc \
	      ll_lexeme_standard.lex \
	      ll_lexeme_c++.cc \
	      ll_lexeme_c++.lex
