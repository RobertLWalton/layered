// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Feb 27 02:04:58 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Data
//	Program Construction
//	Scanner Closures
//	Scanning
//	Printing
//	Printing Programs

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


// Data
// ----

static uns32 scanner_stub_disp[] =
    { min::DISP ( & LEX::scanner_struct::program ),
      min::DISP ( & LEX::scanner_struct::input_buffer ),
      min::DISP ( & LEX::scanner_struct
                       ::translation_buffer ),
      min::DISP ( & LEX::scanner_struct
                       ::read_input ),
      min::DISP ( & LEX::scanner_struct
                       ::input_file ),
      min::DISP ( & LEX::scanner_struct
                       ::erroneous_atom ),
      min::DISP ( & LEX::scanner_struct
                       ::printer ),
      min::DISP_END };

static min::packed_struct<LEX::scanner_struct>
   scanner_type ( "ll::lexeme::scanner_type",
	          NULL, ::scanner_stub_disp );

static min::packed_vec<char>
    char_vec_type ( "ll::lexeme::char_vec_type" );
static min::packed_vec<LEX::uns32>
    uns32_vec_type ( "ll::lexeme::uns32_vec_type" );
static min::packed_vec<LEX::inchar>
    inchar_vec_type ( "ll::lexeme::inchar_vec_type" );
static min::packed_struct<LEX::input_struct>
    input_type ( "ll::lexeme::input_type" );
static min::packed_struct<LEX::erroneous_atom_struct>
    erroneous_atom_type
	( "ll::lexeme::erroneous_atom_type" );

min::locatable_ptr<LEX::program>
     LEX::default_program;
min::locatable_ptr<LEX::input>
     LEX::default_input;
min::locatable_ptr<LEX::erroneous_atom>
     LEX::default_erroneous_atom;
min::locatable_ptr<LEX::scanner>
     LEX::default_scanner;

// Program Construction
// ------- ------------

# define ERR min::init ( min::error_message ) \
    << "LEXICAL PROGRAM CONSTRUCTION ERROR: "


uns32 LEX::create_table
	( uns32 mode, LEX::program program )
{
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
	( uns32 ID, LEX::program program )
{
    table_header & h = * (table_header *) & program[ID];
    return h.mode;
}

void LEX::create_program
	( const char * const * type_name,
	  uns32 max_type,
	  LEX::program & program )
{
    if ( program == NULL_STUB )
        program = uns32_vec_type.new_gen();
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
	  LEX::program program )
{
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
	  LEX::program program )
{
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
	  LEX::program program )
{
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
	  LEX::program program )
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
static bool attach_type_map_to_dispatcher
	( uns32 dispatcher_ID,
	  uns32 type_map_ID,
	  LEX::program program )
{
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
        ERR << "Attempt to attach type map "
	    << type_map_ID
	    << " to dispatcher "
	    << dispatcher_ID
	    << " conflicts with previous attachment of"
	       " type map "
	    << bep[i].type_map_ID
	    << min::eol;
        return false;
    }

    if ( i + 1 != dh.break_elements
         &&
	 bep[i+1].cmin < mh.cmin )
    {
        assert ( bep[i+1].type_map_ID != 0 );
        ERR << "Attempt to attach type map "
	    << type_map_ID
	    << " to dispatcher "
	    << dispatcher_ID
	    << " conflicts with previous attachment of"
	       " type map "
	    << bep[i+1].type_map_ID
	    << min::eol;
        return false;
    }

    int n = 2; // Number of new break elements needed.
    if ( bep[i].cmin == mh.cmin ) -- n;
    if ( ! split_next ) -- n;
    if ( dh.break_elements + n > dh.max_break_elements )
    {
        ERR << "Attempt to attach type map "
	    << type_map_ID
	    << " to dispatcher "
	    << dispatcher_ID
            << " fails because dispatcher already has"
	       " too many breaks"
	    << min::eol;
	return false;
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
    return true;
}

bool LEX::attach
	( uns32 target_ID,
	  uns32 component_ID,
	  LEX::program program )
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
		ERR << "Attempt to attach dispatcher "
		    << component_ID
		    << " to table "
		    << target_ID
		    << " conflicts with previous"
		       " attachment of dispatcher "
		    << h.dispatcher_ID
		    << min::eol;
	        return false;
	    }
	    h.dispatcher_ID = component_ID;
	    return true;
	}
        else if ( component_pctype == INSTRUCTION )
	{
	    if ( h.instruction_ID != 0 )
	    {
		ERR << "Attempt to attach instruction "
		    << component_ID
		    << " to table "
		    << target_ID
		    << " conflicts with previous"
		       " attachment of instruction "
		    << h.instruction_ID
		    << min::eol;
	        return false;
	    }
	    h.instruction_ID = component_ID;
	    return true;
	}
	else assert
	    ( ! "bad attach component pctypes" );
    }
    else if ( target_pctype == DISPATCHER
              &&
	      component_pctype == TYPE_MAP )
        return attach_type_map_to_dispatcher
	    ( target_ID, component_ID, program );
    else
	assert ( ! "bad attach component pctypes" );
}

