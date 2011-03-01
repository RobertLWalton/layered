// Layers Language Lexical Analyzer Test Functions
//
// File:	ll_lexeme_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Feb 28 18:38:51 EST 2011
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
# include <ll_lexeme_test.h>
# include <iostream>
# include <cassert>
# define LEX ll::lexeme
using std::cout;
using std::endl;
using LEX::uns32;

// Basic Input Test
// ----- ----- ----

void LEX::basic_test_input ( uns32 end_of_file_t )
{
    LEX::scanner scanner = LEX::default_scanner;
    min::printer printer = scanner->printer;

    scanner->erroneous_atom =
        LEX::default_erroneous_atom;

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
	else
	    printer
	        << LEX::plexeme
	            ( LEX::default_scanner,
		      first, next, type )
		<< min::eol;

	if ( type == end_of_file_t ) break;
    }
}

// Input Test
// ----- ----

static min::locatable_ptr<min::packed_vec_insptr<char> >
       lexeme_codes;

static min::locatable_ptr<min::packed_vec_insptr<char> >
       erroneous_atom_codes;

static min::packed_vec<char> codes_type
    ( "ll::lexeme::codes_type" );

static const char * type_code;
    // Save of type_code argument for general use.

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
	printer << min::bom << min::nohbreak
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

    assert ( ::next_line <= line );

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

    uns32 line =
        first < scanner->input_buffer->length ?
        scanner->input_buffer[first].line :
	scanner->next_position.line;
    if ( ::next_line <= line ) set_line ( line );

    uns32 begin_column =
        first < scanner->input_buffer->length ?
        scanner->input_buffer[first].column :
	scanner->next_position.column;
    uns32 end_column =
        next < scanner->input_buffer->length ?
        scanner->input_buffer[next].column :
	scanner->next_position.column;

    if ( end_column <= begin_column )
	    end_column = ::line_width;
    if ( end_column <= begin_column )
	    end_column = begin_column + 1;

    while ( codes->length < end_column )
        min::push(codes) = ' ';
    for ( uns32 i = begin_column; i < end_column; ++ i )
        codes[i] = ::type_code[type];
}

static min::locatable_ptr<LEX::erroneous_atom>
       test_erroneous_atom;

static void erroneous_atom_announce
	( uns32 first, uns32 next, uns32 type,
	  LEX::scanner scanner,
	  LEX::erroneous_atom erroneous_atom )
{
    assert ( scanner == LEX::default_scanner );
    set_codes ( ::erroneous_atom_codes,
                first, next, type );
}

void LEX::test_input
	( const char * type_code,
	  uns32 end_of_file_t )
{
    LEX::scanner scanner = LEX::default_scanner;
    min::printer printer = scanner->printer;

    LEX::init
	( ::test_erroneous_atom,
          ::erroneous_atom_announce );
    scanner->erroneous_atom = ::test_erroneous_atom;

    ::type_code = type_code;
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
