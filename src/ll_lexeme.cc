// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Wed Apr  7 06:16:00 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/07 11:11:43 $
//   $RCSfile: ll_lexeme.cc,v $
//   $Revision: 1.2 $

// Table of Contents
//
//	Usage and Setup
//	Data Definitions
//	Functions

// Usage and Setup
// ----- --- -----

# include <ll_lexeme>
# define LLLEX ll::lexeme
using LLLEX::uns8;
using LLLEX::uns32;


// Data Definitions
// ---- -----------

uns32 ** LLLEX::program_pointer;
uns32 LLLEX::header_length;
uns32 LLLEX::length;
uns32 LLLEX::max_length;
uns32 LLLEX::length_increment = 1000;

enum {
    PROGRAM			= 1,
    ATOM_TABLE			= 2,
    TYPE_TABLE			= 3,
    SORTED_DISPATCHER		= 4,
    TYPE_DISPATCHER		= 5,
    INSTRUCTION			= 6
};

struct program_header {
    uns32 type;
    uns32 initial_atom_table_ID;
};
const uns32 program_header_length = 2;

struct atom_table_header {
    uns32 type;
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
// mapped to the type given in the break element if that
// is non-zero, or is mapped by the type table whose
// ID is given in the break element, if that ID is non-
// zero.
//
// If a character is mapped to type t, it is mapped to
// the t+1'st map element.  This optionally gives a
// dispatcher to be used to see if the current atom can
// be extended beyond the current character, and an
// instruction to be used if the current character is
// found to be the last character of the current atom.
//
struct dispatcher_header {
    uns32 type;
    uns32 elements;
    uns32 max_elements;
    uns32 max_types;
};
const uns32 dispatcher_header_length = 4;
struct break_element {
    uns32 cmin;
    uns32 type;
    uns32 type_table_ID;
};
const uns32 break_element_length = 3;
struct map_element {
    uns32 dispatcher_ID;
    uns32 instruction_ID;
};
const uns32 map_element_length = 2;

// A type table is a vector of uns8 elements,
// such that character c maps to base[c-cmin],
// where base is the uns8 * pointer to the first
// byte after the type table header.
//
struct type_table_header {
    uns32 type;
    uns32 cmin, cmax;	// Character range.
};
const uns32 type_table_header_length = 3;

static uns32 allocate_to_program ( uns32 needed_size )
{
    uns32 available = LLLEX::max_length - LLLEX::length;
    if ( needed_size > available )
    {
        LLLEX::max_length += ( needed_size - available )
		           + LLLEX::length_increment;
	LLLEX::resize_program ( LLLEX::max_length );
    }
    uns32 result = LLLEX::length;
    LLLEX::length += needed_size;
    return result;
}

uns32 LLLEX::create_atom_table ( uns8 mode )
{
    uns32 ID = allocate_to_program
    		   ( atom_table_header_length );
    atom_table_header & h =
        * (atom_table_header *) & program()[ID];
    h.type = ATOM_TABLE;
    h.mode = mode;
    h.dispatcher = 0;
    h.instruction = 0;
    return ID;
}

uns32 LLLEX::create_program ( void )
{
    program_pointer = allocate_program();
    max_length = length = header_length =
	    program_length();
    uns32 ID = allocate_to_program
    		   ( program_table_header_length );
    program_table_header & h =
        * (program_table_header *) & program()[ID];
    h.type = PROGRAM_TABLE;
    h.atom_table = create_atom_table ( MASTER );
    return h.atom_table;
}

// Create a sorted dispatcher with the given maximum
// number of breakpoints and return its ID.
//
static uns32 create_sorted_dispatcher
	( uns32 breakpoints )
{
    uns32 length =
          sorted_dispatcher_header_length
	+   sorted_dispatcher_element_length
	  * ( breakpoints + 1 );
    uns32 ID = allocate_to_program ( length );
    sorted_dispatcher_header & h =
        * (sorted_dispatcher_header *)
	& program()[ID];
    h.type = SORTED_DISPATCHER;
    h.elements = 1;
    h.max_elements = breakpoints + 1;
    memset ( & h + 1, 0,
               sizeof ( uns32 )
	     * (   length
	         - sorted_dispatcher_header_length ) );
    return ID;
}

uns32 LLLEX::create_sorted_dispatcher
	( uns32 atom_table_ID, uns32 breakpoints )
{
    atom_table_header & ath =
        * (atom_table_header *)
	& program()[atom_table_ID];
    assert ( ath.dispatcher == 0 );
    ath.dispatcher =
        create_sorted_dispatcher ( breakpoints );
    return ath.dispatcher;
}

// Shift the elements of the sorted dispatcher up in
// memory, beginning with the element at address
// program()[p] and producing space for n new
// elements beginning at that address.  Return 1 if
// this succeeds and 0 if for the dispatcher
// elements + n > max_elements.
//
static void upshift_sorted_dispatcher
	( uns32 sorted_dispatcher_ID,
	  uns32 p, uns32 n )
{
    sorted_dispatcher_header & h =
        * (sorted_dispatcher_header *)
	& program()[sorted_dispatcher_ID];
    assert ( h.elements + n <= h.max_elements );
    uns32 endp = sorted_dispatcher_ID
               + sorted_dispatcher_header_length
               +   sorted_dispatcher_element_length
	         * h.elements;
    memcpy ( & program()
                 [p + n
		    * sorted_dispatcher_element_length],
	     & program()[p],
	       sizeof ( uns32 )
	     * ( endp - p ) );
    return 0;
}


static const char * set_sorted_dispatcher
	( uns32 sorted_dispatcher_ID,
	  uns32 cmin, uns32 cmax,
	  uns32 value_ID )
{
    bool value_is_instruction =
        ( program()[value_ID] == INSTRUCTION );

    sorted_dispatcher_header & h =
        * (sorted_dispatcher_header *)
	& program()[sorted_dispatcher_ID];
    assert ( h.type == SORTED_DISPATCHER );
    uns32 beginp = sorted_dispatcher_ID
            + sorted_dispatcher_header_length;
    uns32 endp = beginp
               +   sorted_dispatcher_element_length
	         * h.elements;
    uns32 p = beginp;
    uns32 nextp;
    while ( true )
    {
        nextp =
	    p + sorted_dispatcher_element_length;
	if ( nextp >= endp ) break;
	sorted_dispatcher_element & nexte =
	    * (sorted_dispatcher_element *)
	    & program()[nextp];
	if ( nexte.cmin > cmin ) break;
	p = nextp;
    }
    sorted_dispatcher_element * ep =
	(sorted_dispatcher_element *)
	& program()[p];
    sorted_dispatcher_element * nextep =
	(sorted_dispatcher_element *)
	& program()[nextp];
    if ( value_is_instruction ?
	 ep->instruction != 0 :
	 ep->dispatchers != 0 )
	return 0;

    uns32 n = 2
    if ( e->cmin == cmin ) -- n;
    bool split_next = nextp == endp ?
                      cmax != (uns32) -1 :
		      cmax != nextp->cmin;
    if ( ! split_next ) -- n;
    if ( h.elements + n > h.max_elements )
	return 0;
    if ( nextp != endp && n != 0 )
        upshift_sorted_dispatcher
	    ( sorted_dispatcher_ID, p, n );
    if ( ep->cmin < cmin )
    {
	nextep->cmin = cmin;
	nextep->instruction = ep->instruction;
	nextep->dispatcher  = ep->dispatcher;
	ep = nextep;
	nextep =
	    (sorted_dispatcher_element *)
	    & program()
	    [  nextp
	     + sorted_dispatcher_element_length];
    }

    if ( split_next )
    {
	nextep->cmin = cmax + 1;
	nextep->instruction = ep->instruction;
	nextep->dispatcher  = ep->dispatcher;
    }

    h.elements += n;

    if ( value_is_instruction )
        ep->instruction = value_ID;
    else
        ep->dispatcher = value_ID;
}


uns32 LLLEX::new_type_table ( uns32 size )
{
    uns32 length = ( size + 3 ) / 4;
    uns32 ID = allocate_to_program
    	( length + type_table_header_length );
    type_table_header & h =
        * (type_table_header) & program[ID];
    h.type = TYPE_TABLE;
    h.length = length;
    memset ( & h + 1, 4 * length );
    return ID;
}

void LLLEX::set_type_table
     ( uns32 ID, uns32 n1, uns32 n2, uns8 t )
{
    type_table_header & h =
        * (type_table_header) & program[ID];
    assert ( h.type == TYPE_TABLE );
    uns8 * p = (uns8 *) ( & h + 1 );
    while ( n1 <= n2 )
    {
        assert ( n1 < h.length );
	p[n1] = t;
	++ n1;
    }
}

// Ditto but attach to another sorted dispatcher in
// the range min_character .. max_character.
//
uns32 LLLEX::create_sorted_dispatcher
	( uns32 sorted_dispatcher_ID,
	  uns32 min_character, uns32 max_character,
	  uns32 n );

// Ditto but attach to a type dispatcher at type t.
//
uns32 LLLEX::create_sorted_dispatcher
	( uns32 type_dispatcher_ID,
	  uns8 t, uns32 n );

// Create a type dispatcher and attach it to a
// sorted dispatcher in the range min_character ..
// max_character.  The type dispatcher is to permit
// types in the range 0 .. t-1.  Return new type
// dispatcher ID.
//
uns32 LLLEX::create_type_dispatcher
	( uns32 sorted_dispatcher_ID,
	  uns32 min_character, uns32 max_character,
	  uns32 t );

// Instruction operations:
//
enum {
    ACCEPT		= 1,
    DISCARD		= 2,
    KEEP		= 3
};

// Attach instruction to sorted dispatcher in the
// range min_character .. max_character.  The
// instruction has the given operation operation.
// If it has a non-zero goto_atom_table, that is
// the atom table changed to by the instruction.
// If it has a non-zero truncation_size, the
// atom is truncated to the given number of
// characters before anything else is done.
// If the instruction has a non-NULL translation
// the atom is replaced by the translation after
// truncation and before anything else is done.
//
uns32 LLLEX::create_instruction
	( uns32 sorted_dispatcher_ID,
	  uns32 min_character, uns32 max_character,
	  uns8 operation,
	  uns32 goto_atom_table = 0,
	  uns32 truncation_size = 0,
	  uns32 * translation = NULL,
	  uns32 translation_size = 0 );

// Ditto but attach to type dispatcher at type t.
//
uns32 LLLEX::create_instruction
	( uns32 type_dispatcher_ID,
	  uns32 t,
	  uns8 operation,
	  uns32 goto_atom_table = 0,
	  uns32 truncation_size = 0,
	  uns32 * translation = NULL,
	  uns32 translation_size = 0 );





