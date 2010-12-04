// Layers Language Standard Lexical Analyzer Test
//
// File:	ll_lexeme_standard_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Dec  3 23:21:49 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	External Runtime
//	Program Construction Test

// Usage and Setup
// ----- --- -----

# include <ll_lexeme.h>
# include <ll_lexeme_standard.h>
# include <iostream>
# include <iomanip>
# include <cstdlib>
# include <cstring>
# include <cassert>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
using std::cout;
using std::endl;
using LEX::uns32;

static void erroneous_atom
	( uns32 first, uns32 last, uns32 type,
	  LEX::scanner_ptr scanner )
{
    char buffer[2000];
    uns32 column =
        sprintf ( buffer, "ERRONEOUS ATOM: " );
    LEX::sperroneous_atom
        ( buffer + column, first, last, type, column,
	  false, scanner );
    cout << buffer << endl;
}

int main ( int argc )
{
    LEXSTD::create_standard_program();
    LEX::print_program ( cout, true );
    LEX::init_scanner();
    LEX::default_scanner->erroneous_atom =
        ::erroneous_atom;
    char buffer[2000];
    while ( true )
    {
	uns32 first, last;
        uns32 type = LEX::scan ( first, last );
	if ( type == LEX::SCAN_ERROR )
	{
	    cout << "Scan Error" << endl
	         << LEX::default_scanner->error_message
		 << endl;
	}
	else
	{
	    uns32 column = 0;
	    LEX::splexeme ( buffer, first, last, type,
	                    column );
	    cout << buffer << endl;
	}
	if ( type == LEXSTD::end_of_file_t ) break;
    }
}
