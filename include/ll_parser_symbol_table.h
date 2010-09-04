// Layers Language Parser Symbol Table
//
// File:	ll_parser_symbol_table.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Sep  3 19:48:21 EDT 2010
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
struct root
{
    min::gen key_element;
        // Next element of key.
    min::gen type;
        // Type.  
    min::uns64 selectors;
        // Selector bits.
    root ** next;
        // Next entry in the stack with the same key.
};

// A key prefix points at the next key element in a
// multi-element key.
//
extern min::gen KEY_PREFIX;
struct key_prefix
{
    root r;  // Type == KEY_PREFIX.

    root ** next_element;
       // This key_prefix represents a prefix of a key
       // and the next_element represents the 

};

// Bracket definition.
//
extern min::gen BRACKET;
struct bracket
{
    root r;  // Type == BRACKET.

    bracket ** opposing_bracket;
        // The opposing bracket of the opening bracket
	// is the closing bracket, and vice versa.

};

} } }

# endif // LL_PARSER_SYMBOL_TABLE_H
