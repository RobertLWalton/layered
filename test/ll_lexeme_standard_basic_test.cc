// Layers Language Standard Lexical Analyzer Test
//
// File:	ll_lexeme_standard_basic_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon May 21 17:47:21 EDT 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include <ll_lexeme.h>
# include <ll_lexeme_test.h>
# include <ll_lexeme_standard.h>
# include <iostream>
# include <cassert>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard

int main ( int argc )
{
    min::initialize();
    LEX::init_ostream
	    ( LEX::default_scanner, std::cout )
        << min::ascii;
    LEXSTD::init_standard_program();
    LEX::print_program
        ( LEX::default_scanner->printer,
	  LEXSTD::default_program, true );
    LEX::init_program ( LEX::default_scanner,
    			LEXSTD::default_program );
    LEX::init_input_stream
        ( LEX::default_scanner,
	  std::cin,
	  min::GRAPHIC_FLAGS );
    LEX::default_scanner->printer << min::noascii;
    if ( argc > 1 )
        LEX::default_scanner->trace = LEX::TRACE;
    LEX::basic_test_input ( LEXSTD::end_of_file_t );
}
