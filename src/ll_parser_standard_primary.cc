// Layered Languages Standard Primary Parser
//
// File:	ll_parser_standard_primary.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Nov  9 03:19:16 AM EST 2024
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
    min::locatable_gen exponent
        ( min::new_str_gen ( "^" ) );

    PRIM::push_value_op
        ( plus, symbol_table, mex::ADD, mex::SUB );
    PRIM::push_value_op
        ( minus, symbol_table, mex::SUB, mex::ADD );

    PRIM::push_builtin_func
        ( minus, symbol_table, mex::NEG );

    PRIM::push_value_op
        ( times, symbol_table, mex::MUL );

    PRIM::push_value_op
        ( divide, symbol_table, mex::DIV );

    PRIM::push_value_op
        ( exponent, symbol_table, mex::POW );
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
        ( min::new_str_gen ( "IF" ) );

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

static void define_test_operators
	( TAB::key_table symbol_table )
{

    min::locatable_gen is_true_op
        ( min::new_lab_gen ( "is", "true" ) );
    min::locatable_gen is_false_op
        ( min::new_lab_gen ( "is", "false" ) );
    min::locatable_gen is_integer_op
        ( min::new_lab_gen ( "is", "integer" ) );
    min::locatable_gen is_finite_op
        ( min::new_lab_gen ( "is", "finite" ) );
    min::locatable_gen is_infinite_op
        ( min::new_lab_gen ( "is", "infinite" ) );
    min::locatable_gen is_nan_op
        ( min::new_lab_gen
	      ( "is", "undefined", "number"  ) );
    min::locatable_gen is_number_op
        ( min::new_lab_gen ( "is", "number" ) );
    min::locatable_gen is_string_op
        ( min::new_lab_gen ( "is", "string" ) );
    min::locatable_gen is_object_op
        ( min::new_lab_gen ( "is", "object" ) );

    PRIM::push_logical_op
        ( is_true_op, symbol_table,
	  PRIM::JMP, mex::JMPTRUE, false );
    PRIM::push_logical_op
        ( is_false_op, symbol_table,
	  PRIM::JMP, mex::JMPFALSE, false );
    PRIM::push_logical_op
        ( is_integer_op, symbol_table,
	  PRIM::JMP, mex::JMPINT, false );
    PRIM::push_logical_op
        ( is_finite_op, symbol_table,
	  PRIM::JMP, mex::JMPFIN, false );
    PRIM::push_logical_op
        ( is_infinite_op, symbol_table,
	  PRIM::JMP, mex::JMPINF, false );
    PRIM::push_logical_op
        ( is_nan_op, symbol_table,
	  PRIM::JMP, mex::JMPNAN, false );
    PRIM::push_logical_op
        ( is_number_op, symbol_table,
	  PRIM::JMP, mex::JMPNUM, false );
    PRIM::push_logical_op
        ( is_string_op, symbol_table,
	  PRIM::JMP, mex::JMPSTR, false );
    PRIM::push_logical_op
        ( is_object_op, symbol_table,
	  PRIM::JMP, mex::JMPOBJ, false );
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
    if ( components & PARSTD::TEST_OPERATORS )
        ::define_test_operators ( symbol_table );
}
