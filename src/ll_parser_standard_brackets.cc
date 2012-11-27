// Layers Language Standard Brackets Parser
//
// File:	ll_parser_standard_brackets.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Nov 27 04:09:15 EST 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Standard Brackets

// Usage and Setup
// ----- --- -----

# include <ll_parser_table.h>
# include <ll_parser.h>
# include <ll_parser_bracketed.h>
# include <ll_parser_standard.h>
# define PAR ll::parser
# define TAB ll::parser::table
# define BRA ll::parser::bracketed
# define PARSTD ll::parser::standard

// Standard Brackets
// -------- --------

void PARSTD::init_brackets
	( min::ref<PAR::parser> parser )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) parser->pass_stack;
    min::locatable_gen opening_parenthesis
        ( min::new_str_gen ( "(" ) );
    min::locatable_gen closing_parenthesis
        ( min::new_str_gen ( ")" ) );

    min::locatable_gen opening_square
        ( min::new_str_gen ( "[" ) );
    min::locatable_gen closing_square
        ( min::new_str_gen ( "]" ) );

    min::locatable_gen opening_brace
        ( min::new_str_gen ( "{" ) );
    min::locatable_gen closing_brace
        ( min::new_str_gen ( "}" ) );

    min::locatable_gen opening_quote
        ( min::new_str_gen ( "`" ) );
    min::locatable_gen closing_quote
        ( min::new_str_gen ( "'" ) );

    min::locatable_gen colon
        ( min::new_str_gen ( ":" ) );
    min::locatable_gen semicolon
        ( min::new_str_gen ( ";" ) );

    parser->selectors = PAR::CODE_SELECTOR;

    min::uns32 block_level =
        PAR::block_level ( parser );
    BRA::push_brackets
        ( opening_parenthesis,
	  closing_parenthesis,
	  PAR::CODE_SELECTOR + PAR::MATH_SELECTOR
	                     + PAR::TEXT_SELECTOR,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  false,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_square,
          closing_square,
	  PAR::CODE_SELECTOR + PAR::MATH_SELECTOR
	                     + PAR::TEXT_SELECTOR,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  false,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_brace,
          closing_brace,
	  PAR::CODE_SELECTOR + PAR::MATH_SELECTOR
	                     + PAR::TEXT_SELECTOR,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( PAR::MATH_SELECTOR,
	        PAR::CODE_SELECTOR + PAR::TEXT_SELECTOR,
		0 ),
	  false,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_quote,
          closing_quote,
	  PAR::CODE_SELECTOR + PAR::MATH_SELECTOR
	                     + PAR::TEXT_SELECTOR,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( PAR::TEXT_SELECTOR,
	        PAR::CODE_SELECTOR + PAR::MATH_SELECTOR,
		0 ),
	  false,
	  bracketed_pass->bracket_table );

    BRA::push_indentation_mark
        ( colon, semicolon,
	  PAR::CODE_SELECTOR + PAR::MATH_SELECTOR
	                     + PAR::TEXT_SELECTOR,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  bracketed_pass->bracket_table,
	  bracketed_pass->split_table );
}
