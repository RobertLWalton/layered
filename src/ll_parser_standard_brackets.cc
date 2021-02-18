// Layered Languages Standard Brackets Parser
//
// File:	ll_parser_standard_brackets.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Feb 18 00:49:56 EST 2021
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

    min::locatable_var<min::phrase_position_vec_insptr>
	pos;
    min::init ( pos, parser->input_file,
		PAR::top_level_position, 0 );

    min::locatable_gen code_name
        ( min::new_str_gen ( "code" ) );
    min::locatable_gen math_name
        ( min::new_str_gen ( "math" ) );
    min::locatable_gen text_name
        ( min::new_str_gen ( "text" ) );
    min::locatable_gen data_name
        ( min::new_str_gen ( "data" ) );
    min::locatable_gen atom_name
        ( min::new_str_gen ( "atom" ) );
    min::locatable_gen data_paragraph_name
        ( min::new_lab_gen ( "data", "paragraph" ) );
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
    TAB::flags atom =
        1ull << TAB::find_name
	    ( parser->selector_name_table, atom_name );
    TAB::flags data_paragraph =
        1ull << TAB::find_name
	    ( parser->selector_name_table,
	      data_paragraph_name );

    min::locatable_gen paragraph_check_name
        ( min::new_str_gen ( "PARAGRAPH-CHECK" ) );
    min::locatable_gen data_check_name
        ( min::new_str_gen ( "DATA-CHECK" ) );
    min::uns32 paragraph_check =
        PAR::get_lexical_master
	    ( paragraph_check_name, parser );
    min::uns32 data_check =
        PAR::get_lexical_master
	    ( data_check_name, parser );

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

    min::locatable_gen opening_brace_dollar
        ( min::new_lab_gen ( "{", "$" ) );
    min::locatable_gen dollar_closing_brace
        ( min::new_lab_gen ( "$", "}" ) );

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

    min::locatable_gen period
        ( min::new_str_gen ( "." ) );
    min::locatable_gen question
        ( min::new_str_gen ( "?" ) );
    min::locatable_gen exclamation
        ( min::new_str_gen ( "!" ) );
    min::locatable_gen s
        ( min::new_str_gen ( "s" ) );

    parser->selectors &= PAR::ALL_OPT;
    parser->selectors |= code | PAR::TOP_LEVEL_SELECTOR
                              | PAR::ALWAYS_SELECTOR;

    // In the following be sure parsing selectors
    // argument does nothing to ALWAYS_SELECTOR.
    // Use COMMAND_SELECTORS instead of ALL_SELECTORS.

    min::uns32 block_level =
        PAR::block_level ( parser );
    BRA::push_brackets
        ( PARLEX::left_parenthesis,
	  PARLEX::right_parenthesis,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  min::NULL_STUB, min::NULL_STUB,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( PARLEX::left_square,
          PARLEX::right_square,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  min::NULL_STUB, min::NULL_STUB,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_double_brace,
          closing_double_brace,
	  data + code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( math, data + code + text ),
	  min::NULL_STUB, min::NULL_STUB,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_quote,
          closing_quote,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( atom, PAR::COMMAND_SELECTORS ^ atom ),
	  min::NULL_STUB, min::NULL_STUB,
	  bracketed_pass->bracket_table );

    min::locatable_var
    	    <min::packed_vec_insptr<min::gen> >
        text_arguments
	    ( min::gen_packed_vec_type.new_stub ( 6 ) );
    min::push ( text_arguments ) = s;
    min::push ( text_arguments ) = period;
    min::push ( text_arguments ) = period;
    min::push ( text_arguments ) = question;
    min::push ( text_arguments ) = exclamation;
    min::push ( text_arguments ) = PARLEX::colon;
    min::push ( text_arguments ) = PARLEX::semicolon;
    BRA::push_brackets
        ( opening_double_quote,
          closing_double_quote,
	  data + code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( text, data + code + math ),
	  PAR::find_reformatter
	      ( text_name,
	        BRA::untyped_reformatter_stack ),
	  text_arguments,
	  bracketed_pass->bracket_table );

    BRA::push_brackets
        ( opening_square_angle,
          angle_closing_square,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( atom, PAR::COMMAND_SELECTORS ^ atom ),
	  PAR::find_reformatter
	      ( label_name,
	        BRA::untyped_reformatter_stack ),
	  min::NULL_STUB,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_square_dollar,
          dollar_closing_square,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( atom, PAR::COMMAND_SELECTORS ^ atom ),
	  PAR::find_reformatter
	      ( special_name,
	        BRA::untyped_reformatter_stack ),
	  min::NULL_STUB,
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( opening_brace_dollar,
          dollar_closing_brace,
	  code,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  min::NULL_STUB, min::NULL_STUB,
	  bracketed_pass->bracket_table );

    BRA::push_indentation_mark
        ( PARLEX::colon, PARLEX::semicolon,
	  code,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( PAR::DEFAULT_EA_OPT,
	                     PAR::ALL_EA_OPT
			   - PAR::DEFAULT_EA_OPT ),
	  min::MISSING(),
	  paragraph_check,
	  data_check,
	  bracketed_pass->bracket_table );

    min::locatable_gen p
        ( min::new_str_gen ( "p" ) );
    min::locatable_gen implied_p_header
        ( min::new_obj_gen ( 10, 1 ) );
    {
        min::obj_vec_insptr vp ( implied_p_header );
	min::attr_insptr ap ( vp );
	min::locate ( ap, min::dot_type );
	min::set ( ap, p );
	min::locate ( ap, min::dot_position );
	min::set ( ap, min::new_stub_gen ( pos ) );
	min::set_flag
	    ( ap, min::standard_attr_hide_flag );
    }


    BRA::push_indentation_mark
        ( PARLEX::colon, min::MISSING(),
	  text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( PAR::DEFAULT_EA_OPT,
	                     PAR::ALL_EA_OPT
			   - PAR::DEFAULT_EA_OPT ),
	  implied_p_header,
	  paragraph_check,
	  data_check,
	  bracketed_pass->bracket_table );

    // This must be the LAST indentation mark pushed.
    //
    BRA::push_indentation_mark
        ( PARLEX::colon, min::MISSING(),
	  data_paragraph,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( PAR::DEFAULT_EA_OPT + data,
	                     PAR::ALL_EA_OPT
			   - PAR::DEFAULT_EA_OPT
			   + COMMAND_SELECTORS
			   - data ),
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
	  PAR::ALL_SELECTORS,
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
    min::locatable_gen data_paragraph_name
        ( min::new_lab_gen ( "data", "paragraph" ) );
    min::locatable_gen three_stars
        ( min::new_str_gen ( "***" ) );

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
    TAB::flags data_paragraph =
        1ull << TAB::find_name
	    ( parser->selector_name_table,
	      data_paragraph_name );

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

    BRA::push_bracket_type
	( data_name,
	    PAR::TOP_LEVEL_SELECTOR
	  + data + code + text + math,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( data_paragraph, 0, 0 ),
	  PARLEX::line,   // group
	  min::MISSING(), // implied_subprefix
	  min::MISSING(), // implied_subprefix_type
	  PAR::MISSING_MASTER,
	  PAR::find_reformatter
	      ( data_name,
	        BRA::bracket_type_reformatter_stack ),
	  data_arguments,
	  bracketed_pass->bracket_type_table );

    BRA::push_bracket_type
	( raw_data,
	    PAR::TOP_LEVEL_SELECTOR
	  + data + code + text + math,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( data + data_paragraph,
	          PAR::COMMAND_SELECTORS
		^ data ^ data_paragraph ),
	  PARLEX::line,   // group
	  min::MISSING(), // implied_subprefix
	  min::MISSING(), // implied_subprefix_type
	  PAR::MISSING_MASTER,
	  PAR::find_reformatter
	      ( data_name,
	        BRA::bracket_type_reformatter_stack ),
	  data_arguments,
	  bracketed_pass->bracket_type_table );

    min::locatable_gen table
        ( min::new_str_gen ( "table" ) );
    min::locatable_gen row
        ( min::new_str_gen ( "row" ) );
    min::locatable_gen row_check
        ( min::new_str_gen ( "ROW-CHECK" ) );
    min::uns32 row_lexical_master =
        PAR::get_lexical_master
	    ( row_check, parser );

    BRA::push_bracket_type
	( table,
	  code + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( code, PAR::COMMAND_SELECTORS ^ code ),
	  PARLEX::paragraph,
	  min::MISSING(), // implied_subprefix
	  min::MISSING(), // implied_subprefix_type
	  row_lexical_master,
	  min::NULL_STUB,
	  min::NULL_STUB,
	  bracketed_pass->bracket_type_table );

    BRA::push_bracket_type
	( row,
	  code,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( data, PAR::COMMAND_SELECTORS ^ data ),
	  PARLEX::line,
	  min::MISSING(),
	  min::MISSING(),
	  PAR::MISSING_MASTER,
	  min::NULL_STUB,
	  min::NULL_STUB,
	  bracketed_pass->bracket_type_table );

    BRA::push_bracket_type
	( three_stars,
	    PAR::TOP_LEVEL_SELECTOR
	  + code + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  PARLEX::reset,
	  min::MISSING(),
	  min::MISSING(),
	  PAR::MISSING_MASTER,
	  min::NULL_STUB,
	  min::NULL_STUB,
	  bracketed_pass->bracket_type_table );

    min::locatable_gen s
        ( min::new_str_gen ( "s" ) );
    min::locatable_gen implied_s_header
        ( min::new_obj_gen ( 10, 1 ) );
    {
        min::obj_vec_insptr vp ( implied_s_header );
	min::attr_insptr ap ( vp );
	min::locate ( ap, min::dot_type );
	min::set ( ap, s );
	min::locate ( ap, min::dot_position );
	min::set ( ap, min::new_stub_gen ( pos ) );
	min::set_flag
	    ( ap, min::standard_attr_hide_flag );
    }

    min::locatable_gen section
        ( min::new_str_gen ( "section" ) );

    BRA::push_bracket_type
	( section,
	    PAR::TOP_LEVEL_SELECTOR
	  + code + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      (   EAPBREAK_OPT + EALTINDENT_OPT
	        + ETPREFIX_OPT + text,
	          EALEINDENT_OPT + EAINDENT_OPT
		+ EALSEP_OPT + EAOCLOSING_OPT
		+ code + math + data ),
	  PARLEX::paragraph, // group
	  implied_s_header,
	  s,
	  PAR::MISSING_MASTER,
	  min::NULL_STUB,
	  min::NULL_STUB,
	  bracketed_pass->bracket_type_table );

    min::locatable_gen p
        ( min::new_str_gen ( "p" ) );

    BRA::push_bracket_type
	( p,
	    PAR::TOP_LEVEL_SELECTOR
	  + code + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      (   EAPBREAK_OPT + EALTINDENT_OPT
	        + ETPREFIX_OPT + STICKY_OPT + text,
	          EALEINDENT_OPT + EAINDENT_OPT
		+ EALSEP_OPT + EAOCLOSING_OPT
		+ code + math + data ),
	  PARLEX::paragraph,
	  implied_s_header,
	  s,
	  PAR::MISSING_MASTER,
	  min::NULL_STUB,
	  min::NULL_STUB,
	  bracketed_pass->bracket_type_table );

    min::locatable_gen quote_name
        ( min::new_str_gen ( "quote" ) );

    BRA::push_bracket_type
	( quote_name,
	    PAR::TOP_LEVEL_SELECTOR
	  + code + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      (   EAPBREAK_OPT + EALTINDENT_OPT
	        + STICKY_OPT + text,
	          EALEINDENT_OPT + EAINDENT_OPT
		+ EALSEP_OPT + EAOCLOSING_OPT
		+ code + math + data ),
	  PARLEX::paragraph,
	  implied_s_header,
	  s,
	  PAR::MISSING_MASTER,
	  min::NULL_STUB,
	  min::NULL_STUB,
	  bracketed_pass->bracket_type_table );

    min::locatable_gen sentence
        ( min::new_str_gen ( "sentence" ) );
    min::locatable_gen period
        ( min::new_str_gen ( "." ) );
    min::locatable_gen question
        ( min::new_str_gen ( "?" ) );
    min::locatable_gen exclamation
        ( min::new_str_gen ( "!" ) );

    min::locatable_var
    	    <min::packed_vec_insptr<min::gen> >
        s_arguments
	    ( min::gen_packed_vec_type.new_stub ( 5 ) );
    min::push ( s_arguments ) = period;
    min::push ( s_arguments ) = question;
    min::push ( s_arguments ) = exclamation;
    min::push ( s_arguments ) = PARLEX::colon;
    min::push ( s_arguments ) = PARLEX::semicolon;

    BRA::push_bracket_type
	( s, text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  min::MISSING(), // group
	  min::MISSING(), // implied_subprefix
	  min::MISSING(), // implied_subprefix_type
	  PAR::MISSING_MASTER,
	  PAR::find_reformatter
	      ( sentence,
	        BRA::bracket_type_reformatter_stack ),
	  s_arguments,
	  bracketed_pass->bracket_type_table );
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
