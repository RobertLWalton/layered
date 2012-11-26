// Layers Language Parser Table Functions
//
// File:	ll_parser_table.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Nov 26 10:04:18 EST 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Name Tables
//	Roots
//	Key Prefixes
//	Undefineds
//	Blocks

// Usage and Setup
// ----- --- -----

# include <ll_parser_table.h>
# define MUP min::unprotected
# define TAB ll::parser::table

// Name Tables
// ---- ------

static min::uns32 name_table_gen_disp[] = {
    0, min::DISP_END };

static min::packed_vec<min::gen>
    name_table_type
        ( "ll::parser::table::name_table_type",
	   ::name_table_gen_disp );

void TAB::init_name_table
	( min::ref<TAB::name_table> name_table,
	  min::uns32 max_length )
{
    if ( name_table == min::NULL_STUB )
        name_table =
	     ::name_table_type.new_stub ( max_length );
}


// Roots
// -----

min::uns32 TAB::root_gen_disp[] = {
    min::DISP ( & TAB::root_struct::label ),
    min::DISP_END };

static min::uns32 root_stub_disp[] = {
    min::DISP ( & TAB::root_struct::next ),
    min::DISP_END };

static min::packed_struct<TAB::root_struct>
    root_type 
	( "ll::parser::table::root_type",
	  TAB::root_gen_disp, ::root_stub_disp );

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

    TAB::table table = ::table_type.new_stub ( length );
    for ( uns32 i = 0; i < length; ++ i )
	min::push ( table ) = NULL_STUB;
    return table;
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
	if ( previous != NULL_STUB )
	    ++ previous->reference_count;

	previous = kprefix;
    }

    return previous;
}

TAB::root TAB::find
	( min::gen key, TAB::flags selectors,
	  TAB::table table )
{
    TAB::key_prefix key_prefix =
        TAB::find_key_prefix ( key, table );
    if ( key_prefix == NULL_STUB ) return NULL_STUB;

    for ( TAB::root root = key_prefix->first;
          root != NULL_STUB; root = root->next )
    {
        if ( root->selectors & selectors )
	    return root;
    }
    return NULL_STUB;
}

void TAB::push ( TAB::table table, TAB::root entry )
{
    min::gen key = entry->label;
    TAB::key_prefix kprefix =
        find_key_prefix ( key, table, true );
    next_ref(entry) = kprefix->first;
    first_ref(kprefix) = entry;
}

void TAB::end_block
	( TAB::table table, uns32 block_level,
	  uns64 & collected_key_prefixes,
	  uns64 & collected_entries )
{
    collected_key_prefixes = 0;
    collected_entries = 0;

    for ( min::uns32 i = 0; i < table->length; ++ i )
    for ( TAB::key_prefix key_prefix = table[i];
          key_prefix != NULL_STUB;
	  key_prefix = key_prefix->next )
    {
        TAB::root root = key_prefix->first;
	while ( root != NULL_STUB )
	{
	    if ( root->block_level <= block_level )
	        break;
	    ++ collected_entries;
	    first_ref ( key_prefix ) = root =
	        root->next;
	}

	// Decrement reference counts.
	//
	TAB::key_prefix collectible = key_prefix;
	while ( collectible->first == NULL_STUB
	        &&
	        collectible->reference_count == 0 )
	{
	    ++ collected_key_prefixes;
	    collectible = collectible->previous;

	    if ( collectible == NULL_STUB ) break;

	    -- collectible->reference_count;
	}
    }

    if ( collected_key_prefixes > 0 )
    for ( min::uns32 i = 0; i < table->length; ++ i )
    {
        TAB::key_prefix previous = NULL_STUB;
	for ( TAB::key_prefix current = table[i];
	      current != NULL_STUB;
	      current = current->next )
	{
	    if ( current->reference_count == 0
	         &&
		 current->first == NULL_STUB )
	    {
	        // Collect current.
		//
		if ( previous == NULL_STUB )
		    table[i] = current->next;
		else
		    next_ref ( previous ) =
		        current->next;
	    }
	    else
	        previous = current;
		    // Do NOT collect current.
	}
    }
}

// Undefineds
// ----------

static min::uns32 undefined_stack_stub_disp[] = {
    min::DISP ( & TAB::undefined_struct::root ),
    min::DISP_END };

static min::packed_vec<TAB::undefined_struct>
    undefined_stack_type 
	( "ll::parser::table::undefined_stack_type",
	  NULL, ::undefined_stack_stub_disp );

void TAB::init_undefined_stack
	( min::ref<TAB::undefined_stack>
	      undefined_stack,
	  min::uns32 max_length )
{
    if ( undefined_stack == min::NULL_STUB )
        undefined_stack =
	     ::undefined_stack_type
	         .new_stub ( max_length );
}



// Blocks
// ------

static min::uns32 block_stack_gen_disp[] = {
    min::DISP ( & TAB::block_struct::name ),
    min::DISP_END };

static min::packed_vec<TAB::block_struct>
    block_stack_type 
	( "ll::parser::table::block_stack_type",
	   ::block_stack_gen_disp );

void TAB::init_block_stack
	( min::ref<TAB::block_stack> block_stack,
	  min::uns32 max_length )
{
    if ( block_stack == min::NULL_STUB )
        block_stack =
	     ::block_stack_type
	         .new_stub ( max_length );
}
