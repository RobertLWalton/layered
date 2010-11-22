// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Nov 21 21:57:30 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Program Construction
//	Scanning
//	Reading
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

unsigned LEX::line_length = 72;
unsigned LEX::indent = 4;
char LEX::error_message[1000];

// Program Construction
// ------- ------------

uns32 LEX::create_table ( uns32 mode )
{
    uns32 ID = program.allocate ( table_header_length );
    table_header & h = * (table_header *) & program[ID];
    h.pctype = TABLE;
    h.mode = mode;
    h.dispatcher_ID = 0;
    h.instruction_ID = 0;

    // First master table becomes the initial table
    // of the program.
    //
    program_header & ph =
        * (program_header *) & program[0];
    if ( ph.initial_table_ID == 0
         &&
	 mode == MASTER )
    {
	ph.initial_table_ID = ID;
    }

    return ID;
}

uns32 LEX::table_mode ( uns32 ID )
{
    table_header & h = * (table_header *) & program[ID];
    return h.mode;
}

void LEX::create_program ( void )
{
    program.resize ( 0 );
    uns32 ID = program.allocate
    		   ( program_header_length );
    assert ( ID == 0 );

    program_header & h =
        * (program_header *) & program[ID];
    h.pctype = PROGRAM;
}

uns32 LEX::create_dispatcher
	( uns32 max_breakpoints,
	  uns32 max_ctype )
{
    uns32 length =
          dispatcher_header_length
	+   break_element_length
	  * ( max_breakpoints + 1 )
	+   map_element_length
	  * ( max_ctype + 1 );
    uns32 ID = program.allocate ( length );
    dispatcher_header & h =
        * (dispatcher_header *)
	& program[ID];
    h.pctype = DISPATCHER;
    h.break_elements = 1;
    h.max_break_elements = max_breakpoints + 1;
    h.max_ctype = max_ctype;

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
    h.pctype = TYPE_MAP;
    h.cmin = cmin;
    h.cmax = cmax;
    h.singleton_ctype = 0;
    memcpy ( & h + 1, map, length );
    return ID;
}

uns32 LEX::create_type_map
	( uns32 cmin, uns32 cmax, uns32 ctype )
{
    assert ( cmax >= cmin );
    uns32 ID = program.allocate
    	( type_map_header_length );
    type_map_header & h =
        * (type_map_header *) & program[ID];
    h.pctype = TYPE_MAP;
    h.cmin = cmin;
    h.cmax = cmax;
    assert ( ctype != 0 );
    h.singleton_ctype = ctype;
    return ID;
}

