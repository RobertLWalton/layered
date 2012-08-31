// Layers Language Standard Brackets Parser
//
// File:	ll_parser_standard_brackets.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Aug 31 05:27:56 EDT 2012
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
# define TAB ll::parser::table

// Standard Brackets
// -------- --------

void PARSTD::init_brackets
	( min::ref<PAR::parser> parser )
{
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

    min::locatable_gen top_level
        ( min::new_str_gen ( "TOP-LEVEL" ) );
        // Note TOP-LEVEL is not a legal lexeme and
	// so cannot appear in the input.

    min::locatable_gen code
        ( min::new_str_gen ( "code" ) );
    min::locatable_gen math
        ( min::new_str_gen ( "math" ) );
    min::locatable_gen text
        ( min::new_str_gen ( "text" ) );

    TAB::push_name
	( parser->selector_name_table, code );
    TAB::push_name
	( parser->selector_name_table, math );
    TAB::push_name
	( parser->selector_name_table, text );

    parser->selectors = PARSTD::CODE;

    min::phrase_position pp;   // TBD: make top level

    TAB::push_brackets
        ( opening_parenthesis,
	  closing_parenthesis,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  0, pp,
	  TAB::new_flags ( 0, 0, 0 ),
	  false,
	  parser->bracket_table );
    TAB::push_brackets
        ( opening_square,
          closing_square,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  0, pp,
	  TAB::new_flags ( 0, 0, 0 ),
	  false,
	  parser->bracket_table );
    TAB::push_brackets
        ( opening_brace,
          closing_brace,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  0, pp,
	  TAB::new_flags
	      ( PARSTD::MATH,
	        PARSTD::CODE + PARSTD::TEXT, 0 ),
	  false,
	  parser->bracket_table );
    TAB::push_brackets
        ( opening_quote,
          closing_quote,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  0, pp,
	  TAB::new_flags
	      ( PARSTD::TEXT,
	        PARSTD::CODE + PARSTD::MATH, 0 ),
	  false,
	  parser->bracket_table );

    TAB::push_indentation_mark
        ( colon, semicolon,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  0, pp,
	  TAB::new_flags ( 0, 0, 0 ),
	  parser->bracket_table,
	  parser->split_table );

    TAB::push_indentation_mark
        ( top_level, semicolon,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  0, pp,
	  TAB::new_flags ( 0, 0, 0 ),
	  parser->bracket_table );

    TAB::top_level_indentation_mark =
        (TAB::indentation_mark)
	 TAB::find
	    ( top_level, TAB::ALL_FLAGS,
	      parser->bracket_table );
}
