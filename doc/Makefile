

# Makefile for Layered Language Documentation
#
# File:		Makefile
# Author:	Bob Walton (walton@acm.org)
# Date:		Fri Jun 20 14:59:22 EDT 2014
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.
#
# RCS Info (may not be true date or author):
#
#   $Author: walton $
#   $Date: 2009/04/14 00:32:33 $
#   $RCSfile: Makefile,v $
#   $Revision: 1.3 $

DOCUMENTS = layered-introduction \
	    layered-l-language \
	    layered-m-language

pdf: ${DOCUMENTS:=.pdf}

layered-introduction:	layered-introduction.pdf
layered-l-language:	layered-l-language.pdf
layered-m-language:	layered-m-language.pdf

# Discontinued
#
layered-l-2012-language:	\
			layered-l-2012-language.pdf

# Kill all implicit rules and make new rule:
#
.SUFFIXES:
.SUFFIXES: .tex .pdf
.tex.pdf:
	pdflatex $* 
	makeindex $*

clean:
	rm -f ${DOCUMENTS:=.tex.bak}
	rm -f ${DOCUMENTS:=.aux}
	rm -f ${DOCUMENTS:=.idx}
	rm -f ${DOCUMENTS:=.ilg}
	rm -f ${DOCUMENTS:=.ind}
	rm -f ${DOCUMENTS:=.log}
	rm -f ${DOCUMENTS:=.toc}
	rm -f ${DOCUMENTS:=.dvi}
	rm -f ${DOCUMENTS:=.ps}
	rm -f ${DOCUMENTS:=.pdf}
