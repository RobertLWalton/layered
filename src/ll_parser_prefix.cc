// Layered Languages Prefix Subexpression Parser
//
// File:	ll_parser_prefix.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Jun 15 02:01:42 EDT 2017
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Prefix Table
//	Prefix Reformatters
//	Prefix Compact Function

// Usage and Setup
// ----- --- -----

# include <ll_parser.h>
# include <ll_parser_prefix.h>
# define MUP min::unprotected
# define PAR ll::parser
# define PARLEX ll::parser::lexeme
# define TAB ll::parser::table
# define PRE ll::parser::prefix

// Prefix Table
// ------ -----

static min::uns32 prefix_gen_disp[] = {
    min::DISP ( & PRE::prefix_struct::label ),
    min::DISP ( & PRE::prefix_struct::group ),
    min::DISP ( & PRE::prefix_struct
                     ::implied_subprefix ),
    min::DISP ( & PRE::prefix_struct
                     ::implied_subprefix_type ),
    min::DISP_END };

static min::uns32 prefix_stub_disp[] = {
    min::DISP ( & PRE::prefix_struct::next ),
    min::DISP ( & PRE::prefix_struct::reformatter ),
    min::DISP ( & PRE::prefix_struct
                     ::reformatter_arguments ),
    min::DISP_END };

static min::packed_struct_with_base
	<PRE::prefix_struct, TAB::root_struct>
    prefix_type ( "ll::parser::bracketed::prefix_type",
	        ::prefix_gen_disp,
	        ::prefix_stub_disp );
const min::uns32 & PRE::PREFIX = ::prefix_type.subtype;

void PRE::push_prefix
	( min::gen prefix_label,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  TAB::new_flags new_selectors,
	  min::gen group,
	  min::gen implied_subprefix,
	  min::gen implied_subprefix_type,
	  min::uns32 paragraph_lexical_master,
	  min::uns32 line_lexical_master,
	  ll::parser::reformatter reformatter,
	  ll::parser::reformatter_arguments
	      reformatter_arguments,
	  TAB::key_table prefix_table )
{
    min::locatable_var<PRE::prefix> prefix
        ( ::prefix_type.new_stub() );

    label_ref(prefix) = prefix_label;
    prefix->selectors = selectors;
    prefix->block_level = block_level;
    prefix->position = position;
    prefix->new_selectors = new_selectors;

    prefix->new_selectors.or_flags &= ~
        PAR::TOP_LEVEL_SELECTOR;
    prefix->new_selectors.not_flags |=
        PAR::TOP_LEVEL_SELECTOR;
    prefix->new_selectors.xor_flags &= ~
        PAR::TOP_LEVEL_SELECTOR;

    group_ref(prefix) = group;
    implied_subprefix_ref(prefix) = implied_subprefix;
    implied_subprefix_type_ref(prefix) =
        implied_subprefix_type;
    prefix->paragraph_lexical_master =
        paragraph_lexical_master;
    prefix->line_lexical_master = line_lexical_master;
    reformatter_ref(prefix) = reformatter;
    reformatter_arguments_ref(prefix) =
        reformatter_arguments;

    TAB::push ( prefix_table, (TAB::root) prefix );
}

// Prefix Reformatters
// ------ ------------


min::locatable_var<PAR::reformatter>
    PRE::prefix_reformatter_stack ( min::NULL_STUB );

static void prefix_reformatter_stack_initialize ( void )
{
    // ::initialize();

    // TBD: see untyped_reformatter_stack below.
}
static min::initializer prefix_reformatter_initializer
    ( ::prefix_reformatter_stack_initialize );


// Prefix Compact Function
// ------ ------- --------

bool PRE::compact_prefix_list
	( PAR::parser parser,
	  PAR::pass pass,
	  PAR::table::flags selectors,
	  PAR::token first,
	  PAR::token next,
          const min::position & separator_found,
	  TAB::root line_sep,
	  TAB::flags trace_flags )
{
    if ( first->next == next && ! separator_found )
    {
	if ( first->type == PAR::IMPLIED_PREFIX
	     ||
	     first->type == PAR::IMPLIED_HEADER )
	{
	    PAR::free
		( PAR::remove ( first_ref(parser),
				first ) );
	    return false;
	}
    }
    else
    {
	first->position.end =
	    next->previous->position.end;
	PAR::token current = first->next;

	PAR::execute_pass_parse
	     ( parser, pass, selectors,
	       current, next );


	min::obj_vec_insptr vp
	    ( first->value );
	min::locatable_var
		<min::phrase_position_vec_insptr>
	    pos;

	if ( first->type == PAR::IMPLIED_PREFIX
	     ||
	     first->type == PAR::IMPLIED_HEADER )
	{
	    min::uns32 unused_size = 0;
	    for ( PAR::token t = first->next;
	          t != next; t = t->next )
	        ++ unused_size;
	    if ( separator_found )
	        unused_size += 5;
	    PAR::value_ref(first) =
	        min::copy ( vp, unused_size );
	    vp = first->value;

	    min::init ( pos, parser->input_file,
	                first->position, 0 );
	    min::attr_insptr ap ( vp );
	    min::locate ( ap, min::dot_position );
	    min::set ( ap, min::new_stub_gen ( pos ) );
	    min::set_flag
	        ( ap, min::standard_attr_hide_flag );
	    if ( separator_found )
	    {
		min::locate ( ap, min::dot_terminator );
		min::set ( ap, line_sep->label );
		first->position.end = separator_found;
	    }
	}
	else
	{
	    min::attr_insptr ap ( vp );
	    min::locate ( ap, min::dot_position );
	    pos = min::get ( ap );
	    if ( separator_found )
	    {
		min::locate ( ap, min::dot_terminator );
		min::set ( ap, line_sep->label );
		first->position.end = separator_found;
	    }
	}
	pos->position = first->position;

	while ( current != next )
	{
	    if (    current->string
		 != min::NULL_STUB )
		PAR::convert_token
		    ( current );

	    if ( min::is_attr_legal ( current->value ) )
	    {
		min::attr_push(vp) = current->value;
		min::push ( pos ) = current->position;
	    }
	    else
	        PAR::parse_error
		    ( parser, current->position,
		      "not a legal object element"
		      " value; `",
		      min::pgen_never_quote
		          ( current->value ),
		      "'; ignored" );

	    current = current->next;
	    PAR::free
		( PAR::remove
		      ( PAR::first_ref
			    (parser),
			current->previous ) );
	}
    }

    first->type = PAR::BRACKETED;

    PAR::trace_subexpression
        ( parser, first, trace_flags );

    return true;
}
