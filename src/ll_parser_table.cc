// Layered Languages Parser Table Functions
//
// File:	ll_parser_table.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon May 15 03:50:26 EDT 2017
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Name Tables
//	Roots
//	Key Tables
//	Undefineds
//	Blocks
//	Lexeme Maps

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

// Key Tables
// --- ------

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
    key_table_type
        ( "ll::parser::table::key_table_type",
	   NULL, ::table_stub_disp );

TAB::key_table TAB::create_key_table ( uns32 length )
{
    // Check for power of two.
    //
    MIN_ASSERT ( ( length & ( length - 1 ) ) == 0,
                 "length argument is not a power of"
		 " 2" );

    TAB::key_table key_table =
        ::key_table_type.new_stub ( length );
    for ( uns32 i = 0; i < length; ++ i )
	min::push ( key_table ) = NULL_STUB;
    return key_table;
}

TAB::key_prefix TAB::find_key_prefix
	( min::gen key, TAB::key_table key_table,
	  bool create )
{

    // Set len to the number of elements in the key and
    // element[] to the vector of key elements.
    //
    bool is_label = min::is_lab ( key );
    min::unsptr len;
    if ( is_label )
    {
	len = min::lablen ( key );
	MIN_ASSERT ( len > 1,
	             "key argument is label of length"
		     " <= 1" );
    }
    else
	len = 1;
    min::gen element[len];
    if ( is_label )
	min::labncpy ( element, key, len );
    else element[0] = key;

    // Loop through the elements setting `previous'
    // to the key prefix found for the last element.
    //
    uns32 hash;
    uns32 table_len = key_table->length;
    uns32 mask = table_len - 1;
    MIN_REQUIRE ( ( table_len & mask ) == 0 );
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
	    MIN_REQUIRE ( 0 <= v && v < (1<<28) );
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
	TAB::key_prefix current =
	    key_table[hash & mask];
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
	    key_table[hash & mask] = kprefix;
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
	( min::gen key,
	  min::uns32 subtype,
	  TAB::flags selectors,
	  TAB::key_table key_table )
{
    TAB::key_prefix key_prefix =
        TAB::find_key_prefix ( key, key_table );
    if ( key_prefix == NULL_STUB ) return NULL_STUB;

    for ( TAB::root root = key_prefix->first;
          root != NULL_STUB; root = root->next )
    {
        if ( ( root->selectors & selectors ) != 0
	     &&
	        min::packed_subtype_of ( root )
	     == subtype )
	    return root;
    }
    return NULL_STUB;
}

void TAB::push
        ( TAB::key_table key_table, TAB::root entry )
{
    min::gen key = entry->label;
    TAB::key_prefix kprefix =
        find_key_prefix ( key, key_table, true );
    next_ref(entry) = kprefix->first;
    first_ref(kprefix) = entry;
}

TAB::root TAB::push_root
	( min::gen label,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  TAB::key_table key_table )
{
    TAB::root root = ::root_type.new_stub();
    label_ref ( root ) = label;
    root->selectors = selectors;
    root->block_level = block_level;
    root->position = position;
    TAB::push ( key_table, root );
    return root;
}

void TAB::end_block
	( TAB::key_table key_table, uns32 block_level,
	  uns64 & collected_key_prefixes,
	  uns64 & collected_entries )
{
    collected_key_prefixes = 0;
    collected_entries = 0;

    for ( min::uns32 i = 0; i < key_table->length;
                            ++ i )
    for ( TAB::key_prefix key_prefix = key_table[i];
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
    for ( min::uns32 i = 0; i < key_table->length;
                            ++ i )
    {
        TAB::key_prefix previous = NULL_STUB;
	for ( TAB::key_prefix current = key_table[i];
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
		    key_table[i] = current->next;
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

// Lexeme Maps
// ------ ----

static min::uns32 lexeme_map_entry_gen_disp[] = {
    min::DISP ( & TAB::lexeme_map_entry_struct::label ),
    min::DISP ( & TAB::lexeme_map_entry_struct
                     ::token_value ),
    min::DISP ( & TAB::lexeme_map_entry_struct
                     ::token_value_type ),
    min::DISP_END };

static min::uns32 lexeme_map_entry_stub_disp[] = {
    min::DISP ( & TAB::lexeme_map_entry_struct::next ),
    min::DISP_END };

static min::packed_struct_with_base
	<TAB::lexeme_map_entry_struct, TAB::root_struct>
    lexeme_map_entry_type 
	( "ll::parser::table::lexeme_map_entry_type",
	  ::lexeme_map_entry_gen_disp,
	  ::lexeme_map_entry_stub_disp );

static min::uns32 lexeme_map_disp[] =
    { 0, min::DISP_END };

static min::packed_vec<TAB::root>
    lexeme_map_type
        ( "ll::parser::lexeme_map_type",
	  NULL, ::lexeme_map_disp );

TAB::lexeme_map TAB::create_lexeme_map
        ( uns32 length )
{
    min::packed_vec_insptr<TAB::root> map =
        ::lexeme_map_type.new_stub();
    min::push ( map, length );
    return map;
}

void TAB::push_lexeme_map_entry
	( min::gen lexeme_name,
	  min::uns32 lexeme_type,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  min::gen token_value,
	  min::uns32 token_type,
	  min::gen token_value_type,
	  min::uns32 lexical_master,
	  TAB::lexeme_map lexeme_map )
{
    TAB::lexeme_map_entry e =
        ::lexeme_map_entry_type.new_stub();
    TAB::label_ref(e) = lexeme_name;
    e->selectors = selectors;
    e->block_level = block_level;
    e->position = position;
    TAB::token_value_ref(e) = token_value;
    e->token_type = token_type;
    TAB::token_value_type_ref(e) = token_value_type;
    e->lexical_master = lexical_master,

    MIN_REQUIRE ( lexeme_type < lexeme_map->length );
    TAB::next_ref(e) = lexeme_map[lexeme_type];
    lexeme_map[lexeme_type] = (TAB::root) e;
}

// Remove from the lexeme map all entries that have
// entry block_level > block_level argument.  Return the
// number of entries that were removed from the table.
// Undefined stack must be processed before this
// function is called.
//
void TAB::end_block
	( TAB::lexeme_map lexeme_map,
          uns32 block_level,
	  uns64 & collected_entries )
{
    for ( min::uns32 i = 0; i < lexeme_map->length;
                            ++ i )
    {
        for ( TAB::root e = lexeme_map[i];
	      e != min::NULL_STUB
	      &&
	      e->block_level > block_level;
	      e = e->next )
	{
	    lexeme_map[i] = e->next;
	    ++ collected_entries;
	}
    }
}
