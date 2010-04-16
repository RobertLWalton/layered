// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Fri Apr 16 09:04:52 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/16 17:42:54 $
//   $RCSfile: ll_lexeme.cc,v $
//   $Revision: 1.25 $

// Table of Contents
//
//	Usage and Setup
//	Program Construction
//	Scanning
//	Printing

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_program_data.h>
# include <iostream>
# include <iomanip>
# include <cstdlib>
# include <cstring>
# include <cassert>
# define LEX ll::lexeme
using std::cout;
using std::endl;
using std::setw;
using std::ios;
using std::ostream;
using namespace LEX;
using namespace LEX::program_data;

char LEX::error_message[1000];

// Program Construction
// ------- ------------

uns32 LEX::create_atom_table
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

uns32 LEX::create_program ( void )
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

uns32 LEX::create_dispatcher
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

uns32 LEX::create_type_map
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

uns32 LEX::create_type_map
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

uns32 LEX::create_instruction
	( uns32 operation,
	  uns32 atom_table_ID,
	  uns32 * translation_vector )
{
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
	    LEX::translate_length ( operation );
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

// This function is LEX::attach for the difficult
// case where break elements may need to be inserted
// into the dispatcher.
//
static uns32 attach_type_map_to_dispatcher
	( uns32 dispatcher_ID,
	  uns32 type_map_ID )
{
    dispatcher_header & dh =
        * (dispatcher_header *)
	& LEX::program[dispatcher_ID];
    assert ( dh.type == DISPATCHER );
    type_map_header & mh =
        * (type_map_header *)
	& LEX::program[type_map_ID];
    assert ( mh.type == TYPE_MAP );

    break_element * bep =
        (break_element *)
	& LEX::program[  dispatcher_ID
                       + dispatcher_header_length ];

    uns32 i = 0;
    for ( ; i + 1 < dh.break_elements
            &&
	    bep[i+1].cmin <= mh.cmin;
	    ++ i );

    bool split_next = ( i == dh.break_elements - 1 ?
                        mh.cmax != 0xFFFFFFFF :
		        mh.cmax != bep[i+1].cmin - 1 );

    if ( bep[i].type_map_ID != 0 )
    {
        sprintf ( error_message,
	          "Attempt to attach type map %u"
		  " to dispatcher %u\n"
		  "conflicts with previous attachment"
		  " of type map %u",
		  type_map_ID, dispatcher_ID,
		  bep[i].type_map_ID );
        return 0;
    }

    if ( i + 1 != dh.break_elements
         &&
	 bep[i+1].cmin < mh.cmin )
    {
        assert ( bep[i+1].type_map_ID != 0 );
        sprintf ( error_message,
	          "Attempt to attach type map %u"
		  " to dispatcher %u\n"
		  "conflicts with previous attachment"
		  " of type map %u",
		  type_map_ID, dispatcher_ID,
		  bep[i+1].type_map_ID );
        return 0;
    }

    int n = 2; // Number of new break elements needed.
    if ( bep[i].cmin == mh.cmin ) -- n;
    if ( ! split_next ) -- n;
    if ( dh.break_elements + n > dh.max_break_elements )
    {
        sprintf ( error_message,
	          "Attempt to attach type map %u"
		  " to dispatcher %u\n"
		  "fails because dispatcher already has"
		  " too many breaks",
		  type_map_ID, dispatcher_ID );
	return 0;
    }

    if ( n != 0 )
        memmove ( & bep[i+n], & bep[i],
		    ( dh.break_elements - i )
		  * sizeof ( break_element ) );
    if ( bep[i].cmin < mh.cmin )
    {
	++ i;
	bep[i].cmin = mh.cmin;
	bep[i].type_map_ID  = 0;
    }

    if ( split_next )
    {
	bep[i+1].cmin = mh.cmax + 1;
	bep[i+1].type_map_ID  = 0;
    }

    dh.break_elements += n;
    bep[i].type_map_ID = type_map_ID;
    return 1;
}

uns32 LEX::attach
	( uns32 target_ID,
	  uns32 component_ID )
{
    uns32 target_type = program[target_ID];
    uns32 component_type = program[component_ID];

    if ( target_type == ATOM_TABLE )
    {
	atom_table_header & h =
	    * (atom_table_header *)
	    & program[target_ID];

        if ( component_type == DISPATCHER )
	{
	    if ( h.dispatcher_ID != 0 )
	    {
		sprintf ( error_message,
			  "Attempt to attach dispatcher"
			  " %u to atom table %u\n"
			  "conflicts with previous"
			  " attachment of dispatcher"
			  " %u",
			  component_ID, target_ID,
			  h.dispatcher_ID );
	        return 0;
	    }
	    h.dispatcher_ID = component_ID;
	    return 1;
	}
        else if ( component_type == INSTRUCTION )
	{
	    if ( h.instruction_ID != 0 )
	    {
		sprintf ( error_message,
			  "Attempt to attach"
			  " instruction %u to atom"
			  " table %u\n"
			  "conflicts with previous"
			  " attachment of instruction"
			  " %u",
			  component_ID, target_ID,
			  h.instruction_ID );
	        return 0;
	    }
	    h.instruction_ID = component_ID;
	    return 1;
	}
	else assert ( ! "bad attach component types" );
    }
    else if ( target_type == DISPATCHER
              &&
	      component_type == TYPE_MAP )
        return attach_type_map_to_dispatcher
		   ( target_ID, component_ID );
    else
	assert ( ! "bad attach component types" );
}

uns32 LEX::attach
    	    ( uns32 target_ID,
    	      uns32 t,
	      uns32 component_ID )
{
    dispatcher_header & h =
        * (dispatcher_header *)
	& program[target_ID];
    assert ( h.type == DISPATCHER );

    uns32 component_type = program[component_ID];
    assert ( component_type == DISPATCHER
             ||
	     component_type == INSTRUCTION );
    assert ( t <= h.max_type );
    map_element & me =
        * (map_element *)
	& program[  target_ID
	          + dispatcher_header_length
		  +   break_element_length
		    * h.max_break_elements
		  +   map_element_length
		    * t];

    if ( component_type == DISPATCHER )
    {
	if ( me.dispatcher_ID != 0 )
	{
	    sprintf ( error_message,
		      "Attempt to attach dispatcher"
		      " %u to dispatcher %u type %u\n"
		      "conflicts with previous"
		      " attachment of dispatcher"
		      " %u",
		      component_ID, target_ID, t,
		      me.dispatcher_ID );
	    return 0;
	}
	me.dispatcher_ID = component_ID;
	return 1;
    }
    else if ( component_type == INSTRUCTION )
    {
	if ( me.instruction_ID != 0 )
	{
	    sprintf ( error_message,
		      "Attempt to attach instruction"
		      " %u to dispatcher %u type %u\n"
		      "conflicts with previous"
		      " attachment of instruction"
		      " %u",
		      component_ID, target_ID, t,
		      me.instruction_ID );
	    return 0;
	}
	me.instruction_ID = component_ID;
	return 1;
    }
    else
	assert ( ! "assert failure" );
}

void LEX::convert_program_endianhood ( void )
{
}

// Scanning
// --------

static const char * pmode ( uns32 mode );

// Scanner state.
//
static uns32 next;
    // input_buffer[next] is the first character of the
    // first yet unscanned atom.
static uns32 master_atom_table_ID;
    // ID of atom table that is the initial atom table
    // or is the last non-continuation atom table
    // gone to.  Used to output the kind and label for
    // an item.

// We assume the program is well formed, in that an
// XXX_ID actually points at a program component of
// type XXX.  We check this with asserts (the attach
// statements check this).  Everything else found
// wrong with the program is a SCAN_ERROR.

void LEX::init_scan ( void )
{
    input_buffer.resize ( 0 );
    translation_buffer.resize ( 0 );

    next = 0;
    assert ( program[0] == PROGRAM );
    program_header & h = * (program_header *)
    			 & program[0];
    assert ( program[h.atom_table_ID] == ATOM_TABLE );
    master_atom_table_ID = h.atom_table_ID;
}

// Write the beginning of a scan error message into
// error message and return a pointer to the next
// location in error message.  Usage is:
//
//	sprintf ( scan_error ( length ), ... )
//
// `length' is the number of characters scanned after
// `next'.
//
static char * scan_error ( uns32 length );

// Write a character using spchar to a static buffer
// and return the static buffer.
//
static const char * sbpchar ( uns32 c );

static uns32 current_atom_table_ID;
    // ID of the atom table currently being used to
    // scan atoms.  Either == master_atom_table_ID or
    // is for a CONTINUATION table.
    //
    // This is a global variable because it is output
    // by scan_error.

uns32 LEX::scan
	( uns32 & first, uns32 & last, uns32 & label )
{
    if ( next >= input_buffer.length_increment )
    {
        // If next has gotten to be as large as
	// length_increment, shift the input_buffer
	// down, eliminating characters in items
	// already returned.
	//
        memmove ( & input_buffer[0],
	          & input_buffer[next],
		    ( input_buffer.length - next )
		  * sizeof ( LEX::inchar ) );
	input_buffer.deallocate ( next );
	next = 0;
    }

    // Initialize first and next and translation
    // buffer and current atom table.
    //
    first = next;
    translation_buffer.deallocate
	( translation_buffer.length );
    current_atom_table_ID = master_atom_table_ID;

    // We scan atoms until we get a current atom table
    // that is not a CONTINUATION table and is not
    // the same as the master table.  We may get this
    // via a GOTO or a SHORTCUT.  In the latter
    // case `shortcut' is set true.
    // 
    // A scan error is when we have no viable
    // instruction or dispatch table that will allow us
    // to continue.  We just immediately return
    // SCAN_ERROR after writing error_message.
    //
    // If we encounter an end of file without scanning
    // any input, we just return END_OF_FILE.  Other
    // end of files may or may not force a scan error.
    // 
    bool shortcut = false;
    while ( true )
    {
        // Scan an atom.

	atom_table_header & cath =
	    * (atom_table_header *)
	    & program[current_atom_table_ID];

	if (    current_atom_table_ID
	     != master_atom_table_ID
	    &&
	    cath.mode != CONTINUATION )
    	    break;

	// As we scan we recognize longer and longer
	// atoms.  If at any point we cannot continue,
	// we revert to the longest atom recognized
	// so far (if none, we may have a scan error).

	uns32 instruction_ID = cath.instruction_ID;
	uns32 atom_length = 0;
	    // Length and instruction_ID for the
	    // longest atom recognized so far.
	uns32 length = 0;
	    // Number of characters scanned so far
	    // starting at input_buffer[next].
	uns32 dispatcher_ID = cath.dispatcher_ID;
	    // Current dispatcher.
	assert ( instruction_ID == 0
	         ||
		    program[instruction_ID]
		 == INSTRUCTION );
	assert ( dispatcher_ID == 0
	         ||
		    program[dispatcher_ID]
		 == DISPATCHER );

	while ( true )
	{
	    // Dispatch the next character.  Stop when
	    // we have no next dispatcher or no next
	    // character (due to end of file).

	    if ( dispatcher_ID == 0 ) break;

	    if ( next + length >= input_buffer.length
	         &&
		 ! read_input() )
	    {
	        // End of file.
		//
		if ( next + length == first )
		    return END_OF_FILE;
		else break;
	    }

	    assert
	        ( next + length < input_buffer.length );
	    uns32 c =
	        input_buffer[next + length].character;
	    ++ length;

	    dispatcher_header & dh =
	        * (dispatcher_header *)
		& program[dispatcher_ID];

	    break_element * bep =
	        (break_element *)
		& program[  dispatcher_ID
		          + dispatcher_header_length];

	    // Binary search of break elements.
	    //
	    // Invariant:
	    //     bep[low].cmin <= c < bep[high].cmin
	    // where bep[high].cmin = infinity if
	    // high == dh.break_elements.
	    //
	    uns32 low = 0,
	          high = dh.break_elements,
	          mid;
	    while ( high - low >= 2 )
	    {
	        mid = ( high + low ) / 2;
		if ( bep[mid].cmin <= c )
		    low = mid;
		else
		    high = mid;
	    }

	    // Compute type from bep[low].
	    //
	    uns32 type_map_ID = bep[low].type_map_ID;
	    uns32 type = 0;
	    if ( type_map_ID != 0 )
	    {
		assert (    program[type_map_ID]
		         == TYPE_MAP );
		type_map_header & tmh =
		    * (type_map_header *)
		    & program[type_map_ID];
		uns32 type = tmh.singleton_type;
		if ( type == 0 )
		    type = ( (uns8 *) ( & tmh + 1 ) )
			   [c - tmh.cmin];
	    }
	    if ( type > dh.max_type )
	    {
	        sprintf ( scan_error ( length ),
		          "type %u computed for"
			  " character %s is too large"
			  " for dispatcher %u",
			  type, sbpchar ( c ),
			  dispatcher_ID );
		return SCAN_ERROR;
	    }

	    // Map to next dispatcher and current
	    // instruction.  If there is a current
	    // instruction, we have recognized a longer
	    // atom.
	    //
	    map_element * mep =
	        (map_element *)
		& program[  dispatcher_ID
		          + dispatcher_header_length
			  +   break_element_length
			    * dh.max_break_elements];
	    if ( mep[type].instruction_ID != 0 )
	    {
	        instruction_ID =
		    mep[type].instruction_ID;
		assert ( program[instruction_ID]
		         == INSTRUCTION );
		atom_length = length;
	    }
	    dispatcher_ID = mep[type].dispatcher_ID;
	    assert ( dispatcher_ID == 0
	             ||
		        program[dispatcher_ID]
		     == DISPATCHER );
	}

	// We are done dispatching characters.

	// If we found no instruction it is a scan
	// error.

	if ( instruction_ID == 0 )
	{
	    assert ( atom_length == 0 );
	    sprintf ( scan_error ( length ),
		      "no instruction found" );
	    return SCAN_ERROR;
	}

	instruction_header & ih =
	    * (instruction_header *)
	    & program[instruction_ID];
	uns32 op = ih.operation;
	if ( op & KEEP_FLAG )
	{
	    uns32 keep_length =
		LEX::keep_length ( op );
	    if ( keep_length > atom_length )
	    {
		sprintf ( scan_error ( length ),
			  "keep length(%u) greater than"
			  " atom length(%u)",
			  keep_length, atom_length );
		return SCAN_ERROR;
	    }
	    atom_length = keep_length;
	}

	if ( op & TRANSLATE_FLAG )
	{
	    uns32 translate_length =
		LEX::translate_length ( op );
	    if ( translate_length > 0 )
	    {
		uns32 q =
		    translation_buffer.allocate
			( translate_length );
		memcpy ( & translation_buffer[q],
			 & ih + 1,
			   translate_length
			 * sizeof ( uns32 ) );
	    }
	}
	else if ( op & TRANSLATE_HEX_FLAG )
	{
	    uns32 p = next
		    + LEX::prefix_length ( op );
	    uns32 endp = next + atom_length
		       - LEX::postfix_length
			     ( op );
	    uns32 tc = 0;
	    while ( p < endp )
	    {
		tc <<= 4;
		uns32 d =
		    input_buffer[p++].character;
		if ( '0' <= d && d <= '9' )
		    tc += d - '0';
		else if ( 'a' <= d && d <= 'f' )
		    tc += d - 'a' + 10;
		else if ( 'A' <= d && d <= 'F' )
		    tc += d - 'A' + 10;
		else
		{
		    sprintf ( scan_error ( length ),
			      "bad hexadecimal"
			      " digit(%s)"
			      " for TRANSLATE_HEX",
			      sbpchar ( d ) );
		    return SCAN_ERROR;
		}
	    }
	    translation_buffer[translation_buffer
				.allocate ( 1 )]
		= tc;
	}
	else if ( op & TRANSLATE_OCT_FLAG )
	{
	    uns32 p = next
		    + LEX::prefix_length ( op );
	    uns32 endp = next + atom_length
		       - LEX::postfix_length
			     ( op );
	    uns32 tc = 0;
	    while ( p < endp )
	    {
		tc <<= 3;
		uns32 d =
		    input_buffer[p++].character;
		if ( '0' <= d && d <= '7' )
		    tc += d - '0';
		else
		{
		    sprintf ( scan_error ( length ),
			      "bad octal digit(%s)"
			      " for TRANSLATE_OCT",
			      sbpchar ( d ) );
		    return SCAN_ERROR;
		}
	    }
	    translation_buffer[translation_buffer
				.allocate ( 1 )]
		= tc;
	}
	else
	{
	    uns32 q =
		translation_buffer.allocate
			( atom_length );
	    uns32 p = next;
	    for ( uns32 i = 0;
		  i < atom_length; ++ i )
		translation_buffer[q++] =
		    input_buffer[p++].character;
	}

	if ( atom_length == 0
	     &&
	     ( ih.atom_table_ID == 0
	       ||
	          ih.atom_table_ID
	       == current_atom_table_ID ) )
	{
	    sprintf ( scan_error ( length ),
		      "no atom scanned and"
		      " current atom table not"
		      " changed" );
	    return SCAN_ERROR;
	}

	if ( op & GOTO )
	    current_atom_table_ID = ih.atom_table_ID;
	else if ( op & SHORTCUT )
	{
	    shortcut = true;
	    current_atom_table_ID = ih.atom_table_ID;
	}
	assert (    program[current_atom_table_ID]
		 == ATOM_TABLE );

	next += atom_length;
    }

    uns32 atom_table_ID =
	shortcut ? current_atom_table_ID :
		   master_atom_table_ID;
    atom_table_header & ath =
	* (atom_table_header *)
	& program[atom_table_ID];

    first = first;
    last = next - 1;
    label = ath.label;

    if ( ! shortcut )
        master_atom_table_ID = current_atom_table_ID;

    switch ( ath.mode )
    {
    case LEXEME:
    case ERROR:
    case WHITESPACE:
        break;
    default:
    {
	int count = sprintf ( error_message,
		  "MASTER ATOM TABLE(%u)"
		  " returning item with bad kind(%s)"
		  " from atom table %u; ATOM:\n",
		  master_atom_table_ID,
		  pmode ( ath.mode ),
		  atom_table_ID );
	count += spinput ( error_message + count,
	                   first, last );
	return SCAN_ERROR;
    }
    }

    return ath.mode;
}

// See documentation above.
//
static const char * sbpchar ( uns32 c )
{
    static char buffer[20];
    spchar ( buffer, c );
    return buffer;
}

// See documentation above.
//
static char * scan_error ( uns32 length )
{
    char * p = error_message;
    p += sprintf
        ( p,
	  "MASTER_ATOM_TABLE(%u) CURRENT_ATOM_TABLE(%u)"
	  " POS(%llu) INPUT_BUFFER:\n",
	  master_atom_table_ID,
	  current_atom_table_ID,
	  input_buffer[next].position );
    p += spinput ( p, next, next + length - 1 );
    * p ++ = '\n';
    return p;
}

// Printing
// --------

int LEX::spchar ( char * buffer, uns32 c )
{
    if ( c == '\\' )
        return sprintf ( buffer, "\\\\" );
    else if ( 33 <= c && c <= 126 )
        return sprintf ( buffer, "%c", (char) c );
    else if ( c == ' ' )
        return sprintf ( buffer, "\\~" );
    else if ( c == '\n' )
        return sprintf ( buffer, "\\n" );
    else if ( c == '\t' )
        return sprintf ( buffer, "\\t" );
    else if ( c == '\f' )
        return sprintf ( buffer, "\\f" );
    else if ( c == '\v' )
        return sprintf ( buffer, "\\v" );
    else if ( c == '\b' )
        return sprintf ( buffer, "\\b" );
    else if ( c == '\r' )
        return sprintf ( buffer, "\\r" );
    else if ( c <= 0xFFFF )
        return sprintf ( buffer, "\\u%04X", c );
    else
        return sprintf ( buffer, "\\U%08X", c );
}

ostream & operator <<
	( ostream & out, const LEX::pchar & pc )
{
    char buffer[20];
    spchar ( buffer, pc.c );
    return out << buffer;
}

int LEX::spinput
	( char * buffer, uns32 first, uns32 last )
{
    assert ( first <= last );
    int columns = LINE;
    char * p = buffer;
    while ( first <= last )
    {
	uns32 c = input_buffer[first++].character;
	int count = spchar ( p, c );
	if ( count > columns )
	{
	    * p ++ = '\n';
	    columns = LINE;
	    spchar ( p, c );
	}
	p += count;
	columns -= count;
    }
    return p - buffer;
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

// cout << pmode ( mode ) prints a mode.  pmode ( mode )
// actually returns a pointer to a static buffer that
// holds the formatted mode, and can be used as an
// sprintf argument.
//
static const char * pmode ( uns32 mode )
{
    static char buffer[60];
    switch ( mode )
    {
    case LEXEME:
	strcpy ( buffer, "LEXEME" ); break;
    case WHITESPACE:
	strcpy ( buffer, "WHITESPACE" ); break;
    case ERROR:
	strcpy ( buffer, "ERROR" ); break;
    case MASTER:
	strcpy ( buffer, "MASTER" ); break;
    case CONTINUATION:
	strcpy ( buffer, "CONTINUATION" ); break;
    default:
	sprintf ( buffer, "ILLEGAL MODE (%u)", mode );
    }
    return buffer;
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

    bool first = true;
#   define COUT ( first ? ( first = false, cout ) : \
                          cout << ", " )
    if ( h.operation & KEEP_FLAG )
        COUT << "KEEP("
	     << LEX::keep_length ( h.operation )
	     << ")";
    if ( h.operation & TRANSLATE_FLAG )
    {
        translate_length =
	     LEX::translate_length ( h.operation );
        COUT << "TRANSLATE(" << translate_length;
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
        COUT << "TRANSLATE_HEX("
	     << LEX::prefix_length ( h.operation )
	     << ","
	     << LEX::postfix_length ( h.operation )
	     << ")";
    if ( h.operation & TRANSLATE_OCT_FLAG )
        COUT << "TRANSLATE_OCT("
	     << LEX::prefix_length ( h.operation )
	     << ","
	     << LEX::postfix_length ( h.operation )
	     << ")";
    if ( h.operation & GOTO )
        COUT << "GOTO(" << h.atom_table_ID << ")";
    if ( h.operation & SHORTCUT )
        COUT << "SHORTCUT(" << h.atom_table_ID << ")";
    if ( first ) COUT << "ACCEPT";
    cout << endl;
#   undef COUT

    return instruction_header_length
         + translate_length;
}

// Iterator that prints out a list of characters
// within LINE columns.  `nonempty' if the list is
// non-empty.  The list is indented by the given
// amount.  The user is responsible for the indent of
// the first line if user_indent is true.
//
struct pclist {
    int indent;
        // Indent set by constructor.
    bool user_indent;
        // User will indent first line:
	// set by constructor.
    bool empty;
        // True if list empty so far.
    int columns;
        // Number of columns remaining on current
	// line; LINE - indent columns are available
	// for each line.

    uns32 c1, c2;
        // If not empty then the range c1-c2 (or just c1
	// if c1 == c2) needs to be printed.  This is
	// delayed to allow c2 to grow.

    pclist ( int indent, bool user_indent = false )
	: indent ( indent ), user_indent ( user_indent )
    {
	assert ( LINE - indent >= 30 );
        empty = true;
	columns = LINE - indent;
    }

    // Print c1-c2 (or just c1 if c1 == c2 ).  Precede
    // by a ' ' or a '\n' and indent ' 's if not first
    // thing on line.
    //
    void flush ( void )
    {
        if ( empty ) return;

	char buffer[40];
	char * p = buffer;
        p += spchar ( p, c1 );
	if ( c2 != c1 )
	{
	    * p ++ = '-';
	    if ( c2 != 0xFFFFFFFF )
	        p += spchar ( p, c2 );
	}
	* p = 0;
	int needed = p - buffer;

	if ( columns == LINE - indent )
	{
	    // If nothing on line, its our first line.
	    //
	    if ( ! user_indent )
	        cout << setw ( indent ) << "";
	    columns -= needed;
	}
	else if ( columns >= 1 + needed )
	{
	    cout << " ";
	    columns -= 1 + needed;
	}
	else
	{
	    cout << endl << setw ( indent ) << "";
	    columns = LINE - indent - needed;
	}

	cout << buffer;
    }

    // Add c1-c2 to the list of characters printed.
    // c1 == c2 is possible.  Must be called in order
    // of increasing c; c2 >= c1 required.
    //
    void add ( uns32 c1, uns32 c2 )
    {
	assert ( empty || this->c2 < c1 );
	assert ( c1 <= c2 );

        if ( ! empty && c1 == this->c2 + 1 )
	    this->c2 = c2;
	else
	{
	    flush();
	    this->c1 = c1;
	    this->c2 = c2;
	    empty = false;
	}
    }
};

// Print the dispatcher at program[ID] with the given
// indent, if ID is non-zero in the cooked format.
// Return length of dispatcher component.  If ID is
// zero, do nothing but return 0.
//
static uns32 print_cooked_dispatcher
    ( uns32 ID, unsigned indent = IDwidth )
{
    if ( ID == 0 ) return 0;

    cout << pID ( ID ) << "DISPATCHER" << endl;

    dispatcher_header & h =
	* (dispatcher_header *) & program[ID];

    uns32 length = dispatcher_header_length;
    break_element * bep =
        (break_element *)
        & program[ID + length];
    length += break_element_length
	    * h.max_break_elements;
    map_element * mep =
        (map_element *)
        & program[ID + length];
    length += map_element_length
	    * ( h.max_type + 1 );

    cout << INDENT << "Break Elements: "
	 << h.break_elements << endl;
    cout << INDENT << "Max Break Elements: "
	 << h.max_break_elements << endl;
    cout << INDENT << "Max Type: "
	 << h.max_type << endl;

    // Construct tmap so that t2 = tmap[t1] iff t2 is
    // the smallest type such that mep[t2] == mep[t1].
    //
    uns32 tmap[h.max_type+1];
    for ( uns32 t1 = 0; t1 <= h.max_type; ++ t1 )
    {
        uns32 t2 = 0;
	while (    mep[t2].instruction_ID
	        != mep[t1].instruction_ID
		||
		   mep[t2].dispatcher_ID
		!= mep[t1].dispatcher_ID ) ++ t2;
	tmap[t1] = t2;
    }

    // For each t such that tmap[t] == t, mep[t] has
    // a non-zero dispatcher_ID or instruction_ID, and
    // some characters map to t, print the list all
    // characters that map to to t and instruction and
    // dispatcher_ID if there are non-zero.
    //
    for ( uns32 t = 0; t <= h.max_type; ++ t )
    {
        if ( t != tmap[t] ) continue;
	if ( mep[t].instruction_ID == 0
	     &&
	     mep[t].dispatcher_ID == 0 )
	    continue;

	pclist pcl ( IDwidth );
	for ( uns32 b = 0; b < h.break_elements; ++ b )
	{
	    uns32 cmin = bep[b].cmin;
	    uns32 cmax = ( b == h.break_elements - 1 ?
	                   0xFFFFFFFF :
			   bep[b+1].cmin - 1 );

	    uns32 type_map_ID = bep[b].type_map_ID;

	    if ( type_map_ID == 0 )
	    {
	        if ( tmap[0] == t )
		    pcl.add ( cmin, cmax );
	    }
	    else
	    {
	        assert (    program[type_map_ID]
		         == TYPE_MAP );
		type_map_header & mh =
		    * (type_map_header *)
		    & program[type_map_ID];
		uns32 type = mh.singleton_type;
		if ( type != 0 )
		{
		    if ( tmap[type] == t )
			pcl.add ( cmin, cmax );
		}
		else
		{
		    uns8 * p = (uns8 *) ( & mh + 1 );
		    for ( uns32 c = cmin;
		          c <= cmax; ++ c )
		    {
		        if ( tmap[p[c-cmin]] == t )
			    pcl.add ( c, c );
		    }
		}
	    }
	}

	if ( pcl.empty ) continue;

	pcl.flush();
	cout << endl;

	print_instruction
	    ( mep[t].instruction_ID, IDwidth + 4 );
	if ( mep[t].dispatcher_ID != 0 )
	    cout << INDENT << "    Dispatcher ID: "
	         << mep[t].dispatcher_ID << endl;
    }

    return length;
}

uns32 LEX::print_program_component
	( std::ostream & out, uns32 ID, bool cooked )
{
    char buffer[100];
    switch ( program[ID] )
    {
    case PROGRAM:
    {
	cout << pID ( ID ) << "PROGRAM" << endl;
	program_header & h =
	    * (program_header *) & program[ID];
	cout << INDENT << "Atom Table ID: "
	     << h.atom_table_ID << endl;
	return program_header_length;
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
	return atom_table_header_length;
    }
    case DISPATCHER:
    if ( cooked )
	return print_cooked_dispatcher ( ID );
    else
    {
	cout << pID ( ID ) << "DISPATCHER" << endl;
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
	uns32 length = dispatcher_header_length;
	uns32 p, n;
	for ( p = ID + length, n = 0;
	      n < h.break_elements;
	      p += break_element_length, ++ n )
	{
	    break_element & be =
		* (break_element *) & program[p];
	    cout << INDENT
		 << setw ( 24 ) << pchar ( be.cmin )
		 << setw ( 16 ) << be.type_map_ID
		 << endl;
	}
	length += break_element_length
	        * h.max_break_elements;
	cout << INDENT << "Map:    type: "
	     << setw ( 16 ) << "dispatcher_ID"
	     << setw ( 16 ) << "instruction_ID"
	     << endl;
	uns32 t;
	for ( p = ID + length, t = 0;
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
	length += map_element_length
	        * ( h.max_type + 1 );
	return length;
    }
    case INSTRUCTION:
    {
	cout << pID ( ID );
	return print_instruction ( ID, 0 );
    }
    case TYPE_MAP:
    {
	cout << pID ( ID ) << "TYPE_MAP" << endl;
	type_map_header & h =
	    * (type_map_header *) & program[ID];
	uns32 length = type_map_header_length;
	if ( h.singleton_type > 0 )
	{
	    cout << setw ( IDwidth + 4 )
		 << h.singleton_type
		 << ": " << pchar ( h.cmin )
	         << "-" << pchar ( h.cmax ) << endl;
	}
	else
	{
	    uns8 * map = (uns8 *) ( & h + 1 );
	    length += ( h.cmax - h.cmin + 4 ) / 4;
	    for ( unsigned t = 0; t < 256; ++ t )
	    {
		pclist pcl ( IDwidth + 6, true );
		for ( uns32 c = h.cmin;
		      c <= h.cmax; ++ c )
		{
		    if ( map[c - h.cmin] == t )
		    {
		        if ( pcl.empty )
			    cout << setw ( IDwidth
					   + 4 )
				 << t << ": ";
			pcl.add ( c, c );
		    }
		}
		pcl.flush();
		if ( ! pcl.empty ) cout << endl;
	    }
	}
	return length;
    }
    default:
    {
	cout << pID ( ID ) << "ILLEGAL ITEM TYPE("
	     << program[ID] << ")" << endl;
	return program.length - ID;
    }
    }
}

void LEX::print_program
	( std::ostream & out, bool cooked )
{
    uns32 ID = 0;
    while ( ID < program.length )
    {
	// If cooked skip some components.
	//
        if ( cooked ) switch ( program[ID] )
	{
	case INSTRUCTION:
	{
	    instruction_header & h =
		* (instruction_header *)
		& program[ID];
	    ID += instruction_header_length;
	    if (   h.operation
		 & LEX::TRANSLATE_FLAG )
		ID += LEX::translate_length
			    ( h.operation );
	    continue;
	}
	case TYPE_MAP:
	{
	    type_map_header & h =
		* (type_map_header *) & program[ID];
	    ID += type_map_header_length;
	    if ( h.singleton_type == 0 )
		ID += ( h.cmax - h.cmin + 4 ) / 4;
	    continue;
	}
	}

        ID += print_program_component
	    ( out, ID, cooked );
    }

    if ( ID > program.length )
        out << "  ILLEGALLY TRUNCATED LAST PROGRAM"
	       " COMPONENT" << endl;
}
