// Layered Languages Lexical Analyzer Test Functions
//
// File:	ll_lexeme_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed May 24 07:19:15 EDT 2023
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Basic Input Test
//	Input Test

// Usage and Setup
// ----- --- -----

# include <ll_lexeme.h>
# include <ll_lexeme_program_data.h>
# include <ll_lexeme_test.h>
# include <iostream>
# define LEX ll::lexeme
# define LEXDATA ll::lexeme::program_data
using std::cout;
using std::endl;
using LEX::uns32;

// Basic Input Test
// ----- ----- ----

void LEX::basic_test_input
	( uns32 end_of_file_t, uns32 indent_t )
{
    LEX::scanner scanner = LEX::default_scanner;
    min::printer printer = scanner->printer;
    LEX::translation_buffer translation_buffer =
        scanner->translation_buffer;

    LEX::init ( erroneous_atom_ref(scanner),
                LEX::BASIC );

    uns32 lexical_master = LEX::MISSING_MASTER;
    min::locatable_gen missing_master
        ( min::new_str_gen ( "MISSING_MASTER" ) );
    while ( true )
    {
	uns32 first, next;
        uns32 type = LEX::scan ( first, next );

	if ( type == LEX::SCAN_ERROR )
	{
	    printer << "Scan Error" << min::eol
	            << min::error_message;
	    return;
	}

	printer
	    << LEX::plexeme
		( LEX::default_scanner,
		  first, next, type )
	    << min::eol;

	if ( type == end_of_file_t ) break;

	if (    type == indent_t
	     && lexical_master != MISSING_MASTER )
	    LEX::set_lexical_master
	        ( lexical_master, scanner );

	uns32 length = translation_buffer->length;
	if ( indent_t != 0
	     &&
	     length > 2
	     &&
	     translation_buffer[0] == '<'
	     &&
	     translation_buffer[length-1] == '>' )
	{
	    min::locatable_gen master_name
	        ( min::new_str_gen
		    ( min::begin_ptr_of
		      ( translation_buffer ) + 1,
		      length - 2 ) );
	    if ( master_name == missing_master )
	        lexical_master = LEX::MISSING_MASTER;
	    else
	    {
	        uns32 master_index =
		    LEX::lexical_master_index
		        ( master_name, scanner );
		if (    master_index
		     != LEX::MISSING_MASTER )
		    lexical_master = master_index;
	    }
	}
    }
}

// Input Test
// ----- ----

static min::locatable_var<min::packed_vec_insptr<char> >
       lexeme_codes;

static min::locatable_var<min::packed_vec_insptr<char> >
       erroneous_atom_codes;

static min::packed_vec<char> codes_type
    ( "ll::lexeme::codes_type" );

static min::locatable_var < min::ptr<const char> >
	type_codes;
    // Save of type_codes from lexical program for
    // general use.

static min::uns32 max_type;
    // Save of max_type from lexical program for
    // general use.

static LEX::uns32 next_line;
    // Line number of next line to be printed.
    // 0 if no lines printed yet.
static LEX::uns32 line_width;
    // Line width of last line printed.

// Flush the lexeme_codes and erroneous_atom_codes.
//
static void flush_codes ( void )
{
    LEX::scanner scanner = LEX::default_scanner;
    min::printer printer = scanner->printer;

    for ( uns32 i = 0;
	  i < erroneous_atom_codes->length; ++ i )
    {
	char c = erroneous_atom_codes[i];
	if ( c == ' ' ) continue;
	while ( i >= lexeme_codes->length )
	    min::push(lexeme_codes) = ' ';
	lexeme_codes[i] = c;
    }
    min::pop ( erroneous_atom_codes,
	       erroneous_atom_codes->length );
    if ( lexeme_codes->length > 0 )
    {
	min::push(lexeme_codes) = 0;
	printer << min::bom << min::no_auto_break
		<< & lexeme_codes[0]
		<< min::eom;
	min::pop ( lexeme_codes,
		   lexeme_codes->length );
    }
}

