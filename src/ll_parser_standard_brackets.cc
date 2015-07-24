// Layered Languages Standard Brackets Parser
//
// File:	ll_parser_standard_brackets.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Jul 24 16:12:57 EDT 2015
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
    min::locatable_gen data_name
        ( min::new_str_gen ( "data" ) );
    min::locatable_gen label_name
        ( min::new_str_gen ( "label" ) );
    min::locatable_gen typed_bracketed_name
        ( min::new_lab_gen ( "typed", "bracketed" ) );

    TAB::flags code =
        1ull << TAB::find_name
	    ( parser->selector_name_table, code_name );
    TAB::flags math =
        1ull << TAB::find_name
	    ( parser->selector_name_table, math_name );
    TAB::flags text =
        1ull << TAB::find_name
	    ( parser->selector_name_table, text_name );
    TAB::flags data =
        1ull << TAB::find_name
	    ( parser->selector_name_table, data_name );

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

    min::locatable_gen opening_double_brace
        ( min::new_lab_gen ( "{", "{" ) );
    min::locatable_gen closing_double_brace
        ( min::new_lab_gen ( "}", "}" ) );

    min::locatable_gen opening_quote
        ( min::new_str_gen ( "`" ) );
    min::locatable_gen closing_quote
        ( min::new_str_gen ( "'" ) );

    min::locatable_gen opening_square_angle
        ( min::new_lab_gen ( "[", "<" ) );
    min::locatable_gen angle_closing_square
        ( min::new_lab_gen ( ">", "]" ) );

    min::locatable_gen colon
        ( min::new_str_gen ( ":" ) );
    min::locatable_gen semicolon
        ( min::new_str_gen ( ";" ) );
    min::locatable_gen comma
        ( min::new_str_gen ( "," ) );
    min::locatable_gen bar
        ( min::new_str_gen ( "|" ) );
    min::locatable_gen equal
        ( min::new_str_gen ( "=" ) );

    min::locatable_gen no
        ( min::new_str_gen ( "no" ) );

    min::locatable_gen opening_brace_star
        ( min::new_lab_gen ( "{", "*" ) );

    parser->selectors = code;

    min::uns32 block_level =
        PAR::block_level ( parser );
    BRA::push_brackets
        ( opening_parenthesis,
	  closing_parenthesis,
	  code + math + text + data,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  min::NULL_STUB, min::NULL_STUB, 0,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_square,
          closing_square,
	  code + math + text + data,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  min::NULL_STUB, min::NULL_STUB, 0,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_double_brace,
          closing_double_brace,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( math, code + text, 0 ),
	  min::NULL_STUB, min::NULL_STUB, 0,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_double_brace,
          closing_double_brace,
	  data,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  min::NULL_STUB, min::NULL_STUB, 0,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_quote,
          closing_quote,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( text, code + math, 0 ),
	  min::NULL_STUB, min::NULL_STUB, 0,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_quote,
          closing_quote,
	  data,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  min::NULL_STUB, min::NULL_STUB, 0,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_square_angle,
          angle_closing_square,
	  code + math + text + data,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( data, TAB::ALL_FLAGS ^ data, 0 ),
	  PAR::find_reformatter
	      ( label_name, BRA::reformatter_stack ),
	  min::NULL_STUB, 0,
	  bracketed_pass->bracket_table );

    BRA::push_indentation_mark
        ( colon, semicolon,
	  code,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  bracketed_pass->bracket_table );

    BRA::push_typed_brackets
        ( opening_brace, bar, closing_brace,
	  code + math + text + data,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( data, TAB::ALL_FLAGS ^ data, 0 ),
	  PAR::find_reformatter
	      ( typed_bracketed_name,
	        BRA::reformatter_stack ),
	  min::NULL_STUB, 0,
	  colon, equal, comma, no,
	  opening_square,
	  min::standard_attr_flag_parser,
	  opening_brace_star,
	  bracketed_pass->bracket_table );
}
