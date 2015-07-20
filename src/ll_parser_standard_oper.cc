// Layered Languages Standard Operators Parser
//
// File:	ll_parser_standard_oper.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jul 20 15:51:02 EDT 2015
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
// -------- --------_

OP::oper_pass PARSTD::init_oper
	( PAR::parser parser,
	  PAR::pass next )
{
    OP::oper_pass oper_pass =
        (OP::oper_pass) OP::new_pass();

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

    min::locatable_gen comma
        ( min::new_str_gen ( "," ) );

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
        separator_arguments
	    ( min::gen_packed_vec_type.new_stub ( 1 ) );
    min::push ( separator_arguments ) = comma;
    min::locatable_var
    	    <min::packed_vec_insptr<min::gen> >
        and_arguments
	    ( min::gen_packed_vec_type.new_stub ( 1 ) );
    min::push ( and_arguments ) = and_op;

    min::locatable_gen infix_and
        ( min::new_lab_gen ( "infix", "and" ) );
    min::locatable_gen right_associative
        ( min::new_lab_gen ( "right", "associative" ) );
    min::locatable_gen separator
        ( min::new_str_gen ( "separator" ) );

    min::uns32 block_level =
        PAR::block_level ( parser );
    OP::push_oper
        ( comma,
	  min::MISSING(),
	  code + math + data,
	  block_level, PAR::top_level_position,
	  OP::NOFIX,
	  4000,
	  PAR::find_reformatter
	      ( separator,
	        OP::reformatter_stack ),
	  separator_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  9000,
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
	  9000,
	  PAR::find_reformatter
	      ( right_associative,
	        OP::reformatter_stack ),
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( minus_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  9000,
	  PAR::find_reformatter
	      ( right_associative,
	        OP::reformatter_stack ),
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( times_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  9000,
	  PAR::find_reformatter
	      ( right_associative,
	        OP::reformatter_stack ),
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( divide_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  9000,
	  PAR::find_reformatter
	      ( right_associative,
	        OP::reformatter_stack ),
	  min::NULL_STUB,
	  oper_pass->oper_table );

    OP::push_oper
        ( equal_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  15000,
	  PAR::find_reformatter
	      ( infix_and,
	        OP::reformatter_stack ),
	  and_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( less_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  15000,
	  PAR::find_reformatter
	      ( infix_and,
	        OP::reformatter_stack ),
	  and_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( greater_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  15000,
	  PAR::find_reformatter
	      ( infix_and,
	        OP::reformatter_stack ),
	  and_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( not_equal,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  15000,
	  PAR::find_reformatter
	      ( infix_and,
	        OP::reformatter_stack ),
	  and_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( less_than,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  15000,
	  PAR::find_reformatter
	      ( infix_and,
	        OP::reformatter_stack ),
	  and_arguments,
	  oper_pass->oper_table );

    OP::push_oper
        ( greater_than,
	  min::MISSING(),
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  15000,
	  PAR::find_reformatter
	      ( infix_and,
	        OP::reformatter_stack ),
	  and_arguments,
	  oper_pass->oper_table );

    return oper_pass;
}
