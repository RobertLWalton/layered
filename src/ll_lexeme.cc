// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sat Apr 10 20:07:06 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/11 00:08:01 $
//   $RCSfile: ll_lexeme.cc,v $
//   $Revision: 1.8 $

// Table of Contents
//
//	Usage and Setup
//	Data Definitions
//	Program Creation

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_program_data.h>
# include <iostream>
# include <iomanip>
# include <cstdlib>
# include <cstring>
# include <cassert>
# define LLLEX ll::lexeme
using std::cout;
using std::endl;
using std::setw;
using LLLEX::uns8;
using LLLEX::uns32;
using namespace LLLEX::program_data;


// Program Creation
// ------- --------

uns32 LLLEX::create_atom_table
	( uns8 mode, uns32 label )
{
    uns32 ID = program.allocate
    		   ( atom_table_header_length );
    atom_table_header & h =
        * (atom_table_header *) & program[ID];
    h.type = ATOM_TABLE;
    h.mode = mode;
    h.label = label;
    h.dispatcher_ID = 0;
    h.instruction_ID = 0;
    return ID;
}

uns32 LLLEX::create_program ( void )
{
    program.resize ( 0 );
    uns32 ID = program.allocate
    		   ( program_header_length );
    assert ( ID == 0 );

    program_header & h =
        * (program_header *) & program[ID];
    h.type = PROGRAM;
    h.atom_table_ID = create_atom_table ( MASTER, 0 );
    return h.atom_table_ID;
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
    uns32 ID = program.allocate ( length );
    dispatcher_header & h =
        * (dispatcher_header *)
	& program[ID];
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
    uns32 ID = program.allocate
    	(   type_map_header_length
	  + ( length + 3 ) / 4 );
    type_map_header & h =
        * (type_map_header *) & program[ID];
    h.type = TYPE_MAP;
    h.cmin = cmin;
    h.cmax = cmax;
    h.singleton_type = 0;
    memcpy ( & h + 1, map, length );
    return ID;
}

uns32 LLLEX::create_type_map
	( uns32 cmin, uns32 cmax, uns32 type )
{
    uns32 length = cmin - cmax + 1;
    uns32 ID = program.allocate
    	( type_map_header_length );
    type_map_header & h =
        * (type_map_header *) & program[ID];
    h.type = TYPE_MAP;
    h.cmin = cmin;
    h.cmax = cmax;
    assert ( type != 0 );
    h.singleton_type = type;
    return ID;
}

