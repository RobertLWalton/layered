// Layered Languages Standard Primary Parser
//
// File:	ll_parser_standard_primary.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Feb  7 07:52:07 PM EST 2025
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
# include <mexcom.h>
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
	( TAB::key_table symbol_table,
	  min::gen modifying_ops )
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
    min::locatable_gen left_shift
        ( min::new_str_gen ( "<<" ) );
    min::locatable_gen right_shift
        ( min::new_str_gen ( ">>" ) );

    PRIM::push_value_op
        ( plus, symbol_table, mex::ADD, mex::SUB );
    PRIM::push_value_op
        ( minus, symbol_table, mex::SUB, mex::ADD );

    PRIM::push_builtin_func
        ( minus, symbol_table, mex::NEG );

    PRIM::push_builtin_func
        ( plus, symbol_table, mex::ADDI );
	// immedD assumed to be set to 0

    PRIM::push_value_op
        ( times, symbol_table, mex::MUL );

    PRIM::push_value_op
        ( divide, symbol_table, mex::DIV );

    PRIM::push_value_op
        ( exponent, symbol_table, mex::POW );

    PRIM::push_value_op
        ( left_shift, symbol_table, mex::LSH );
    PRIM::push_value_op
        ( right_shift, symbol_table, mex::RSH );

    min::locatable_gen plus_equal
        ( min::new_lab_gen ( "+", "=" ) );
    min::locatable_gen minus_equal
        ( min::new_lab_gen ( "-", "=" ) );
    min::locatable_gen times_equal
        ( min::new_lab_gen ( "*", "=" ) );
    min::locatable_gen divide_equal
        ( min::new_lab_gen ( "/", "=" ) );
    min::locatable_gen left_shift_equal
        ( min::new_lab_gen ( "<<", "=" ) );
    min::locatable_gen right_shift_equal
        ( min::new_lab_gen ( ">>", "=" ) );

    min::locatable_gen ADD
        ( min::new_num_gen ( mex::ADD ) );
    min::locatable_gen SUB
        ( min::new_num_gen ( mex::SUB ) );
    min::locatable_gen MUL
        ( min::new_num_gen ( mex::MUL ) );
    min::locatable_gen DIV
        ( min::new_num_gen ( mex::DIV ) );
    min::locatable_gen LSH
        ( min::new_num_gen ( mex::LSH ) );
    min::locatable_gen RSH
        ( min::new_num_gen ( mex::RSH ) );
    min::set ( modifying_ops, plus_equal, ADD );
    min::set ( modifying_ops, minus_equal, SUB );
    min::set ( modifying_ops, times_equal, MUL );
    min::set ( modifying_ops, divide_equal, DIV );
    min::set ( modifying_ops, left_shift_equal, LSH );
    min::set ( modifying_ops, right_shift_equal, RSH );
             
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
    min::locatable_gen is_truth_value_op
        ( min::new_lab_gen ( "is", "truth", "value" ) );
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
        ( is_truth_value_op, symbol_table,
	  PRIM::JMP, mex::JMPTRUTH, false );
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

    PRIM::compile_error_function =
        mexcom::compile_error;
    PRIM::compile_warn_function =
        mexcom::compile_warn;
    min::init ( PAR::printer_ref ( parser ) );
    min::init ( PAR::input_file_ref ( parser ) );
    min::init_printer
        ( PAR::input_file_ref(parser), parser->printer );
    mexcom::input_file = parser->input_file;
        // Printer and input_file will be attached to
	// input and output streams or files later,
	// but here we must be sure they exist and
	// parser->printer == parser->input_file->
	// printer.

    PRIM::primary_pass pass =
        PRIM::init_primary ( parser );
    TAB::key_table symbol_table = pass->primary_table;
    min::gen modifying_ops = pass->modifying_ops;

    if ( components & PARSTD::ARITHMETIC_OPERATORS )
        ::define_arithmetic_operators
	    ( symbol_table, modifying_ops );
    if ( components & PARSTD::COMPARISON_OPERATORS )
        ::define_comparison_operators ( symbol_table );
    if ( components & PARSTD::SELECTION_OPERATORS )
        ::define_selection_operators ( symbol_table );
    if ( components & PARSTD::LOGICAL_OPERATORS )
        ::define_logical_operators ( symbol_table );
    if ( components & PARSTD::TEST_OPERATORS )
        ::define_test_operators ( symbol_table );
}
