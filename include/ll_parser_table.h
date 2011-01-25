// Layers Language Parser Table
//
// File:	ll_parser_table.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jan 25 07:35:03 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Root
//	Key Element
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

// Root 
// ----

namespace ll { namespace parser { namespace table {

// All hash table entries begin with a root.
//
struct root_struct;
typedef min::packed_struct_updptr<root_struct>
        root_ptr;
extern const uns32 & ROOT;
struct root_struct
{
    uns32 control;
    	// Packed structure control word.
    root_ptr next;
        // Next entry in the stack with the same key.
	// NULL_STUB if no next entry.
    uns64 selectors;
        // Selector bits.
};

// Key Element
// --- -------

// A hash table maps keys, which are symbols, integers
// in the range 0 .. (2^28)-1, or multi-element labels,
// to stacks of hash entries.  If a key is a symbol or
// integer, it is a 1-element key, with the element
// being the symbol or integer.  If the key is a label,
// it must have more than one element, and each element
// must be a symbol or integer in the range 0 ..
// (2^28)-1.
//
// A hash table lookup takes a sequence of key elements
// (encoded in tokens usually) and finds the hash table
// entry corresponding to the longest initial segment
// of the key element sequence for which there is a hash
// table entry.  This entry may later be rejected, in
// which case the entry for the next longest sequence
// for which there is an entry is found.
//
// A hash table contains key_prefix entries, one for
// each non-empty initial segment of each label in the
// hash table.  Given the sequence of key elements to
// look up, these are looked up consecutively beginning
// with the first key element and continuing till
// lookup fails.  A key element hash is the hash code
// of the label prefix that ends with the key element.
// If this prefix is a symbol or integer, it is the
// hash of the symbol or integer.  Otherwise it is
// the hash of the multi-element label made from the
// key elements of the prefix.
//
struct key_prefix_struct;
typedef min::packed_struct_updptr<key_prefix_struct>
        key_prefix_ptr;
extern const uns32 & KEY_PREFIX;
struct key_prefix_struct
{
    uns32 control;
	// Packed structure subtype is KEY_PREFIX.
    min::gen key_element;
        // Key element of this entry.
    key_prefix_ptr previous;
        // Entry for label prefix ending just before
	// this key element, or NULL_STUB if none,
	// i.e., if this is the first key of the label.
    root_ptr first;
        // First hash table entry whose label ends with
	// this key element, or NULL_STUB if none.
};

// A Hash table is just a vector of key_prefix_ptr
// values.  The `length' of this vector MUST BE a
// power of two.
//
typedef min::packed_vec_updptr<key_element_ptr>
    table_ptr;

// Push the given hash table entry into a hash table.
//
void push ( root_ptr entry, table_ptr table );

// Return the topmost table hash table element with
// the given key, or return NULL_STUB if none.
//
root_ptr find ( min::gen key, table_ptr table );

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
    selectors	or_selectors;
    selectors	not_selectors;
    selectors	xor_selectors;
};

// Brackets
// --------

// Bracket definition.
//
struct opening_bracket_struct;
typedef min::packed_struct_updptr
	    <opening_bracket_struct>
        opening_bracket_ptr;
extern const uns32 & OPENING_BRACKET;
struct closing_bracket_struct;
typedef min::packed_struct_updptr
	    <closing_bracket_struct>
        closing_bracket_ptr;
extern const uns32 & CLOSING_BRACKET;
struct opening_bracket_struct : public root_struct
{
    // Packed structure subtype is OPENING_BRACKET.

    closing_bracket_ptr closing_bracket;
        // The opposing bracket of the opening bracket.

    new_selectors new_select;
    	// New selectors associated with this opening
	// bracket.

};
struct closing_bracket_struct : public root_struct
{
    // Packed structure subtype is CLOSING_BRACKET.

    closing_bracket_ptr opening_bracket;
        // The opposing bracket of the closing bracket.
};

void push_brackets
	( min::gen opening_label,
	  min::gen closing_label,
	  selectors select,
	  const new_selectors & new_select );

// Indentation Marks
// ----------- -----

// Indentation mark definition.
//
struct indentation_mark_struct;
typedef min::packed_vec_updptr
	    <uns8,indentation_mark_struct>
        indentation_mark_ptr;
extern const uns32 & INDENTATION_MARK;
struct indentation_mark_struct : public root_struct
{
    // Packed vector subtype is INDENTATION_MARK.

    new_selectors new_select;

    bool is_gluing;

    // The vector is the indentation mark in UTF-8.
    // Lexemes at the end of a line can be checked
    // to see if they end with this.  The last
    // byte is used as a hash in the indentation
    // mark table, so, for example, most line ending
    // lexemes (e.g. those ending in letters) will
    // not need to be checked (while those ending in
    // `:' probably will be checked).

    uns32 length, max_length;

};

void push_indentation_mark
	( min::gen label,
	  selectors select,
	  const new_selectors & new_select,
	  bool is_gluing );

} } }

# endif // LL_PARSER_TABLE_H
