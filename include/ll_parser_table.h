// Layers Language Parser Table
//
// File:	ll_parser_table.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Feb  2 19:03:16 EST 2012
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

# ifndef LL_PARSER_TABLE_H
# define LL_PARSER_TABLE_H

# include <min.h>

namespace ll { namespace parser { namespace table {

    using min::uns8;
    using min::uns32;
    using min::uns64;
    using min::NULL_STUB;

// Selectors
// ---------

typedef min::uns64 selectors;

typedef min::packed_vec_ptr<min::gen>
	selector_name_table;
typedef min::packed_vec_insptr<min::gen>
	selector_name_table_insptr;

// Return the index associated with a name in a selector
// name table, or return -1 if none.
//
inline int get_index
    ( ll::parser::table::selector_name_table name_table,
      min::gen name )
{
    int i = name_table->length;
    while ( -- i >= 0 && name_table[i] != name );
    return i;
}


// Roots
// -----

// All hash table entries begin with a root.
//
struct root_struct;
typedef min::packed_struct_updptr<root_struct>
        root;
struct root_struct
{
    uns32 control;
    	// Packed structure control word.
    const ll::parser::table::root next;
        // Next entry in the stack with the same key.
	// NULL_STUB if no next entry.
    ll::parser::table::selectors selectors;
        // Selector bits.
    const min::gen label;
        // Label of hash table entry.  A sequence of one
	// or more symbols, where a symbol is a min::gen
	// string value encoding the translation string
	// of a word, mark, separator, or natural number
	// lexeme (i.e., the value of a symbol token).
	//
	// The `label' is also called the `key' of a
	// hash table entry.
};

MIN_REF ( ll::parser::table::root, next,
          ll::parser::table::root )
MIN_REF ( min::gen, label,
          ll::parser::table::root )

// Key Prefixes
// --- --------

// A hash table maps keys, which are labels (see Roots
// above), to stacks of hash entries.  A key is a
// sequence of one or more symbols, each represented
// in the hash table by a min::gen string value and in
// the input by a symbol token whose value is a min::gen
// string.
//
// A key prefix is an initial segment of a key, viewing
// the key as a list of key elements.
//
// A hash table more specifically takes a key prefix and
// finds a key_prefix structure whose key is the key
// prefix.  The key_prefix structure points at the list
// of hash table entries whose labels equal the key of
// the key prefix structure, if there are such entries.
//
// Thus if there is a hash table entry with label
// [A B C], there will be key_prefix structures with
// keys A, [A B], and [A B C], but only the last
// necessarily points at a non-empty list of hash table
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

    const min::gen key_element;
        // Last element of the key of this key prefix.
    const ll::parser::table::key_prefix previous;
        // Entry for key prefix whose key consists of
	// all the key elements of this prefix but the
	// last, or NULL_STUB if none (because the key
	// of this key prefix has only one element).

    const ll::parser::table::key_prefix next;
        // Next key prefix in hash list.

    const ll::parser::table::root first;
        // First hash table entry whose label equals
	// the key of this key prefix, or NULL_STUB
	// if none.
};

MIN_REF ( min::gen, key_element,
          ll::parser::table::key_prefix )
MIN_REF ( ll::parser::table::key_prefix, previous,
          ll::parser::table::key_prefix )
MIN_REF ( ll::parser::table::root, first,
          ll::parser::table::key_prefix )
MIN_REF ( ll::parser::table::key_prefix, next,
          ll::parser::table::key_prefix )

// A Hash table is just a vector of key_prefix values.
// The `length' of this vector MUST BE a power of two.
//
typedef min::packed_vec_insptr
	    <ll::parser::table::key_prefix> table;

// Create a hash table of given length which MUST BE a
// power of two.
//
ll::parser::table::table create_table ( uns32 length );

// Return the hash table key_prefix with the given key.
// If none and `create' is true, create the key_prefix.
// If none and `create' is false, return NULL_STUB.
//
ll::parser::table::key_prefix find_key_prefix
	( min::gen key,
	  ll::parser::table::table table,
	  bool create = false );

// Return the hash table entry with the given key, or
// NULL_STUB if there is none.
//
ll::parser::table::root find
	( min::gen key,
	  ll::parser::table::table table );

// Push the given hash table entry into to the top of
// the hash table stack for key = entry->label.
//
void push ( ll::parser::table::table table,
            ll::parser::table::root entry );

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
    // Packed_struct subtype is OPENING_BRACKET.

    const ll::parser::table::closing_bracket
          closing_bracket;
        // The opposing bracket of the opening bracket.

