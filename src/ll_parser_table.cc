// Layers Language Parser Table Functions
//
// File:	ll_parser_table.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Feb 22 10:59:00 EST 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Selectors
//	Roots
//	Key Prefixes
//	Brackets
//	Named Brackets
//	Indentation Marks

// Usage and Setup
// ----- --- -----

# include <ll_parser_table.h>
# define TAB ll::parser::table

// Selectors
// ---------

static min::uns32 selector_name_table_gen_disp[] = {
    0, min::DISP_END };

static min::packed_vec<min::gen>
    selector_name_table_type
        ( "ll::parser::table::selector_name_table_type",
	   ::selector_name_table_gen_disp );

TAB::selector_name_table
    TAB::create_selector_name_table ( void )
{
    return ::selector_name_table_type.new_stub ( 64 );
}


// Roots
// -----

static min::uns32 root_gen_disp[] = {
    min::DISP ( & TAB::root_struct::label ),
    min::DISP_END };

static min::uns32 root_stub_disp[] = {
    min::DISP ( & TAB::root_struct::next ),
    min::DISP_END };

static min::packed_struct<TAB::root_struct>
    root_type 
	( "ll::parser::table::root_type",
	  ::root_gen_disp, ::root_stub_disp );

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

static min::uns32 table_stub_disp[] = {
    0, min::DISP_END };

static min::packed_vec<TAB::key_prefix>
    table_type
        ( "ll::parser::table::table_type",
	   NULL, ::table_stub_disp );

TAB::table TAB::create_table ( uns32 length )
{
    // Check for power of two.
    //
    assert ( ( length & ( length -1 ) ) == 0 );

    return ::table_type.new_stub ( length );
}

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

void TAB::push ( TAB::table table, TAB::root entry )
{
    min::gen key = entry->label;
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
	  ::root_gen_disp,
	  ::opening_bracket_stub_disp );
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
	  ::root_gen_disp,
	  ::closing_bracket_stub_disp );
const min::uns32 & TAB::CLOSING_BRACKET =
    closing_bracket_type.subtype;

void TAB::push_brackets
	( min::gen opening_label,
	  min::gen closing_label,
	  TAB::selectors selectors,
	  const TAB::new_selectors & new_selectors,
	  bool full_line,
	  TAB::table bracket_table )
{
    min::locatable_var<TAB::opening_bracket> opening
        ( ::opening_bracket_type.new_stub() );
    min::locatable_var<TAB::closing_bracket> closing
        ( ::closing_bracket_type.new_stub() );

    label_ref(opening) = opening_label;
    label_ref(closing) = closing_label;

    closing_bracket_ref(opening) = closing;
    opening_bracket_ref(closing) = opening;

    opening->selectors = selectors;
    closing->selectors = selectors;

    opening->new_selectors = new_selectors;
    opening->full_line = full_line;

    TAB::push ( bracket_table, (TAB::root) opening );
    TAB::push ( bracket_table, (TAB::root) closing );
}

// Named Brackets
// ----- --------

static min::uns32 named_opening_stub_disp[] = {
    min::DISP ( & TAB::named_opening_struct
                     ::next ),
    min::DISP ( & TAB::named_opening_struct
                     ::named_separator ),
    min::DISP ( & TAB::named_opening_struct
                     ::named_middle ),
    min::DISP ( & TAB::named_opening_struct
                     ::named_closing ),
    min::DISP ( & TAB::named_opening_struct
                     ::named_middle_closing ),
    min::DISP_END };

static min::packed_struct_with_base
	<TAB::named_opening_struct, TAB::root_struct>
    named_opening_type
	( "ll::parser::table::named_opening_type",
	  ::root_gen_disp,
	  ::named_opening_stub_disp );
const min::uns32 & TAB::NAMED_OPENING =
    named_opening_type.subtype;

static min::uns32 named_separator_stub_disp[] = {
    min::DISP ( & TAB::named_separator_struct
    		     ::next ),
    min::DISP ( & TAB::named_separator_struct
                     ::named_opening ),
    min::DISP_END };

