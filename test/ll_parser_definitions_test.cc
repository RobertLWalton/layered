// Layers Language Parser Definitions Test
//
// File:	ll_parser_definitions_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Feb 19 11:47:06 EST 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

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
    PAR::init_print_flags
        ( PAR::default_parser,
	    min::GRAPHIC_VSPACE_FLAG
	  + min::GRAPHIC_NSPACE_FLAG );
PAR::default_parser->trace |= PAR::TRACE_EXPLICIT_SUBEXPRESSIONS;
    PAR::parse();
}
