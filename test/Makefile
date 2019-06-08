# Layered Language Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@acm.org)
# Date:		Sat Jun  8 14:00:06 EDT 2019
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.

# We need the following to mask RCS lines
#
D = $$

# The following locates the MIN source code.
#
MIN = ~/min

# The following turns on MIN debugging.
#
GFLAGS = -DMIN_PROTECT=1
WFLAGS = -Wall -Wno-char-subscripts

.SUFFIXES:
.SUFFIXES: .out .test .diff

# O3 is needed to inline functions.
#
OPTIMIZE = -O3

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
	       ll_parser_standard_print_test

TESTS = ${LEXEME_TESTS} ${PARSER_TESTS}

MIN_H =	${MIN}/include/min.h \
        ${MIN}/include/min_parameters.h \
        ${MIN}/include/min_os.h \
        ${MIN}/include/min_unicode.h \
        ${MIN}/unicode/unicode_data.h \
        ${MIN}/unicode/unicode_data_support_sets.h


MIN_O =	min_assert.o min.o min_os.o min_acc.o \
        min_unicode.o

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
		ll_parser_standard.o \
		ll_parser_standard_input.o \
		ll_parser_standard_brackets.o \
		ll_parser_standard_oper.o

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
	    -e '/\$DDate: .*\$D/s//\$DRCS Date: XXX\$D/' \
	    -e '/\$DRevision: .*\$D/s//\$DRCS Revision: XXX\$D/' \
	    -e '/\$DRCSfile: .*\$D/s//\$DRCS RCSfile: XXX\$D/' \
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
	    -e '/\$DDate: .*\$D/s//\$DRCS Date: XXX\$D/' \
	    -e '/\$DRevision: .*\$D/s//\$DRCS Revision: XXX\$D/' \
	    -e '/\$DRCSfile: .*\$D/s//\$DRCS RCSfile: XXX\$D/' \
            < $*.out | diff --minimal -u ${DIFF_FLAGS} $*.test -

%.out:	%
	rm -f $*.out
	${RUNENV} $* > $*.out

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

min.o:	${MIN_H} \
	${MIN}/src/min.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ${MIN}/include \
	    -c ${MIN}/src/min.cc

min_assert.o:	${MIN}/src/min_assert.cc \
		${MIN}/include/min_parameters.h
	rm -f min_assert.o
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ${MIN}/include \
	    -c ${MIN}/src/min_assert.cc \

min_os.o:	${MIN_H} \
	${MIN}/src/min_os.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ${MIN}/include \
	    -c ${MIN}/src/min_os.cc

min_acc.o:	${MIN_H} \
        ${MIN}/include/min_acc.h \
        ${MIN}/include/min_acc_parameters.h \
	${MIN}/src/min_acc.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ${MIN}/include \
	    -c ${MIN}/src/min_acc.cc

min_unicode.o:	${MIN_H} \
        ${MIN}/include/min_unicode.h \
	${MIN}/src/min_unicode.cc \
	${MIN}/unicode/unicode_data.cc \
	Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ${MIN}/include \
	    -c ${MIN}/src/min_unicode.cc

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
	    ${MIN_O} \
	    ll_lexeme_basic_test.cc

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
	    ${LEXEME_O} \
	    ll_lexeme_test.o \
	    ll_lexeme_standard_basic_test.cc

ll_lexeme_standard_basic_test.out:		\
	    ll_lexeme_standard_basic_test \
	    ll_lexeme_standard_test.in
	rm -f $*.out
	${RUNENV} $* -p < ll_lexeme_standard_test.in \
	             > $*.out

ll_lexeme_standard_special_test.out:		\
	    ll_lexeme_standard_basic_test \
	    ll_lexeme_standard_special_test.in
	rm -f $*.out
	${RUNENV} ll_lexeme_standard_basic_test \
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
	    ${LEXEME_O} \
	    ll_lexeme_standard_test.cc

ll_lexeme_standard_test.out:		\
	    ll_lexeme_standard_test \
	    ll_lexeme_standard_test.in
	rm -f $*.out
	${RUNENV} $* < ll_lexeme_standard_test.in \
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
	    ${LEXEME_ALTERNATE_O} \
	    ll_lexeme_test.o \
	    ll_lexeme_alternate_basic_test.cc

ll_lexeme_alternate_basic_test.out:		\
	    ll_lexeme_alternate_basic_test \
	    ll_lexeme_alternate_test.in
	rm -f $*.out
	${RUNENV} $* < ll_lexeme_alternate_test.in \
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
	    ${LEXEME_ALTERNATE_O} \
	    ll_lexeme_alternate_test.cc

ll_lexeme_alternate_test.out:		\
	    ll_lexeme_alternate_test \
	    ll_lexeme_alternate_test.in
	rm -f $*.out
	${RUNENV} $* < ll_lexeme_alternate_test.in \
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
	    ${LEXEME_O} \
	    ll_lexeme_name_string_test.cc