static min::packed_struct_with_base
	<TAB::named_separator_struct, TAB::root_struct>
    named_separator_type
	( "ll::parser::table::named_separator_type",
	  ::root_gen_disp,
	  ::named_separator_stub_disp );
const min::uns32 & TAB::NAMED_SEPARATOR =
    named_separator_type.subtype;

static min::uns32 named_middle_stub_disp[] = {
    min::DISP ( & TAB::named_middle_struct
                     ::next ),
    min::DISP ( & TAB::named_middle_struct
                     ::named_opening ),
    min::DISP_END };

static min::packed_struct_with_base
	<TAB::named_middle_struct, TAB::root_struct>
    named_middle_type
	( "ll::parser::table::named_middle_type",
	  ::root_gen_disp,
	  ::named_middle_stub_disp );
const min::uns32 & TAB::NAMED_MIDDLE =
    named_middle_type.subtype;

static min::uns32 named_closing_stub_disp[] = {
    min::DISP ( & TAB::named_closing_struct
                     ::next ),
    min::DISP ( & TAB::named_closing_struct
                     ::named_opening ),
    min::DISP_END };

static min::packed_struct_with_base
	<TAB::named_closing_struct, TAB::root_struct>
    named_closing_type
	( "ll::parser::table::named_closing_type",
	  ::root_gen_disp,
	  ::named_closing_stub_disp );
const min::uns32 & TAB::NAMED_CLOSING =
    named_closing_type.subtype;

static min::uns32 named_middle_closing_stub_disp[] = {
    min::DISP ( & TAB::named_middle_closing_struct
	             ::next ),
    min::DISP ( & TAB::named_middle_closing_struct
                     ::named_opening ),
    min::DISP_END };

static min::packed_struct_with_base
	<TAB::named_middle_closing_struct,
	 TAB::root_struct>
    named_middle_closing_type
	( "ll::parser::table"
	    "::named_middle_closing_type",
	  ::root_gen_disp,
	  ::named_middle_closing_stub_disp );
const min::uns32 & TAB::NAMED_MIDDLE_CLOSING =
    named_middle_closing_type.subtype;

void TAB::push_named_brackets
	( min::gen named_opening_label,
	  min::gen named_separator_label,
	  min::gen named_middle_label,
	  min::gen named_closing_label,
	  min::gen named_middle_closing_label,
	  TAB::selectors selectors,
	  TAB::table bracket_table )
{
    min::locatable_var<TAB::named_opening>
        named_opening
        ( ::named_opening_type.new_stub() );

    min::locatable_var<TAB::named_separator>
        named_separator
        ( named_separator_label != min::MISSING() ?
	  ::named_separator_type.new_stub() :
	  min::NULL_STUB );

    min::locatable_var<TAB::named_middle>
        named_middle
        ( named_middle_label != min::MISSING() ?
	  ::named_middle_type.new_stub() :
	  min::NULL_STUB );

    min::locatable_var<TAB::named_closing>
        named_closing
        ( ::named_closing_type.new_stub() );

    min::locatable_var
	    <TAB::named_middle_closing>
        named_middle_closing
        ( named_middle_closing_label != min::MISSING() ?
          ::named_middle_closing_type.new_stub() :
	  min::NULL_STUB );

    label_ref(named_opening) = named_opening_label;
    named_opening->selectors = selectors;
    named_separator_ref(named_opening) =
        named_separator;
    named_middle_ref(named_opening) =
        named_middle;
    named_closing_ref(named_opening) =
        named_closing;
    named_middle_closing_ref(named_opening) =
        named_middle_closing;
    TAB::push ( bracket_table,
                (TAB::root) named_opening );

    label_ref(named_closing) = named_closing_label;
    named_closing->selectors = selectors;
    named_opening_ref(named_closing) =
        named_opening;
    TAB::push ( bracket_table,
                (TAB::root) named_closing );

    if ( named_separator_label != min::MISSING() )
    {
	label_ref(named_separator) = named_separator_label;
	named_separator->selectors = selectors;
	named_opening_ref(named_separator) =
	    named_opening;
	TAB::push ( bracket_table,
		    (TAB::root) named_separator );
    }

    if ( named_middle_label != min::MISSING() )
    {
	label_ref(named_middle) = named_middle_label;
	named_middle->selectors = selectors;
	named_opening_ref(named_middle) =
	    named_opening;
	TAB::push ( bracket_table,
		    (TAB::root) named_middle );
    }

    if ( named_middle_closing_label != min::MISSING() )
    {
	label_ref(named_middle_closing) =
	    named_middle_closing_label;
	named_middle_closing->selectors = selectors;
	named_opening_ref(named_middle_closing) =
	    named_opening;
	TAB::push ( bracket_table,
		    (TAB::root) named_middle_closing );
    }
}