bool LEX::attach
	( uns32 target_ID,
	  uns32 ctype,
	  uns32 component_ID,
	  LEX::program program )
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
	    ERR << "Attempt to attach dispatcher "
		<< component_ID
		<< " to dispatcher "
		<< target_ID
		<< " ctype "
		<< ctype
		<< " conflicts with previous attachment"
		   " of dispatcher "
		<< me.dispatcher_ID
		<< min::eol;
	    return false;
	}
	me.dispatcher_ID = component_ID;
	return true;
    }
    else if ( component_pctype == INSTRUCTION )
    {
	if ( me.instruction_ID != 0 )
	{
	    ERR << "Attempt to attach instruction "
		<< component_ID
		<< " to dispatcher "
		<< target_ID
		<< " ctype "
		<< ctype
		<< " conflicts with previous attachment"
		   " of instruction "
		<< me.instruction_ID
		<< min::eol;
	    return false;
	}
	me.instruction_ID = component_ID;
	return true;
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
	( LEX::program program )
{
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

# undef ERR

// Scanner Closures
// ------- --------

void LEX::init
	( LEX::input & input,
	  bool (*get) ( LEX::scanner scanner,
			LEX::input input ) )
{
    if ( input == NULL_STUB )
        input = ::input_type.new_stub();
    input->get = get;
}

void LEX::init
	( LEX::erroneous_atom & erroneous_atom,
	  void (* announce )
	    ( uns32 first, uns32 next, uns32 type,
	      LEX::scanner scanner,
	      LEX::erroneous_atom erroneous_atom ) )
{
    if ( erroneous_atom == NULL_STUB )
        erroneous_atom =
	    ::erroneous_atom_type.new_stub();
    erroneous_atom->announce = announce;
}

static bool default_input_get
	( LEX::scanner scanner,
	  LEX::input input )
{
    min::file file = scanner->input_file;

    min::uns32 offset = min::next_line ( file );
    min::uns32 length = 0xFFFFFFFF;

    LEX::inchar ic;
    ic.line = file->next_line_number;
    ic.index = 0;
    ic.column = 0;

    if ( offset != min::NO_LINE )
        -- ic.line;
    else
    {
        length = min::partial_length ( file );
        if ( length == 0 ) return false;
	offset = min::partial_offset ( file );
	min::skip_partial ( file );
    }

    LEX::input_buffer input_buffer =
        scanner->input_buffer;

    for ( ; length != 0 && file->buffer[offset] != 0;
            -- length )
    {
	const char * beginp = & file->buffer[offset];
	const char * p = beginp;
	uns32 unicode = min::utf8_to_unicode ( p );
	uns32 bytes_read = p - beginp;
	offset += bytes_read;

	ic.character = unicode;
	min::push(input_buffer) = ic;
	ic.index += bytes_read;
	min::pwidth ( ic.column, unicode,
		      file->print_flags );
    }

    if ( length != 0 )
    {
	ic.character = '\n';
	min::push(input_buffer) = ic;
	++ ic.line;
	ic.index = 0;
	ic.column = 0;
    }

    scanner->next_position = (LEX::position) ic;

    return true;
}

static void default_erroneous_atom_announce
	( uns32 first, uns32 next, uns32 type,
	  LEX::scanner scanner,
	  LEX::erroneous_atom erroneous_atom )
{
    if ( scanner->printer == min::NULL_STUB )
        min::init ( scanner->printer );

    scanner->printer
        << "ERRONEOUS ATOM: "
	<< LEX::perroneous_atom
	       ( scanner, first, next, type )
	<< min::eol;
}

// Scanning
// --------

// Return true if table_ID is in return_stack.
//
static bool is_recursive
	( LEX::scanner scanner, uns32 table_ID )
{
    for ( uns32 i = 0;
          i < scanner->return_stack_p; ++ i )
    {
        if ( scanner->return_stack[i] == table_ID )
	    return true;
    }
    return false;
}

// Create scanner.  Scanner variable must be locatable
// by garbage collector.
//
static void create_scanner
	( LEX::scanner & scanner )
{
    LEX::init
	( LEX::default_input,
	  ::default_input_get );
    init
	( LEX::default_erroneous_atom,
	  ::default_erroneous_atom_announce );

    scanner = scanner_type.new_stub();

    scanner->input_buffer =
	inchar_vec_type.new_gen();
    scanner->translation_buffer =
	uns32_vec_type.new_gen();

    scanner->reinitialize = true;
}

// We assume the program is well formed, in that an
// XXX_ID actually points at a program component of
// pctype XXX.  We check this with asserts (the attach
// statements check this).  Everything else found
// wrong with the program is a SCAN_ERROR.

void LEX::init ( LEX::scanner & scanner )
{

    if ( scanner == NULL_STUB )
        create_scanner ( scanner );
    else
    {
	min::pop ( scanner->input_buffer,
		   scanner->input_buffer->length );
	min::resize ( scanner->input_buffer, 1000 );
	min::pop
	    ( scanner->translation_buffer,
	      scanner->translation_buffer->length );
	min::resize
	    ( scanner->translation_buffer, 1000 );

	scanner->next_position.line = 0;
	scanner->next_position.index = 0;
	scanner->next_position.column = 0;

	scanner->next = 0;

	scanner->return_stack_p = 0;
	memset ( scanner->return_stack, 0,
		 sizeof ( scanner->return_stack ) );
    }

    scanner->reinitialize = true;
}

void LEX::init_program
	( LEX::scanner & scanner,
	  LEX::program program )
{
    init ( scanner );
    scanner->program = program;
}

void LEX::init_input_file
	( LEX::scanner & scanner,
	  min::file input_file )
{
    init ( scanner );
    scanner->input_file = input_file;
}

void LEX::init_printer
	( LEX::scanner & scanner,
	  min::printer printer )
{
    init ( scanner );
    scanner->printer = printer;
}

bool LEX::init_input_named_file
	( LEX::scanner & scanner,
	  min::gen file_name,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
	  
{
    init ( scanner );
    return min::init_input_named_file
	( scanner->input_file,
	  file_name,
	  print_flags,
	  spool_lines );
}

void LEX::init_input_stream
	( LEX::scanner & scanner,
	  std::istream & istream,
	  min::uns32 print_flags,
	  uns32 spool_lines )
{
    init ( scanner );
    min::init_input_stream
	( scanner->input_file, istream,
	  print_flags, spool_lines );
}

void LEX::init_input_string
	( LEX::scanner & scanner,
	  const char * data,
	  min::uns32 print_flags,
	  uns32 spool_lines )
{
    init ( scanner );
    min::init_input_string
	( scanner->input_file, data,
	  print_flags, spool_lines );
}

void LEX::init_input
	( LEX::scanner & scanner )
{
    init ( scanner );
    min::init_input ( scanner->input_file );
}

void LEX::init_print_flags
	( LEX::scanner & scanner,
	  min::uns32 print_flags )
{
    init ( scanner );
    min::init_print_flags
	( scanner->input_file, print_flags );
}

void LEX::init_spool_lines
	( LEX::scanner & scanner,
	  min::uns32 spool_lines )
{
    init ( scanner );
    min::init_spool_lines
	( scanner->input_file, spool_lines );
}

min::printer LEX::init_output_stream
	( LEX::scanner & scanner,
	  std::ostream & out )
{
    init ( scanner );
    return min::init_output_stream
	( scanner->printer, out );
}

// Init min::error_message and write the beginning of a
// scan error message into it.  Return min::error_
// message.
//
// Usage is:
//
//    scan_error ( scanner, length, next ), ... )
//	      << ... rest of error message ...
//	      << min::eol;
//
// `length' is the number of characters scanned after
// `next'.  `next' defaults to scanner->next.
//
static min::printer scan_error
        ( LEX::scanner scanner,
	  uns32 length, uns32 next );
inline min::printer scan_error
        ( LEX::scanner scanner,
	  uns32 length )
{
    return scan_error
        ( scanner, length, scanner->next );
}

// Print the instruction at scanner->program[ID] with
// the given indent and min::eol, if ID is non-zero, and
// return ID repositioned just after instuction.  How-
// ever, if scanner->program[ID] does not ==
// INSTRUCTION, print ILLEGAL instruction message and
// return scanner->program->length + 1.
//
// If ID == 0 do nothing but return 0.
//
static uns32 print_instruction
    ( min::printer printer, LEX::program program,
      uns32 ID, unsigned indent );

// Given a dispatcher_ID and a character c return the
// ctype that the dispatcher maps c onto.
//
static uns32 ctype ( LEX::scanner scanner,
                     uns32 dispatcher_ID, uns32 c )
{
    LEX::program program = scanner->program;

    bool trace =
        ( scanner->trace & LEX::TRACE_DISPATCH );

    if ( trace )
	scanner->printer
	    << "  Character = " << pgraphic ( c )
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
	if ( trace )
	    scanner->printer
	        << " Type Map = " << type_map_ID;
	type_map_header & tmh =
	    * (type_map_header *)
	    & program[type_map_ID];
	ctype = tmh.singleton_ctype;
	if ( ctype == 0 )
	    ctype = ( (uns8 *) ( & tmh + 1 ) )
		    [c - tmh.cmin];
    }

    if ( trace )
	scanner->printer
	    << " CType = " << ctype << min::eol;

    return ctype;
}

// Scan atom given current table.  Locate and process
// instruction group, but not
//
//	ERRONEOUS_ATOM
//	OUTPUT
//	GOTO
//	CALL
//	RETURN
//	FAIL
//
// Skip failed instructions in instruction group.
// Return instruction_ID of first non-failed instruction
// so the above can be processed.  Return 0 if error,
// leaving error message in min::error_message.  If no
// error, return atom_length in argument variable and
// add translation of atom to translation buffer.
//
static uns32 scan_atom
    ( LEX::scanner scanner, uns32 & atom_length )
{
    LEX::program program =
        scanner->program;
    LEX::input_buffer input_buffer =
        scanner->input_buffer;
    LEX::translation_buffer translation_buffer =
        scanner->translation_buffer;

    const uns32 SCAN_ERROR = 0;
        // Local version of SCAN_ERROR.

    if ( scanner->trace & LEX::TRACE_TABLE )
        scanner->printer
	    << "Start atom scan: table = "
	    << scanner->current_table_ID << min::eol;

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
	     ! (*scanner->read_input->get)
	         ( scanner, scanner->read_input ) )
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
	    scan_error ( scanner, length )
		    << "Ctype " << ctype
		    << " computed for character "
		    << LEX::pgraphic ( c )
		    << " is too large for dispatcher "
		    << dispatcher_ID << min::eol;
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
	    scan_error ( scanner, length )
		<< "No instruction found" << min::eol;
	    return SCAN_ERROR;
	}

	if ( scanner->trace & LEX::TRACE_INSTRUCTION )
	    print_instruction
	        ( scanner->printer, program,
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
		scan_error ( scanner, atom_length )
		    << "MATCH in instruction "
		    << instruction_ID
		    << " executed by table "
		    << scanner->current_table_ID
		    << " targets non-atom table"
		    << min::eol;
		return SCAN_ERROR;
	    }
	    else if (    scanner->return_stack_p
		      == LEX::return_stack_size )
	    {
		scan_error ( scanner, atom_length )
		    << "MATCH in instruction "
		    << instruction_ID
		    << " executed by table "
		    << scanner->current_table_ID
		    << " but return stack is full"
		    << min::eol;
		return SCAN_ERROR;
	    }

	    scanner->return_stack
	        [scanner->return_stack_p++] =
	            scanner->current_table_ID;

	    if ( is_recursive
	             ( scanner, ih.atom_table_ID ) )
	    {
		scan_error ( scanner, atom_length )
		    << "Recursive MATCH to table "
		    << ih.atom_table_ID
		    << " in instruction "
		    << instruction_ID
		    << " executed by table "
		    << scanner->current_table_ID
		    << min::eol;
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
		scan_error ( scanner, length )
		    << "Keep length(" << keep
		    << ") greater than atom length("
		    << keep_length << ")" << min::eol;
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
		    scan_error ( scanner, length )
		        << "Ctype " << ctype
			<< " computed for character "
			<< pgraphic ( c )
			<< " is too large for"
			   " dispatcher "
		        << dispatcher_ID << min::eol;
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
		    scan_error ( scanner, length )
		        << "No instruction for ELSE in"
			   " failed instruction "
		        << instruction_ID
			<< " executed by table "
		        << scanner->current_table_ID
			<< min::eol;
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
		scan_error ( scanner, length )
		    << "No ELSE in failed instruction "
		    << instruction_ID
		    << " executed by table "
		    << scanner->current_table_ID
		    << min::eol;
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

uns32 LEX::scan ( uns32 & first, uns32 & next,
                  LEX::scanner scanner )
{
    if ( scanner->reinitialize )
    {
        if ( scanner->read_input == NULL_STUB )
	    scanner->read_input = default_input;

        if ( scanner->erroneous_atom == NULL_STUB )
	    scanner->erroneous_atom =
	        default_erroneous_atom;

	if ( scanner->program == NULL_STUB )
	{
	    min::init ( min::error_message )
	        << "LEXICAL SCANNER ERROR: no program"
		<< min::eol;
	    return SCAN_ERROR;
	}
	else if ( scanner->program[0] != PROGRAM )
	{
	    min::init ( min::error_message )
	        << "LEXICAL SCANNER ERROR:"
		   " illegal program"
		<< min::eol;
	    return SCAN_ERROR;
	}

	if ( scanner->read_input == NULL_STUB )
	{
	    init_input_stream
	        ( scanner->input_file, std::cin );
	    init_file_name
		( scanner->input_file,
		  min::new_str_gen
		      ( "standard input" ) );
	}

	program_header & h = * (program_header *)
			     & scanner->program[0];
	assert (    scanner->program[h.initial_table_ID]
		 == TABLE );
	scanner->current_table_ID = h.initial_table_ID;

        scanner->reinitialize = false;
    }

    if (    scanner->trace != 0
	 && scanner->printer == NULL_STUB )
	min::init ( scanner->printer );

    LEX::program program =
        scanner->program;
    LEX::input_buffer input_buffer =
        scanner->input_buffer;
    LEX::translation_buffer translation_buffer =
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
	        scan_error ( scanner,
		             scanner->next - first,
			     first )
		    << "Attempt to OUTPUT when the next"
		       " table "
		    << scanner->current_table_ID
		    << " is not a master table"
		    << min::eol;
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
		scan_error ( scanner, atom_length )
		    << "ERRONEOUS_ATOM in instruction "
		    << instruction_ID
		    << " executed by table "
		    << scanner->current_table_ID
		    << " but atom is of zero length"
		    << min::eol;
		return SCAN_ERROR;
	    }
	    else if (    scanner->erroneous_atom
	              == NULL_STUB )
	    {
		scan_error ( scanner, atom_length )
		    << "ERRONEOUS_ATOM in instruction "
		    << instruction_ID
		    << " executed by table "
		    << scanner->current_table_ID
		    << " but erroneous_atom closure"
		       " does not exist"
		    << min::eol;
		return SCAN_ERROR;
	    }
	    else
	    {
		(*scanner->erroneous_atom->announce)
		    ( scanner->next,
		      scanner->next + atom_length,
		      ih.erroneous_atom_type,
		      scanner,
		      scanner->erroneous_atom );
	    }
	}

	if ( op & OUTPUT )
	    output_type = ih.output_type;

	if ( op & RETURN )
	{
	    if (    scanner->return_stack_p
	         == LEX::return_stack_size )
	    {
		scan_error ( scanner, atom_length )
		    << "RETURN in instruction "
		    << instruction_ID
		    << " executed by table "
		    << scanner->current_table_ID
		    << " but return stack is empty"
		    << min::eol;
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
		scan_error ( scanner, atom_length )
		    << "GOTO in instruction "
		    << instruction_ID
		    << " executed by table "
		    << scanner->current_table_ID
		    << " targets atom table"
		    << min::eol;
		return SCAN_ERROR;
	    }
	}

	if ( op & CALL )
	{
	    if (    scanner->return_stack_p
		 == LEX::return_stack_size )
	    {
		scan_error ( scanner, atom_length )
		    << "CALL in instruction "
		    << instruction_ID
		    << " executed by table "
		    << scanner->current_table_ID
		    << " but return stack is full"
		    << min::eol;
		return SCAN_ERROR;
	    }

	    scanner->return_stack
	        [scanner->return_stack_p++] =
	        scanner->current_table_ID;

	    if ( is_recursive
	             ( scanner, ih.call_table_ID ) )
	    {
		scan_error ( scanner, atom_length )
		    << "Recursive CALL to table "
		    << ih.call_table_ID
		    << " in instruction "
		    << instruction_ID
		    << " executed by table "
		    << scanner->current_table_ID
		    << min::eol;
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
		scan_error ( scanner, atom_length )
		    << "CALL in instruction "
		    << instruction_ID
		    << " executed by table "
		    << scanner->current_table_ID
		    << " targets non-lexeme table"
		    << min::eol;
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
	    scan_error ( scanner, atom_length )
	        << "Endless loop in scanner"
		<< min::eol;
	    return SCAN_ERROR;
	}
    }

    first = first;
    next = scanner->next;

    uns32 type = output_type != 0 ?
                 output_type :
		 last_mode;

    switch ( type )
    {
    case MASTER:
    case ATOM:
    case SCAN_ERROR:
    {
	scan_error ( scanner, next - first, first )
	    << "Returning lexeme with bad type("
	    << pmode ( scanner->program, type ) << ")"
	    << min::eol;
	return SCAN_ERROR;
    }
    }

    return type;
}

