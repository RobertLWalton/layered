// Layered Languages Alternate Lexical Analyzer Basic
// Test
//
// File:	ll_lexeme_alternate_basic_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed May 24 07:10:16 EDT 2023
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// This file was copied from ll_lexeme_standard_basic_
// test.cc on Tue May 19 15:44:38 EDT 2015.  See the
// earlier history of that file for the earlier history
// of this file.

# include <ll_lexeme.h>
# include <ll_lexeme_test.h>
# include <ll_lexeme_alternate.h>
# include <iostream>
# include <cassert>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::alternate

int main ( int argc, const char * argv[] )
{
    min::initialize();
    LEX::init_ostream
	    ( LEX::default_scanner, std::cout )
        << min::ascii;
    LEXSTD::init_alternate_program();
    LEX::print_program
        ( LEX::default_scanner->printer,
	  LEXSTD::default_program, true );
    LEX::init_program ( LEX::default_scanner,
    			LEXSTD::default_program );
    LEX::init_input_stream
        ( LEX::default_scanner,
	  std::cin );
    if ( argc > 1 )
        LEX::default_scanner->trace = LEX::TRACE;
    LEX::default_scanner->printer
        << min::display_picture << min::latin1;
    LEX::basic_test_input ( LEXSTD::end_of_file_t );
}
