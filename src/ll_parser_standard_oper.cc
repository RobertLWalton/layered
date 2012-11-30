// Layers Language Standard Operators Parser
//
// File:	ll_parser_standard_oper.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Nov 30 05:34:05 EST 2012
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
        OP::place ( parser, next );

    min::locatable_gen code_name
        ( min::new_str_gen ( "code" ) );
    min::locatable_gen math_name
        ( min::new_str_gen ( "math" ) );

    TAB::flags code =
        1ull << TAB::find_name
	    ( parser->selector_name_table, code_name );
    TAB::flags math =
        1ull << TAB::find_name
	    ( parser->selector_name_table, math_name );

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

    min::locatable_gen compare
        ( min::new_str_gen ( "compare" ) );
    min::locatable_gen right_associative
        ( min::new_lab_gen ( "right", "associative" ) );
    min::locatable_gen separator
        ( min::new_str_gen ( "separator" ) );

    min::uns32 block_level =
        PAR::block_level ( parser );
    min::uns32 forget;
    OP::push_oper
        ( comma,
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::NOFIX,
	  4000,
	  OP::find_reformatter
	      ( separator, forget ),
	  oper_pass->oper_table );

    OP::push_oper
        ( equal,
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  9000,
	  OP::find_reformatter
	      ( right_associative, forget ),
	  oper_pass->oper_table );

    OP::push_oper
        ( plus_equal,
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  9000,
	  OP::find_reformatter
	      ( right_associative, forget ),
	  oper_pass->oper_table );

    OP::push_oper
        ( minus_equal,
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  9000,
	  OP::find_reformatter
	      ( right_associative, forget ),
	  oper_pass->oper_table );

    OP::push_oper
        ( times_equal,
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  9000,
	  OP::find_reformatter
	      ( right_associative, forget ),
	  oper_pass->oper_table );

    OP::push_oper
        ( divide_equal,
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  9000,
	  OP::find_reformatter
	      ( right_associative, forget ),
	  oper_pass->oper_table );

    OP::push_oper
        ( equal_equal,
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  15000,
	  OP::find_reformatter
	      ( compare, forget ),
	  oper_pass->oper_table );

    OP::push_oper
        ( less_equal,
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  15000,
	  OP::find_reformatter
	      ( compare, forget ),
	  oper_pass->oper_table );

    OP::push_oper
        ( greater_equal,
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  15000,
	  OP::find_reformatter
	      ( compare, forget ),
	  oper_pass->oper_table );

    OP::push_oper
        ( not_equal,
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  15000,
	  OP::find_reformatter
	      ( compare, forget ),
	  oper_pass->oper_table );

    OP::push_oper
        ( less_than,
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  15000,
	  OP::find_reformatter
	      ( compare, forget ),
	  oper_pass->oper_table );

    OP::push_oper
        ( greater_than,
	  code + math,
	  block_level, PAR::top_level_position,
	  OP::INFIX,
	  15000,
	  OP::find_reformatter
	      ( compare, forget ),
	  oper_pass->oper_table );

    return oper_pass;
}
