// Layered Languages Parser Table
//
// File:	ll_parser_table.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Aug  9 04:22:06 PM EDT 2024
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Flags
//	Name Tables
//	Roots
//	Key Tables
//	Undefineds
//	Blocks
//	Lexeme Maps

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_TABLE_H
# define LL_PARSER_TABLE_H

# include <min.h>

namespace ll { namespace parser {

namespace bracketed {

    // We this small piece of ll_parser_bracketed.h
    // is needed here.
    //
    struct indentation_mark_struct;
    typedef min::packed_struct_updptr
		<indentation_mark_struct>
	    indentation_mark;
}

namespace table {

    using min::uns8;
    using min::uns16;
    using min::uns32;
    using min::uns64;
    using min::NULL_STUB;

// Flags
// -----

typedef min::uns64 flags;

struct new_flags
     // Encoding of a flag modification list.  The
     // modification operation is
     //
     // new-flags = (   ( old-flags | or_flags )
     //		      & ~ not_flags ) ^ xor_flags
     //
     // where
     //		( or_flags & not_flags ) = 0
     //		( or_flags & xor_flags ) = 0
     //		( not_flags & xor_flags ) = 0
     //
     //	Can also encode a flag list where
     //
     //	    or_flags = flags; not_flags = ~ flags;
     //	    xor_flags = 0
{
    ll::parser::table::flags or_flags;
    ll::parser::table::flags not_flags;
    ll::parser::table::flags xor_flags;

    new_flags ( flags or_flags = 0,
                flags not_flags = 0,
                flags xor_flags = 0 )
	: or_flags ( or_flags ),
	  not_flags ( not_flags ),
	  xor_flags ( xor_flags ) {}
};

const flags ALL_FLAGS = (flags) -1ll;

inline ll::parser::table::flags all_flags
    ( const ll::parser::table::new_flags & new_flags,
      const ll::parser::table::flags & on_flags = 0,
      const ll::parser::table::flags & off_flags = 0 )
{
    return   ( new_flags.or_flags & ~ on_flags )
           | ( new_flags.not_flags & ~ off_flags )
           | new_flags.xor_flags;
}

inline ll::parser::table::flags modified_flags
	( ll::parser::table::flags f,
	  ll::parser::table::new_flags const & nf )
{
    f |= nf.or_flags;
    f &= ~ nf.not_flags;
    f ^= nf.xor_flags;
    return f;
}

// Name Tables
// ---- ------

typedef min::packed_vec_ptr<min::gen>
	name_table;
typedef min::packed_vec_insptr<min::gen>
	name_table_insptr;

// Initialize a flag name table with given max_length
// and length = 0.
//
void init_name_table
    ( min::ref<ll::parser::table::name_table>
	  name_table,
      min::uns32 max_length = 64 );

// Return the index associated with a name in a flag
// name table, or return -1 if none.  If there is more
// then one index associated with the name, return the
// largest such index.
//
inline int get_index
    ( ll::parser::table::name_table name_table,
      min::gen name )
{
    int i = name_table->length;
    while ( -- i >= 0 && name_table[i] != name );
    return i;
}

// Push into a flag name table (overcoming the fact
// that its readonly).  Name argument must be protected
// from garbage collection.  Returns index of name.
//
inline int push_name
	( ll::parser::table::name_table table,
	  min::gen name )
{
    int result = table->length;
    min::push
        ( (ll::parser::table::name_table_insptr)
	  table ) = name;
    return result;
}

// Same as get_index if name is already in table, or
// push_name if name is not already in table.
// Returns index of name.
//
inline int find_name
	( ll::parser::table::name_table table,
	  min::gen name )
{
    int result = get_index ( table, name );
    if ( result == -1 )
        result = push_name ( table, name );
    return result;
}

// Pop a flag name table (overcoming the fact that its
// readonly).
//
inline min::gen pop_name
	( ll::parser::table::name_table table )
{
    return min::pop
        ( (ll::parser::table::name_table_insptr)
	  table );
}


// Roots
// -----

// Key prefix forward reference.
//
struct key_prefix_struct;
typedef min::packed_struct_updptr<key_prefix_struct>
        key_prefix;

// All key table entries begin with a root.
//
struct root_struct;
typedef min::packed_struct_updptr<root_struct>
        root;
extern min::uns32 root_gen_disp[];
struct root_struct
{
    uns32 control;
    	// Packed structure control word.
	//
    uns32 block_level;
        // Parser block_level at time entry was
	// defined.  For a key table used for
	// primaries:
	//     ( lexical-level << 16 ) + depth
	//
	// When an entry is added to a key table,
	// its block_level must NOT BE LESS than
	// the block_level of any entry already
	// in the table.  Thus when entries are
	// removed in reverse order of their addition
	// the block_levels of the removed entries
	// will be non-increasing.

