// Layers Language Parser Bracketed Subexpression Test
//
// File:	ll_parser_bracketed_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Nov 12 04:24:39 EST 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include <ll_parser.h>
# include <ll_parser_bracketed.h>
# include <ll_parser_standard.h>
# include <iostream>
# include <cassert>
# define PAR ll::parser
# define TAB ll::parser::table
# define BRA ll::parser::bracketed
# define PARSTD ll::parser::standard
using std::cout;

int main ( int argc, const char * argv[] )
{
    min::initialize();
    PAR::init ( PAR::default_parser, false );
    PARSTD::init_input ( PAR::default_parser );
    PARSTD::init_brackets ( PAR::default_parser );
    PAR::init_input_stream
        ( PAR::default_parser, std::cin );
    PAR::init_ostream
        ( PAR::default_parser, std::cout );
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass)
	PAR::default_parser->pass_stack;
    PAR::default_parser->trace_flags =
        bracketed_pass->trace_subexpressions
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

    min::locatable_gen math_name
        ( min::new_str_gen ( "math" ) );
    min::locatable_gen text_name
        ( min::new_str_gen ( "text" ) );

    TAB::flags math =
        1ull << TAB::find_name
	    ( PAR::default_parser->selector_name_table,
	      math_name );
    TAB::flags text =
        1ull << TAB::find_name
	    ( PAR::default_parser->selector_name_table,
	      text_name );

    min::locatable_gen openplus
        ( min::new_str_gen ( "<+" ) );
    min::locatable_gen closeplus
        ( min::new_str_gen ( "+>" ) );
    min::locatable_gen openminus
        ( min::new_str_gen ( "<-" ) );
    min::locatable_gen closeminus
        ( min::new_str_gen ( "->" ) );

    min::uns32 block_level =
        PAR::block_level ( PAR::default_parser );
    BRA::push_brackets
        ( openplus,
          closeplus,
	  math,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  false,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( openminus,
          closeminus,
	  text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  false,
	  bracketed_pass->bracket_table );
    PAR::parse();
}
