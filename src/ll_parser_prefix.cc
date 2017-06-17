// Layered Languages Prefix Subexpression Parser
//
// File:	ll_parser_prefix.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jun 17 16:00:48 EDT 2017
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
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define PARLEX ll::parser::lexeme
# define TAB ll::parser::table
# define PRE ll::parser::prefix

static bool initialize_called = false;
static min::locatable_gen equals;

static void initialize ( void )
{
    if ( initialize_called ) return;
    initialize_called = true;

    ::equals = min::new_str_gen ( "=" );
}
static min::initializer initializer ( ::initialize );

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

static bool data_reformatter_function
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  const min::phrase_position & position,
	  TAB::flags trace_flags,
	  TAB::root entry )
{
    PRE::prefix prefix_entry = (PRE::prefix) entry;
    PAR::reformatter_arguments args =
        prefix_entry->reformatter_arguments;
    MIN_REQUIRE ( args->length == 2 );

    MIN_REQUIRE ( first != next );
    if ( first->next == next ) return true;
    if ( first->next->next == next ) return true;

    if ( first->next->value != args[1] ) return true;
    if ( first->next->type != LEXSTD::numeric_t )
        return true;

    min::str_ptr argsp ( args[0] );
    char expl = argsp[0];
    char at   = argsp[1];

    min::str_ptr sp ( first->next->value );
    MIN_REQUIRE ( sp );
    min::unsptr i =
        ( sp[0] == at ? 1 :
          ( sp[0] == expl && sp[1] == at ) ? 2 :
	  0 );
    if ( i == 0 ) return true;
    if ( sp[i] == '0' ) return true;
    min::uns32 ID;
    if ( ! min::strto ( ID, sp, i, 10 ) )
        return true;
    if ( sp[i] != 0 ) return true;

    PAR::free ( PAR::remove ( first_ref(parser),
			      first->next  ) );
    PAR::free ( PAR::remove ( first_ref(parser),
			      first->next  ) );

    min::locatable_gen attributes ( min::MISSING() );
    if ( next->previous != first
         &&
	 next->previous->value_type == PARLEX::colon )
    {
        attributes = next->previous->value;
	PAR::free ( PAR::remove ( first_ref(parser),
			          next->previous  ) );
    }

    PRE::compact_prefix_list
        ( parser, pass, selectors, first, next,
	  min::MISSING_POSITION, min::NULL_STUB,
	  trace_flags, true );

    if ( attributes != min::MISSING() )
    {
        min::obj_vec_ptr paragraph ( attributes );
	for ( min::uns32 i = 0;
	      i < min::size_of ( paragraph ); ++ i )
        {
	    min::obj_vec_ptr line ( paragraph[i] );
	}
    }

    PAR::trace_subexpression
	( parser, first, trace_flags );

    return true;
}

static void prefix_reformatter_stack_initialize ( void )
{
    ::initialize();

    // TBD: see untyped_reformatter_stack below.

    min::locatable_gen label
        ( min::new_str_gen ( "data" ) );
    PAR::push_reformatter
        ( label, 0, 0, 0,
	  ::data_reformatter_function,
	  PRE::prefix_reformatter_stack );
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
	  TAB::flags trace_flags,
	  bool dont_delete )
{
    if (    first->next == next
         && ! separator_found
	 && ! dont_delete
	 && ( first->type == PAR::IMPLIED_PREFIX
	      ||
	      first->type == PAR::IMPLIED_HEADER ) )
    {
	PAR::free
	    ( PAR::remove ( first_ref(parser),
			    first ) );
	return false;
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
