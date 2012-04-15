// Layers Language Operator Parser Pass
//
// File:	ll_parser_oper.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Apr 15 16:26:58 EDT 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Operator Table Entries
//	Operator Parser Pass

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser_oper.h>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table
# define OP ll::parser::oper

static min::locatable_gen oper_lexeme;

static void initialize ( void )
{
    ::oper_lexeme
	= min::new_dot_lab_gen ( "operator" );
}
static min::initializer initializer ( ::initialize );

// Operator Table Entries
// -------- ----- -------


static min::uns32 oper_gen_disp[] = {
    min::DISP ( & TAB::root_struct::label ),
    min::DISP_END };

static min::uns32 oper_stub_disp[] = {
    min::DISP ( & OP::oper_struct::next ),
    min::DISP_END };

static min::packed_struct_with_base
	<OP::oper_struct, TAB::root_struct>
    oper_type ( "ll::parser::table::oper_type",
	        ::oper_gen_disp,
	        ::oper_stub_disp );
const min::uns32 & OP::OPER = oper_type.subtype;

void OP::push_oper
	( min::gen oper_label,
	  TAB::selectors selectors,
	  min::uns32 flags,
	  min::int32 precedence,
	  OP::reformatter reformatter,
	  TAB::table oper_table )
{
    min::locatable_var<OP::oper> oper
        ( ::oper_type.new_stub() );

    label_ref(oper) = oper_label;
    oper->selectors = selectors;
    oper->flags = flags;
    oper->precedence = precedence;
    oper->reformatter = reformatter;

    TAB::push ( oper_table, (TAB::root) oper );
}

// Operator Parser Pass
// -------- ------ ----

struct oper_stack_struct
{
    // Save of various operator parser variables.
    // None of these saved values are ACC locatable.
    //
    min::int32 precedence;
    PAR::token first;
    OP::oper first_oper;
        // First OPERATOR in subexpression, or NULL_
	// STUB if none yet.
};

typedef min::packed_vec_insptr< ::oper_stack_struct >
    oper_stack;

static min::packed_vec< ::oper_stack_struct >
    oper_stack_type
        ( "(ll_parser_oper.cc)::oper_stack_type" );

struct oper_pass_struct : public PAR::pass_struct
{
    const TAB::table oper_table;
    const ::oper_stack oper_stack;
};

typedef min::packed_struct_updptr<oper_pass_struct>
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
	  ::oper_stack oper_stack )
{
    for ( min::uns32 i = 0; i < oper_stack->length;
    			    ++ i )
    {
        if ( oper_stack[i].precedence == precedence )
	    return true;
    }
    return false;
}

static min::packed_struct_with_base
	< ::oper_pass_struct, PAR::pass_struct >
    oper_pass_type
        ( "(ll_parser_oper.cc)::oper_pass_type",
	  NULL, oper_pass_stub_disp );

