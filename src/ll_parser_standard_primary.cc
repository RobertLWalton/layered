// Layered Languages Standard Primary Parser
//
// File:	ll_parser_standard_primary.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Aug 24 05:07:55 PM EDT 2024
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
    min::locatable_gen times
        ( min::new_str_gen ( "*" ) );
    min::locatable_gen divide
        ( min::new_str_gen ( "/" ) );

    PRIM::push_infix_op
        ( plus, symbol_table, mex::ADD, mex::SUB );
    PRIM::push_infix_op
        ( minus, symbol_table, mex::SUB, mex::ADD );

    PRIM::push_builtin_func
        ( minus, symbol_table, mex::NEG );

    PRIM::push_infix_op
        ( times, symbol_table, mex::MUL );

    PRIM::push_infix_op
        ( divide, symbol_table, mex::DIV );
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

