// Layers Language Input Pass Test
//
// File:	ll_input_pass_test.cc
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
# include <ll_lexeme_standard.h>
# include <ll_parser_pass.h>
# include <ll_parser_standard.h>
# include <iostream>
# include <cassert>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define PARSTD ll::parser::standard
using std::cout;

int main ( int argc )
{
    LEXSTD::create_standard_program();
    LEX::init_scanner();
    PAR::pass_ptr input_pass =
        PARSTD::create_input_pass();
    PAR::pass_ptr output_pass =
        PAR::create_output_pass ( input_pass );
    input_pass->trace = & cout;
    while ( get ( output_pass ) );
}
