// Layered Languages Standard Operators Parser
//
// File:	ll_parser_standard_oper.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Jun 17 16:14:19 EDT 2021
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Standard Operators

// Usage and Setup
// ----- --- -----

# include <ll_parser.h>
# include <ll_parser_standard.h>
# include <ll_parser_table.h>
# include <ll_parser_oper.h>
# define PAR ll::parser
# define PARLEX ll::parser::lexeme
# define PARSTD ll::parser::standard
# define TAB ll::parser::table
# define OP ll::parser::oper
# define OPLEX ll::parser::oper::lexeme

// Standard Operators
// -------- ---------

static void define_control_operators
	( PAR::parser parser,
	  TAB::flags code,
	  TAB::flags math )
{
    if ( code == 0 ) return;

    OP::oper_pass oper_pass = OP::init_oper ( parser );
    min::uns32 block_level =
        PAR::block_level ( parser );

    oper_pass->selectors |= code;

    PAR::reformatter control_reformatter =
        PAR::find_reformatter
	    ( OPLEX::control, OP::reformatter_stack );

    min::locatable_var
    	    <min::packed_vec_insptr<min::gen> >
        condition_arguments
	    ( min::gen_packed_vec_type.new_stub ( 2 ) );
    min::push ( condition_arguments ) = PARLEX::colon;
    min::push ( condition_arguments ) =
        OPLEX::has_condition;

    min::locatable_var
    	    <min::packed_vec_insptr<min::gen> >
        else_arguments
	    ( min::gen_packed_vec_type.new_stub ( 1 ) );
    min::push ( else_arguments ) = PARLEX::colon;

    min::locatable_gen if_name
        ( min::new_str_gen ( "if" ) );

    OP::push_oper
        ( if_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::PREFIX + OP::LINE,
	  0000,
	  control_reformatter,
	  condition_arguments,
	  oper_pass->oper_table );

    min::locatable_gen else_if_name
        ( min::new_lab_gen ( "else", "if" ) );

    OP::push_oper
        ( else_if_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::PREFIX + OP::LINE,
	  0000,
	  control_reformatter,
	  condition_arguments,
	  oper_pass->oper_table );

    min::locatable_gen while_name
        ( min::new_str_gen ( "while" ) );

    OP::push_oper
        ( while_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::PREFIX + OP::LINE,
	  0000,
	  control_reformatter,
	  condition_arguments,
	  oper_pass->oper_table );

    min::locatable_gen until_name
        ( min::new_str_gen ( "until" ) );

    OP::push_oper
        ( until_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::PREFIX + OP::LINE,
	  0000,
	  control_reformatter,
	  condition_arguments,
	  oper_pass->oper_table );

    min::locatable_gen else_name
        ( min::new_str_gen ( "else" ) );

    OP::push_oper
        ( else_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INITIAL + OP::LINE,
	  0000,
	  control_reformatter,
	  else_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( PARLEX::colon,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::AFIX + OP::RIGHT + OP::LINE,
	  0000,
	  min::NULL_STUB,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( PARLEX::colon,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::POSTFIX + OP::AFIX + OP::LINE,
	  0000,
	  min::NULL_STUB,
	  min::NULL_STUB,
	  oper_pass->oper_bracket_table );
}

static void define_assignment_operators
	( PAR::parser parser,
	  TAB::flags code,
	  TAB::flags math )
{
    OP::oper_pass oper_pass = OP::init_oper ( parser );
    min::uns32 block_level =
        PAR::block_level ( parser );

    oper_pass->selectors |= code | math;

    min::locatable_gen binary
        ( min::new_str_gen ( "binary" ) );
    PAR::reformatter binary_reformatter =
        PAR::find_reformatter
	    ( binary, OP::reformatter_stack );
    min::locatable_gen separator
        ( min::new_str_gen ( "separator" ) );
    PAR::reformatter separator_reformatter =
        PAR::find_reformatter
	    ( separator, OP::reformatter_stack );

    min::locatable_gen equal
        ( min::new_str_gen ( "=" ) );

    if ( math ) OP::push_oper
        ( equal,
	  min::MISSING(),
	  math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  1000,
	  binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    if ( code ) OP::push_oper
        ( equal,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000,
	  binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    min::locatable_gen comma
        ( min::new_str_gen ( "," ) );

    OP::push_oper
        ( comma,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::NOFIX,
	  2000, separator_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );
}

static void define_selection_operators
	( PAR::parser parser,
	  TAB::flags code,
	  TAB::flags math )
{
    OP::oper_pass oper_pass = OP::init_oper ( parser );
    min::uns32 block_level =
        PAR::block_level ( parser );

    oper_pass->selectors |= code | math;

    min::locatable_gen selector
        ( min::new_str_gen ( "selector" ) );
    PAR::reformatter selector_reformatter =
        PAR::find_reformatter
	    ( selector, OP::reformatter_stack );

    min::locatable_gen if_op
        ( min::new_str_gen ( "if" ) );
    min::locatable_gen else_op
        ( min::new_str_gen ( "else" ) );

    min::locatable_var
    	    <min::packed_vec_insptr<min::gen> >
        selector_arguments
	    ( min::gen_packed_vec_type.new_stub ( 2 ) );
    min::push ( selector_arguments ) = if_op;
    min::push ( selector_arguments ) = else_op;

    OP::push_oper
        ( if_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  3000, selector_reformatter,
	  selector_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( else_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::AFIX + OP::INFIX,
	  3000,
	  min::NULL_STUB,
	  min::NULL_STUB,
	  oper_pass->oper_table );
}

static void define_logical_operators
	( PAR::parser parser,
	  TAB::flags code,
	  TAB::flags math )
{
    OP::oper_pass oper_pass = OP::init_oper ( parser );
    min::uns32 block_level =
        PAR::block_level ( parser );

    oper_pass->selectors |= code | math;

    min::locatable_gen binary
        ( min::new_str_gen ( "binary" ) );
    PAR::reformatter binary_reformatter =
        PAR::find_reformatter
	    ( binary, OP::reformatter_stack );
    min::locatable_gen infix
        ( min::new_str_gen ( "infix" ) );
    PAR::reformatter infix_reformatter =
        PAR::find_reformatter
	    ( infix, OP::reformatter_stack );
    min::locatable_gen unary
        ( min::new_str_gen ( "unary" ) );
    PAR::reformatter unary_reformatter =
        PAR::find_reformatter
	    ( unary, OP::reformatter_stack );

    min::locatable_gen but_not_op
        ( min::new_lab_gen ( "BUT", "NOT" ) );
    min::locatable_gen and_op
        ( min::new_str_gen ( "AND" ) );
    min::locatable_gen or_op
        ( min::new_str_gen ( "OR" ) );
    min::locatable_gen not_op
        ( min::new_str_gen ( "NOT" ) );

    OP::push_oper
        ( but_not_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  4000, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    min::locatable_var
    	    <min::packed_vec_insptr<min::gen> >
        and_arguments
	    ( min::gen_packed_vec_type.new_stub ( 1 ) );
    min::push ( and_arguments ) = and_op;

    OP::push_oper
        ( and_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  4100, infix_reformatter,
	  and_arguments,
	  oper_pass->oper_table );

    min::locatable_var
    	    <min::packed_vec_insptr<min::gen> >
        or_arguments
	    ( min::gen_packed_vec_type.new_stub ( 1 ) );
    min::push ( or_arguments ) = or_op;

    OP::push_oper
        ( or_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  4100, infix_reformatter,
	  or_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( not_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::PREFIX,
	  4200, unary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );
}

static void define_comparison_operators
	( PAR::parser parser,
	  TAB::flags code,
	  TAB::flags math )
{
    OP::oper_pass oper_pass = OP::init_oper ( parser );
    min::uns32 block_level =
        PAR::block_level ( parser );

    oper_pass->selectors |= code | math;

    min::locatable_gen infix
        ( min::new_str_gen ( "infix" ) );
    PAR::reformatter infix_reformatter =
        PAR::find_reformatter
	    ( infix, OP::reformatter_stack );

    min::locatable_gen equal_equal
        ( min::new_str_gen ( "==" ) );
    min::locatable_gen less_equal
        ( min::new_str_gen ( "<=" ) );
    min::locatable_gen greater_equal
        ( min::new_str_gen ( ">=" ) );
    min::locatable_gen not_equal
        ( min::new_str_gen ( "!=" ) );
    min::locatable_gen greater_than
        ( min::new_str_gen ( ">" ) );
    min::locatable_gen less_than
        ( min::new_str_gen ( "<" ) );

    OP::push_oper
        ( equal_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  5000, infix_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( less_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  5000, infix_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( greater_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  5000, infix_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( not_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  5000, infix_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( less_than,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  5000, infix_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( greater_than,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  5000, infix_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );
}


static void define_arithmetic_operators
	( PAR::parser parser,
	  TAB::flags code,
	  TAB::flags math )
{
    OP::oper_pass oper_pass = OP::init_oper ( parser );
    min::uns32 block_level =
        PAR::block_level ( parser );

    oper_pass->selectors |= code | math;

    min::locatable_gen binary
        ( min::new_str_gen ( "binary" ) );
    PAR::reformatter binary_reformatter =
        PAR::find_reformatter
	    ( binary, OP::reformatter_stack );
    min::locatable_gen infix
        ( min::new_str_gen ( "infix" ) );
    PAR::reformatter infix_reformatter =
        PAR::find_reformatter
	    ( infix, OP::reformatter_stack );
    min::locatable_gen unary
        ( min::new_str_gen ( "unary" ) );
    PAR::reformatter unary_reformatter =
        PAR::find_reformatter
	    ( unary, OP::reformatter_stack );

    min::locatable_gen plus_equal
        ( min::new_str_gen ( "+=" ) );
    min::locatable_gen minus_equal
        ( min::new_str_gen ( "-=" ) );
    min::locatable_gen times_equal
        ( min::new_str_gen ( "*=" ) );
    min::locatable_gen divide_equal
        ( min::new_str_gen ( "/=" ) );

    if ( code ) OP::push_oper
        ( plus_equal,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    if ( code ) OP::push_oper
        ( minus_equal,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    if ( code ) OP::push_oper
        ( times_equal,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    if ( code ) OP::push_oper
        ( divide_equal,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    min::locatable_gen plus
        ( min::new_str_gen ( "+" ) );
    min::locatable_gen minus
        ( min::new_str_gen ( "-" ) );
    min::locatable_gen divide
        ( min::new_str_gen ( "/" ) );
    min::locatable_gen multiply
        ( min::new_str_gen ( "*" ) );
    min::locatable_gen exponent
        ( min::new_str_gen ( "**" ) );
    min::locatable_var
    	    <min::packed_vec_insptr<min::gen> >
        plus_minus_arguments
	    ( min::gen_packed_vec_type.new_stub ( 2 ) );
    min::push ( plus_minus_arguments ) = plus;
    min::push ( plus_minus_arguments ) = minus;

    OP::push_oper
        ( plus,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  6000, infix_reformatter,
	  plus_minus_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( plus,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::PREFIX,
	  OP::prefix_precedence, unary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( minus,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  6000, infix_reformatter,
	  plus_minus_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( minus,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::PREFIX,
	  OP::prefix_precedence, unary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( divide,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  6100, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    min::locatable_var
    	    <min::packed_vec_insptr<min::gen> >
        multiply_arguments
	    ( min::gen_packed_vec_type.new_stub ( 1 ) );
    min::push ( multiply_arguments ) = multiply;

    OP::push_oper
        ( multiply,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  6200, infix_reformatter,
	  multiply_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( exponent,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  6300, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );
}


static void define_bitwise_operators
	( PAR::parser parser,
	  TAB::flags code,
	  TAB::flags math )
{
    if ( code == 0 ) return;

    OP::oper_pass oper_pass = OP::init_oper ( parser );
    min::uns32 block_level =
        PAR::block_level ( parser );

    oper_pass->selectors |= code;

    min::locatable_gen binary
        ( min::new_str_gen ( "binary" ) );
    PAR::reformatter binary_reformatter =
        PAR::find_reformatter
	    ( binary, OP::reformatter_stack );
    min::locatable_gen infix
        ( min::new_str_gen ( "infix" ) );
    PAR::reformatter infix_reformatter =
        PAR::find_reformatter
	    ( infix, OP::reformatter_stack );
    min::locatable_gen unary
        ( min::new_str_gen ( "unary" ) );
    PAR::reformatter unary_reformatter =
        PAR::find_reformatter
	    ( unary, OP::reformatter_stack );

    min::locatable_gen or_equal
        ( min::new_str_gen ( "|=" ) );
    min::locatable_gen and_equal
        ( min::new_str_gen ( "&=" ) );
    min::locatable_gen xor_equal
        ( min::new_str_gen ( "^=" ) );
    min::locatable_gen shift_left
        ( min::new_str_gen ( "<<=" ) );
    min::locatable_gen shift_right
        ( min::new_str_gen ( ">>=" ) );

    OP::push_oper
        ( or_equal,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( and_equal,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( xor_equal,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( shift_left,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( shift_right,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    min::locatable_gen or_name
        ( min::new_str_gen ( "|" ) );
    min::locatable_gen and_name
        ( min::new_str_gen ( "&" ) );
    min::locatable_gen xor_name
        ( min::new_str_gen ( "^" ) );
    min::locatable_gen left_shift
        ( min::new_str_gen ( "<<" ) );
    min::locatable_gen right_shift
        ( min::new_str_gen ( ">>" ) );
    min::locatable_gen complement_name
        ( min::new_str_gen ( "~" ) );

    min::locatable_var
    	    <min::packed_vec_insptr<min::gen> >
        or_arguments
	    ( min::gen_packed_vec_type.new_stub ( 1 ) );
    min::push ( or_arguments ) = or_name;

    OP::push_oper
        ( or_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  6000, infix_reformatter,
	  or_arguments,
	  oper_pass->oper_table );

    min::locatable_var
    	    <min::packed_vec_insptr<min::gen> >
        and_arguments
	    ( min::gen_packed_vec_type.new_stub ( 1 ) );
    min::push ( and_arguments ) = and_name;

    OP::push_oper
        ( and_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  6000, infix_reformatter,
	  and_arguments,
	  oper_pass->oper_table );

    min::locatable_var
    	    <min::packed_vec_insptr<min::gen> >
        xor_arguments
	    ( min::gen_packed_vec_type.new_stub ( 1 ) );
    min::push ( xor_arguments ) = xor_name;

    OP::push_oper
        ( xor_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  6000, infix_reformatter,
	  xor_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( left_shift,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  6000, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( right_shift,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  6000, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( complement_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::PREFIX,
	  OP::prefix_precedence, unary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );
}

void PARSTD::define_operators
	( PAR::parser parser, TAB::flags components )
{
    TAB::flags needed = 0;
        // Only needed selectors are defined, as
	// unneeded selectors are never referenced.

    if ( components & (   PARSTD::CONTROL_OPERATORS
                        + PARSTD::BITWISE_OPERATORS ) )
	needed |= PARSTD::CODE;
    if ( components & (   PARSTD::ASSIGNMENT_OPERATORS
                        + PARSTD::SELECTION_OPERATORS
                        + PARSTD::LOGICAL_OPERATORS
                        + PARSTD::COMPARISON_OPERATORS
                        + PARSTD::ARITHMETIC_OPERATORS )
       )
	needed |= PARSTD::CODE + PARSTD::MATH;

    TAB::flags code = 0;
    if ( needed & PARSTD::CODE )
    {
	min::locatable_gen code_name
	    ( min::new_str_gen ( "code" ) );
	code = 1ull << TAB::find_name
		  ( parser->selector_name_table,
		    code_name );
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
    if ( code == 0 && math == 0 ) return;

    if ( components & PARSTD::CONTROL_OPERATORS )
	::define_control_operators
	    ( parser, code, math );
    if ( components & PARSTD::ASSIGNMENT_OPERATORS )
	::define_assignment_operators
	    ( parser, code, math );
    if ( components & PARSTD::SELECTION_OPERATORS )
	::define_selection_operators
	    ( parser, code, math );
    if ( components & PARSTD::LOGICAL_OPERATORS )
	::define_logical_operators
	    ( parser, code, math );
    if ( components & PARSTD::COMPARISON_OPERATORS )
	::define_comparison_operators
	    ( parser, code, math );
    if ( components & PARSTD::ARITHMETIC_OPERATORS )
	::define_arithmetic_operators
	    ( parser, code, math );
    if ( components & PARSTD::BITWISE_OPERATORS )
	::define_bitwise_operators
	    ( parser, code, math );
}
