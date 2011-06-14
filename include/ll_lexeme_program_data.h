// Layers Language Lexical Analyzer Program Data
//
// File:	ll_lexeme_program_data.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jun 13 18:58:47 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	LL Lexeme Program Data

// Usage and Setup
// ----- --- -----

# ifndef LL_LEXEME_PROGRAM_DATA_H
# define LL_LEXEME_PROGRAM_DATA_H

# include <ll_lexeme.h>
using ll::lexeme::uns8;
using ll::lexeme::uns32;

// LL Lexeme Program Data
// -- ------ ------- ----

namespace ll { namespace lexeme
    { namespace program_data {

// All component headers begin with a program component
// type, a.k.a. `pctype', immediately followed by a
// line_number.  See ll_lexeme.h for the definitions of
// program component types.
//
// If the line_number is non-zero, then `#line-number'
// should be printed in place of the component ID.

// A program begins with a program header.  This is
// `ID' == 0 always, but this is never returned to the
// user.  The pctype can be used to tell whether the
// program is in proper endianhood or needs endianhood
// conversion.  The initial_table_ID is the ID of the
// initial master table.
//
// The lexeme types are 0 .. max_type.  The program
// header is followed by a vector of max_type+1 uns32
// values which are indices of type names.  This vector
// is followed by the type names as NUL terminated UTF-8
// character strings.  Given type t then
//
//	uns32 offset = program[program_header_length+t];
//	const char * type_name =
//	    offset == 0 ? NULL :
//	        (const char *) & program[0] + offset;
//
// computes the type name corresponding to t.
//
// program[component_length] is the first location after
// the program header, i.e., after the type name
// strings.  The type name UTF-8 strings therefore
// occupy component_length - program_header_length - 1
// - max_type uns32 elements.
//
struct program_header {
    uns32 pctype;		// == PROGRAM
    uns32 line_number;
    uns32 initial_table_ID;	// Initial master table.
    uns32 max_type;
    uns32 component_length;
};
const uns32 program_header_length = 5;

// A (lexical) table consists of just a header which
// records the mode, the dispatcher for the first char-
// acter of an atom, and the default instruction to be
// used if no atom is recognized.
//
struct table_header {
    uns32 pctype;		// == TABLE
    uns32 line_number;
    uns32 mode;
    uns32 dispatcher_ID;
    uns32 instruction_ID;
};
const uns32 table_header_length = 5;

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
    uns32 line_number;
    uns32 break_elements;
    uns32 max_break_elements;
    uns32 max_ctype;
};
const uns32 dispatcher_header_length = 5;
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
// header.  The component length is type_map_
// header_length + ( cmax - cmin + 4 ) / 4 if
// singleton_ctype is zero.
//
struct type_map_header {
    uns32 pctype;	    // == TYPE_MAP
    uns32 line_number;
    uns32 cmin, cmax;	    // Character range.
    uns32 singleton_ctype;  // If 0 use vector.
};
const uns32 type_map_header_length = 5;

// Instruction.  If operation includes TRANSLATE_TO(n)
// this is followed by the n uns32 characters of the
// translation.
//
struct instruction_header {
    uns32 pctype;	    // == INSTRUCTION
    uns32 line_number;
    uns32 operation;
    uns32 atom_table_ID;
    uns32 require_dispatcher_ID;
    uns32 else_instruction_ID;
    uns32 erroneous_atom_type;
    uns32 output_type;
    uns32 goto_table_ID;
    uns32 call_table_ID;
};
const uns32 instruction_header_length = 10;

} } }

# endif // LL_LEXEME_PROGRAM_DATA_H
