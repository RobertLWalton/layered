// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Mon Apr 12 04:51:04 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/12 10:56:40 $
//   $RCSfile: ll_lexeme.cc,v $
//   $Revision: 1.12 $

// Table of Contents
//
//	Usage and Setup
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

char LLLEX::error_message[1000];

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

    uns32 translate_length = 0;
    if ( operation & TRANSLATE_FLAG )
        translate_length =
	    LLLEX::translate_length ( operation );
    else assert ( translation_vector == NULL );
        
    uns32 ID = program.allocate
    	(   instruction_header_length
	  + translate_length );
    instruction_header & h =
        * (instruction_header *) & program[ID];
    h.type = INSTRUCTION;
    h.operation = operation;
    h.atom_table_ID = atom_table_ID;
    if ( translate_length > 0 )
    {
	assert ( translation_vector != NULL );
	uns32 * p = (uns32 *) ( & h + 1 );
	while ( translate_length -- )
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
	(break_element *)
	& LLLEX::program[nextp];

    bool split_next = nextp == endp ?
                      mh.cmax != (uns32) -1 :
		      mh.cmax != nextbep->cmin;

    if ( bep->type_map_ID != 0 )
    {
        sprintf ( error_message,
	          "Attempt to attach type map %d"
		  " to dispatcher %d\n"
		  "conflicts with previous attachment"
		  " of type map %d",
		  type_map_ID, dispatcher_ID,
		  bep->type_map_ID );
        return 0;
    }

    if ( nextp != endp
         &&
	 nextbep->cmin < mh.cmin )
    {
        assert ( bep->type_map_ID != 0 );
        sprintf ( error_message,
	          "Attempt to attach type map %d"
		  " to dispatcher %d\n"
		  "conflicts with previous attachment"
		  " of type map %d",
		  type_map_ID, dispatcher_ID,
		  nextbep->type_map_ID );
        return 0;
    }

    uns32 n = 2; // Number of new break elements needed.
    if ( bep->cmin == mh.cmin ) -- n;
    if ( ! split_next ) -- n;
    if ( dh.break_elements + n > dh.max_break_elements )
    {
        sprintf ( error_message,
	          "Attempt to attach type map %d"
		  " to dispatcher %d\n"
		  "fails because dispatcher already has"
		  " too many breaks",
		  type_map_ID, dispatcher_ID );
	return 0;
    }

    if ( nextp != endp && n != 0 )
        memmove ( bep + n, bep,
	            break_element_length
		  * ( endp - p )
		  * sizeof ( uns32 ) );
    if ( bep->cmin < mh.cmin )
    {
	nextbep->cmin = mh.cmin;
	nextbep->type_map_ID  = 0;
	p = nextp;
	nextp += break_element_length;
	bep = nextbep;
	nextbep =
	    (break_element *)
	    & LLLEX::program [nextp];
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
	    {
		sprintf ( error_message,
			  "Attempt to attach dispatcher"
			  " %d to atom table %d\n"
			  "conflicts with previous"
			  " attachment of dispatcher"
			  " %d",
			  item_ID, target_ID,
			  h.dispatcher_ID );
	        return 0;
	    }
	    h.dispatcher_ID = item_ID;
	    return 1;
	}
        else if ( item_type == INSTRUCTION )
	{
	    if ( h.instruction_ID != 0 )
	    {
		sprintf ( error_message,
			  "Attempt to attach"
			  " instruction %d to atom"
			  " table %d\n"
			  "conflicts with previous"
			  " attachment of instruction"
			  " %d",
			  item_ID, target_ID,
			  h.instruction_ID );
	        return 0;
	    }
	    h.instruction_ID = item_ID;
	    return 1;
	}
	else assert ( ! "bad attach item types" );
    }
    else if ( target_type == DISPATCHER
              &&
	      item_type == TYPE_MAP )
        return attach_type_map_to_dispatcher
		   ( target_ID, item_ID );
    else
	assert ( ! "bad attach item types" );
}

uns32 LLLEX::attach
    	    ( uns32 target_ID,
    	      uns32 t,
	      uns32 item_ID )
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
	{
	    sprintf ( error_message,
		      "Attempt to attach dispatcher"
		      " %d to dispatcher %d type %d\n"
		      "conflicts with previous"
		      " attachment of dispatcher"
		      " %d",
		      item_ID, target_ID, t,
		      me.dispatcher_ID );
	    return 0;
	}
	me.dispatcher_ID = item_ID;
	return 1;
    }
    else if ( item_type == INSTRUCTION )
    {
	if ( me.instruction_ID != 0 )
	{
	    sprintf ( error_message,
		      "Attempt to attach instruction"
		      " %d to dispatcher %d type %d\n"
		      "conflicts with previous"
		      " attachment of instruction"
		      " %d",
		      item_ID, target_ID, t,
		      me.instruction_ID );
	    return 0;
	}
	me.instruction_ID = item_ID;
	return 1;
    }
    else
	assert ( ! "assert failure" );
}