// See documentation above.
//
static min::printer scan_error
        ( LEX::scanner scanner,
	  uns32 length, uns32 next )
{
    min::init ( min::error_message )
        << "LEXICAL SCANNER ERROR: current_table "
	             << scanner->current_table_ID;
    const LEX::position & pos =
	scanner->next < scanner->input_buffer->length ?
	    (LEX::position)
	        scanner->input_buffer[next] :
	    scanner->next_position;
    min::error_message << ": position "
	               << pos.line << "("
	               << pos.index << ")"
	               << pos.column;

    return min::error_message
        << ": "
	<< min::reserve ( length )
	<< pinput ( scanner, next, next + length )
	<< ": " << min::reserve ( 20 );
}

// Printing
// --------

min::printer operator <<
	( min::printer printer,
	  const LEX::pinput & pinput )
{
    min::uns32 first = pinput.first;
    min::uns32 next  = pinput.next;

    if ( first >= next )
        return printer << "<empty>";

    LEX::scanner scanner = pinput.scanner;

    printer << min::push_parameters << min::graphic;
    while ( first < next )
        printer << min::punicode
	    ( scanner->input_buffer[first++]
	              .character );
    return printer << min::pop_parameters;
}

min::printer operator <<
	( min::printer printer,
	  const LEX::ptranslation & ptranslation )
{
    LEX::translation_buffer translation_buffer =
        ptranslation.scanner->translation_buffer;

    if ( translation_buffer->length == 0 )
        return printer << "<empty>";

    printer << min::push_parameters << min::graphic;
    for ( unsigned i = 0;
          i < translation_buffer->length; ++ i )
        printer << min::punicode
	    ( translation_buffer[i] );
    return printer << min::pop_parameters;
}