uns32 LEX::create_instruction
	( uns32 operation,
	  uns32 * translation_vector,
	  uns32 atom_table_ID,
	  uns32 require_dispatcher_ID,
	  uns32 else_instruction_ID,
	  uns32 output_error_type,
	  uns32 goto_call_table_ID )
{
    assert ( ( ( operation & TRANSLATE_TO_FLAG ) != 0 )
	     +
             ( ( operation & TRANSLATE_HEX_FLAG ) != 0 )
	     +
             ( ( operation & TRANSLATE_OCT_FLAG ) != 0 )
	     <= 1 );

    assert ( ( operation & REQUIRE ) == 0
             ||
	     ( operation & TRANSLATE_HEX_FLAG )
             ||
	     ( operation & TRANSLATE_OCT_FLAG )
             ||
	     ( operation & MATCH ) );

    assert ( ( operation & ELSE ) == 0
             ||
	     ( operation & TRANSLATE_HEX_FLAG )
             ||
	     ( operation & TRANSLATE_OCT_FLAG )
             ||
	     ( operation & MATCH )
             ||
	     ( operation & REQUIRE ) );

    assert ( ( ( operation & ERRONEOUS_ATOM ) != 0 )
	     +
             ( ( operation & OUTPUT ) != 0 )
	     <= 1 );

    assert ( ( ( operation & GOTO ) != 0 )
             +
	     ( ( operation & CALL ) != 0 )
             +
	     ( ( operation & RETURN ) != 0 )
	     <= 1 );

    uns32 translate_to_length =
        ( operation & TRANSLATE_TO_FLAG ) ?
	  LEX::translate_to_length ( operation ) :
	0;

    if ( translate_to_length > 0 )
        assert ( translation_vector != NULL );
    else
        assert ( translation_vector == NULL );

    if ( operation & MATCH )
        assert ( atom_table_ID != 0 );
    else
        assert ( atom_table_ID == 0 );

    if ( operation & REQUIRE )
        assert ( require_dispatcher_ID != 0 );
    else
        assert ( require_dispatcher_ID == 0 );

    if ( operation & ELSE )
        assert ( else_instruction_ID != 0 );
    else
        assert ( else_instruction_ID == 0 );

    if ( ( operation & OUTPUT )
         ||
	 ( operation & ERRONEOUS_ATOM ) )
        assert ( output_error_type != 0 );
    else
        assert ( output_error_type == 0 );

    if ( ( operation & GOTO )
         ||
	 ( operation & CALL ) )
        assert ( goto_call_table_ID != 0 );
    else
        assert ( goto_call_table_ID == 0 );

    uns32 ID = program.allocate
    	(   instruction_header_length
	  + translate_to_length );
    instruction_header & h =
        * (instruction_header *) & program[ID];
    h.pctype = INSTRUCTION;
    h.operation = operation;
    h.atom_table_ID = atom_table_ID;
    h.require_dispatcher_ID = require_dispatcher_ID;
    h.else_instruction_ID = else_instruction_ID;
    h.output_error_type = output_error_type;
    h.goto_call_table_ID = goto_call_table_ID;

    uns32 * p = (uns32 *) ( & h + 1 );

    while ( translate_to_length -- )
	* p ++ = * translation_vector ++;

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
    assert ( dh.pctype == DISPATCHER );
    type_map_header & mh =
        * (type_map_header *)
	& LEX::program[type_map_ID];
    assert ( mh.pctype == TYPE_MAP );

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
    uns32 target_pctype = program[target_ID];
    uns32 component_pctype = program[component_ID];

    if ( target_pctype == TABLE )
    {
	table_header & h =
	    * (table_header *)
	    & program[target_ID];

        if ( component_pctype == DISPATCHER )
	{
	    if ( h.dispatcher_ID != 0 )
	    {
		sprintf ( error_message,
			  "Attempt to attach dispatcher"
			  " %u to table %u\n"
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
        else if ( component_pctype == INSTRUCTION )
	{
	    if ( h.instruction_ID != 0 )
	    {
		sprintf ( error_message,
			  "Attempt to attach"
			  " instruction %u to table"
			  " %u\n"
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
	else assert
	    ( ! "bad attach component pctypes" );
    }
    else if ( target_pctype == DISPATCHER
              &&
	      component_pctype == TYPE_MAP )
        return attach_type_map_to_dispatcher
		   ( target_ID, component_ID );
    else
	assert ( ! "bad attach component pctypes" );
}

uns32 LEX::attach
    	    ( uns32 target_ID,
    	      uns32 ctype,
	      uns32 component_ID )
{
    dispatcher_header & h =
        * (dispatcher_header *)
	& program[target_ID];
    assert ( h.pctype == DISPATCHER );

    uns32 component_pctype = program[component_ID];
    assert ( component_pctype == DISPATCHER
             ||
	     component_pctype == INSTRUCTION );
    assert ( ctype <= h.max_ctype );
    map_element & me =
        * (map_element *)
	& program[  target_ID
	          + dispatcher_header_length
		  +   break_element_length
		    * h.max_break_elements
		  +   map_element_length
		    * ctype];

    if ( component_pctype == DISPATCHER )
    {
	if ( me.dispatcher_ID != 0 )
	{
	    sprintf ( error_message,
		      "Attempt to attach dispatcher"
		      " %u to dispatcher %u ctype %u\n"
		      "conflicts with previous"
		      " attachment of dispatcher"
		      " %u",
		      component_ID, target_ID, ctype,
		      me.dispatcher_ID );
	    return 0;
	}
	me.dispatcher_ID = component_ID;
	return 1;
    }
    else if ( component_pctype == INSTRUCTION )
    {
	if ( me.instruction_ID != 0 )
	{
	    sprintf ( error_message,
		      "Attempt to attach instruction"
		      " %u to dispatcher %u ctype %u\n"
		      "conflicts with previous"
		      " attachment of instruction"
		      " %u",
		      component_ID, target_ID, ctype,
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

// Scanner state.
//
static uns32 next;
    // input_buffer[next] is the first character of the
    // first yet unscanned atom.
static uns32 current_table_ID;
    // Current table ID.
static uns32 return_stack[64],
      * return_stack_p = return_stack,
      * return_stack_endp = return_stack + 64;
    // Return stack.

// Return true if table_ID is in return_stack.
//
static bool is_recursive ( uns32 table_ID )
{
    uns32 * p = return_stack;
    while ( p < return_stack_p )
    {
        if ( * p ++ == table_ID ) return true;
    }
    return false;
}

// We assume the program is well formed, in that an
// XXX_ID actually points at a program component of
// pctype XXX.  We check this with asserts (the attach
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
    assert ( program[h.initial_table_ID] == TABLE );
    current_table_ID = h.initial_table_ID;
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
static char * scan_error
        ( uns32 length, uns32 next = ::next );

// Write a character using spchar to a static buffer
// and return the static buffer.
//
static const char * sbpchar ( uns32 c );

// Write a mode or return value using spchar to a static
// buffer and return the static buffer.
//
static const char * sbpmode ( uns32 mode );

// Print the instruction at program[ID] with the given
// indent and endl, if ID is non-zero, and return ID
// repositioned just after instuction.  However, if
// program[ID] does not == INSTRUCTION, print ILLEGAL
// instruction message and return 0.
//
// If ID == 0 do nothing but return 0.
//
static uns32 print_instruction
    ( std::ostream & out, uns32 ID, unsigned indent );

// If non-NULL, function to call on executing
// instruction with ERRONEOUS_ATOM flag.
//
void (* LEX::erroneous_atom)
    ( uns32 first, uns32 last, uns32 type ) = NULL;

// Set non-NULL to enable scan tracing
// (see ll_lexeme.h).
//
std::ostream * LEX::scan_trace_out = NULL;
# define TOUT if ( scan_trace_out ) (* scan_trace_out)

// Given a dispatcher_ID and a character c return the
// ctype that the dispatcher maps c onto.
//
static uns32 ctype ( uns32 dispatcher_ID, uns32 c )
{
    TOUT << "  Character = " << pchar ( c )
	 << " Dispatcher = " << dispatcher_ID;

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

    // Compute ctype from bep[low].
    //
    uns32 type_map_ID = bep[low].type_map_ID;
    uns32 ctype = 0;
    if ( type_map_ID != 0 )
    {
	assert (    program[type_map_ID]
		 == TYPE_MAP );
	TOUT << " Type Map = " << type_map_ID;
	type_map_header & tmh =
	    * (type_map_header *)
	    & program[type_map_ID];
	ctype = tmh.singleton_ctype;
	if ( ctype == 0 )
	    ctype = ( (uns8 *) ( & tmh + 1 ) )
		    [c - tmh.cmin];
    }

    TOUT << " CType = " << ctype << endl;

    return ctype;
}

// Scan atom given current table.  Process instruction
// group but not
//
//	ERRONEOUS_ATOM
//	OUTPUT
//	GOTO
//	CALL
//	RETURN
//	FAIL
//
// and return instruction.  Return 0 if error, leaving
// error message in error message.  Return atom_length
// and add translation of atom to translation buffer.
//
static uns32 scan_atom ( uns32 & atom_length )
{
    const uns32 SCAN_ERROR = 0;
        // Local version of SCAN_ERROR.

    TOUT << "Start atom scan: atom table = "
	 << current_table_ID << endl;

    table_header & cath =
	* (table_header *)
	& program[current_table_ID];

    // As we scan we recognize longer and longer atoms.
    // If at any point we cannot continue, we revert to
    // the longest atom recognized so far (if none, we
    // may have a scan error).

    uns32 instruction_ID = cath.instruction_ID;
    atom_length = 0;
	// Length and instruction_ID for the longest
	// atom recognized so far.
    uns32 length = 0;
	// Number of characters scanned so far starting
	// at input_buffer[next].
    uns32 dispatcher_ID = cath.dispatcher_ID;
	// Current dispatcher.
    uns32 tnext = translation_buffer.length;
	// Save of current translation buffer position
	// for REQUIRE and ELSE.

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
	     ! (*read_input)() )
	    break; // End of file.

	assert
	    ( next + length < input_buffer.length );
	uns32 c =
	    input_buffer[next + length].character;
	++ length;

	uns32 ctype = ::ctype ( dispatcher_ID, c );

	dispatcher_header & dh =
	    * (dispatcher_header *)
	    & program[dispatcher_ID];

	if ( ctype > dh.max_ctype )
	{
	    sprintf ( scan_error ( length ),
		      "ctype %u computed for character"
		      " %s is too large for dispatcher"
		      " %u",
		      ctype, sbpchar ( c ),
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
	if ( mep[ctype].instruction_ID != 0 )
	{
	    instruction_ID = mep[ctype].instruction_ID;
	    assert ( program[instruction_ID]
		     == INSTRUCTION );
	    atom_length = length;
	}
	dispatcher_ID = mep[ctype].dispatcher_ID;
	assert ( dispatcher_ID == 0
		 ||
		    program[dispatcher_ID]
		 == DISPATCHER );
    }

    // We are done dispatching characters.

    // Loop to process instruction group.
    //
    while ( true )
    {
	// If we found no instruction it is a scan
	// error.

	if ( instruction_ID == 0 )
	{
	    assert ( atom_length == 0 );
	    sprintf ( scan_error ( length ),
		      "no instruction found" );
	    return SCAN_ERROR;
	}

	if ( scan_trace_out )
	    print_instruction ( * scan_trace_out,
				instruction_ID,
				2 );

	bool fail = false;

	instruction_header & ih =
	    * (instruction_header *)
	    & program[instruction_ID];
	uns32 op = ih.operation;

	uns32 keep_length = atom_length;
	    // Keep_length becomes the effective atom_
	    // length until we are sure we are not
	    // no-oping the instruction for the sake
	    // of an ELSE.

	if ( op & MATCH )
	{
	    table_header & th =
		* (table_header *)
		& program[ih.atom_table_ID];
	    assert ( cath.pctype == TABLE );
	    if ( th.mode != ATOM )
	    {
		sprintf
		    ( scan_error ( atom_length ),
		      "MATCH in"
		      " instruction %d executed by"
		      " table %d targets"
		      " non-translation table",
		      instruction_ID,
		      current_table_ID );
		return SCAN_ERROR;
	    }
	    else if (    return_stack_p
		      == return_stack_endp )
	    {
		sprintf
		    ( scan_error ( atom_length ),
		      "MATCH in"
		      " instruction %d executed by"
		      " table %d but return stack is"
		      " full",
		      instruction_ID,
		      current_table_ID );
		return SCAN_ERROR;
	    }

	    * return_stack_p ++ = current_table_ID;

	    if ( is_recursive
	             ( ih.atom_table_ID ) )
	    {
		sprintf
		    ( scan_error ( atom_length ),
		      "recursive MATCH to table %d"
		      " in instruction %d executed"
		      " by table %d",
		      ih.atom_table_ID,
		      instruction_ID,
		      current_table_ID );
		return SCAN_ERROR;
	    }

	    current_table_ID = ih.atom_table_ID;
	    uns32 tinstruction_ID =
	        scan_atom ( keep_length );
	    current_table_ID = * -- return_stack_p;

	    instruction_header & tih =
		* (instruction_header *)
		& program[tinstruction_ID];
	    if ( tih.operation & FAIL )
	    	fail = true;
	    else if ( op & ( TRANSLATE_TO_FLAG
	                     |
			     TRANSLATE_HEX_FLAG
	                     |
			     TRANSLATE_OCT_FLAG ) )
		translation_buffer.resize ( tnext );
	}

	if ( ! fail && ( op & KEEP_FLAG ) )
	{
	    // Due to possible MATCH, actual atom
	    // length is in keep_length.
	    //
	    uns32 keep = LEX::keep_length ( op );
	    if ( keep > keep_length )
	    {
		sprintf ( scan_error ( length ),
			  "keep length(%u) greater"
			  " than atom length(%u)",
			  keep,
			  keep_length );
		return SCAN_ERROR;
	    }
	    keep_length = keep;
	}

	if ( fail ) ; // Do nothing
	else if ( op & ( TRANSLATE_HEX_FLAG
		         |
		         TRANSLATE_OCT_FLAG ) )
	{
	    uns32 p = next
		    + LEX::prefix_length ( op );
	    uns32 endp = next + keep_length
		       - LEX::postfix_length
			     ( op );
	    uns32 tc = 0;

	    if ( op & TRANSLATE_HEX_FLAG )
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
			fail = true;
			break;
		    }
		}
	    else if ( op & TRANSLATE_OCT_FLAG )
		while ( p < endp )
		{
		    tc <<= 3;
		    uns32 d =
			input_buffer[p++].character;
		    if ( '0' <= d && d <= '7' )
			tc += d - '0';
		    else
		    {
			fail = true;
			break;
		    }
		}

	    if ( ! fail )
		translation_buffer
		    [translation_buffer.allocate ( 1 )]
		    = tc;
	}
	else if ( op & TRANSLATE_TO_FLAG )
	{
	    uns32 translate_to_length =
		LEX::translate_to_length ( op );
	    if ( translate_to_length > 0 )
	    {
		uns32 q =
		    translation_buffer.allocate
			( translate_to_length );
		memcpy ( & translation_buffer[q],
			 & ih + 1,
			   translate_to_length
			 * sizeof ( uns32 ) );
	    }
	}
	else if ( ! ( op & MATCH ) )
	{
	    uns32 q =
		translation_buffer.allocate
			( keep_length );
	    uns32 p = next;
	    for ( uns32 i = 0; i < keep_length; ++ i )
		translation_buffer[q++] =
		    input_buffer[p++].character;
	}

	if ( ! fail && ( op & REQUIRE ) )
	{
	    uns32 dispatcher_ID =
		ih.require_dispatcher_ID;
	    uns32 tlength = 0;
	    while ( true )
	    {
		// Dispatch the next translate_buffer
		// character.  Stop with if we have
		// no next dispatcher or run out of
		// translation buffer characters.

		if ( dispatcher_ID == 0 )
		{
		    fail = (    tnext + tlength
			     != translation_buffer
				    .length );
		    break;
		}
		assert (    program[dispatcher_ID]
			 == DISPATCHER );

		if (    tnext + tlength
		     >= translation_buffer.length )
		{
		    fail = true;
		    break;
		}

		uns32 c = translation_buffer
			      [tnext + tlength];
		++ tlength;

		uns32 ctype =
		    ::ctype ( dispatcher_ID, c );

		dispatcher_header & dh =
		    * (dispatcher_header *)
		    & program[dispatcher_ID];

		if ( ctype > dh.max_ctype )
		{
		    sprintf ( scan_error ( length ),
			      "ctype %u computed for"
			      " character %s is too "
			      " large for dispatcher"
			      " %u",
			      ctype, sbpchar ( c ),
			      dispatcher_ID );
		    return SCAN_ERROR;
		}

		if ( ctype == 0 )
		{
		    fail = true;
		    break;
		}

		// Map to next dispatcher.
		//
		map_element * mep =
		    (map_element *)
		    & program
		          [  dispatcher_ID
			   + dispatcher_header_length
			   +   break_element_length
			     * dh.max_break_elements];

		dispatcher_ID =
		    mep[ctype].dispatcher_ID;
	    }
	}

	if ( fail )
	{
	    translation_buffer.resize ( tnext );
	    if ( op & ELSE )
	    {
		instruction_ID =
		    ih.else_instruction_ID;
		if ( instruction_ID == 0 )
		{
		    sprintf ( scan_error ( length ),
			      "no instruction for ELSE"
			      " in failed instruction"
			      " %d executed by table"
			      " %d",
			      instruction_ID,
			      current_table_ID );
		    return SCAN_ERROR;
		}
		assert (    program[instruction_ID]
			 == INSTRUCTION );

		// Loop to next instruction.
		//
		continue;
	    }
	    else
	    {
		sprintf ( scan_error ( length ),
			  "no ELSE in failed"
			  " instruction %d executed by"
			  " table %d",
			  instruction_ID,
			  current_table_ID );
		return SCAN_ERROR;
	    }
	}

	// We are at un-failed instruction of
	// instruction group.
	//
	atom_length = keep_length;

	return instruction_ID;
    }
}

uns32 LEX::scan ( uns32 & first, uns32 & last )
{
    if ( next >= input_buffer.length_increment )
    {
        // If next has gotten to be as large as
	// length_increment, shift the input_buffer
	// down, eliminating characters in lexemes
	// already returned.
	//
        memmove ( & input_buffer[0],
	          & input_buffer[next],
		    ( input_buffer.length - next )
		  * sizeof ( LEX::inchar ) );
	input_buffer.deallocate ( next );
	next = 0;
    }

    // Initialize first and translation buffer.
    //
    first = next;
    translation_buffer.deallocate
	( translation_buffer.length );

    // We scan atoms until we get to a point where the
    // atom table is to be changed from a table with
    // mode != MASTER to one with mode == MASTER and
    // next != first, or `output_type' is set (in this
    // case next == first is allowed).
    // 
    // A scan error is when we have no viable
    // instruction or dispatch table that will allow us
    // to continue.  We just immediately return
    // SCAN_ERROR after writing error_message.
    //
    // If we encounter an end of file we end the current
    // atom (which might be of zero length).
    //
    // We decrement loop_count whenever we find no atom
    // and signal a scan error if it goes to zero.  We
    // set loop_count to a number at least as large as
    // the total number of tables, in this case program
    // .length, whenever we find an atom.  If it goes to
    // zero we must be in a loop changing tables and not
    // finding an atom.
    // 
    uns32 output_type = 0;
    uns32 last_mode = MASTER;
    return_stack_p = return_stack;
    uns32 loop_count = program.length;
        // Set to max number of tables in order to
	// detect endless loops.
    while ( true )
    {
        // Scan next atom of current lexeme.

	table_header & cath =
	    * (table_header *)
	    & program[current_table_ID];

	// First check lexeme ending conditions.

	if ( output_type != 0 )
	{
	    if ( cath.mode != MASTER )
	    {
	        sprintf ( scan_error ( last + 1 - first,
		                       first ),
		          "attempt to OUTPUT when the"
			  " next table %d is not a"
			  " master table",
			  current_table_ID );
		return SCAN_ERROR;
	    }

	    break;
	}

	if ( last_mode != MASTER
	     &&
	     cath.mode == MASTER
	     &&
	     first != next )
	    break;

	last_mode = cath.mode;

	// Reset return stack if current mode is master.

	if ( cath.mode == MASTER )
	    return_stack_p = return_stack;

	// Scan atom.  Return atom length and
	// instruction.
	//
	uns32 atom_length;
	uns32 instruction_ID =
	    scan_atom ( atom_length );
	if ( instruction_ID == 0 ) return SCAN_ERROR;

	instruction_header & ih =
	    * (instruction_header *)
	    & program[instruction_ID];
        uns32 op = ih.operation;

	if ( op & ERRONEOUS_ATOM )
	{
	    if ( atom_length == 0 )
	    {
		sprintf ( scan_error ( atom_length ),
			  "ERRONEOUS_ATOM in"
			  " instruction %d executed"
			  " by table %d but atom is of"
			  " zero length",
			  instruction_ID,
			  current_table_ID );
		return SCAN_ERROR;
	    }
	    else if ( erroneous_atom == NULL )
	    {
		sprintf ( scan_error ( atom_length ),
			  "ERRONEOUS_ATOM in"
			  " instruction %d executed"
			  " by table %d but"
			  " erroneous_atom function"
			  " does not exist",
			  instruction_ID,
			  current_table_ID );
		return SCAN_ERROR;
	    }
	    else
		(*erroneous_atom)
		    ( next, next + atom_length - 1,
		      ih.output_error_type );
	}
	else if ( op & OUTPUT )
	    output_type = ih.output_error_type;

	if ( op & RETURN )
	{
	    if ( return_stack_p == return_stack )
	    {
		sprintf
		    ( scan_error ( atom_length ),
		      "RETURN in instruction %d"
		      " executed by table %d but"
		      " return stack is empty",
		      instruction_ID,
		      current_table_ID );
		return SCAN_ERROR;
	    }
	    current_table_ID = * -- return_stack_p;
	    assert (    program[current_table_ID]
		     == TABLE );
	}
	else if ( op & CALL )
	{
	    if (    return_stack_p
		 == return_stack_endp )
	    {
		sprintf
		    ( scan_error ( atom_length ),
		      "CALL in"
		      " instruction %d executed by"
		      " table %d but return stack is"
		      " full",
		      instruction_ID,
		      current_table_ID );
		return SCAN_ERROR;
	    }

	    * return_stack_p ++ = current_table_ID;

	    if ( is_recursive
	             ( ih.goto_call_table_ID ) )
	    {
		sprintf
		    ( scan_error ( atom_length ),
		      "recursive CALL to table %d"
		      " in instruction %d executed"
		      " by table %d",
		      ih.goto_call_table_ID,
		      instruction_ID,
		      current_table_ID );
		return SCAN_ERROR;
	    }

	    current_table_ID =
		ih.goto_call_table_ID;

	    table_header & cath =
		* (table_header *)
		& program[current_table_ID];
	    assert ( cath.pctype == TABLE );
	    if ( cath.mode == MASTER
	         ||
		 cath.mode == ATOM )
	    {
		sprintf
		    ( scan_error ( atom_length ),
		      "CALL in"
		      " instruction %d executed by"
		      " table %d targets non-lexeme"
		      " table",
		      instruction_ID,
		      current_table_ID );
		return SCAN_ERROR;
	    }
	}
	else if ( op & GOTO )
	{
	    current_table_ID =
		ih.goto_call_table_ID;
	    table_header & cath =
		* (table_header *)
		& program[current_table_ID];
	    assert ( cath.pctype == TABLE );
	    if ( cath.mode == ATOM )

	    {
		sprintf
		    ( scan_error ( atom_length ),
		      "GOTO in"
		      " instruction %d executed by"
		      " table %d targets translation"
		      " table",
		      instruction_ID,
		      current_table_ID );
		return SCAN_ERROR;
	    }
	}

	if ( atom_length > 0 )
	{
	    next += atom_length;
	    loop_count = program.length;
	}
	else if ( -- loop_count == 0 )
	{
	    sprintf ( scan_error ( atom_length ),
		      "endless loop in scanner" );
	    return SCAN_ERROR;
	}
    }

    first = first;
    last = next - 1;

    uns32 type = output_type != 0 ?
                 output_type :
		 last_mode;

    switch ( type )
    {
    case MASTER:
    case SCAN_ERROR:
    {
	sprintf ( scan_error ( last - first + 1,
	                       first ),
		  "returning lexeme with bad type(%s)",
		  sbpmode ( type ) );
	return SCAN_ERROR;
    }
    }

    return type;
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
static const char * sbpmode ( uns32 mode )
{
    static char buffer[400];
    spmode ( buffer, mode );
    return buffer;
}

// See documentation above.
//
static char * scan_error ( uns32 length, uns32 next )
{
    char * p = error_message;
    p += sprintf ( p, "CURRENT_TABLE %u",
	              current_table_ID );
    if ( next < input_buffer.length )
	p += sprintf
	    ( p, " POSITION %u(%u)%u:",
	      input_buffer[next].line,
	      input_buffer[next].index,
	      input_buffer[next].column );
    else
        p += sprintf ( p, ":" );

    unsigned column = p - error_message;
    p += spinput ( p, next, next + length - 1,
		   column, true );
    * p ++ = '\n';
    return p;
}

// Reading
// -------

static uns32 default_read_input ( void )
{
    uns32 i = input_buffer.allocate ( 100 );
    uns32 endi = i + 100;
    while ( i < endi )
    {
        int c = read_input_istream->get();
	unsigned bytes_read = 1;

	if ( c == EOF )
	{
	    input_buffer.deallocate ( endi - i );
	    return ( endi - i < 100 );
	}

	c &= 0xFF;

	uns32 unicode = c;
	unsigned extra_characters = 0;
	if ( c <= 0x7F ) ; // Do nothing;
	else if ( c < 0xE0 )
	{
	    unicode &= 0x1F;
	    extra_characters = 1;
	}
	else if ( c < 0xF0 )
	{
	    unicode &= 0x0F;
	    extra_characters = 2;
	}
	else if ( c < 0xF8 )
	{
	    unicode &= 0x07;
	    extra_characters = 3;
	}
	else if ( c < 0xFC )
	{
	    unicode &= 0x03;
	    extra_characters = 4;
	}
	else if ( c < 0xFE )
	{
	    unicode &= 0x01;
	    extra_characters = 5;
	}
	else if ( c < 0xFF )
	{
	    unicode &= 0x00;
	    extra_characters = 6;
	}
	else
	{
	    unicode = 0xFFFFFFFF;
	}

	while ( extra_characters -- )
	{
	    c = read_input_istream->get();

	    if ( c == EOF )
	    {
	        unicode = 0xFFFFFFFF;
		break;
	    }

	    ++ bytes_read;
	    
	    c &= 0xFF;
	    if ( c < 0x80 || 0xBF < c )
	    {
	        unicode = 0xFFFFFFFF;
		break;
	    }
	    unicode <<= 6;
	    unicode |= ( c & 0x3F );
	}

	read_input_inchar.character = unicode;
	input_buffer[i++] = read_input_inchar;

	read_input_inchar.index += bytes_read;

	switch ( unicode )
	{
	case '\n':
	    ++ read_input_inchar.line;
	    read_input_inchar.column = 0;
	    read_input_inchar.index = 0;
	    break;

	case '\f':
	case '\v':
	    read_input_inchar.column = 0;
	    break;

	case '\t':
	    read_input_inchar.column +=
	        8 - read_input_inchar.column % 8;
	    break;

	default:
	    ++ read_input_inchar.column;
	    break;
	}
    }
    return 1;
}

LEX::uns32 (* LEX::read_input) ( void ) =
    ::default_read_input;
std::istream * LEX::read_input_istream =
    & std::cin;
LEX::inchar LEX::read_input_inchar;

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


unsigned LEX::spstring
	( char * buffer,
	  const char * string, unsigned n,
	  unsigned & column,
	  bool preface_with_space,
	  unsigned indent, unsigned line_length )
{
    char * p = buffer;
    if (    column + preface_with_space + n
         >= line_length )
    {
        * p ++ = '\n';
	while ( indent -- ) * p ++ = ' ';
	column = indent;
    }
    else if ( preface_with_space )
        * p ++ = ' ', ++ column;
    strncpy ( p, string, n );
    p += n, column += n;
    * p = 0;
    return p - buffer;
}

unsigned LEX::spinput
	( char * buffer, uns32 first, uns32 last,
	  unsigned & column,
	  bool preface_with_space,
	  unsigned indent, unsigned line_length )
{
    if ( first > last )
        return spstring ( buffer, "<empty>", 7,
	                  column,
			  preface_with_space,
			  indent, line_length );

    char * p = buffer;
    while ( first <= last )
    {
	uns32 c = input_buffer[first++].character;
	char buffer2[40];
	int n = spchar ( buffer2, c );
	p += spstring ( p, buffer2, n,
	                column,
			preface_with_space,
			indent, line_length );
	preface_with_space = false;
    }
    return p - buffer;
}

unsigned LEX::sptranslation
	( char * buffer,
	  unsigned & column,
	  bool preface_with_space,
	  unsigned indent, unsigned line_length )
{
    if ( translation_buffer.length == 0 )
        return spstring ( buffer, "<empty>", 7,
	                  column,
			  preface_with_space,
			  indent, line_length );

    char * p = buffer;
    for ( unsigned i = 0;
          i < translation_buffer.length; ++ i )
    {
	uns32 c = translation_buffer[i];
	char buffer2[40];
	int n = spchar ( buffer2, c );
	p += spstring ( p, buffer2, n,
	                column,
			preface_with_space,
			indent, line_length );
	preface_with_space = false;
    }
    return p - buffer;
}

unsigned LEX::splexeme
	( char * buffer,
	  uns32 first, uns32 last, uns32 type,
	  unsigned & column,
	  bool preface_with_space,
	  unsigned indent, unsigned line_length )
{
    char buffer2[500];
    char * p2 = buffer2;
    p2 += spmode ( p2, type );
    * p2 ++ = ':';

    if ( first <= last )
    {
	inchar & ic = input_buffer[first];
	p2 += sprintf ( p2, "%u(%u)%u:",
	                ic.line, ic.index, ic.column );
    }

    char * p = buffer;
    p += spstring ( p, buffer2, p2 - buffer2,
                    column,
		    preface_with_space,
		    indent, line_length );
    p += spinput ( p, first, last,
                   column, true, indent, line_length );
    if ( ! translation_is_exact ( first, last ) )
    {
        p += spstring ( p, "translated to:", 14,
	                column, true,
			indent, line_length );
	p += sptranslation ( p, column, true,
	                     indent, line_length );
    }
    return p - buffer;
}

unsigned LEX::sperroneous_atom
	( char * buffer,
	  uns32 first, uns32 last, uns32 type,
	  unsigned & column,
	  bool preface_with_space,
	  unsigned indent, unsigned line_length )
{
    char buffer2[500];
    char * p2 = buffer2;
    p2 += spmode ( p2, type );
    * p2 ++ = ':';

    if ( first <= last )
    {
	inchar & ic = input_buffer[first];
	p2 += sprintf ( p2, "%u(%u)%u:",
	                ic.line, ic.index, ic.column );
    }

    char * p = buffer;
    p += spstring ( p, buffer2, p2 - buffer2,
                    column,
		    preface_with_space,
		    indent, line_length );
    p += spinput ( p, first, last,
                   column, true, indent, line_length );
    return p - buffer;
}

bool LEX::translation_is_exact
	( uns32 first, uns32 last )
{
    uns32 i = 0;
    if (    translation_buffer.length
         != last - first + 1 )
        return false;
    while ( first <= last )
    {
        if (    input_buffer[first].character
	     != translation_buffer[i] )
	    return false;
	++ first, ++ i;
    }
    return true;
}

const char * const * LEX::type_name = NULL;
LEX::uns32 LEX::max_type = 0;

int LEX::spmode ( char * buffer, uns32 mode )
{

    if (    LEX::type_name != NULL
         && mode <= LEX::max_type
         && LEX::type_name[mode] != NULL )
        return sprintf
	    ( buffer, "%s", LEX::type_name[mode] );
        
    switch ( mode )
    {
    case MASTER:
	return sprintf ( buffer, "MASTER" );
    case SCAN_ERROR:
	return sprintf ( buffer, "SCAN_ERROR" );
    default:
	return sprintf
	    ( buffer, "TYPE (%u)", mode );
    }
}

ostream & operator <<
	( ostream & out, const LEX::pmode & pm )
{
    char buffer[40];
    spmode ( buffer, pm.mode );
    return out << buffer;
}

static const unsigned IDwidth = 12;
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

// See documentation above.
//
static uns32 print_instruction
    ( std::ostream & out, uns32 ID,
      unsigned indent = IDwidth )
{
    if ( ID == 0 ) return 0;
    if ( indent > 0 ) out << setw ( indent ) << "";

    instruction_header & h =
        * (instruction_header *) & program[ID];
    uns32 translate_to_length = 0;
    uns32 instruction_length =
        instruction_header_length;

    if ( h.operation & TRANSLATE_TO_FLAG )
    {
        translate_to_length =
	    LEX::translate_to_length ( h.operation );
	instruction_length += translate_to_length;
    }
    if ( h.pctype != INSTRUCTION )
    {
        out << "ILLEGAL INSTRUCTION TYPE ("
	    << h.pctype << ")" << endl;
	return 0xFFFFFFFF;
    }
    if ( ( ( h.operation & TRANSLATE_TO_FLAG ) != 0 )
	 +
         ( ( h.operation & TRANSLATE_HEX_FLAG ) != 0 )
	 +
         ( ( h.operation & TRANSLATE_OCT_FLAG ) != 0 )
	 > 1 ) out << "ILLEGAL: ";
    else
    if ( ( h.operation & REQUIRE ) != 0
         &&
	 ( h.operation & TRANSLATE_HEX_FLAG ) == 0
         &&
	 ( h.operation & TRANSLATE_OCT_FLAG ) == 0
         &&
	 ( h.operation & MATCH ) == 0 )
        out << "ILLEGAL: ";
    else
    if ( ( h.operation & ELSE ) != 0
         &&
	 ( h.operation & TRANSLATE_HEX_FLAG ) == 0
         &&
	 ( h.operation & TRANSLATE_OCT_FLAG ) == 0
         &&
	 ( h.operation & MATCH ) == 0
         &&
	 ( h.operation & REQUIRE ) == 0 )
        out << "ILLEGAL: ";
    else
    if ( ( ( h.operation & OUTPUT ) != 0 )
	 +
         ( ( h.operation & ERRONEOUS_ATOM ) != 0 )
	 > 1 ) out << "ILLEGAL: ";
    else
    if ( ( ( h.operation & GOTO ) != 0 )
	 +
         ( ( h.operation & CALL ) != 0 )
	 +
         ( ( h.operation & RETURN ) != 0 )
	 > 1 ) out << "ILLEGAL: ";
    else
    if ( ( h.operation & MATCH )
         &&
         h.atom_table_ID == 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & MATCH ) == 0
         &&
         h.atom_table_ID != 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & REQUIRE )
         &&
         h.require_dispatcher_ID == 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & REQUIRE ) == 0
         &&
         h.require_dispatcher_ID != 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & ELSE )
         &&
         h.else_instruction_ID == 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & ELSE ) == 0
         &&
         h.else_instruction_ID != 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & OUTPUT )
         &&
         h.output_error_type == 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & ERRONEOUS_ATOM )
         &&
         h.output_error_type == 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & OUTPUT ) == 0
	 &&
         ( h.operation & ERRONEOUS_ATOM ) == 0
         &&
         h.output_error_type != 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & GOTO )
         &&
         h.goto_call_table_ID == 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & CALL )
         &&
         h.goto_call_table_ID == 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & GOTO ) == 0
	 &&
         ( h.operation & CALL ) == 0
         &&
         h.goto_call_table_ID != 0 )
	out << "ILLEGAL: ";

    bool first = true;
