// Layers Language Parser Table Functions
//
// File:	ll_parser_table.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Mar 24 08:43:46 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Roots
//	Key Prefixes
//	Brackets
//	Indentation Marks

// Usage and Setup
// ----- --- -----

# include <ll_parser_table.h>
# define TAB ll::parser::table

// Roots
// -----

static min::uns32 root_stub_disp[] = {
    min::DISP ( & TAB::root_struct::next ),
    min::DISP_END };

static min::packed_struct<TAB::root_struct>
    root_type 
	( "ll::parser::table::root_type",
	  NULL, ::root_stub_disp );

// Key Prefixes
// --- --------

static min::uns32 key_prefix_gen_disp[] = {
    min::DISP ( & TAB::key_prefix_struct::key_element ),
    min::DISP_END };

static min::uns32 key_prefix_stub_disp[] = {
    min::DISP ( & TAB::key_prefix_struct::previous ),
    min::DISP ( & TAB::key_prefix_struct::first ),
    min::DISP ( & TAB::key_prefix_struct::next ),
    min::DISP_END };

static min::packed_struct<TAB::key_prefix_struct>
    key_prefix_type
	( "ll::parser::table::key_prefix_type",
	  ::key_prefix_gen_disp,
	  ::key_prefix_stub_disp );

static min::uns32 table_gen_disp[] = {
    0, min::DISP_END };

static min::packed_vec<TAB::key_prefix>
    table_type
        ( "ll::parser::table::table_type",
	   ::table_gen_disp );

TAB::key_prefix TAB::find_key_prefix
	( min::gen key, TAB::table table, bool create )
{

    // Set len to the number of elements in the key and
    // element[] to the vector of key elements.
    //
    bool is_label = min::is_lab ( key );
    min::unsptr len;
    if ( is_label )
    {
	len = min::lablen ( key );
	MIN_ASSERT ( len > 1 );
    }
    else
	len = 1;
    min::gen element[len];
    if ( is_label )
	min::lab_of ( element, len, key );
    else element[0] = key;

    // Loop through the elements setting `previous'
    // to the key prefix found for the last element.
    //
    uns32 hash;
    uns32 table_len = table->length;
    uns32 mask = table_len - 1;
    MIN_ASSERT ( ( table_len & mask ) == 0 );
    TAB::key_prefix previous = NULL_STUB;
    for ( min::unsptr i = 0; i < len; ++ i )
    {
	// Compute element e and its hash ehash.
	//
	min::gen e = element[i];
	uns32 ehash;
	if ( min::is_str ( e ) )
	    ehash = min::strhash ( e );
	else if ( min::is_num ( e ) )
	{
	    int v = min::int_of ( e );
	    MIN_ASSERT ( 0 <= v && v < (1<<28) );
	    ehash = min::numhash ( e );
	}
	else
	    MIN_ABORT ( "bad key element type" );

	// Compute hash of this element's key prefix.
	//
	if ( i == 0 ) hash = ehash;
	else
	{
	    if ( i == 1 )
	        hash = min::labhash
		    ( min::labhash_initial, hash );
	    hash = min::labhash ( hash, ehash );
	}

	// Locate key prefix as `current'.  If none,
	// current == NULL_STUB and `last' is last
	// key prefix in hash list.
	//
	TAB::key_prefix last = NULL_STUB;
	TAB::key_prefix current = table[hash & mask];
	while ( current != NULL_STUB )
	{
	    if ( current->key_element == e
	         &&
		 current->previous == previous )
	        break;
	    last = current;
	    current = current->next;
	}
	if ( current != NULL_STUB )
	{
	    previous = current;
	    continue;
	}
	else if ( ! create )
	    return NULL_STUB;

	// Create key prefix for element[i] and
	// `previous'.  This becomes the key prefix
	// found for element[i].
	//
	TAB::key_prefix kprefix =
	    ::key_prefix_type.new_stub();
	if ( last == NULL_STUB )
	    table[hash & mask] = kprefix;
	else
	    next_ref(last) = kprefix;
	key_element_ref(kprefix) = e;
	previous_ref(kprefix) = previous;

	previous = kprefix;
    }

    return previous;
}

TAB::root TAB::find ( min::gen key, TAB::table table )
{
    TAB::key_prefix key_prefix =
        TAB::find_key_prefix ( key, table );
    if ( key_prefix == NULL_STUB ) return NULL_STUB;
    else return key_prefix->first;
}

void TAB::push
	( min::gen key, TAB::root entry,
	  TAB::table table )
{
    TAB::key_prefix kprefix =
        find_key_prefix ( key, table, true );
    next_ref(entry) = kprefix->first;
    first_ref(kprefix) = entry;
}

// Brackets
// --------

