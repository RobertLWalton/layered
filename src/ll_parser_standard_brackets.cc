// Layered Languages Standard Brackets Parser
//
// File:	ll_parser_standard_brackets.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Mar 10 13:13:40 EST 2023
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

static void define_brackets
	( PAR::parser parser,
	  TAB::flags components,
	  TAB::flags label,
	  TAB::flags code,
	  TAB::flags text,
	  TAB::flags math )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) parser->pass_stack;
    min::uns32 block_level =
        PAR::block_level ( parser );

    min::locatable_gen data_name
        ( min::new_str_gen ( "data" ) );
    min::locatable_gen atom_name
        ( min::new_str_gen ( "atom" ) );
    min::locatable_gen label_name
        ( min::new_str_gen ( "label" ) );
    min::locatable_gen special_name
        ( min::new_str_gen ( "special" ) );

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

    // In the following be sure parsing selectors
    // argument does nothing to ALWAYS_SELECTOR.
    // Use COMMAND_SELECTORS instead of ALL_SELECTORS.

    BRA::push_brackets
        ( opening_square_dollar,
          dollar_closing_square,
	  label + code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags
	      ( data, PAR::COMMAND_SELECTORS ^ data ),
	  PAR::find_reformatter
	      ( special_name,
	        BRA::untyped_reformatter_stack ),
	  min::MISSING(),
	  bracketed_pass->bracket_table );

    if ( label )
    {
	BRA::push_brackets
	    ( opening_square_angle,
	      angle_closing_square,
	      label + code + math + text,
	      block_level, PAR::top_level_position,
	      TAB::new_flags
		  ( label,
		    PAR::COMMAND_SELECTORS ^ label ),
	      PAR::find_reformatter
		  ( label_name,
		    BRA::untyped_reformatter_stack ),
	      min::MISSING(),
	      bracketed_pass->bracket_table );
    }

    if ( code + text + math == 0 ) return;

    BRA::push_brackets
        ( PARLEX::left_parenthesis,
	  PARLEX::right_parenthesis,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  min::NULL_STUB, min::MISSING(),
	  bracketed_pass->bracket_table );
    BRA::push_brackets
        ( PARLEX::left_square,
          PARLEX::right_square,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  min::NULL_STUB, min::MISSING(),
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

    if ( math ) BRA::push_brackets
        ( opening_double_brace,
          closing_double_brace,
	  code + math + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( math, code + text ),
	  min::NULL_STUB, min::MISSING(),
	  bracketed_pass->bracket_table );

    if ( text )
    {
	BRA::push_brackets
	    ( opening_quote,
	      closing_quote,
	      code + math + text,
	      block_level, PAR::top_level_position,
	      TAB::new_flags
		  ( text,
		    PAR::COMMAND_SELECTORS ^ text ),
	      min::NULL_STUB, min::MISSING(),
	      bracketed_pass->bracket_table );

	min::locatable_gen text_name
	    ( min::new_str_gen ( "text" ) );
	min::locatable_gen period
	    ( min::new_str_gen ( "." ) );
	min::locatable_gen question
	    ( min::new_str_gen ( "?" ) );
	min::locatable_gen exclamation
	    ( min::new_str_gen ( "!" ) );
	min::locatable_gen s
	    ( min::new_str_gen ( "s" ) );

	min::locatable_gen text_arguments
		( min::new_obj_gen ( 6 ) );
	min::obj_vec_insptr tavp ( text_arguments );
	min::attr_push ( tavp ) = s;
	min::attr_push ( tavp ) = period;
	min::attr_push ( tavp ) = question;
	min::attr_push ( tavp ) = exclamation;
	min::attr_push ( tavp ) = PARLEX::colon;
	min::attr_push ( tavp ) = PARLEX::semicolon;
	BRA::push_brackets
	    ( opening_double_quote,
	      closing_double_quote,
	      code + math + text,
	      block_level, PAR::top_level_position,
	      TAB::new_flags ( text + PAR::ETPREFIX_OPT,
	                       code + math ),
	      PAR::find_reformatter
		  ( text_name,
		    BRA::untyped_reformatter_stack ),
	      text_arguments,
	      bracketed_pass->bracket_table );
    }
}

