// Layers Language Parser Command Test
//
// File:	ll_parser_command_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Sep 15 07:20:03 EDT 2012
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

int main ( int argc, const char * argv[] )
{
    min::initialize();
    PARSTD::init_input ( PAR::default_parser );
    PARSTD::init_brackets ( PAR::default_parser );
    PAR::init_input_stream
        ( PAR::default_parser, std::cin );
    PAR::init_ostream
        ( PAR::default_parser, std::cout );
    PAR::init_print_flags
        ( PAR::default_parser,
	    min::GRAPHIC_VSPACE_FLAG
	  + min::GRAPHIC_NSPACE_FLAG );
    PAR::default_parser->trace_flags |=
        PAR::default_parser->trace_commands;
    PAR::parse();
}
