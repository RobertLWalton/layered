// Layered Languages Operator Parser
//
// File:	ll_parser_oper.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Jun 25 15:48:14 EDT 2015
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Operator Table Entries
//	Operator Pass

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_OPER_H
# define LL_PARSER_OPER_H

# include <ll_parser.h>

// As `operator' is a C++ keyword, we use `oper'
// instead.

namespace ll { namespace parser { namespace oper {

extern min::locatable_gen dollar;
extern min::locatable_gen AND;
extern min::locatable_gen prefix;
extern min::locatable_gen infix;
extern min::locatable_gen postfix;
extern min::locatable_gen afix;
extern min::locatable_gen nofix;

extern min::locatable_var<ll::parser::reformatter>
		reformatter_stack;


// Operator Table Entries
// -------- ----- -------

// Operator definition.
//
enum oper_flags
{
    PREFIX	= ( 1 << 0 ),
    INFIX	= ( 1 << 1 ),
    POSTFIX	= ( 1 << 2 ),
    NOFIX	= ( 1 << 3 ),
    AFIX	= ( 1 << 4 ),
    ALLFIX	= PREFIX | INFIX | POSTFIX
                | NOFIX | AFIX
};
const min::int32 NO_PRECEDENCE = -1 << 31;
    // Value less than any allowed precedence.

struct oper_struct;
typedef min::packed_struct_updptr<oper_struct> oper;
extern const uns32 & OPER;
    // Subtype of min::packed_struct<oper_struct>.

struct oper_struct
    : public ll::parser::table::root_struct
{
    // Packed_struct subtype is OPER.

    min::uns32 flags;
    min::int32 precedence;
    min::gen terminator;
        // For an entry in the oper_bracket_table, this
	// is the terminator required of a BRACKETTED
	// expression in order for that to be recognized
	// as an operator.  The root label is the
	// required initiator.  min::MISSING() if
	// entry as no terminator (as do entries in the
	// oper_table).
    ll::parser::reformatter reformatter;
    	// NULL_STUB if none.
    const ll::parser::reformatter_arguments
    	    reformatter_arguments;
        // Vector of arguments each a min string or
	// label.  NULL_STUB if none.
};

MIN_REF ( min::gen, label, ll::parser::oper::oper )
MIN_REF ( min::gen, terminator, ll::parser::oper::oper )
MIN_REF ( ll::parser::reformatter, reformatter,
          ll::parser::oper::oper )
MIN_REF ( ll::parser::reformatter_arguments,
          reformatter_arguments,
	  ll::parser::oper::oper )

// Create an operator definition entry with given
// label, terminator (MISSING except for oper_bracket_
// table), selectors, flags, precedence, and
// reformatter, and push it into the given oper_table.
//
void push_oper
	( min::gen oper_label,
	  min::gen terminator,
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  min::uns32 flags,
	  min::int32 precedence,
	  ll::parser::reformatter reformatter,
	  ll::parser::reformatter_arguments
	      reformatter_arguments,
	  ll::parser::table::key_table oper_table );

// Operator Pass
// -------- ----

// An oper_stack is part of each oper_pass.
//
struct oper_stack_struct
{
    // Save of various operator parser variables.
    // None of these saved values are ACC locatable.
    //
    min::int32 precedence;
    ll::parser::token first;
    ll::parser::oper::oper first_oper;
        // First OPERATOR in subexpression, or NULL_
	// STUB if none yet.  first_oper->reformatter
	// is used as the reformatter, first_oper->
	// reformatter_arguments as the reformatter
	// arguments, and first_oper->label as the
	// .operator attribute value by the default
	// reformatter.
};

typedef min::packed_vec_insptr< oper_stack_struct >
    oper_stack;

struct oper_pass_struct;
typedef min::packed_struct_updptr<oper_pass_struct>
        oper_pass;
extern const uns32 & OPER_PASS;
    // Subtype of min::packed_struct<oper_pass_struct>.
struct oper_pass_struct
    : public ll::parser::pass_struct
{
    // Packed_struct subtype is OPER_PASS.

    const ll::parser::table::key_table
        oper_table;
	// Table of normal operators.

    const ll::parser::table::key_table
        oper_bracket_table;
	// Table of bracket and indentation mark
	// operators.  The .initiators are the root
	// labels.

    const ll::parser::oper::oper_stack oper_stack;

    min::uns32 temporary_count;
        // Number of temporary variables created so far.
	// The next temporary variable to be create will
	// be `$ T' where T == temporary_count.

    ll::parser::table::flags trace_subexpressions;
        // Trace flag named `operator subexpressions'
	// that traces operator subexpressions.

};

MIN_REF ( min::gen, name,
          ll::parser::oper::oper_pass )
MIN_REF ( ll::parser::parser, parser,
          ll::parser::oper::oper_pass )
MIN_REF ( ll::parser::pass, next,
          ll::parser::oper::oper_pass )
MIN_REF ( ll::parser::table::key_table, oper_table,
          ll::parser::oper::oper_pass )
MIN_REF ( ll::parser::table::key_table,
          oper_bracket_table,
          ll::parser::oper::oper_pass )
MIN_REF ( ll::parser::oper::oper_stack, oper_stack,
          ll::parser::oper::oper_pass )

// Return a new operator parser pass.
//
ll::parser::pass new_pass ( void );

} } }

# endif // LL_PARSER_OPER_H
