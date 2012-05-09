// Layers Language Operator Parser
//
// File:	ll_parser_oper.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed May  9 02:09:10 EDT 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Operator Table Entries
//	Operator Pass
//	Operator Reformatters

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
typedef void ( * reformatter )
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
    	// NULL if none.
};

MIN_REF ( min::gen, label, ll::parser::oper::oper )

// Create an operator definition entry with given
// label, selectors, flags, precedence, and reformatter,
// and push it into the given oper_table.
//
void push_oper
	( min::gen oper_label,
	  ll::parser::table::selectors selectors,
	  min::uns32 flags,
	  min::int32 precedence,
	  ll::parser::oper::reformatter reformatter,
	  ll::parser::table::table oper_table );

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
	// is used as the reformatter, and first_oper->
	// label is used as the .operator attribute
	// value by the default reformatter.
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

    const ll::parser::table::table oper_table;
    const ll::parser::oper::oper_stack oper_stack;
    min::uns32 temporary_count;
        // Number of temporary variables created so far.
	// The next temporary variable to be create will
	// be `$ T' where T == temporary_count.

};

MIN_REF ( ll::parser::pass, next,
          ll::parser::oper::oper_pass )
MIN_REF ( ll::parser::table::table, oper_table,
          ll::parser::oper::oper_pass )
MIN_REF ( ll::parser::oper::oper_stack, oper_stack,
          ll::parser::oper::oper_pass )

// Create an operator parser pass and place it on the
// parser->pass_stack just before `next', or if `next'
// is NULL_STUB, put `pass' at the end of the stack.
// Return the new operator parser pass.
//
ll::parser::oper::oper_pass place
	( ll::parser::parser parser,
	  ll::parser::pass next = NULL_STUB );

// Operator Reformatters
// -------- ------------

extern ll::parser::oper::reformatter
    separator_reformatter,
        // Nofix separator.  Empty expression inserted
	// to make operators all infix.  Operators
	// deleted and moved to .separator.
    right_associative_reformatter,
        // Infix right associative operators like
	// = and +=.  Operators made prefix binary.
    commutative_reformatter,
        // Infix commutative operators like *.
	// All operators must be equal: operator
	// made prefix with multiple operands.
    binary_reformatter,
        // Infix binary operator.  Errors announced
	// if more than one operator.
    sum_reformatter,
        // Infix + and - operators.  Rewritten to use
	// prefix - and commutative infix +.
    compare_reformatter,
    logical_reformatter;

} } }

# endif // LL_PARSER_OPER_H
