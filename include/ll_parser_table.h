// Layers Language Parser Table
//
// File:	ll_parser_table.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jan  4 17:05:44 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	LL Parser Table Root

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_TABLE_H
# define LL_PARSER_TABLE_H

# include <min.h>

// LL Parser Table Root
// -- ------ ----- ----

namespace ll { namespace parser
    { namespace table {

// All table entries begin with a root.
//
struct root_struct;
min::packed_struct_insptr<root_struct> root_ptr;
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
min::packed_struct_insptr<key_suffix_struct>
    key_suffix_ptr;
struct key_suffix_struct : public root_struct
{
    root_ptr key_prefix;
        // Entry for key minus last element.
    min::gen last_key_element;
        // Last element of key.
};

// Bracket definition.
//
struct bracket_struct;
min::packed_struct_insptr<bracket_struct> root_ptr;
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