// Indentation Marks
// ----------- -----

static min::uns32 indentation_mark_stub_disp[] = {
    min::DISP ( & TAB::indentation_mark_struct::next ),
    min::DISP ( & TAB::indentation_mark_struct
                     ::indentation_separator ),
    min::DISP ( & TAB::indentation_mark_struct
                     ::indentation_split ),
    min::DISP_END };

static min::packed_struct_with_base
	<TAB::indentation_mark_struct, TAB::root_struct>
    indentation_mark_type
	( "ll::parser::table::indentation_mark_type",
	  ::root_gen_disp,
	  ::indentation_mark_stub_disp );
const min::uns32 & TAB::INDENTATION_MARK =
    indentation_mark_type.subtype;

static min::uns32 indentation_separator_stub_disp[] = {
    min::DISP ( & TAB::indentation_separator_struct
                     ::next ),
    min::DISP ( & TAB::indentation_separator_struct
                     ::indentation_mark ),
    min::DISP_END };

static min::packed_struct_with_base
	<TAB::indentation_separator_struct,
	 TAB::root_struct>
    indentation_separator_type
	( "ll::parser::table"
	    "::indentation_separator_type",
	  ::root_gen_disp,
	  ::indentation_separator_stub_disp );
const min::uns32 & TAB::INDENTATION_SEPARATOR =
    indentation_separator_type.subtype;

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
    0, min::DISP_END };

static min::packed_vec<TAB::indentation_split>
    split_table_type
	( "ll::parser::table::split_table_type",
	  NULL, ::split_table_stub_disp );

min::locatable_var<TAB::indentation_mark>
    TAB::top_level_indentation_mark;

TAB::split_table TAB::create_split_table ( void )
{
    return ::split_table_type.new_stub ( 256 );
}

void TAB::push_indentation_mark
	( min::gen mark_label,
	  min::gen separator_label,
	  TAB::selectors selectors,
	  const TAB::new_selectors & new_selectors,
	  TAB::table bracket_table,
	  TAB::split_table split_table )
{
    min::locatable_var<TAB::indentation_mark> imark
        ( ::indentation_mark_type.new_stub() );
    label_ref(imark) = mark_label;
    imark->selectors = selectors;
    imark->new_selectors = new_selectors;
    TAB::push ( bracket_table, (TAB::root) imark );

    if ( separator_label != min::MISSING() )
    {
	min::locatable_var<TAB::indentation_separator>
	    separator
	    ( ::indentation_separator_type.new_stub() );
	label_ref(separator) = separator_label;
	separator->selectors = selectors;
	indentation_mark_ref(separator) = imark;
	indentation_separator_ref(imark) = separator;
	TAB::push ( bracket_table,
	            (TAB::root) separator );
    }

    if ( split_table != NULL_STUB )
    {
        MIN_ASSERT ( min::is_str ( mark_label ) );
	min::str_ptr s ( mark_label );
	min::unsptr length = min::strlen ( s );
	min::locatable_var<TAB::indentation_split>
	    isplit;
	isplit = ::indentation_split_type.new_stub
			( length );
	min::push
	    ( isplit, length,
	      (min::uns8 *)
	      (const char *) min::begin_ptr_of ( s ) );
	indentation_mark_ref(isplit) = imark;
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
