// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sun Apr 11 21:52:12 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/12 01:56:31 $
//   $RCSfile: ll_lexeme.cc,v $
//   $Revision: 1.11 $

// Table of Contents
//
//	Usage and Setup
//	External Interface
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
using std::ios;
using std::ostream;
using namespace LLLEX;
using namespace LLLEX::program_data;

// External Interface
// -------- ---------

template < typename T >
uns32 LLLEX::buffer<T>::allocate ( uns32 n )
{
    uns32 available = max_length - length;
    if ( available < n )
    {
        uns32 new_max_length =
	    max_length + n - available
	               + length_increment;
	resize ( new_max_length );
    }
    uns32 location = length;
    length += n;
    return location;
}
template 
uns32 LLLEX::buffer<uns32>::allocate ( uns32 n );
template 
uns32 LLLEX::buffer<inchar>::allocate ( uns32 n );

template < typename T >
uns32 LLLEX::buffer<T>::deallocate ( uns32 n )
{
    assert ( length >= n );
    length -= n;
}
template 
uns32 LLLEX::buffer<uns32>::deallocate ( uns32 n );
template 
uns32 LLLEX::buffer<inchar>::deallocate ( uns32 n );

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
    assert ( cmax >= cmin );
    uns32 length = cmax - cmin + 1;
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
    assert ( cmax >= cmin );
    uns32 length = cmax - cmin + 1;
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

// cout << pchar ( uns32 c, width, {LEFT,RIGHT} )
// outputs a character c either with no padding if
// width == 0 or with padding and the indicated adjust-
// ment (default RIGHT).  Format is 1 character for
// printable non-space ASCII characters, \uXXXX for
// other characters <= 0xFFFF, and \UXXXXXXXX
// otherwise.
//
enum { LEFT = 1, RIGHT = 0 };
struct pchar
{
    uns32 c;
    unsigned width;
    bool left_adjust;
    pchar ( uns32 c,
            unsigned width = 0,
	    bool left_adjust = RIGHT ) :
        c ( c ), width ( width ),
	left_adjust ( left_adjust ) {}
};

static ostream & operator << ( ostream & s, const pchar & pc )
{
    char buffer[20];
    if ( 33 <= pc.c && pc.c <= 126 )
        sprintf ( buffer, "%c", (char) pc.c );
    else if ( pc.c <= 0xFFFF )
        sprintf ( buffer, "\\\\u%04X", pc.c );
    else
        sprintf ( buffer, "\\\\U%08X", pc.c );

    if ( pc.width > 0 && pc.left_adjust )
        s << setiosflags ( ios::left );
    if ( pc.width > 0 ) s << setw ( pc.width );
    s << buffer;
    if ( pc.width > 0 && pc.left_adjust )
        s << resetiosflags ( ios::left );
    return s;
}

// Print the instruction at program[ID] with the given
// indent, if ID is non-zero.  Otherwise do nothing.
// If ID is non-zero, return ID repositioned just after
// instuction (else return 0).  However, if program[ID]
// does not == INSTRUCTION, print ILLEGAL instruction
// message and return 0xFFFFFFFF.
//
static uns32 print_instruction
    ( uns32 ID, unsigned indent = 12 )
{
    if ( ID == 0 ) return 0;
    if ( indent > 0 ) cout << setw ( indent ) << " ";

    instruction_header & h =
        * (instruction_header *) & program[ID];
    uns32 translate_length = 0;

    if ( h.type != INSTRUCTION )
    {
        cout << "ILLEGAL INSTRUCTION TYPE ("
	     << h.type << ")" << endl;
	return 0xFFFFFFFF;
    }
    if ( ( ( h.operation & ACCEPT ) != 0 )
	 +
         ( ( h.operation & DISCARD ) != 0 )
	 > 1 ) cout << "ILLEGAL: ";
    else
    if ( ( ( h.operation & TRANSLATE_FLAG ) != 0 )
	 +
         ( ( h.operation & TRANSLATE_HEX_FLAG ) != 0 )
	 +
         ( ( h.operation & TRANSLATE_OCT_FLAG ) != 0 )
	 > 1 ) cout << "ILLEGAL: ";
    else
    if ( ( ( h.operation & GOTO ) != 0 )
	 +
         ( ( h.operation & SHORTCUT ) != 0 )
	 > 1 ) cout << "ILLEGAL: ";

    switch ( h.operation & ( ACCEPT + DISCARD ) )
    {
    case ACCEPT: cout << "ACCEPT"; break;
    case DISCARD: cout << "DISCARD"; break;
    case 0: cout << "KEEP"; break;
    default: cout << "ACCEPT, DISCARD"; break;
    }

    if ( h.operation & TRUNCATE_FLAG )
        cout << ", TRUNCATE("
	     << (   (    h.operation
	              >> TRUNCATE_LENGTH_SHIFT )
		  & TRUNCATE_LENGTH_MASK )
	     << ")";
    if ( h.operation & TRANSLATE_FLAG )
    {
        translate_length =
	      (    h.operation
	        >> TRANSLATE_LENGTH_SHIFT )
	    & TRANSLATE_LENGTH_MASK;
        cout << ", TRANSLATE(" << translate_length;
	if ( translate_length > 0 )
	{
	    cout << ",";
	    for ( uns32 p =
	            ID + instruction_header_length;
		  0 < translate_length;
		  ++ p, -- translate_length )
	        cout << pchar ( program[p] );
	}
	cout << ")";
    }
    if ( h.operation & TRANSLATE_HEX_FLAG )
        cout << ", TRANSLATE_HEX("
	     << (   (    h.operation
	              >> PREFIX_LENGTH_SHIFT )
		  & PREFIX_LENGTH_MASK )
		<< ","
	     << (   (    h.operation
	              >> POSTFIX_LENGTH_SHIFT )
		  & POSTFIX_LENGTH_MASK )
	     << ")";
    if ( h.operation & TRANSLATE_OCT_FLAG )
        cout << ", TRANSLATE_OCT("
	     << (   (    h.operation
	              >> PREFIX_LENGTH_SHIFT )
		  & PREFIX_LENGTH_MASK )
		<< ","
	     << (   (    h.operation
	              >> POSTFIX_LENGTH_SHIFT )
		  & POSTFIX_LENGTH_MASK )
	     << ")";
    if ( h.operation & GOTO )
        cout << ", GOTO(" << h.atom_table_ID << ")";
    if ( h.operation & SHORTCUT )
        cout << ", SHORTCUT(" << h.atom_table_ID << ")";

    return ID + instruction_header_length
              + translate_length;
}

