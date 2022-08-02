// Layered Languages Standard Operators Parser
//
// File:	ll_parser_standard_oper.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Aug  2 14:08:23 EDT 2022
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

    min::locatable_gen condition_arguments
	    ( min::new_obj_gen ( 2 ) );
    min::obj_vec_insptr cavp ( condition_arguments );
    min::attr_push ( cavp ) = PARLEX::colon;
    min::attr_push ( cavp ) = OPLEX::has_condition;

    min::locatable_gen else_arguments
	    ( min::new_obj_gen ( 1 ) );
    min::obj_vec_insptr eavp ( else_arguments );
    min::attr_push ( eavp ) = PARLEX::colon;

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
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( PARLEX::colon,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::POSTFIX + OP::AFIX + OP::LINE,
	  0000,
	  min::NULL_STUB,
	  min::MISSING(),
	  oper_pass->oper_bracket_table );
}

static void define_iteration_operators
	( PAR::parser parser,
	  TAB::flags code,
	  TAB::flags math )
{
    if ( code == 0 ) return;

    OP::oper_pass oper_pass = OP::init_oper ( parser );
    min::uns32 block_level =
        PAR::block_level ( parser );

    oper_pass->selectors |= code;

    min::locatable_gen unary
        ( min::new_str_gen ( "unary" ) );
    PAR::reformatter unary_reformatter =
        PAR::find_reformatter
	    ( unary, OP::reformatter_stack );
    PAR::reformatter iteration_reformatter =
        PAR::find_reformatter
	    ( OPLEX::iteration, OP::reformatter_stack );

    min::locatable_gen iteration_arguments
	    ( min::new_obj_gen ( 1 ) );
    min::obj_vec_insptr iavp ( iteration_arguments );
    min::attr_push ( iavp ) = OPLEX::times;

    min::locatable_gen do_name
        ( min::new_str_gen ( "do" ) );
    min::locatable_gen while_name
        ( min::new_str_gen ( "while" ) );
    min::locatable_gen until_name
        ( min::new_str_gen ( "until" ) );
    min::locatable_gen repeat
        ( min::new_str_gen ( "repeat" ) );
    min::locatable_gen at_most
        ( min::new_lab_gen ( "at", "most" ) );

    OP::push_oper
        ( do_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::PREFIX + OP::LINE,
	  3000,
	  unary_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( while_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::PREFIX + OP::LINE,
	  3000,
	  unary_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( until_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::PREFIX + OP::LINE,
	  3000,
	  unary_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( repeat,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::PREFIX + OP::LINE,
	  3000,
	  iteration_reformatter,
	  iteration_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( at_most,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::PREFIX + OP::LINE,
	  3000,
	  iteration_reformatter,
	  iteration_arguments,
	  oper_pass->oper_table );


    OP::push_oper
        ( OPLEX::times,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::AFIX + OP::LINE,
	  3000,
	  min::NULL_STUB,
	  min::MISSING(),
	  oper_pass->oper_table );
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

    min::locatable_gen assignment
        ( min::new_str_gen ( "assignment" ) );
    PAR::reformatter assignment_reformatter =
        PAR::find_reformatter
	    ( assignment, OP::reformatter_stack );
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

    min::locatable_gen assignment_arguments
	    ( min::new_obj_gen ( 1 ) );
    min::obj_vec_insptr cavp ( assignment_arguments );
    min::attr_push ( cavp ) = PARLEX::colon;

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
	  min::MISSING(),
	  oper_pass->oper_table );

    if ( code ) OP::push_oper
        ( equal,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000,
	  assignment_reformatter,
	  assignment_arguments,
	  oper_pass->oper_table );

    if ( code ) OP::push_oper
        ( PARLEX::colon,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::AFIX + OP::LINE,
	  1000,
	  min::NULL_STUB,
	  min::MISSING(),
	  oper_pass->oper_bracket_table );

    if ( code ) OP::push_oper
        ( PARLEX::colon,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::POSTFIX + OP::LINE,
	  1000,
	  min::NULL_STUB,
	  min::MISSING(),
	  oper_pass->oper_bracket_table );

    min::locatable_gen comma
        ( min::new_str_gen ( "," ) );

    OP::push_oper
        ( comma,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::NOFIX,
	  2000, separator_reformatter,
	  min::MISSING(),
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

    min::locatable_gen selector_arguments
	    ( min::new_obj_gen ( 2 ) );
    min::obj_vec_insptr savp ( selector_arguments );
    min::attr_push ( savp ) = if_op;
    min::attr_push ( savp ) = else_op;

    OP::push_oper
        ( if_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  10000, selector_reformatter,
	  selector_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( else_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::AFIX + OP::INFIX,
	  10000,
	  min::NULL_STUB,
	  min::MISSING(),
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
	  11000, binary_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    min::locatable_gen and_arguments
	    ( min::new_obj_gen ( 1 ) );
    min::obj_vec_insptr aavp ( and_arguments );
    min::attr_push ( aavp ) = and_op;

    OP::push_oper
        ( and_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  11100, infix_reformatter,
	  and_arguments,
	  oper_pass->oper_table );

    min::locatable_gen or_arguments
	    ( min::new_obj_gen ( 1 ) );
    min::obj_vec_insptr oavp ( or_arguments );
    min::attr_push ( oavp ) = or_op;

    OP::push_oper
        ( or_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  11100, infix_reformatter,
	  or_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( not_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::PREFIX,
	  11200, unary_reformatter,
	  min::MISSING(),
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
	  12000, infix_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( less_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  12000, infix_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( greater_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  12000, infix_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( not_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  12000, infix_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( less_than,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  12000, infix_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( greater_than,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  12000, infix_reformatter,
	  min::MISSING(),
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
	  min::MISSING(),
	  oper_pass->oper_table );

    if ( code ) OP::push_oper
        ( minus_equal,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000, binary_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    if ( code ) OP::push_oper
        ( times_equal,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000, binary_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    if ( code ) OP::push_oper
        ( divide_equal,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000, binary_reformatter,
	  min::MISSING(),
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
    min::locatable_gen plus_minus_arguments
	    ( min::new_obj_gen ( 2 ) );
    min::obj_vec_insptr pmavp ( plus_minus_arguments );
    min::attr_push ( pmavp ) = plus;
    min::attr_push ( pmavp ) = minus;

    OP::push_oper
        ( plus,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  13000, infix_reformatter,
	  plus_minus_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( plus,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::PREFIX,
	  OP::prefix_precedence, unary_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( minus,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  13000, infix_reformatter,
	  plus_minus_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( minus,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::PREFIX,
	  OP::prefix_precedence, unary_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( divide,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  13100, binary_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    min::locatable_gen multiply_arguments
	    ( min::new_obj_gen ( 1 ) );
    min::obj_vec_insptr mavp ( multiply_arguments );
    min::attr_push ( mavp ) = multiply;

    OP::push_oper
        ( multiply,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  13200, infix_reformatter,
	  multiply_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( exponent,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  13300, binary_reformatter,
	  min::MISSING(),
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
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( and_equal,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000, binary_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( xor_equal,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000, binary_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( shift_left,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000, binary_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( shift_right,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX + OP::LINE,
	  1000, binary_reformatter,
	  min::MISSING(),
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

    min::locatable_gen or_arguments
	    ( min::new_obj_gen ( 1 ) );
    min::obj_vec_insptr oavp ( or_arguments );
    min::attr_push ( oavp ) = or_name;

    OP::push_oper
        ( or_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  13000, infix_reformatter,
	  or_arguments,
	  oper_pass->oper_table );

    min::locatable_gen and_arguments
	    ( min::new_obj_gen ( 1 ) );
    min::obj_vec_insptr aavp ( and_arguments );
    min::attr_push ( aavp ) = and_name;

    OP::push_oper
        ( and_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  13000, infix_reformatter,
	  and_arguments,
	  oper_pass->oper_table );

    min::locatable_gen xor_arguments
	    ( min::new_obj_gen ( 1 ) );
    min::obj_vec_insptr xavp ( xor_arguments );
    min::attr_push ( xavp ) = xor_name;

    OP::push_oper
        ( xor_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  13000, infix_reformatter,
	  xor_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( left_shift,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  13000, binary_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( right_shift,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  13000, binary_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );

    OP::push_oper
        ( complement_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::PREFIX,
	  OP::prefix_precedence, unary_reformatter,
	  min::MISSING(),
	  oper_pass->oper_table );
}

void PARSTD::define_operators
	( PAR::parser parser, TAB::flags components )
{
    TAB::flags needed = 0;
        // Only needed selectors listed in components
	// are defined, as unneeded selectors are never
	// referenced.

    if ( components & (   PARSTD::CONTROL_OPERATORS
                        + PARSTD::ITERATION_OPERATORS
                        + PARSTD::BITWISE_OPERATORS ) )
	needed |= PARSTD::CODE;
    if ( components & (   PARSTD::ASSIGNMENT_OPERATORS
                        + PARSTD::SELECTION_OPERATORS
                        + PARSTD::LOGICAL_OPERATORS
                        + PARSTD::COMPARISON_OPERATORS
                        + PARSTD::ARITHMETIC_OPERATORS )
       )
	needed |= PARSTD::CODE + PARSTD::MATH;

    needed &= components;

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
    if ( components & PARSTD::ITERATION_OPERATORS )
	::define_iteration_operators
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
