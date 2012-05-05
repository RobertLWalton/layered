// Layers Language Operator Parser Pass
//
// File:	ll_parser_oper.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri May  4 15:26:31 EDT 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Operator Table Entries
//	Operator Parser Pass
//	Operator Parser Pass Run Routine

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser_oper.h>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table
# define OP ll::parser::oper


// Operator Table Entries
// -------- ----- -------

static min::uns32 oper_gen_disp[] = {
    min::DISP ( & OP::oper_struct::label ),
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

static min::packed_vec< OP::oper_stack_struct >
    oper_stack_type
        ( "(ll_parser_oper.cc)::oper_stack_type" );

static min::uns32 oper_pass_stub_disp[] =
{
    min::DISP ( & OP::oper_pass_struct::next ),
    min::DISP ( & OP::oper_pass_struct::oper_table ),
    min::DISP ( & OP::oper_pass_struct::oper_stack ),
    min::DISP_END
};

static min::packed_struct_with_base
	<OP::oper_pass_struct, PAR::pass_struct>
    oper_pass_type
        ( "ll::parser::oper::oper_pass_type",
	  NULL,
	  ::oper_pass_stub_disp );
const min::uns32 & OP::OPER_PASS =
    ::oper_pass_type.subtype;

static void run_oper_pass ( PAR::parser parser,
		            PAR::pass pass,
		            TAB::selectors selectors,
		            PAR::token & first,
		            PAR::token next );

OP::oper_pass OP::place
	( ll::parser::parser parser,
	  ll::parser::pass next )
{
    min::locatable_var<OP::oper_pass> oper_pass
        ( ::oper_pass_type.new_stub() );

    OP::oper_table_ref ( oper_pass ) =
        TAB::create_table ( 1024 );
    min::push ( oper_pass->oper_table, 1024 );
    OP::oper_stack_ref ( oper_pass ) =
	::oper_stack_type.new_stub ( 100 );

    oper_pass->run_pass = ::run_oper_pass;

    PAR::place
        ( parser, (PAR::pass) oper_pass, next );
    return oper_pass;
}

// Operator Parser Pass Run Routine
// -------- ------ ---- --- -------

// Return true iff the argument is a precedence in the
// oper_stack.
//
inline bool check_precedence
	( int precedence,
	 OP::oper_stack oper_stack )
{
    for ( min::uns32 i = 0; i < oper_stack->length;
    			    ++ i )
    {
        if ( oper_stack[i].precedence == precedence )
	    return true;
    }
    return false;
}

static void run_oper_pass ( PAR::parser parser,
		            PAR::pass pass,
		            TAB::selectors selectors,
		            PAR::token & first,
		            PAR::token next )
{
    OP::oper_pass oper_pass = (OP::oper_pass) pass;
    OP::oper_stack oper_stack = oper_pass->oper_stack;
    bool trace =
        (   parser->trace
          & PAR::TRACE_OPERATOR_SUBEXPRESSIONS );

    // We add to the stack but leave alone what is
    // already in the stack so this function can be
    // called recursively.
    //
    min::unsptr initial_length = oper_stack->length;

    // Data that is pushed to oper_stack.  D is in
    // effect the top of the stack.
    //
    OP::oper_stack_struct D;
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
	// Note that next_current ends up pointing after
	// the OPERATOR token and current is left intact
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

	    if ( trace )
	    {
		parser->printer
		    << min::bom
		    << min::set_indent ( 7 )
		    << "OPERATOR `"
		    << min::pgen
			 ( current->value,
			   & PAR::name_format )
		    << "' found; "
		    << min::pline_numbers
			   ( parser->input_file,
			     current->position )
		    << ":" << min::eom;
		min::print_phrase_lines
		    ( parser->printer,
		      parser->input_file,
		      current->position );
	    }
	}

	min::int32 oper_precedence = OP::NO_PRECEDENCE;
	    // Effective operator precedence.

	// Insert ERROR'OPERATOR token just before
	// current position if bad token (e.g.,
	// non-operator or operator with too high a
	// precedence) found after a postfix operator.
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
	        next_current->position.begin;
	    current->position.end =
	        next_current->position.begin;
	    PAR::value_ref ( current ) =
	        PAR::error_operator;
	    PAR::put_before
	        ( first_ref(parser), next_current,
		  current );
	    oper_precedence = D.precedence - 1;
	    D.first = current;

	    parser->printer
		<< min::bom
		<< min::set_indent ( 7 )
		<< "ERROR: missing"
		   " operator of precedence "
		<< oper_precedence
		<< " inserted; "
		<< min::pline_numbers
		       ( parser->input_file,
			 current->position )
		<< ":" << min::eom;
	    min::print_phrase_lines
		( parser->printer,
		  parser->input_file,
		  current->position );
	    ++ parser->error_count;
	}
	else if ( oper != min::NULL_STUB )
	    oper_precedence = oper->precedence;

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

	// Insert ERROR'OPERAND token just before
	// current if bad token (e.g., operator with too
	// low a precedence) found after infix or prefix
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
	        current->position.begin;
	    t->position.end =
	        current->position.begin;
	    PAR::value_ref ( t ) = PAR::error_operand;
	    PAR::put_before
	        ( first_ref(parser), current, t );
	    D.first = t;

	    parser->printer
		<< min::bom
		<< min::set_indent ( 7 )
		<< "ERROR: missing"
		   " operand inserted; "
		<< min::pline_numbers
		       ( parser->input_file,
			 t->position )
		<< ":" << min::eom;
	    min::print_phrase_lines
		( parser->printer,
		  parser->input_file,
		  t->position );
	    ++ parser->error_count;
	}

	// Close previous subexpressions until
	// D.precedence < oper_precedence or
	// oper_stack->length == initial_length.
	//
	OP::oper first_oper = min::NULL_STUB;
	while ( current == next
		||
		oper_precedence <= D.precedence )
	{
	    if ( current != D.first )
	    {
		if ( first_oper != min::NULL_STUB
		     &&
		     first_oper->reformatter != NULL )
		    ( * first_oper->reformatter )
			( parser, pass->next, selectors,
			  D.first, current,
			  first_oper );
		else if ( first_oper != min::NULL_STUB )
		{
		    min::phrase_position position;
		    position.begin =
		        D.first->position.begin;
		    position.end =
		        current->previous->position.end;
		    PAR::attr attr
		        ( PAR::dot_oper,
			  first_oper->label );
		    PAR::compact
		        ( parser, pass->next,
			  selectors,
			  PAR::BRACKETABLE, trace,
			  D.first, current, position,
			  1, & attr );
		}
		else if ( D.first->next != current
		          &&
		             oper_stack->length
		          != initial_length )
		{
		    min::phrase_position position;
		    position.begin =
		        D.first->position.begin;
		    position.end =
		        current->previous->position.end;
		    PAR::compact
			( parser, pass->next,
			  selectors,
			  PAR::BRACKETABLE, trace,
			  D.first, current, position );
		}
	    }

	    if ( current == next )
	    {
		if (    oper_stack->length
		     == initial_length )
		{
		    first = D.first;
		    return;
		}
		first_oper = D.first_oper;
		D = min::pop ( oper_stack );
	    }
	    else if ( oper_precedence < D.precedence )
	    {
		first_oper = D.first_oper;
	        D = min::pop ( oper_stack );
	    }
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

// Operator Reformatters
// -------- ------------

static void separator_reformatter
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::selectors selectors,
	  PAR::token & first,
	  PAR::token next,
	  OP::oper first_oper )
{
    MIN_ASSERT ( first != next );

    min::phrase_position position =
	{ first->position.begin,
          next->previous->position.end };

    min::gen separator = first_oper->label;
    bool separator_should_be_next = false;
    for ( PAR::token t = first; t != next; )
    {
        if ( t->type == PAR::OPERATOR )
	{
	    if ( separator != t->value )
	    {
		parser->printer
		    << min::bom
		    << min::set_indent ( 7 )
		    << "ERROR: wrong"
		       " separator `"
		    << min::pgen
			 ( t->value,
			   & PAR::name_format )
		    << "' changed to `"
		    << min::pgen
			 ( separator,
			   & PAR::name_format )
		    << "'; "
		    << min::pline_numbers
			   ( parser->input_file,
			     t->position )
		    << ":" << min::eom;
		min::print_phrase_lines
		    ( parser->printer,
		      parser->input_file,
		      t->position );
		++ parser->error_count;
	    }

	    if ( ! separator_should_be_next )
	    {
	        PAR::put_empty_before ( parser, t );
		if ( t == first ) first = t->previous;
	    }
	    else separator_should_be_next = false;

	    t = t->next;
	    PAR::free
		( PAR::remove
		      ( PAR::first_ref(parser),
			t->previous ) );
	}
	else if ( separator_should_be_next )
	{
	    min::phrase_position position =
	        { t->position.begin,
		  t->position.begin };

	    parser->printer
		<< min::bom
		<< min::set_indent ( 7 )
		<< "ERROR: missing"
		   " separator `"
		<< min::pgen
		     ( separator,
		       & PAR::name_format )
		<< "' inserted; "
		<< min::pline_numbers
		       ( parser->input_file,
			 position )
		<< ":" << min::eom;
	    min::print_phrase_lines
		( parser->printer,
		  parser->input_file,
		  position );
	    ++ parser->error_count;

	    t = t->next;
	}
	else
	{
	    separator_should_be_next = true;
	    t = t->next;
	}
    }

    if ( ! separator_should_be_next )
        PAR::put_empty_after ( parser, next->previous );

    PAR::attr separator_attr
        ( PAR::dot_separator, separator );

    bool trace =
        (   parser->trace
          & PAR::TRACE_OPERATOR_SUBEXPRESSIONS );
    PAR::compact
        ( parser, pass, selectors,
	  PAR::BRACKETABLE, trace,
	  first, next, position,
	  1, & separator_attr );
    first->type = PAR::BRACKETABLE;
}
OP::reformatter OP::separator_reformatter =
    ::separator_reformatter;

