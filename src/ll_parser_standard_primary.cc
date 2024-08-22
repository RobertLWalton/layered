// Layered Languages Standard Primary Parser
//
// File:	ll_parser_standard_standard.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Aug 22 04:51:56 PM EDT 2024
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Define Standard Primary

// Usage and Setup
// ----- --- -----

# include <mex.h>
# include <ll_parser.h>
# include <ll_parser_table.h>
# include <ll_parser_standard.h>
# include <ll_parser_primary.h>
# define PAR ll::parser
# define TAB ll::parser::table
# define PARSTD ll::parser::standard
# define PRIM ll::parser::primary

// Define Standard Primary
// ------ -------- -------
//
static void define_arithmetic_operators
	( TAB::key_table symbol_table )
{
    min::locatable_gen plus
        ( min::new_str_gen ( "+" ) );
    min::locatable_gen minus
        ( min::new_str_gen ( "-" ) );
	  
    PRIM::push_op_func
        ( plus, PRIM::INFIX,
	  symbol_table,
	  ( ( (min::uns32) mex::ADD << 24 )
	    +
	    ( (min::uns32) mex::SUB << 16 )
	    +
	    PRIM::OPERATOR_CALL ) );
}

void PARSTD::define_primary
	( PAR::parser parser, TAB::flags components )
{
    if ( ! ( components & PARSTD::PRIMARY_PARSING ) )
        return;

    PRIM::primary_pass pass =
        PRIM::init_primary ( parser );
    TAB::key_table symbol_table = pass->primary_table;

    if ( components & PARSTD::ARITHMETIC_OPERATORS )
        ::define_arithmetic_operators ( symbol_table );
}

