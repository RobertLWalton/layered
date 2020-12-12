// Layered Languages Standard Operators Parser
//
// File:	ll_parser_standard_oper.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Dec 12 01:50:53 EST 2020
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
# define PARSTD ll::parser::standard
# define TAB ll::parser::table
# define OP ll::parser::oper

// Standard Operators
// -------- ---------

OP::oper_pass PARSTD::init_oper
	( PAR::parser parser,
	  PAR::pass next )
{
    OP::oper_pass oper_pass =
        (OP::oper_pass) OP::new_pass ( parser );

    PAR::place_before ( parser, (PAR::pass) oper_pass );

    min::locatable_gen code_name
        ( min::new_str_gen ( "code" ) );
    min::locatable_gen math_name
        ( min::new_str_gen ( "math" ) );
    min::locatable_gen data_name
        ( min::new_str_gen ( "data" ) );

    TAB::flags code =
        1ull << TAB::find_name
	    ( parser->selector_name_table, code_name );
    TAB::flags math =
        1ull << TAB::find_name
	    ( parser->selector_name_table, math_name );
    TAB::flags data =
        1ull << TAB::find_name
	    ( parser->selector_name_table, data_name );

    oper_pass->selectors = code | math | data;

    min::locatable_gen declare
        ( min::new_str_gen ( "declare" ) );
    min::locatable_gen right_associative
        ( min::new_lab_gen ( "right", "associative" ) );
    min::locatable_gen separator
        ( min::new_str_gen ( "separator" ) );
    min::locatable_gen infix
        ( min::new_str_gen ( "infix" ) );
    min::locatable_gen unary
        ( min::new_str_gen ( "unary" ) );
    min::locatable_gen infix_and
        ( min::new_lab_gen ( "infix", "and" ) );
    min::locatable_gen sum
        ( min::new_str_gen ( "sum" ) );
    min::locatable_gen binary
        ( min::new_str_gen ( "binary" ) );

    PAR::reformatter declare_reformatter =
        PAR::find_reformatter
	    ( declare, OP::reformatter_stack );
    PAR::reformatter right_associative_reformatter =
        PAR::find_reformatter
	    ( right_associative,
	      OP::reformatter_stack );
    PAR::reformatter separator_reformatter =
        PAR::find_reformatter
	    ( separator, OP::reformatter_stack );
    PAR::reformatter infix_reformatter =
        PAR::find_reformatter
	    ( infix, OP::reformatter_stack );
    PAR::reformatter unary_reformatter =
        PAR::find_reformatter
	    ( unary, OP::reformatter_stack );
    PAR::reformatter infix_and_reformatter =
        PAR::find_reformatter
	    ( infix_and, OP::reformatter_stack );
    PAR::reformatter sum_reformatter =
        PAR::find_reformatter
	    ( sum, OP::reformatter_stack );
    PAR::reformatter binary_reformatter =
        PAR::find_reformatter
	    ( binary, OP::reformatter_stack );

    min::uns32 block_level =
        PAR::block_level ( parser );

    min::locatable_gen arrow
        ( min::new_str_gen ( "<--" ) );
    min::locatable_gen colon
        ( min::new_str_gen ( ":" ) );
    min::locatable_gen opening_brace_dollar
        ( min::new_lab_gen ( "{", "$" ) );
    min::locatable_gen dollar_closing_brace
        ( min::new_lab_gen ( "$", "}" ) );

    OP::push_oper
        ( arrow,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::NOFIX,
	  0, declare_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( colon,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::NOFIX + OP::AFIX,
	  0,
	  min::NULL_STUB, min::NULL_STUB,
	  oper_pass->oper_bracket_table );

    OP::push_oper
        ( opening_brace_dollar,
	  dollar_closing_brace,
	  code,
	  block_level, PAR::top_level_position,
	  OP::NOFIX + OP::AFIX,
	  0,
	  min::NULL_STUB, min::NULL_STUB,
	  oper_pass->oper_bracket_table );

    min::locatable_gen equal
        ( min::new_str_gen ( "=" ) );
    min::locatable_gen plus_equal
        ( min::new_str_gen ( "+=" ) );
    min::locatable_gen minus_equal
        ( min::new_str_gen ( "-=" ) );
    min::locatable_gen times_equal
        ( min::new_str_gen ( "*=" ) );
    min::locatable_gen divide_equal
        ( min::new_str_gen ( "/=" ) );

    OP::push_oper
        ( equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  1000,
	  PAR::find_reformatter
	      ( right_associative,
	        OP::reformatter_stack ),
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( plus_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  1000, right_associative_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( minus_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  1000, right_associative_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( times_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  1000, right_associative_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( divide_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  1000, right_associative_reformatter,
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
	  OP::NOFIX,
	  3000, binary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( and_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::NOFIX,
	  3100, infix_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( or_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::NOFIX,
	  3100, infix_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( not_op,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::NOFIX,
	  3100, unary_reformatter,
	  min::NULL_STUB,
	  oper_pass->oper_table );

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
	  OP::PREFIX + OP::INFIX,
	  5000, sum_reformatter,
	  sum_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( minus,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::PREFIX + OP::INFIX,
	  5000, sum_reformatter,
	  sum_arguments,
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

    return oper_pass;
}