static bool oper_pass_run ( PAR::parser parser,
		            PAR::pass pass,
		            TAB::selectors selectors,
		            PAR::token & first,
		            PAR::token next )
{
    ::oper_pass oper_pass = (::oper_pass) pass;
    ::oper_stack oper_stack = oper_pass->oper_stack;

    if ( oper_stack == min::NULL_STUB )
        oper_stack = ::oper_stack_ref ( oper_pass ) =
	    ::oper_stack_type.new_gen ( 100 );

    // We add to the stack but leave alone what is
    // already in the stack so this function can be
    // called recursively.
    //
    min::unsptr initial_length = oper_stack->length;

    // Data that is pushed to oper_stack.  D is in
    // effect the top of the stack.
    //
    ::oper_stack_struct D;
    D.first = first;
    D.precedence = OP::NO_PRECEDENCE;
    D.first_oper = min::NULL_STUB;

    min::uns32 last_oper_flags = 0;
        // Flags of the last operator seen in the
	// expression.

    PAR::token current = D.first;
    while ( true )
    {

	// Find operator if possible.
	//
	PAR::token next_current = current;
	TAB::key_prefix key_prefix;
	TAB::root root = PAR::find_entry
	    ( parser, next_current, key_prefix,
	      selectors, oper_pass->oper_table,
	      next );
	    // If current == next at beginning of loop,
	    // then find_entry will return NULL_STUB
	    // and leave next_current == current.
	OP::oper oper = (OP::oper) root;
	while ( root != min::NULL_STUB
		&&
		( oper == min::NULL_STUB
		  ||	     
		  ( oper->flags & OP::PREFIX
		    &&
		    ( current != D.first
		      ||
		      oper->precedence < D.precedence
		      ||
		      ( oper->precedence == D.precedence
		        &&
			   (   last_oper_flags
			     & OP::PREFIX )
			== 0 )
		    )
		  )
		  ||
		  ( oper->flags & OP::INFIX
		    &&
		    current == D.first
		    &&
		    oper->precedence >= D.precedence
		  )
		  ||
		  ( oper->flags & OP::POSTFIX
		    &&
		    current == D.first
		    &&
		    ( oper->precedence > D.precedence
		      ||
		      ( oper->precedence == D.precedence
		        &&
			   (   last_oper_flags
			     & OP::POSTFIX )
			== 0 )
		    )
		  )
		  ||
		  ( oper->flags & OP::AFIX
		    &&
		    ! check_precedence
		          ( oper->precedence,
			    oper_stack ) )
		  )
              )
	{
	    root = PAR::find_next_entry
		( parser, next_current, key_prefix,
		      selectors, root );
	    oper = (OP::oper) root;
	}

	// Make OPERATOR token if an operator was found.
	// Note that next_current ends up pointing after the
	// OPERATOR token and current is left intact
	// and points at the new OPERATOR token.  If no
	// operator was found, current == next_current.
	//
	if ( oper != min::NULL_STUB )
	{
	    current->position.end =
	        next_current->previous->position.end;
	    while ( current != next_current->previous )
		PAR::free
		    ( PAR::remove
			  ( PAR::first_ref(parser),
			    next_current->previous ) );
	    current->type = PAR::OPERATOR;
	    PAR::value_ref ( current ) = oper->label;
	    MIN_ASSERT
	      ( current->string == min::NULL_STUB );
	}

	min::int32 oper_precedence = OP::NO_PRECEDENCE;
	    // Effective operator precedence.

	// Insert ERROR'OPERATOR token at current
	// position if bad stuff found after a postfix
	// operator.
	//
	// Also compute oper_precedence.
	//
	if ( current == D.first
	     &&
	     last_oper_flags & OP::POSTFIX
	     &&
	     current != next
	     &&
	     ( oper == min::NULL_STUB
	       ||
	       oper->precedence > D.precedence
	       ||
	       ( oper->precedence == D.precedence
	         &&
		    ( oper->flags & OP::POSTFIX )
		 == 0 ) ) )
	{
	    next_current = current;
	    current = PAR::new_token ( PAR::OPERATOR );
	    current->position.begin =
	        next_current->previous->position.end;
	    current->position.end =
	        next_current->position.begin;
	    PAR::value_ref ( current ) =
	        PAR::error_operator;
	    PAR::put_before
	        ( first_ref(parser), next_current,
		  current );
	    oper_precedence = D.precedence - 1;
	}
	else if ( oper != min::NULL_STUB )
	    oper_precedence = D.precedence;

	// If no operator found and not at end of
	// expression, move to next token and loop.
	//
	if ( current == next_current
	     &&
	     current != next )
	{
	    current = current->next;
	    continue;
	}

	// If insert ERROR'OPERAND token before current
	// if bad stuff found after infix or prefix
	// operator.
	//
	if ( current == D.first
	     &&
	     ( ( last_oper_flags & OP::INFIX
	         &&
		 ( current == next
		   ||
		   oper_precedence <= D.precedence )
	       )
	       ||
	       ( last_oper_flags & OP::PREFIX
	         &&
		 ( current == next
		   ||
		   oper_precedence < D.precedence
		   ||
	           ( oper->precedence == D.precedence
	             &&
		        ( oper->flags & OP::PREFIX )
		     == 0 )
		 )
	       )
	     )
	   )
	{
	    PAR::token t =
	        PAR::new_token ( LEXSTD::word_t  );
	    t->position.begin =
	        current->previous->position.end;
	    t->position.end =
	        current->position.begin;
	    PAR::value_ref ( t ) = PAR::error_operand;
	    PAR::put_before
	        ( first_ref(parser), current, t );
	}

	// Close previous subexpressions until
	// D.precedence < oper_precedence or
	// oper_stack->length == initial_length.
	//
	while ( current == next
		||
		oper_precedence <= D.precedence )
	{
	    if ( current != D.first )
	    {
		bool ok;
		if ( D.first_oper != min::NULL_STUB
		     &&
		     D.first_oper->reformatter != NULL )
		    ok = ( * D.first_oper->reformatter )
			     ( parser, pass->next,
			       selectors,
			       D.first, current,
			       D.first_oper );
		else if ( pass->next != min::NULL_STUB )
		    ok = (* pass->next->run_pass)
		             ( parser, pass->next,
			       selectors,
			       D.first, current );

	        if ( ! ok ) return false;
	    }

	    if ( current == next )
	    {
		D = min::pop ( oper_stack );
		if (    oper_stack->length
		     == initial_length )
		    return true;
	    }
	    else if ( oper_precedence < D.precedence )
	        D = min::pop ( oper_stack );
	    else break;
	}

	if ( oper_precedence > D.precedence )
	{
	    min::push ( oper_stack ) = D;
	    D.precedence = oper_precedence;
	    D.first_oper = oper;
	}

	D.first = next_current;
	last_oper_flags = oper->flags;
	current = next_current;
    }
}
