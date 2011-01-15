// Layers Language Parser Table
//
// File:	ll_parser_table.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jan 15 08:26:13 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Root and Suffix
//	Brackets

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_TABLE_H
# define LL_PARSER_TABLE_H

# include <min.h>

namespace ll { namespace parser { namespace table {

    using min::uns32;
    using min::NULL_STUB;
} } }

// Root and Suffix
// ---- --- ------

namespace ll { namespace parser { namespace table {

// All hash table entries begin with a root.
//
struct root_struct;
typedef min::packed_struct_updptr<root_struct>
        root_ptr;
struct root_struct
{
    min::uns32 control;
    	// Packed structure control word.
    min::gen key;
        // Key of this entry.
    min::uns32 hash;
        // Hash of key.
    root_ptr next;
        // Next entry in the stack with the same key.
	// NULL_STUB if no next entry.
    min::uns64 selectors;
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
extern min::uns32 KEY_SUFFIX;
struct key_suffix_struct;
typedef min::packed_struct_updptr<key_suffix_struct>
        key_suffix_ptr;
struct key_suffix_struct : public root_struct
{
    root_ptr key_prefix;
        // Entry for key minus last element.  If that
	// key has a key_prefix entry, this points at
	// the key_prefix.
    min::gen last_key_element;
        // Last element of key.
};

// A Hash table is just a vector of root_ptr values.
//
typedef min::packed_vec_updptr<root_ptr>
        table_ptr;

// Default hash table.
//
extern table_ptr default_table;

// Return the topmost table element with the given key,
// or return NULL_STUB if none.
//
root_ptr find ( min::gen key,
                table_ptr table = default_table );

// Push the given entry into the table.  Return the
// entry if the key is not a label, or the key_suffix
// created for the key if the key is a label.
//
root_ptr push ( root_ptr entry,
	        table_ptr table = default_table );

// Brackets
// --------

// Bracket definition.
//
struct bracket_struct;
typedef min::packed_struct_updptr<bracket_struct>
        bracket_ptr;
extern min::uns32 BRACKET;
struct bracket_struct : public root_struct
{
    // Packed structure type is BRACKET.

    bracket_ptr opposing_bracket;
        // The opposing bracket of the opening bracket
	// is the closing bracket, and vice versa.

};

} } }

# endif // LL_PARSER_TABLE_H
