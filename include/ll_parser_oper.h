// Layered Languages Operator Parser
//
// File:	ll_parser_oper.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri May 23 07:21:46 AM EDT 2025
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
# include <climits>

// As `operator' is a C++ keyword, we use `oper'
// instead.

namespace ll { namespace parser { namespace oper {

namespace lexeme {

    extern min::locatable_gen
    	dollar,		// $
	AND,		// AND
	initial,	// initial
	left,		// left
	right,		// right
	final,		// final
	afix,		// afix
	line,		// line
	prefix,		// prefix
	infix,		// infix
	postfix,	// postfix
	nofix,		// nofix
	end_operator,	// END'OPERATOR
	error_operator,	// ERROR'OPERATOR
	error_operand,	// ERROR'OPERAND
	error_separator,// ERROR'SEPARATOR
        operator_subexpressions,
	                // operator subexpressions
        oper,           // operator
        bracket,        // bracket
        indentation,    // indentation
        mark,           // mark
        precedence,     // precedence
        operators,      // operators
        control,        // control
        iteration,      // iteration
        times;		// times
}

extern min::locatable_var<ll::parser::reformatter>
		reformatter_stack;


// Operator Table Entries
// -------- ----- -------

// Operator definition.
//
enum oper_flags
{
    LEFT	= ( 1 << 0 ),
    FINAL	= ( 1 << 1 ),
    RIGHT	= ( 1 << 2 ),
    INITIAL	= ( 1 << 3 ),
    AFIX	= ( 1 << 4 ),
    LINE	= ( 1 << 5 ),

    PREFIX	= INITIAL + RIGHT,
    INFIX	= LEFT + RIGHT,
    POSTFIX	= FINAL + LEFT,
    NOFIX	= 0,

    ALLFIX	= LEFT | FINAL | RIGHT | INITIAL |
                  AFIX | LINE
};
const min::int32 NO_PRECEDENCE = INT_MIN;
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
    min::gen reformatter_arguments;
        // Reformatter and its arguments, or NULL_STUB
	// and MISSING if none.
};

MIN_REF ( ll::parser::table::root, next,
          ll::parser::oper::oper )
MIN_REF ( min::gen, label, ll::parser::oper::oper )
MIN_REF ( min::gen, terminator, ll::parser::oper::oper )
MIN_REF ( ll::parser::reformatter, reformatter,
          ll::parser::oper::oper )
MIN_REF ( min::gen, reformatter_arguments,
	  ll::parser::oper::oper )

extern min::locatable_var<oper> end_oper;
extern min::locatable_var<oper> error_oper;

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
	  min::gen reformatter_arguments,
	  ll::parser::table::key_table oper_table );

// Operator Pass
// -------- ----

// Highest and lowest precedence of defined infix or
// nofix operators:
//
extern min::int32 high_precedence;
extern min::int32 low_precedence;

// An oper_stack is part of each oper_pass.
//
struct oper_stack_struct
{
    // Save of various operator parser variables.
    // Members are not ACC locatable.
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

// An oper_vec lists the operators and non-operators in
// the current expression in order, and is part of each
// oper_pass.  Each element corresponds to an operator
// or non-operator in a current expression, in the order
// that these appear.
//
// Oper_parser_pass_1 builds the oper_vec but does NOT
// change tokens.  It considers each operator or non-
// operator to be a sequence of token.  Oper_parser_
// pass_2 packages each operator or non-operator into
// a single token.  If the expression is not a
// declaration, subsequence passes are run on non-
// operators, and if the result is more than one token,
// the tokens are packages into a single PURELIST token.
// If the expression is a declaration, non-operators
// that are not single tokens are packaged into a single
// PURELIST token, but no subsequence passes are run.
//
// Non-operators cannot be adjacent to one another.
//
struct oper_vec_struct
{
    // Members are not ACC locatable.

    ll::parser::token first;
        // First token of operator or non-operator.
	// Set by oper_parser_pass_1 and used only by
	// oper_parser_pass_2.
    oper op;	// NULL_STUB for non-operator.
    min::uns32 flags;		// Only for operators.
    min::int32 precedence;	// Only for operators.
};

typedef min::packed_vec_insptr< oper_vec_struct >
    oper_vec;
 
// Return true iff a possible operator of given
// precedence and flags is allowed after the
// token to its left, or in the case of op ==
// NULL_STUB, if a non-operator is allowed after the
// token to its left.  The OP::LINE flag is NOT
// checked.  Return false otherwise.
//
// If true is returned, push an entry with first,
// op, flags, and precedence into v.
//
bool flags_OK ( oper_vec v,
		ll::parser::token first,
		oper op = min::NULL_STUB,
	        min::uns32 flags = 0,
		min::int32 precedence = 0 );

// Insert a token with name ERROR'OPERAND and type
// word_t before/after token t.  Call parse_error with
// missing operand error message.
//
void put_error_operand_before
    ( ll::parser::parser parser,
      ll::parser::token t );
void put_error_operand_after
    ( ll::parser::parser parser,
      ll::parser::token t );

// Delete t and return token after t.  Call parse_error
// with message
// 	"message [pgen_quote(t->value)]; deleted".
//
ll::parser::token delete_bad_token
    ( ll::parser::parser parser,
      ll::parser::token t,
      const char * message );

// If t != next, delete tokens from t to next->previous
// and call parse_error about extra stuff at end of
// expression.  Return `next'.
//
ll::parser::token delete_extra_stuff
    ( ll::parser::parser parser,
      ll::parser::token t,
      ll::parser::token next );

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

    const ll::parser::oper::oper_vec oper_vec;

    const ll::parser::oper::oper_stack oper_stack;

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
MIN_REF ( ll::parser::oper::oper_vec, oper_vec,
          ll::parser::oper::oper_pass )
MIN_REF ( ll::parser::oper::oper_stack, oper_stack,
          ll::parser::oper::oper_pass )

// Return a new operator parser pass.
//
ll::parser::pass new_pass ( ll::parser::parser parser );

// Find the oper_pass in the parser pass table, or if
// not there, add it before next_pass (NULL_STUB means
// add at end).  Return the oper_pass.
//
ll::parser::oper::oper_pass init_oper
    ( ll::parser::parser parser,
      ll::parser::pass next = NULL_STUB );

} } }

# endif // LL_PARSER_OPER_H
