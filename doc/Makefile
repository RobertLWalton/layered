

# Makefile for Layered Language Documentation
#
# File:		Makefile
# Author:	Bob Walton (walton@acm.org)
# Date:		Sat Apr 11 10:20:15 EDT 2009
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

dvi: ${DOCUMENTS:=.dvi}
ps: ${DOCUMENTS:=.ps}
pdf: ${DOCUMENTS:=.pdf}
lpr: ${DOCUMENTS:=.lpr}

layered-introduction:	layered-introduction.dvi
layered-l-language:	layered-l-language.dvi
layered-m-language:	layered-m-language.dvi

# Kill all implicit rules and make new rule:
#
.SUFFIXES:
.SUFFIXES: .tex .dvi .ps .pdf .lpr
.tex.dvi:
	latex $*
	makeindex $*
	latex $*
	makeindex $*
	latex $*
	makeindex $*

.dvi.ps:
	dvips -O 0in,1in -o $*.ps $*.dvi

.ps.pdf:
	ps2pdf $*.ps $*.pdf

.ps.lpr:
	lpr $*.ps

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