uns32 LLLEX::create_instruction
	( uns32 operation,
	  uns32 atom_table_ID,
	  uns32 * translation_vector )
{
    assert ( ( operation & ( ACCEPT + DISCARD ) )
             !=
	     ( ACCEPT + DISCARD ) );

    assert ( ( operation & ( GOTO + SHORTCUT ) )
             !=
	     ( GOTO + SHORTCUT ) );

    assert ( ( ( operation & TRANSLATE_FLAG ) != 0 )
	     +
             ( ( operation & TRANSLATE_HEX_FLAG ) != 0 )
	     +
             ( ( operation & TRANSLATE_OCT_FLAG ) != 0 )
	     <= 1 );

    if ( operation & ( GOTO + SHORTCUT ) )
        assert ( atom_table_ID != 0 );
    else
        assert ( atom_table_ID == 0 );

    uns32 translation_length = 0;
    if ( operation & TRANSLATE_FLAG )
        translation_length =
	      ( operation >> TRANSLATE_LENGTH_SHIFT )
	    & TRANSLATE_LENGTH_MASK;
    else assert ( translation_vector == NULL );
        
    uns32 ID = program.allocate
    	(   instruction_header_length
	  + translation_length );
    instruction_header & h =
        * (instruction_header *) & program[ID];
    h.type = INSTRUCTION;
    h.operation = operation;
    h.atom_table_ID = atom_table_ID;
    if ( translation_length > 0 )
    {
	assert ( translation_vector != NULL );
	uns32 * p = (uns32 *) ( & h + 1 );
	while ( translation_length -- )
	    * p ++ = * translation_vector ++;
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
	& LLLEX::program[dispatcher_ID];
    assert ( dh.type == DISPATCHER );
    type_map_header & mh =
        * (type_map_header *)
	& LLLEX::program[type_map_ID];
    assert ( mh.type == TYPE_MAP );

    uns32 beginp = dispatcher_ID
                 + dispatcher_header_length;
    uns32 endp = beginp
               +   break_element_length
	         * dh.break_elements;
    uns32 p = beginp;
    uns32 nextp;
    while ( true )
    {
        nextp =
	    p + break_element_length;
	if ( nextp >= endp ) break;
	break_element & nexte =
	    * (break_element *)
	    & LLLEX::program[nextp];
	if ( nexte.cmin > mh.cmin ) break;
	p = nextp;
    }

    break_element * bep =
	(break_element *)
	& LLLEX::program[p];
    break_element * nextbep =
        nextp == endp ? NULL :
	(break_element *)
	& LLLEX::program[nextp];

    bool split_next = nextp == endp ?
                      mh.cmax != (uns32) -1 :
		      mh.cmax != nextbep->cmin;

    if ( bep->type_map_ID != 0 )
        return 0;

    if ( nextp != endp
         &&
	 nextbep->cmin < mh.cmin )
        return 0;

    uns32 n = 2;
    if ( bep->cmin == mh.cmin ) -- n;
    if ( ! split_next ) -- n;
    if ( dh.break_elements + n > dh.max_break_elements )
	return 0;

    if ( nextp != endp && n != 0 )
        memmove ( bep + n, bep,
	          break_element_length * ( endp - p ) );
    if ( bep->cmin < mh.cmin )
    {
	nextbep->cmin = mh.cmin;
	nextbep->type_map_ID  = 0;
	bep = nextbep;
	nextbep =
	    (break_element *)
	    & LLLEX::program [  nextp
	                      + break_element_length];
    }

    if ( split_next )
    {
	nextbep->cmin = mh.cmax + 1;
	nextbep->type_map_ID  = 0;
    }

    dh.break_elements += n;
    bep->type_map_ID = type_map_ID;
    return 1;
}

uns32 LLLEX::attach
	( uns32 target_ID,
	  uns32 item_ID )
{
    uns32 target_type = program[target_ID];
    uns32 item_type = program[item_ID];

    if ( target_type == ATOM_TABLE )
    {
	atom_table_header & h =
	    * (atom_table_header *)
	    & program[target_ID];

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
	else assert ( ! "bad attach arguments" );
    }
    else if ( target_type == DISPATCHER
              &&
	      item_type == TYPE_MAP )
        return attach_type_map_to_dispatcher
		   ( target_ID, item_ID );
    else
	assert ( ! "bad attach arguments" );
}

uns32 LLLEX::attach
    	    ( uns32 target_ID,
    	      uns32 item_ID,
	      uns32 t )
{
    dispatcher_header & h =
        * (dispatcher_header *)
	& program[target_ID];
    assert ( h.type == DISPATCHER );

    uns32 item_type = program[item_ID];
    assert ( item_type == DISPATCHER
             ||
	     item_type == INSTRUCTION );
    assert ( t <= h.max_type );
    map_element & me =
        * (map_element *)
	& program[  target_ID
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
    else
	assert ( ! "bad attach arguments" );
}

void LLLEX::print_program ( std::ostream & out )
{
    uns32 ID = 0;
    bool error = false;
    char buffer[100];
    while ( ! error && ID < program.length )
    {
	cout << setw ( 11 ) << ID << ": ";

        switch ( program[ID] )
	{
	case PROGRAM:
	{
	    cout << "PROGRAM" << endl;
	    program_header & h =
	        * (program_header *) & program[ID];
	    cout << "            Atom Table ID: "
	         << h.atom_table_ID << endl;
	    ID += program_header_length;
	    break;
	}
	case ATOM_TABLE:
	{
	    cout << "ATOM_TABLE" << endl;
	    atom_table_header & h =
	        * (atom_table_header *) & program[ID];
	    cout << "            Mode: ";
	    switch ( h.mode )
	    {
	    case LEXEME:
	        cout << "LEXEME"; break;
	    case WHITESPACE:
	        cout << "WHITESPACE"; break;
	    case ERROR:
	        cout << "ERROR"; break;
	    case MASTER:
	        cout << "MASTER"; break;
	    case CONTINUATION:
	        cout << "CONTINUATION"; break;
	    default:
	    	cout << "ILLEGAL (" << h.mode << ")";
	    }
	    cout << endl;
	    cout << "            Label: "
	         << h.label << endl;
	    cout << "            Dispatcher ID: "
	         << h.dispatcher_ID << endl;
	    cout << "            Instruction ID: "
	         << h.instruction_ID << endl;
	    ID += atom_table_header_length;
	    break;
	}
	case DISPATCHER:
	{
	    cout << "DISPATCHER" << endl;
	    dispatcher_header & h =
	        * (dispatcher_header *) & program[ID];
	    cout << "            Break Elements: "
	         << h.break_elements << endl;
	    cout << "            Max Break Elements: "
	         << h.max_break_elements << endl;
	    cout << "            Max Type: "
	         << h.max_type << endl;
	    cout << "     Breaks:"
	         << "        cmin"
	         << " type_map_ID"
	         << endl;
	    ID += dispatcher_header_length;
	    uns32 p, n;
	    for ( p = ID, n = 0;
	          n < h.break_elements;
		  p += break_element_length, ++ n )
	    {
		break_element & be =
		    * (break_element *) & program[ID];
		cout << "            ";
		if ( 33 <= be.cmin && be.cmin <= 126 )
		    cout << (sprintf ( buffer,
		                       "         %c -",
			               (char ) be.cmin ),
			     buffer);
		if ( be.cmin <= 0xFFFF )
		    cout << (sprintf ( buffer,
		                       "    %#04X -",
			               be.cmin ),
			     buffer);
		else
		    cout << (sprintf ( buffer,
		                       "%#08X -",
			               be.cmin ),
			     buffer);
		cout << setw ( 12 ) << be.type_map_ID
		     << endl;
	    }
	    ID += break_element_length
	        * h.max_break_elements;
	    cout << "     Map:"
	         << "type"
	         << "   dispatcher_ID"
	         << "  instruction_ID"
	         << endl;
	    uns32 t;
	    for ( p = ID, t = 0;
	          t <= h.max_type;
		  p += map_element_length, ++ n )
	    {
		map_element & me =
		    * (map_element *) & program[ID];
		cout << "            "
		     << setw ( 4 ) << t
		     << setw ( 16 )
		     << me.dispatcher_ID
		     << setw ( 16 )
		     << me.instruction_ID
		     << endl;
	    }
	    ID += map_element_length
	        * ( h.max_type + 1 );
	    break;
	}

	}
    }
    
}

void LLLEX::convert_program_endianhood ( void )
{
}
