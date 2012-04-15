// Layers Language Operator Parser
//
// File:	ll_parser_oper.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Apr 15 16:27:44 EDT 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Operator Table Entries

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_OPER_H
# define LL_PARSER_OPER_H

# include <ll_parser.h>

// As `operator' is a C++ keyword, we use `oper'
// instead.

namespace ll { namespace parser { namespace oper {


// Operator Table Entries
// -------- ----- -------

// Operator definition.
//
struct oper_struct;
typedef min::packed_struct_updptr<oper_struct> oper;
extern const uns32 & OPER;
    // Subtype of min::packed_struct<oper_struct>.
enum oper_flags
{
    PREFIX	= ( 1 << 0 ),
    INFIX	= ( 1 << 1 ),
    POSTFIX	= ( 1 << 2 ),
    NOFIX	= ( 1 << 3 ),
    AFIX	= ( 1 << 4 )
};
const min::int32 NO_PRECEDENCE = -1 << 31;
    // Value less than any allowed precedence.
typedef bool ( * reformatter )
        ( ll::parser::parser parser,
	  ll::parser::pass pass,
	  ll::parser::table::selectors selectors,
	  ll::parser::token & first,
	  ll::parser::token next,
	  ll::parser::oper::oper first_oper );
struct oper_struct
    : public ll::parser::table::root_struct
{
    // Packed_struct subtype is OPER.

    min::uns32 flags;
    min::int32 precedence;
    ll::parser::oper::reformatter reformatter;

};

MIN_REF ( min::gen, label, ll::parser::oper::oper )

void push_oper
	( min::gen oper_label,
	  ll::parser::table::selectors selectors,
	  min::uns32 flags,
	  min::int32 precedence,
	  ll::parser::oper::reformatter reformatter,
	  ll::parser::table::table oper_table );

} } }

# endif // LL_PARSER_OPER_H
