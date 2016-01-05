// Layered Languages Parser Prefix Standard Test
//
// File:	ll_parser_prefix_standard_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jan  4 18:50:43 EST 2016
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include <ll_parser.h>
# include <ll_parser_bracketed.h>
# include <ll_parser_prefix.h>
# include <ll_parser_standard.h>
# include <iostream>
# include <cassert>
# define PAR ll::parser
# define TAB ll::parser::table
# define BRA ll::parser::bracketed
# define PRE ll::parser::prefix
# define PARSTD ll::parser::standard
using std::cout;

int main ( int argc, const char * argv[] )
{
    min::initialize();
    PAR::init ( PAR::default_parser, false );
    PARSTD::init_input ( PAR::default_parser );
    PARSTD::init_block ( PAR::default_parser );
    PARSTD::init_brackets ( PAR::default_parser );
    PAR::init_input_stream
        ( PAR::default_parser, std::cin );
    PAR::init_ostream
        ( PAR::default_parser, std::cout );
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass)
	PAR::default_parser->pass_stack;
    min::locatable_gen prefix
        ( min::new_str_gen ( "prefix" ) );
    PRE::prefix_pass prefix_pass =
	(PRE::prefix_pass)
        PAR::find_on_pass_stack
	    ( PAR::default_parser, prefix );
    PAR::default_parser->trace_flags |=
	prefix_pass->trace_subexpressions
	+
	PAR::TRACE_SUBEXPRESSION_ELEMENTS
	+
	PAR::TRACE_SUBEXPRESSION_DETAILS
	+
	PAR::TRACE_SUBEXPRESSION_LINES
	+
	PAR::TRACE_KEYS
	+
	PAR::TRACE_PARSER_OUTPUT;
    PAR::init_line_display
        ( PAR::default_parser,
	    min::DISPLAY_PICTURE );

    PAR::parse();
}