// Layers Language Operator Parser
//
// File:	ll_parser_oper.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Feb 13 03:41:07 EST 2014
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
extern min::locatable_gen prefix;
extern min::locatable_gen infix;
extern min::locatable_gen postfix;
extern min::locatable_gen afix;
extern min::locatable_gen nofix;


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
    AFIX	= ( 1 << 4 ),
    ALLFIX	= PREFIX | INFIX | POSTFIX
                | NOFIX | AFIX
};
const min::int32 NO_PRECEDENCE = -1 << 31;
    // Value less than any allowed precedence.
typedef bool ( * reformatter )
        ( ll::parser::parser parser,
	  ll::parser::pass pass,
	  ll::parser::table::flags selectors,
	  ll::parser::token & first,
	  ll::parser::token next,
	  ll::parser::table::flags trace_flags,
	  ll::parser::oper::oper first_oper,
	  min::phrase_position & position );
    // A reformatter reformats the tokens from first to
    // next->previous.  Trace_flags are passed to
    // `compact' if the reformatter calls that.
    // First_oper is the first OPERATOR in the tokens
    // and `position' is the position of the tokens.
    // Pass is the operator pass, and parser and
    // selectors are the current parser and selectors.
    //
    // The reformatter may change `first'.  Note that if
    // this is done, a recalculated position would be
    // incorrect, which is why `position' is calculated
    // before the reformatter is called.
    //
    // If true is returned, the caller of the
    // reformatter will immediately call ll:: parser::
    // compact (even if first->next == next) to make a
    // BRACKETABLE token with no syntax attributes, but
    // with the given `position' as the .position
    // attribute and with first_oper->label as the
    // .operator attribute.

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
    min::packed_vec_ptr<min::gen> reformatter_arguments;
        // Vector of arguments each a min string or
	// label.  NULL if none.
};

MIN_REF ( min::gen, label, ll::parser::oper::oper )
MIN_REF ( min::gen, terminator, ll::parser::oper::oper )
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

// Operator Reformatters
// -------- ------------

// There is a single reformatter table set up by program
// initialization.  This maps reformatter names to
// reformatters and vice versa.  A reformatter is legal
// only for operators all of whose operator flags are
// in the set of legal operator flags given.

struct reformatter_table_struct
{
    min::gen name;
    min::uns32 oper_flags;
    ll::parser::oper::reformatter reformatter;
};

struct reformatter_table_struct;
typedef min::packed_vec_insptr
	    <reformatter_table_struct>
	reformatter_table_type;
extern const uns32 & REFORMATTER_TABLE_TYPE;
    // Subtype of min::packed_vec
    // //              <reformatter_table_struct>.

// The one and only reformatter table.
//
extern min::locatable_var
	<ll::parser::oper::reformatter_table_type>
    reformatter_table;

// Look up reformatter name in reformatter table, and
// return reformatter if found, or NULL if not found.
// If a reformatter is found, also return the set
// of operator flags allowed for operators having the
// reformatter.
//
inline ll::parser::oper::reformatter find_reformatter
	( min::gen name, min::uns32 & oper_flags )
{
    ll::parser::oper::reformatter_table_type t =
        ll::parser::oper::reformatter_table;
    for ( min::uns32 i = 0; i < t->length; ++ i )
        if ( t[i].name == name )
	{
	    oper_flags = t[i].oper_flags;
	    return t[i].reformatter;
	}
    return NULL;
}
//
// Look up reformatter in reformatter table, and return
// reformatter name if found, or MISSING if not found.
//
inline min::gen find_name
	( ll::parser::oper::reformatter reformatter )
{
    ll::parser::oper::reformatter_table_type t =
        ll::parser::oper::reformatter_table;
    for ( min::uns32 i = 0; i < t->length; ++ i )
        if ( t[i].reformatter == reformatter )
	    return t[i].name;
    return min::MISSING();
}

// Push reformatter name and reformatter into
// reformatter table.
//
inline void push_reformatter
    ( min::gen name,
      min::uns32 oper_flags,
      ll::parser::oper::reformatter reformatter )
{
    reformatter_table_struct e =
        { name, oper_flags, reformatter };
    min::push ( (ll::parser
                   ::oper::reformatter_table_type)
                ll::parser::oper::reformatter_table )
        = e;
    min::unprotected::acc_write_update 
	( ll::parser::oper::reformatter_table,
	  name );
}

} } }

# endif // LL_PARSER_OPER_H
