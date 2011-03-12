// Layers Language Parser Table
//
// File:	ll_parser_table.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Mar 12 14:09:24 EST 2011
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

# ifndef LL_PARSER_TABLE_H
# define LL_PARSER_TABLE_H

# include <min.h>

namespace ll { namespace parser { namespace table {

    using min::uns8;
    using min::uns32;
    using min::uns64;
    using min::NULL_STUB;

    typedef min::uns64 selectors;
} } }

// Roots
// -----

namespace ll { namespace parser { namespace table {

// All hash table entries begin with a root.
//
struct root_struct;
typedef min::packed_struct_updptr<root_struct>
        root;
struct root_struct
{
    uns32 control;
    	// Packed structure control word.
    ll::parser::table::root next;
        // Next entry in the stack with the same key.
	// NULL_STUB if no next entry.
    ll::parser::table::selectors selectors;
        // Selector bits.
    min::gen label;
        // Label of hash table entry.  A symbol,
	// integral number in the range 0 .. 2^28-1,
	// or MIN label with more than 1 element.
};

// Key Prefixes
// --- --------

// A hash table maps keys, which are symbols, integers
// in the range 0 .. (2^28)-1, or multi-element labels,
// to stacks of hash entries.  If a key is a symbol or
// integer, it is a 1-element key, with the element
// being the symbol or integer.  If the key is a label,
// it must have more than one element, and each element
// must be a symbol or integer in the range 0 ..
// (2^28)-1.
//
// A key prefix is an initial segment of a key, viewing
// the key as a list of key elements.
//
// A hash table more specifically takes key prefix and
// finds for each a key_prefix structure whose `label'
// is the key prefix.  The key_prefix structure points
// at the list of hash table entries whose keys equal
// the label of the key prefix structure, if there
// are such entries.
//
// Thus if there is a hash table entry with key [A B C],
// there will be key_prefix structures labelled A,
// [A B], and [A B C], but only the last necessarily
// points at a non-empty list of hash table entries,
// which would consist of all entries with key [A B C].
//
// The hash code of a key_prefix is the hash code of its
// label.  More specifically, if the label has just one
// key element, which must necessarily be a symbol or an
// integer, the hash is the hash of that element, and
// if the label has more than one key element, the hash
// is the hash as per min::labhash of the label made
// from the key elements.
//
struct key_prefix_struct;
typedef min::packed_struct_updptr<key_prefix_struct>
        key_prefix;
struct key_prefix_struct
{
    uns32 control;
	// Packed structure subtype is KEY_PREFIX.
    min::gen key_element;
        // Key element of this entry.
    ll::parser::table::key_prefix previous;
        // Entry for label prefix ending just before
	// this key element, or NULL_STUB if none,
	// i.e., if this is the first key element of the
	// label.
    ll::parser::table::root first;
        // First hash table entry whose label ends with
	// this key element, or NULL_STUB if none.
    ll::parser::table::key_prefix next;
        // Next key prefix in hash list.
};

// A Hash table is just a vector of key_prefix values.
// The `length' of this vector MUST BE a power of two.
//
typedef min::packed_vec_insptr
	    <ll::parser::table::key_prefix> table;

// Return the hash table key_prefix with the given key.
// If none and `create' is true, create key_prefix.
// If none and `create' is false, return NULL_STUB.
//
ll::parser::table::key_prefix find_key_prefix
	( min::gen key,
	  ll::parser::table::table table,
	  bool create = false );

// Return hash table entry with the given key, or
// NULL_STUB if none.
//
ll::parser::table::root find
	( min::gen key,
	  ll::parser::table::table table );

// Push the given hash table entry into a hash table
// stack for the given key.
//
void push ( min::gen key,
            ll::parser::table::root entry,
            ll::parser::table::table table );

struct new_selectors
     // Upon encountering opening bracket or indentation
     // mark, new-selectors =
     //	    ( ( old_selectors | or_selectors )
     //		    & ~ not_selectors )
     //		    ^ xor_selectors
     //
     // where
     //		( or_selectors & not_selectors ) = 0
     //		( or_selectors & xor_selectors ) = 0
     //		( not_selectors & xor_selectors ) = 0
{
    ll::parser::table::selectors or_selectors;
    ll::parser::table::selectors not_selectors;
    ll::parser::table::selectors xor_selectors;

    new_selectors ( selectors or_selectors = 0,
                    selectors not_selectors = 0,
                    selectors xor_selectors = 0 )
	: or_selectors ( or_selectors ),
	  not_selectors ( not_selectors ),
	  xor_selectors ( xor_selectors ) {}
};

// Brackets
// --------

// Bracket definition.
//
struct opening_bracket_struct;
typedef min::packed_struct_updptr
	    <opening_bracket_struct>
        opening_bracket;
extern const uns32 & OPENING_BRACKET;
    // Subtype of min::packed_struct
    //		       <opening_bracket_struct>.
struct closing_bracket_struct;
typedef min::packed_struct_updptr
	    <closing_bracket_struct>
        closing_bracket;
extern const uns32 & CLOSING_BRACKET;
    // Subtype of min::packed_struct
    //		       <closing_bracket_struct>.
struct opening_bracket_struct : public root_struct
{
    // Packed structure subtype is OPENING_BRACKET.