static void define_indentation_marks
	( PAR::parser parser,
	  TAB::flags components,
	  TAB::flags label,
	  TAB::flags code,
	  TAB::flags text,
	  TAB::flags math )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) parser->pass_stack;
    min::uns32 block_level =
        PAR::block_level ( parser );

    min::locatable_var<min::phrase_position_vec_insptr>
	pos;
    min::init ( pos, parser->input_file,
		PAR::top_level_position, 0 );

    min::uns32 paragraph_check = PAR::MISSING_MASTER;
    min::uns32 data_check = PAR::MISSING_MASTER;
    if ( components & ( PARSTD::ID | PARSTD::TABLE ) )
    {
	min::locatable_gen paragraph_check_name
	    ( min::new_str_gen ( "PARAGRAPH-CHECK" ) );
	min::locatable_gen data_check_name
	    ( min::new_str_gen ( "DATA-CHECK" ) );
	paragraph_check =
	    PAR::get_lexical_master
		( paragraph_check_name, parser );
	data_check =
	    PAR::get_lexical_master
		( data_check_name, parser );
    }

    if ( code ) BRA::push_indentation_mark
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

    if ( text )
    {
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
    }

    // This must be the LAST indentation mark pushed.
    //
    if ( components & PARSTD::ID )
    {
	min::locatable_gen data_name
	    ( min::new_str_gen ( "data" ) );
	min::locatable_gen data_paragraph_name
	    ( min::new_lab_gen
	          ( "data", "paragraph" ) );

	TAB::flags data =
	    1ull << TAB::find_name
		( parser->selector_name_table,
		  data_name );
	TAB::flags data_paragraph =
	    1ull << TAB::find_name
		( parser->selector_name_table,
		  data_paragraph_name );
	BRA::push_indentation_mark
	    ( PARLEX::colon, min::MISSING(),
	      data_paragraph,
	      block_level, PAR::top_level_position,
	      TAB::new_flags (   PAR::DEFAULT_EA_OPT
	                       + data,
				 PAR::ALL_EA_OPT
			       - PAR::DEFAULT_EA_OPT
			       + PAR::COMMAND_SELECTORS
			       - data ),
	      min::MISSING(),
	      PAR::MISSING_MASTER,
	      PAR::MISSING_MASTER,
	      bracketed_pass->bracket_table );
    }
}

