// Layered Languages Standard Primary Parser
//
// File:	ll_parser_standard_primary.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Aug 27 03:43:05 AM EDT 2024
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

static void define_comparison_operators
	( TAB::key_table symbol_table )
{
    min::locatable_gen leq
        ( min::new_str_gen ( "<=" ) );
    min::locatable_gen lt
        ( min::new_str_gen ( "<" ) );
    min::locatable_gen geq
        ( min::new_str_gen ( ">=" ) );
    min::locatable_gen gt
        ( min::new_str_gen ( ">" ) );
    min::locatable_gen eq
        ( min::new_str_gen ( "==" ) );
    min::locatable_gen neq
        ( min::new_str_gen ( "!=" ) );

    PRIM::push_logical_op
        ( leq, symbol_table, PRIM::COMPARE );
    PRIM::push_logical_op
        ( lt, symbol_table, PRIM::COMPARE );
    PRIM::push_logical_op
        ( geq, symbol_table, PRIM::COMPARE );
    PRIM::push_logical_op
        ( gt, symbol_table, PRIM::COMPARE );
    PRIM::push_logical_op
        ( eq, symbol_table, PRIM::COMPARE );
    PRIM::push_logical_op
        ( neq, symbol_table, PRIM::COMPARE );
}

static void define_selection_operators
	( TAB::key_table symbol_table )
{

    min::locatable_gen if_op
        ( min::new_str_gen ( "if" ) );

    PRIM::push_logical_op
        ( if_op, symbol_table, PRIM::IF );
}

static void define_logical_operators
	( TAB::key_table symbol_table )
{

    min::locatable_gen but_not_op
        ( min::new_lab_gen ( "BUT", "NOT" ) );
    min::locatable_gen and_op
        ( min::new_str_gen ( "AND" ) );
    min::locatable_gen or_op
        ( min::new_str_gen ( "OR" ) );
    min::locatable_gen not_op
        ( min::new_str_gen ( "NOT" ) );

    PRIM::push_logical_op
        ( but_not_op, symbol_table, PRIM::BUT_NOT );
    PRIM::push_logical_op
        ( and_op, symbol_table, PRIM::AND );
    PRIM::push_logical_op
        ( or_op, symbol_table, PRIM::OR );
    PRIM::push_logical_func
        ( not_op, symbol_table, PRIM::NOT );
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
    if ( components & PARSTD::COMPARISON_OPERATORS )
        ::define_comparison_operators ( symbol_table );
    if ( components & PARSTD::SELECTION_OPERATORS )
        ::define_selection_operators ( symbol_table );
    if ( components & PARSTD::LOGICAL_OPERATORS )
        ::define_logical_operators ( symbol_table );
}

