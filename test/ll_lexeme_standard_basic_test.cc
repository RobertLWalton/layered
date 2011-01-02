// Layers Language Standard Lexical Analyzer Test
//
// File:	ll_lexeme_standard_basic_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Jan  2 06:16:43 EST 2011
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
# include <ll_lexeme_test.h>
# include <ll_lexeme_standard.h>
# include <iostream>
# include <cassert>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
using std::cout;

int main ( int argc )
{
    LEXSTD::create_standard_program();
    LEX::print_program ( cout, true );
    LEX::basic_test_input ( LEXSTD::end_of_file_t );
}
