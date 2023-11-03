// Layered Languages Standard Parser Settings
//
// File:	ll_parser_standard.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Nov  3 02:10:17 EDT 2023
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Standard

// Usage and Setup
// ----- --- -----

# include <ll_parser_table.h>
# include <ll_parser.h>
# include <ll_parser_standard.h>
# include <ll_parser_bracketed.h>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define PARLEX ll::parser::lexeme
# define TAB ll::parser::table
# define BRA ll::parser::bracketed
# define PARSTD ll::parser::standard

static min::locatable_gen label;
static min::locatable_gen code;
static min::locatable_gen text;
static min::locatable_gen math;
static min::locatable_gen id;
static min::locatable_gen table;
static min::locatable_gen block;
static min::locatable_gen top_level;
static min::locatable_gen concatenator;
static min::locatable_gen lexeme_map;
static min::locatable_gen brackets;
static min::locatable_gen indentation_marks;
static min::locatable_gen bracket_types;
static min::locatable_gen control_operators;
static min::locatable_gen iteration_operators;
static min::locatable_gen assignment_operators;
static min::locatable_gen selection_operators;
static min::locatable_gen logical_operators;
static min::locatable_gen comparison_operators;
static min::locatable_gen arithmetic_operators;
static min::locatable_gen bitwise_operators;
static min::locatable_gen other_qualifiers;
static min::locatable_gen all_qualifiers;
static min::locatable_gen other_setup;
static min::locatable_gen all_setup;
static min::locatable_gen other_brackets;
static min::locatable_gen all_brackets;
static min::locatable_gen other_operators;
static min::locatable_gen all_operators;
static min::locatable_gen all_other;

min::locatable_var<ll::parser::table::name_table>
    PARSTD::component_name_table;
min::locatable_var<ll::parser::table::key_table>
    PARSTD::component_group_name_table;

