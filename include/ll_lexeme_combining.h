// Layered Languages Combining Characters Table
//
// File:	ll_lexeme_combining.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Jun  5 05:53:34 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// UNICODE ranges of characters to be treated as combin-
// ing characters which may appear as non-initial
// characters in identifiers.
//
// Taken from Doc no. WG21/N3146, 2004 Proposal for a
// set of allowed identifier characters.

    { 0x0300,	0x036F },
    { 0x1DC0,	0x1DFF },
    { 0x20D0,	0x20FF },
    { 0xFE20,	0xFE2F },
