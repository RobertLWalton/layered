// Layers Language Parser Table Functions
//
// File:	ll_parser_table.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jan 25 05:25:24 EST 2011
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

# include <ll_parser_table.h>
# define PAR ll::parser
# define TAB ll::parser::table

// Root and Suffix
// ---- --- ------

static min::uns32 root_gen_disp[] = {
    min::DISP ( & TAB::root_struct::key ),
    min::DISP_END };

static min::uns32 root_stub_disp[] = {
    min::DISP ( & TAB::root_struct::next ),
    min::DISP_END };

static min::packed_struct<TAB::root_struct>
    root_type 
	( "ll:parser::table::root_type",
	  ::root_gen_disp, ::root_stub_disp );
const min::uns32 & TAB::ROOT = root_type.subtype;

static min::uns32 key_suffix_gen_disp[] = {
    min::DISP ( & TAB::key_suffix_struct::key ),
    min::DISP ( & TAB::key_suffix_struct
                     ::last_key_element ),
    min::DISP_END };

static min::uns32 key_suffix_stub_disp[] = {
    min::DISP ( & TAB::key_suffix_struct::next ),
    min::DISP ( & TAB::key_suffix_struct::key_prefix ),
    min::DISP_END };

min::packed_struct_with_base
	<TAB::key_suffix_struct, TAB::root_struct>
    key_suffix_type
	( "ll:parser::table::key_suffix_type",
	  ::key_suffix_gen_disp,
	  ::key_suffix_stub_disp );
const min::uns32 & TAB::KEY_SUFFIX =
    key_suffix_type.subtype;

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

// Brackets
// --------

static min::uns32 opening_bracket_stub_disp[] = {
    min::DISP ( & TAB::opening_bracket_struct::next ),
    min::DISP ( & TAB::opening_bracket_struct
                     ::closing_bracket ),
    min::DISP_END };

min::packed_struct_with_base
	<TAB::opening_bracket_struct, TAB::root_struct>
    opening_bracket_type
	( "ll:parser::table::opening_bracket_type",
	  ::root_gen_disp,
	  ::opening_bracket_stub_disp );
const min::uns32 & TAB::OPENING_BRACKET =
    opening_bracket_type.subtype;

static min::uns32 closing_bracket_stub_disp[] = {
    min::DISP ( & TAB::closing_bracket_struct::next ),
    min::DISP ( & TAB::closing_bracket_struct
                     ::opening_bracket ),
    min::DISP_END };

min::packed_struct_with_base
	<TAB::closing_bracket_struct, TAB::root_struct>
    closing_bracket_type
	( "ll:parser::table::closing_bracket_type",
	  ::root_gen_disp,
	  ::closing_bracket_stub_disp );
const min::uns32 & TAB::CLOSING_BRACKET =
    closing_bracket_type.subtype;
