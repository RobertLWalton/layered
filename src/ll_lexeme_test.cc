// Layers Language Lexical Analyzer Test Functions
//
// File:	ll_lexeme_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jan  1 08:26:11 EST 2011
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

static void basic_erroneous_atom
	( uns32 first, uns32 last, uns32 type,
	  LEX::scanner_ptr scanner )
{
    char buffer[1000];
    uns32 column =
        sprintf ( buffer, "ERRONEOUS ATOM:" );
    LEX::sperroneous_atom
        ( buffer + column, first, last, type, column,
	    LEX::ENFORCE_LINE_LENGTH
	  + LEX::PREFACE_WITH_SPACE,
	  scanner );
    cout << buffer << endl;
}

void LEX::basic_test_input
	( std::istream & in,
	  const char * file_name,
	  uns32 end_of_file_t )
{
    init_scanner();
    default_scanner->erroneous_atom =
        ::basic_erroneous_atom;
    init_stream ( default_scanner->input_file,
                  in, file_name, 0 );

    char buffer[10000];
    while ( true )
    {
	uns32 first, last;
        uns32 type = LEX::scan ( first, last );
	if ( type == LEX::SCAN_ERROR )
	{
	    cout << "Scan Error" << endl
	         << LEX::default_scanner->error_message
		 << endl;
	    return;
	}
	else
	{
	    uns32 column = 0;
	    LEX::splexeme
	        ( buffer, first, last, type, column );
	    cout << buffer << endl;
	}
	if ( type == end_of_file_t ) break;
    }
}

// Input Test
// ----- ----

min::static_stub<2> input_vec;
static min::packed_vec_insptr<char>
    & lexeme_codes =
        * (min::packed_vec_insptr<char> *) input_vec[0];
static min::packed_vec_insptr<char>
    & erroneous_atom_codes =
        * (min::packed_vec_insptr<char> *) input_vec[1];

static min::packed_vec<char> codes_type
    ( "ll::lexeme::codes_type" );

static const char * type_code;
    // Save of type_code argument for general use.

static LEX::uns32 next_line;
    // Line number of next line to be printed.
    // 0 if no lines printed yet.
static LEX::uns32 line_width;
    // Line width of last line printed.

// Set next_line = line + 1.
//
static void set_line ( uns32 line )
{
    assert ( ::next_line <= line );
    if ( ::next_line == 0 )
    {
	if ( lexeme_codes == min::NULL_STUB )
	{
	    lexeme_codes = codes_type.new_stub();
	    erroneous_atom_codes =
	        codes_type.new_stub();
	}
    }
    else
    {
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
	    cout << & lexeme_codes[0] << endl;
	    min::pop ( lexeme_codes,
	               lexeme_codes->length );
	}
    }

    LEX::scanner_ptr scanner = LEX::default_scanner;
    while ( ::next_line <= line )
    {
        uns32 offset =
	    LEX::next_line ( scanner->input_file );
	const char * p =
	    & scanner->input_file->data[offset];
	char buffer [10 + 4 * strlen ( p )];
	::line_width = 0;
	uns32 boffset =
	    LEX::spstring ( buffer, p,
	                    ::line_width, 0, scanner );
	LEX::spstring ( buffer + boffset, "\n",
	                ::line_width, 0, scanner );
	++ ::next_line;
    }

    assert ( next_line == line + 1 );
}

static void set_codes
	( min::packed_vec_insptr<char> codes,
	  uns32 first, uns32 last, uns32 type )
{
    LEX::scanner_ptr scanner = LEX::default_scanner;

    uns32 begin_column =
        scanner->input_buffer[first].column;
    uns32 end_column =
        last + 1 < scanner->input_buffer->length ?
        scanner->input_buffer[last+1].column :
	scanner->next_position.column;
    uns32 line = scanner->input_buffer[first].line;
    if ( line <= ::next_line ) set_line ( line );

    if ( end_column <= begin_column )
        end_column = ::line_width;

    while ( codes->length < end_column )
        min::push(codes) = ' ';
    for ( uns32 i = begin_column; i < end_column; ++ i )
        codes[i] = ::type_code[type];
}

static void erroneous_atom
	( uns32 first, uns32 last, uns32 type,
	  LEX::scanner_ptr scanner )
{
    assert ( scanner == LEX::default_scanner );
    set_codes ( ::erroneous_atom_codes,
                first, last, type );
}

void LEX::test_input
	( const char * type_code,
	  std::istream & in,
	  const char * file_name,
	  uns32 end_of_file_t )
{
    ::type_code = type_code;

    init_scanner();
    scanner_ptr scanner = LEX::default_scanner;

    scanner->erroneous_atom = ::erroneous_atom;
    init_stream ( scanner->input_file,
                  in, file_name, 0 );

    while ( true )
    {
	uns32 first, last;
        uns32 type = LEX::scan ( first, last );
	if ( type == LEX::SCAN_ERROR )
	{
	    cout << "Scan Error" << endl
	         << scanner->error_message
		 << endl;
	    return;
	}
	else
	    set_codes ( lexeme_codes,
	                first, last, type );

	if ( type == end_of_file_t ) break;
    }
}
