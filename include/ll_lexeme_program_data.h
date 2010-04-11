// Layers Language Lexical Analyzer Program Data
//
// File:	ll_lexeme_program_data.h
// Author:	Bob Walton (walton@seas.harvard.edu)
// Date:	Sat Apr 10 20:07:35 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/11 00:07:49 $
//   $RCSfile: ll_lexeme_program_data.h,v $
//   $Revision: 1.3 $

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

// Program item types.
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
// user.  The type can be used to tell whether the
// program is in proper endianhood or needs endianhood
// conversion.
//
struct program_header {
    uns32 type;			// == PROGRAM
    uns32 atom_table_ID;	// Initial atom table.
};
const uns32 program_header_length = 2;

// An atom table consists of just a header which records
// the mode and label, and the dispatcher for the first
// character of an atom, and the instruction to be used
// if no atom is recognized.
//
struct atom_table_header {
    uns32 type;			// == ATOM_TABLE
    uns32 mode;
    uns32 label;
    uns32 dispatcher_ID;
    uns32 instruction_ID;
};
const uns32 atom_table_header_length = 5;

// The format of a dispatcher is
//
//	dispatcher header
//	vector of dispatcher break elements
//	vector of dispatcher map elements
//
// If there are n breakpoints in a dispatcher there are
// n+1 break elements in the dispatcher.  Each break
// element applies to the range of characters cmin ..
// cmax, where cmin is a member of the break element,
// and cmax = next break element's cmin - 1, if there
// is a next break element, or = 0xFFFFFFFF if there
// is no next break element.
//
// A character c maps to the break element for which
// cmin <= c <= cmax.  The character is then further
// mapped by the type map whose ID is given in the break
// element, if that ID is non-zero.  If the ID is zero,
// the character is mapped to type 0.
//
// If a character is mapped to type t, it is mapped to
// the t+1'st map element.  This gives a dispatcher to
// be used to see if the current atom can be extended
// beyond the current character, and an instruction to
// be used if the current character is found to be the
// last character of the current atom.  If the
// dispatcher is missing (ID == 0) then the current
// character can only be the last character of the atom,
// and if the instruction is missing (ID == 0) the
// current character cannot be the last character of the
// atom.
//
struct dispatcher_header {
    uns32 type;			// == DISPATCHER
    uns32 break_elements;
    uns32 max_break_elements;
    uns32 max_type;
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
// a singleton_type is that is non-zero, or to
// map[c-cmin] otherwise, where map is the uns8 *
// pointer to the first byte after the type map
// header.
//
struct type_map_header {
    uns32 type;		    // == TYPE_MAP
    uns32 cmin, cmax;	    // Character range.
    uns32 singleton_type;   // If 0 use vector.
};
const uns32 type_map_header_length = 4;

// Instruction.  If operation includes TRANSLATE(n)
// this is followed by the n uns32 characters of the
// translation.
//
struct instruction_header {
    uns32 type;			// == INSTRUCTION
    uns32 operation;
    uns32 atom_table_ID;
};
const uns32 instruction_header_length = 3;

} } }

# endif // LL_LEXEME_PROGRAM_H
