// Layers Language Parser Table Functions
//
// File:	ll_parser_table.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Mar  6 19:15:12 EST 2011
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

// Usage and Setup
// ----- --- -----

# include <ll_parser_table.h>
# define PAR ll::parser
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

TAB::key_prefix_ptr TAB::find
	( min::gen key, table_ptr table, bool create )
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
    key_prefix_ptr previous = NULL_STUB;
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
	key_prefix_ptr last = NULL_STUB;
	key_prefix_ptr current = table[hash & mask];
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
	key_prefix_ptr kprefix =
	    ::key_prefix_type.new_stub();
	if ( last == NULL_STUB )
	    table[hash & mask] = kprefix;
	else
	    last->next = kprefix;
	kprefix->key_element = e;
	kprefix->previous = previous;

	previous = kprefix;
    }

    return previous;
}

void TAB::push
	( min::gen key, root_ptr entry,
	  table_ptr table )
{
    key_prefix_ptr kprefix =
        find ( key, table, true );
    entry->next = kprefix->first;
    kprefix->first = entry;
}


// Brackets
// --------

static min::uns32 opening_bracket_stub_disp[] = {
    min::DISP ( & TAB::opening_bracket_struct::next ),
    min::DISP ( & TAB::opening_bracket_struct
                     ::closing_bracket ),
    min::DISP_END };

min::packed_struct_with_base
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

min::packed_struct_with_base
	<TAB::closing_bracket_struct, TAB::root_struct>
    closing_bracket_type
	( "ll::parser::table::closing_bracket_type",
	  NULL, ::closing_bracket_stub_disp );
const min::uns32 & TAB::CLOSING_BRACKET =
    closing_bracket_type.subtype;
