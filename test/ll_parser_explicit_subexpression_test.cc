// Layers Language Parser Explicit Subexpression Test
//
// File:	ll_parser_explicit_subexpression_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Feb 19 22:47:32 EST 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include <ll_parser.h>
# include <ll_parser_standard.h>
# include <iostream>
# include <cassert>
# define PAR ll::parser
# define TAB ll::parser::table
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
    PAR::default_parser->trace =
        PAR::TRACE_EXPLICIT_SUBEXPRESSIONS;
    PAR::init_print_flags
        ( PAR::default_parser,
	    min::GRAPHIC_VSPACE_FLAG
	  + min::GRAPHIC_NSPACE_FLAG );
    min::locatable_gen openplus
        ( min::new_str_gen ( "<+" ) );
    min::locatable_gen closeplus
        ( min::new_str_gen ( "+>" ) );
    min::locatable_gen openminus
        ( min::new_str_gen ( "<-" ) );
    min::locatable_gen closeminus
        ( min::new_str_gen ( "->" ) );
    TAB::push_brackets
        ( openplus,
          closeplus,
	  PARSTD::MATH,
	  TAB::new_selectors ( 0, 0, 0 ),
	  false,
	  PAR::default_parser->bracket_table );
    TAB::push_brackets
        ( openminus,
          closeminus,
	  PARSTD::TEXT,
	  TAB::new_selectors ( 0, 0, 0 ),
	  false,
	  PAR::default_parser->bracket_table );
    PAR::parse();
}