static void initialize ( void )
{
    ::label = min::new_str_gen ( "label" );
    ::code = min::new_str_gen ( "code" );
    ::text = min::new_str_gen ( "text" );
    ::math = min::new_str_gen ( "math" );
    ::id = min::new_str_gen ( "id" );
    ::table = min::new_str_gen ( "table" );

    ::block = min::new_str_gen ( "block" );

    ::top_level = min::new_lab_gen ( "top", "level" );
    ::concatenator =
        min::new_str_gen ( "concatenator" );
    ::lexeme_map = min::new_lab_gen ( "lexeme", "map" );

    ::brackets = min::new_str_gen ( "brackets" );
    ::indentation_marks =
        min::new_lab_gen ( "indentation", "marks" );
    ::bracket_types =
        min::new_lab_gen ( "bracket", "types" );

    ::control_operators =
        min::new_lab_gen ( "control", "operators" );
    ::iteration_operators =
        min::new_lab_gen ( "iteration", "operators" );
    ::assignment_operators =
        min::new_lab_gen ( "assignment", "operators" );
    ::selection_operators =
        min::new_lab_gen ( "selection", "operators" );
    ::logical_operators =
        min::new_lab_gen ( "logical", "operators" );
    ::comparison_operators =
        min::new_lab_gen ( "comparison", "operators" );
    ::arithmetic_operators =
        min::new_lab_gen ( "arithmetic", "operators" );
    ::bitwise_operators =
        min::new_lab_gen ( "bitwise", "operators" );

    ::other_qualifiers =
        min::new_lab_gen ( "other", "qualifiers" );
    ::all_qualifiers =
        min::new_lab_gen ( "all", "qualifiers" );
    ::other_setup =
        min::new_lab_gen ( "other", "setup" );
    ::all_setup =
        min::new_lab_gen ( "all", "setup" );
    ::other_brackets =
        min::new_lab_gen ( "other", "brackets" );
    ::all_brackets =
        min::new_lab_gen ( "all", "brackets" );
    ::other_operators =
        min::new_lab_gen ( "other", "operators" );
    ::all_operators =
        min::new_lab_gen ( "all", "operators" );
    ::all_other =
        min::new_lab_gen ( "all", "other" );

    TAB::init_name_table
        ( PARSTD::component_name_table );

    MIN_REQUIRE
	(    PARSTD::LABEL
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::label ) );

    MIN_REQUIRE
	(    PARSTD::CODE
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::code ) );
    MIN_REQUIRE
	(    PARSTD::TEXT
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::text ) );
    MIN_REQUIRE
	(    PARSTD::MATH
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::math ) );

    MIN_REQUIRE
	(    PARSTD::ID
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::id ) );
    MIN_REQUIRE
	(    PARSTD::TABLE
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::table ) );

    MIN_REQUIRE
	(    PARSTD::BLOCK
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::block ) );

    MIN_REQUIRE
	(    PARSTD::TOP_LEVEL
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::top_level ) );
    MIN_REQUIRE
	(    PARSTD::CONCATENATOR
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::concatenator ) );
    MIN_REQUIRE
	(    PARSTD::LEXEME_MAP
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::lexeme_map ) );

    MIN_REQUIRE
	(    PARSTD::BRACKETS
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::brackets ) );
    MIN_REQUIRE
	(    PARSTD::INDENTATION_MARKS
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::indentation_marks ) );
    MIN_REQUIRE
	(    PARSTD::BRACKET_TYPES
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::bracket_types ) );

    MIN_REQUIRE
	(    PARSTD::CONTROL_OPERATORS
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::control_operators ) );
    MIN_REQUIRE
	(    PARSTD::ITERATION_OPERATORS
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::iteration_operators ) );
    MIN_REQUIRE
	(    PARSTD::ASSIGNMENT_OPERATORS
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::assignment_operators ) );
    MIN_REQUIRE
	(    PARSTD::SELECTION_OPERATORS
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::selection_operators ) );
    MIN_REQUIRE
	(    PARSTD::LOGICAL_OPERATORS
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::logical_operators ) );
    MIN_REQUIRE
	(    PARSTD::COMPARISON_OPERATORS
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::comparison_operators ) );
    MIN_REQUIRE
	(    PARSTD::ARITHMETIC_OPERATORS
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::arithmetic_operators ) );
    MIN_REQUIRE
	(    PARSTD::BITWISE_OPERATORS
	  == 1ull << TAB::push_name
		  ( PARSTD::component_name_table,
		    ::bitwise_operators ) );

    PARSTD::component_group_name_table =
	TAB::create_key_table ( 32 );

    TAB::push_root
	( ::other_qualifiers, PARSTD::ALL_QUALIFIERS,
	  0, PAR::top_level_position,
	  PARSTD::component_group_name_table );
    TAB::push_root
	( ::all_qualifiers, PARSTD::ALL_QUALIFIERS,
	  0, PAR::top_level_position,
	  PARSTD::component_group_name_table );
    TAB::push_root
	( ::other_setup, PARSTD::ALL_SETUP,
	  0, PAR::top_level_position,
	  PARSTD::component_group_name_table );
    TAB::push_root
	( ::all_setup, PARSTD::ALL_SETUP,
	  0, PAR::top_level_position,
	  PARSTD::component_group_name_table );
    TAB::push_root
	( ::other_brackets, PARSTD::ALL_BRACKETS,
	  0, PAR::top_level_position,
	  PARSTD::component_group_name_table );
    TAB::push_root
	( ::all_brackets, PARSTD::ALL_BRACKETS,
	  0, PAR::top_level_position,
	  PARSTD::component_group_name_table );
    TAB::push_root
	( ::other_operators, PARSTD::ALL_OPERATORS,
	  0, PAR::top_level_position,
	  PARSTD::component_group_name_table );
    TAB::push_root
	( ::all_operators, PARSTD::ALL_OPERATORS,
	  0, PAR::top_level_position,
	  PARSTD::component_group_name_table );
    TAB::push_root
	( ::all_other, PARSTD::ALL,
	  0, PAR::top_level_position,
	  PARSTD::component_group_name_table );
}

static min::initializer initializer ( ::initialize );

// Standard
// --------

static void define_block ( PAR::parser parser )
{

    min::gen result =
        PAR::begin_block
	    ( parser, PARLEX::standard,
	      PAR::top_level_position );

    MIN_REQUIRE ( result == min::SUCCESS() );
}

