// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jul 10 03:14:52 EDT 2012
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
//	Name String Scanning

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
#define MUP min::unprotected

#define FOR(i,n) for ( uns32 i = 0; i < (n); ++ i )


// Data
// ----

static uns32 scanner_stub_disp[] =
    { min::DISP ( & LEX::scanner_struct::program ),
      min::DISP ( & LEX::scanner_struct::input_buffer ),
      min::DISP ( & LEX::scanner_struct
                       ::translation_buffer ),
      min::DISP ( & LEX::scanner_struct
                       ::input ),
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

min::locatable_var<LEX::program>
     LEX::default_program;
min::locatable_var<LEX::input>
     LEX::default_input;
min::locatable_var<LEX::erroneous_atom>
     LEX::default_erroneous_atom;
min::locatable_var<LEX::scanner>
     LEX::default_scanner;

// printer << pID ( ID, program ) prints `ID' or
// `#line_number' (if line_number != 0 ).
//
struct pID
{
    uns32 ID; uns32 line_number;
    pID ( uns32 ID, LEX::program program )
	: ID ( ID ), line_number ( program[ID+1] ) {}
};
inline min::printer operator <<
	( min::printer printer, const pID & p )
{
    if ( p.line_number != 0 )
        return printer << "#" << p.line_number;
    else
        return printer << p.ID;
}

// Program Construction
// ------- ------------

# define ERR min::init ( min::error_message ) \
    << "LEXICAL PROGRAM CONSTRUCTION ERROR: "

# define PUSH(header,length) \
    assert (    sizeof ( header ) \
             == (length) * sizeof ( min::uns32 ) ); \
    min::push ( program, \
		(length), \
                (min::uns32 *) & (header) )

inline min::uns32 round32 ( min::uns32 length )
{
    return   ( length + sizeof ( min::uns32 ) - 1 )
           / sizeof ( min::uns32 );
}

uns32 LEX::create_table
	( uns32 line_number,
	  uns32 mode, LEX::program program )
{
    min::ptr<program_header> php =
	LEX::ptr<program_header> ( program, 0 );
    if (    mode != ATOM
         && mode != MASTER
	 && mode != NONE )
	assert (    php->max_type == 0
	         || mode <= php->max_type );

    uns32 ID = program->length;
    table_header h;
    h.pctype = TABLE;
    h.line_number = line_number;
    h.mode = mode;
    h.dispatcher_ID = 0;
    h.instruction_ID = 0;
    PUSH ( h, table_header_length );

    // First master table becomes the default initial
    // table of the program.
    //
    if ( php->initial_table_ID[0] == 0
         &&
	 mode == MASTER )
    {
	php->initial_table_ID[0] = ID;
    }

    return ID;
}

uns32 LEX::table_mode
	( uns32 ID, LEX::program program )
{
    min::ptr<table_header> thp =
        LEX::ptr<table_header> ( program, ID );
    return thp->mode;
}

void LEX::create_program
	( uns32 line_number,
	  const char * const * type_name,
	  uns32 max_type,
	  min::ref<LEX::program> program_arg )
{
    if ( program_arg == NULL_STUB )
        program_arg = uns32_vec_type.new_gen();
    else
    {
	min::pop
	    ( (LEX::program) program_arg,
	      program_arg->length );
	min::resize
	    ( (LEX::program) program_arg, 1000 );
    }

    // Implicit type conversion of min::ref type does
    // not work well, so -
    //
    LEX::program program = program_arg;

    uns32 ID = program->length;
    assert ( ID == 0 );

    program_header h;
    h.pctype = PROGRAM;
    h.line_number = line_number;
    memset ( h.initial_table_ID, 0,
             sizeof ( h.initial_table_ID ) );
    h.max_type = max_type;
    h.component_length = 0;
    PUSH ( h, program_header_length );

    min::push ( program, max_type + 1 );
    uns32 length = program_header_length + max_type + 1;
    uns32 origin = sizeof ( uns32 ) * length;
    for ( uns32 t = 0; t <= max_type; ++ t )
    {
	if ( type_name == NULL || type_name[t] == NULL )
	{
	    program[ID + program_header_length + t] = 0;
	    continue;
	}
	program[ID + program_header_length + t] =
	    origin;

	uns32 next_origin = origin
	                  + strlen ( type_name[t] ) + 1;
	uns32 next_length = ::round32 ( next_origin );
	min::push ( program, next_length - length );

	strcpy ( (char *) & program[ID] + origin,
	         type_name[t] );
	origin = next_origin;
	length = next_length;
    }

    min::ptr<program_header> php =
	LEX::ptr<program_header> ( program, ID );
    php->component_length = length;
}

uns32 LEX::create_dispatcher
	( uns32 line_number,
	  uns32 max_breakpoints,
	  uns32 max_ctype,
	  LEX::program program )
{

    uns32 ID = program->length;
    dispatcher_header h;
    h.pctype = DISPATCHER;
    h.line_number = line_number;
    h.break_elements = 1;
    h.max_break_elements = max_breakpoints + 1;
    h.max_ctype = max_ctype;
    PUSH ( h, dispatcher_header_length );

    assert (    sizeof ( break_element )
             ==   break_element_length
                * sizeof ( uns32 ) );
    assert (    sizeof ( map_element )
             ==   map_element_length
                * sizeof ( uns32 ) );
    min::push
        ( program,
	      break_element_length
	    * ( max_breakpoints + 1 )
	  +   map_element_length
	    * ( max_ctype + 1 ) );

    return ID;
}

uns32 LEX::create_type_map
	( uns32 line_number,
	  uns32 cmin, uns32 cmax,
	  uns8 * map,
	  LEX::program program )
{
    assert ( cmax >= cmin );
    uns32 ID = program->length;

    type_map_header h;
    h.pctype = TYPE_MAP;
    h.line_number = line_number;
    h.cmin = cmin;
    h.cmax = cmax;
    h.singleton_ctype = 0;
    PUSH ( h, type_map_header_length );

    uns32 length = cmax - cmin + 1;
    min::push ( program, ::round32 ( length ) );
    memcpy ( & program[ID+type_map_header_length],
             map, length );

    return ID;
}

uns32 LEX::create_type_map
	( uns32 line_number,
	  uns32 cmin, uns32 cmax,
	  uns32 ctype,
	  LEX::program program )
{
    assert ( cmax >= cmin );
    uns32 ID = program->length;

    type_map_header h;
    h.pctype = TYPE_MAP;
    h.line_number = line_number;
    h.cmin = cmin;
    h.cmax = cmax;
    assert ( ctype != 0 );
    h.singleton_ctype = ctype;
    PUSH ( h, type_map_header_length );

    return ID;
}

uns32 LEX::create_instruction
	( uns32 line_number,
	  uns32 operation,
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
    assert ( ( ( operation & MATCH ) != 0 )
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

	min::ptr<program_header> php =
	    LEX::ptr<program_header> ( program, 0 );
	assert (    php->max_type == 0
		 || output_type <= php->max_type );
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

    instruction_header h;
    h.pctype = INSTRUCTION;
    h.line_number = line_number;
    h.operation = operation;
    h.atom_table_ID = atom_table_ID;
    h.require_dispatcher_ID = require_dispatcher_ID;
    h.else_instruction_ID = else_instruction_ID;
    h.erroneous_atom_type = erroneous_atom_type;
    h.output_type = output_type;
    h.goto_table_ID = goto_table_ID;
    h.call_table_ID = call_table_ID;
    PUSH ( h, instruction_header_length );

    min::push ( program,
		translate_to_length,
                translation_vector );

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
    min::ptr<dispatcher_header> dhp =
        LEX::ptr<dispatcher_header>
	    ( program, dispatcher_ID );
    assert ( dhp->pctype == DISPATCHER );
    min::ptr<type_map_header> tmhp =
        LEX::ptr<type_map_header>
	    (program, type_map_ID );
    assert ( tmhp->pctype == TYPE_MAP );

    min::ptr<break_element> bep =
        LEX::ptr<break_element>
	    ( program,   dispatcher_ID
                       + dispatcher_header_length );

    uns32 i = 0;
    for ( ; i + 1 < dhp->break_elements
            &&
	    bep[i+1].cmin <= tmhp->cmin;
	    ++ i );

    bool split_next =
	( i == dhp->break_elements - 1 ?
	  tmhp->cmax != 0xFFFFFFFF :
	  tmhp->cmax != bep[i+1].cmin - 1 );

    if ( bep[i].type_map_ID != 0 )
    {
	uns32 old_cmin = bep[i].cmin;
	uns32 old_cmax = i+1 < dhp->break_elements ?
	                 bep[i+1].cmin - 1 :
		         0xFFFFFFFF;
        ERR << "Attempt to attach type map "
	    << pID ( type_map_ID, program )
	    << " with range "
	    << min::puns ( tmhp->cmin, "0x%X" )
	    << "-"
	    << min::puns ( tmhp->cmax, "0x%X" )
	    << " to dispatcher "
	    << pID ( dispatcher_ID, program )
	    << " conflicts with previous attachment of"
	       " type map "
	    << pID ( bep[i].type_map_ID, program )
	    << " to range "
	    << min::puns ( old_cmin, "0x%X" )
	    << "-"
	    << min::puns ( old_cmax, "0x%X" )
	    << min::eol;
        return false;
    }

    if ( i + 1 != dhp->break_elements
         &&
	 bep[i+1].cmin < tmhp->cmin )
    {
        assert ( bep[i+1].type_map_ID != 0 );
	uns32 old_cmin = bep[i+1].cmin;
	uns32 old_cmax = i+2 < dhp->break_elements ?
	                 bep[i+2].cmin - 1 :
		         0xFFFFFFFF;
        ERR << "Attempt to attach type map "
	    << pID ( type_map_ID, program )
	    << " with range "
	    << min::puns ( tmhp->cmin, "0x%X" )
	    << "-"
	    << min::puns ( tmhp->cmax, "0x%X" )
	    << " to dispatcher "
	    << pID ( dispatcher_ID, program )
	    << " conflicts with previous attachment of"
	       " type map "
	    << pID ( bep[i+1].type_map_ID, program )
	    << " to range "
	    << min::puns ( old_cmin, "0x%X" )
	    << "-"
	    << min::puns ( old_cmax, "0x%X" )
	    << min::eol;
        return false;
    }

    int n = 2; // Number of new break elements needed.
    if ( bep[i].cmin == tmhp->cmin ) -- n;
    if ( ! split_next ) -- n;
    if (   dhp->break_elements + n
         > dhp->max_break_elements )
    {
        ERR << "Attempt to attach type map "
	    << pID ( type_map_ID, program )
	    << " to dispatcher "
	    << pID ( dispatcher_ID, program )
            << " fails because dispatcher already has"
	       " too many breaks"
	    << min::eol;
	return false;
    }

    if ( n != 0 )
        memmove ( & bep[i+n], & bep[i],
		    ( dhp->break_elements - i )
		  * sizeof ( break_element ) );
    if ( bep[i].cmin < tmhp->cmin )
    {
	++ i;
	bep[i].cmin = tmhp->cmin;
	bep[i].type_map_ID  = 0;
    }

    if ( split_next )
    {
	bep[i+1].cmin = tmhp->cmax + 1;
	bep[i+1].type_map_ID  = 0;
    }

    dhp->break_elements += n;
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
	min::ptr<table_header> thp =
	    LEX::ptr<table_header>
	        ( program, target_ID );

        if ( component_pctype == DISPATCHER )
	{
	    if ( thp->dispatcher_ID != 0 )
	    {
		ERR << "Attempt to attach dispatcher "
		    << pID ( component_ID, program )
		    << " to table "
		    << pID ( target_ID, program )
		    << " conflicts with previous"
		       " attachment of dispatcher "
		    << pID ( thp->dispatcher_ID,
		             program )
		    << min::eol;
	        return false;
	    }
	    thp->dispatcher_ID = component_ID;
	    return true;
	}
        else if ( component_pctype == INSTRUCTION )
	{
	    if ( thp->instruction_ID != 0 )
	    {
		ERR << "Attempt to attach instruction "
		    << pID ( component_ID, program )
		    << " to table "
		    << pID ( target_ID, program )
		    << " conflicts with previous"
		       " attachment of instruction "
		    << pID ( thp->instruction_ID,
		             program )
		    << min::eol;
	        return false;
	    }
	    thp->instruction_ID = component_ID;
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
    min::ptr<dispatcher_header> dhp =
        LEX::ptr<dispatcher_header>
	    ( program, target_ID );
    assert ( dhp->pctype == DISPATCHER );

    uns32 component_pctype = program[component_ID];
    assert ( component_pctype == DISPATCHER
             ||
	     component_pctype == INSTRUCTION );
    assert ( ctype <= dhp->max_ctype );
    min::ptr<map_element> mep =
        LEX::ptr<map_element>
	    ( program,   target_ID
	               + dispatcher_header_length
		       +   break_element_length
		         * dhp->max_break_elements
		       +   map_element_length
		         * ctype );

    if ( component_pctype == DISPATCHER )
    {
	if ( mep->dispatcher_ID != 0 )
	{
	    ERR << "Attempt to attach dispatcher "
		<< pID ( component_ID, program )
		<< " to dispatcher "
		<< pID ( target_ID, program )
		<< " ctype "
		<< ctype
		<< " conflicts with previous attachment"
		   " of dispatcher "
		<< pID ( mep->dispatcher_ID, program )
		<< min::eol;
	    return false;
	}
	mep->dispatcher_ID = component_ID;
	return true;
    }
    else if ( component_pctype == INSTRUCTION )
    {
	if ( mep->instruction_ID != 0 )
	{
	    ERR << "Attempt to attach instruction "
		<< pID ( component_ID, program )
		<< " to dispatcher "
		<< pID ( target_ID, program )
		<< " ctype "
		<< ctype
		<< " conflicts with previous attachment"
		   " of instruction "
		<< pID ( mep->instruction_ID, program )
		<< min::eol;
	    return false;
	}
	mep->instruction_ID = component_ID;
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
	    min::ptr<program_header> php =
	        LEX::ptr<program_header>
		    ( program, cID );
	    FOR(i,php->max_type+1) NEXT;
	    ID = cID + php->component_length;
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

	    min::ptr<dispatcher_header> dhp =
		LEX::ptr<dispatcher_header>
		    ( program, cID );
	    FOR(i,  break_element_length
		  * dhp->max_break_elements) NEXT;
	    FOR(i,  map_element_length
		  * ( dhp->max_ctype + 1 )) NEXT;
	    break;
	}
	case TYPE_MAP:
	{
	    FOR(i,type_map_header_length-1) NEXT;

	    min::ptr<type_map_header> tmhp =
		LEX::ptr<type_map_header>
		    ( program, cID );
	    if ( tmhp->singleton_ctype == 0 )
		ID += ( tmhp->cmax - tmhp->cmin + 4 )
		    / 4;
	    break;
	}
	case INSTRUCTION:
	{
	    FOR(i,instruction_header_length-1) NEXT;
	    
	    min::ptr<instruction_header> ihp =
		LEX::ptr<instruction_header>
		    ( program, cID );
	    if (   ihp->operation
		 & LEX::TRANSLATE_TO_FLAG )
		FOR(i,LEX::translate_to_length
			    ( ihp->operation ))
		    NEXT;
	    break;
	}
	default:
	    min::init ( min::error_message )
	        << "LEXICAL PROGRAM ENDIAN CONVERSION"
		   " ERROR: undefined program component"
		   " type at offset "
		<< pID ( cID, program )
		<< " in the program vector of uns32"
		   " elements."
		<< min::eol;
	    return false;
	}
    }
#   undef NEXT
    return true;
}

# undef ERR

// Scanner Closures
// ------- --------

static bool default_input_get
	( LEX::scanner scanner,
	  LEX::input input )
{
    min::file file = scanner->input_file;

    LEX::inchar ic;
    ic.line = scanner->next_position.line;
    ic.offset = scanner->next_position.offset;
    ic.indent = scanner->next_indent;
    MIN_ASSERT (    ic.line
                 == file->next_line_number );

    min::uns32 offset = min::next_line ( file );
    min::uns32 length;
    bool add_eol = true;

    if ( offset == min::NO_LINE )
    {
        length = min::remaining_length ( file );
        if ( length == 0 ) return false;
	offset = min::remaining_offset ( file );
	min::skip_remaining ( file );
	add_eol = false;
    }
    else
        length = ::strlen ( & file->buffer[offset] );

    LEX::input_buffer input_buffer =
        scanner->input_buffer;

    while ( length != 0 )
    {
	const char * beginp = & file->buffer[offset];
	const char * endp = beginp + length;
	const char * p = beginp;
	uns32 unicode =
	    min::utf8_to_unicode ( p, endp );
	uns32 bytes_read = p - beginp;
	assert ( length >= bytes_read );
	offset += bytes_read;
	length -= bytes_read;

	ic.character = unicode;
	min::push(input_buffer) = ic;

	ic.offset += bytes_read;
	if ( ic.indent != AFTER_GRAPHIC )
	    switch ( unicode )
	{
	case ' ' : ++ ic.indent;
	           break;
	case '\t': ic.indent += 8 - ic.indent % 8;
	           break;
	case '\f':
	case '\v':
		   break;
	default:
		   ic.indent = AFTER_GRAPHIC;
	}
    }

    if ( add_eol )
    {
	ic.character = '\n';
	min::push(input_buffer) = ic;
	++ ic.line;
	ic.offset = 0;
	ic.indent = 0;
    }

    scanner->next_position = (min::position) ic;
    scanner->next_indent = ic.indent;

    return true;
}

static void default_erroneous_atom_announce
	( uns32 first, uns32 next, uns32 type,
	  LEX::scanner scanner,
	  LEX::erroneous_atom erroneous_atom )
{
    if ( scanner->printer == min::NULL_STUB )
        min::init ( LEX::printer_ref(scanner) );

    ++ erroneous_atom->count;

    scanner->printer
        << "ERRONEOUS ATOM: ";

    if ( erroneous_atom->mode == LEX::BASIC )
    {
        scanner->printer
	    << LEX::perroneous_atom
	           ( scanner, first, next, type )
	    << min::eol;
	return;
    }

    scanner->printer
        << LEX::pmode ( scanner->program, type );

    switch ( erroneous_atom->mode )
    {
    case NORMAL:
	scanner->printer
	    << " " << LEX::pline_numbers
	                  ( scanner, first, next );

    case NO_LINE_NUMBERS:
	scanner->printer << ":" << min::eol;

	LEX::print_phrase_lines
	    ( scanner->printer, scanner, first, next,
	      '^', NULL, NULL, NULL );
	break;

    default:
        MIN_ABORT
	    ( "Bad erroneous atom announce mode" );
    }
}

void LEX::init
	( min::ref<LEX::input> input,
	  bool (*get) ( LEX::scanner scanner,
			LEX::input input ) )
{
    if ( input == NULL_STUB )
        input = ::input_type.new_stub();
    input->get = get;
}

void LEX::init
	( min::ref<LEX::erroneous_atom> erroneous_atom,
	  void (* announce )
	    ( uns32 first, uns32 next, uns32 type,
	      LEX::scanner scanner,
	      LEX::erroneous_atom erroneous_atom ),
	  LEX::uns32 mode )
{
    if ( erroneous_atom == NULL_STUB )
        erroneous_atom =
	    ::erroneous_atom_type.new_stub();
    erroneous_atom->announce = announce;
    erroneous_atom->mode = mode;
    erroneous_atom->count = 0;
}

void LEX::init
	( min::ref<LEX::erroneous_atom> erroneous_atom,
	  LEX::uns32 mode )
{
    if ( erroneous_atom == NULL_STUB )
    {
        erroneous_atom =
	    ::erroneous_atom_type.new_stub();
	erroneous_atom->announce = 
	    ::default_erroneous_atom_announce;
	erroneous_atom->count = 0;
    }
    erroneous_atom->mode = mode;
}

static void initialize ()
{
    LEX::init ( LEX::default_input,
		::default_input_get );
    LEX::init ( LEX::default_erroneous_atom,
		::default_erroneous_atom_announce );
}
static min::initializer initializer ( ::initialize );


// Scanning
// --------

// Return true if table_ID is in return_stack.
//
inline bool is_recursive
        ( uns32 return_stack[LEX::return_stack_size],
          uns32 return_stack_p,
	  uns32 table_ID )
{
    for ( uns32 i = 0;
          i < return_stack_p; ++ i )
    {
        if ( return_stack[i] == table_ID )
	    return true;
    }
    return false;
}


// We assume the program is well formed, in that an
// XXX_ID actually points at a program component of
// pctype XXX.  We check this with asserts (the attach
// statements check this).  Everything else found
// wrong with the program is a SCAN_ERROR.

void LEX::init ( min::ref<LEX::scanner> scanner )
{

    if ( scanner == NULL_STUB )
    {
	scanner = scanner_type.new_stub();

	LEX::input_buffer_ref(scanner) =
	    inchar_vec_type.new_gen();
	LEX::translation_buffer_ref(scanner) =
	    uns32_vec_type.new_gen();
	scanner->initial_table = 0;
    }
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
	scanner->next_position.offset = 0;
	scanner->next_indent = 0;

	scanner->next = 0;
    }

    scanner->reinitialize = true;
}