#   define OUT ( first ? ( first = false, out ) : \
                         out << ", " )
    if ( h.operation & KEEP_FLAG )
        OUT << "KEEP("
	     << LEX::keep_length ( h.operation )
	     << ")";

    if ( h.operation & TRANSLATE_TO_FLAG )
    {
        translate_to_length =
	     LEX::translate_to_length ( h.operation );
        OUT << "TRANSLATE_TO(" << translate_to_length;
	if ( translate_to_length > 0 )
	{
	    out << ",";
	    uns32 n = translate_to_length;
	    for ( uns32 p =
	            ID + instruction_header_length;
		  0 < n; ++ p, -- n )
	        out << pchar ( program[p] );
	}
	out << ")";
    }

    if ( h.operation & TRANSLATE_HEX_FLAG )
        OUT << "TRANSLATE_HEX("
	    << LEX::prefix_length ( h.operation )
	    << ","
	    << LEX::postfix_length ( h.operation )
	    << ")";

    if ( h.operation & TRANSLATE_OCT_FLAG )
        OUT << "TRANSLATE_OCT("
	    << LEX::prefix_length ( h.operation )
	    << ","
	    << LEX::postfix_length ( h.operation )
	    << ")";

    if ( h.operation & MATCH )
        OUT << "MATCH("
	    << h.atom_table_ID << ")";

    if ( h.operation & REQUIRE )
        OUT << "REQUIRE("
	    << h.require_dispatcher_ID << ")";

    if ( h.operation & ERRONEOUS_ATOM )
        OUT << "ERRONEOUS_ATOM("
	    << h.output_error_type << ")";

    if ( h.operation & OUTPUT )
        OUT << "OUTPUT("
	    << h.output_error_type << ")";

    if ( h.operation & GOTO )
        OUT << "GOTO(" << h.goto_call_table_ID << ")";

    if ( h.operation & CALL )
        OUT << "CALL(" << h.goto_call_table_ID << ")";

    if ( h.operation & RETURN )
        OUT << "RETURN";

    if ( h.operation & FAIL )
        OUT << "FAIL";

    if ( h.operation & ELSE )
    {
        OUT << "ELSE:" << endl;
	print_instruction
	    ( out, h.else_instruction_ID, indent );
    }

    if ( first ) out << "ACCEPT" << endl;
    else out << endl;
