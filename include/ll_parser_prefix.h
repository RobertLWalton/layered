// Layered Languages Prefix Subexpression Parser
//
// File:	ll_parser_prefix.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Apr 21 03:21:58 EDT 2019
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Prefix Table
//	Prefix Functions

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_PREFIX_H
# define LL_PARSER_PREFIX_H

# include <ll_parser.h>

namespace ll { namespace parser { namespace prefix {

// Prefix Table
// ------ -----

struct prefix_struct;
typedef min::packed_struct_updptr<prefix_struct> prefix;
extern const uns32 & PREFIX;
    // Subtype of min::packed_struct<prefix_struct>.

struct prefix_struct
    : public ll::parser::table::root_struct
{
    // Packed_struct subtype is PREFIX.

    ll::parser::table::new_flags new_selectors;
    min::gen group;
    min::gen implied_subprefix;
    min::gen implied_subprefix_type;
        // This is just a cache of the .type of
	// the implied prefix, or MISSING if none.
    min::uns32 paragraph_lexical_master;
    min::uns32 line_lexical_master;
        // ll::lexeme::MISSING_MASTER if missing.
    ll::parser::reformatter reformatter;
    ll::parser::reformatter_arguments
	    reformatter_arguments;
	// Reformater and its arguments, or (both) NULL_
	// STUB if none.
};

MIN_REF ( ll::parser::table::root, next,
          ll::parser::prefix::prefix )
MIN_REF ( min::gen, label,
          ll::parser::prefix::prefix )
MIN_REF ( min::gen, group,
          ll::parser::prefix::prefix )
MIN_REF ( min::gen, implied_subprefix,
          ll::parser::prefix::prefix )
MIN_REF ( min::gen, implied_subprefix_type,
          ll::parser::prefix::prefix )
MIN_REF ( ll::parser::reformatter, reformatter,
          ll::parser::prefix::prefix )
MIN_REF ( ll::parser::reformatter_arguments,
	  reformatter_arguments,
          ll::parser::prefix::prefix )

// Create a prefix definition entry with given
// label and parameters, and push it into the
// given prefix_table.
//
void push_prefix
	( min::gen prefix_label,
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  ll::parser::table::new_flags new_selectors,
	  min::gen group,
	  min::gen implied_subprefix,
	  min::gen implied_subprefix_type,
	  min::uns32 paragraph_lexical_master,
	  min::uns32 line_lexical_master,
	      // May be ll::lexeme::MISSING_MASTER
	  ll::parser::reformatter reformatter,
	  ll::parser::reformatter_arguments
	      reformatter_arguments,
	  ll::parser::table::key_table prefix_table );

extern min::locatable_var<ll::parser::reformatter>
    prefix_reformatter_stack;


// Prefix Compact Function
// ------ ------- ---------

// Given an expression beginning with first and ending
// just before next, in which the first token is a
// PREFIX token, add the non-first tokens to the first
// token value as elements.  Execute pass->next on
// the list of non-first elements before doing this, and
// then convert any non-first element tokens with
// strings to tokens with values of .type <Q> or #.
// The end position of the expanded PREFIX token is
// updated to equal the end position of next->previous,
// and the type of the PREFIX token is changed to
// BRACKETED.  Lastly non-first element tokens are
// removed, but both the first and the next tokens are
// not.
//
// If separator_found is true, set the .terminator
// attribute of the expanded PREFIX to separator
// and update the end position of the expanded PREFIX
// token to separator_found.
//
// If the first token is an IMPLIED_HEADER or IMPLIED_
// PREFIX token, the value of this token is replaced by
// a copy and then treated as if it were a PREFIX token.
//
void compact_prefix_list
	( ll::parser::parser parser,
	  ll::parser::pass pass,
	  ll::parser::table::flags selectors,
	  ll::parser::token first,
	  ll::parser::token next,
	  const min::position & separator_found,
	  min::gen separator,
	  ll::parser::table::flags trace_flags );

} } }

# endif // LL_PARSER_PREFIX_H
