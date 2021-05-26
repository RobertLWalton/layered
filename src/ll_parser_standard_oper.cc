// Layered Languages Standard Operators Parser
//
// File:	ll_parser_standard_oper.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed May 26 12:38:35 EDT 2021
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
	  OP::AFIX + OP::LINE,
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
	  3000, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( and_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  3100, infix_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( or_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  3100, infix_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( not_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::PREFIX,
	  3200, unary_reformatter,
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

    min::locatable_gen infix_and
        ( min::new_lab_gen ( "infix", "and" ) );
    PAR::reformatter infix_and_reformatter =
        PAR::find_reformatter
	    ( infix_and, OP::reformatter_stack );

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
    min::locatable_gen and_op
        ( min::new_str_gen ( "AND" ) );
    min::locatable_var
    	    <min::packed_vec_insptr<min::gen> >
        and_arguments
	    ( min::gen_packed_vec_type.new_stub ( 1 ) );
    min::push ( and_arguments ) = and_op;

    OP::push_oper
        ( equal_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  4000, infix_and_reformatter,
	  and_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( less_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  4000, infix_and_reformatter,
	  and_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( greater_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  4000, infix_and_reformatter,
	  and_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( not_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  4000, infix_and_reformatter,
	  and_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( less_than,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  4000, infix_and_reformatter,
	  and_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( greater_than,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  4000, infix_and_reformatter,
	  and_arguments,
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
    min::locatable_gen sum
        ( min::new_str_gen ( "sum" ) );
    PAR::reformatter sum_reformatter =
        PAR::find_reformatter
	    ( sum, OP::reformatter_stack );

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
        ( min::new_str_gen ( "^" ) );
    min::locatable_var
    	    <min::packed_vec_insptr<min::gen> >
        sum_arguments
	    ( min::gen_packed_vec_type.new_stub ( 2 ) );
    min::push ( sum_arguments ) = plus;
    min::push ( sum_arguments ) = minus;

    OP::push_oper
        ( plus,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  5000, sum_reformatter,
	  sum_arguments,
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
	  5000, sum_reformatter,
	  sum_arguments,
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
	  5100, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( multiply,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  5200, infix_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( exponent,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  5300, binary_reformatter,
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
        ( min::new_str_gen ( "~=" ) );

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

    min::locatable_gen or_name
        ( min::new_str_gen ( "|" ) );
    min::locatable_gen and_name
        ( min::new_str_gen ( "&" ) );
    min::locatable_gen xor_name
        ( min::new_str_gen ( "~" ) );

    OP::push_oper
        ( or_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  5000, infix_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( and_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  5000, infix_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( xor_name,
	  min::MISSING(),
	  code,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  5000, infix_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( xor_name,
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
    if ( ( components & PARSTD::ALL_OPERATORS ) == 0 )
        return;   // Don't compute selectors.

    TAB::flags code = 0;
    if ( components & PARSTD::CODE )
    {
	min::locatable_gen code_name
	    ( min::new_str_gen ( "code" ) );
	code = 1ull << TAB::find_name
		  ( parser->selector_name_table,
		    code_name );
    }
    TAB::flags math = 0;
    if ( components & PARSTD::MATH )
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
