// Layers Language Operator Parser Pass
//
// File:	ll_parser_operator.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Mar 18 05:31:34 EDT 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Operator Parser Pass

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser_oper.h>
# define MUP min::unprotected
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table
# define OPER ll::parser::oper

min::locatable_gen OPER::oper;

static void initialize ( void )
{
    PAR::oper
	= min::new_dot_lab_gen ( "operator" );
}
static min::initializer initializer ( ::initialize );

// Operator Parser Pass
// -------- ------ ----

struct oper_stack_struct
{
    // Save of various operator parser variables.
    // None of these saved values are ACC locatable.
    //
    min::int32 precedence;
    PAR::token first;
};

typedef min::packed_vec_insptr<oper_stack_stuct>
    oper_stack;

static min::packed_vec<oper_stack_struct>
    oper_stack_type
        ( "(ll_parser_oper.cc)::oper_stack_type" );

struct oper_pass_struct : public PAR::pass_struct
{
    const ::oper_stack oper_stack;
};

typedef min::packed_struct_updptr<oper_pass_stuct>
    oper_pass;

MIN_REF ( PAR::pass, next, ::oper_pass );
MIN_REF ( ::oper_stack, oper_stack, ::oper_pass );

static min::uns32 oper_pass_stub_disp[] =
{
    min::DISP ( & ::oper_pass_struct::next ),
    min::DISP ( & ::oper_pass_struct::oper_stack ),
    min::DISP_END
};

static min::packed_struct_with_base
	<oper_stack_struct,PAR::pass_struct>
    oper_pass_type
        ( "(ll_parser_oper.cc)::oper_pass_type",
	  NULL, oper_pass_stub_disp );

static oper_pass_run ( PAR::parser parser,
		       PAR::pass pass,
		       PAR::token & first,
		       PAR::token end )
{
    ::oper_pass oper_pass = pass;
    ::oper_stack oper_stack = oper_pass->oper_stack;

    if ( oper_stack == min::NULL_STUB )
        oper_stack = ::oper_stack_ref ( oper_pass ) =
	    ::oper_stack_type.new_gen ( 100 );
    else
        min::pop ( oper_stack, oper_stack->length );

    // Data that is pushed to oper_stack.
    //
    ::oper_stack_struct D;
    D.first = first;

    PAR::token current = first;
    while ( current != end )
    {
    }



}