    ll::parser::table::closing_bracket closing_bracket;
        // The opposing bracket of the opening bracket.

    ll::parser::table::new_selectors new_selectors;
    	// New selectors associated with this opening
	// bracket.

};
struct closing_bracket_struct : public root_struct
{
    // Packed structure subtype is CLOSING_BRACKET.

    ll::parser::table::opening_bracket opening_bracket;
        // The opposing bracket of the closing bracket.
};

void push_brackets
	( min::gen opening_label,
	  min::gen closing_label,
	  ll::parser::table::selectors selectors,
	  const ll::parser::table::new_selectors
	      & new_selectors,
	  ll::parser::table::table table );

// Indentation Marks
// ----------- -----

// Indentation mark definition.
//
struct indentation_mark_struct;
typedef min::packed_struct_updptr
	    <indentation_mark_struct>
        indentation_mark;
extern const uns32 & INDENTATION_MARK;
    // Subtype of min::packed_struct
    //		       <indentation_mark_struct>.

// A gluing indentation mark has an associated indenta-
// tion split that contains the mark label, points at
// the indentation mark, and is entered in an indenta-
// tion split table.
//
struct indentation_split_struct;
typedef min::packed_vec_insptr
	    <uns8,indentation_split_struct>
        indentation_split;

// A split table has exactly 256 elements, each the
// head of a length-sorted list of indentation_splits.
// Each split is entered in the list indexed by the
// last byte of its indentation mark's label.
//
typedef min::packed_vec_insptr
	    <ll::parser::table::indentation_split>
	split_table;

struct indentation_mark_struct : public root_struct
{
    ll::parser::table::new_selectors new_selectors;

    ll::parser::table::indentation_split
	    indentation_split;
	// If gluing, the split for this indentation.
	// If non-gluing, NULL_STUB.
};

struct indentation_split_struct
{
    const min::uns32 control;
    const min::uns32 length;
    const min::uns32 max_length;

    ll::parser::table::indentation_split next;
        // Next in the length-sorted list of splits
	// whose head is an indentation_split table
	// element.

    ll::parser::table::indentation_mark
	    indentation_mark;
	// Indentation_mark associated with split.

    // The vector of min::uns8 elements is the indenta-
    // tion mark label in UTF-8.  Lexemes at the end of
    // a line can be checked to see if they end with
    // this.  The last byte is used as a hash in the
    // indentation split table, so, for example, most
    // line ending lexemes (e.g. those ending in
    // letters) will not need to be checked (while
    // those ending in `:' probably will be checked).

};

void push_indentation_mark
	( min::gen label,
	  ll::parser::table::selectors selectors,
	  const ll::parser::table::new_selectors
	      & new_selectors,
	  ll::parser::table::table bracket_table,
	  ll::parser::table::split_table split_table =
	      NULL_STUB );

} } }

# endif // LL_PARSER_TABLE_H