min::printer operator <<
	( min::printer printer,
	  const LEX::plexeme & plexeme )
{
    LEX::scanner scanner = plexeme.scanner;
    printer << perroneous_atom
        ( scanner, plexeme.first,
		   plexeme.next,
		   plexeme.type );

    if ( ! translation_is_exact
               ( scanner, plexeme.first,
	                  plexeme.next ) )
        printer << min::reserve ( 30 )
	        << " translated to: "
	        << ptranslation ( scanner );
    return printer;
}

min::printer operator <<
	( min::printer printer,
	  const LEX::perroneous_atom & perroneous_atom )
{
    LEX::scanner scanner = perroneous_atom.scanner;
    uns32 type = perroneous_atom.type;
    uns32 first = perroneous_atom.first;
    uns32 next = perroneous_atom.next;

    LEX::input_buffer input_buffer =
        scanner->input_buffer;

    printer << LEX::pmode ( scanner->program, type )
            << " ";

    LEX::position pos =
        first < input_buffer->length ?
	    (LEX::position) input_buffer[first] :
	    scanner->next_position;

    printer << pos.line << "("
	    << pos.index << ")"
	    << pos.column << ": "
	    << min::reserve ( next + 1 - first )
	    << LEX::pinput ( scanner, first, next );
}

bool LEX::translation_is_exact
	( LEX::scanner scanner,
	  uns32 first, uns32 next )
{
    LEX::input_buffer input_buffer =
        scanner->input_buffer;
    LEX::translation_buffer translation_buffer =
        scanner->translation_buffer;

    uns32 i = 0;
    if (    translation_buffer->length
         != next - first )
        return false;
    while ( first < next )
    {
        if (    input_buffer[first].character
	     != translation_buffer[i] )
	    return false;
	++ first, ++ i;
    }
    return true;
}