// cout << pchar ( uns32 c, width, {LEFT,RIGHT} )
// outputs a character c either with no padding if
// width == 0 or with padding and the indicated adjust-
// ment (default RIGHT).  Format is 1 character for
// printable non-space ASCII characters, \uXXXX for
// other characters <= 0xFFFF, and \UXXXXXXXX
// otherwise.
//
// columnschar ( c ) is the number of columns taken by
// pchar when width == 0.
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

static ostream & operator <<
	( ostream & s, const pchar & pc )
{
    char buffer[20];
    if ( 33 <= pc.c && pc.c <= 126 )
        sprintf ( buffer, "%c", (char) pc.c );
    else if ( pc.c <= 0xFFFF )
        sprintf ( buffer, "\\u%04X", pc.c );
    else
        sprintf ( buffer, "\\U%08X", pc.c );

    if ( pc.width > 0 && pc.left_adjust )
        s << setiosflags ( ios::left );
    if ( pc.width > 0 ) s << setw ( pc.width );
    s << buffer;
    if ( pc.width > 0 && pc.left_adjust )
        s << resetiosflags ( ios::left );
    return s;
}

inline unsigned columnschar ( uns32 c )
{
    return ( 33 <= c && c <= 126 ) ? 1 :
           ( c <= 0xFFFF ) ? 6 : 10;
}

const unsigned IDwidth = 12;
    // Width of field containing ID at the beginning
    // of each print_program line.

// cout << pID ( ID ) prints `ID: ' right adjusted in
// a field of width IDwidth.
//
struct pID { uns32 ID;
             pID ( uns32 ID ) : ID ( ID ) {} };
inline ostream & operator <<
	( ostream & out, const pID & p )
{
    return out << setw ( IDwidth - 2 ) << p.ID << ": ";
}

// cout << INDENT prints a blank field of width IDwidth.
//
#define INDENT setw ( IDwidth ) << ""

// cout << pmode ( mode ) prints a mode.
//
struct pmode { uns32 mode;
             pmode ( uns32 mode ) : mode ( mode ) {} };
inline ostream & operator <<
	( ostream & out, const pmode & p )
{
    switch ( p.mode )
    {
    case LEXEME:
	return out << "LEXEME";
    case WHITESPACE:
	return out << "WHITESPACE";
    case ERROR:
	return out << "ERROR";
    case MASTER:
	return out << "MASTER";
    case CONTINUATION:
	return out << "CONTINUATION";
    default:
	return out << "ILLEGAL MODE ("
	           << p.mode << ")";
    }
}



