// Layered Languages Operator Parser
//
// File:	ll_parser_oper.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Jun 25 13:37:25 EDT 2015
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Reformatter Table Entries
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


// Reformatter Table Entries
// ----------- ----- -------

// There is a single reformatter stack set up by program
// initialization.  This lists all the reformatters.  A
// reformatter is legal only for operators all of whose
// operator flags are in the set of legal operator flags
// given.  A minimum and maximum is also given for the
// number of arguments that a reformatter may have.

struct oper_struct;
typedef min::packed_struct_updptr<oper_struct> oper;
extern const uns32 & OPER;
    // Subtype of min::packed_struct<oper_struct>.

typedef min::packed_vec_ptr<min::gen>
	reformatter_arguments;
    // Argument list for a reformatter.

typedef bool ( * reformatter_function )
        ( ll::parser::parser parser,
	  ll::parser::pass pass,
	  ll::parser::table::flags selectors,
	  ll::parser::token & first,
	  ll::parser::token next,
	  ll::parser::table::flags trace_flags,
	  ll::parser::oper::reformatter_arguments
	  	reformatter_arguments,
	  min::phrase_position & position );
    //
    // A reformatter_function reformats the tokens from
    // first to next->previous.  Trace_flags are passed
    // to `compact', if the function calls that.
    // First_oper is the first OPERATOR in the tokens
    // and `position' is the position of the tokens.
    // Pass is the operator pass, and parser and
    // selectors are the current parser and selectors.
    //
    // The function may change `first'.  Note that if
    // this is done, a recalculated position would be
    // incorrect, which is why `position' is calculated
    // before the function is called.
    //
    // If true is returned, the caller of the function
    // will immediately call ll:: parser::compact (even
    // if first->next == next) to make a BRACKETABLE
    // token with no syntax attributes, but with the
    // given `position' as the .position attribute and
    // with first_oper->label as the .operator
    // attribute.

struct reformatter_struct;
typedef min::packed_struct_updptr<reformatter_struct>
        reformatter;
extern const uns32 & REFORMATTER;
    // Subtype of
    // min::packed_struct<reformatter_struct>.
struct reformatter_struct
{
    min::uns32 control;

    ll::parser::oper::reformatter next;
    min::gen name;
    min::uns32 oper_flags;
    min::uns32 minimum_arguments, maximum_arguments;
        // Minimum and maximum number of arguments.
    ll::parser::oper::reformatter_function
        reformatter_function;
};
MIN_REF ( ll::parser::oper::reformatter, next,
          ll::parser::oper::reformatter )
MIN_REF ( min::gen, name,
          ll::parser::oper::reformatter )

// The head of the reformatter stack.
//
extern min::locatable_var<ll::parser::oper::reformatter>
    reformatter_stack;

// Look up reformatter name in reformatter stack, and
// return reformatter if found, or NULL_STUB if not
// found.
//
inline ll::parser::oper::reformatter find_reformatter
	( min::gen name )
{
    ll::parser::oper::reformatter r =
        ll::parser::oper::reformatter_stack;
    while ( r != min::NULL_STUB )
    {
        if ( r->name == name ) return r;
	r = r->next;
    }
    return min::NULL_STUB;
}

// Push a new reformatter into the reformatter stack.
//
void push_reformatter
    ( min::gen name,
      min::uns32 oper_flags,
      min::uns32 minimum_arguments, 
      min::uns32 maximum_arguments,
      ll::parser::oper::reformatter_function
          reformatter_function );

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
    ll::parser::oper::reformatter reformatter;
    	// NULL if none.
    const ll::parser::oper::reformatter_arguments
    	    reformatter_arguments;
        // Vector of arguments each a min string or
	// label.  NULL if none.
};

MIN_REF ( min::gen, label, ll::parser::oper::oper )
MIN_REF ( min::gen, terminator, ll::parser::oper::oper )
MIN_REF ( ll::parser::oper::reformatter, reformatter,
          ll::parser::oper::oper )
MIN_REF ( min::packed_vec_ptr<min::gen>,
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
	  ll::parser::oper::reformatter reformatter,
	  ll::parser::oper::reformatter_arguments
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
