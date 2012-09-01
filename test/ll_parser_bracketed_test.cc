// Layers Language Parser Bracketed Subexpression Test
//
// File:	ll_parser_bracketed_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Aug 31 23:54:34 EDT 2012
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
# define PARBRA ll::parser::bracketed
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
    PAR::default_parser->trace =
        PAR::TRACE_BRACKETED_SUBEXPRESSIONS;
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

    min::phrase_position pp; // TBD: set to top level.

    PARBRA::bracketed_pass bracketed_pass =
	(PARBRA::bracketed_pass)
	PAR::default_parser->pass_stack;

    TAB::push_brackets
        ( openplus,
          closeplus,
	  PARSTD::MATH,
	  PAR::default_parser->block_level, pp,
	  TAB::new_flags ( 0, 0, 0 ),
	  false,
	  bracketed_pass->bracket_table );
    TAB::push_brackets
        ( openminus,
          closeminus,
	  PARSTD::TEXT,
	  PAR::default_parser->block_level, pp,
	  TAB::new_flags ( 0, 0, 0 ),
	  false,
	  bracketed_pass->bracket_table );
    PAR::parse();
}
