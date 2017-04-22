// Layered Languages Standard Parser Settings
//
// File:	ll_parser_standard.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Apr 22 03:04:14 EDT 2017
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
