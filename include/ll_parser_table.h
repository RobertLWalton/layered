// Layers Language Parser Table
//
// File:	ll_parser_table.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jan 25 05:24:37 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Root and Key Suffix
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

// Root and Key Suffix
// ---- --- --- ------

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
    min::gen key;
        // Key of this entry.
    uns32 hash;
        // Hash of key.
    root_ptr next;
        // Next entry in the stack with the same key.
	// NULL_STUB if no next entry.
    uns64 selectors;
        // Selector bits.
};

// A multi-element key has two table entries.  Lower in
// the stack is a normal entry.  Higher (toward the top)
// is a suffix entry that contains the breakdown of the
// key, which is a label, into an initial segment and a
// final element.  This higher level entry points via
// its `next' element at the normal lower level entry.
//
// The suffix entry permits lookup without computing
// new label objects.  It has no other function.
//
struct key_suffix_struct;
typedef min::packed_struct_updptr<key_suffix_struct>
        key_suffix_ptr;
extern const uns32 & KEY_SUFFIX;
struct key_suffix_struct : public root_struct
{
    // Packed structure subtype is KEY_SUFFIX.

    root_ptr key_prefix;
        // Entry for key minus last element.  If that
	// key has a key_prefix entry, this points at
	// the key_prefix.
    min::gen last_key_element;
        // Last element of key.
};

// A Hash table is just a vector of root_ptr values.
// This is just a vector whose `length' MUST BE a
// power of two.
//
typedef min::packed_vec_updptr<root_ptr> table_ptr;

// Push the given entry into the table.
//
void push ( root_ptr entry, table_ptr table );

// Return the topmost table element with the given key,
// or return NULL_STUB if none.
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
