// Layered Languages Standard Lexical Analyzer Test
//
// File:	ll_lexeme_standard_basic_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed May 24 07:09:06 EDT 2023
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// ll_lexeme_standard_basic_test [-t|-p]
//
//     -t turns on LEX::TRACE
//     -p prints program

# include <ll_lexeme.h>
# include <ll_lexeme_test.h>
# include <ll_lexeme_standard.h>
# include <iostream>
# include <cassert>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard

bool trace = false;
bool print_program = false;
int main ( int argc, const char * argv[] )
{
    while ( argc > 1 )
    {
        if ( strcmp ( argv[1], "-t" ) == 0 )
	    trace = true;
        else if ( strcmp ( argv[1], "-p" ) == 0 )
	    print_program = true;
	else break;
	-- argc, ++ argv;
    }
    min::initialize();
    LEX::init_ostream
	    ( LEX::default_scanner, std::cout )
        << min::ascii;
    LEXSTD::init_standard_program();
    if ( print_program )
	LEX::print_program
	    ( LEX::default_scanner->printer,
	      LEXSTD::default_program, true );
    LEX::init_program ( LEX::default_scanner,
    			LEXSTD::default_program );
    LEX::init_input_stream
        ( LEX::default_scanner,
	  std::cin );
    if ( trace )
        LEX::default_scanner->trace = LEX::TRACE;
    LEX::default_scanner->printer
        << min::display_picture << min::latin1;
    LEX::basic_test_input
        ( LEXSTD::end_of_file_t,
	  LEXSTD::indent_t );
}
