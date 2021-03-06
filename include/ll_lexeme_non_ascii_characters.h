// Layered Languages Non-Ascii Characters Tables
//
// File:	ll_lexeme_non_ascii_characters.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Jun 24 06:38:03 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// UNICODE ranges of characters to be be allowed in
// identifiers.  This list does NOT include combining
// characters that should also be allowed as non-
// initial identifier characters.
//
// Taken from Doc no. WG21/N3146, 2004 Proposal for a
// set of allowed identifier characters.
//
#define NON_ASCII_LETTERS \
    { 0x00A8,	0x00A8 }, \
    { 0x00AA,	0x00AA }, \
    { 0x00AD,	0x00AD }, \
    { 0x00AF,	0x00AF }, \
    { 0x00B2,	0x00B5 }, \
    { 0x00B7,	0x00BA }, \
    { 0x00BC,	0x00BE }, \
    { 0x00C0,	0x00D6 }, \
    { 0x00D8,	0x00F6 }, \
    { 0x00F8,	0x00FF }, \
    { 0x0100,	0x02FF }, \
    /* { 0x0300,	0x036F }, excluded */ \
    { 0x0370,	0x167F }, \
    { 0x1681,	0x180D }, \
    { 0x180F,	0x1DBF }, \
    /* { 0x1DC0,	0x1DFF }, excluded */ \
    { 0x1E00,	0x1FFF }, \
    { 0x200B,	0x200D }, \
    { 0x202A,	0x202E }, \
    { 0x203F,	0x2040 }, \
    { 0x2054,	0x2054 }, \
    { 0x2060,	0x206F }, \
    { 0x2070,	0x20CF }, \
    /* { 0x20D0,	0x20FF }, excluded */ \
    { 0x2100,	0x218F }, \
    { 0x2460,	0x24FF }, \
    { 0x2776,	0x2793 }, \
    { 0x2C00,	0x2DFF }, \
    { 0x2E80,	0x2FFF }, \
    { 0x3004,	0x3007 }, \
    { 0x3021,	0x302F }, \
    { 0x3031,	0x303F }, \
    { 0x3040,	0xD7FF }, \
    { 0xF900,	0xFD3D }, \
    { 0xFD40,	0xFDCF }, \
    { 0xFDF0,	0xFE1F }, \
    /* { 0xFE20,	0xFE2F }, excluded */ \
    { 0xFE30,	0xFE44 }, \
    { 0xFE47,	0xFFFD }, \
    { 0x10000,	0x1FFFD }, \
    { 0x20000,	0x2FFFD }, \
    { 0x30000,	0x3FFFD }, \
    { 0x40000,	0x4FFFD }, \
    { 0x50000,	0x5FFFD }, \
    { 0x60000,	0x6FFFD }, \
    { 0x70000,	0x7FFFD }, \
    { 0x80000,	0x8FFFD }, \
    { 0x90000,	0x9FFFD }, \
    { 0xA0000,	0xAFFFD }, \
    { 0xB0000,	0xBFFFD }, \
    { 0xC0000,	0xCFFFD }, \
    { 0xD0000,	0xDFFFD }, \
    { 0xE0000,	0xEFFFD }

// UNICODE ranges of characters to be allowed as non-
// initial characters in identifiers.
//
// Taken from Doc no. WG21/N3146, 2004 Proposal for a
// set of allowed identifier characters.
//
#define NON_ASCII_COMBINING_CHARACTERS \
    { 0x0300,	0x036F }, \
    { 0x1DC0,	0x1DFF }, \
    { 0x20D0,	0x20FF }, \
    { 0xFE20,	0xFE2F }

// UNICODE ranges of non-ASCII control characters; i.e.,
// non-graphic characters.
//
#define NON_ASCII_CONTROL_CHARACTERS \
    { 0x0080,	0x00A0 }, \
    { 0x2000,	0x200F }, \
    { 0x2011,	0x2011 }, \
    { 0x2028,	0x202F }, \
    { 0x205F,	0x206F }
