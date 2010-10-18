// Layers Language Parser Symbol Table
//
// File:	ll_parser_symbol_table.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Oct 18 06:53:25 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	LL Parser Symbol Table Root

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_SYMBOL_TABLE_H
# define LL_PARSER_SYMBOL_TABLE_H

# include <min.h>

// LL Parser Symbol Table Root
// -- ------ ------ ----- ----

namespace ll { namespace parser
    { namespace symbol_table {

// All symbol table entries begin with a root.
//
struct key_prefix;
struct root
{
    min::uns32 type;
    	// Packed structure type.
    min::gen key_element;
        // This element of key.
    key_prefix ** prefix;
        // Prefix specifying previous elements of the
	// key, or NULL if no previous elements.
    root ** next;
        // Next entry in the stack with the same
	// key_element and prefix

    min::uns64 selectors;
        // Selector bits.
};

// A key prefix points at the next key element in a
// multi-element key.
//
extern min::uns32 KEY_PREFIX;
struct key_prefix
{
    root r;
        // Type is KEY_PREFIX.

    min::uns32 hash;
        // Hash of the label that is the prefix
	// ending with this element.  Computed using
	// min::next_label_hash.

    min::uns32 count;
        // Number of possible next elements.

    root ** next_element[5];
        // If count <= 5, the next elements in
	// order.  Otherwise not used, and the
	// next elements are looked up by
	//   min::next_label_hash
	//       ( hash-of-next-key-element,
	//	   this->hash );

};

// Bracket definition.
//
extern min::uns32 BRACKET;
struct bracket
{
    root r;  // Type is BRACKET.

    bracket ** opposing_bracket;
        // The opposing bracket of the opening bracket
	// is the closing bracket, and vice versa.

};

} } }

# endif // LL_PARSER_SYMBOL_TABLE_H