min::printer operator <<
	( min::printer printer,
	  const LEX::pmode & pmode )
{
    LEX::program program = pmode.program;
    uns32 mode = pmode.mode;
    program_header & ph =
        * (program_header *) & program[0];
    if ( pmode.mode <= ph.max_type )
    {
        uns32 offset =
	    program[program_header_length + mode];
	if ( offset != 0 )
	    return printer
	        << min::push_parameters
	        << min::noautobreak
	        <<   (const char *) & program[0]
		   + offset
	        << min::pop_parameters;
    }

    switch ( mode )
    {
    case MASTER:
	return printer << "MASTER";
    case ATOM:
	return printer << "ATOM";
    case SCAN_ERROR:
	return printer << "SCAN_ERROR";
    default:
	return printer << min::push_parameters
	               << min::noautobreak
	               << "TYPE (" << mode << ")"
	               << min::pop_parameters;
    }
}

min::printer operator <<
	( min::printer printer,
	  const LEX::pline_numbers & pline_numbers )
{
    LEX::scanner scanner = pline_numbers.scanner;
    LEX::input_buffer input_buffer =
        scanner->input_buffer;
    min::uns32 first = pline_numbers.first;
    min::uns32 next = pline_numbers.next;

    LEX::position begin =
        first < input_buffer->length ?
	input_buffer[first] :
	scanner->next_position;
    LEX::position end =
        next < input_buffer->length ?
	input_buffer[next] :
	scanner->next_position;
    uns32 begin_line = begin.line;
    uns32 end_line =
        end.column != 0 ?	  end.line :
	end.line <= begin_line ?  end.line :
	                          end.line - 1;
    return printer << min::pline_numbers
    			   ( scanner->input_file,
			     begin_line, end_line );
}

