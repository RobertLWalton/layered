// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Jan  2 17:40:34 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Data
//	Program Construction
//	Scanning
//	Reading
//	Input Files
//	Printing

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_program_data.h>
# include <min_os.h>
# include <iostream>
# include <iomanip>
# include <cstdlib>
# include <cstring>
# include <cstdio>
# include <cerrno>
# include <cassert>
# define LEX ll::lexeme
using std::cout;
using std::endl;
using std::setw;
using std::ios;
using std::ostream;
using namespace LEX;
using namespace LEX::program_data;

#define FOR(i,n) for ( uns32 i = 0; i < (n); ++ i )

// Read a UTF-8 encoded UNCODE character c from a buffer
// and return the number of buffer bytes read.
//
// On an invalid encoding, return c == 0xFFFFFFFF and
// either the number of bytes read before the first
// invalid byte, or 1 if the first byte is invalid.
//
int read_utf8 ( min::uns32 & c, const char * buffer )
{
    uns32 unicode = buffer[0] & 0xFF;
    int bytes_read = 1;
    if ( unicode <= 0x7F ) ; // Do nothing;
    else if ( unicode < 0xC0 )
    {
        c = 0xFFFFFFFF;
	return 1;
    }
    else if ( unicode < 0xE0 )
    {
	unicode &= 0x1F;
	bytes_read = 2;
    }
    else if ( unicode < 0xF0 )
    {
	unicode &= 0x0F;
	bytes_read = 3;
    }
    else if ( unicode < 0xF8 )
    {
	unicode &= 0x07;
	bytes_read = 4;
    }
    else if ( unicode < 0xFC )
    {
	unicode &= 0x03;
	bytes_read = 5;
    }
    else if ( unicode < 0xFE )
    {
	unicode &= 0x01;
	bytes_read = 6;
    }
    else if ( unicode < 0xFF )
    {
	unicode &= 0x00;
	bytes_read = 7;
    }
    else
    {
	c = 0xFFFFFFFF;
	return 1;
    }

    for ( int i = 1; i < bytes_read; ++ i )
    {
	unsigned ch = buffer[i] & 0xFF;
	if ( ch < 0x80 || 0xBF < ch )
	{
	    c = 0xFFFFFFFF;
	    return i;
	}
	unicode <<= 6;
	unicode |= ( ch & 0x3F );
    }

    c = unicode;
    return bytes_read;
}

// Data
// ----

static uns32 scanner_stub_disp[] =
    { min::DISP ( & LEX::scanner_struct::program ),
      min::DISP ( & LEX::scanner_struct::input_buffer ),
      min::DISP ( & LEX::scanner_struct
                       ::translation_buffer ),
      min::DISP ( & LEX::scanner_struct::input_file ),
      min::DISP_END };

static min::packed_struct<LEX::scanner_struct>
       scanner_type
           ( "ll::lexeme::scanner_type",
	     NULL, ::scanner_stub_disp );

static uns32 file_gen_disp[] =
    { min::DISP ( & LEX::file_struct::file_name ),
      min::DISP_END };
static uns32 file_stub_disp[] =
    { min::DISP ( & LEX::file_struct::data ),
      min::DISP_END };

static min::packed_vec<LEX::uns32,LEX::file_struct>
       file_type
           ( "ll::lexeme::file_type",
	     ::file_gen_disp, ::file_stub_disp );

static min::packed_vec<char> char_vec_type
           ( "ll::lexeme::char_vec_type" );
static min::packed_vec<LEX::uns32> uns32_vec_type
           ( "ll::lexeme::uns32_vec_type" );
static min::packed_vec<LEX::inchar> inchar_vec_type
           ( "ll::lexeme::inchar_vec_type" );

static min::static_stub<1> default_scanner_stub;
LEX::scanner_ptr & LEX::default_scanner =
    * (LEX::scanner_ptr *) & default_scanner_stub[0];

// Program Construction
// ------- ------------