ll_lexeme_name_string_test.out:		\
	    ll_lexeme_name_string_test
	rm -f $*.out
	${RUNENV} $* > $*.out

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
	    ${MIN_O} \
	    ll_lexeme_c++_basic_test.cc

ll_lexeme_c++_basic_test.out:		\
	    ll_lexeme_c++_basic_test \
	    ll_lexeme_c++_test.in
	rm -f $*.out
	${RUNENV} $* < ll_lexeme_c++_test.in \
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
	    ${MIN_O} \
	    ll_lexeme_c++_test.cc

ll_lexeme_c++_test.out:		\
	    ll_lexeme_c++_test \
	    ll_lexeme_c++_test.in
	rm -f $*.out
	${RUNENV} $* < ll_lexeme_c++_test.in \
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
	    ${PARSER_O} \
	    ll_parser_input_test.cc

ll_parser_input_test.out:		\
	    ll_parser_input_test \
	    ll_lexeme_standard_test.in
	rm -f $*.out
	${RUNENV} $* < ll_lexeme_standard_test.in \
	             > $*.out

ll_parser_lexeme_map_standard_test.out:		\
	    ll_parser_input_test \
	    ll_parser_lexeme_map_standard_test.in
	rm -f $*.out
	${RUNENV} ll_parser_input_test -m \
	    < ll_parser_lexeme_map_standard_test.in \
	    > $*.out

ll_parser_table_test:		\
		ll_parser_table_test.cc \
		${PARSER_O} \
		Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ${PARSER_O} \
	    ll_parser_table_test.cc

ll_parser_table_test.out:		\
	    ll_parser_table_test
	rm -f $*.out
	${RUNENV} $* > $*.out

ll_parser_bracketed_standard_test:		\
	    ll_parser_bracketed_standard_test.cc \
	    ${PARSER_O} \
	    Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ${PARSER_O} \
	    ll_parser_bracketed_standard_test.cc

ll_parser_bracketed_standard_test.out:	\
	    ll_parser_bracketed_standard_test \
	    ll_parser_bracketed_standard_test.in
	rm -f $*.out
	${RUNENV} $* < \
	    ll_parser_bracketed_standard_test.in \
	    > $*.out

ll_parser_command_test:		\
	    ll_parser_command_test.cc \
	    ${PARSER_O} \
	    Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ${PARSER_O} \
	    ll_parser_command_test.cc

ll_parser_command_test.out:	\
	    ll_parser_command_test \
	    ll_parser_command_test.in
	rm -f $*.out
	${RUNENV} \
	    ll_parser_command_test < $*.in > $*.out

ll_parser_bracketed_command_test.out:	\
	    ll_parser_command_test \
	    ll_parser_bracketed_command_test.in
	rm -f $*.out
	${RUNENV} \
	    ll_parser_command_test \
		< $*.in > $*.out

ll_parser_prefix_command_test.out:	\
	    ll_parser_command_test \
	    ll_parser_prefix_command_test.in
	rm -f $*.out
	${RUNENV} \
	    ll_parser_command_test < $*.in > $*.out

ll_parser_oper_standard_test:		\
	    ll_parser_oper_standard_test.cc \
	    ${PARSER_O} \
	    Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ${PARSER_O} \
	    ll_parser_oper_standard_test.cc

ll_parser_oper_standard_test.out:	\
	    ll_parser_oper_standard_test \
	    ll_parser_oper_standard_test.in
	rm -f $*.out
	${RUNENV} \
	    ll_parser_oper_standard_test < $*.in > $*.out

ll_parser_oper_command_test:		\
	    ll_parser_oper_command_test.cc \
	    ${PARSER_O} \
	    Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ${PARSER_O} \
	    ll_parser_oper_command_test.cc

ll_parser_oper_command_test.out:	\
	    ll_parser_oper_command_test \
	    ll_parser_oper_command_test.in
	rm -f $*.out
	${RUNENV} \
	    ll_parser_oper_command_test < $*.in > $*.out

ll_parser_standard_print_test:		\
	    ll_parser_standard_print_test.cc \
	    ${PARSER_O} \
	    Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ${PARSER_O} \
	    ll_parser_standard_print_test.cc

ll_parser_standard_print_test.out:	\
	    ll_parser_standard_print_test \
	    ll_parser_standard_print_test.in
	rm -f $*.out
	${RUNENV} $* < \
	    ll_parser_standard_print_test.in \
	    > $*.out

clean:
	rm -f ${TESTS} ${TESTS:=.out} *.o \
	      ll_lexeme_standard.cc \
	      ll_lexeme_standard.lex \
	      ll_lexeme_c++.cc \
	      ll_lexeme_c++.lex
