// Layers Language Parser Operator Command Test
//
// File:	ll_parser_oper_command_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Nov 20 01:29:32 EST 2012
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
# define PARSTD ll::parser::standard
using std::cout;

int main ( int argc, const char * argv[] )
{
    bool debug = ( argc > 1 );

    min::initialize();
    PARSTD::init_input ( PAR::default_parser );
    PARSTD::init_brackets ( PAR::default_parser );
    BRA::bracketed_pass bracketed_pass =
	(BRA::bracketed_pass)
	PAR::default_parser->pass_stack;
    OP::oper_pass oper_pass =
        OP::place ( PAR::default_parser );
    PAR::init_input_stream
        ( PAR::default_parser, std::cin );
    PAR::init_ostream
        ( PAR::default_parser, std::cout );
    PAR::init_print_flags
        ( PAR::default_parser,
	    min::EOL_FLUSH_FLAG
	  + min::GRAPHIC_VSPACE_FLAG
	  + min::GRAPHIC_NSPACE_FLAG );
    if ( debug )
	PAR::default_parser->trace_flags =
	    bracketed_pass->trace_subexpressions
	    +
	    oper_pass->trace_subexpressions;
	    
    PAR::parse();
}