void LEX::init_program
	( min::ref<LEX::scanner> scanner,
	  LEX::program program,
	  LEX::uns32 initial_table )
{
    init ( scanner );
    program_ref(scanner) = program;
    scanner->initial_table = initial_table;
}

void LEX::init_input_file
	( min::ref<LEX::scanner> scanner,
	  min::file input_file )
{
    init ( scanner );
    input_file_ref(scanner) = input_file;
}

void LEX::init_printer
	( min::ref<LEX::scanner> scanner,
	  min::printer printer )
{
    init ( scanner );
    LEX::printer_ref(scanner) = printer;
}

bool LEX::init_input_named_file
	( min::ref<LEX::scanner> scanner,
	  min::gen file_name,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
	  
{
    init ( scanner );
    return min::init_input_named_file
	( input_file_ref(scanner),
	  file_name,
	  print_flags,
	  spool_lines );
}

void LEX::init_input_stream
	( min::ref<LEX::scanner> scanner,
	  std::istream & istream,
	  min::uns32 print_flags,
	  uns32 spool_lines )
{
    init ( scanner );
    min::init_input_stream
	( input_file_ref(scanner),
	  istream, print_flags, spool_lines );
}

void LEX::init_input_string
	( min::ref<LEX::scanner> scanner,
	  min::ptr<const char> data,
	  min::uns32 print_flags,
	  uns32 spool_lines )
{
    init ( scanner );
    min::init_input_string
	( input_file_ref(scanner),
	  data, print_flags, spool_lines );
}

void LEX::init_input
	( min::ref<LEX::scanner> scanner )
{
    init ( scanner );
    min::init_input ( input_file_ref(scanner) );
}

void LEX::init_print_flags
	( min::ref<LEX::scanner> scanner,
	  min::uns32 print_flags )
{
    init ( scanner );
    min::init_print_flags
	( input_file_ref(scanner), print_flags );
}

void LEX::init_spool_lines
	( min::ref<LEX::scanner> scanner,
	  min::uns32 spool_lines )
{
    init ( scanner );
    min::init_spool_lines
	( input_file_ref(scanner), spool_lines );
}

min::printer LEX::init_ostream
	( min::ref<LEX::scanner> scanner,
	  std::ostream & out )
{
    init ( scanner );
    return min::init_ostream
	( LEX::printer_ref(scanner), out );
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
// If no_line_number is true, surpress printing line
// number/ID (using pID).
//
static uns32 print_instruction
    ( min::printer printer,
      LEX::program program, uns32 ID,
      bool no_line_number = false );

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
	    << " Dispatcher = "
	    << pID ( dispatcher_ID, program );

    min::ptr<dispatcher_header> dhp =
	LEX::ptr<dispatcher_header>
	    ( program, dispatcher_ID );

    min::ptr<break_element> bep =
	LEX::ptr<break_element>
	    ( program,   dispatcher_ID
		       + dispatcher_header_length );

    // Binary search of break elements.
    //
    // Invariant:
    //     bep[low].cmin <= c < bep[high].cmin
    // where bep[high].cmin = infinity if
    // high == dhp->break_elements.
    //
    uns32 low = 0,
	  high = dhp->break_elements,
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
	        << " Type Map = "
		<< pID ( type_map_ID, program );
	min::ptr<type_map_header> tmhp =
	    LEX::ptr<type_map_header>
		( program, type_map_ID );
	ctype = tmhp->singleton_ctype;
	if ( ctype == 0 )
	    ctype = ( (uns8 *) ( & tmhp[1] ) )
		    [c - tmhp->cmin];
    }

    if ( trace )
	scanner->printer
	    << " CType = " << ctype << min::eol;

    return ctype;
}

