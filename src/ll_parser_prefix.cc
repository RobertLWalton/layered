// Layered Languages Prefix Subexpression Parser
//
// File:	ll_parser_prefix.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun May 19 15:54:35 EDT 2019
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
    MIN_REQUIRE ( args->length == 4 );
    MIN_REQUIRE ( first != next );
    if ( first->next == next ) return true;
    if ( first->next->next == next ) return true;
    if ( first->next->next->value != args[0] )
        return true;
    if ( min::is_obj ( first->next->value ) )
    {
        min::obj_vec_ptr tvp ( first->next->value );
	if ( min::has_single_attr ( tvp ) )
	    return true;
    }
    else
    if ( first->next->type != PAR::DERIVED
         ||
	 ! min::is_preallocated ( first->next->value ) )
        return true;

    // If prefix has attributes other than .type and
    // .position, return true.
    //
    {
        min::obj_vec_ptr pvp ( first->value );
	min::attr_ptr pap ( pvp );
        min::attr_info info[2];
	min::unsptr n =
	    min::attr_info_of ( info, 2, pap );
	if ( n != 2 ) return true;
	for ( unsigned i = 0; i < n; ++ i )
	{
	    if ( info[i].name != min::dot_type
	         &&
		 info[i].name != min::dot_position )
		return true;
	    if ( info[i].reverse_attr_count > 0 )
	        return true;
        }
    }

    min::phrase_position ID_position =
        first->next->position;
    min::position end_position =
        next->previous->position.end;

    min::locatable_gen ID_gen ( first->next->value );

    PAR::free ( PAR::remove ( first_ref(parser),
			      first->next  ) );
    PAR::free ( PAR::remove ( first_ref(parser),
			      first->next  ) );

    min::locatable_gen attributes ( min::MISSING() );
    min::unsptr asize = 0;
    if (    min::get ( next->previous->value,
	               min::dot_terminator )
	 == min::INDENTED_PARAGRAPH() )
    {
        attributes = next->previous->value;
	PAR::free ( PAR::remove ( first_ref(parser),
			          next->previous  ) );
	min::obj_vec_ptr avp ( attributes );
	asize = min::size_of ( avp );
    }

    PRE::compact_prefix_list
        ( parser, pass, selectors, first, next,
	  min::MISSING_POSITION, min::MISSING(),
	  trace_flags );

    min::obj_vec_ptr fvp ( first->value );
    min::unsptr fvpsize = min::size_of ( fvp );

    // If value has one element and no attributes (other
    // than .position) then replace the value by its
    // sole element and finish up.
    //
    if (    min::size_of ( fvp ) == 1
         && attributes == min::MISSING()
	 && min::is_preallocated ( ID_gen ) )
    {
	PAR::value_ref(first) = fvp[0];
	fvp = min::NULL_STUB;
	PAR::trace_subexpression
	    ( parser, first, trace_flags );

	if ( min::is_obj ( first->value ) )
	    min::copy ( ID_gen, first->value, 0 );
	else
	{
	    min::uns32 ID =
		min::id_of_preallocated ( ID_gen );
	    min::id_map map = parser->id_map;
	    MIN_REQUIRE ( ID < map->length );
	    MIN_REQUIRE ( map[ID] == ID_gen );
	    MIN_REQUIRE
		( map->hash_table == min::NULL_STUB );
	    if (    min::count_of_preallocated
	                ( ID_gen )
		 != 1 )
		PAR::parse_error
		    ( parser, ID_position,
		      "previous uses of ID exist and"
		      " will be dangling" );
	    min::put ( map, ID, first->value );
	}

	first = first->next;
	PAR::free ( PAR::remove ( first_ref(parser),
				  first->previous ) );
	return false;
    }

    if ( min::is_preallocated ( ID_gen ) )
	min::new_obj_gen
	    ( ID_gen, 5 + fvpsize + 5 * asize,
	              2 * asize + 4 );

    min::obj_vec_insptr idvp ( ID_gen );
    for ( min::unsptr i = 0; i < fvpsize; ++ i )
	min::attr_push ( idvp ) = fvp[i];

    min::attr_ptr fap ( fvp );
    min::attr_insptr idap ( idvp );
    locate ( fap, min::dot_position );
    min::phrase_position_vec idppvec =
        min::get ( fap );
    locate ( idap, min::dot_position );
    min::set ( idap, min::new_stub_gen ( idppvec ) );
    min::set_flag
        ( idap, min::standard_attr_hide_flag );

    if ( attributes != min::MISSING() )
    {
        {
	    first->position.end = end_position;
	    min::phrase_position_vec_insptr insppvec =
		min::phrase_position_vec_insptr
		    ( idppvec );
	    insppvec->position.end = end_position;
	}

        min::obj_vec_ptr paragraph ( attributes );
	for ( min::uns32 i = 0;
	      i < min::size_of ( paragraph ); ++ i )
        {
	    min::obj_vec_ptr line ( paragraph[i] );
	    min::phrase_position_vec lppvec =
		min::position_of ( line );
	    min::unsptr value_index;
	    min::uns32 lsize = min::size_of ( line );

	    if ( lsize == 0 ) continue;
	    min::uns32 j = 0;
	    bool has_negator = false;
	    if ( line[0] == args[1] )
	    {
	        ++ j;
		has_negator = true;
	    }
	    min::locatable_gen name
		( PAR::scan_label
		      ( line, j, args[0] ) );
	    if ( name == min::MISSING() )
	    {
		PAR::parse_error
		    ( parser, lppvec[i],
		      "line does not begin with a"
		      " (possibly negated)"
		      " attribute label;"
		      " line ignored" );
		continue;
	    }

	    min::gen flags = min::MISSING();

	    const char * message =
	        "after attribute label `";
	    if ( j < lsize && min::is_obj ( line[j] ) )
	    {
	        min::obj_vec_ptr lvp ( line[j] );
		min::attr_ptr lap ( lvp );
		min::locate ( lap, min::dot_initiator );
		if ( min::get ( lap ) == args[2] )
		{
		    flags = line[j++];
		    message =
			"after attribute label flags `";
		}
	    }

	    if ( j < lsize && line[j] != args[0] )
	    {
		PAR::parse_error
		    ( parser, lppvec[j],
		      message,
		      min::pgen_never_quote
			  ( args[0] ),
		      "' was expected but not"
		      " found; line ignored" );
		continue;
	    }

	    if ( j < lsize && has_negator )
	    {
		PAR::parse_error
		    ( parser,
		      lppvec[0],
		      "negator preceding"
		      " attribute label"
		      " that is followed"
		      " by `",
		      min::pgen_never_quote
			  ( args[0] ),
		      "'; negator"
		      " ignored" );
	    }

	    // Now if j < lsize the last thing scanned
	    // was an args[0] (e.g., "=").

	    min::locatable_gen value
	        ( has_negator ? min::FALSE
		              : min::TRUE );
	    bool is_multivalue = false;
	    min::locatable_gen reverse_name
		( min::MISSING() );
	    min::gen reverse_flags = min::MISSING();
	    if ( j + 1 == lsize )
	    {
		PAR::parse_error
		    ( parser, lppvec[j],
		      "after `",
		      min::pgen_never_quote
			  ( args[0] ),
		      "' argument value was expected"
		      " but not found; line ignored" );
		continue;
	    }
	    else if ( j + 2 == lsize
	              ||
	    	      (    j + 3 < lsize
	                && line[j+3] == args[0] ) )
	    {
	        // Single token value, including
		// possible multi-value bracketed
		// list.
		//
	        value_index = ++ j;
		value = line[j++];
		if ( min::is_obj ( value ) )
		{
		    min::obj_vec_ptr vvp ( value );
		    min::attr_ptr vap ( vvp );
		    min::locate
		        ( vap, min::dot_initiator );
		    is_multivalue =
			( min::get ( vap ) == args[3] );
		}
		if ( j < lsize )
		{
		    // name = value(s) = reverse_name
		    // Ready to scan second "=".
		    //
		    MIN_REQUIRE ( line[j] == args[0] );
		    ++ j;
		    reverse_name =
			( PAR::scan_label ( line, j ) );
		    if (    reverse_name
		         == min::MISSING() )
		    {
			PAR::parse_error
			    ( parser, lppvec[i],
			      "reverse attribute label"
			      " after second `",
			      min::pgen_never_quote
				  ( args[0] ),
			      "' was expected but not"
			      " found; line ignored" );
			continue;
		    }

		    if (    j < lsize
		         && min::is_obj ( line[j] ) )
		    {
			min::obj_vec_ptr lvp
			    ( line[j] );
			min::attr_ptr lap ( lvp );
			min::locate
			    ( lap, min::dot_initiator );
			if (    min::get ( lap )
			     == args[2] )
			    reverse_flags = line[j++];
		    }
		}
	    }
	    else if ( j + 2 < lsize )
	    {
	        // Multi-token value, i.e., a label.
		//
		int j0 = j ++;
		value = PAR::scan_label ( line, j );
		if ( value == min::MISSING() )
		{
		    PAR::parse_error
			( parser, lppvec[j0],
			  "after `",
			  min::pgen_never_quote
			      ( args[0] ),
			  "' attribute value (label or"
			  " single bracketed"
			  " subexpression) was"
			  " expected but none found;"
			  " line ignored" );
		    continue;
		}
	    }

	    if ( j < lsize )
	    {
		min::phrase_position pos =
		    { (&lppvec[j])->begin,
		      (&lppvec[lsize-1])->end };
		PAR::parse_error
		    ( parser, pos,
		      "extra stuff at end of line;"
		      " line ignored" );
		continue;
	    }

	    min::locate ( idap, name );
	    if ( reverse_name == min::MISSING() )
	    {
	        min::attr_info info;
		if ( min::attr_info_of ( info, idap ) )
		{
		    PAR::parse_error
			( parser, lppvec->position,
			  "single-arrow attribute has"
			  " been set previously;"
			  " line ignored" );
		    continue;
		}

		if ( flags != min::MISSING() )
		    PAR::set_attr_flags
			( parser, idap, flags );
	    }
	    else
	    {
	        min::attr_info info;
		min::attr_info_of ( info, idap );
		if ( info.flag_count > 0 )
		{
		    if ( flags == min::MISSING() )
		    {
			PAR::parse_error
			    ( parser, lppvec->position,
			      "double-arrow attribute"
			      " has flags but no flags"
			      " given in line;"
			      " line ignored" );
			continue;
		    }
		    else if ( ! PAR::test_attr_flags
		                   ( parser, idap,
				     flags ) )
		    {
			PAR::parse_error
			    ( parser, lppvec->position,
			      "double-arrow attribute"
			      " has flags that disagree"
			      " with flags given in"
			      " line (see above"
			      " errors);"
			      " line ignored" );
			continue;
		    }
		}

		if ( ! min::is_obj ( value ) )
		{
		    PAR::parse_error
			( parser, lppvec[value_index],
			  "double-arrow attribute"
			  " value is NOT an object;"
			  " line ignored" );
		    continue;
		}

		{
		    min::obj_vec_insptr rvp ( value );
		    min::attr_insptr rap ( rvp );
		    min::attr_info rinfo;
		    min::attr_info_of ( rinfo, rap );
		    if ( rinfo.flag_count > 0 )
		    {
			if (    reverse_flags
			     == min::MISSING() )
			{
			    PAR::parse_error
				( parser,
				  lppvec->position,
				  "double-arrow"
				  " attribute has"
				  " reverse flags but"
				  " no reverse flags"
				  " are given in line;"
				  " line ignored" );
			    continue;
			}
			else if ( ! PAR::test_attr_flags
				       ( parser, rap,
				         reverse_flags )
				)
			{
			    PAR::parse_error
				( parser,
				  lppvec->position,
				  "double-arrow"
				  " attribute has"
				  " reverse flags that"
				  " disagree with"
				  " reverse flags given"
				  " in line (see above"
				  " errors);"
				  " line ignored" );
			    continue;
			}
		    }
		    else if (    reverse_flags
		              != min::MISSING() )
			PAR::set_attr_flags
			    ( parser, rap,
			      reverse_flags );
		}

		if (    info.flag_count == 0
		     && flags != min::MISSING() );
		    PAR::set_attr_flags
			( parser, idap, flags );

	        min::locate_reverse
		    ( idap, reverse_name );
	    }

	    if ( ! is_multivalue )    
	    {
		PAR::set_attr_value
		    ( parser, idap, value,
		      lppvec[value_index] );
	    }
	    else
		PAR::set_attr_multivalue
		    ( parser, idap, value );
	}
    }

    PAR::value_ref(first) = ID_gen;
    idvp = min::NULL_STUB;
    PAR::trace_subexpression
	( parser, first, trace_flags );

    first = first->next;
    PAR::free ( PAR::remove ( first_ref(parser),
			      first->previous ) );

    return false;
}