// Set ::next_line == line + 1.
//
void set_line ( LEX::uns32 line )
{
    LEX::scanner scanner = LEX::default_scanner;
    min::printer printer = scanner->printer;

    MIN_REQUIRE ( ::next_line <= line );

    flush_codes();

    for ( ; ::next_line <= line; ++ ::next_line )
        ::line_width =
	    min::print_line
	        ( printer, scanner->input_file,
		  ::next_line );
}

static void set_codes
	( min::packed_vec_insptr<char> codes,
	  uns32 first, uns32 next, uns32 type )
{
    LEX::scanner scanner = LEX::default_scanner;

    min::printer printer = scanner->printer;
    const min::print_format & print_format =
        printer->print_format;

    min::position begin_pos =
        first < scanner->input_buffer->length ?
        (min::position) scanner->input_buffer[first] :
	scanner->next_position;
    min::position end_pos =
        next < scanner->input_buffer->length ?
        (min::position) scanner->input_buffer[next] :
	scanner->next_position;

    do
    {
	if ( ::next_line <= begin_pos.line )
	    set_line ( begin_pos.line );

	uns32 begin_column =
	    min::print_line_column
		( scanner->input_file,
		  begin_pos,
		  print_format );
	uns32 end_column =
	    begin_pos.line < end_pos.line ?
	    ::line_width :
	    min::print_line_column
		( scanner->input_file,
		  end_pos,
		  print_format );

	// Handle zero length lexeme.  This may be
	// overwritten by following lexeme, but will
	// not be if at end of line.
	//
	if ( end_column == begin_column )
	    end_column = begin_column + 1;

	while ( codes->length < end_column )
	    min::push(codes) = ' ';
	for ( uns32 i = begin_column; i < end_column;
	                              ++ i )
	    codes[i] = ( type <= ::max_type ?
			 ::type_codes[type] : ' ' );

	if ( begin_pos.line == end_pos.line )
	    break;
	
	++ begin_pos.line;
	begin_pos.offset = 0;

    } while ( begin_pos < end_pos );
}

static min::locatable_var<LEX::erroneous_atom>
       test_erroneous_atom;

static void erroneous_atom_announce
	( uns32 first, uns32 next, uns32 type,
	  LEX::scanner scanner,
	  LEX::erroneous_atom erroneous_atom )
{
    MIN_REQUIRE ( scanner == LEX::default_scanner );
    set_codes ( ::erroneous_atom_codes,
                first, next, type );
}

void LEX::test_input ( uns32 end_of_file_t )
{
    LEX::scanner scanner = LEX::default_scanner;
    min::printer printer = scanner->printer;

    LEX::init
	( ::test_erroneous_atom,
          ::erroneous_atom_announce );
    erroneous_atom_ref(scanner) = ::test_erroneous_atom;

    
    ::type_codes =
        LEXDATA::type_codes ( scanner->program );
    ::max_type =
        LEXDATA::max_type ( scanner->program );

    ::next_line = 0;

    if ( lexeme_codes == min::NULL_STUB )
    {
	lexeme_codes = codes_type.new_stub();
	erroneous_atom_codes =
	    codes_type.new_stub();
    }
    else
    {
	min::pop ( lexeme_codes,
		   lexeme_codes->length );
	min::pop ( erroneous_atom_codes,
		   erroneous_atom_codes->length );
    }

    while ( true )
    {
	uns32 first, last;
        uns32 type = LEX::scan ( first, last );
	if ( type == LEX::SCAN_ERROR )
	{
	    printer << "Scan Error" << min::eol
	            << min::error_message
		    << min::eol;
	    return;
	}
	else
	    set_codes ( lexeme_codes,
	                first, last, type );

	if ( type == end_of_file_t ) break;
    }

    flush_codes();
}
