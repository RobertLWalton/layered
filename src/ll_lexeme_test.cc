// Layers Language Lexical Analyzer Test Functions
//
// File:	ll_lexeme_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Dec 30 07:46:43 EST 2010
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

static void erroneous_atom
	( uns32 first, uns32 last, uns32 type,
	  LEX::scanner_ptr scanner )
{
}

void LEX::test_input
	( const char * type_code,
	  std::istream & in,
	  const char * file_name,
	  uns32 end_of_file_t )
{
    init_scanner();
    default_scanner->erroneous_atom =
        ::erroneous_atom;
    init_stream ( default_scanner->input_file,
                  in, file_name, 0 );

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
	}
	if ( type == end_of_file_t ) break;
    }
}
