// Layers Language Lexical Analyzer Program Data
//
// File:	ll_lexeme_program_data.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat May  8 04:40:19 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/05/08 09:19:59 $
//   $RCSfile: ll_lexeme_program_data.h,v $
//   $Revision: 1.10 $

// Table of Contents
//
//	Usage and Setup
//	LL Lexeme Program Data

// Usage and Setup
// ----- --- -----

# ifndef LL_LEXEME_PROGRAM_H
# define LL_LEXEME_PROGRAM_H

# include <ll_lexeme.h>
using ll::lexeme::uns8;
using ll::lexeme::uns32;

// LL Lexeme Program Data
// -- ------ ------- ----

namespace ll { namespace lexeme
    { namespace program_data {

// Program component types (aka `pctypes').
//
enum {
    PROGRAM			= 1,
    ATOM_TABLE			= 2,
    TYPE_MAP			= 3,
    DISPATCHER			= 4,
    INSTRUCTION			= 5
};

// A program begins with a program header.  This is
// `ID' == 0 always, but this is never returned to the
// user.  The pctype can be used to tell whether the
// program is in proper endianhood or needs endianhood
// conversion.
//
struct program_header {
    uns32 pctype;		// == PROGRAM
    uns32 atom_table_ID;	// Initial atom table.
};
const uns32 program_header_length = 2;

// An atom table consists of just a header which records
// the mode and label, and the dispatcher for the first
// character of an atom, and the instruction to be used
// if no atom is recognized.
//
struct atom_table_header {
    uns32 pctype;		// == ATOM_TABLE
    uns32 mode;
    uns32 dispatcher_ID;
    uns32 instruction_ID;
};
const uns32 atom_table_header_length = 4;

// The format of a dispatcher is
//
//	dispatcher header
//	vector of dispatcher break elements
//	vector of dispatcher map elements
//
// The dispatcher break elements map a character C to a
// type map M.  The type map M maps C to a character
// type (a.k.a. `ctype') T, and then the dispatcher map
// elements map T to an instruction I and another
// dispatcher D.
//
// If I is present it is the instruction executed if
// the current atom ends with C.  If D is present it
// is the dispatcher applied to the character after C
// to see if a longer atom can be recognized.  If
// neither I or D are present then C is not legal in
// the current atom, which must end before C.
//
// If there are n breakpoints in a dispatcher there are
// n+1 break elements in the dispatcher.  Each break
// element applies to the range of characters cmin ..
// cmax, where cmin is a member of the break element,
// and cmax = next break element's cmin - 1, if there
// is a next break element, or = 0xFFFFFFFF if there
// is no next break element.  The first break element
// always has cmin = 0.  Break elements are sorted
// to have ascending cmin values, so a binary search
// of the break elements can be used to map C to a
// break element.
//
// A character C maps to the break element for which
// cmin <= C <= cmax.  The character is then further
// mapped by the type map M whose ID is given in the
// break element, if that ID is non-zero.  If the ID
// is zero, the character is mapped to ctype 0.
//
// If a character is mapped to ctype T (by M if it is
// present, or to T = 0 otherwise), then T is mapped to
// the T+1'st dispatcher map element.  This gives the
// IDs of the instruction I and dispatcher D indicated
// above.  If either ID is 0 the instruction or
// dispatcher is missing.
//
struct dispatcher_header {
    uns32 pctype;		// == DISPATCHER
    uns32 break_elements;
    uns32 max_break_elements;
    uns32 max_ctype;
};
const uns32 dispatcher_header_length = 4;
struct break_element {
    uns32 cmin;
    uns32 type_map_ID;
};
const uns32 break_element_length = 2;
struct map_element {
    uns32 dispatcher_ID;
    uns32 instruction_ID;
};
const uns32 map_element_length = 2;

// A type map maps a character range to either
// a singleton_ctype is that is non-zero, or to
// map[c-cmin] otherwise, where map is the uns8 *
// pointer to the first byte after the type map
// header.
//
struct type_map_header {
    uns32 pctype;	    // == TYPE_MAP
    uns32 cmin, cmax;	    // Character range.
    uns32 singleton_ctype;  // If 0 use vector.
};
const uns32 type_map_header_length = 4;

// Instruction.  If operation includes TRANSLATE(n)
// this is followed by the n uns32 characters of the
// translation.  If operation includes ELSE this is
// followed by the else_instruction struct.
//
struct instruction_header {
    uns32 pctype;	    // == INSTRUCTION
    uns32 operation;
    uns32 atom_table_ID;    // for GOTO
    uns32 type;		    // for ERRONEOUS_ATOM or
    			    //     OUTPUT
};
const uns32 instruction_header_length = 4;

struct else_instruction {
    uns32 else_dispatcher_ID;
    uns32 else_instruction_ID;
};
const uns32 else_instruction_length = 2;

} } }

# endif // LL_LEXEME_PROGRAM_H