    // Create a sorted dispatcher with n breakpoints and
    // attach it to the atom table with the given table
    // ID.  Return new sorted dispatcher ID.
    //
    uns32 create_sorted_dispatcher
	    ( uns32 atom_table_ID, uns32 n );

    // Ditto but attach to another sorted dispatcher in
    // the range cmin .. cmax.
    //
    uns32 create_sorted_dispatcher
	    ( uns32 sorted_dispatcher_ID,
	      uns32 cmin, uns32 cmax,
	      uns32 n );

    // Ditto but attach to a type dispatcher at type t.
    //
    uns32 create_sorted_dispatcher
	    ( uns32 type_dispatcher_ID,
	      uns8 t, uns32 n );

    // Create a type table for characters in the range
    // cmin .. cmax and types in the
    // range 0 .. tsize-1.  Return the type table ID.
    //
    uns32 new_type_table ( uns32 size );
	    ( uns32 cmin, uns32 cmax,
	      uns32 tsize );

    // Set type table entries for characters cmin ..
    // cmax to type t.
    //
    void set_type_table
	     ( uns32 type_table_ID,
	       uns32 cmin, uns32 cmax, uns8 t );

    // Create a type dispatcher and attach it to a
    // sorted dispatcher in the range cmin ..
    // cmax.  The type dispatcher uses the
    // type table with the given ID.  Return new type
    // dispatcher ID.
    //
    uns32 create_type_dispatcher
	    ( uns32 sorted_dispatcher_ID,
	      uns32 cmin, uns32 cmax,
	      uns32 type_table_ID );

