// Layered Languages Standard Brackets Parser
//
// File:	ll_parser_standard_brackets.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Apr 12 03:29:46 EDT 2019
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Standard Brackets
//	Standard Prefixes
//	Standard ID Character

// Usage and Setup
// ----- --- -----

# include <ll_parser_table.h>
# include <ll_parser.h>
# include <ll_parser_bracketed.h>
# include <ll_parser_standard.h>
# define PAR ll::parser
# define PARLEX ll::parser::lexeme
# define TAB ll::parser::table
# define PRE ll::parser::prefix
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

    min::locatable_gen opening_double_quote
        ( min::new_str_gen ( "``" ) );
    min::locatable_gen closing_double_quote
        ( min::new_str_gen ( "''" ) );

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
	  code + math + text + data,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( 0, code + math + text + data, 0 ),
	  min::NULL_STUB, min::NULL_STUB,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_double_quote,
          closing_double_quote,
	  code + math + text + data,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( text, code + math + data, 0 ),
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
	  min::MISSING(),
	  PAR::MISSING_MASTER,
	  PAR::MISSING_MASTER,
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

// Standard Prefixes
// -------- --------

void PARSTD::init_prefix ( PAR::parser parser )
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

    min::uns32 block_level =
        PAR::block_level ( parser );

    min::locatable_var<min::phrase_position_vec_insptr>
	pos;
    min::init ( pos, parser->input_file,
		PAR::top_level_position, 0 );

    min::locatable_gen raw_data
        ( min::new_lab_gen ( "raw", "data" ) );

    min::locatable_var
    	    <min::packed_vec_insptr<min::gen> >
        data_arguments
	    ( min::gen_packed_vec_type.new_stub ( 4 ) );
    min::push ( data_arguments ) = PARLEX::equal;
    min::push ( data_arguments ) = PARLEX::no;
    min::push ( data_arguments ) = PARLEX::left_square;
    min::locatable_gen left_curly_star
        ( min::new_lab_gen ( "{", "*" ) );
    min::push ( data_arguments ) = left_curly_star;

    PRE::push_prefix
	( data_name,
	    PAR::TOP_LEVEL_SELECTOR
	  + data + code + text + math,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  min::MISSING(), // group
	  min::MISSING(), // implied_subprefix
	  min::MISSING(), // implied_subprefix_type
	  PAR::MISSING_MASTER,
	  PAR::MISSING_MASTER,
	  PAR::find_reformatter
	      ( data_name,
	        PRE::prefix_reformatter_stack ),
	  data_arguments,
	  bracketed_pass->prefix_table );

    PRE::push_prefix
	( raw_data,
	    PAR::TOP_LEVEL_SELECTOR
	  + data + code + text + math,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( data, PAR::ALL_SELECTORS ^ data, 0 ),
	  min::MISSING(), // group
	  min::MISSING(), // implied_subprefix
	  min::MISSING(), // implied_subprefix_type
	  PAR::MISSING_MASTER,
	  PAR::MISSING_MASTER,
	  min::NULL_STUB,
	  min::NULL_STUB,
	  bracketed_pass->prefix_table );

    min::locatable_gen table
        ( min::new_str_gen ( "table" ) );
    min::locatable_gen row
        ( min::new_str_gen ( "row" ) );
    min::locatable_gen row_check
        ( min::new_str_gen ( "ROW-CHECK" ) );
    min::uns32 row_lexical_master =
        PAR::get_lexical_master
	    ( row_check, parser );

    PRE::push_prefix
	( table,
	    PAR::TOP_LEVEL_SELECTOR
	  + data + code + text + math,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  PARLEX::paragraph,
	  min::MISSING(), // implied_subprefix
	  min::MISSING(), // implied_subprefix_type
	  PAR::MISSING_MASTER,
	  row_lexical_master,
	  min::NULL_STUB,
	  min::NULL_STUB,
	  bracketed_pass->prefix_table );

    PRE::push_prefix
	( row,
	    PAR::TOP_LEVEL_SELECTOR
	  + data + code + text + math,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  PARLEX::line,
	  min::MISSING(),
	  min::MISSING(),
	  PAR::MISSING_MASTER,
	  PAR::MISSING_MASTER,
	  min::NULL_STUB,
	  min::NULL_STUB,
	  bracketed_pass->prefix_table );

    min::locatable_gen p
        ( min::new_str_gen ( "p" ) );

    PRE::push_prefix
	( p,
	    PAR::TOP_LEVEL_SELECTOR
	  + code + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( EAPBREAK_OPT + EALTINDENT_OPT + text,
	          EALEINDENT_OPT + EAINDENT_OPT
		+ EALSEP_OPT + EAOCLOSING_OPT
		+ code + math + data, 0 ),
	  PARLEX::paragraph,
	  min::MISSING(),
	  min::MISSING(),
	  PAR::MISSING_MASTER,
	  PAR::MISSING_MASTER,
	  min::NULL_STUB,
	  min::NULL_STUB,
	  bracketed_pass->prefix_table );
}

// Standard ID Character
// -------- -- ---------

void PARSTD::init_ID_character ( PAR::parser parser )
{
    parser->ID_character = '@';
}

// Standard Concatenators
// -------- -------------

void PARSTD::init_concatenator_character
	( PAR::parser parser )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) parser->pass_stack;

    BRA::string_concatenator_ref ( bracketed_pass ) =
        PARLEX::number_sign;
    BRA::middle_break mb = { "#", "#", 1, 1 };
    bracketed_pass->middle_break = mb;
}
