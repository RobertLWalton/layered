// Layered Languages Lexical Analyzer Program Data
//
// File:	ll_lexeme_program_data.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Nov  5 12:54:00 EDT 2016
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
// conversion.
//
// The master tables are given indices 0 .. max_master.
// The lexeme types are 0 .. max_type.
//
// The program header proper (of length program_header_
// length) is followed in order by:
//
//     a vector of max_master+1 uns32 master table IDs
//       for master tables 0 through max_master
//     a vector of max_master+1 uns32 name offsets for
//       master tables 0 through max_master
//     a vector of max_type+1 uns32 name offsets for
//       types 0 through max_type
//     a vector of max_type+1 char type codes (i.e.,
//       short names) for types 0 through max_type
//     a list of all the master table and type names
//       whose offsets are given just above; these names
//       are UTF-8 NUL terminated character strings.
//
// Name offsets are in characters (not uns32's).
//
// The default master table always has master table
// index 0.  There is always at least one master table.
//
// If a name is missing its offset is 0.  If a type code
// is missing, it is 0.
//
// If no type names or type codes are given, max_type is
// 0, and type 0 has missing name and type code.
//
// program[component_length] is the first location after
// the complete program header, i.e., after the name
// strings.
//
struct program_header {
    uns32 pctype;		// == PROGRAM
    uns32 line_number;
    uns32 max_master;
    uns32 max_type;
    uns32 component_length;
};
const uns32 program_header_length =
    4 + ll::lexeme::MAX_INITIAL_TABLES;

// Returns max_master from program header.
//
inline min::uns32 max_master
	( ll::lexeme::program program )
{
    min::ptr<program_header> php =
        ll::lexeme::ptr<program_header> ( program, 0 );
    return php->max_master;
}

// Returns max_type from program header.
//
inline min::uns32 max_type
	( ll::lexeme::program program )
{
    min::ptr<program_header> php =
        ll::lexeme::ptr<program_header> ( program, 0 );
    return php->max_type;
}

// Returns pointer to ID of master m, or if none,
// returns min::ptr<min::uns32>().  The last includes
// the case where m > max_master of the program header.
//
inline min::ptr<min::uns32> master_ID
	( ll::lexeme::program program, uns32 m )
{
    min::ptr<program_header> php =
        ll::lexeme::ptr<program_header> ( program, 0 );
    min::uns32 max_master = php->max_master;
    if ( m > max_master )
        return min::ptr<min::uns32>();
    return program[program_header_length + m];
}

// Returns pointer to name of master m, or if none,
// returns min::ptr<const char>().  The last includes
// the case where m > max_master of the program header.
//
inline min::ptr<const char> master_name
	( ll::lexeme::program program, uns32 m )
{
    min::ptr<program_header> php =
        ll::lexeme::ptr<program_header> ( program, 0 );
    min::uns32 max_master = php->max_master;
    if ( m > max_master )
        return min::ptr<const char>();
    min::uns32 offset =
        program[program_header_length + max_master + 1
	                              + m];
    if ( offset == 0 )
	return min::ptr<const char>();
    else
	return   ll::lexeme::ptr<const char>
	             ( program, 0 )
	       + offset;
}

// Returns pointer to type name of type t, or if none,
// returns min::ptr<const char>().  The last includes
// the case where t > max_type of the program header.
//
inline min::ptr<const char> type_name
	( ll::lexeme::program program, uns32 t )
{
    min::ptr<program_header> php =
        ll::lexeme::ptr<program_header> ( program, 0 );
    min::uns32 max_master = php->max_master;
    min::uns32 max_type = php->max_type;
    if ( t > max_type )
        return min::ptr<const char>();
    min::uns32 offset =
        program[  program_header_length
	        + 2 * (max_master + 1 ) +  t];
    if ( offset == 0 )
	return min::ptr<const char>();
    else
	return   ll::lexeme::ptr<const char>
	             ( program, 0 )
	       + offset;
}

