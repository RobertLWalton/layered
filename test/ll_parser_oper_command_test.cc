// Layers Language Parser Operator Command Test
//
// File:	ll_parser_oper_command_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Dec 27 05:51:40 EST 2013
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include <ll_parser.h>
# include <ll_parser_bracketed.h>
# include <ll_parser_oper.h>
# include <ll_parser_standard.h>
# include <iostream>
# include <cassert>
# define PAR ll::parser
# define BRA ll::parser::bracketed
# define OP ll::parser::oper
# define STD ll::parser::standard
using std::cout;

int main ( int argc, const char * argv[] )
{
    min::initialize();

    PAR::init ( PAR::default_parser, false );
    PAR::pass oper_pass = OP::new_pass();
    PAR::place_at_end
        ( PAR::default_parser, oper_pass );
    STD::init_block ( PAR::default_parser );
    STD::init_brackets ( PAR::default_parser );

    STD::init_input ( PAR::default_parser );
    PAR::init_input_stream
        ( PAR::default_parser, std::cin );
    PAR::init_ostream
        ( PAR::default_parser, std::cout );
    PAR::init_print_flags
        ( PAR::default_parser,
	    min::EOL_FLUSH_FLAG
	  + min::GRAPHIC_VSPACE_FLAG
	  + min::GRAPHIC_NSPACE_FLAG );
    PAR::parse();
}
