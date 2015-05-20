// Layered Languages Parser Operator Standard Test
//
// File:	ll_parser_oper_standard_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Jan 16 07:07:20 EST 2015
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
# define TAB ll::parser::table
# define BRA ll::parser::bracketed
# define OP ll::parser::oper
# define PARSTD ll::parser::standard
using std::cout;

int main ( int argc, const char * argv[] )
{
    bool debug = ( argc > 1 );

    min::initialize();
    PAR::init ( PAR::default_parser, false );
    PARSTD::init_input ( PAR::default_parser );
    PARSTD::init_brackets ( PAR::default_parser );
    BRA::bracketed_pass bracketed_pass =
	(BRA::bracketed_pass)
	PAR::default_parser->pass_stack;
    OP::oper_pass oper_pass =
        PARSTD::init_oper ( PAR::default_parser );
    PAR::init_input_stream
        ( PAR::default_parser, std::cin );
    PAR::init_ostream
        ( PAR::default_parser, std::cout );
    PAR::init_line_display
        ( PAR::default_parser,
	    min::DISPLAY_PICTURE );
    PAR::default_parser->trace_flags =
    	  PAR::TRACE_SUBEXPRESSION_ELEMENTS
    	+ PAR::TRACE_SUBEXPRESSION_LINES;
    if ( debug )
	PAR::default_parser->trace_flags =
	    bracketed_pass->trace_subexpressions
	    +
	    oper_pass->trace_subexpressions;
	    
    PAR::parse();
}
