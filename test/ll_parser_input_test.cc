// Layered Languages Parser Input Test
//
// File:	ll_parser_input_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Nov 11 04:34:33 EST 2014
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
    PAR::init ( PAR::default_parser, false );
    PARSTD::init_input ( PAR::default_parser );
    PAR::init_input_stream
        ( PAR::default_parser, std::cin );
    PAR::init_ostream
        ( PAR::default_parser, std::cout );
    PAR::default_parser->printer
        << min::display_picture;
    PAR::default_parser->trace_flags =
        PAR::TRACE_PARSER_INPUT;
    PAR::init_line_display
        ( PAR::default_parser,
	  min::DISPLAY_PICTURE );
    PAR::parse();
}
