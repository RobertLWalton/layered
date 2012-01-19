// Layers Language Parser Explicit Subexpression Test
//
// File:	ll_parser_explicit_subexpression_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Jan 19 03:32:24 EST 2012
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

# include <ll_parser.h>
# include <ll_parser_standard.h>
# include <iostream>
# include <cassert>
# define PAR ll::parser
# define PARSTD ll::parser::standard
using std::cout;

int main ( int argc )
{
    min::initialize();
    PARSTD::init_input ( PAR::default_parser );
    PARSTD::init_brackets ( PAR::default_parser );
    PAR::init_input_stream
        ( PAR::default_parser, std::cin );
    PAR::init_output_stream
        ( PAR::default_parser, std::cout );
    PAR::default_parser->trace =
        PAR::TRACE_EXPLICIT_SUBEXPRESSIONS;
    PAR::init_print_flags
        ( PAR::default_parser,
	    min::GRAPHIC_VSPACE_FLAG
	  + min::GRAPHIC_NSPACE_FLAG );
    PAR::parse();
}
