// Layered Languages Standard Brackets Parser
//
// File:	ll_parser_standard_brackets.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Dec  1 01:48:17 EST 2012
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

void PARSTD::init_brackets ( PAR::parser parser )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) parser->pass_stack;

    min::locatable_gen code_name
        ( min::new_str_gen ( "code" ) );
    min::locatable_gen math_name
        ( min::new_str_gen ( "math" ) );
    min::locatable_gen text_name
        ( min::new_str_gen ( "text" ) );

    TAB::flags code =
        1ull << TAB::find_name
	    ( parser->selector_name_table, code_name );
    TAB::flags math =
        1ull << TAB::find_name
	    ( parser->selector_name_table, math_name );
    TAB::flags text =
        1ull << TAB::find_name
	    ( parser->selector_name_table, text_name );

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

    parser->selectors = code;

    min::uns32 block_level =
        PAR::block_level ( parser );
    BRA::push_brackets
        ( opening_parenthesis,
	  closing_parenthesis,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  false,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_square,
          closing_square,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  false,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_brace,
          closing_brace,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( math, code + text, 0 ),
	  false,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_quote,
          closing_quote,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( text, code + math, 0 ),
	  false,
	  bracketed_pass->bracket_table );

    BRA::push_indentation_mark
        ( colon, semicolon,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  bracketed_pass->bracket_table,
	  bracketed_pass->split_table );
}