    ll::parser::table::new_selectors new_selectors;
    	// New selectors associated with this opening
	// bracket.

    bool full_line;

};
struct closing_bracket_struct : public root_struct
{
    // Packed_struct subtype is CLOSING_BRACKET.

    const ll::parser::table::opening_bracket
          opening_bracket;
        // The opposing bracket of the closing bracket.
};

MIN_REF ( min::gen, label,
          ll::parser::table::opening_bracket )
MIN_REF ( ll::parser::table::closing_bracket,
          closing_bracket,
          ll::parser::table::opening_bracket )

MIN_REF ( min::gen, label,
          ll::parser::table::closing_bracket )
MIN_REF ( ll::parser::table::opening_bracket,
          opening_bracket,
          ll::parser::table::closing_bracket )

void push_brackets
	( min::gen opening_label,
	  min::gen closing_label,
	  ll::parser::table::selectors selectors,
	  const ll::parser::table::new_selectors
	      & new_selectors,
	  bool full_line,
	  ll::parser::table::table bracket_table );

// Named Brackets
// ----- --------

// `named_' abbreviates `named_bracket_'.

// Named bracket definition.

struct named_opening_struct;
typedef min::packed_struct_updptr
	    <named_opening_struct>
        named_opening;
extern const uns32 & NAMED_OPENING;
    // Subtype of min::packed_struct
    //		       <named_opening_struct>.

struct named_separator_struct;
typedef min::packed_struct_updptr
	    <named_separator_struct>
        named_separator;
extern const uns32 & NAMED_SEPARATOR;
    // Subtype of min::packed_struct
    //		       <named_separator_struct>.

struct named_middle_struct;
typedef min::packed_struct_updptr
	    <named_middle_struct>
        named_middle;
extern const uns32 & NAMED_MIDDLE;
    // Subtype of min::packed_struct
    //		       <named_middle_struct>.

struct named_closing_struct;
typedef min::packed_struct_updptr
	    <named_closing_struct>
        named_closing;
extern const uns32 & NAMED_CLOSING;
    // Subtype of min::packed_struct
    //		       <named_closing_struct>.

struct named_middle_closing_struct;
typedef min::packed_struct_updptr
	    <named_middle_closing_struct>
        named_middle_closing;
extern const uns32 & NAMED_MIDDLE_CLOSING;
    // Subtype of
    //     min::packed_struct
    //	       <named_middle_closing_struct>.

struct named_opening_struct : public root_struct
{
    // Packed_struct subtype is NAMED_OPENING.

    // Syntax is
    //
    //      named-opening ...
    //	    { named-separator ... }*
    //	    named-middle
    //	    ***
    //      named-middle ... named-closing
    //
    // or
    //
    //      named-opening ...
    //	    { named-separator ... }*
    //	    named-middle
    //	    ***
    //      named-middle-closing
    //
    // or
    //
    //      named-opening ...
    //	    { named-separator ... }*
    //	    named-closing
    //
    // Where ... is any sequence of words, numbers,
    // quoted strings, or unnamed explicit subexpres-
    // sions (but NOT separators or marks outside
    // explicit subexpressions), and *** is any
    // sequence of lexemes and explicit subexpres-
    // sions (possibly including named ones).
    //
    // E.g, if the named-opening is <, the named-
    // separator is #, the named-middle is |, and
    // the named-closing is >, then
    //
    //	    < ... {# ...}* | *** | ... >
    // or
    //	    < ... {# ...}* | *** |>
    // or
    //	    < ... {# ...}* >
    //
    // The named-middle-closing should be specified as
    // MISSING if the named-middle and named-closing
    // cannot glue together into a single lexeme.

    const ll::parser::table::named_separator
          named_separator;
    const ll::parser::table::named_middle
          named_middle;
    const ll::parser::table::named_closing
          named_closing;
    const ll::parser::table
                    ::named_middle_closing
          named_middle_closing;

};

struct named_separator_struct : public root_struct
{
    // Packed_struct subtype is NAMED_SEPARATOR.

    const ll::parser::table::named_opening
          named_opening;
        // Corresponding named opening bracket.
};

struct named_middle_struct : public root_struct
{
    // Packed_struct subtype is NAMED_MIDDLE.

    const ll::parser::table::named_opening
          named_opening;
        // Corresponding named opening bracket.
};

struct named_closing_struct : public root_struct
{
    // Packed_struct subtype is NAMED_CLOSING.