    const ll::parser::table::root next;
        // Next entry in the stack with the same key.
	// NULL_STUB if no next entry.

    const ll::parser::table::key_prefix last;
        // Save of the value of key_table->last when
	// this entry is pushed into the key_table.
	// Used to remove entries in the reverse order
	// in which they were added (a.k.a., `pushed').

    ll::parser::table::flags selectors;
        // Selector bits.
    const min::gen label;
        // Label of key table entry.  A MIN label whose
	// elements are the symbols of entry's key.
	//
	// The `label' is also called the `key' of a
	// key table entry.
    min::phrase_position position;
        // Definition position.
};

MIN_REF ( ll::parser::table::root, next,
          ll::parser::table::root )
MIN_REF ( ll::parser::table::key_prefix, last,
          ll::parser::table::root )
MIN_REF ( min::gen, label,
          ll::parser::table::root )

// Key Tables
// --- ------

// A key table maps keys, which are labels (see Roots
// above), to lists of key table entries.  A key is a
// sequence of one or more symbols, each represented
// in the key table by a min::gen string value or
// min::gen number value and in the input by a symbol
// token whose value is a min::gen string or min::gen
// number.  The list of entries for a key is in most
// recently added (to the key table) first order.
//
// The entire key table is also itself a stack, in the
// sense that entries can be removed in reverse order
// from that in which they were added.
//
// A key prefix is an initial segment of a key, viewing
// the key as a list of symbols.
//
// A key table more specifically takes a key prefix and
// finds a key_prefix structure whose key is the key
// prefix.  The key_prefix structure points at the list
// of key table entries whose labels equal the key of
// the key prefix structure, if there are such entries.
// The entries are in most recently added (to the key
// table) first order.
//
// Thus if there is a key table entry with label
// [A B C], there will be key_prefix structures with
// keys A, [A B], and [A B C], but only the last
// necessarily points at a non-empty list of key table
// entries, which would consist of all entries with
// label [A B C].
//
// The key K of a key prefix is not stored directly in
// the key prefix structure.  Rather the last `key_
// element' of K is stored, along with a `previous'
// pointer that points to the key prefix whose key is
// the all the key elements of K but the last.
//
// The hash code of a key_prefix is the hash code of its
// key.  More specifically, if the key has just one
// element, which must necessarily be a symbol (i.e., a
// min::gen string), the hash is the hash of that
// element, and if the key has more than one element,
// the hash is the hash as per min::labhash of the min::
// gen label made from the key elements.
//
struct key_prefix_struct;
typedef min::packed_struct_updptr<key_prefix_struct>
        key_prefix;
struct key_prefix_struct
{
    uns32 control;
	// Packed_struct subtype is KEY_PREFIX.

    uns32 reference_count;
        // Number of other key_prefixes in this key
	// table whose previous value equals this key
	// prefix.  This key_prefix can be garbage
	// collected when its reference_count == 0 and
	// its first == NULL_STUB.

    const min::gen key_element;
        // Last element of the key of this key prefix.
	// Either a MIN string or a MIN number.
    const ll::parser::table::key_prefix previous;
        // Entry for key prefix whose key consists of
	// all the key elements of this prefix but the
	// last, or NULL_STUB if none (because the key
	// of this key prefix has only one element).

