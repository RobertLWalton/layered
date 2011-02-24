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

static min::locatable_ptr<min::printer> printer;

int main ( int argc )
{
    min::init_output_stream ( printer, std::cout );
    LEXSTD::create_standard_program();
    LEX::print_program
        ( printer, LEX::default_program, true );
    LEX::init_program ( LEX::default_scanner,
    			LEX::default_program );
    LEX::init_printer ( LEX::default_scanner,
    			printer );
    LEX::init_input_stream ( LEX::default_scanner,
                             std::cin );
    LEX::basic_test_input ( LEXSTD::end_of_file_t );
}
