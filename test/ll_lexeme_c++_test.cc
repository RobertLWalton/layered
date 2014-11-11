// Layers Language C++ Lexical Analyzer Test
//
// File:	ll_lexeme_c++_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Nov 11 02:39:48 EST 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include <ll_lexeme.h>
# include <ll_lexeme_test.h>
# include <ll_lexeme_c++.h>
# include <iostream>
# include <cassert>
# define LEX ll::lexeme
# define LEXCPP ll::lexeme::cpp

int main ( int argc, const char * argv[] )
{
    min::initialize();
    LEX::init_ostream
	    ( LEX::default_scanner, std::cout )
        << min::ascii;
    LEXCPP::init_cpp_program();
    LEX::init_program ( LEX::default_scanner,
                        LEXCPP::default_program );
    LEX::init_input_stream
        ( LEX::default_scanner,
	  std::cin,
	    min::DISPLAY_PICTURE
	  + min::DISPLAY_NON_GRAPHIC
	  + min::DISPLAY_EOL );
    LEX::test_input ( LEXCPP::type_code,
                      LEXCPP::end_of_file_t );
}