#   undef OUT

    return instruction_length;
}

// Iterator that prints out a list of characters within
// LEX::line_length columns.  `nonempty' if the list is
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
	// line; line_length - indent columns are
	// available for each line.

    uns32 c1, c2;
        // If not empty then the range c1-c2 (or just c1
	// if c1 == c2) needs to be printed.  This is
	// delayed to allow c2 to grow.

    pclist ( int indent, bool user_indent = false )
	: indent ( indent ), user_indent ( user_indent )
    {
	assert ( line_length - indent >= 30 );
        empty = true;
	columns = line_length - indent;
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

	if ( columns == line_length - indent )
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
	    columns = line_length - indent - needed;
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
	    * ( h.max_ctype + 1 );

    cout << INDENT << "Break Elements: "
	 << h.break_elements << endl;
    cout << INDENT << "Max Break Elements: "
	 << h.max_break_elements << endl;
    cout << INDENT << "Max CType: "
	 << h.max_ctype << endl;

    // Construct tmap so that t2 = tmap[t1] iff t2 is
    // the smallest ctype such that mep[t2] == mep[t1].
    //
    uns32 tmap[h.max_ctype+1];
    for ( uns32 t1 = 0; t1 <= h.max_ctype; ++ t1 )
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
    for ( uns32 t = 0; t <= h.max_ctype; ++ t )
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
		uns32 ctype = mh.singleton_ctype;
		if ( ctype != 0 )
		{
		    if ( tmap[ctype] == t )
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
	    ( cout, mep[t].instruction_ID,
	            IDwidth + 4 );
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
	cout << INDENT << "Initial Table ID: "
	     << h.initial_table_ID << endl;
	return program_header_length;
    }
    case TABLE:
    {
	cout << pID ( ID ) << "TABLE" << endl;
	table_header & h =
	    * (table_header *) & program[ID];
	cout << INDENT << "Mode: "
	     << pmode ( h.mode ) << endl;
	cout << INDENT << "Dispatcher ID: "
	     << h.dispatcher_ID << endl;
	if ( cooked )
	    print_instruction
		( cout, h.instruction_ID, IDwidth );
	else
	    cout << INDENT << "Instruction ID: "
		 << h.instruction_ID << endl;
	return table_header_length;
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
	cout << INDENT << "Max CType: "
	     << h.max_ctype << endl;
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
	cout << INDENT << "Map:   CType: "
	     << setw ( 16 ) << "dispatcher_ID"
	     << setw ( 16 ) << "instruction_ID"
	     << endl;
	uns32 t;
	for ( p = ID + length, t = 0;
	      t <= h.max_ctype;
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
	        * ( h.max_ctype + 1 );
	return length;
    }
    case INSTRUCTION:
    {
	cout << pID ( ID );
	return print_instruction ( cout, ID, 0 );
    }
    case TYPE_MAP:
    {
	cout << pID ( ID ) << "TYPE_MAP" << endl;
	type_map_header & h =
	    * (type_map_header *) & program[ID];
	uns32 length = type_map_header_length;
	if ( h.singleton_ctype > 0 )
	{
	    cout << setw ( IDwidth + 4 )
		 << h.singleton_ctype
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
	cout << pID ( ID ) << "ILLEGAL COMPONENT TYPE("
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
		 & LEX::TRANSLATE_TO_FLAG )
		ID += LEX::translate_to_length
			    ( h.operation );
	    continue;
	}
	case TYPE_MAP:
	{
	    type_map_header & h =
		* (type_map_header *) & program[ID];
	    ID += type_map_header_length;
	    if ( h.singleton_ctype == 0 )
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
