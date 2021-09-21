// Layered Languages Standard Print Test
//
// File:	ll_parser_standard_print_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Sep 21 01:56:26 EDT 2021
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include <ll_parser.h>
# include <iostream>
# include <cassert>
# define PAR ll::parser
using std::cout;

int main ( int argc, const char * argv[] )
{
    min::initialize();
    PAR::init ( PAR::default_parser, true );

    PAR::init_input_stream
        ( PAR::default_parser, std::cin );
    PAR::init_ostream
        ( PAR::default_parser, std::cout );

    PAR::parse();

    min::print_id_map ( PAR::default_parser->printer );
}

