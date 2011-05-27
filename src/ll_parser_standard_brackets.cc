// Layers Language Standard Brackets Parser
//
// File:	ll_parser_standard_brackets.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri May 27 03:42:00 EDT 2011
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
    parser->selectors = PARSTD::CODE;
    PARTAB::push_brackets
        ( min::new_str_gen ( "(" ),
          min::new_str_gen ( ")" ),
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  PARTAB::new_selectors ( 0, 0, 0 ),
	  parser->bracket_table );
    PARTAB::push_brackets
        ( min::new_str_gen ( "[" ),
          min::new_str_gen ( "]" ),
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  PARTAB::new_selectors ( 0, 0, 0 ),
	  parser->bracket_table );
    PARTAB::push_brackets
        ( min::new_str_gen ( "{" ),
          min::new_str_gen ( "}" ),
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  PARTAB::new_selectors
	      ( PARSTD::MATH,
	        PARSTD::CODE + PARSTD::TEXT, 0 ),
	  parser->bracket_table );
    PARTAB::push_brackets
        ( min::new_str_gen ( "`" ),
          min::new_str_gen ( "'" ),
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  PARTAB::new_selectors
	      ( PARSTD::TEXT,
	        PARSTD::CODE + PARSTD::MATH, 0 ),
	  parser->bracket_table );

    PARTAB::push_indentation_mark
        ( min::new_str_gen ( ":" ),
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  PARTAB::new_selectors ( 0, 0, 0 ),
	  parser->bracket_table,
	  parser->split_table );
}