void LEX::print_item_lines
	( min::printer printer,
	  min::file file,
	  const LEX::position & begin,
	  const LEX::position & end,
	  char mark,
	  const char * blank_line )
{
    assert ( end.line >= begin.line );

    uns32 line = begin.line;
    uns32 first_column = begin.column;

    uns32 width = min::print_line
        ( printer, file, line );

    while ( true )
    {
        for ( uns32 i = 0; i < first_column; ++ i )
	    printer << ' ';

	uns32 end_column =
	    end.line == line ? end.column : width;
	if ( end_column <= first_column )
	    end_column = width;
	if ( end_column <= first_column )
	    end_column = first_column + 1;

        for ( uns32 i = first_column;
	      i < end_column; ++ i )
	    printer << mark;
	printer << min::eol;

	if ( line == end.line ) return;

	++ line;

	if ( line == end.line && end.column == 0 )
	    return;

	first_column = 0;
	width = min::print_line
	    ( printer, file, line );
    }
}

void LEX::print_item_lines
	( min::printer printer,
	  LEX::scanner scanner,
	  min::uns32 first,
	  min::uns32 next,
	  char mark,
	  const char * blank_line )
{
    LEX::input_buffer input_buffer =
        scanner->input_buffer;

    const LEX::position begin =
        first < input_buffer->length ?
	    (LEX::position) input_buffer[first] :
	    scanner->next_position;

    const LEX::position end =
        next < input_buffer->length ?
	    (LEX::position) input_buffer[next] :
	    scanner->next_position;

    print_item_lines
        ( printer, scanner->input_file,
	  begin, end, mark, blank_line );
}

// Printing Programs
// -------- --------

// Program lines are printed with min::noautobreak
// and min::set_indent ( IDwidth ).

static const unsigned IDwidth = 12;
    // Width of field containing ID at the beginning
    // of each print_program line.

// printer << pID ( ID ) prints `ID: ' right adjusted in
// a field of width IDwidth.
//
struct pID { uns32 ID;
             pID ( uns32 ID ) : ID ( ID ) {} };
inline min::printer operator <<
	( min::printer printer, const pID & p )
{
    return printer << min::setbreak << p.ID << ": "
                   << min::right ( IDwidth );
}

