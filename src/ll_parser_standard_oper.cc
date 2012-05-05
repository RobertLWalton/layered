// Layers Language Standard Operators Parser
//
// File:	ll_parser_standard_oper.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat May  5 04:12:41 EDT 2012
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

    OP::oper_pass oper_pass =
        OP::place ( parser, next );

    OP::push_oper
        ( comma,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  OP::NOFIX,
	  1000,
	  OP::separator_reformatter,
	  oper_pass->oper_table );

    OP::push_oper
        ( equal,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  OP::INFIX,
	  900,
	  OP::right_associative_reformatter,
	  oper_pass->oper_table );

    OP::push_oper
        ( plus_equal,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  OP::INFIX,
	  900,
	  OP::right_associative_reformatter,
	  oper_pass->oper_table );

    OP::push_oper
        ( minus_equal,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  OP::INFIX,
	  900,
	  OP::right_associative_reformatter,
	  oper_pass->oper_table );

    OP::push_oper
        ( times_equal,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  OP::INFIX,
	  900,
	  OP::right_associative_reformatter,
	  oper_pass->oper_table );

    OP::push_oper
        ( divide_equal,
	  PARSTD::CODE + PARSTD::MATH + PARSTD::TEXT,
	  OP::INFIX,
	  900,
	  OP::right_associative_reformatter,
	  oper_pass->oper_table );

    return oper_pass;
}
