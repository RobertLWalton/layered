// Layered Languages Standard Print Test
//
// File:	ll_parser_standard_print_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Jul  6 16:47:36 EDT 2022
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
    PAR::init ( PAR::default_parser, PARSTD::ALL );

    PAR::init_input_stream
        ( PAR::default_parser, std::cin );
    PAR::init_ostream
        ( PAR::default_parser, std::cout );

    PAR::parse();

    min::print_id_map ( PAR::default_parser->printer );
}