// Returns pointer to vector of type codes of a program.
//
inline min::ptr<const char> type_codes
	( ll::lexeme::program program )
{
    min::ptr<program_header> php =
        ll::lexeme::ptr<program_header> ( program, 0 );
    min::uns32 max_master = php->max_master;
    min::uns32 max_type = php->max_type;
    return ll::lexeme::ptr<const char>
	       ( program,
		   program_header_length
		 + 2 * ( max_master + 1 )
		 + max_type + 1 );
}

// Given a master table name, return the index of the
// master table, or NOT_FOUND if none found.  It is
// assumed that no two master tables have the same
// name.
//
const uns32 NOT_FOUND = 0xFFFFFFFF;
inline uns32 master_index
	( ll::lexeme::program program, const char name )
{
    min::ptr<program_header> php =
        ll::lexeme::ptr<program_header> ( program, 0 );
    min::uns32 max_master = php->max_master;
    min::ptr<const char> base =
        ll::lexeme::ptr<const char> ( program, 0 );
    for ( uns32 m = 0; m < max_master; ++ m )
    {
	min::uns32 offset =
	    program[program_header_length + max_master
	                                  + 1 + m];
	if ( offset == 0 ) continue;
	if ( strcmp ( name, ! ( base + offset ) ) == 0 )
	    return m;
    }
    return NOT_FOUND;
}

// Given a type name, return the type, or NOT_FOUND if
// none found.  It is assumed that no two types have the
// same name.
//
inline uns32 type
	( ll::lexeme::program program, const char name )
{
    min::ptr<program_header> php =
        ll::lexeme::ptr<program_header> ( program, 0 );
    min::uns32 max_master = php->max_master;
    min::uns32 max_type = php->max_type;
    min::ptr<const char> base =
        ll::lexeme::ptr<const char> ( program, 0 );
    for ( uns32 t = 0; t < max_type; ++ t )
    {
	min::uns32 offset =
	    program[program_header_length
	            +2 * (  max_master + 1 ) + t];
	if ( offset == 0 ) continue;
	if ( strcmp ( name, ! ( base + offset ) ) == 0 )
	    return t;
    }
    return NOT_FOUND;
}

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
//	vector of dispatcher map elements
//	vector of dispatcher character types (ctype_map)
//
// The vector of dispatcher character types (the ctype
// map) maps the UNICODE character index of a character
// C (i.e., cindex = min::Uindex(C)) to an uns8 charac-
// ter type, ctype.  The vector has ctype_map_size uns8
// elements.  If cindex is too large for the vector
// (i.e., cindex >= ctype_map_size), then ctype is 0.
// It is always true that ctype <= max_ctype.
//
// The dispatcher map elements map the ctype to an
// instruction I, another dispatcher D, and a repeat
// count R.  There are max_ctype+1 map elements.  I and
// D are represented by instruction and dispatcher IDs,
// which are 0 if I or D is absent.  The repetition
// count is 0 if it is absent.
//
// If I is present it is the instruction executed if
// the current atom ends with C.  If D is present it
// is the dispatcher applied to the character after C
// to see if a longer atom can be recognized.  If
// neither I or D are present then C is not legal in
// the current atom, which must end before C.
//
// A non-zero repeat count R means that characters after
// C are examined to see if they have the same ctype as
// C, and if they do, as many as R are recognized by
// this the dispatcher, as if D had been this dispatcher
// itself.  When there are no more characters with the
// same ctype as C, or when R characters have been
// processed, execution continues to dispatcher D.
//
// component_length is the total number of uns32
// elements in the dispatcher, including the
// ctype_map_size uns8 ctype elements at the end.
//
struct dispatcher_header {
    uns32 pctype;		// == DISPATCHER
    uns32 line_number;
    uns32 max_ctype;
    uns32 ctype_map_size;
    uns32 component_length;
};
const uns32 dispatcher_header_length = 5;
struct map_element {
    uns32 dispatcher_ID;
    uns32 instruction_ID;
    uns32 repeat_count;
};
const uns32 map_element_length = 3;

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
