// Layered Languages Standard Lexical Analyzer Test
//
// File:	ll_lexeme_standard_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun May 24 11:58:11 EDT 2015
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

int main ( int argc, const char * argv[] )
{
    min::initialize();
    LEX::init_ostream
	    ( LEX::default_scanner, std::cout )
        << min::ascii;
    LEXSTD::init_standard_program();
    LEX::init_program ( LEX::default_scanner,
                        LEXSTD::default_program );
    LEX::init_input_stream
        ( LEX::default_scanner,
	  std::cin,
	    min::DISPLAY_PICTURE
	  + min::DISPLAY_NON_GRAPHIC
	  + min::DISPLAY_EOL );
    LEX::test_input ( LEXSTD::end_of_file_t );
}
