# Layered Language Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@acm.org)
# Date:		Fri Aug  6 20:17:53 EDT 2010
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.
#
# RCS Info (may not be true date or author):
#
#   $Author: walton $
#   $Date: 2010/04/12 01:56:22 $
#   $RCSfile: Makefile,v $
#   $Revision: 1.2 $

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

PROGRAMS = ll_lexeme_test ll_standard_lexeme_test

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

ll_lexeme.o:	\
		../src/ll_lexeme.cc \
		../include/ll_lexeme.h \
		../include/ll_lexeme_program_data.h
	g++ -g -I ../include -c ../src/ll_lexeme.cc

ll_lexeme_test:		\
		ll_lexeme_test.cc \
		ll_lexeme.o \
		Makefile
	g++ -g ${GFLAGS} \
	    -I ../include \
	    -o $@ \
	    ll_lexeme.o \
	    ll_lexeme_test.cc

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

ll_lexeme_min.o:	${MIN}/include/min.h \
        ${MIN}/include/min_parameters.h \
	../include/ll_lexeme.h \
	../src/ll_lexeme_min.cc
	g++ -g ${GFLAGS} \
	    -I ../include -I ${MIN}/include \
	    -c ../src/ll_lexeme_min.cc

clean:
	rm -f ${PROGRAMS} ${PROGRAMS:=.out} *.o