    const ll::parser::table::key_prefix next;
        // Next key prefix in the key prefix's hash
	// list.  A key table is a vector of key
	// prefix hash lists, with the hash of a
	// key prefix's key determining the vector
	// index of the key prefix's hash list.

    const ll::parser::table::root first;
        // First key table entry whose label equals
	// the key of this key prefix, or NULL_STUB
	// if none.  The keys are in most recently
	// added first order.
};

MIN_REF ( min::gen, key_element,
          ll::parser::table::key_prefix )
MIN_REF ( ll::parser::table::key_prefix, previous,
          ll::parser::table::key_prefix )
MIN_REF ( ll::parser::table::root, first,
          ll::parser::table::key_prefix )
MIN_REF ( ll::parser::table::key_prefix, next,
          ll::parser::table::key_prefix )

// A key table is just a vector of key_prefix values.
// The `length' of this vector MUST BE a power of two.
//
// The table also records the key_prefix of the last
// key table entry to be added to the table.
//
struct key_table_header;
typedef min::packed_vec_insptr
	    <ll::parser::table::key_prefix,
	     ll::parser::table::key_table_header>
    key_table;
struct key_table_header
{
    const min::uns32 control;
    const min::uns32 length;
    const min::uns32 max_length;
    const ll::parser::table::key_prefix last;
        // The last key_prefix in the table into which
	// an element was pushed.  Key_table->last->next
	// is the last key table entry added to the
	// table.
	//
	// When an entry is added to the table, this
	// value as it was just before the addition is
	// saved in the entry, and when an entry is
	// removed, the value saved is restored.
};
MIN_REF ( ll::parser::table::key_prefix, last,
          ll::parser::table::key_table )

// Create a key table of given length which MUST BE a
// power of two.
//
ll::parser::table::key_table create_key_table
        ( uns32 length );

// Return the key table key_prefix with the given key.
// If none and `create' is true, create the key_prefix.
// If none and `create' is false, return NULL_STUB.
//
// Also garbage collects any key_prefix found with
// 0 reference_count and NULL_STUB `first'.  Garbage
// collection is done here because this function must
// traverse the key's hash list from the beginning
// and can easily delete a key prefix from that list
// if the key prefix is garbage.
//
ll::parser::table::key_prefix find_key_prefix
	( min::gen key,
	  ll::parser::table::key_table key_table,
	  bool create = false );

// Return the first key table entry with the given
// key, subtype, and a selector in common with the
// selectors argument, or NULL_STUB if there is none.
//
ll::parser::table::root find
	( min::gen key,
	  min::uns32 subtype,
	  ll::parser::table::flags selectors,
	  ll::parser::table::key_table key_table );

// Push (a.k.a `add') the given key table entry into
// the key table.
//
void push ( ll::parser::table::key_table key_table,
            ll::parser::table::root entry );

// Create a pure entry (a root that as no extension)
// and push it into a key table.
//
// Used to map keys to selectors (as for the group name
// table).
//
ll::parser::table::root
    push_root
	( min::gen label,
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  ll::parser::table::key_table key_table );

// Return the root at the top of the key_table stack,
// or return NULL_STUB if the table is empty.
//
inline ll::parser::table::root
    top ( ll::parser::table::key_table key_table )
{
    ll::parser::table::key_prefix key_prefix =
        key_table->last;
    if ( key_prefix == min::NULL_STUB )
        return min::NULL_STUB;
    else
        return key_prefix->first;
}

// Pop the most recent root entry from the key_table.
// Key_table must NOT be empty.
//
inline void pop
	( ll::parser::table::key_table key_table )
{
    ll::parser::table::key_prefix key_prefix =
        key_table->last;
    MIN_REQUIRE ( key_prefix != min::NULL_STUB );
    ll::parser::table::root root = key_prefix->first;
    ll::parser::table::last_ref(key_table) =
    	root->last;
    ll::parser::table::first_ref(key_prefix) =
        root->next;
}

// Return the root that is just before the root argument
// in its key_table stack, or return NULL_STUB if the
// root argument is at the bottom of the stack.
//
inline ll::parser::table::root previous
	( ll::parser::table::root root )
{
    ll::parser::table::key_prefix key_prefix =
        root->last;
    if ( key_prefix == min::NULL_STUB )
    	return min::NULL_STUB;
    ll::parser::table::root result = key_prefix->first;
    MIN_REQUIRE ( result != min::NULL_STUB );
    return result;
}
    

// Remove from the key_table all entries that have
// entry->block_level >= block_level argument.  Return
// the numbers of entries and key prefixes that were
// `garbage collected', i.e., removed from table.
//
// Removal of entries is done one entry at a time most
// recently added entry first.  It is assumed that any
// newly added entry has a block_level at least as
// great as any entry in the table at the time of the
// addition, so entry block levels can only decrease
// when entries are repeatedly removed from the top
// of the key table stack.
//
// Actual garbage collection of key prefixes is done by
// find_key_prefix.
//
void end_block
	( ll::parser::table::key_table key_table,
          uns32 block_level,
	  uns64 & collected_key_prefixes,
	  uns64 & collected_entries );

// Iterator that finds all the entries in a key_table.
// Must be created in a stack frame.  Assumes that the
// key table is not changed or garbage collected while
// the iterator is in use (but it may be compacted).
//
// The iterator iterates first over key prefixes and
// then over entries with the same key prefix.  It does
// NOT present entries in key table stack order.
//
struct key_table_iterator
{
    ll::parser::table::key_table key_table;
        // Key table.
    ll::parser::table::root root;
        // Value returned by next().
    ll::parser::table::key_prefix key_prefix;
        // Next key_prefix to use.  key_prefix.first
	// must NOT be NULL_STUB.  NULL_STUB if no more
	// key_prefixes with non-NULL_STUB .first in
	// table.
    min::uns32 index;
        // Index of next element in key_table to use.
	// >= key_table->length if no more elements
	// to use.