static min::uns32 opening_bracket_stub_disp[] = {
    min::DISP ( & TAB::opening_bracket_struct::next ),
    min::DISP ( & TAB::opening_bracket_struct
                     ::closing_bracket ),
    min::DISP_END };

static min::packed_struct_with_base
	<TAB::opening_bracket_struct, TAB::root_struct>
    opening_bracket_type
	( "ll::parser::table::opening_bracket_type",
	  NULL, ::opening_bracket_stub_disp );
const min::uns32 & TAB::OPENING_BRACKET =
    opening_bracket_type.subtype;

static min::uns32 closing_bracket_stub_disp[] = {
    min::DISP ( & TAB::closing_bracket_struct::next ),
    min::DISP ( & TAB::closing_bracket_struct
                     ::opening_bracket ),
    min::DISP_END };

static min::packed_struct_with_base
	<TAB::closing_bracket_struct, TAB::root_struct>
    closing_bracket_type
	( "ll::parser::table::closing_bracket_type",
	  NULL, ::closing_bracket_stub_disp );
const min::uns32 & TAB::CLOSING_BRACKET =
    closing_bracket_type.subtype;

void TAB::push_brackets
	( min::gen opening_label,
	  min::gen closing_label,
	  TAB::selectors selectors,
	  const TAB::new_selectors & new_selectors,
	  TAB::table table )
{
    min::locatable_var<TAB::opening_bracket> opening;
    opening = ::opening_bracket_type.new_stub();
    min::locatable_var<TAB::closing_bracket> closing;
    closing = ::closing_bracket_type.new_stub();
    label_ref(opening) = opening_label;
    label_ref(closing) = closing_label;
    closing_bracket_ref(opening) = closing;
    opening_bracket_ref(closing) = opening;
    opening->selectors = selectors;
    closing->selectors = selectors;
    opening->new_selectors = new_selectors;
    TAB::push ( opening_label, (TAB::root) opening,
                table );
    TAB::push ( closing_label, (TAB::root) closing,
                table );
}

// Indentation Marks
// ----------- -----

static min::uns32 indentation_mark_stub_disp[] = {
    min::DISP ( & TAB::indentation_mark_struct::next ),
    min::DISP ( & TAB::indentation_mark_struct
                     ::indentation_split ),
    min::DISP_END };

static min::packed_struct_with_base
	<TAB::indentation_mark_struct, TAB::root_struct>
    indentation_mark_type
	( "ll::parser::table::indentation_mark_type",
	  NULL, ::indentation_mark_stub_disp );
const min::uns32 & TAB::INDENTATION_MARK =
    indentation_mark_type.subtype;

static min::uns32 indentation_split_stub_disp[] = {
    min::DISP ( & TAB::indentation_split_struct
                     ::next ),
    min::DISP ( & TAB::indentation_split_struct
                     ::indentation_mark ),
    min::DISP_END };

static min::packed_vec
	<min::uns8,TAB::indentation_split_struct>
    indentation_split_type
	( "ll::parser::table::indentation_split_type",
	  NULL, NULL, NULL,
	  ::indentation_split_stub_disp );

static min::uns32 split_table_stub_disp[] = {
    0,
    min::DISP_END };

static min::packed_vec<TAB::indentation_split>
    split_table_type
	( "ll::parser::table::split_table_type",
	  NULL, ::split_table_stub_disp );

void TAB::push_indentation_mark
	( min::gen label,
	  TAB::selectors selectors,
	  const TAB::new_selectors & new_selectors,
	  TAB::table bracket_table,
	  TAB::split_table split_table )
{
    min::locatable_var<TAB::indentation_mark> imark;
    imark = ::indentation_mark_type.new_stub();
    label_ref(imark) = label;
    imark->selectors = selectors;
    imark->new_selectors = new_selectors;
    TAB::push ( label, (TAB::root) imark,
                bracket_table );
    if ( split_table != NULL_STUB )
    {
        MIN_ASSERT ( min::is_str ( label ) );
	min::str_ptr s ( label );
	min::unsptr length = min::strlen ( s );
        MIN_ASSERT ( length > 0 );
	min::locatable_var<TAB::indentation_split>
	    isplit;
	isplit = ::indentation_split_type.new_stub
			( length );
	min::push
	    ( isplit, length, (min::uns8 *) & s[0] );
	isplit->indentation_mark = imark;
	indentation_split_ref(imark) = isplit;

	min::uns8 lastb = s[length - 1];

	min::ptr<TAB::indentation_split> p =
	    & split_table[lastb];
	while ( * p != NULL_STUB
	        &&
		(*p)->length > length )
	    p = & next_ref ( * p );
	next_ref(isplit) = * p;
	* p = isplit;
    }
}