// Scan atom given current table.  Locate and process
// instruction group, but not
//
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
    ( LEX::scanner scanner,
      uns32 return_stack[LEX::return_stack_size],
      uns32 & return_stack_p,
      uns32 & atom_length )
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
	    << pID ( scanner->current_table_ID,
	             program )
	    << min::eol;

    min::ptr<table_header> cathp =
	LEX::ptr<table_header>
	    ( program, scanner->current_table_ID );

    // As we scan we recognize longer and longer atoms.
    // If at any point we cannot continue, we revert to
    // the longest atom recognized so far (if none, we
    // may have a scan error).

    uns32 instruction_ID = cathp->instruction_ID;
    atom_length = 0;
	// Length and instruction_ID for the longest
	// atom recognized so far.
    uns32 length = 0;
	// Number of characters scanned so far starting
	// at input_buffer[next].
    uns32 dispatcher_ID = cathp->dispatcher_ID;
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
	     ! (*scanner->input->get)
	         ( scanner, scanner->input ) )
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

	min::ptr<dispatcher_header> dhp =
	    LEX::ptr<dispatcher_header>
	        ( program, dispatcher_ID );

	if ( ctype > dhp->max_ctype )
	{
	    scan_error ( scanner, length )
		    << "Ctype " << ctype
		    << " computed for character "
		    << LEX::pgraphic ( c )
		    << " is too large for dispatcher "
		    << pID ( dispatcher_ID, program )
		    << min::eol;
	    return SCAN_ERROR;
	}

	// Map to next dispatcher and current
	// instruction.  If there is a current
	// instruction, we have recognized a longer
	// atom.
	//
	min::ptr<map_element> mep =
	    LEX::ptr<map_element>
		( program,
		    dispatcher_ID
		  + dispatcher_header_length
		  +   break_element_length
		    * dhp->max_break_elements );
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
	        ( scanner->printer << "  ",
		  program, instruction_ID );

	bool fail = false;

	min::ptr<instruction_header> ihp =
	    LEX::ptr<instruction_header>
		( program, instruction_ID );
	uns32 op = ihp->operation;

	uns32 keep_length = atom_length;
	    // Keep_length becomes the effective atom_
	    // length until we are sure we are not
	    // no-oping the instruction for the sake
	    // of an ELSE.

	if ( op & MATCH )
	{
	    min::ptr<table_header> thp =
		LEX::ptr<table_header>
		    ( program, ihp->atom_table_ID );
	    assert ( cathp->pctype == TABLE );
	    if ( thp->mode != ATOM )
	    {
		scan_error ( scanner, atom_length )
		    << "MATCH in instruction "
		    << pID ( instruction_ID, program )
		    << " executed by table "
		    << pID ( scanner->current_table_ID,
		             program )
		    << " targets non-atom table"
		    << min::eol;
		return SCAN_ERROR;
	    }
	    else if (    return_stack_p
		      == LEX::return_stack_size )
	    {
		scan_error ( scanner, atom_length )
		    << "MATCH in instruction "
		    << pID ( instruction_ID, program )
		    << " executed by table "
		    << pID ( scanner->current_table_ID,
		             program )
		    << " but return stack is full"
		    << min::eol;
		return SCAN_ERROR;
	    }

	    return_stack[return_stack_p++] =
	            scanner->current_table_ID;

	    if ( is_recursive
	             ( return_stack,
		       return_stack_p,
		       ihp->atom_table_ID ) )
	    {
		scan_error ( scanner, atom_length )
		    << "Recursive MATCH to table "
		    << pID ( ihp->atom_table_ID,
		             program )
		    << " in instruction "
		    << pID ( instruction_ID, program )
		    << " executed by table "
		    << pID ( scanner->current_table_ID,
		             program )
		    << min::eol;
		return SCAN_ERROR;
	    }

	    scanner->current_table_ID =
	        ihp->atom_table_ID;
	    uns32 tinstruction_ID =
	        scan_atom ( scanner,
		            return_stack,
			    return_stack_p,
			    keep_length );

	    scanner->current_table_ID =
		return_stack[--return_stack_p];

	    if ( tinstruction_ID == 0 )
	        return SCAN_ERROR;

	    min::ptr<instruction_header> tihp =
		LEX::ptr<instruction_header>
		    ( program, tinstruction_ID );
	    if ( tihp->operation & FAIL )
	    	fail = true;
	}
	else if ( op & (   TRANSLATE_HEX_FLAG
		         | TRANSLATE_OCT_FLAG ) )
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

	if ( ! fail && ( op & REQUIRE ) )
	{
	    uns32 dispatcher_ID =
		ihp->require_dispatcher_ID;
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

		min::ptr<dispatcher_header> dhp =
		    LEX::ptr<dispatcher_header>
		        ( program, dispatcher_ID );

		if ( ctype > dhp->max_ctype )
		{
		    scan_error ( scanner, length )
		        << "Ctype " << ctype
			<< " computed for character "
			<< pgraphic ( c )
			<< " is too large for"
			   " dispatcher "
		        << pID ( dispatcher_ID,
			         program )
			<< min::eol;
		    return SCAN_ERROR;
		}

		if ( ctype == 0 )
		{
		    fail = true;
		    break;
		}

		// Map to next dispatcher.
		//
		min::ptr<map_element> mep =
		    LEX::ptr<map_element>
			( program,
			    dispatcher_ID
			  + dispatcher_header_length
			  +   break_element_length
			    * dhp->max_break_elements );

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
		    ihp->else_instruction_ID;
		if ( instruction_ID == 0 )
		{
		    scan_error ( scanner, length )
		        << "No instruction for ELSE in"
			   " failed instruction "
		        << pID ( instruction_ID,
			         program )
			<< " executed by table "
		        << pID ( scanner->
			             current_table_ID,
			         program )
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
		    << pID ( instruction_ID, program )
		    << " executed by table "
		    << pID ( scanner->current_table_ID,
		             program )
		    << min::eol;
		return SCAN_ERROR;
	    }
	}

	// We are at un-failed instruction of
	// instruction group.
	//
	atom_length = keep_length;

	if ( op & KEEP_FLAG )
	{
	    uns32 keep = LEX::keep_length ( op );
	    if ( keep > atom_length )
	    {
		scan_error ( scanner, atom_length )
		    << "Keep length(" << keep
		    << ") greater than atom length("
		    << atom_length << ")" << min::eol;
		return SCAN_ERROR;
	    }
	    atom_length = keep;
	}

	if ( op & TRANSLATE_TO_FLAG )
	{
	    min::pop ( translation_buffer,
		         translation_buffer->length
		       - tnext );
	    uns32 translate_to_length =
		LEX::translate_to_length ( op );
	    if ( translate_to_length > 0 )
		min::push
		    ( translation_buffer,
		      translate_to_length,
		      min::ptr<const uns32>
		          ( ihp + 1 ) );
	}
	else if ( ! ( op & (   MATCH
	                     | TRANSLATE_HEX_FLAG
	                     | TRANSLATE_OCT_FLAG ) ) )
	{
	    uns32 p = scanner->next;
	    for ( uns32 i = 0; i < atom_length; ++ i )
		min::push(translation_buffer) =
		    input_buffer[p++].character;
	}

	if ( op & ERRONEOUS_ATOM )
	{
	    if (    scanner->erroneous_atom
	              == NULL_STUB )
	    {
		scan_error ( scanner, atom_length )
		    << "ERRONEOUS_ATOM in instruction "
		    << pID ( instruction_ID, program )
		    << " executed by table "
		    << pID ( scanner->current_table_ID,
		             program )
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
		      ihp->erroneous_atom_type,
		      scanner,
		      scanner->erroneous_atom );
	    }
	}

	return instruction_ID;
    }
    abort();
}