// See documentation .
//
static uns32 print_instruction
    ( min::printer printer,
      LEX::program program,
      uns32 ID,
      unsigned indent )
{
    if ( ID == 0 ) return 0;

    printer << min::push_parameters
            << min::set_indent ( indent )
	    << min::noautobreak
	    << min::indent;

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
        printer << "ILLEGAL INSTRUCTION TYPE ("
	        << h.pctype << ")" << min::eol
		<< min::pop_parameters;
	return program->length + 1;
    }
    if ( ( ( h.operation & TRANSLATE_TO_FLAG ) != 0 )
	 +
         ( ( h.operation & TRANSLATE_HEX_FLAG ) != 0 )
	 +
         ( ( h.operation & TRANSLATE_OCT_FLAG ) != 0 )
	 > 1 ) printer << "ILLEGAL: ";
    else
    if ( ( h.operation & REQUIRE ) != 0
         &&
	 ( h.operation & TRANSLATE_HEX_FLAG ) == 0
         &&
	 ( h.operation & TRANSLATE_OCT_FLAG ) == 0
         &&
	 ( h.operation & MATCH ) == 0 )
        printer << "ILLEGAL: ";
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
        printer << "ILLEGAL: ";
    else
    if ( ( ( h.operation & GOTO ) != 0 )
	 +
         ( ( h.operation & RETURN ) != 0 )
	 > 1 )
	printer << "ILLEGAL: ";
    else
    if ( ( ( h.operation & CALL ) != 0 )
	 +
         ( ( h.operation & RETURN ) != 0 )
	 > 1 ) printer << "ILLEGAL: ";
    else
    if ( ( h.operation & MATCH )
         &&
         h.atom_table_ID == 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( h.operation & MATCH ) == 0
         &&
         h.atom_table_ID != 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( h.operation & REQUIRE )
         &&
         h.require_dispatcher_ID == 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( h.operation & REQUIRE ) == 0
         &&
         h.require_dispatcher_ID != 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( h.operation & ELSE )
         &&
         h.else_instruction_ID == 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( h.operation & ELSE ) == 0
         &&
         h.else_instruction_ID != 0 )
	printer << "ILLEGAL: ";
    if ( ( h.operation & ERRONEOUS_ATOM )
         &&
         h.erroneous_atom_type == 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( h.operation & ERRONEOUS_ATOM ) == 0
         &&
         h.erroneous_atom_type != 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( h.operation & OUTPUT )
         &&
         h.output_type == 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( h.operation & OUTPUT ) == 0
         &&
         h.output_type != 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( h.operation & GOTO )
         &&
         h.goto_table_ID == 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( h.operation & GOTO ) == 0
         &&
         h.goto_table_ID != 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( h.operation & CALL )
         &&
         h.call_table_ID == 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( h.operation & CALL ) == 0
         &&
         h.call_table_ID != 0 )
	printer << "ILLEGAL: ";

    bool first = true;
#   define OUT ( first ? ( first = false, printer ) : \
                         printer << ", " ) \
	       << min::setbreak
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
	    printer << ",";
	    uns32 n = translate_to_length;
	    for ( uns32 p =
	            ID + instruction_header_length;
		  0 < n; ++ p, -- n )
	        printer << pgraphic ( program[p] );
	}
	printer << ")";
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
        OUT << "ELSE:" << min::eol;
	print_instruction
	    ( printer, program, h.else_instruction_ID,
	               indent );
    }

    if ( first ) printer << "ACCEPT" << min::eol;
    else printer << min::eol;
#   undef OUT

    printer << min::pop_parameters;

    return instruction_length;
}

// Iterator that prints out a list of characters.
// Breaks are set at appropriate points.  Assumes
// printer->parameters.indent is set to the desired
// indent.
//
struct pclist {
    min::printer printer;
    bool empty;

    uns32 c1, c2;
        // If not empty then the range c1-c2 (or just c1
	// if c1 == c2) needs to be printed.  This is
	// delayed to allow c2 to grow.

    pclist ( min::printer printer )
	: printer ( printer ), empty ( true ) {}

