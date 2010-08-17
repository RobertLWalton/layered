// Layers Language Standard Lexical Analyzer Test
//
// File:	ll_lexeme_standard_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Aug 16 23:48:19 EDT 2010
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

int main ( int argc )
{
    LEXSTD::create_standard_program();
    LEX::print_program ( cout, true );
    LEX::init_scan();
    char buffer[2000];
    while ( true )
    {
	uns32 first, last;
        uns32 type = LEX::scan ( first, last );
	if ( type == LEX::SCAN_ERROR )
	{
	    cout << "Scan Error" << endl
	         << LEX::error_message << endl;
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