static void define_bracket_types
	( PAR::parser parser,
	  TAB::flags components,
	  TAB::flags label,
	  TAB::flags code,
	  TAB::flags text,
	  TAB::flags math )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) parser->pass_stack;
    min::uns32 block_level =
        PAR::block_level ( parser );

    min::locatable_gen three_stars
        ( min::new_str_gen ( "***" ) );

    BRA::push_bracket_type
	( three_stars,
	    PAR::TOP_LEVEL_SELECTOR
	  + code + text,
	  block_level, PAR::top_level_position,
	  TAB::new_flags ( 0, 0, 0 ),
	  PARLEX::RESET,
	  min::MISSING(),
	  min::MISSING(),
	  PAR::MISSING_MASTER,
	  min::NULL_STUB,
	  min::MISSING(),
	  bracketed_pass->bracket_type_table );

    min::locatable_gen data_name
        ( min::new_str_gen ( "data" ) );
    TAB::flags data =
        1ull << TAB::find_name
	    ( parser->selector_name_table, data_name );

    min::locatable_var<min::phrase_position_vec_insptr>
	pos;
    min::init ( pos, parser->input_file,
		PAR::top_level_position, 0 );

    if ( components & PARSTD::ID )
    {
	min::locatable_gen data_paragraph_name
	    ( min::new_lab_gen
	          ( "data", "paragraph" ) );
	TAB::flags data_paragraph =
	    1ull << TAB::find_name
		( parser->selector_name_table,
	      data_paragraph_name );

	min::locatable_gen data_initiators
		( min::new_obj_gen ( 2 ) );
	min::obj_vec_insptr divp ( data_initiators );
	min::attr_push ( divp ) = PARLEX::colon;
	min::attr_push ( divp ) = PARLEX::double_colon;

	min::locatable_gen data_arguments
		( min::new_obj_gen ( 7 ) );
	min::obj_vec_insptr davp ( data_arguments );
	min::locatable_gen clear_sign
	    ( min::new_str_gen ( "@@@@" ) );
	min::attr_push ( davp ) = clear_sign;
	min::locatable_gen assign_sign
	    ( min::new_str_gen ( ":=" ) );
	min::attr_push ( davp ) = assign_sign;
	min::attr_push ( davp ) = data_initiators;
	min::attr_push ( davp ) = PARLEX::equal;
	min::attr_push ( davp ) = PARLEX::no;
	min::attr_push ( davp ) = PARLEX::left_square;
	min::locatable_gen left_curly_star
	    ( min::new_lab_gen ( "{", "*" ) );
	min::attr_push ( davp ) = left_curly_star;

	min::locatable_gen data_type_name
	    ( min::new_str_gen ( "*DATA*" ) );
	BRA::push_bracket_type
	    ( data_type_name,
		PAR::TOP_LEVEL_SELECTOR
	      + data + code + text + math,
	      block_level, PAR::top_level_position,
	      TAB::new_flags
	          ( data_paragraph,
		    PAR::ALL_PREFIX_OPT ),
	      PARLEX::LINE,   // group
	      min::MISSING(), // implied_subprefix
	      min::MISSING(), // implied_subprefix_type
	      PAR::MISSING_MASTER,
	      PAR::find_reformatter
		( data_name,
		  BRA::bracket_type_reformatter_stack ),
	      data_arguments,
	      bracketed_pass->bracket_type_table );

	min::locatable_gen raw_data_type_name
	    ( min::new_lab_gen ( "*RAW*", "*DATA*" ) );
	BRA::push_bracket_type
	    ( raw_data_type_name,
		PAR::TOP_LEVEL_SELECTOR
	      + data + code + text + math,
	      block_level, PAR::top_level_position,
	      TAB::new_flags
		  ( data + data_paragraph,
		      (   PAR::COMMAND_SELECTORS
		        ^ data ^ data_paragraph )
		     + PAR::ALL_PREFIX_OPT ),
	      PARLEX::LINE,   // group
	      min::MISSING(), // implied_subprefix
	      min::MISSING(), // implied_subprefix_type
	      PAR::MISSING_MASTER,
	      PAR::find_reformatter
		( data_name,
		  BRA::bracket_type_reformatter_stack ),
	      data_arguments,
	      bracketed_pass->bracket_type_table );
    }

    if ( ( components & PARSTD::TABLE )
         &&
	 ( code | text ) )
    {
	min::locatable_gen table
	    ( min::new_str_gen ( "table" ) );
	min::locatable_gen row
	    ( min::new_str_gen ( "row" ) );

	min::locatable_gen row_check_name
	    ( min::new_str_gen ( "ROW-CHECK" ) );
	min::uns32 row_check =
	    PAR::get_lexical_master
		( row_check_name, parser );

	BRA::push_bracket_type
	    ( table,
	      code + text,
	      block_level, PAR::top_level_position,
	      TAB::new_flags
		  ( text,
		      ( PAR::COMMAND_SELECTORS ^ text )
		    + PAR::EALSEP_OPT ),
	      PARLEX::PARAGRAPH,
	      min::MISSING(), // implied_subprefix
	      min::MISSING(), // implied_subprefix_type
	      row_check,
	      min::NULL_STUB,
	      min::MISSING(),
	      bracketed_pass->bracket_type_table );

	BRA::push_bracket_type
	    ( row,
	      text,
	      block_level, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      PARLEX::LINE,
	      min::MISSING(),
	      min::MISSING(),
	      PAR::MISSING_MASTER,
	      min::NULL_STUB,
	      min::MISSING(),
	      bracketed_pass->bracket_type_table );
    }

    if ( text )
    {
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
	      code + text,
	      block_level, PAR::top_level_position,
	      TAB::new_flags
		  (   PAR::EAPBREAK_OPT
		    + PAR::EALTINDENT_OPT
		    + PAR::ETPREFIX_OPT + text,
		      PAR::EALEINDENT_OPT
		    + PAR::EAINDENT_OPT
		    + PAR::EALSEP_OPT
		    + PAR::EAOCLOSING_OPT
		    + code + math + data ),
	      PARLEX::PARAGRAPH, // group
	      implied_s_header,
	      s,
	      PAR::MISSING_MASTER,
	      min::NULL_STUB,
	      min::MISSING(),
	      bracketed_pass->bracket_type_table );

	min::locatable_gen p
	    ( min::new_str_gen ( "p" ) );

	BRA::push_bracket_type
	    ( p,
	      code + text,
	      block_level, PAR::top_level_position,
	      TAB::new_flags
		  (   PAR::EAPBREAK_OPT
		    + PAR::EALTINDENT_OPT
		    + PAR::ETPREFIX_OPT
		    + text,
		      PAR::EALEINDENT_OPT
		    + PAR::EAINDENT_OPT
		    + PAR::EALSEP_OPT
		    + PAR::EAOCLOSING_OPT
		    + code + math + data ),
	      PARLEX::PARAGRAPH,
	      implied_s_header,
	      s,
	      PAR::MISSING_MASTER,
	      min::NULL_STUB,
	      min::MISSING(),
	      bracketed_pass->bracket_type_table );

	min::locatable_gen quote_name
	    ( min::new_str_gen ( "quote" ) );

	BRA::push_bracket_type
	    ( quote_name,
	      code + text,
	      block_level, PAR::top_level_position,
	      TAB::new_flags
		  (   PAR::EAPBREAK_OPT
		    + PAR::EALTINDENT_OPT
		    + PAR::ETPREFIX_OPT
		    + text,
		      PAR::EALEINDENT_OPT
		    + PAR::EAINDENT_OPT
		    + PAR::EALSEP_OPT
		    + PAR::EAOCLOSING_OPT
		    + code + math + data ),
	      PARLEX::PARAGRAPH,
	      implied_s_header,
	      s,
	      PAR::MISSING_MASTER,
	      min::NULL_STUB,
	      min::MISSING(),
	      bracketed_pass->bracket_type_table );

	min::locatable_gen sentence
	    ( min::new_str_gen ( "sentence" ) );
	min::locatable_gen period
	    ( min::new_str_gen ( "." ) );
	min::locatable_gen question
	    ( min::new_str_gen ( "?" ) );
	min::locatable_gen exclamation
	    ( min::new_str_gen ( "!" ) );

	min::locatable_gen s_arguments
		( min::new_obj_gen ( 3 ) );
	min::obj_vec_insptr savp ( s_arguments );
	min::attr_push ( savp ) = period;
	min::attr_push ( savp ) = question;
	min::attr_push ( savp ) = exclamation;

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
}

