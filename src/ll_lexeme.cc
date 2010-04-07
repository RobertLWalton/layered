// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Wed Apr  7 13:43:35 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/07 18:38:30 $
//   $RCSfile: ll_lexeme.cc,v $
//   $Revision: 1.4 $

// Table of Contents
//
//	Usage and Setup
//	Data Definitions
//	Program Creation

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

inline uns32 * & program ( void )
    { return * LLLEX::program_pointer; }

enum {
    PROGRAM			= 1,
    ATOM_TABLE			= 2,
    TYPE_MAP			= 3,
    DISPATCHER			= 4,
    INSTRUCTION			= 5
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
// is non-zero, or is mapped by the type map whose
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
// map[c-cmin] otherwise, where may is the uns8 *
// pointer to the first byte after the type map
// header.
//
struct type_map_header {
    uns32 type;
    uns32 cmin, cmax;	    // Character range.
    uns32 singleton_type;   // If 0 use vector.
};
const uns32 type_map_header_length = 4;

// Instruction.  If operation & TRANSLATE != 0,
// this is followed by translation_length uns32
// characters of the translation.
//
struct instruction_header {
    uns32 operation,
    uns32 goto_atom_table,
    uns32 truncation_length,
    uns32 translation_length
};
const uns32 instruction_header_length = 4;

// Program Creation
// ------- --------

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
    h.dispatcher_ID = 0;
    h.instruction_ID = 0;
    return ID;
}

uns32 LLLEX::create_program ( void )
{
    program_pointer = allocate_program();
    max_length = length = header_length =
	    program_length();
    uns32 ID = allocate_to_program
    		   ( program_table_header_length );
    assert ( ID == LLLEX::header_length );
    program_table_header & h =
        * (program_table_header *) & program()[ID];
    h.type = PROGRAM_TABLE;
    h.atom_table = create_atom_table ( MASTER );
    return h.atom_table;
}

uns32 LLLEX::create_dispatcher
	( uns32 max_breakpoints,
	  uns32 max_type )
{
    uns32 length =
          dispatcher_header_length
	+   break_element_length
	  * ( max_breakpoints + 1 )
	+   map_element_length
	  * ( max_type + 1 );
    uns32 ID = allocate_to_program ( length );
    dispatcher_header & h =
        * (dispatcher_header *)
	& program()[ID];
    h.type = DISPATCHER;
    h.break_elements = 1;
    h.max_break_elements = max_breakpoints + 1;
    h.max_type = max_type;

    memset ( & h + 1, 0,
               sizeof ( uns32 )
	     * (   length
	         - dispatcher_header_length ) );
    return ID;
}

uns32 LLLEX::create_type_map
	( uns32 cmin, uns32 cmax, uns8 * map )
{
    uns32 length = cmin - cmax + 1;
    uns32 ID = allocate_to_program
    	(   type_table_header_length
	  + ( length + 3 ) / 4 ) );
    type_map_header & h =
        * (type_map_header) & program[ID];
    h.type = TYPE_TABLE;
    h.cmin = cmin;
    h.cmax = cmax;
    h.singleton_type = 0;
    memcpy ( & h + 1, map, length );
    return ID;
}

uns32 LLLEX::create_type_map
	( uns32 cmin, uns32 cmax, uns8 type )
{
    uns32 length = cmin - cmax + 1;
    uns32 ID = allocate_to_program
    	( type_table_header_length );
    type_map_header & h =
        * (type_map_header) & program[ID];
    h.type = TYPE_TABLE;
    h.cmin = cmin;
    h.cmax = cmax;
    assert ( type != 0 );
    h.singleton_type = type;
    return ID;
}

uns32 LLLEX::create_instruction
	( uns32 operation,
	  uns32 goto_atom_table_ID,
	  uns32 truncation_length,
	  uns32 * translation,
	  uns32 translation_length )
{
    assert ( ( operation & ( ACCEPT + DISCARD ) )
             !=
	     ( ACCEPT + DISCARD ) );

    assert ( ( operation & ( GOTO + SINGLETON ) )
             !=
	     ( GOTO + SINGLETON ) );

    if ( operation & ( GOTO + SINGLETON ) )
        assert ( goto_atom_table_ID != 0 );
    else
        assert ( goto_atom_table_ID == 0 );
        
    if ( operation & TRANSLATE )
        assert ( translation_length == 0
	         ||
		 translation != NULL );
    else
        assert ( translation_length == 0
	         &&
		 translation == NULL );
    if ( ( operation & TRUNCATE ) == 0 )
        assert ( truncation_length == 0 );

    uns32 ID = allocate_to_program
    	(   instruction_header_length
	  + translation_length );
    instruction_header & h =
        * (instruction_header) & program[ID];
    h.type = INSTRUCTION;
    h.operation = operation;
    h.goto_atom_table_ID = goto_atom_table_ID;
    h.truncation_length = truncation_length;
    h.translation_length = translation_length;
    if ( translation_length > 0 )
    {
        uns32 * p = (uns32 *) ( & h + 1 );
	while ( translation_length -- )
	    * p ++ = * translation ++;
    }
    return ID;
}