static bool sentence_reformatter_function
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

    MIN_REQUIRE ( first != next );

    for ( PAR::token t = first->next; t != next;
                                      t = t->next )
    {
	if ( ! PAR::is_lexeme ( t->type ) )
	    continue;

	min::unsptr i = 0;
	while (    i < args->length
	        && args[i] != t->value )
	    ++ i;
	if ( i >= args->length ) continue;

	min::position separator_found =
	    t->position.end;
	PAR::token next_first = min::NULL_STUB;
	if ( t->next != next )
	{
	    next_first = t;
	    next_first->position.begin =
	    next_first->position.end =
		separator_found;
	    PAR::value_ref(next_first) =
		first->value;
	    PAR::value_type_ref(next_first) =
		first->value_type;
	    next_first->type = first->type;
	    first->type = PAR::IMPLIED_PREFIX;
	}

	PRE::compact_prefix_list
	    ( parser, pass, selectors, first, t,
	      separator_found, args[i],
	      trace_flags );

	first = next_first;
    }
    if ( first == min::NULL_STUB )
	PAR::free
	    ( PAR::remove ( first_ref(parser),
			    next->previous ) );
    else
    {
	PRE::compact_prefix_list
	    ( parser, pass, selectors, first, next,
	      min::MISSING_POSITION, min::MISSING(),
	      trace_flags );
    }
    return false;
}