// Define Standard Bracket Pass Components
//

void PARSTD::define_brackets
	( PAR::parser parser, TAB::flags components )
{
    TAB::flags needed = 0;
        // Only needed selectors listed in components
	// are defined, as unneeded selectors are never
	// referenced.

    if ( components & PARSTD::BRACKETS )
	needed |= PARSTD::LABEL + PARSTD::CODE
	        + PARSTD::TEXT + PARSTD::MATH;
    if ( components & PARSTD::INDENTATION_MARKS )
	needed |= PARSTD::CODE + PARSTD::TEXT;
    if ( components & PARSTD::BRACKET_TYPES )
	needed |= PARSTD::CODE + PARSTD::TEXT
	                       + PARSTD::MATH;
    needed &= components;

    TAB::flags label = 0;
    if ( needed & PARSTD::LABEL )
    {
	min::locatable_gen label_name
	    ( min::new_str_gen ( "label" ) );
        label = 1ull << TAB::find_name
		  ( parser->selector_name_table,
		    label_name );
    }
    TAB::flags code = 0;
    if ( needed & PARSTD::CODE )
    {
	min::locatable_gen code_name
	    ( min::new_str_gen ( "code" ) );
        code = 1ull << TAB::find_name
		  ( parser->selector_name_table,
		    code_name );
    }
    TAB::flags text = 0;
    if ( needed & PARSTD::TEXT )
    {
	min::locatable_gen text_name
	    ( min::new_str_gen ( "text" ) );
        text = 1ull << TAB::find_name
	          ( parser->selector_name_table,
		    text_name );
    }
    TAB::flags math = 0;
    if ( needed & PARSTD::MATH )
    {
	min::locatable_gen math_name
	    ( min::new_str_gen ( "math" ) );
        math = 1ull << TAB::find_name
	          ( parser->selector_name_table,
		    math_name );
    }

    if ( label + code + text + math == 0 ) return;

    if ( components & PARSTD::BRACKETS )
	::define_brackets
	    ( parser, components,
	      label, code, text, math );
    if ( components & PARSTD::INDENTATION_MARKS )
	::define_indentation_marks
	    ( parser, components,
	      label, code, text, math );
    if ( components & PARSTD::BRACKET_TYPES )
	::define_bracket_types
	    ( parser, components,
	      label, code, text, math );
}