// This function is LLLEX::attach for the difficult
// case where break elements may need to be inserted
// into the dispatcher.
//
static uns32 attach_type_map_to_dispatcher
	( uns32 dispatcher_ID,
	  uns32 type_map_ID )
{
    dispatcher_header & dh =
        * (dispatcher_header *)
	& program()[dispatcher_ID];
    assert ( dh.type == DISPATCHER );
    type_map_header & mh =
        * (type_map_header *)
	& program()[type_map_ID];
    assert ( mh.type == TYPE_MAP );

    uns32 beginp = dispatcher_ID
                 + dispatcher_header_length;
    uns32 endp = beginp
               +   break_element_length
	         * dh.elements;
    uns32 p = beginp;
    uns32 nextp;
    while ( true )
    {
        nextp =
	    p + break_element_length;
	if ( nextp >= endp ) break;
	break_element & nexte =
	    * (break_element *)
	    & program()[nextp];
	if ( nexte.cmin > mh.cmin ) break;
	p = nextp;
    }

    bool split_next = nextp == endp ?
                      mh.cmax != (uns32) -1 :
		      mh.cmax != nextp->cmin;

    break_element * bep =
	(break_element *)
	& program()[p];
    break_element * nextbep =
        nextp == endp ? NULL :
	(break_element *)
	& program()[nextp];

    if ( bep->type_map_ID != 0 )
        return 0;

    if ( nextp != endp
         &&
	 nextbep.cmin < mh.cmin )
        return 0;

    uns32 n = 2
    if ( bep->cmin == mh.cmin ) -- n;
    if ( ! split_next ) -- n;
    if ( dh.break_elements + n > dh.max_break_elements )
	return 0;

    if ( nextp != endp && n != 0 )
        memmove ( bep + n, bep,
	          break_element_length * ( endp - p ) );
    if ( bep->cmin < mp.cmin )
    {
	nextbep->cmin = mp.cmin;
	nextbep->type_map_ID  = 0;
	bep = nextbep;
	nextbep =
	    (break_element *)
	    & program()
	    [  nextp
	     + break_element_length];
    }

    if ( split_next )
    {
	nextbep->cmin = mh.cmax + 1;
	nextbep->type_map_ID  = 0;
    }

    dh.break_elements += n;
    bep->type_map_id = type_map_id;
    return 1;
}

uns32 LLLEX::attach
	( uns32 target_ID,
	  uns32 item_ID )
{
    uns32 target_type = program()[target_ID];
    uns32 item_type = program()[item_ID];

    if ( target_type == ATOM_TABLE )
    {
	atom_table_header & h =
	    * (atom_table_header *)
	    & program()[target_ID];

        if ( item_type == DISPATCHER )
	{
	    if ( h.dispatcher_ID != 0 )
	        return 0;
	    h.dispatcher_ID = item_ID;
	    return 1;
	}
        else if ( item_type == INSTRUCTION )
	{
	    if ( h.instruction_ID != 0 )
	        return 0;
	    h.instruction_ID = item_ID;
	    return 1;
	}
	else abort();
    }
    else if ( target_type == DISPATCHER
              &&
	      item_type == TYPE_MAP )
        return attach_type_map_to_dispatcher
		   ( target_ID, item_ID );
    else
        abort();
}

uns32 LLLEX::attach
    	    ( uns32 target_ID,
    	      uns32 item_ID,
	      uns32 t )
{
    dispatcher_header & h =
        * (dispatcher_header *)
	& program()[target_ID];
    assert ( h.type == DISPATCHER );

    uns32 item_type = program()[item_ID];
    assert ( item_type == DISPATCHER
             ||
	     item_type == INSTRUCTION );
    assert ( t <= h.max_type );
    map_element & me =
        * (map_element *)
	& program()[  target_ID
	            + dispatcher_header_length
		    +   break_element_length
		      * h.max_break_elements
		    +   map_element_length
		      * t];

    if ( item_type == DISPATCHER )
    {
	if ( me.dispatcher_ID != 0 )
	    return 0;
	me.dispatcher_ID = item_ID;
	return 1;
    }
    else if ( item_type == INSTRUCTION )
    {
	if ( me.instruction_ID != 0 )
	    return 0;
	me.instruction_ID = item_ID;
	return 1;
    }
    else abort();
}
