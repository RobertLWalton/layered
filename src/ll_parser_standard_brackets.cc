// Layers Language Standard Brackets Parser
//
// File:	ll_parser_standard_brackets.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jul 18 05:08:15 EDT 2011
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

# include <ll_parser.h>
# include <ll_parser_standard.h>
# include <ll_parser_table.h>
# define PAR ll::parser
# define PARSTD ll::parser::standard
# define PARTAB ll::parser::table

// Standard Brackets
// -------- --------

void PARSTD::init_brackets
	( min::ref<PAR::parser> parser )
{
    min::locatable_gen opening_parenthesis;
    opening_parenthesis = min::new_str_gen ( "(" );
    min::locatable_gen closing_parenthesis;
    closing_parenthesis = min::new_str_gen ( ")" );

    min::locatable_gen opening_square;
    opening_square = min::new_str_gen ( "[" );
    min::locatable_gen closing_square;
    closing_square = min::new_str_gen ( "]" );

    min::locatable_gen opening_brace;
    opening_brace = min::new_str_gen ( "{" );
    min::locatable_gen closing_brace;
    closing_brace = min::new_str_gen ( "}" );

    min::locatable_gen opening_quote;
    opening_quote = min::new_str_gen ( "`" );
    min::locatable_gen closing_quote;
    closing_quote = min::new_str_gen ( "'" );

    min::locatable_gen colon;
    colon = min::new_str_gen ( ":" );
    min::locatable_gen semicolon;
    semicolon = min::new_str_gen ( ";" );

    parser->selectors = PARSTD::CODE;
    PARTAB::push_brackets
        ( opening_parenthesis,
	  closing_parenthesis,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  PARTAB::new_selectors ( 0, 0, 0 ),
	  false,
	  parser->bracket_table );
    PARTAB::push_brackets
        ( opening_square,
          closing_square,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  PARTAB::new_selectors ( 0, 0, 0 ),
	  false,
	  parser->bracket_table );
    PARTAB::push_brackets
        ( opening_brace,
          closing_brace,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  PARTAB::new_selectors
	      ( PARSTD::MATH,
	        PARSTD::CODE + PARSTD::TEXT, 0 ),
	  false,
	  parser->bracket_table );
    PARTAB::push_brackets
        ( opening_quote,
          closing_quote,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  PARTAB::new_selectors
	      ( PARSTD::TEXT,
	        PARSTD::CODE + PARSTD::MATH, 0 ),
	  false,
	  parser->bracket_table );

    PARTAB::push_indentation_mark
        ( colon, semicolon,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  PARTAB::new_selectors ( 0, 0, 0 ),
	  parser->bracket_table,
	  parser->split_table );
}
