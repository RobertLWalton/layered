// Layered Languages Standard Parser Settings
//
// File:	ll_parser_standard.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jun 10 22:21:53 EDT 2017
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
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define PARLEX ll::parser::lexeme
# define TAB ll::parser::table
# define PARSTD ll::parser::standard

// Standard
// --------

void PARSTD::init_block ( PAR::parser parser )
{

    min::gen result =
        PAR::begin_block
	    ( parser, PARLEX::standard,
	      PAR::top_level_position );

    MIN_REQUIRE ( result == min::SUCCESS() );

    min::locatable_gen code_name
        ( min::new_str_gen ( "code" ) );

    parser->selectors &= PAR::ALL_OPT;
    parser->selectors |=
          1ull << TAB::find_name
	      ( parser->selector_name_table,
	        code_name )
	| PAR::TOP_LEVEL_SELECTOR
	| PAR::ALWAYS_SELECTOR;
}

void PARSTD::init_lexeme_map ( PAR::parser parser )
{
    PAR::init_lexeme_map ( parser );

    min::locatable_var<min::phrase_position_vec_insptr>
	pos;
    min::init ( pos, parser->input_file,
		PAR::top_level_position, 0 );

    min::locatable_gen paragraph_check
        ( min::new_str_gen ( "PARAGRAPH-CHECK" ) );
    min::locatable_gen data_check
        ( min::new_str_gen ( "DATA-CHECK" ) );
    parser->paragraph_lexical_master =
        PAR::get_lexical_master
	    ( paragraph_check, parser );
    parser->line_lexical_master =
        PAR::get_lexical_master
	    ( data_check, parser );

    min::uns32 block_level =
        PAR::block_level ( parser );

    min::gen data_t_name =
        LEX::lexeme_type_name
	    ( LEXSTD::data_t, parser->scanner );
    min::locatable_gen data_value
        ( min::new_obj_gen ( 10, 1 ) );
    {
        min::obj_vec_insptr vp ( data_value );
	min::attr_insptr ap ( vp );
	min::locate ( ap, min::dot_type );
	min::set ( ap, PARLEX::data );
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
	  PAR::PREFIX,
	  PARLEX::data,
	  PAR::MISSING_MASTER,
	  parser->lexeme_map );

    min::locatable_gen raw_data
        ( min::new_lab_gen ( "raw", "data" ) );
    min::gen raw_data_t_name =
        LEX::lexeme_type_name
	    ( LEXSTD::raw_data_t, parser->scanner );
    min::locatable_gen raw_data_value
        ( min::new_obj_gen ( 10, 1 ) );
    {
        min::obj_vec_insptr vp ( raw_data_value );
	min::attr_insptr ap ( vp );
	min::locate ( ap, min::dot_type );
	min::set ( ap, raw_data );
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
	  PAR::PREFIX,
	  raw_data,
	  PAR::MISSING_MASTER,
	  parser->lexeme_map );

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
	  PAR::PREFIX,
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
	  PAR::PREFIX,
	  row,
	  PAR::MISSING_MASTER,
	  parser->lexeme_map );
}
