// Layered Languages Prefix Parser
//
// File:	ll_parser_prefix.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jan 12 11:20:38 EST 2016
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Prefix Pass

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_PREFIX_H
# define LL_PARSER_PREFIX_H

# include <ll_parser.h>

namespace ll { namespace parser { namespace prefix {



// Prefix Pass
// ------ ----

// An prefix_stack is part of each prefix_pass.
//
struct prefix_stack_struct
{
    // Save of various prefix pass parser variables.
    // None of these saved values are ACC locatable.
    //
    ll::parser::token first;
        // First token of subexpression.  This is a
	// prefix token if default_prefix is MISSING, or
	// is the first token after the implied default
	// prefix otherwise.

    min::gen default_prefix;
        // MISSING if `first' is subexpression prefix.
	// Otherwise a prefix that is to be copied to
	// produce the subexpression prefix.
};

typedef min::packed_vec_insptr< prefix_stack_struct >
    prefix_stack;

struct prefix_pass_struct;
typedef min::packed_struct_updptr<prefix_pass_struct>
        prefix_pass;
extern const uns32 & PREFIX_PASS;
    // Subtype of
    //     min::packed_struct<prefix_pass_struct>.
struct prefix_pass_struct
    : public ll::parser::pass_struct
{
    // Packed_struct subtype is PREFIX_PASS.

    const ll::parser::prefix::prefix_stack prefix_stack;
        // Stack of prefix subexpressions used during
	// prefix parsing pass.

    ll::parser::table::flags trace_subexpressions;
        // Trace flag named `prefix subexpressions'
	// that traces prefix subexpressions.

};

MIN_REF ( min::gen, name,
          ll::parser::prefix::prefix_pass )
MIN_REF ( ll::parser::parser, parser,
          ll::parser::prefix::prefix_pass )
MIN_REF ( ll::parser::pass, next,
          ll::parser::prefix::prefix_pass )
MIN_REF ( ll::parser::prefix::prefix_stack,
          prefix_stack,
          ll::parser::prefix::prefix_pass )

// Return a new prefix parser pass.
//
ll::parser::pass new_pass ( ll::parser::parser parser );

} } }

# endif // LL_PARSER_PREFIX_H
