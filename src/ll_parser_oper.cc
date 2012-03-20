// Layers Language Operator Parser Pass
//
// File:	ll_parser_operator.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Mar 20 04:22:29 EDT 2012
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
# define OP ll::parser::oper

min::locatable_gen OP::oper;

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
    bool operator_required;
        // True first token is a postfix operator
	// terminated expression, so that the next
	// thing in the input must be an operator.
};

typedef min::packed_vec_insptr<oper_stack_stuct>
    oper_stack;

static min::packed_vec<oper_stack_struct>
    oper_stack_type
        ( "(ll_parser_oper.cc)::oper_stack_type" );

struct oper_pass_struct : public PAR::pass_struct
{
    const TAB::table oper_table;
    const ::oper_stack oper_stack;
};

typedef min::packed_struct_updptr<oper_pass_stuct>
    oper_pass;

MIN_REF ( PAR::pass, next, ::oper_pass );
MIN_REF ( TAB::table, oper_table, ::oper_pass );
MIN_REF ( ::oper_stack, oper_stack, ::oper_pass );

static min::uns32 oper_pass_stub_disp[] =
{
    min::DISP ( & ::oper_pass_struct::next ),
    min::DISP ( & ::oper_pass_struct::oper_table ),
    min::DISP ( & ::oper_pass_struct::oper_stack ),
    min::DISP_END
};

// Return true iff the argument is a precedence in the
// oper_stack.
//
inline bool check_precedence
	( int precedence,
	  OP::oper_stack oper_stack )
{
    for ( min::uns32 i = 0; i < oper_stack->length; ++ )
    {
        if ( oper_stack[i].precedence == precedence )
	    return true;
    }
    return false;
}

static min::packed_struct_with_base
	<oper_stack_struct,PAR::pass_struct>
    oper_pass_type
        ( "(ll_parser_oper.cc)::oper_pass_type",
	  NULL, oper_pass_stub_disp );

static oper_pass_run ( PAR::parser parser,
		       PAR::pass pass,
		       PAR::token & first,
		       PAR::token next,
		       PAR::selectors selectors )
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
    D.precedence = OP::NO_PRECEDENCE;
    D.operator_required = false;

    PAR::token current = first;
    while ( current != next )
    {
	PAR::token oper_first = current;

	// Find operator if possible.
	// * Unselected operator do not qualify.
	// * AFIX operators only qualify if they have
	//   a precedence matching a precedence in
	//   oper_stack.
	// * PREFIX operators qualify only if
	//   oper_first == first.
	// * INFIX and POSTFIX operators qualify only if
	//   oper_first != first.
	// After rejection operators deeper in the stack
	// are tried, and then shorter operators are
	// tried.
	//
	TAB::key_prefix key_prefix;
	TAB::root root = PAR::find_entry
	    ( parser, current, key_prefix,
	      selectors, oper_pass->oper_table,
	      next );
	OP::oper oper = (OP::oper) root;
	TAB::root next_root = root;
	while ( next_root != NULL_STUB
		&&
		( oper == NULL_STUB
		  ||	     
		  ( oper->flags & OP::AFIX
		    &&
		    ( ! operator_found
		      ||
		      check_precedence
		          ( oper->precedence,
			    oper_stack ) ) )
		  ||
		  ( oper->flags & OP::PREFIX
		    &&
		    oper_first != first )
		  ||
		  ( oper->flags & (   OP::INFIX
		                    | OP::POSTFIX )
		    &&
		    oper_first == first ) ) )
	{
	    next_root = next_root->next;
	    oper = (OP::oper) next_root;
	    if ( next_root != NULL_STUB ) continue;
	    root = PAR::find_next_entry
		( parser, current, key_prefix,
		      selectors, root );
	    next_root = root;
	    oper = (OP::oper) next_root;
	}

	if ( oper == NULL_STUB )
	{
	    if ( D.oper_required )
	    {
	    }

	    current = current->next;
	    continue;
	}

	if ( oper_first == first )
	{
	    // Prefix, nofix, or afix operator.
	    //
	    assert ( oper->flags & (   OP::PREFIX
		                     | OP::NOFIX
		                     | OP::AFIX ) );
	}
	else
	{
	    // Postfix, nofix, afix, or infix operator.
	    //
	    assert ( oper->flags & (   OP::POSTFIX
		                     | OP::NOFIX
		                     | OP::AFIX
		                     | OP::INFIX ) );
	}
    }
}