uns32 LEX::scan ( uns32 & first, uns32 & next,
                  LEX::scanner scanner )
{
    if ( scanner->reinitialize )
    {
        if ( scanner->input == NULL_STUB )
	    input_ref(scanner) = default_input;

        if ( scanner->erroneous_atom == NULL_STUB )
	    erroneous_atom_ref(scanner) =
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
	else if (    scanner->initial_table
	          >= LEX::MAX_INITIAL_TABLES )
	{
	    min::init ( min::error_message )
	        << "LEXICAL SCANNER ERROR:"
		   " initial_table >="
		   " MAX_INITIAL_TABLES"
		<< min::eol;
	    return SCAN_ERROR;
	}

	min::ptr<program_header> php =
	    LEX::ptr<program_header>
		( scanner->program, 0 );
	scanner->current_table_ID =
	    php->initial_table_ID
	        [scanner->initial_table];

	if ( scanner->current_table_ID == 0 )
	{
	    min::init ( min::error_message )
	        << "LEXICAL SCANNER ERROR:"
		   " initial_table "
		<< scanner->initial_table
		<< " is undefined"
		<< min::eol;
	    return SCAN_ERROR;
	}

	if ( scanner->input_file == NULL_STUB )
	{
	    init_input_stream
	        ( input_file_ref(scanner), std::cin );
	    init_file_name
	        ( input_file_ref(scanner),
		  min::new_str_gen
		      ( "standard input" ) );
	}

        scanner->scan_error = false;
        scanner->reinitialize = false;

	min::ptr<table_header> cathp =
	    LEX::ptr<table_header>
		( scanner->program,
		  scanner->current_table_ID );
	assert ( cathp->pctype == TABLE );
	assert ( cathp->mode == MASTER );
    }

    assert ( scanner->scan_error == false );

    if (    scanner->trace != 0
	 && scanner->printer == NULL_STUB )
	min::init ( LEX::printer_ref(scanner) );

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

    // We scan atoms until we get to a point where the
    // table is MASTER mode and the lexeme type is not
    // NONE, at which point we output a lexeme.
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
    uns32 lexeme_type = NONE;
    uns32 loop_count = program->length;
        // Set to max number of tables in order to
	// detect endless loops.
    uns32 return_stack[LEX::return_stack_size];
    uns32 return_stack_p;
	// Return stack containing return_stack_p
	// elements (0 is first and return_stack_p
	// - 1 element is top).
    while ( true )
    {
        // Scan next atom of current lexeme.

	min::ptr<table_header> cathp =
	    LEX::ptr<table_header>
		( program,
		  scanner->current_table_ID );

	assert ( cathp->mode != ATOM );

	if ( cathp->mode == MASTER )
	{
	    if ( lexeme_type != NONE )
	        break;

	    first = scanner->next;
	    min::pop ( translation_buffer,
		       translation_buffer->length );

	    return_stack_p = 0;
	}
	else if ( cathp->mode != NONE )
	    lexeme_type = cathp->mode;

	// Scan atom.  Return atom length and
	// instruction.
	//
	uns32 atom_length;

	uns32 instruction_ID =
	    scan_atom ( scanner,
	                return_stack,
			return_stack_p,
			atom_length );

	if ( instruction_ID == 0 )
	    return SCAN_ERROR;

	min::ptr<instruction_header> ihp =
	    LEX::ptr<instruction_header>
		( program, instruction_ID );
        uns32 op = ihp->operation;

	if ( op & OUTPUT )
	    lexeme_type = ihp->output_type;

	if ( op & RETURN )
	{
	    if (    return_stack_p
	         == LEX::return_stack_size )
	    {
		scan_error ( scanner, atom_length )
		    << "RETURN in instruction "
		    << pID ( instruction_ID, program )
		    << " executed by table "
		    << pID ( scanner->current_table_ID,
		             program )
		    << " but return stack is empty"
		    << min::eol;
		return SCAN_ERROR;
	    }
	    scanner->current_table_ID =
		return_stack[--return_stack_p];
	    assert (    program
	                    [scanner->current_table_ID]
		     == TABLE );
	}
	else if ( op & GOTO )
	{
	    scanner->current_table_ID =
	        ihp->goto_table_ID;
	    min::ptr<table_header> cathp =
		LEX::ptr<table_header>
		    ( program,
		      scanner->current_table_ID );
	    assert ( cathp->pctype == TABLE );
	    if ( cathp->mode == ATOM )

	    {
		scan_error ( scanner, atom_length )
		    << "GOTO in instruction "
		    << pID ( instruction_ID, program )
		    << " executed by table "
		    << pID ( scanner->current_table_ID,
		             program )
		    << " targets atom table"
		    << min::eol;
		return SCAN_ERROR;
	    }
	}

	if ( op & CALL )
	{
	    if (    return_stack_p
		 == LEX::return_stack_size )
	    {
		scan_error ( scanner, atom_length )
		    << "CALL in instruction "
		    << pID ( instruction_ID, program )
		    << " executed by table "
		    << pID ( scanner->current_table_ID,
		             program )
		    << " but return stack is full"
		    << min::eol;
		return SCAN_ERROR;
	    }

	    return_stack[return_stack_p++] =
	        scanner->current_table_ID;

	    if ( is_recursive
	             ( return_stack,
		       return_stack_p,
	               ihp->call_table_ID ) )
	    {
		scan_error ( scanner, atom_length )
		    << "Recursive CALL to table "
		    << pID ( ihp->call_table_ID,
		             program )
		    << " in instruction "
		    << pID ( instruction_ID, program )
		    << " executed by table "
		    << pID ( scanner->current_table_ID,
		             program )
		    << min::eol;
		return SCAN_ERROR;
	    }

	    scanner->current_table_ID =
	        ihp->call_table_ID;

	    min::ptr<table_header> cathp =
		LEX::ptr<table_header>
		    ( program,
		      scanner->current_table_ID );
	    assert ( cathp->pctype == TABLE );
	    if ( cathp->mode == MASTER
	         ||
		 cathp->mode == ATOM )
	    {
		scan_error ( scanner, atom_length )
		    << "CALL in instruction "
		    << pID ( instruction_ID, program )
		    << " executed by table "
		    << pID ( scanner->current_table_ID,
		             program )
		    << " targets non-(sub)lexeme table"
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

    next = scanner->next;

    switch ( lexeme_type )
    {
    case MASTER:
    case ATOM:
    case NONE:
    case SCAN_ERROR:
    {
	scan_error ( scanner, next - first, first )
	    << "Returning lexeme with bad type("
	    << pmode ( scanner->program, lexeme_type )
	    << ")" << min::eol;
	return SCAN_ERROR;
    }
    }

    return lexeme_type;
}

// See documentation above.
//
static min::printer scan_error
        ( LEX::scanner scanner,
	  uns32 length, uns32 next )
{
    scanner->scan_error = true;

    min::init ( min::error_message )
        << "LEXICAL SCANNER ERROR: current_table "
	             << pID ( scanner->current_table_ID,
		              scanner->program );
    min::position position =
	scanner->next < scanner->input_buffer->length ?
	    (min::position)
	        scanner->input_buffer[next] :
	    scanner->next_position;
    min::uns32 indent =
	scanner->next < scanner->input_buffer->length ?
	    scanner->input_buffer[next].indent :
	    scanner->next_indent;
    min::error_message << ": position "
	               << position.line << "("
	               << position.offset << ")";
    if ( indent != LEX::AFTER_GRAPHIC )
	min::error_message << indent;

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

    printer << min::save_print_format
            << min::graphic << min::gbreak;
    while ( first < next )
        printer << min::punicode
	    ( scanner->input_buffer[first++]
	              .character );
    return printer << min::restore_print_format;
}

min::printer operator <<
	( min::printer printer,
	  const LEX::ptranslation & ptranslation )
{
    LEX::translation_buffer translation_buffer =
        ptranslation.scanner->translation_buffer;

    if ( translation_buffer->length == 0 )
        return printer << "<empty>";

    printer << min::save_print_format
            << min::graphic << min::gbreak;
    for ( unsigned i = 0;
          i < translation_buffer->length; ++ i )
        printer << min::punicode
	    ( translation_buffer[i] );
    return printer << min::restore_print_format;
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

    min::position position =
        first < input_buffer->length ?
	    (min::position) input_buffer[first] :
	    scanner->next_position;
    min::uns32 indent =
        first < input_buffer->length ?
	    input_buffer[first].indent :
	    scanner->next_indent;

    printer << position.line << "("
	    << position.offset << ")";
    if ( indent != LEX::AFTER_GRAPHIC )
	printer << indent;
    printer << ": "
	    << min::reserve ( next + 1 - first )
	    << LEX::pinput ( scanner, first, next );
    return printer;
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
    min::ptr<program_header> php =
        LEX::ptr<program_header> ( program, 0 );
    if ( pmode.mode <= php->max_type )
    {
        uns32 offset =
	    program[program_header_length + mode];
	if ( offset != 0 )
	    return printer
	        << min::save_print_format
	        << min::nohbreak
	        << min::ptr<char> ( php ) + offset
	        << min::restore_print_format;
    }

    switch ( mode )
    {
    case MASTER:
	return printer << "MASTER";
    case ATOM:
	return printer << "ATOM";
    case NONE:
	return printer << "NONE";
    case SCAN_ERROR:
	return printer << "SCAN_ERROR";
    default:
	return printer << min::save_print_format
	               << min::nohbreak
	               << "TYPE (" << mode << ")"
	               << min::restore_print_format;
    }
}

min::pline_numbers LEX::pline_numbers
	( LEX::scanner scanner,
	  min::uns32 first, min::uns32 next )
{
    LEX::input_buffer input_buffer =
        scanner->input_buffer;

    min::phrase_position position;
    position.begin =
        first < input_buffer->length ?
	input_buffer[first] :
	scanner->next_position;
    position.end =
        next < input_buffer->length ?
	input_buffer[next] :
	scanner->next_position;

    return min::pline_numbers
		( scanner->input_file, position );
}

void LEX::print_phrase_lines
	( min::printer printer,
	  LEX::scanner scanner,
	  min::uns32 first,
	  min::uns32 next,
	  char mark,
	  const char * blank_line,
	  const char * end_of_file,
	  const char * unavailable_line )
{
    LEX::input_buffer input_buffer =
        scanner->input_buffer;

    min::phrase_position position;
    position.begin =
        first < input_buffer->length ?
	    (min::position) input_buffer[first] :
	    scanner->next_position;

    position.end =
        next < input_buffer->length ?
	    (min::position) input_buffer[next] :
	    scanner->next_position;

    min::print_phrase_lines
        ( printer, scanner->input_file, position, mark,
	  blank_line, end_of_file, unavailable_line );
}

// Printing Programs
// -------- --------

// Program lines are printed with min::nohbreak
// and min::set_indent ( IDwidth ).

static const unsigned IDwidth = 12;
    // Width of field containing ID at the beginning
    // of each print_program line.

// printer << pIDindent ( ID, program ) prints `ID: ' or
// `#line_number: ' (if line_number != 0 ) right
// adjusted in a field of width IDwidth.
//
struct pIDindent
{
    uns32 ID; uns32 line_number;
    pIDindent ( uns32 ID, LEX::program program )
	: ID ( ID ), line_number ( program[ID+1] ) {}
};
inline min::printer operator <<
	( min::printer printer, const pIDindent & p )
{
    printer << min::set_break;
    if ( p.line_number != 0 )
        printer << "#" << p.line_number;
    else
        printer << p.ID;
    return printer << ": " << min::right ( IDwidth );
}

// Print instruction at program[ID] at the current
// column, ending with a new line.  Preface with the
// instruction line number unless no_line_number is
// true.  If no_line_number is true, print
// ELSE(else-ID) instead of ELSE: followed by ELSE
// instruction.
//
static uns32 print_instruction
    ( min::printer printer,
      LEX::program program,
      uns32 ID,
      bool no_line_number )
{
    if ( ID == 0 ) return 0;

    min::uns32 indent = printer->column;
    printer << min::bom
	    << min::nohbreak;

    min::ptr<instruction_header> ihp =
        LEX::ptr<instruction_header> ( program, ID );
    uns32 op = ihp->operation;
    uns32 translate_to_length = 0;
    uns32 instruction_length =
        instruction_header_length;

    if ( ihp->pctype != INSTRUCTION )
    {
        if ( ! no_line_number )
	    printer << ID << ": ";
        printer << min::place_indent ( 0 )
	        << "ILLEGAL INSTRUCTION TYPE ("
	        << ihp->pctype << ")"
		<< min::eom;
	return program->length + 1;
    }

    if ( ! no_line_number )
        printer << pID ( ID, program ) << ": ";
    printer << min::place_indent ( 0 );

    if ( op & TRANSLATE_TO_FLAG )
    {
        translate_to_length =
	    LEX::translate_to_length ( op );
	instruction_length += translate_to_length;
    }

    if ( ( ( op & MATCH ) != 0 )
	 +
         ( ( op & TRANSLATE_HEX_FLAG ) != 0 )
	 +
         ( ( op & TRANSLATE_OCT_FLAG ) != 0 )
	 > 1 ) printer << "ILLEGAL: ";
    else
    if ( ( op & REQUIRE ) != 0
         &&
	 ( op & TRANSLATE_HEX_FLAG ) == 0
         &&
	 ( op & TRANSLATE_OCT_FLAG ) == 0
         &&
	 ( op & MATCH ) == 0 )
        printer << "ILLEGAL: ";
    else
    if ( ( op & ELSE ) != 0
         &&
	 ( op & TRANSLATE_HEX_FLAG ) == 0
         &&
	 ( op & TRANSLATE_OCT_FLAG ) == 0
         &&
	 ( op & MATCH ) == 0
         &&
	 ( op & REQUIRE ) == 0 )
        printer << "ILLEGAL: ";
    else
    if ( ( ( op & GOTO ) != 0 )
	 +
         ( ( op & RETURN ) != 0 )
	 > 1 )
	printer << "ILLEGAL: ";
    else
    if ( ( ( op & CALL ) != 0 )
	 +
         ( ( op & RETURN ) != 0 )
	 > 1 ) printer << "ILLEGAL: ";
    else
    if ( ( op & MATCH )
         &&
         ihp->atom_table_ID == 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( op & MATCH ) == 0
         &&
         ihp->atom_table_ID != 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( op & REQUIRE )
         &&
         ihp->require_dispatcher_ID == 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( op & REQUIRE ) == 0
         &&
         ihp->require_dispatcher_ID != 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( op & ELSE )
         &&
         ihp->else_instruction_ID == 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( op & ELSE ) == 0
         &&
         ihp->else_instruction_ID != 0 )
	printer << "ILLEGAL: ";
    if ( ( op & ERRONEOUS_ATOM )
         &&
         ihp->erroneous_atom_type == 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( op & ERRONEOUS_ATOM ) == 0
         &&
         ihp->erroneous_atom_type != 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( op & OUTPUT )
         &&
         ihp->output_type == 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( op & OUTPUT ) == 0
         &&
         ihp->output_type != 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( op & GOTO )
         &&
         ihp->goto_table_ID == 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( op & GOTO ) == 0
         &&
         ihp->goto_table_ID != 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( op & CALL )
         &&
         ihp->call_table_ID == 0 )
	printer << "ILLEGAL: ";
    else
    if ( ( op & CALL ) == 0
         &&
         ihp->call_table_ID != 0 )
	printer << "ILLEGAL: ";

    bool first = true;
#   define OUT ( first ? ( first = false, printer ) : \
                         printer << ", " ) \
	       << min::set_break
    if ( op & KEEP_FLAG )
        OUT << "KEEP("
	     << LEX::keep_length ( op )
	     << ")";

    if ( op & TRANSLATE_TO_FLAG )
    {
        translate_to_length =
	     LEX::translate_to_length ( op );
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

    if ( op & TRANSLATE_HEX_FLAG )
        OUT << "TRANSLATE_HEX("
	    << LEX::prefix_length ( op )
	    << ","
	    << LEX::postfix_length ( op )
	    << ")";

    if ( op & TRANSLATE_OCT_FLAG )
        OUT << "TRANSLATE_OCT("
	    << LEX::prefix_length ( op )
	    << ","
	    << LEX::postfix_length ( op )
	    << ")";

    if ( op & MATCH )
        OUT << "MATCH("
	    << pID ( ihp->atom_table_ID, program )
	    << ")";

    if ( op & REQUIRE )
        OUT << "REQUIRE("
	    << pID ( ihp->require_dispatcher_ID,
	             program )
	    << ")";

    if ( op & ERRONEOUS_ATOM )
        OUT << "ERRONEOUS_ATOM("
	    << ihp->erroneous_atom_type << ")";

    if ( op & OUTPUT )
        OUT << "OUTPUT("
	    << ihp->output_type << ")";

    if ( op & GOTO )
        OUT << "GOTO("
	    << pID ( ihp->goto_table_ID, program )
	    << ")";

    if ( op & CALL )
        OUT << "CALL("
	    << pID ( ihp->call_table_ID, program )
	    << ")";

    if ( op & RETURN )
        OUT << "RETURN";

    if ( op & FAIL )
        OUT << "FAIL";

    if ( op & ELSE )
    {
	if (    no_line_number
	     || ihp->else_instruction_ID == 0 )
	{
	    OUT << "ELSE("
		<< pID ( ihp->else_instruction_ID,
		         program )
		<< ")";
	}
	else
	{
	    OUT << "ELSE:"
		<< min::set_indent ( indent )
		<< min::indent;
	    print_instruction
		( printer,
		  program, ihp->else_instruction_ID,
		  false );
	}
    }
    else if ( first ) printer << "ACCEPT";

    printer << min::eom;

#   undef OUT

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
	     < printer->line_break.indent )
	     printer << min::indent;
	else if (   printer->column
	          > printer->line_break.indent )
	    printer << " ";

	printer << min::set_break << pgraphic ( c1 );
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

// Print the dispatcher at program[ID] with indent
// equal to the current column in the cooked format.
// Return length of dispatcher component.  But if ID is
// zero, do nothing but return 0.
//
static uns32 print_cooked_dispatcher
    ( min::printer printer, LEX::program program,
      uns32 ID )
{
    if ( ID == 0 ) return 0;

    printer << min::bom
	    << min::nohbreak;

    printer << "DISPATCHER" << min::eol;

    min::ptr<dispatcher_header> dhp =
	LEX::ptr<dispatcher_header> ( program, ID );

    uns32 length = dispatcher_header_length;
    min::ptr<break_element> bep =
        LEX::ptr<break_element>
            ( program, ID + length );
    length += break_element_length
	    * dhp->max_break_elements;
    min::ptr<map_element> mep =
        LEX::ptr<map_element>
	    ( program, ID + length );
    length += map_element_length
	    * ( dhp->max_ctype + 1 );

    printer << min::indent << "Break Elements: "
	<< dhp->break_elements << min::eol;
    printer << min::indent << "Max Break Elements: "
	<< dhp->max_break_elements << min::eol;
    printer << min::indent << "Max CType: "
	<< dhp->max_ctype << min::eol;

    // Construct tmap so that t2 = tmap[t1] iff t2 is
    // the smallest ctype such that mep[t2] == mep[t1].
    //
    uns32 tmap[dhp->max_ctype+1];
    for ( uns32 t1 = 0; t1 <= dhp->max_ctype; ++ t1 )
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
    for ( uns32 t = 0; t <= dhp->max_ctype; ++ t )
    {
        if ( t != tmap[t] ) continue;
	if ( mep[t].instruction_ID == 0
	     &&
	     mep[t].dispatcher_ID == 0 )
	    continue;

	pclist pcl ( printer );
	for ( uns32 b = 0; b < dhp->break_elements;
	                   ++ b )
	{
	    uns32 cmin = bep[b].cmin;
	    uns32 cmax =
		( b == dhp->break_elements - 1 ?
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
		min::ptr<type_map_header> tmhp =
		    LEX::ptr<type_map_header>
			( program, type_map_ID );
		uns32 ctype = tmhp->singleton_ctype;
		if ( ctype != 0 )
		{
		    if ( tmap[ctype] == t )
			pcl.add ( cmin, cmax );
		}
		else
		{
		    min::ptr<uns8> p =
		        min::ptr<uns8> ( tmhp + 1 );
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

	if ( mep[t].instruction_ID != 0 )
	    print_instruction
		( printer << min::indent << "    ",
		  program, mep[t].instruction_ID );
	if ( mep[t].dispatcher_ID != 0 )
	    printer << min::indent
	            << "    Dispatcher ID: "
	            << pID ( mep[t].dispatcher_ID,
		             program )
		    << min::eol;
    }

    printer << min::restore_indent
            << min::restore_print_format;

    return length;
}

uns32 LEX::print_program_component
	( min::printer printer, LEX::program program,
	  uns32 ID, bool cooked )
{
    printer << min::save_print_format
            << min::nohbreak
	    << min::set_indent ( IDwidth );

    uns32 length;

    switch ( program[ID] )
    {
    case PROGRAM:
    {
	printer << pIDindent ( ID, program )
	        << "PROGRAM" << min::eol;
	min::ptr<program_header> php =
	    LEX::ptr<program_header> ( program, ID );
	for ( unsigned i = 0;
	      i < LEX::MAX_INITIAL_TABLES; ++ i )
	{
	    uns32 ID = php->initial_table_ID[i];
	    if ( ID == 0 ) continue;
	    printer << min::indent
	            << "Initial Table ID[" << i << "]:"
		    << pID ( ID, program )
		    << min::eol;
	}
	printer << min::indent << "Max Type: "
	    << php->max_type << min::eol;
	for ( uns32 t = 0; t <= php->max_type; ++ t )
	{
	    uns32 offset =
	        program[ID+program_header_length+t];
	    if ( offset == 0 ) continue;
	    printer << min::indent
	            << t << ": " << min::right ( 8 )
	            << min::ptr<char> ( php ) + offset
		    << min::eol;
	}
	length = php->component_length;
	break;
    }
    case TABLE:
    {
	printer << pIDindent ( ID, program )
	        << "TABLE" << min::eol;
	min::ptr<table_header> thp =
	    LEX::ptr<table_header> ( program, ID );
	printer << min::indent << "Mode: "
	        << pmode ( program, thp->mode )
		<< min::eol;
	printer << min::indent << "Dispatcher ID: "
	        << pID ( thp->dispatcher_ID, program )
		<< min::eol;
	if ( thp->instruction_ID == 0 )
	    /* do nothing */;
	else if ( cooked )
	    print_instruction
		( printer << min::indent, program,
		  thp->instruction_ID );
	else
	    printer << min::indent << "Instruction ID: "
		    << pID ( thp->instruction_ID,
		             program )
		    << min::eol;
	length = table_header_length;
	break;
    }
    case DISPATCHER:
    if ( cooked )
    {
	printer << pIDindent ( ID, program );
	length = print_cooked_dispatcher
		       ( printer, program, ID );
	break;
    }
    else
    {
	printer << pIDindent ( ID, program )
	        << "DISPATCHER" << min::eol;
	min::ptr<dispatcher_header> dhp =
	    LEX::ptr<dispatcher_header> ( program, ID );
	printer << min::indent << "Break Elements: "
	        << dhp->break_elements << min::eol;
	printer << min::indent << "Max Break Elements: "
	        << dhp->max_break_elements << min::eol;
	printer << min::indent << "Max CType: "
	        << dhp->max_ctype << min::eol;
	printer << min::indent << "Breaks: "
	        << min::set_break
	        << "cmin" << min::right ( 16 )
	        << "type_map_ID" << min::right ( 16 )
	        << min::eol;
	length = dispatcher_header_length;
	uns32 p, n;
	for ( p = ID + length, n = 0;
	      n < dhp->break_elements;
	      p += break_element_length, ++ n )
	{
	    min::ptr<break_element> bep =
		LEX::ptr<break_element> ( program, p );
	    printer << min::indent
		    << pgraphic ( bep->cmin )
		    << min::right ( 24 )
		    << pID ( bep->type_map_ID, program )
		    << min::right ( 16 )
		    << min::eol;
	}
	length += break_element_length
	        * dhp->max_break_elements;
	printer << min::indent << "Map:   CType: "
	        << min::set_break
	        << "dispatcher_ID" << min::right ( 16 )
	        << "instruction_ID" << min::right ( 16 )
	        << min::eol;
	uns32 t;
	for ( p = ID + length, t = 0;
	      t <= dhp->max_ctype;
	      p += map_element_length, ++ t )
	{
	    min::ptr<map_element> mep =
		LEX::ptr<map_element> ( program, p );
	    printer << min::indent
		    << t << ": " << min::right ( 14 )
		    << pID ( mep->dispatcher_ID,
		             program )
		    << min::right ( 16 )
		    << pID ( mep->instruction_ID,
		             program )
		    << min::right ( 16 )
		    << min::eol;
	}
	length += map_element_length
	        * ( dhp->max_ctype + 1 );
	break;
    }
    case INSTRUCTION:
    {
	printer << pIDindent ( ID, program );
	length = print_instruction
		   ( printer, program, ID, true );
	break;
    }
    case TYPE_MAP:
    {
	printer << pIDindent ( ID, program )
	        << "TYPE_MAP" << min::eol;
	min::ptr<type_map_header> tmhp =
	    LEX::ptr<type_map_header> ( program, ID );
	length = type_map_header_length;
	if ( tmhp->singleton_ctype > 0 )
	{
	    printer << tmhp->singleton_ctype
	            << min::right ( IDwidth + 4 )
		    << ": " << pgraphic ( tmhp->cmin )
	            << "-" << pgraphic ( tmhp->cmax )
		    << min::eol;
	}
	else
	{
	    printer << min::save_print_format
	            << min::set_indent ( IDwidth + 6 );

	    min::ptr<uns8> map =
	        min::ptr<uns8> ( tmhp + 1 );
	    length += ( tmhp->cmax - tmhp->cmin + 4 )
	            / 4;
	    for ( unsigned t = 0; t < 256; ++ t )
	    {
		pclist pcl ( printer );
		for ( uns32 c = tmhp->cmin;
		      c <= tmhp->cmax; ++ c )
		{
		    if ( map[c - tmhp->cmin] == t )
		    {
			if ( pcl.empty )
			    printer
				<< min::set_break
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

	    printer << min::restore_print_format
	            << min::set_indent ( IDwidth );
	}
	break;
    }
    default:
    {
	printer << pIDindent ( ID, program )
	        << "ILLEGAL COMPONENT TYPE("
	        << program[ID] << ")" << min::eol;
	length = program->length - ID;
	break;
    }
    }

    printer << min::restore_print_format;
    return length;
}

void LEX::print_program
	( min::printer printer, LEX::program program,
	  bool cooked )
{
    printer << min::bom;

    uns32 ID = 0;
    while ( ID < program->length )
    {
	// If cooked skip some components.
	//
        if ( cooked ) switch ( program[ID] )
	{
	case INSTRUCTION:
	{
	    min::ptr<instruction_header> ihp =
		LEX::ptr<instruction_header>
		    ( program, ID );
	    ID += instruction_header_length;
	    if (   ihp->operation
		 & LEX::TRANSLATE_TO_FLAG )
		ID += LEX::translate_to_length
			    ( ihp->operation );
	    continue;
	}
	case TYPE_MAP:
	{
	    min::ptr<type_map_header> tmhp =
		LEX::ptr<type_map_header>
		    ( program, ID );
	    ID += type_map_header_length;
	    if ( tmhp->singleton_ctype == 0 )
		ID += ( tmhp->cmax - tmhp->cmin + 4 )
		    / 4;
	    continue;
	}
	}

        ID += print_program_component
	    ( printer, program, ID, cooked );
    }

    if ( ID > program->length )
        printer << "  ILLEGALLY TRUNCATED LAST PROGRAM"
	           " COMPONENT" << min::eol;
    printer << min::eom;
}

// Name String Scanning
// ---- ------ --------

// Because we do not know the number of elements in the
// label being created by scan_name_string, we use a
// function that calls itself recursively to scan the
// next element.  This maintains a list of the elements
// scanned in the stack frames of the recursive calls,
// and a count of these elements in a function argument.
//
struct scan_name_string_var
{
    min::locatable_gen element;
    scan_name_string_var * previous;
};
static min::gen scan_name_string_make_label
	( ::scan_name_string_var * last,
	  min::uns32 count )
{
    // Given the elements in the stack and a count of
    // such, return a MIN label containing the elements,
    // except if count == 1 return just the one element.

    if ( count == 1 )
    {
        assert ( last->previous == NULL );
	return last->element;
    }

    min::gen elements[count];

    for ( int i = count - 1; i >= 0; -- i )
    {
        elements[i] = last->element;
	last = last->previous;
    }
    assert ( last == NULL );

    return min::new_lab_gen ( elements, count );
}
static min::gen scan_name_string_next_element
	( min::ref<ll::lexeme::scanner> scanner,
	  min::uns64 accepted_types,
	  min::uns64 ignored_types,
	  min::uns64 end_types,
	  ::scan_name_string_var * previous = NULL,
	  min::uns32 count = 0 )
{
    LEX::uns32 first, next, type;
    min::uns64 flag;

    // Get the next non-ignorable element, or call
    // scan_name_string_make_label, or announce error
    // and return min::ERROR().
    //
    while ( true )
    {
	uns64 erroneous_atom_count =
	    scanner->erroneous_atom->count;
	type = LEX::scan ( first, next, scanner );

	if ( type == LEX::SCAN_ERROR )
	{
	    scanner->printer
	        << "FATAL ERROR (I.E., SCAN ERROR) IN"
		   " THE CODE OF THE LEXICAL PROGRAM:"
		<< min::eol << min::error_message;
	    return min::ERROR();
	}

	if (   scanner->erroneous_atom->count
	     > erroneous_atom_count )
	    return min::ERROR();

	if ( type < 64 )
	{
	    flag = 1ull << type;
	    if ( flag & accepted_types ) break;
	    else if ( flag & ignored_types )
	    {
	        if ( flag & end_types )
		{
		    if ( count == 0 )
		    {
			scanner->printer
			    << "ERROR: empty name"
			       " string" << min::eol;
			return min::ERROR();
		    }
		    else
			return
			  ::scan_name_string_make_label
		    		( previous, count );
		}
		else
		    continue;
	    }
	}

	scanner->printer
	    << "ERROR: Lexeme of illegal type ("
	    << LEX::pmode ( scanner->program, type )
	    << ") in name string:"
	    << min::eol;
	LEX::print_phrase_lines
	    ( scanner->printer, scanner, first, next,
	      '^', NULL, NULL, NULL );
        return min::ERROR();
    }

    ++ count;
    scan_name_string_var var;
    var.previous = previous;
    var.element =
        min::new_str_gen
	    ( min::begin_ptr_of
	          ( scanner->translation_buffer ),
	      scanner->translation_buffer->length );

    if ( flag & end_types )
        return ::scan_name_string_make_label
	    ( & var, count );
    else
        return ::scan_name_string_next_element
	    ( scanner,
	      accepted_types,
	      ignored_types,
	      end_types,
	      & var, count );
}

min::gen LEX::scan_name_string
	( min::ref<ll::lexeme::scanner> scanner,
	  min::uns64 accepted_types,
	  min::uns64 ignored_types,
	  min::uns64 end_types )
{

   if ( scanner->erroneous_atom == NULL_STUB )
       LEX::init ( erroneous_atom_ref(scanner),
                   LEX::NO_LINE_NUMBERS );

    return ::scan_name_string_next_element
        ( scanner,
	  accepted_types, ignored_types, end_types );
}