static void right_associative_reformatter
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::selectors selectors,
	  PAR::token & first,
	  PAR::token next,
	  OP::oper first_oper )
{
    MIN_ASSERT ( first != next );

    bool trace =
        (   parser->trace
          & PAR::TRACE_OPERATOR_SUBEXPRESSIONS );

    // As operators must be infix, operands and
    // operators must alternate with operands first and
    // last.

    // Work from end to beginning taking 3 tokens at a
    // time and rewriting them.
    //
    for ( PAR::token t = next; t != first; )
    {
        t = t->previous;
	MIN_ASSERT ( t->type != PAR::OPERATOR );
	MIN_ASSERT ( t != first );
        t = t->previous;
	MIN_ASSERT ( t->type == PAR::OPERATOR );
	MIN_ASSERT ( t != first );
        t = t->previous;
	MIN_ASSERT ( t->type != PAR::OPERATOR );

        min::phrase_position position =
	    { t->position.begin,
	      t->next->next->position.end };

	// Switch operator and first operand.
	//
	PAR::token oper =
	    PAR::remove ( PAR::first_ref ( parser ),
	                  t->next );
	PAR::put_before ( PAR::first_ref ( parser ),
	                  t, oper );
	bool t_is_first = ( t == first );
	t = t->previous;

	PAR::attr oper_attr
	    ( PAR::dot_oper, oper->value );
	PAR::compact
	    ( parser, pass, selectors,
	      PAR::BRACKETABLE, trace,
	      t, next, position,
	      1, & oper_attr );
	t->type = PAR::BRACKETABLE;

	if ( t_is_first ) first = t;
    }
}
OP::reformatter OP::right_associative_reformatter =
    ::right_associative_reformatter;
