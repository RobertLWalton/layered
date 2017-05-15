// Layered Languages Standard Brackets Parser
//
// File:	ll_parser_standard_brackets.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon May 15 03:22:40 EDT 2017
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
# define PARLEX ll::parser::lexeme
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
    min::locatable_gen special_name
        ( min::new_str_gen ( "special" ) );

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

    min::locatable_gen opening_square_dollar
        ( min::new_lab_gen ( "[", "$" ) );
    min::locatable_gen dollar_closing_square
        ( min::new_lab_gen ( "$", "]" ) );

    min::locatable_gen no
        ( min::new_str_gen ( "no" ) );

    min::locatable_gen opening_brace_star
        ( min::new_lab_gen ( "{", "*" ) );

    min::locatable_gen double_vbar
        ( min::new_str_gen ( "||" ) );

    parser->selectors &= PAR::ALL_OPT;
    parser->selectors |= code | PAR::TOP_LEVEL_SELECTOR
                              | PAR::ALWAYS_SELECTOR;

    min::uns32 block_level =
        PAR::block_level ( parser );
    BRA::push_brackets
        ( PARLEX::left_parenthesis,
	  PARLEX::right_parenthesis,
	  code + math + text + data,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  min::NULL_STUB, min::NULL_STUB,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( PARLEX::left_square,
          PARLEX::right_square,
	  code + math + text + data,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  min::NULL_STUB, min::NULL_STUB,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_double_brace,
          closing_double_brace,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( math, code + text, 0 ),
	  min::NULL_STUB, min::NULL_STUB,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_quote,
          closing_quote,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( text, code + math, 0 ),
	  min::NULL_STUB, min::NULL_STUB,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_square_angle,
          angle_closing_square,
	  code + math + text + data,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( data, PAR::ALL_SELECTORS ^ data, 0 ),
	  PAR::find_reformatter
	      ( label_name,
	        BRA::untyped_reformatter_stack ),
	  min::NULL_STUB,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_square_dollar,
          dollar_closing_square,
	  code + math + text + data,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( data, PAR::ALL_SELECTORS ^ data, 0 ),
	  PAR::find_reformatter
	      ( special_name,
	        BRA::untyped_reformatter_stack ),
	  min::NULL_STUB,
	  bracketed_pass->bracket_table );

    BRA::push_indentation_mark
        ( PARLEX::colon, PARLEX::semicolon,
	  code,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( PAR::DEFAULT_EA_OPT,
	                     PAR::ALL_EA_OPT
			   - PAR::DEFAULT_EA_OPT,
			   0 ),
	  min::MISSING(), PAR::MISSING_MASTER,
	  bracketed_pass->bracket_table );

    BRA::push_typed_brackets
        ( PARLEX::left_curly,
	  PARLEX::vbar,
	  double_vbar,
	  PARLEX::right_curly,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  data,
	  PARLEX::colon, PARLEX::equal, PARLEX::comma,
	  PARLEX::no,
	  PARLEX::left_square,
	  min::standard_attr_flag_parser,
	  opening_brace_star,
	  true,
	  bracketed_pass->bracket_table );
}