// Print the dispatcher at program[ID] with the given
// indent, if ID is non-zero in the cooked format.
// Return ID pointing after dispatcher.  If ID is zero,
// do nothing but return 0.
//
static uns32 print_cooked_dispatcher
    ( uns32 ID, unsigned indent = 12 )
{
    if ( ID == 0 ) return 0;
}

void LLLEX::print_program
	( std::ostream & out, bool cooked )
{
    uns32 ID = 0;
    bool error = false;
    char buffer[100];
    while ( ! error && ID < program.length )
    {
        switch ( program[ID] )
	{
	case PROGRAM:
	{
	    cout << setw ( 11 ) << ID << ": "
	         << "PROGRAM" << endl;
	    program_header & h =
	        * (program_header *) & program[ID];
	    cout << "            Atom Table ID: "
	         << h.atom_table_ID << endl;
	    ID += program_header_length;
	    break;
	}
	case ATOM_TABLE:
	{
	    cout << setw ( 11 ) << ID << ": "
	         << "ATOM_TABLE" << endl;
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
	    if ( cooked )
	        print_instruction
		    ( h.instruction_ID );
	    else
		cout << "            Instruction ID: "
		     << h.instruction_ID << endl;
	    ID += atom_table_header_length;
	    break;
	}
	case DISPATCHER:
	    cout << setw ( 11 ) << ID << ": "
	         << "DISPATCHER" << endl;
	if ( cooked )
	    ID = print_cooked_dispatcher ( ID );
	else
	{
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
	case INSTRUCTION:
	    if ( ! cooked )
	    {
		cout << setw ( 11 ) << ID << ": ";
		ID = print_instruction ( ID, 0 );
	    }
	    break;
	case TYPE_MAP:
	if ( ! cooked )
	{
	    cout << setw ( 11 ) << ID << ": "
	         << "TYPE_MAP" << endl;
	    type_map_header & h =
	        * (type_map_header *) & program[ID];
	    ID += type_map_header_length;
	    if ( h.singleton_type > 0 )
	        cout << setw ( 16 ) << h.singleton_type
		     << ": " << pchar ( h.cmin )
		     << "-" << pchar ( h.cmax ) << endl;
	    else
	    {
		char * map = (char *) ( & h + 1 );
		ID += ( h.cmax - h.cmin + 4 ) / 4;
		for ( unsigned t = 0; t < 256; ++ t )
		{
		    bool found = false;
		    bool last = false;
		    for ( uns32 c = h.cmin;
			  c <= h.cmax; ++ c )
		    {
			if ( map[c - h.cmin] != t )
			{
			    if ( last )
			    {
			        cout << "-"
				     << pchar ( c - 1 );
				last = false;
			    }
			}
			else
			{
			    if ( ! found )
			        cout << setw ( 16 ) << t
				     << ": ";
			    if ( ! last )
			    {
			        if ( found ) cout << ",";
			        cout << pchar ( c );
				last = true;
			    }
			    found = true;
			}
		    }
		    if ( found ) cout << endl;
		}
	    }
        }
	else
	{
	    type_map_header & h =
	        * (type_map_header *) & program[ID];
	    ID += type_map_header_length;
	    if ( h.singleton_type == 0 )
	        ID += ( h.cmax - h.cmin + 4 ) / 4;
	}
	    break;
	default:
	    cout << "ILLEGAL ITEM TYPE("
	         << program[ID] << ")" << endl;
	    return;
	}
    }
}

void LLLEX::convert_program_endianhood ( void )
{
}
