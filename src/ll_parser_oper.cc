// Layers Language Operator Parser Pass
//
// File:	ll_parser_operator.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Apr  1 05:00:04 EDT 2012
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
    TAB::oper primary_oper;
    PAR::token primary_oper_token;
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
    D.precedence = OP::MIN_PRECEDENCE;
    D.primary_oper = min::NULL_STUB;
    D.primary_oper_token = min::NULL_STUB;

    enum // Preceeding part of expression is:
    {
	EMPTY			= ( 1 << 0 ),
	    // Empty
	OPERAND			= ( 1 << 1 ),
	    // Non-operator, non-empty
        POSTFIX		 	= ( 1 << 2 ),
	    // Subexpression terminated by postfix
	    // operator
	PREFIX			= ( 1 << 3 ),
	    // Prefix operator
	INFIX			= ( 1 << 4 ),
	    // Infix operator
	NOFIX			= ( 1 << 5 ),
	    // Nofix operator
    } state = EMPTY;

    PAR::token current = D.first;
    while ( current != next )
    {
	PAR::token oper_first = current;

	// Find operator if possible.
	// * Unselected operators do not qualify.
	// * AFIX operators only qualify if they have
	//   a precedence matching a precedence in
	//   oper_stack.
	// * PREFIX operators qualify only if
	//   oper_first == D.first.
	// * INFIX and POSTFIX operators qualify only if
	//   oper_first != D.first.
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
	while ( root != NULL_STUB
		&&
		( oper == NULL_STUB
		  ||	     
		  ( oper->flags & OP::AFIX
		    &&
		    ( check_precedence
		          ( oper->precedence,
			    oper_stack ) ) )
		  ||
		  ( oper->flags & OP::PREFIX
		    &&
		    oper_first != D.first )
		  ||
		  ( oper->flags & (   OP::INFIX
		                    | OP::POSTFIX )
		    &&
		    oper_first == D.first ) ) )
	{
	    root = PAR::find_next_entry
		( parser, current, key_prefix,
		      selectors, root );
	    oper = (OP::oper) root;
	}

	if ( oper == NULL_STUB )
	{
	    if ( state & POSTFIX )
	    {
	        // TBD: insert operator
	    }
	    else
	    {
		current = current->next;
		state = OPERAND;
		continue;
	    }
	}
	else if ( state & ( PREFIX | INFIX ) )
	{
	    // TDB: insert operand
	}

	// Make operator token.
	//
	while ( oper_first != current->previous )
	    PAR::free
		( PAR::remove
		      ( PAR::first_ref(parser),
			current->previous ) );

	oper_first->type = PAR::OPERATOR;
	oper_first->value = oper->label;

	while ( ( oper->precedence <= D.precedence
	          &&
		  ( oper->oper_flags & OP::PREFIX ) == 0 )
	{
	    if ( D.primary_oper != min::NULL_STUB )
		D.primary_oper->reformat
		    ( parser, pass->next,
		      D.primary_oper,
		      D.first, oper_first,
		      D.primary_oper_token );
	    else
	        OP::default_reformat
		    ( parser, pass->next,
		      D.primary_oper,
		      D.first, oper_first,
		      D.primary_oper_token );
	    D = oper_stack.pop();
	}

	if ( oper_first == D.first )
	{
	    // Prefix or nofix operator.
	    //
	    assert ( oper->flags & (   OP::PREFIX
		                     | OP::NOFIX ) );
	    D.primary_oper = oper;
	    D.primary_oper_token = oper_first;
	    oper_stack.push() = D;
	    D.first = current;
	    D.precedence = oper->precedence;
	    D.primary_oper = NULL_STUB;
	    D.primary_oper_token = NULL_STUB;
	}
	else
	{
	    // Postfix, nofix, or infix operator.
	    //
	    assert ( oper->flags & (   OP::POSTFIX
		                     | OP::NOFIX
		                     | OP::INFIX ) );
	}
    }
}
