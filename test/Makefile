# Layered Language Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@acm.org)
# Date:		Sun Jan 23 01:09:24 EST 2011
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

.SUFFIXES:
.SUFFIXES: .out .test .diff

# O3 is needed to inline functions.
#
OPTIMIZE = -O3

PROGRAMS = ll_lexeme_basic_test \
           ll_lexeme_standard_basic_test \
           ll_lexeme_standard_test \
	   ll_parser_input_test

TESTS = ${PROGRAMS}

.SECONDARY:	${TESTS:=.out}

all:		diff

diff:		${TESTS:=.diff}

out:		${PROGRAMS:=.out}

test:		${PROGRAMS:=.test}


%.test:	%.out Makefile
	rm -f $*.test
	sed \
	    -e '/\.cc:[0-9]* assert/s//.cc:XXXX assert/' \
	    -e '/\.h:[0-9]* assert/s//.h:XXXX assert/' \
	    -e '/\.cc:[0-9]* desire/s//.cc:XXXX desire/' \
	    -e '/\.L/s/\(\.L[A-Z][A-Z]*\)[0-9][0-9]*/\1XXX/g' \
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
	    -e '/\$DDate: .*\$D/s//\$DRCS Date: XXX\$D/' \
	    -e '/\$DRevision: .*\$D/s//\$DRCS Revision: XXX\$D/' \
	    -e '/\$DRCSfile: .*\$D/s//\$DRCS RCSfile: XXX\$D/' \
            < $*.out | diff --minimal $*.test -

%.out:	%
	rm -f $*.out
	${RUNENV} $* > $*.out

min.o:	${MIN}/include/min.h \
        ${MIN}/include/min_parameters.h \
	${MIN}/src/min.cc
	g++ -g ${GFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ${MIN}/src/min.cc

min_os.o:	${MIN}/include/min.h \
        ${MIN}/include/min_parameters.h \
        ${MIN}/include/min_os.h \
	${MIN}/src/min_os.cc
	g++ -g ${GFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ${MIN}/src/min_os.cc

min_acc.o:	${MIN}/include/min.h \
        ${MIN}/include/min_parameters.h \
        ${MIN}/include/min_acc.h \
        ${MIN}/include/min_acc_parameters.h \
        ${MIN}/include/min_os.h \
	${MIN}/src/min_acc.cc
	g++ -g ${GFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ${MIN}/src/min_acc.cc

ll_lexeme.o:	\
	${MIN}/include/min.h \
        ${MIN}/include/min_parameters.h \
	../include/ll_lexeme.h \
	../include/ll_lexeme_program_data.h \
	../src/ll_lexeme.cc
	g++ -g ${GFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_lexeme.cc

ll_lexeme_test.o:	\
	${MIN}/include/min.h \
        ${MIN}/include/min_parameters.h \
	../include/ll_lexeme.h \
	../include/ll_lexeme_program_data.h \
	../src/ll_lexeme_test.cc
	g++ -g ${GFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_lexeme_test.cc

ll_lexeme_basic_test:		\
		ll_lexeme_basic_test.cc \
		ll_lexeme.o \
		min.o \
		min_os.o \
		min_acc.o \
		Makefile
	g++ -g ${GFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_lexeme.o \
	    min.o \
	    min_os.o \
	    min_acc.o \
	    ll_lexeme_basic_test.cc

ll_lexeme_ndl.o:	\
		../src/ll_lexeme_ndl.cc \
		../include/ll_lexeme.h \
		../include/ll_lexeme_ndl.h \
		../include/ll_lexeme_ndl_data.h
	g++ -g ${GFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_lexeme_ndl.cc

ll_lexeme_standard.o:	\
		../src/ll_lexeme_standard.cc \
		../include/ll_lexeme.h \
		../include/ll_lexeme_ndl.h \
		../include/ll_lexeme_standard.h
	g++ -g ${GFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_lexeme_standard.cc

ll_lexeme_standard_basic_test:		\
		ll_lexeme_standard_basic_test.cc \
		ll_lexeme.o \
		ll_lexeme_test.o \
		ll_lexeme_ndl.o \
		ll_lexeme_standard.o \
		min.o \
		min_os.o \
		min_acc.o \
		Makefile
	g++ -g ${GFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_lexeme.o \
	    ll_lexeme_test.o \
	    ll_lexeme_ndl.o \
	    ll_lexeme_standard.o \
	    min.o \
	    min_os.o \
	    min_acc.o \
	    ll_lexeme_standard_basic_test.cc

ll_lexeme_standard_basic_test.out:		\
	    ll_lexeme_standard_basic_test \
	    ll_lexeme_standard_test.in
	rm -f $*.out
	${RUNENV} $* < ll_lexeme_standard_test.in \
	             > $*.out

ll_lexeme_standard_test:		\
		ll_lexeme_standard_test.cc \
		ll_lexeme.o \
		ll_lexeme_test.o \
		ll_lexeme_ndl.o \
		ll_lexeme_standard.o \
		min.o \
		min_os.o \
		min_acc.o \
		Makefile
	g++ -g ${GFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_lexeme.o \
	    ll_lexeme_test.o \
	    ll_lexeme_ndl.o \
	    ll_lexeme_standard.o \
	    min.o \
	    min_os.o \
	    min_acc.o \
	    ll_lexeme_standard_test.cc

ll_lexeme_standard_test.out:		\
	    ll_lexeme_standard_test \
	    ll_lexeme_standard_test.in
	rm -f $*.out
	${RUNENV} $* < ll_lexeme_standard_test.in \
	             > $*.out

ll_parser.o:	\
	${MIN}/include/min.h \
        ${MIN}/include/min_parameters.h \
	../include/ll_parser.h \
	../include/ll_parser_input.h \
	../src/ll_parser.cc
	g++ -g ${GFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_parser.cc

ll_parser_input.o:	\
	${MIN}/include/min.h \
        ${MIN}/include/min_parameters.h \
	../include/ll_parser.h \
	../include/ll_parser_input.h \
	../src/ll_parser_input.cc
	g++ -g ${GFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_parser_input.cc

ll_parser_input_test:		\
		ll_parser_input_test.cc \
		ll_lexeme.o \
		ll_lexeme_ndl.o \
		ll_lexeme_standard.o \
		ll_parser.o \
		ll_parser_input.o \
		min.o \
		min_os.o \
		min_acc.o \
		Makefile
	g++ -g ${GFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -o $@ \
	    ll_lexeme.o \
	    ll_lexeme_ndl.o \
	    ll_lexeme_standard.o \
	    ll_parser.o \
	    ll_parser_input.o \
	    min.o \
	    min_os.o \
	    min_acc.o \
	    ll_parser_input_test.cc

ll_parser_input_test.out:		\
	    ll_parser_input_test \
	    ll_lexeme_standard_test.in
	rm -f $*.out
	${RUNENV} $* < ll_lexeme_standard_test.in \
	             > $*.out

ll_parser_table.o:	\
	${MIN}/include/min.h \
        ${MIN}/include/min_parameters.h \
	../include/ll_lexeme.h \
	../include/ll_parser_pass.h \
	../include/ll_parser_table.h \
	../src/ll_parser_table.cc
	g++ -g ${GFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_parser_table.cc

clean:
	rm -f ${PROGRAMS} ${PROGRAMS:=.out} *.o
