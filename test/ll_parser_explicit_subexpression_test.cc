// Layers Language Parser Explicit Subexpression Test
//
// File:	ll_parser_explicit_subexpression_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Apr  8 02:53:30 EDT 2011
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
# include <ll_parser_input.h>
# include <iostream>
# include <cassert>
# define PAR ll::parser
using std::cout;

int main ( int argc )
{
    PAR::init_standard_input ( PAR::default_parser );
    PAR::init_input_stream
        ( PAR::default_parser, std::cin );
    PAR::init_output_stream
        ( PAR::default_parser, std::cout );
    PAR::default_parser->trace =
        PAR::TRACE_EXPLICIT_SUBEXPRESSIONS
	+ PAR::TRACE_INPUT;
    PAR::init_print_flags
        ( PAR::default_parser,
	    min::GRAPHIC_VSPACE_FLAG
	  + min::GRAPHIC_NSPACE_FLAG );
    PAR::parse();
}