static void define_top_level
        ( PAR::parser parser, TAB::flags components )
{
    min::uns32 block_level =
        PAR::block_level ( parser );
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) parser->pass_stack;
    TAB::key_table bracket_table =
        bracketed_pass->bracket_table;

    const char * selector_name =
        ( components & PARSTD::CODE ) ? "code" :
        ( components & PARSTD::TEXT ) ? "text" :
        ( components & PARSTD::MATH ) ? "math" :
	                                "data";

    min::locatable_gen name
        ( min::new_str_gen ( selector_name ) );

    BRA::indentation_mark imark =
        parser->top_level_indentation_mark;

    TAB::flags selector =
        1ull << TAB::find_name
	    ( parser->selector_name_table, name );

    imark->parsing_selectors.or_flags |= selector;
    imark->parsing_selectors.not_flags &= ~ selector;
    parser->selectors =
          imark->parsing_selectors.or_flags
	| PAR::TOP_LEVEL_OFF_SELECTORS
	| PAR::ALWAYS_SELECTOR;

    if ( components & PARSTD::CODE )
	line_sep_ref(imark) =
	    BRA::push_line_sep
		( PARLEX::semicolon,
		  block_level,
		  PAR::top_level_position,
	      bracket_table );

    if ( components & PARSTD::ID )
    {
	* (min::Uchar *) & parser->id_map
	                         ->ID_character = '@';

	min::locatable_gen data_check
	    ( min::new_str_gen ( "DATA-CHECK" ) );
	imark->line_lexical_master =
	      PAR::get_lexical_master
		  ( data_check, parser );
	if ( components & PARSTD::TABLE )
	{
	    min::locatable_gen paragraph_check
		( min::new_str_gen
		      ( "PARAGRAPH-CHECK" ) );
	    imark->paragraph_lexical_master =
		  PAR::get_lexical_master
		      ( paragraph_check, parser );
	}
	else
	    imark->paragraph_lexical_master =
	        imark->line_lexical_master;
    }
    else if ( components & PARSTD::TABLE )
    {
	min::locatable_gen table_check
	    ( min::new_str_gen ( "TABLE-CHECK" ) );
	imark->paragraph_lexical_master =
	      PAR::get_lexical_master
		  ( table_check, parser );
    }
}

static void define_concatenator ( PAR::parser parser )
{
    BRA::bracketed_pass bracketed_pass =
	(BRA::bracketed_pass) parser->pass_stack;

    BRA::string_concatenator_ref ( bracketed_pass ) =
	PARLEX::number_sign;
    BRA::middle_break mb = { "#", "#", 1, 1 };
    bracketed_pass->middle_break = mb;
}

