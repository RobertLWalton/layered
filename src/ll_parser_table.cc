// Layers Language Parser Table Functions
//
// File:	ll_parser_table.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jan 15 12:13:04 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Root and Suffix

// Usage and Setup
// ----- --- -----

# include <ll_parser_table.h>
# define PAR ll::parser
# define TAB ll::parser::table

// Root and Suffix
// ---- --- ------

TAB::root_ptr TAB::find
	( min::gen key, table_ptr table )
{
    uns32 hash = min::hash ( key );
    uns32 length = table->length;
    assert ( ( length & ( length - 1 ) ) == 0 );
    hash &= ( length - 1 );
    root_ptr p = table[hash];
    while ( p != NULL_STUB )
    {
        if ( p->key == key ) return p;
	p = p->next;
    }
    return NULL_STUB;
}