    // Return next entry in table, and update iterator
    // to point at the entry following that.  Returns
    // min::NULL_STUB if no more entries.
    //
    ll::parser::table::root next ( void )
    {
        ll::parser::table::root result = root;
	if ( root == min::NULL_STUB ) return result;
	root = root->next;
	if ( root != min::NULL_STUB ) return result;
	if ( key_prefix == min::NULL_STUB )
	    return result;
	root = key_prefix->first;
	while ( true )
	{
	    key_prefix = key_prefix->next;
	    while ( key_prefix == min::NULL_STUB )
	    {
	        if ( index >= key_table->length )
		    return result;
		key_prefix = key_table[index++];
	    }
	    if ( key_prefix->first != min::NULL_STUB )
	        return result;
	}
    }

    // Construct an iterator.
    //
    key_table_iterator
	    ( ll::parser::table::key_table key_table )
	: key_table ( key_table )
    {
	root = min::NULL_STUB;
        for ( index = 0; index < key_table->length; )
	{
	    key_prefix = key_table[index++];
	    while ( key_prefix != min::NULL_STUB )
	    {
		if ( root == min::NULL_STUB )
		    root = key_prefix->first;
		else if (    key_prefix->first
		          != min::NULL_STUB )
		    return;
	        key_prefix = key_prefix->next;
	    }
	}
    }
};


// Undefineds
// ----------

struct undefined_struct
    // Records the zeroing of a root selectors field.
{
    ll::parser::table::root root;
        // root->selectors is zeroed.

    ll::parser::table::flags saved_selectors;
        // Value of root->selectors before it is zeroed.
};

typedef min::packed_vec_insptr
            <ll::parser::table::undefined_struct>
	undefined_stack;

// Initialize an undefined stack with given max_length
// and length = 0.
//
void init_undefined_stack
    ( min::ref<ll::parser::table::undefined_stack>
	  undefined_stack,
      min::uns32 max_length = 64 );

inline void push_undefined
	( ll::parser::table::undefined_stack
	      undefined_stack,
	  ll::parser::table::root root,
	  ll::parser::table::flags excluded_selectors )
{
    ll::parser::table::undefined_struct u =
        { root, root->selectors };
    min::push ( undefined_stack ) = u;
    min::unprotected::acc_write_update
        ( undefined_stack, root );
    root->selectors &= ~ excluded_selectors;
}

// Blocks
// ------

struct block_struct
    // Records information about a block.
{
    min::gen name;
        // Block name.

    min::uns32 saved_selector_name_table_length;
        // Selector_name_table length when block begins.

    min::uns32 saved_undefined_stack_length;
        // Undefined_stack length when block begins.
    
    ll::parser::bracketed::indentation_mark
        saved_top_level_indentation_mark;
        // Top level indentation mark when block begins.

    ll::parser::table::flags saved_input_flags;
    	// Top level input flags when block begins.

    ll::parser::table::flags saved_trace_flags;
    	// Top level trace flags when block begins.

    min::Uchar saved_ID_character;
    	// ID_character when block begins.
};

typedef min::packed_vec_insptr
            <ll::parser::table::block_struct>
	block_stack;

// Initialize a block stack with given max_length
// and length = 0.
//
void init_block_stack
    ( min::ref<ll::parser::table::block_stack>
	  block_stack,
      min::uns32 max_length = 64 );

inline void push_block
	( ll::parser::table::block_stack block_stack,
	  min::gen name,
	  ll::parser::table::name_table
	      selector_name_table,
	  ll::parser::table::undefined_stack
	      undefined_stack,
	  ll::parser::bracketed::indentation_mark
	      indentation_mark,
	  ll::parser::table::flags input_flags,
	  ll::parser::table::flags trace_flags,
	  min::Uchar ID_character )
{
    ll::parser::table::block_struct b =
        { name,
	  selector_name_table->length,
	  undefined_stack->length,
	  indentation_mark,
	  input_flags, trace_flags, ID_character };
    min::push ( block_stack ) = b;
    min::unprotected::acc_write_update
        ( block_stack, name );
    min::unprotected::acc_write_update
        ( block_stack, indentation_mark );
}

// Lexeme Maps
// ------ ----

struct lexeme_map_entry_struct;
typedef min::packed_struct_updptr
     <lexeme_map_entry_struct>
    lexeme_map_entry;

struct lexeme_map_entry_struct
    : public ll::parser::table::root_struct
{
    min::gen token_value;
        // min::NONE() if token is to be discarded.
    min::uns32 token_type;
        // Token type, computed from token value.
    min::gen token_value_type;
        // Token value_type, computed from token value.
    min::uns32 lexical_master;
        // ll::lexeme::MISSING_MASTER if missing.
};

MIN_REF ( ll::parser::table::root, next,
          ll::parser::table::lexeme_map_entry )
MIN_REF ( min::gen, label,
          ll::parser::table::lexeme_map_entry )
MIN_REF ( min::gen, token_value,
          ll::parser::table::lexeme_map_entry )
MIN_REF ( min::gen, token_value_type,
          ll::parser::table::lexeme_map_entry )

typedef min::packed_vec_updptr<ll::parser::table::root>
    lexeme_map;

// Create a lexeme map of the given length.
//
ll::parser::table::lexeme_map create_lexeme_map
        ( uns32 length );

// Push lexeme map entry into lexeme map.  It is an
// error if lexeme name maps to a lexeme type that
// is too big for the lexeme map.
//
void push_lexeme_map_entry
	( min::gen lexeme_name,
	  min::uns32 lexeme_type,
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  min::gen token_value,
	  min::uns32 token_type,
	  min::gen token_value_type,
	  min::uns32 lexical_master,
	  ll::parser::table::lexeme_map lexeme_map );

// Remove from the lexeme map all entries that have
// entry block_level >= block_level argument.  Add the
// number of entries that were removed from the table
// to collected_entries.  Undefined stack must be
// processed before this function is called.
//
void end_block
	( ll::parser::table::lexeme_map lexeme_map,
          uns32 block_level,
	  uns64 & collected_entries );

} } }

# endif // LL_PARSER_TABLE_H
