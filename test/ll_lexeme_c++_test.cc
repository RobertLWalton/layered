// Layers Language C++ Lexical Analyzer Test
//
// File:	ll_lexeme_c++_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Jan 19 03:31:14 EST 2012
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
# include <ll_lexeme_c++.h>
# include <iostream>
# include <cassert>
# define LEX ll::lexeme
# define LEXCPP ll::lexeme::cpp

int main ( int argc )
{
    min::initialize();
    LEX::init_output_stream
	    ( LEX::default_scanner, std::cout )
        << min::ascii;
    LEXCPP::init_cpp_program();
    LEX::init_program ( LEX::default_scanner,
                        LEXCPP::default_program );
    LEX::init_input_stream
        ( LEX::default_scanner,
	  std::cin,
	    min::GRAPHIC_FLAGS
	  + min::DISPLAY_EOL_FLAG );
    LEX::test_input ( LEXCPP::type_code,
                      LEXCPP::end_of_file_t );
}