// Print the instruction at program[ID] with the given
// indent and endl, if ID is non-zero, and return ID
// repositioned just after instuction.  However, if
// program[ID] does not == INSTRUCTION, print ILLEGAL
// instruction message and return 0.
//
// If ID == 0 do nothing but return 0.
//
static uns32 print_instruction
    ( uns32 ID, unsigned indent = IDwidth )
{
    if ( ID == 0 ) return 0;
    if ( indent > 0 ) cout << setw ( indent ) << "";

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
    else
    if ( ( h.operation & ( GOTO + SHORTCUT ) )
         &&
	 h.atom_table_ID == 0 ) cout << "ILLEGAL: ";
    else
    if ( ( h.operation & ( GOTO + SHORTCUT ) ) == 0
         &&
	 h.atom_table_ID != 0 ) cout << "ILLEGAL: ";

    switch ( h.operation & ( ACCEPT + DISCARD ) )
    {
    case ACCEPT: cout << "ACCEPT"; break;
    case DISCARD: cout << "DISCARD"; break;
    case 0: cout << "KEEP"; break;
    default: cout << "ACCEPT, DISCARD"; break;
    }

    if ( h.operation & TRUNCATE_FLAG )
        cout << ", TRUNCATE("
	     << LLLEX::truncate_length ( h.operation )
	     << ")";
    if ( h.operation & TRANSLATE_FLAG )
    {
        translate_length =
	     LLLEX::translate_length ( h.operation );
        cout << ", TRANSLATE(" << translate_length;
	if ( translate_length > 0 )
	{
	    cout << ",";
	    uns32 n = translate_length;
	    for ( uns32 p =
	            ID + instruction_header_length;
		  0 < n; ++ p, -- n )
	        cout << pchar ( program[p] );
	}
	cout << ")";
    }
    if ( h.operation & TRANSLATE_HEX_FLAG )
        cout << ", TRANSLATE_HEX("
	     << LLLEX::prefix_length ( h.operation )
	     << ","
	     << LLLEX::postfix_length ( h.operation )
	     << ")";
    if ( h.operation & TRANSLATE_OCT_FLAG )
        cout << ", TRANSLATE_OCT("
	     << LLLEX::prefix_length ( h.operation )
	     << ","
	     << LLLEX::postfix_length ( h.operation )
	     << ")";
    if ( h.operation & GOTO )
        cout << ", GOTO(" << h.atom_table_ID << ")";
    if ( h.operation & SHORTCUT )
        cout << ", SHORTCUT(" << h.atom_table_ID << ")";
    cout << endl;

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
    char buffer[100];
    while ( ID < program.length )
    {
        switch ( program[ID] )
	{
	case PROGRAM:
	{
	    cout << pID ( ID ) << "PROGRAM" << endl;
	    program_header & h =
	        * (program_header *) & program[ID];
	    cout << INDENT << "Atom Table ID: "
	         << h.atom_table_ID << endl;
	    ID += program_header_length;
	    break;
	}
	case ATOM_TABLE:
	{
	    cout << pID ( ID ) << "ATOM_TABLE" << endl;
	    atom_table_header & h =
	        * (atom_table_header *) & program[ID];
	    cout << INDENT << "Mode: "
	         << pmode ( h.mode ) << endl;
	    cout << INDENT << "Label: "
	         << h.label << endl;
	    cout << INDENT << "Dispatcher ID: "
	         << h.dispatcher_ID << endl;
	    if ( cooked )
	        print_instruction
		    ( h.instruction_ID );
	    else
		cout << INDENT << "Instruction ID: "
		     << h.instruction_ID << endl;
	    ID += atom_table_header_length;
	    break;
	}
	case DISPATCHER:
	    cout << pID ( ID ) << "DISPATCHER" << endl;
	if ( cooked )
	    ID = print_cooked_dispatcher ( ID );
	else
	{
	    dispatcher_header & h =
	        * (dispatcher_header *) & program[ID];
	    cout << INDENT << "Break Elements: "
	         << h.break_elements << endl;
	    cout << INDENT << "Max Break Elements: "
	         << h.max_break_elements << endl;
	    cout << INDENT << "Max Type: "
	         << h.max_type << endl;
	    cout << INDENT << "Breaks: "
	         << setw ( 16 ) << "cmin"
	         << setw ( 16 ) << "type_map_ID"
	         << endl;
	    ID += dispatcher_header_length;
	    uns32 p, n;
	    for ( p = ID, n = 0;
	          n < h.break_elements;
		  p += break_element_length, ++ n )
	    {
		break_element & be =
		    * (break_element *) & program[p];
		cout << INDENT
		     << pchar ( be.cmin, 24 )
		     << setw ( 16 ) << be.type_map_ID
		     << endl;
	    }
	    ID += break_element_length
	        * h.max_break_elements;
	    cout << INDENT << "Map:    type: "
	         << setw ( 16 ) << "dispatcher_ID"
	         << setw ( 16 ) << "instruction_ID"
	         << endl;
	    uns32 t;
	    for ( p = ID, t = 0;
	          t <= h.max_type;
		  p += map_element_length, ++ t )
	    {
		map_element & me =
		    * (map_element *) & program[p];
		cout << INDENT
		     << setw ( 12 ) << t << ": "
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
		cout << pID ( ID );
		ID = print_instruction ( ID, 0 );
	    }
	    else
	    {
		instruction_header & h =
		    * (instruction_header *)
		    & program[ID];
	        ID += instruction_header_length;
		if (   h.operation
		     & LLLEX::TRANSLATE_FLAG )
		    ID += LLLEX::translate_length
		   		( h.operation );
	    }
	    break;
	case TYPE_MAP:
	if ( ! cooked )
	{
	    cout << pID ( ID ) << "TYPE_MAP" << endl;
	    type_map_header & h =
	        * (type_map_header *) & program[ID];
	    ID += type_map_header_length;
	    if ( h.singleton_type > 0 )
	        cout << setw ( IDwidth + 4 )
		     << h.singleton_type
		     << ": " << pchar ( h.cmin )
		     << "-" << pchar ( h.cmax ) << endl;
	    else
	    {
		uns8 * map = (uns8 *) ( & h + 1 );
		ID += ( h.cmax - h.cmin + 4 ) / 4;
		for ( unsigned t = 0; t < 256; ++ t )
		{
		    bool found = false;
		    uns32 range = 0;
		    unsigned columns = 72 - IDwidth - 6;
		    for ( uns32 c = h.cmin;
			  c <= h.cmax; ++ c )
		    {
			if ( map[c - h.cmin] == t )
			{
			    if ( ! found )
			        cout << setw ( IDwidth
				               + 4 )
				     << t << ": ";
			    if ( range == 0 )
			    {
				if ( columns
				     <
				       columnschar ( c )
				     + 4
				       // allows for
				       // ", " and "-"
				       // and line
				       // ending ","
				     + columnschar
				         ( 0xFFFFFFFF )
				   )
				{
				    assert
				      ( columns >= 0 );
				    cout << "," << endl
				         << setw
					      ( IDwidth
					        + 6 )
					 << "";
				    columns = 72
				            - IDwidth
					    - 6;
				}
			        else if ( found )
				{
				    cout << ", ";
				    columns -= 2;
				}
			        cout << pchar ( c );
				columns -=
				    columnschar ( c );
			    }
			    found = true;
			    ++ range;
			}
			else
			{
			    if ( range > 1 )
			    {
			        cout << "-"
				     << pchar ( c - 1 );
				columns -=
				    1 + columnschar
				            ( c - 1 );
				assert ( columns >= 0 );
			    }
			    range = 0;
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
	    cout << pID ( ID ) << "ILLEGAL ITEM TYPE("
	         << program[ID] << ")" << endl;
	    return;
	}
    }
}

void LLLEX::convert_program_endianhood ( void )
{
}