    // Instruction operations:
    //
    enum {
    	ACCEPT		= 1,
	DISCARD		= 2,
	KEEP		= 3
    };

    // Attach instruction to sorted dispatcher in the
    // range cmin .. cmax.  The
    // instruction has the given operation operation.
    // If it has a non-zero goto_atom_table, that is
    // the atom table changed to by the instruction.
    // If it has a non-zero truncation_size, the
    // atom is truncated to the given number of
    // characters before anything else is done.
    // If the instruction has a non-NULL translation
    // the atom is replaced by the translation after
    // truncation and before anything else is done.
    //
    uns32 create_instruction
	    ( uns32 sorted_dispatcher_ID,
	      uns32 cmin, uns32 cmax,
	      uns8 operation,
	      uns32 goto_atom_table = 0,
	      uns32 truncation_size = 0,
	      uns32 * translation = NULL,
	      uns32 translation_size = 0 );

    // Ditto but attach to type dispatcher at type t.
    //
    uns32 create_instruction
	    ( uns32 type_dispatcher_ID,
	      uns32 t,
	      uns8 operation,
	      uns32 goto_atom_table = 0,
	      uns32 truncation_size = 0,
	      uns32 * translation = NULL,
	      uns32 translation_size = 0 );


# endif // LL_LEXEME_H