    // Print c1-c2 (or just c1 if c1 == c2 ).
    //
    void flush ( void )
    {
        if ( empty ) return;

	if (   printer->column
	     < printer->parameters.indent )
	     printer << min::indent;
	else if (   printer->column
	          > printer->parameters.indent )
	    printer << " ";

	printer << min::setbreak << pgraphic ( c1 );
	if ( c2 != c1 )
	{
	    printer << "-";
	    if ( c2 != 0xFFFFFFFF )
	        printer << pgraphic ( c2 );
	}

	empty = true;
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
    ( min::printer printer, LEX::program program,
      uns32 ID, unsigned indent = IDwidth )
{
    if ( ID == 0 ) return 0;

    printer << min::push_parameters
            << min::set_indent ( indent )
	    << min::noautobreak;

    printer << pID ( ID ) << "DISPATCHER" << min::eol;

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

    printer << min::indent << "Break Elements: "
	<< h.break_elements << min::eol;
    printer << min::indent << "Max Break Elements: "
	<< h.max_break_elements << min::eol;
    printer << min::indent << "Max CType: "
	<< h.max_ctype << min::eol;

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
    // dispatcher_ID if these are non-zero.
    //
    for ( uns32 t = 0; t <= h.max_ctype; ++ t )
    {
        if ( t != tmap[t] ) continue;
	if ( mep[t].instruction_ID == 0
	     &&
	     mep[t].dispatcher_ID == 0 )
	    continue;

	pclist pcl ( printer );
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
	printer << min::eol;

	print_instruction
	    ( printer, program, mep[t].instruction_ID,
	               IDwidth + 4 );
	if ( mep[t].dispatcher_ID != 0 )
	    printer << min::indent
	            << "    Dispatcher ID: "
	            << mep[t].dispatcher_ID << min::eol;
    }

    printer << min::pop_parameters;

    return length;
}

uns32 LEX::print_program_component
	( min::printer printer, LEX::program program,
	  uns32 ID, bool cooked )
{
    printer << min::push_parameters
            << min::noautobreak
	    << min::set_indent ( IDwidth );

    uns32 length;

    switch ( program[ID] )
    {
    case PROGRAM:
    {
	printer << pID ( ID ) << "PROGRAM" << min::eol;
	program_header & h =
	    * (program_header *) & program[ID];
	printer << min::indent << "Initial Table ID: "
	    << h.initial_table_ID << min::eol;
	printer << min::indent << "Max Type: "
	    << h.max_type << min::eol;
	for ( uns32 t = 0; t <= h.max_type; ++ t )
	{
	    uns32 offset =
	        program[ID+program_header_length+t];
	    if ( offset == 0 ) continue;
	    printer << min::indent
	            << t << ": " << min::right ( 8 )
	            << (const char *) & program[ID]
		           + offset
		    << min::eol;
	}
	length = h.component_length;
	break;
    }
    case TABLE:
    {
	printer << pID ( ID ) << "TABLE" << min::eol;
	table_header & h =
	    * (table_header *) & program[ID];
	printer << min::indent << "Mode: "
	        << pmode ( program, h.mode )
		<< min::eol;
	printer << min::indent << "Dispatcher ID: "
	        << h.dispatcher_ID << min::eol;
	if ( cooked )
	    print_instruction
		( printer, program,
		  h.instruction_ID, IDwidth );
	else
	    printer << min::indent << "Instruction ID: "
		    << h.instruction_ID << min::eol;
	length = table_header_length;
	break;
    }
    case DISPATCHER:
    if ( cooked )
    {
	length = print_cooked_dispatcher
		       ( printer, program, ID );
	break;
    }
    else
    {
	printer << pID ( ID ) << "DISPATCHER"
	        << min::eol;
	dispatcher_header & h =
	    * (dispatcher_header *) & program[ID];
	printer << min::indent << "Break Elements: "
	        << h.break_elements << min::eol;
	printer << min::indent << "Max Break Elements: "
	        << h.max_break_elements << min::eol;
	printer << min::indent << "Max CType: "
	        << h.max_ctype << min::eol;
	printer << min::indent << "Breaks: "
	        << min::setbreak
	        << "cmin" << min::right ( 16 )
	        << "type_map_ID" << min::right ( 16 )
	        << min::eol;
	length = dispatcher_header_length;
	uns32 p, n;
	for ( p = ID + length, n = 0;
	      n < h.break_elements;
	      p += break_element_length, ++ n )
	{
	    break_element & be =
		* (break_element *) & program[p];
	    printer << min::indent
		    << pgraphic ( be.cmin )
		    << min::right ( 24 )
		    << be.type_map_ID
		    << min::right ( 16 )
		    << min::eol;
	}
	length += break_element_length
	        * h.max_break_elements;
	printer << min::indent << "Map:   CType: "
	        << min::setbreak
	        << "dispatcher_ID" << min::right ( 16 )
	        << "instruction_ID" << min::right ( 16 )
	        << min::eol;
	uns32 t;
	for ( p = ID + length, t = 0;
	      t <= h.max_ctype;
	      p += map_element_length, ++ t )
	{
	    map_element & me =
		* (map_element *) & program[p];
	    printer << min::indent
		    << t << ": " << min::right ( 14 )
		    << me.dispatcher_ID
		    << min::right ( 16 )
		    << me.instruction_ID
		    << min::right ( 16 )
		    << min::eol;
	}
	length += map_element_length
	        * ( h.max_ctype + 1 );
	break;
    }
    case INSTRUCTION:
    {
	printer << pID ( ID );
	length = print_instruction
		   ( printer, program, ID, IDwidth );
	break;
    }
    case TYPE_MAP:
    {
	printer << pID ( ID ) << "TYPE_MAP" << min::eol;
	type_map_header & h =
	    * (type_map_header *) & program[ID];
	length = type_map_header_length;
	if ( h.singleton_ctype > 0 )
	{
	    printer << h.singleton_ctype
	            << min::right ( IDwidth + 4 )
		    << ": " << pgraphic ( h.cmin )
	            << "-" << pgraphic ( h.cmax )
		    << min::eol;
	}
	else
	{
	    printer << min::push_parameters
	            << min::set_indent ( IDwidth + 6 );

	    uns8 * map = (uns8 *) ( & h + 1 );
	    length += ( h.cmax - h.cmin + 4 ) / 4;
	    for ( unsigned t = 0; t < 256; ++ t )
	    {
		pclist pcl ( printer );
		for ( uns32 c = h.cmin;
		      c <= h.cmax; ++ c )
		{
		    if ( map[c - h.cmin] == t )
		    {
			if ( pcl.empty )
			    printer
				<< min::setbreak
				<< t << ": "
				<< min::right
				       ( IDwidth + 6 );
			pcl.add ( c, c );
		    }
		}
		if ( ! pcl.empty )
		{
		    pcl.flush();
		    printer << min::eol;
		}
	    }

	    printer << min::pop_parameters;
	}
	break;
    }
    default:
    {
	printer << pID ( ID )
	        << "ILLEGAL COMPONENT TYPE("
	        << program[ID] << ")" << min::eol;
	length = program->length - ID;
	break;
    }
    }

    printer << min::pop_parameters;
    return length;
}

void LEX::print_program
	( min::printer printer, LEX::program program,
	  bool cooked )
{
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
	    ( printer, program, ID, cooked );
    }

    if ( ID > program->length )
        printer << "  ILLEGALLY TRUNCATED LAST PROGRAM"
	           " COMPONENT" << min::eol;
}
