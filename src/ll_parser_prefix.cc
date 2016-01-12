// Layered Languages Prefix Parser Pass
//
// File:	ll_parser_prefix.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jan 12 11:20:12 EST 2016
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Prefix Parser Pass
//	Prefix Parse Function
//	Prefix Pass Command Function

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser_prefix.h>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table
# define PRE ll::parser::prefix

static min::locatable_gen prefix;
static min::locatable_gen prefix_subexpressions;

static void initialize ( void )
{
    ::prefix = min::new_str_gen ( "prefix" );
    ::prefix_subexpressions =
        min::new_lab_gen ( "prefix", "subexpressions" );

    PAR::push_new_pass ( ::prefix, PRE::new_pass );
}
static min::initializer initializer ( ::initialize );

// Prefix Parser Pass
// ------ ------ ----

static min::packed_vec< PRE::prefix_stack_struct >
    prefix_stack_type
        ( "ll::parser::prefix::prefix_stack_type" );

static min::uns32 prefix_pass_gen_disp[] =
{
    min::DISP ( & PRE::prefix_pass_struct::name ),
    min::DISP_END
};

static min::uns32 prefix_pass_stub_disp[] =
{
    min::DISP ( & PRE::prefix_pass_struct::parser ),
    min::DISP ( & PRE::prefix_pass_struct::next ),
    min::DISP ( & PRE::prefix_pass_struct
                     ::prefix_stack ),
    min::DISP_END
};

static min::packed_struct_with_base
	<PRE::prefix_pass_struct, PAR::pass_struct>
    prefix_pass_type
        ( "ll::parser::prefix::prefix_pass_type",
	  ::prefix_pass_gen_disp,
	  ::prefix_pass_stub_disp );
const min::uns32 & PRE::PREFIX_PASS =
    ::prefix_pass_type.subtype;

static void prefix_pass_place
	( PAR::parser parser,
	  PAR::pass pass )
{
    PRE::prefix_pass prefix_pass =
        (PRE::prefix_pass) pass;

    int index = TAB::find_name
        ( parser->trace_flag_name_table,
	  ::prefix_subexpressions );
    MIN_REQUIRE
      ( (unsigned) index < 8 * sizeof ( TAB::flags ) );
    
    prefix_pass->trace_subexpressions =
        1ull << index;
}

static void prefix_pass_reset
	( PAR::parser parser,
	  PAR::pass pass )
{
    PRE::prefix_pass prefix_pass =
        (PRE::prefix_pass) pass;

    PRE::prefix_stack prefix_stack =
        prefix_pass->prefix_stack;
    min::pop ( prefix_stack, prefix_stack->length );
}

static void prefix_parse ( PAR::parser parser,
		         PAR::pass pass,
		         TAB::flags selectors,
		         PAR::token & first,
		         PAR::token next );

PAR::pass PRE::new_pass ( PAR::parser parser )
{
    min::locatable_var<PRE::prefix_pass> prefix_pass
        ( ::prefix_pass_type.new_stub() );

    PRE::name_ref ( prefix_pass ) = ::prefix;

    PRE::prefix_stack_ref ( prefix_pass ) =
	::prefix_stack_type.new_stub ( 100 );

    prefix_pass->parse = ::prefix_parse;
    prefix_pass->place = ::prefix_pass_place;
    prefix_pass->reset = ::prefix_pass_reset;

    return (PAR::pass) prefix_pass;
}

// Prefix Parse Function
// ------ ----- --------

static void prefix_parse ( PAR::parser parser,
		         PAR::pass pass,
		         TAB::flags selectors,
		         PAR::token & first,
		         PAR::token next )
{
    PRE::prefix_pass prefix_pass =
        (PRE::prefix_pass) pass;
    PRE::prefix_stack prefix_stack =
        prefix_pass->prefix_stack;

    TAB::flags trace_flags = parser->trace_flags;
    if (   trace_flags
         & prefix_pass->trace_subexpressions )
    {
	trace_flags &=
	      PAR::TRACE_SUBEXPRESSION_ELEMENTS
	    + PAR::TRACE_SUBEXPRESSION_DETAILS
	    + PAR::TRACE_SUBEXPRESSION_LINES
	    + PAR::TRACE_KEYS;
	if ( trace_flags == 0 )
	    trace_flags =
	        PAR::TRACE_SUBEXPRESSION_ELEMENTS;
    }
    else
        trace_flags = 0;

    // We add to the stack but leave alone what is
    // already in the stack so this function can be
    // called recursively.
    //
    min::unsptr initial_length = prefix_stack->length;

    bool prefix_found = false;
    bool at_phrase_beginning = true;
    PAR::token current = first;
    while ( true )
    {

	// Find prefix if possible.
	//
	while ( current != next
		&&
		current->type != PAR::PREFIX )
	{
	    at_phrase_beginning = false;
	    current = current->next;
	}

        // Discover if prefix is in stack.  Set i to
	// stack position + 1 if yes, initial_length
	// if no.
	//
	min::unsptr i =
	    ( current != next ?
	      prefix_stack->length :
	      initial_length );
	while ( i > initial_length
	        &&
		   current->value_type
		!= (&prefix_stack[i-1])->first
		                       ->value_type )
	    -- i;
	
	// If prefix is in the stack or we are a the end
	// of the expression, close all the previous
	// subexpressions delimited by the stack entry
	// of prefix and entries closer to the top of
	// the stack, or all stack entries if we are at
	// the end of the expression.
	//
	if ( i > initial_length || current == next )
	{
	    at_phrase_beginning = true;

	    min::unsptr j = prefix_stack->length;
	    while ( j >= i && j > initial_length )
	    {
		PAR::token prefix =
		    min::pop(prefix_stack).first;
		j = prefix_stack->length;

		PAR::token first = prefix->next;

		if ( first != current )
		{
		    PAR::execute_pass_parse
			 ( parser, pass->next,
			   selectors,
			   first, current );

		    min::phrase_position position =
			{ prefix->position.begin,
			  current->previous
			         ->position.end };

		    min::obj_vec_insptr vp
		    	( prefix->value );
		    min::attr_insptr ap ( vp );

		    min::locate
		        ( ap, min::dot_position );
		    min::phrase_position_vec_insptr
		        pos = min::get ( ap );
		    pos->position = position;

		    while ( first != current )
		    {
			if (    first->string
			     != min::NULL_STUB )
			    PAR::convert_token
			        ( first );

			min::attr_push(vp) =
			    first->value;
			min::push ( pos ) =
			    first->position;

			first = first->next;
			PAR::free
			    ( PAR::remove
				  ( PAR::first_ref
				        (parser),
				    first->previous ) );
		    }
		    prefix->position = position;
		}

		prefix->type = PAR::BRACKETED;
		PAR::trace_subexpression
		    ( parser, prefix, trace_flags );
	    }
	}

	if ( current == next ) break;

	else if ( at_phrase_beginning )
	{
	    PRE::prefix_stack_struct s =
	        { current, min::MISSING() };
	    min::push(prefix_stack) = s;
	    current = current->next;
	    prefix_found = true;
	}

	else
	{
	    PAR::parse_error
	        ( parser,
		  current->position,
		  "prefix token not at beginning of"
		  " phrase; deleted and ignored" );
	    current = current->next;
	    PAR::free
		( PAR::remove
		      ( PAR::first_ref
			    (parser),
			current->previous ) );
	}
    }

    if ( ! prefix_found )
	PAR::execute_pass_parse
	    ( parser, pass->next, selectors,
	      first, current );
}