    const ll::parser::table::named_opening
          named_opening;
        // Corresponding named opening bracket.
};

struct named_middle_closing_struct
    : public root_struct
{
    // Packed_struct subtype is NAMED_MIDDLE_CLOSING_
    // BRACKET.

    const ll::parser::table::named_opening
          named_opening;
        // Corresponding named opening bracket.
};

MIN_REF ( min::gen, label,
          ll::parser::table::named_opening )
MIN_REF ( ll::parser::table::named_separator,
          named_separator,
          ll::parser::table::named_opening )
MIN_REF ( ll::parser::table::named_middle,
          named_middle,
          ll::parser::table::named_opening )
MIN_REF ( ll::parser::table::named_closing,
          named_closing,
          ll::parser::table::named_opening )
MIN_REF ( ll::parser::table
                    ::named_middle_closing,
          named_middle_closing,
          ll::parser::table::named_opening )

MIN_REF ( min::gen, label,
          ll::parser::table::named_separator )
MIN_REF ( ll::parser::table::named_opening,
          named_opening,
          ll::parser::table::named_separator )

MIN_REF ( min::gen, label,
          ll::parser::table::named_middle )
MIN_REF ( ll::parser::table::named_opening,
          named_opening,
          ll::parser::table::named_middle )

MIN_REF ( min::gen, label,
          ll::parser::table::named_closing )
MIN_REF ( ll::parser::table::named_opening,
          named_opening,
          ll::parser::table::named_closing )

MIN_REF ( min::gen, label,
          ll::parser::table
	    ::named_middle_closing )
MIN_REF ( ll::parser::table::named_opening,
          named_opening,
          ll::parser::table
	    ::named_middle_closing )

void push_named_brackets
	( min::gen named_opening_label,
	  min::gen named_separator_label,
	  min::gen named_middle_label,
	  min::gen named_closing_label,
	  min::gen named_middle_closing_label,
	  ll::parser::table::selectors selectors,
	  ll::parser::table::table bracket_table );

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

extern min::locatable_var
	<ll::parser::table::indentation_mark>
	top_level_indentation_mark;
    // Virtual indentation mark for top level
    // parsing.

struct indentation_separator_struct;
typedef min::packed_struct_updptr
	    <indentation_separator_struct>
        indentation_separator;
extern const uns32 & INDENTATION_SEPARATOR;
    // Subtype of min::packed_struct
    //		       <indentation_separator_struct>.

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

// Create an indentation split table.  Note that the
// length is always 256 (table index is uns8).
//
ll::parser::table::split_table create_split_table
	( void );

struct indentation_mark_struct : public root_struct
{
    ll::parser::table::new_selectors new_selectors;

    const ll::parser::table::indentation_separator
	    indentation_separator;

    const ll::parser::table::indentation_split
	    indentation_split;
	// If gluing, the split for this indentation.
	// If non-gluing, NULL_STUB.
};

MIN_REF ( min::gen, label,
          ll::parser::table::indentation_mark )
MIN_REF ( ll::parser::table::indentation_separator,
          indentation_separator,
          ll::parser::table::indentation_mark )
MIN_REF ( ll::parser::table::indentation_split,
          indentation_split,
          ll::parser::table::indentation_mark )

struct indentation_separator_struct : public root_struct
{
    // Packed_struct subtype is INDENTATION_SEPARATOR.

    const ll::parser::table::indentation_mark
    	    indentation_mark;
        // The indentation_mark for which this is a
	// separator.
};

MIN_REF ( min::gen, label,
          ll::parser::table::indentation_separator )
MIN_REF ( ll::parser::table::indentation_mark,
          indentation_mark,
          ll::parser::table::indentation_separator )

struct indentation_split_struct
{
    const min::uns32 control;
    const min::uns32 length;
    const min::uns32 max_length;

    const ll::parser::table::indentation_split next;
        // Next in the length-sorted list of splits
	// whose head is an indentation_split table
	// element.

    const ll::parser::table::indentation_mark
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

MIN_REF ( ll::parser::table::indentation_split,
          next,
          ll::parser::table::indentation_split )
MIN_REF ( ll::parser::table::indentation_mark,
          indentation_mark,
          ll::parser::table::indentation_split )

void push_indentation_mark
	( min::gen mark_label,
	  min::gen separator_label,
	  ll::parser::table::selectors selectors,
	  const ll::parser::table::new_selectors
	      & new_selectors,
	  ll::parser::table::table bracket_table,
	  ll::parser::table::split_table split_table =
	      NULL_STUB );
} } }

# endif // LL_PARSER_TABLE_H