static void define_lexeme_map
        ( PAR::parser parser, TAB::flags components )
{
    min::uns32 block_level =
        PAR::block_level ( parser );

    min::locatable_var<min::phrase_position_vec_insptr>
	pos;
    min::init ( pos, parser->input_file,
		PAR::top_level_position, 0 );

    PAR::init_lexeme_map ( parser );

    if ( components & PARSTD::ID )
    {
	min::locatable_gen data_name
	    ( min::new_str_gen ( "*DATA*" ) );
	min::gen data_t_name =
	    LEX::lexeme_type_name
		( LEXSTD::data_t, parser->scanner );
	min::locatable_gen data_value
	    ( min::new_obj_gen ( 10, 1 ) );
	{
	    min::obj_vec_insptr vp ( data_value );
	    min::attr_insptr ap ( vp );
	    min::locate ( ap, min::dot_type );
	    min::set ( ap, data_name );
	    min::locate ( ap, min::dot_position );
	    min::set ( ap, min::new_stub_gen ( pos ) );
	    min::set_flag
		( ap, min::standard_attr_hide_flag );
	}

	TAB::push_lexeme_map_entry
	    ( data_t_name,
	      LEXSTD::data_t,
	      PAR::TOP_LEVEL_SELECTOR,
	      block_level,
	      PAR::top_level_position,
	      data_value,
	      PAR::MAPPED_PREFIX,
	      data_name,
	      PAR::MISSING_MASTER,
	      parser->lexeme_map );

	min::locatable_gen raw_data_name
	    ( min::new_lab_gen ( "*RAW*", "*DATA*" ) );
	min::gen raw_data_t_name =
	    LEX::lexeme_type_name
		( LEXSTD::raw_data_t, parser->scanner );
	min::locatable_gen raw_data_value
	    ( min::new_obj_gen ( 10, 1 ) );
	{
	    min::obj_vec_insptr vp ( raw_data_value );
	    min::attr_insptr ap ( vp );
	    min::locate ( ap, min::dot_type );
	    min::set ( ap, raw_data_name );
	    min::locate ( ap, min::dot_position );
	    min::set ( ap, min::new_stub_gen ( pos ) );
	    min::set_flag
		( ap, min::standard_attr_hide_flag );
	}

	TAB::push_lexeme_map_entry
	    ( raw_data_t_name,
	      LEXSTD::raw_data_t,
	      PAR::TOP_LEVEL_SELECTOR,
	      block_level,
	      PAR::top_level_position,
	      raw_data_value,
	      PAR::MAPPED_PREFIX,
	      raw_data_name,
	      PAR::MISSING_MASTER,
	      parser->lexeme_map );
    }

    if ( components & PARSTD::TABLE )
    {
	min::locatable_gen table
	    ( min::new_str_gen ( "table" ) );
	min::gen table_t_name =
	    LEX::lexeme_type_name
		( LEXSTD::table_t, parser->scanner );
	min::locatable_gen table_value
	    ( min::new_obj_gen ( 10, 1 ) );
	{
	    min::obj_vec_insptr vp ( table_value );
	    min::attr_insptr ap ( vp );
	    min::locate ( ap, min::dot_type );
	    min::set ( ap, table );
	    min::locate ( ap, min::dot_position );
	    min::set ( ap, min::new_stub_gen ( pos ) );
	    min::set_flag
		( ap, min::standard_attr_hide_flag );
	}

	TAB::push_lexeme_map_entry
	    ( table_t_name,
	      LEXSTD::table_t,
	      PAR::TOP_LEVEL_SELECTOR,
	      block_level,
	      PAR::top_level_position,
	      table_value,
	      PAR::MAPPED_PREFIX,
	      table,
	      PAR::MISSING_MASTER,
	      parser->lexeme_map );

	min::locatable_gen row
	    ( min::new_str_gen ( "row" ) );
	min::gen row_t_name =
	    LEX::lexeme_type_name
		( LEXSTD::row_t, parser->scanner );
	min::locatable_gen row_value
	    ( min::new_obj_gen ( 10, 1 ) );
	{
	    min::obj_vec_insptr vp ( row_value );
	    min::attr_insptr ap ( vp );
	    min::locate ( ap, min::dot_type );
	    min::set ( ap, row );
	    min::locate ( ap, min::dot_position );
	    min::set ( ap, min::new_stub_gen ( pos ) );
	    min::set_flag
		( ap, min::standard_attr_hide_flag );
	}

	TAB::push_lexeme_map_entry
	    ( row_t_name,
	      LEXSTD::row_t,
	      PAR::TOP_LEVEL_SELECTOR,
	      block_level,
	      PAR::top_level_position,
	      row_value,
	      PAR::MAPPED_PREFIX,
	      row,
	      PAR::MISSING_MASTER,
	      parser->lexeme_map );
    }
}

void PARSTD::define_standard
	( PAR::parser parser, TAB::flags components )
{
    if ( components & PARSTD::BLOCK )
        ::define_block ( parser );
    if ( components & PARSTD::TOP_LEVEL )
        ::define_top_level ( parser, components );
    if ( components & PARSTD::CONCATENATOR )
        ::define_concatenator ( parser );
    if ( components & PARSTD::LEXEME_MAP )
	::define_lexeme_map ( parser, components );

    PARSTD::define_brackets ( parser, components );
    PARSTD::define_operators ( parser, components );
    PARSTD::define_primary ( parser, components );
}