uns32 LEX::create_table
	( uns32 mode, scanner_ptr scanner )
{
    program_ptr program = scanner->program;

    if ( mode != ATOM && mode != MASTER )
    {
	program_header & ph =
	    * (program_header *) & program[0];
	assert (    ph.max_type == 0
	         || mode <= ph.max_type );
    }

    uns32 ID = program->length;
    table_header & h = * (table_header *)
        & min::push ( program, table_header_length );
    assert ( sizeof ( h ) ==   table_header_length
                             * sizeof ( uns32 ) );
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

uns32 LEX::table_mode
	( uns32 ID, scanner_ptr scanner )
{
    program_ptr program = scanner->program;

    table_header & h = * (table_header *) & program[ID];
    return h.mode;
}

void LEX::create_program
	( const char * const * type_name,
	  uns32 max_type,
	  scanner_ptr scanner )
{
    program_ptr program = scanner->program;

    if ( program == NULL_STUB )
        program = scanner->program =
	    uns32_vec_type.new_gen();
    else
    {
	min::pop ( program, program->length );
	min::resize ( program, 1000 );
    }

    uns32 origin = 4 * (   program_header_length
                         + 1 + max_type );
    uns32 length = origin;
    for ( uns32 t = 0; t <= max_type; ++ t )
    {
	if ( type_name == NULL || type_name[t] == NULL )
	    continue;
	length += strlen ( type_name[t] ) + 1;
    }
    uns32 header_length = ( length + 3 ) / 4;

    uns32 ID = program->length;
    assert ( ID == 0 );
    program_header & h =
        * (program_header *)
	& min::push ( program, header_length );
    assert ( sizeof ( h ) ==   program_header_length
                             * sizeof ( uns32 ) );
    h.pctype = PROGRAM;
    h.max_type = max_type;
    h.component_length = header_length;

    for ( uns32 t = 0; t <= max_type; ++ t )
    {
	if ( type_name == NULL || type_name[t] == NULL )
	{
	    program[program_header_length + t] = 0;
	    continue;
	}
	program[program_header_length + t] = origin;
	strcpy ( (char *) & program[0] + origin,
	         type_name[t] );
	origin += strlen ( type_name[t] ) + 1;
    }
}

uns32 LEX::create_dispatcher
	( uns32 max_breakpoints,
	  uns32 max_ctype,
	  scanner_ptr scanner )
{
    program_ptr program = scanner->program;

    uns32 length =
          dispatcher_header_length
	+   break_element_length
	  * ( max_breakpoints + 1 )
	+   map_element_length
	  * ( max_ctype + 1 );
    uns32 ID = program->length;
    dispatcher_header & h =
        * (dispatcher_header *)
	& min::push ( program, length );
    assert ( sizeof ( h ) ==   dispatcher_header_length
                             * sizeof ( uns32 ) );
    assert (    sizeof ( break_element )
             ==   break_element_length
                * sizeof ( uns32 ) );
    assert (    sizeof ( map_element )
             ==   map_element_length
                * sizeof ( uns32 ) );
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
	( uns32 cmin, uns32 cmax,
	  uns8 * map,
	  scanner_ptr scanner )
{
    program_ptr program = scanner->program;

    assert ( cmax >= cmin );
    uns32 length = cmax - cmin + 1;
    uns32 ID = program->length;
    type_map_header & h =
        * (type_map_header *)
        & min::push ( program,
    	                type_map_header_length
	              + ( length + 3 ) / 4 );
    assert ( sizeof ( h ) ==   type_map_header_length
                             * sizeof ( uns32 ) );
    h.pctype = TYPE_MAP;
    h.cmin = cmin;
    h.cmax = cmax;
    h.singleton_ctype = 0;
    memcpy ( & h + 1, map, length );
    return ID;
}

uns32 LEX::create_type_map
	( uns32 cmin, uns32 cmax,
	  uns32 ctype,
	  scanner_ptr scanner )
{
    program_ptr program = scanner->program;

    assert ( cmax >= cmin );
    uns32 ID = program->length;
    type_map_header & h =
        * (type_map_header *)
        & min::push ( program, type_map_header_length );
    assert ( sizeof ( h ) ==   type_map_header_length
                             * sizeof ( uns32 ) );
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
	  uns32 erroneous_atom_type,
	  uns32 output_type,
	  uns32 goto_table_ID,
	  uns32 call_table_ID,
	  scanner_ptr scanner )
{
    program_ptr program = scanner->program;

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

    assert ( ( ( operation & GOTO ) != 0 )
             +
	     ( ( operation & RETURN ) != 0 )
	     <= 1 );

    assert ( ( ( operation & CALL ) != 0 )
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

    if ( operation & ERRONEOUS_ATOM )
        assert ( erroneous_atom_type != 0 );
    else
        assert ( erroneous_atom_type == 0 );

    if ( operation & OUTPUT )
    {
        assert ( output_type != 0 );

	program_header & ph =
	    * (program_header *) & program[0];
	assert (    ph.max_type == 0
		 || output_type <= ph.max_type );
    }
    else
        assert ( output_type == 0 );

    if ( operation & GOTO )
        assert ( goto_table_ID != 0 );
    else
        assert ( goto_table_ID == 0 );

    if ( operation & CALL )
        assert ( call_table_ID != 0 );
    else
        assert ( call_table_ID == 0 );

    uns32 ID = program->length;
    instruction_header & h =
        * (instruction_header *)
	& min::push ( program,
    	                instruction_header_length
	              + translate_to_length );
    assert ( sizeof ( h ) ==   instruction_header_length
                             * sizeof ( uns32 ) );
    h.pctype = INSTRUCTION;
    h.operation = operation;
    h.atom_table_ID = atom_table_ID;
    h.require_dispatcher_ID = require_dispatcher_ID;
    h.else_instruction_ID = else_instruction_ID;
    h.erroneous_atom_type = erroneous_atom_type;
    h.output_type = output_type;
    h.goto_table_ID = goto_table_ID;
    h.call_table_ID = call_table_ID;

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
	( scanner_ptr scanner,
	  uns32 dispatcher_ID,
	  uns32 type_map_ID )
{
    program_ptr program = scanner->program;

    dispatcher_header & dh =
        * (dispatcher_header *)
	& program[dispatcher_ID];
    assert ( dh.pctype == DISPATCHER );
    type_map_header & mh =
        * (type_map_header *)
	& program[type_map_ID];
    assert ( mh.pctype == TYPE_MAP );

    break_element * bep =
        (break_element *)
	& program[  dispatcher_ID
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
        sprintf ( scanner->error_message,
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
        sprintf ( scanner->error_message,
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
        sprintf ( scanner->error_message,
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
	  uns32 component_ID,
	  scanner_ptr scanner )
{
    program_ptr program = scanner->program;

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
		sprintf ( scanner->error_message,
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
		sprintf ( scanner->error_message,
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
		   ( scanner, target_ID, component_ID );
    else
	assert ( ! "bad attach component pctypes" );
}

uns32 LEX::attach
	( uns32 target_ID,
	  uns32 ctype,
	  uns32 component_ID,
	  scanner_ptr scanner )
{
    program_ptr program = scanner->program;

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
	    sprintf ( scanner->error_message,
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
	    sprintf ( scanner->error_message,
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

inline uns32 conv ( uns32 & v )
{
    uns32 b0 = v >> 24;
    uns32 b1 = ( v >> 16 ) & 0xFF;
    uns32 b2 = ( v >>  8 ) & 0xFF;
    uns32 b3 = v & 0xFF;
    v = ( b3 << 24 ) + ( b2 << 16 )
      + ( b1 << 8 ) + b0;
    return v;
}

bool LEX::convert_program_endianhood
	( scanner_ptr scanner )
{
    program_ptr program = scanner->program;

    if ( program[0] == PROGRAM ) return true;

    uns32 ID = 0;
#   define NEXT conv(program[ID++])
    while ( ID < program->length )
    {
        uns32 cID = ID;

        switch ( NEXT )
	{
	case PROGRAM:
	{
	    FOR(i,program_header_length-1) NEXT;
	    program_header & ph =
	        * (program_header *)
		& program[cID];
	    FOR(i,ph.max_type+1) NEXT;
	    ID = cID + ph.component_length;
	    break;
	}
	case TABLE:
	{
	    FOR(i,table_header_length-1) NEXT;
	    break;
	}
	case DISPATCHER:
	{
	    FOR(i,dispatcher_header_length-1) NEXT;

	    dispatcher_header & dh =
		* (dispatcher_header *) & program[cID];
	    FOR(i,  break_element_length
		  * dh.max_break_elements) NEXT;
	    FOR(i,  map_element_length
		  * ( dh.max_ctype + 1 )) NEXT;
	    break;
	}
	case TYPE_MAP:
	{
	    FOR(i,type_map_header_length-1) NEXT;

	    type_map_header & tmh =
		* (type_map_header *) & program[cID];
	    if ( tmh.singleton_ctype == 0 )
		ID += ( tmh.cmax - tmh.cmin + 4 ) / 4;
	    break;
	}
	case INSTRUCTION:
	{
	    FOR(i,instruction_header_length-1) NEXT;
	    
	    instruction_header & ih =
		* (instruction_header *)
		& program[cID];
	    if (   ih.operation
		 & LEX::TRANSLATE_TO_FLAG )
		FOR(i,LEX::translate_to_length
			    ( ih.operation ))
		    NEXT;
	    break;
	}
	default:
	    return false;
	}
    }
#   undef NEXT
    return true;
}

// Scanning
// --------

// Return true if table_ID is in return_stack.
//
static bool is_recursive
	( scanner_ptr scanner, uns32 table_ID )
{
    for ( uns32 i = 0;
          i < scanner->return_stack_p; ++ i )
    {
        if ( scanner->return_stack[i] == table_ID )
	    return true;
    }
    return false;
}

// We assume the program is well formed, in that an
// XXX_ID actually points at a program component of
// pctype XXX.  We check this with asserts (the attach
// statements check this).  Everything else found
// wrong with the program is a SCAN_ERROR.

void LEX::init_scanner
	( scanner_ptr & scanner,
	  program_ptr program )
{
    min::static_stub<1> scanner_vec;

    if ( scanner == NULL_STUB )
    {
        scanner = scanner_type.new_stub();
	scanner_vec[0] = scanner;

	scanner->input_buffer =
	    inchar_vec_type.new_gen();
	scanner->translation_buffer =
	    uns32_vec_type.new_gen();
	scanner->program = NULL_STUB;
	scanner->print_mode = default_print_mode;
	scanner->line_length = 72;
	scanner->indent = 4;
	scanner->read_input = default_read_input;
	scanner->input_file = create_file();
	init_stream
	    ( scanner->input_file,
	      std::cin, "standard input", 0 );
	scanner->scan_trace_out = NULL;
	scanner->erroneous_atom = NULL;
    }
    else
    {
	scanner_vec[0] = scanner;
	min::pop ( scanner->input_buffer,
	           scanner->input_buffer->length );
	min::resize ( scanner->input_buffer, 1000 );
	min::pop
	    ( scanner->translation_buffer,
	      scanner->translation_buffer->length );
	min::resize
	    ( scanner->translation_buffer, 1000 );
    }

    if ( program != NULL_STUB )
	scanner->program = program;

    scanner->next_position.line = 0;
    scanner->next_position.index = 0;
    scanner->next_position.column = 0;
    scanner->next_position.character = 0;

    scanner->next = 0;

    if ( scanner->program != NULL_STUB )
    {
	assert ( scanner->program[0] == PROGRAM );
	program_header & h = * (program_header *)
			     & scanner->program[0];
	assert (    scanner->program[h.initial_table_ID]
		 == TABLE );
	scanner->current_table_ID = h.initial_table_ID;
    }
    else
	scanner->current_table_ID = 0;

    scanner->return_stack_p = 0;
    memset ( scanner->return_stack, 0,
             sizeof ( scanner->return_stack ) );

    memset ( scanner->error_message, 0,
             sizeof ( scanner->error_message ) );

    memset ( scanner->work, 0,
             sizeof ( scanner->work ) );
}

// Write the beginning of a scan error message into
// error message and return a pointer to the next
// location in error message.  Usage is:
//
//    sprintf
//      ( scan_error ( scanner, length, next ), ... )
//
// `length' is the number of characters scanned after
// `next'.  `next' defaults to scanner->next.
//
static char * scan_error
        ( scanner_ptr scanner,
	  uns32 length, uns32 next );
inline char * scan_error
        ( scanner_ptr scanner,
	  uns32 length )
{
    return scan_error
        ( scanner, length, scanner->next );
}

// Write a character using spchar to scanner->work
// and return a pointer to scanner->work.
//
static const char * sbpchar
	( scanner_ptr scanner, uns32 c );

// Write a mode or return value to scanner->work and
// return a pointer to scanner->work.
//
static const char * sbpmode
	( scanner_ptr scanner, uns32 mode );

// Print the instruction at scanner->program[ID] with
// the given indent and endl, if ID is non-zero, and
// return ID repositioned just after instuction.  How-
// ever, if scanner->program[ID] does not ==
// INSTRUCTION, print ILLEGAL instruction message and
// return 0.
//
// If ID == 0 do nothing but return 0.
//
static uns32 print_instruction
    ( scanner_ptr scanner,
      std::ostream & out, uns32 ID, unsigned indent );

# define TOUT if ( scanner->scan_trace_out ) \
                 (* scanner->scan_trace_out)

// Given a dispatcher_ID and a character c return the
// ctype that the dispatcher maps c onto.
//
static uns32 ctype ( scanner_ptr scanner,
                     uns32 dispatcher_ID, uns32 c )
{
    program_ptr program = scanner->program;

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
static uns32 scan_atom
    ( scanner_ptr scanner, uns32 & atom_length )
{
    program_ptr program =
        scanner->program;
    min::packed_vec_insptr<inchar> input_buffer =
        scanner->input_buffer;
    min::packed_vec_insptr<uns32> translation_buffer =
        scanner->translation_buffer;

    const uns32 SCAN_ERROR = 0;
        // Local version of SCAN_ERROR.

    TOUT << "Start atom scan: table = "
	 << scanner->current_table_ID << endl;

    table_header & cath =
	* (table_header *)
	& program[scanner->current_table_ID];

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
    uns32 tnext = translation_buffer->length;
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

	if (    scanner->next + length
	     >= input_buffer->length
	     &&
	     ! (*scanner->read_input) ( scanner ) )
	    break; // End of file.

	assert
	    (   scanner->next + length
	      < input_buffer->length );
	uns32 c =
	    input_buffer[scanner->next + length]
	                .character;
	++ length;

	uns32 ctype =
	    ::ctype ( scanner, dispatcher_ID, c );

	dispatcher_header & dh =
	    * (dispatcher_header *)
	    & program[dispatcher_ID];

	if ( ctype > dh.max_ctype )
	{
	    sprintf ( scan_error ( scanner, length ),
		      "ctype %u computed for character"
		      " %s is too large for dispatcher"
		      " %u",
		      ctype, sbpchar ( scanner, c ),
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
	    sprintf ( scan_error ( scanner, length ),
		      "no instruction found" );
	    return SCAN_ERROR;
	}

	if ( scanner->scan_trace_out )
	    print_instruction
	        ( scanner,
	          * scanner->scan_trace_out,
		  instruction_ID, 2 );

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
		    ( scan_error
		        ( scanner, atom_length ),
		      "MATCH in"
		      " instruction %d executed by"
		      " table %d targets"
		      " non-atom table",
		      instruction_ID,
		      scanner->current_table_ID );
		return SCAN_ERROR;
	    }
	    else if (    scanner->return_stack_p
		      == LEX::return_stack_size )
	    {
		sprintf
		    ( scan_error
		        ( scanner, atom_length ),
		      "MATCH in"
		      " instruction %d executed by"
		      " table %d but return stack is"
		      " full",
		      instruction_ID,
		      scanner->current_table_ID );
		return SCAN_ERROR;
	    }

	    scanner->return_stack
	        [scanner->return_stack_p++] =
	            scanner->current_table_ID;

	    if ( is_recursive
	             ( scanner, ih.atom_table_ID ) )
	    {
		sprintf
		    ( scan_error
		        ( scanner, atom_length ),
		      "recursive MATCH to table %d"
		      " in instruction %d executed"
		      " by table %d",
		      ih.atom_table_ID,
		      instruction_ID,
		      scanner->current_table_ID );
		return SCAN_ERROR;
	    }

	    scanner->current_table_ID =
	        ih.atom_table_ID;
	    uns32 tinstruction_ID =
	        scan_atom ( scanner, keep_length );

	    scanner->current_table_ID =
		scanner->return_stack
		    [--scanner->return_stack_p];

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
		min::pop
		    ( translation_buffer,
		        translation_buffer->length
		      - tnext );
	}

	if ( ! fail && ( op & KEEP_FLAG ) )
	{
	    // Due to possible MATCH, actual atom
	    // length is in keep_length.
	    //
	    uns32 keep = LEX::keep_length ( op );
	    if ( keep > keep_length )
	    {
		sprintf ( scan_error
		              ( scanner, length ),
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
	    uns32 p = scanner->next
		    + LEX::prefix_length ( op );
	    uns32 endp = scanner->next + keep_length
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
		min::push(translation_buffer) = tc;
	}
	else if ( op & TRANSLATE_TO_FLAG )
	{
	    uns32 translate_to_length =
		LEX::translate_to_length ( op );
	    if ( translate_to_length > 0 )
		min::push ( translation_buffer,
			    translate_to_length,
			    (uns32 *) ( & ih + 1 ) );
	}
	else if ( ! ( op & MATCH ) )
	{
	    uns32 p = scanner->next;
	    for ( uns32 i = 0; i < keep_length; ++ i )
		min::push(translation_buffer) =
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
				    ->length );
		    break;
		}
		assert (    program[dispatcher_ID]
			 == DISPATCHER );

		if (    tnext + tlength
		     >= translation_buffer->length )
		{
		    fail = true;
		    break;
		}

		uns32 c = translation_buffer
			      [tnext + tlength];
		++ tlength;

		uns32 ctype =
		    ::ctype ( scanner,
		              dispatcher_ID, c );

		dispatcher_header & dh =
		    * (dispatcher_header *)
		    & program[dispatcher_ID];

		if ( ctype > dh.max_ctype )
		{
		    sprintf ( scan_error
		                ( scanner, length ),
			      "ctype %u computed for"
			      " character %s is too "
			      " large for dispatcher"
			      " %u",
			      ctype,
			      sbpchar ( scanner, c ),
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
	    min::pop ( translation_buffer,
		         translation_buffer->length
		       - tnext );
	    if ( op & ELSE )
	    {
		instruction_ID =
		    ih.else_instruction_ID;
		if ( instruction_ID == 0 )
		{
		    sprintf ( scan_error
		                ( scanner, length ),
			      "no instruction for ELSE"
			      " in failed instruction"
			      " %d executed by table"
			      " %d",
			      instruction_ID,
			      scanner->current_table_ID
			    );
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
		sprintf ( scan_error
			      ( scanner, length ),
			  "no ELSE in failed"
			  " instruction %d executed by"
			  " table %d",
			  instruction_ID,
			  scanner->current_table_ID );
		return SCAN_ERROR;
	    }
	}

	// We are at un-failed instruction of
	// instruction group.
	//
	atom_length = keep_length;

	return instruction_ID;
    }
    abort();
}

uns32 LEX::scan ( uns32 & first, uns32 & last,
                  scanner_ptr scanner )
{
    program_ptr program =
        scanner->program;
    min::packed_vec_insptr<inchar> input_buffer =
        scanner->input_buffer;
    min::packed_vec_insptr<uns32> translation_buffer =
        scanner->translation_buffer;

    if (    scanner->next
         >= inchar_vec_type.max_increment )
    {
        // If next has gotten to be as large as
	// max_increment, shift the input_buffer
	// down, eliminating characters in lexemes
	// already returned.
	//
        memmove ( & input_buffer[0],
	          & input_buffer[scanner->next],
		    (   input_buffer->length
		      - scanner->next )
		  * sizeof ( LEX::inchar ) );
	min::pop ( input_buffer, scanner->next );
	scanner->next = 0;
    }

    // Initialize first and translation buffer.
    //
    first = scanner->next;
    min::pop ( translation_buffer,
               translation_buffer->length );

    // We scan atoms until we get to a point where the
    // table is to be changed from a table with mode !=
    // MASTER to one with mode == MASTER and next !=
    // first, or `output_type' is set (in this case next
    // == first is allowed).
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
    // ->length, whenever we find an atom.  If it goes
    // to zero we must be in a loop changing tables and
    // not finding an atom.
    // 
    uns32 output_type = 0;
    uns32 last_mode = MASTER;
    scanner->return_stack_p = 0;
    uns32 loop_count = program->length;
        // Set to max number of tables in order to
	// detect endless loops.
    while ( true )
    {
        // Scan next atom of current lexeme.

	table_header & cath =
	    * (table_header *)
	    & program[scanner->current_table_ID];

	// First check lexeme ending conditions.

	if ( output_type != 0 )
	{
	    if ( cath.mode != MASTER )
	    {
	        sprintf ( scan_error ( scanner,
		                       last + 1 - first,
		                       first ),
		          "attempt to OUTPUT when the"
			  " next table %d is not a"
			  " master table",
			  scanner->current_table_ID );
		return SCAN_ERROR;
	    }

	    break;
	}

	if ( last_mode != MASTER
	     &&
	     cath.mode == MASTER
	     &&
	     first != scanner->next )
	    break;

	last_mode = cath.mode;

	// Reset return stack if current mode is master.

	if ( cath.mode == MASTER )
	    scanner->return_stack_p = 0;

	// Scan atom.  Return atom length and
	// instruction.
	//
	uns32 atom_length;

	uns32 instruction_ID =
	    scan_atom ( scanner, atom_length );

	if ( instruction_ID == 0 )
	    return SCAN_ERROR;

	instruction_header & ih =
	    * (instruction_header *)
	    & program[instruction_ID];
        uns32 op = ih.operation;

	if ( op & ERRONEOUS_ATOM )
	{
	    if ( atom_length == 0 )
	    {
		sprintf ( scan_error
		              ( scanner, atom_length ),
			  "ERRONEOUS_ATOM in"
			  " instruction %d executed"
			  " by table %d but atom is of"
			  " zero length",
			  instruction_ID,
			  scanner->current_table_ID );
		return SCAN_ERROR;
	    }
	    else if ( scanner->erroneous_atom == NULL )
	    {
		sprintf ( scan_error
		              ( scanner, atom_length ),
			  "ERRONEOUS_ATOM in"
			  " instruction %d executed"
			  " by table %d but"
			  " erroneous_atom function"
			  " does not exist",
			  instruction_ID,
			  scanner->current_table_ID );
		return SCAN_ERROR;
	    }
	    else
	    {
		(*scanner->erroneous_atom)
		    ( scanner->next,
		      scanner->next + atom_length - 1,
		      ih.erroneous_atom_type,
		      scanner );
	    }
	}

	if ( op & OUTPUT )
	    output_type = ih.output_type;

	if ( op & RETURN )
	{
	    if (    scanner->return_stack_p
	         == LEX::return_stack_size )
	    {
		sprintf
		    ( scan_error
			  ( scanner, atom_length ),
		      "RETURN in instruction %d"
		      " executed by table %d but"
		      " return stack is empty",
		      instruction_ID,
		      scanner->current_table_ID );
		return SCAN_ERROR;
	    }
	    scanner->current_table_ID =
		scanner->return_stack
		    [--scanner->return_stack_p];
	    assert (    program
	                    [scanner->current_table_ID]
		     == TABLE );
	}
	else if ( op & GOTO )
	{
	    scanner->current_table_ID =
	        ih.goto_table_ID;
	    table_header & cath =
		* (table_header *)
		& program[scanner->current_table_ID];
	    assert ( cath.pctype == TABLE );
	    if ( cath.mode == ATOM )

	    {
		sprintf
		    ( scan_error
			  ( scanner, atom_length ),
		      "GOTO in"
		      " instruction %d executed by"
		      " table %d targets atom"
		      " table",
		      instruction_ID,
		      scanner->current_table_ID );
		return SCAN_ERROR;
	    }
	}

	if ( op & CALL )
	{
	    if (    scanner->return_stack_p
		 == LEX::return_stack_size )
	    {
		sprintf
		    ( scan_error
			  ( scanner, atom_length ),
		      "CALL in"
		      " instruction %d executed by"
		      " table %d but return stack is"
		      " full",
		      instruction_ID,
		      scanner->current_table_ID );
		return SCAN_ERROR;
	    }

	    scanner->return_stack
	        [scanner->return_stack_p++] =
	        scanner->current_table_ID;

	    if ( is_recursive
	             ( scanner, ih.call_table_ID ) )
	    {
		sprintf
		    ( scan_error
			  ( scanner, atom_length ),
		      "recursive CALL to table %d"
		      " in instruction %d executed"
		      " by table %d",
		      ih.call_table_ID,
		      instruction_ID,
		      scanner->current_table_ID );
		return SCAN_ERROR;
	    }

	    scanner->current_table_ID =
	        ih.call_table_ID;

	    table_header & cath =
		* (table_header *)
		& program[scanner->current_table_ID];
	    assert ( cath.pctype == TABLE );
	    if ( cath.mode == MASTER
	         ||
		 cath.mode == ATOM )
	    {
		sprintf
		    ( scan_error
			  ( scanner, atom_length ),
		      "CALL in"
		      " instruction %d executed by"
		      " table %d targets non-lexeme"
		      " table",
		      instruction_ID,
		      scanner->current_table_ID );
		return SCAN_ERROR;
	    }
	}

	if ( atom_length > 0 )
	{
	    scanner->next += atom_length;
	    loop_count = program->length;
	}
	else if ( -- loop_count == 0 )
	{
	    sprintf ( scan_error
			  ( scanner, atom_length ),
		      "endless loop in scanner" );
	    return SCAN_ERROR;
	}
    }

    first = first;
    last = scanner->next - 1;

    uns32 type = output_type != 0 ?
                 output_type :
		 last_mode;

    switch ( type )
    {
    case MASTER:
    case ATOM:
    case SCAN_ERROR:
    {
	sprintf ( scan_error ( scanner,
	                       last - first + 1,
	                       first ),
		  "returning lexeme with bad type(%s)",
		  sbpmode ( scanner, type ) );
	return SCAN_ERROR;
    }
    }

    return type;
}

// See documentation above.
//
static const char * sbpchar
	( scanner_ptr scanner, uns32 c )
{
    spchar ( scanner->work, c );
    return scanner->work;
}

// See documentation above.
//
static const char * sbpmode
	( scanner_ptr scanner, uns32 mode )
{
    spmode ( scanner->work, mode, scanner );
    return scanner->work;
}

// See documentation above.
//
static char * scan_error
        ( scanner_ptr scanner,
	  uns32 length, uns32 next )
{
    char * p = scanner->error_message;
    p += sprintf ( p, "CURRENT_TABLE %u",
	              scanner->current_table_ID );
    if ( scanner->next < scanner->input_buffer->length )
	p += sprintf
	    ( p, " POSITION %u(%u)%u:",
	      scanner->input_buffer[next].line,
	      scanner->input_buffer[next].index,
	      scanner->input_buffer[next].column );
    else
        p += sprintf ( p, ":" );

    unsigned column = p - scanner->error_message;
    p += spinput ( p, next, next + length - 1,
		   column, true, scanner );
    * p ++ = '\n';
    return p;
}

// Reading
// -------

bool LEX::default_read_input
	( LEX::scanner_ptr scanner )
{
    LEX::file_ptr file = scanner->input_file;

    LEX::uns32 offset = LEX::next_line ( file );
    if ( offset == LEX::NO_LINE ) return false;

    min::packed_vec_insptr<LEX::inchar> input_buffer =
        scanner->input_buffer;

    LEX::inchar & ic = scanner->next_position;
    ic.line = file->line_number - 1;
    ic.index = 0;
    ic.column = 0;

    while ( file->data[offset] != 0 )
    {
	uns32 unicode;
	int bytes_read =
	    read_utf8 ( unicode, & file->data[offset] );
	offset += bytes_read;

	ic.character = unicode;
	min::push(input_buffer) = ic;
	ic.index += bytes_read;
	min::uns32 width =
	    LEX::wchar ( unicode, scanner->print_mode );
	if ( width == 0 && unicode == '\t' )
	    ic.column += 8 - ic.column % 8;
	else
	    ic.column += width;
    }

    ic.character = '\n';
    min::push(input_buffer) = ic;
    ++ ic.line;
    ic.index = 0;
    ic.column = 0;

    return true;
}

// Input Files
// ----- -----

file_ptr LEX::create_file ( void )
{
   return file_ptr ( file_type.new_gen() );
}

bool LEX::init_file
	( file_ptr file,
	  const char * file_name,
	  char error_message[512] )
{
    // Use OS independent min::os::file_size.
    //
    min::uns64 file_size;
    if ( ! min::os::file_size
               ( file_size, file_name, error_message ) )
        return false;

    if ( file_size >= ( 1ull << 32 ) - 1 )
    {
        sprintf ( error_message,
	          "ERROR: file %s too large (%llu)",
		  file_name, file_size );
	return false;
    }

    file->file_name = min::new_str_gen ( file_name );
    if ( file->data == NULL_STUB )
         file->data =
	     char_vec_type.new_gen
	         ( (min::uns32) file_size + 1,
		   (min::uns32) file_size );
    else
    {
        min::resize
	    ( file->data, (min::uns32) file_size + 1 );
	min::pop ( file->data, file->data->length );
	min::push ( file->data, file_size );
    }

    file->istream = NULL;
    file->spool_length = 0;
    file->line_number = 0;
    file->offset = 0;

    min::resize
	( file, uns32_vec_type.initial_max_length );
    min::pop ( file, file->length );

    // We use FILE IO because it is standard for C
    // while open/read is OS dependent.

    FILE * in = fopen ( file_name, "r" );

    if ( in == NULL )
    {
        sprintf ( error_message,
	          "ERROR: opening file %s\n"
	          "       %s",
		  file_name,
		  strerror ( errno ) );
	return false;
    }

    errno = 0;
    min::uns64 bytes =
        fread ( & file->data[0], 1,
	        (size_t) file_size, in );
    if ( bytes != file_size )
    {
	if ( errno != 0 )
	    sprintf ( error_message,
		      "ERROR: reading file %s\n"
		      "       %s",
		      file_name,
		      strerror ( errno ) );
	else
	    sprintf ( error_message,
		      "ERROR: reading file %s\n"
		      "       only %llu bytes"
		            " out of %llu read",
		      file_name, bytes, file_size );
	fclose ( in );
	return false;
    }

    fclose ( in );
    min::push(file->data) = 0;
    return true;
}

void LEX::init_stream ( file_ptr file,
			std::istream & istream,
			const char * file_name,
			uns32 spool_length )
{
    file->file_name = min::new_str_gen ( file_name );
    if ( file->data == NULL_STUB )
         file->data =
	     char_vec_type.new_gen ();
    else
    {
        min::resize
	    ( file->data,
	      char_vec_type.initial_max_length );
	min::pop ( file->data, file->data->length );
    }

    file->istream = & istream;
    file->spool_length = spool_length;
    file->line_number = 0;
    file->offset = 0;

    min::resize
	( file, uns32_vec_type.initial_max_length );
    min::pop ( file, file->length );
}

void LEX::init_string ( file_ptr file,
			const char * file_name,
			const char * data )
{
    file->file_name = min::new_str_gen ( file_name );
    uns64 length = strlen ( data );

    assert ( length <( 1ull << 32 ) - 1 );

    if ( file->data == NULL_STUB )
         file->data =
	     char_vec_type.new_gen
	         ( (min::uns32) length + 1 );
    else
    {
        min::resize
	    ( file->data, (min::uns32) length + 1 );
	min::pop ( file->data, file->data->length );
    }
    min::push ( file->data,
                (min::uns32) length + 1, data );

    file->istream = NULL;
    file->spool_length = 0;
    file->line_number = 0;
    file->offset = 0;

    min::resize
	( file, uns32_vec_type.initial_max_length );
    min::pop ( file, file->length );
}

uns32 LEX::next_line ( file_ptr file )
{
    if ( file->offset == NO_LINE ) return NO_LINE;

    uns32 length;  // Length of next line.

    if ( file->istream != NULL )
    {
	uns32 file_offset =
	    file->length - file->spool_length;

	// If spooling and unused data is 1/2 of total
	// data perform downshift to eliminate unused
	// data and file elements.
	//
	if (    file->spool_length > 0
	     && file->spool_length <= file->length
	     &&     2 * file[file_offset]
	        >=  file->data->length )
	{
	    // Perform downshift of file vector and
	    // file->data.  First file_offset elements
	    // of file vector are unused and eliminated,
	    // as are first data_offset chars of data.
	    //
	    uns32 data_offset = file[file_offset];
	    memcpy ( & file->data[0],
	             & file->data[data_offset],
		     file->data->length - data_offset );
	    min::pop ( file->data, data_offset );
	    memcpy ( & file[0], & file[file_offset],
	               sizeof ( uns32 )
		     * file->spool_length );
	    min::pop ( file, file_offset );
	    for ( uns32 i = 0; i < file->length; ++ i )
	        file[i] -= data_offset;
	    file->offset = file->data->length;
	}

	// Input line.
	//
	int c;
	while ( c = file->istream->get(),
	        c != EOF && c != '\n' )
	    min::push(file->data) = (char) c;

	length = file->data->length - file->offset;
	if ( length == 0 && c == EOF )
	    return file->offset = NO_LINE;
	min::push(file->data) = 0;
    }
    else
    {
        if ( file->offset == file->data->length )
	    return file->offset = NO_LINE;

        char * p = & file->data[file->offset];
	char * q = p;
	while ( * p && * p != '\n' ) ++ p;
	* p = 0;
	length = p - q;
    }

    uns32 offset = file->offset;
    min::push(file) = offset;
    file->offset += length + 1;
    assert ( file->offset <= file->data->length );
    ++ file->line_number;
    return offset;
}

uns32 LEX::line ( file_ptr file, uns32 line_number )
{
    if ( line_number >= file->line_number )
        return NO_LINE;
    else if ( file->spool_length == 0 )
        return file[line_number];
    else if ( file->spool_length >= file->length )
        return file[line_number];
    else if (   file->spool_length
              < file->line_number - line_number )
        return NO_LINE;
    else
        return file[  file->length
	            - (   file->line_number
		        - line_number)];
}

// Printing
// --------

min::uns32 LEX::default_print_mode = LEX::ASCIIGRAPHIC;

int LEX::spchar ( char * buffer, uns32 c,
                  uns32 print_mode )
{
    // Optimization
    //
    if ( 0x21 <= c && c <= 0x7E && c != '\\' )
    {
        * buffer ++ = (char) c;
	* buffer = 0;
	return 1;
    }

    switch ( print_mode )
    {
    case UTF8PRINT:
        if ( c <= 0x1f )
	{
	    if ( c == '\r' || c == '\n' || c == '\f'
	                   || c == '\v' || c == '\t' )
	    {
	        * buffer ++ = (char) c;
		* buffer = 0;
		return 1;
	    }
	}
    case UTF8GRAPHIC:
        if ( c == 0x7F ) c = 0x2421;
	else if ( c == '\n' ) c = 0x2424;
	else if ( c == ' ' ) c = 0x2423;
	else if ( c <= 0x1F ) c += 0x2400;
    case UTF8:
    {
        if ( c <= 0x7F )
	{
	    * buffer ++ = (char) c;
	    * buffer = 0;
	    return 1;
	}
	int shift;
	char * p = buffer;
	if ( c <= 0x7FF )
	    * p ++ = 0xC0 + ( c >> 6 ), shift = 0;
	else if ( c <= 0xFFFF )
	    * p ++ = 0xE0 + ( c >> 12 ), shift = 6;
	else if ( c <= 0x1FFFFF )
	    * p ++ = 0xF0 + ( c >> 18 ), shift = 12;
	else if ( c <= 0x3FFFFFF )
	    * p ++ = 0xF8 + ( c >> 24 ), shift = 18;
	else if ( c <= 0x7FFFFFFF )
	    * p ++ = 0xFC + ( c >> 30 ), shift = 24;
	else 
	    * p ++ = 0xFE, shift = 30;
	while ( true )
	{
	    * p ++ = 0x80 + ( ( c >> shift ) & 0x3F );
	    if ( shift == 0 ) break;
	    shift -= 6;
	}
	* p = 0;
	return p - buffer;
    }
    case ASCIIPRINT:
        if ( c <= 0x1F )
	{
	    if ( c == '\r' || c == '\n' || c == '\f'
	                   || c == '\v' || c == '\t' )
	    {
	        * buffer ++ = (char) c;
		* buffer = 0;
		return 1;
	    }
	}
    case ASCIIGRAPHIC:
	if ( c == '\\' )
	    return sprintf ( buffer, "\\/" );
	else if ( 0x21 <= c && c <= 0x7E )
	    return sprintf ( buffer, "%c", (char) c );
	else if ( c == ' ' )
	    return sprintf ( buffer, "\\~/" );
	else if ( c == '\n' )
	    return sprintf ( buffer, "\\lf/" );
	else if ( c == '\t' )
	    return sprintf ( buffer, "\\ht/" );
	else if ( c == '\f' )
	    return sprintf ( buffer, "\\ff/" );
	else if ( c == '\v' )
	    return sprintf ( buffer, "\\vt/" );
	else if ( c == '\b' )
	    return sprintf ( buffer, "\\bs/" );
	else if ( c == '\r' )
	    return sprintf ( buffer, "\\cr/" );
	else if ( c <= 0xF )
	    return sprintf ( buffer, "\\%02X/", c );
	else if ( c <= 0xFF )
	    return sprintf ( buffer, "\\%03X/", c );
	else if ( c <= 0xFFF )
	    return sprintf ( buffer, "\\%04X/", c );
	else if ( c <= 0xFFFF )
	    return sprintf ( buffer, "\\%05X/", c );
	else if ( c <= 0xFFFFF )
	    return sprintf ( buffer, "\\%06X/", c );
	else if ( c <= 0xFFFFFF )
	    return sprintf ( buffer, "\\%07X/", c );
	else if ( c <= 0xFFFFFFF )
	    return sprintf ( buffer, "\\%08X/", c );
	else
	    return sprintf ( buffer, "\\%09X/", c );
    default:
        abort();
    }
}

int LEX::wchar ( uns32 c, uns32 print_mode )
{
    // Optimization
    //
    if ( 0x21 <= c && c <= 0x7E && c != '\\' )
        return 1;

    switch ( print_mode )
    {
    case UTF8PRINT:
        if ( c <= 0x1f )
	{
	    if ( c == '\r' || c == '\n' || c == '\f'
	                   || c == '\v' || c == '\t' )
	        return 0;
	}
    case UTF8GRAPHIC:
        if ( c == 0x7F ) return 1;
	else if ( c <= 0x1F ) return 1;
    case UTF8:
    {
        if ( c <= 0x1F ) return 0;
	else return 1;
    }
    case ASCIIPRINT:
        if ( c <= 0x1f )
	{
	    if ( c == '\r' || c == '\n' || c == '\f'
	                   || c == '\v' || c == '\t' )
	        return 0;
	}
    case ASCIIGRAPHIC:
	if ( c == '\\' )
	    return 2;
	else if ( 33 <= c && c <= 126 )
	    return 1;
	else if ( c == ' ' )
	    return 3;
	else if ( c == '\n' )
	    return 4;
	else if ( c == '\t' )
	    return 4;
	else if ( c == '\f' )
	    return 4;
	else if ( c == '\v' )
	    return 4;
	else if ( c == '\b' )
	    return 4;
	else if ( c == '\r' )
	    return 4;
	else if ( c <= 0xF )
	    return 4;
	else if ( c <= 0xFF )
	    return 5;
	else if ( c <= 0xFFF )
	    return 6;
	else if ( c <= 0xFFFF )
	    return 7;
	else if ( c <= 0xFFFFF )
	    return 8;
	else if ( c <= 0xFFFFFF )
	    return 9;
	else if ( c <= 0xFFFFFFF )
	    return 10;
	else
	    return 11;
    default:
        abort();
    }
}

ostream & operator <<
	( ostream & out, const LEX::pchar & pc )
{
    char buffer[20];
    spchar ( buffer, pc.c, pc.print_mode );
    return out << buffer;
}

int LEX::spchar
	( char * buffer, uns32 c,
	  unsigned & column,
	  uns32 space_mode,
	  scanner_ptr scanner,
	  uns32 print_mode )
{
    // Optimization
    //
    if ( 0x21 <= c && c <= 0x7E && c != '\\' )
    {
	switch ( space_mode )
	{
	case ENFORCE_LINE_LENGTH:
	    if ( column >= scanner->line_length )
	        break;
	case 0:
	    * buffer ++ = (char) c;
	    ++ column;
	    * buffer = 0;
	    return 1;
	case PREFACE_WITH_SPACE + ENFORCE_LINE_LENGTH:
	    if ( column + 1 >= scanner->line_length )
	        break;
	case PREFACE_WITH_SPACE:
	    * buffer ++ = ' ';
	    ++ column;
	    * buffer ++ = (char) c;
	    ++ column;
	    * buffer = 0;
	    return 2;
	}
    }

    if ( print_mode == LEX::DEFAULT_PRINT_MODE )
        print_mode = scanner->print_mode;

    int width = wchar ( c, print_mode ); 
    bool is_tab = ( width == 0 && c == '\t' );
    if ( is_tab ) width = column += 8 - column % 8;

    char * p = buffer;
    switch ( space_mode )
    {
    case ENFORCE_LINE_LENGTH:
	if ( column + width >= scanner->line_length )
	{
	    p += sprintf ( p, "\n%*s",
		           scanner->indent, "" );
	    column = scanner->indent;
	}
	break;
    case PREFACE_WITH_SPACE + ENFORCE_LINE_LENGTH:
	if (    column + width + 1
	     >= scanner->line_length )
	{
	    p += sprintf ( p, "\n%*s",
		           scanner->indent, "" );
	    column = scanner->indent;
	    break;
	}
    case PREFACE_WITH_SPACE:
	* p ++ = ' ';
	++ column;
	break;
    }
    p += spchar ( p, c, print_mode );
    if ( is_tab ) width = column += 8 - column % 8;
    column += width;
    return p - buffer;
}

int LEX::spword
	( char * buffer, const char * word,
	  unsigned & column,
	  uns32 space_mode,
	  scanner_ptr scanner )
{
    int width = strlen ( word );
    char * p = buffer;
    switch ( space_mode )
    {
    case ENFORCE_LINE_LENGTH:
	if ( column + width >= scanner->line_length )
	{
	    p += sprintf ( p, "\n%*s",
		           scanner->indent, "" );
	    column = scanner->indent;
	}
	break;
    case PREFACE_WITH_SPACE + ENFORCE_LINE_LENGTH:
	if (    column + width + 1
	     >= scanner->line_length )
	{
	    p += sprintf ( p, "\n%*s",
		           scanner->indent, "" );
	    column = scanner->indent;
	    break;
	}
    case PREFACE_WITH_SPACE:
	* p ++ = ' ';
	++ column;
	break;
    }
    strcpy ( p, word );
    p += width;
    column += width;
    return p - buffer;
}

int LEX::spstring
	( char * buffer,
	  const char * string,
	  unsigned & column,
	  uns32 space_mode,
	  scanner_ptr scanner,
	  uns32 print_mode )
{
    if ( print_mode == LEX::DEFAULT_PRINT_MODE )
        print_mode = scanner->print_mode;

    uns32 c;
    char * p = buffer;
    while ( * string )
    {
        string += read_utf8 ( c, string );
	p += spchar ( p, c, column,
	              space_mode, scanner, print_mode );
	space_mode &= ~ PREFACE_WITH_SPACE;
    }
    return p - buffer;
}

int LEX::spinput
	( char * buffer, uns32 first, uns32 last,
	  unsigned & column,
	  uns32 space_mode,
	  scanner_ptr scanner,
	  uns32 print_mode )
{
    if ( print_mode == LEX::DEFAULT_PRINT_MODE )
        print_mode = scanner->print_mode;

    if ( first > last )
        return spword ( buffer, "<empty>", column,
			space_mode, scanner );

    char * p = buffer;
    while ( first <= last )
    {
	uns32 c = scanner->input_buffer[first++]
	                  .character;
	p += spchar ( p, c, column,
	              space_mode, scanner, print_mode );
	space_mode &= ~ PREFACE_WITH_SPACE;
    }
    return p - buffer;
}

int LEX::sptranslation
	( char * buffer,
	  unsigned & column,
	  uns32 space_mode,
	  scanner_ptr scanner,
	  uns32 print_mode )
{
    if ( print_mode == LEX::DEFAULT_PRINT_MODE )
        print_mode = scanner->print_mode;

    min::packed_vec_insptr<uns32> translation_buffer =
        scanner->translation_buffer;

    if ( translation_buffer->length == 0 )
        return spword ( buffer, "<empty>", column,
			space_mode, scanner );

    char * p = buffer;
    for ( unsigned i = 0;
          i < translation_buffer->length; ++ i )
    {
	uns32 c = translation_buffer[i];
	p += spchar ( p, c, column,
	              space_mode, scanner, print_mode );
	space_mode &= ~ PREFACE_WITH_SPACE;
    }
    return p - buffer;
}

int LEX::splexeme
	( char * buffer,
	  uns32 first, uns32 last, uns32 type,
	  unsigned & column,
	  uns32 space_mode,
	  scanner_ptr scanner,
	  uns32 print_mode )
{
    if ( print_mode == LEX::DEFAULT_PRINT_MODE )
        print_mode = scanner->print_mode;

    min::packed_vec_insptr<inchar> input_buffer =
        scanner->input_buffer;
    
    char buffer2[500];
    char * p2 = buffer2;
    p2 += spmode ( p2, type, scanner );
    * p2 ++ = ':';

    if ( first <= last )
    {
	inchar & ic = input_buffer[first];
	sprintf ( p2, "%u(%u)%u:",
	          ic.line, ic.index, ic.column );
    }
    else * p2 = 0;

    char * p = buffer;
    p += spword ( p, buffer2, column,
                  space_mode, scanner );
    p += spinput ( p, first, last,
		   column,
		   space_mode | PREFACE_WITH_SPACE,
		   scanner, print_mode );

    if ( ! translation_is_exact
               ( first, last, scanner ) )
    {
        p += spword ( p, "translated to:",
	              column,
		      space_mode | PREFACE_WITH_SPACE,
		      scanner );
	p += sptranslation
	         ( p, column,
		   space_mode | PREFACE_WITH_SPACE,
		   scanner, print_mode );
    }
    return p - buffer;
}

int LEX::sperroneous_atom
	( char * buffer,
	  uns32 first, uns32 last, uns32 type,
	  unsigned & column,
	  uns32 space_mode,
	  scanner_ptr scanner,
	  uns32 print_mode )
{
    if ( print_mode == LEX::DEFAULT_PRINT_MODE )
        print_mode = scanner->print_mode;

    min::packed_vec_insptr<inchar> input_buffer =
        scanner->input_buffer;

    char buffer2[500];
    char * p2 = buffer2;
    p2 += spmode ( p2, type, scanner );
    * p2 ++ = ':';

    if ( first <= last )
    {
	inchar & ic = input_buffer[first];
	sprintf ( p2, "%u(%u)%u:",
	          ic.line, ic.index, ic.column );
    }
    else * p2 = 0;

    char * p = buffer;
    p += spword ( p, buffer2, column,
                  space_mode, scanner );
    p += spinput ( p, first, last,
                   column,
		   space_mode | PREFACE_WITH_SPACE,
		   scanner, print_mode );
    return p - buffer;
}

bool LEX::translation_is_exact
	( uns32 first, uns32 last,
	  scanner_ptr scanner )
{
    min::packed_vec_insptr<inchar> input_buffer =
        scanner->input_buffer;
    min::packed_vec_insptr<uns32> translation_buffer =
        scanner->translation_buffer;

    uns32 i = 0;
    if (    translation_buffer->length
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

int LEX::spmode ( char * buffer, uns32 mode,
                  scanner_ptr scanner )
{
    program_ptr program = scanner->program;
    program_header & ph =
        * (program_header *) & program[0];
    if ( mode <= ph.max_type )
    {
        uns32 offset =
	    program[program_header_length + mode];
	if ( offset != 0 )
	    return sprintf
		( buffer, "%s",
		    (const char *) & program[0]
		  + offset );
    }

    switch ( mode )
    {
    case MASTER:
	return sprintf ( buffer, "MASTER" );
    case ATOM:
	return sprintf ( buffer, "ATOM" );
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
    scanner_ptr scanner = pm.scanner;

    spmode ( scanner->work, pm.mode, scanner );
    return out << scanner->work;
}

static const unsigned IDwidth = 12;
    // Width of field containing ID at the beginning
    // of each print_program line.

// out << pID ( ID ) prints `ID: ' right adjusted in
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
    ( scanner_ptr scanner,
      std::ostream & out, uns32 ID,
      unsigned indent = IDwidth )
{
    LEX::program_ptr program = scanner->program;

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
    if ( ( ( h.operation & GOTO ) != 0 )
	 +
         ( ( h.operation & RETURN ) != 0 )
	 > 1 ) out << "ILLEGAL: ";
    else
    if ( ( ( h.operation & CALL ) != 0 )
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
    if ( ( h.operation & ERRONEOUS_ATOM )
         &&
         h.erroneous_atom_type == 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & ERRONEOUS_ATOM ) == 0
         &&
         h.erroneous_atom_type != 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & OUTPUT )
         &&
         h.output_type == 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & OUTPUT ) == 0
         &&
         h.output_type != 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & GOTO )
         &&
         h.goto_table_ID == 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & GOTO ) == 0
         &&
         h.goto_table_ID != 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & CALL )
         &&
         h.call_table_ID == 0 )
	out << "ILLEGAL: ";
    else
    if ( ( h.operation & CALL ) == 0
         &&
         h.call_table_ID != 0 )
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
	    << h.erroneous_atom_type << ")";

    if ( h.operation & OUTPUT )
        OUT << "OUTPUT("
	    << h.output_type << ")";

    if ( h.operation & GOTO )
        OUT << "GOTO(" << h.goto_table_ID << ")";

    if ( h.operation & CALL )
        OUT << "CALL(" << h.call_table_ID << ")";

    if ( h.operation & RETURN )
        OUT << "RETURN";

    if ( h.operation & FAIL )
        OUT << "FAIL";

    if ( h.operation & ELSE )
    {
        OUT << "ELSE:" << endl;
	print_instruction
	    ( scanner, out, h.else_instruction_ID,
	               indent );
    }

    if ( first ) out << "ACCEPT" << endl;
    else out << endl;
#   undef OUT

    return instruction_length;
}

// Iterator that prints out a list of characters within
// scanner->line_length columns.  `nonempty' if the list
// is non-empty.  The list is indented by the given
// amount.  The user is responsible for the indent of
// the first line if user_indent is true.
//
struct pclist {
    std::ostream & out;
        // Output stream.
    LEX::scanner_ptr scanner;
        // Scanner that provides line_length.
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

    pclist ( std::ostream & out,
             LEX::scanner_ptr scanner,
             int indent, bool user_indent = false )
	: out ( out ), scanner ( scanner ),
	  indent ( indent ), user_indent ( user_indent )
    {
	assert ( scanner->line_length - indent >= 30 );
        empty = true;
	columns = scanner->line_length - indent;
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

	if ( columns == scanner->line_length - indent )
	{
	    // If nothing on line, its our first line.
	    //
	    if ( ! user_indent )
	        out << setw ( indent ) << "";
	    columns -= needed;
	}
	else if ( columns >= 1 + needed )
	{
	    out << " ";
	    columns -= 1 + needed;
	}
	else
	{
	    out << endl << setw ( indent ) << "";
	    columns = scanner->line_length
	            - indent - needed;
	}

	out << buffer;
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
    ( scanner_ptr scanner, std::ostream & out,
      uns32 ID, unsigned indent = IDwidth )
{
    LEX::program_ptr program = scanner->program;

    if ( ID == 0 ) return 0;

    out << pID ( ID ) << "DISPATCHER" << endl;

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

    out << INDENT << "Break Elements: "
	<< h.break_elements << endl;
    out << INDENT << "Max Break Elements: "
	<< h.max_break_elements << endl;
    out << INDENT << "Max CType: "
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

	pclist pcl ( out, scanner, IDwidth );
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
	out << endl;

	print_instruction
	    ( scanner, out, mep[t].instruction_ID,
	               IDwidth + 4 );
	if ( mep[t].dispatcher_ID != 0 )
	    out << INDENT << "    Dispatcher ID: "
	        << mep[t].dispatcher_ID << endl;
    }

    return length;
}

uns32 LEX::print_program_component
	( std::ostream & out, uns32 ID, bool cooked,
	  scanner_ptr scanner )
{
    LEX::program_ptr program = scanner->program;

    char buffer[100];
    switch ( program[ID] )
    {
    case PROGRAM:
    {
	out << pID ( ID ) << "PROGRAM" << endl;
	program_header & h =
	    * (program_header *) & program[ID];
	out << INDENT << "Initial Table ID: "
	    << h.initial_table_ID << endl;
	out << INDENT << "Max Type: "
	    << h.max_type << endl;
	for ( uns32 t = 0; t <= h.max_type; ++ t )
	{
	    uns32 offset =
	        program[ID+program_header_length+t];
	    if ( offset == 0 ) continue;
	    out << INDENT << setw ( 6 ) << t << ": "
	        << (const char *) & program[ID] + offset
		<< endl;
	}
	return h.component_length;
    }
    case TABLE:
    {
	out << pID ( ID ) << "TABLE" << endl;
	table_header & h =
	    * (table_header *) & program[ID];
	out << INDENT << "Mode: "
	    << pmode ( h.mode, scanner ) << endl;
	out << INDENT << "Dispatcher ID: "
	    << h.dispatcher_ID << endl;
	if ( cooked )
	    print_instruction
		( scanner, out,
		  h.instruction_ID, IDwidth );
	else
	    out << INDENT << "Instruction ID: "
		<< h.instruction_ID << endl;
	return table_header_length;
    }
    case DISPATCHER:
    if ( cooked )
	return print_cooked_dispatcher
	           ( scanner, out, ID );
    else
    {
	out << pID ( ID ) << "DISPATCHER" << endl;
	dispatcher_header & h =
	    * (dispatcher_header *) & program[ID];
	out << INDENT << "Break Elements: "
	    << h.break_elements << endl;
	out << INDENT << "Max Break Elements: "
	    << h.max_break_elements << endl;
	out << INDENT << "Max CType: "
	    << h.max_ctype << endl;
	out << INDENT << "Breaks: "
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
	    out << INDENT
		<< setw ( 24 ) << pchar ( be.cmin )
		<< setw ( 16 ) << be.type_map_ID
		<< endl;
	}
	length += break_element_length
	        * h.max_break_elements;
	out << INDENT << "Map:   CType: "
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
	    out << INDENT
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
	out << pID ( ID );
	return print_instruction
	    ( scanner, out, ID, 0 );
    }
    case TYPE_MAP:
    {
	out << pID ( ID ) << "TYPE_MAP" << endl;
	type_map_header & h =
	    * (type_map_header *) & program[ID];
	uns32 length = type_map_header_length;
	if ( h.singleton_ctype > 0 )
	{
	    out << setw ( IDwidth + 4 )
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
		pclist pcl ( out, scanner,
		             IDwidth + 6, true );
		for ( uns32 c = h.cmin;
		      c <= h.cmax; ++ c )
		{
		    if ( map[c - h.cmin] == t )
		    {
		        if ( pcl.empty )
			    out << setw ( IDwidth
					  + 4 )
				<< t << ": ";
			pcl.add ( c, c );
		    }
		}
		pcl.flush();
		if ( ! pcl.empty ) out << endl;
	    }
	}
	return length;
    }
    default:
    {
	out << pID ( ID ) << "ILLEGAL COMPONENT TYPE("
	    << program[ID] << ")" << endl;
	return program->length - ID;
    }
    }
}

void LEX::print_program
	( std::ostream & out, bool cooked,
	  scanner_ptr scanner )
{
    LEX::program_ptr program = scanner->program;

    uns32 ID = 0;
    while ( ID < program->length )
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

    if ( ID > program->length )
        out << "  ILLEGALLY TRUNCATED LAST PROGRAM"
	       " COMPONENT" << endl;
}