static void prefix_reformatter_stack_initialize ( void )
{
    ::initialize();

    min::locatable_gen data_name
        ( min::new_str_gen ( "data" ) );
    PAR::push_reformatter
        ( data_name, 0, 4, 4,
	  ::data_reformatter_function,
	  PRE::prefix_reformatter_stack );

    min::locatable_gen sentence_name
        ( min::new_str_gen ( "sentence" ) );
    PAR::push_reformatter
        ( sentence_name, 0, 0, 1000,
	  ::sentence_reformatter_function,
	  PRE::prefix_reformatter_stack );
}
static min::initializer prefix_reformatter_initializer
    ( ::prefix_reformatter_stack_initialize );


// Prefix Compact Function
// ------ ------- --------

void PRE::compact_prefix_list
	( PAR::parser parser,
	  PAR::pass pass,
	  PAR::table::flags selectors,
	  PAR::token first,
	  PAR::token next,
          const min::position & separator_found,
	  min::gen separator,
	  TAB::flags trace_flags )
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

    bool merge_purelist =
        ( current->next == next
	  &&
	  current->type == PAR::PURELIST );

    bool implied =
        ( first->type == PAR::IMPLIED_PREFIX
	  ||
	  first->type == PAR::IMPLIED_HEADER );

    min::uns32 size = 0;
        // Number vector elements to add to prefix.
    min::obj_vec_ptr vp2;
        // vp to purelist, or NULL.
    if ( merge_purelist )
    {
        vp2 = current->value;
	size += min::attr_size_of ( vp2 );
    }
    else for ( PAR::token t = current;
	       t != next; t = t->next )
	    ++ size;

    if ( implied )
    {
	PAR::value_ref(first) =
	    min::copy
	        ( vp,   size
		      + ( separator_found ? 5 : 0 ) );
	vp = first->value;
    }

    min::attr_insptr ap ( vp );

    if ( separator_found )
    {
	min::locate ( ap, min::dot_terminator );
	min::set ( ap, separator );
	first->position.end = separator_found;
    }

    if ( merge_purelist )
    {
	min::attr_ptr ap2 ( vp );
	min::locate ( ap, min::dot_position );
	pos = min::get ( ap );
	for ( min::uns32 i = 0;
	      i < min::attr_size_of ( vp2 ); ++ i )
	    min::attr_push(vp) = min::attr ( vp2, i );
	current = current->next;
	PAR::free
	    ( PAR::remove
		  ( PAR::first_ref (parser),
		    current->previous ) );
    }
    else
    {
	min::locate ( ap, min::dot_position );
	if ( implied )
	{
	    min::init ( pos, parser->input_file,
			first->position, size );
	    min::set ( ap, min::new_stub_gen ( pos ) );
	    min::set_flag
		( ap, min::standard_attr_hide_flag );
	}
	else
	    pos = min::get ( ap );

	while ( current != next )
	{
	    if (   ( 1 << current->type )
		 & LEXSTD::convert_mask )
		PAR::convert_token ( current );

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
		      ( PAR::first_ref (parser),
			current->previous ) );
	}
    }

    pos->position = first->position;
    first->type = PAR::BRACKETED;

    vp = min::NULL_STUB;
        // To allow trace to print object.
    PAR::trace_subexpression
        ( parser, first, trace_flags );
}
