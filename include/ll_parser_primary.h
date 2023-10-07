// Layered Languages Primary Parser
//
// File:	ll_parser_primary.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Oct  7 06:04:38 EDT 2023
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Primary Table Entries
//	Primary Pass
//	Primary Functions

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_PRIMARY_H
# define LL_PARSER_PRIMARY_H

# include <mex.h>
# include <ll_parser.h>

namespace ll { namespace parser { namespace primary {

const MAX_CONSECUTIVE_ARG_LISTS = 4;
    // Maximum number of argument lists that may appear
    // consecutively in a function call or pattern
    // term.

namespace lexeme {

    extern min::locatable_gen
    	primary,		// primary
	primary_subexpressions;	// primary
				//   subexpressions
}


// Primary Table Entries
// -------- ----- -------

// Variable Definition.
//
struct var_struct;
typedef min::packed_struct_updptr<var_struct> var;
extern const uns32 & VAR;
    // Subtype of min::packed_struct<var_struct>.

struct var_struct
    : public ll::parser::table::root_struct
{
    // Packed_struct subtype is VAR.

    min::uns32 level; // Lexical level.
    min::int32 depth; // Nesting depth within level.

    min::int32 location;  // Offset in stack.
    mex::module module;   // Module contining location
    			  // if variable global, or
			  // min::NULL_STUB if local.
};

MIN_REF ( min::gen, label,
          ll::parser::primary::var )
MIN_REF ( parser::table::root, next,
          ll::parser::primary::var )
MIN_REF ( mex::module, module,
          ll::parser::primary::var )

// Create a variable definition entry with given
// label, selectors, lexical level, depth, location,
// and module, and push it into the given primary_table.
//
void push_var
	( min::gen var_label,
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  min::uns32 level,
	  min::uns32 depth,
	  min::uns32 location,
	  mex::module module,
	  ll::parser::table::key_table primary_table );

// Function Definition.
//
struct func_struct;
typedef min::packed_vec_updptr<min::gen,func_struct>
	func;
extern const uns32 & FUNC;
    // Subtype of min::packed_vec<func_struct>.

// The output of parsing a function call is a reformat
// of the call that contains a function location
// followed by argument expressions; the list of
// argument expressions is called the argument vector.
//
// The argument vector elements correspond to the
// prototype arguments in left to right order.  A `func'
// is a prototype vector for the argument vector that
// contains the min::gen default value of each argument
// (or NONE() if there is no default).
//
// `func' also contains descriptions of the argument
// lists preceeding the initial function-term-name and
// immediately following that name, and a term-table
// that maps the remaining function-term-names onto
// argument list descriptions.

struct arg_list_description
{
    min::uns32 number_of_args;
    	// Number of arguments in the list.
    min::uns32 first;
        // Index of first argument in the argument
	// vector.
    bool is_square;
        // True if [] list, false if () list.
};

typedef arg_list_description[MAX_CONSECUTIVE_ARG_LISTS]
    arg_list_descriptions;

struct func_struct
    : public ll::parser::table::root_struct
{
    // Packed_vec subtype is FUNC.

    min::uns32 level; // Lexical level.
    min::int32 depth; // Nesting depth within level.

    min::int32 location;  // Offset in module code
    			  // vector.
    mex::module module;   // Module contining location
    			  // if function global, or
			  // min::NULL_STUB if local.
    ll::parser::table::key_table term_table;
    			  // See term_struc below.
    ll::parser::primary::arg_list_descriptions
         initial_arg_lists;
	 // For arg lists preceeding first function-
	 // term-name.
    ll::parser::primary::arg_list_descriptions
         following_arg_lists;
	 // For arg lists following first function-
	 // term-name.
};

MIN_REF ( min::gen, label,
          ll::parser::primary::func )
MIN_REF ( parser::table::root, next,
          ll::parser::primary::func )
MIN_REF ( mex::module, module,
          ll::parser::primary::func )
MIN_REF ( ll::parser::tabke::key_table, term_table,
          ll::parser::primary::func )

// Create a function definition entry with given
// label, selectors, lexical level, depth, location,
// module, intitial_arg_lists, and following_arg_
// lists, and push it into the given primary_table.
//
void push_func
	( min::gen func_label,
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  min::uns32 level,
	  min::uns32 depth,
	  min::uns32 location,
	  mex::module module,
	  ll::parser::primary::arg_list_descriptions
	      initial_arg_lists,
	  ll::parser::primary::arg_list_descriptions
	      following_arg_lists,
	  ll::parser::table::key_table primary_table );

// Prototype Function Term Definition.
//
struct func_term_struct;
typedef min::packed_struct_updptr<func_term_struct>
	func_term;
extern const uns32 & FUNC_TERM;
    // Subtype of min::packed_struct<func_term_struct>.

struct func_term_struct
    : public ll::parser::table::root_struct
{
    ll::parser::primary::arg_list_descriptions
        arg_lists;
    bool is_bool;
        // True iff term is a boolean term.
};

MIN_REF ( min::gen, label,
          ll::parser::primary::func_term )
MIN_REF ( parser::table::root, next,
          ll::parser::primary::func_term )

// Create a function term definition entry with given
// label, arg_list_descriptions, and is_bool value, 
// and push it into the given term_table.  Selectors
// and block level are not relevant.
//
void push_func
	( min::gen func_label,
	  const min::phrase_position & position,
	  ll::parser::primary::arg_list_descriptions
	      arg_lists,
	  ll::parser::table::key_table term_table );

// Primary Pass
// ------- ----

struct primary_pass_struct;
typedef min::packed_struct_updptr<primary_pass_struct>
        primary_pass;
extern const uns32 & PRIMARY_PASS;
    // Subtype of min::packed_struc
    //                      <primary_pass_struct>.
struct primary_pass_struct
    : public ll::parser::pass_struct
{
    // Packed_struct subtype is PRIMARY_PASS.

    const ll::parser::table::key_table
        primary_table;
	// Table of primary keys.

    ll::parser::table::flags trace_subexpressions;
        // Trace flag named `primary subexpressions'
	// that traces primary subexpressions.

};

MIN_REF ( min::gen, name,
          ll::parser::primary::primary_pass )
MIN_REF ( ll::parser::parser, parser,
          ll::parser::primary::primary_pass )
MIN_REF ( ll::parser::pass, next,
          ll::parser::primary::primary_pass )
MIN_REF ( ll::parser::table::key_table, primary_table,
          ll::parser::primary::primary_pass )

// Return a new primary parser pass.
//
ll::parser::pass new_pass ( ll::parser::parser parser );

// Find the primary_pass in the parser pass table, or if
// not there, add it before next_pass (NULL_STUB means
// add at end).  Return the primary_pass.
//
ll::parser::primary::primary_pass init_primary
    ( ll::parser::parser parser,
      ll::parser::pass next = NULL_STUB );

// Primary Functions
// ------- ---------

// Given an object vector pointer vp pointing at an
// expression, and an index i of an element in the
// vector, then if vp[i] is as exists, is a string
// or number, and has ll::parser_lexical_type in
// initial_types, scan a name and return it.
// Otherwise return min::NONE().
//
// In the first case accept vp[i] as a name element
// and increment i.  Then as long as vp[i] is a string
// or number and has ll::parser::lexical_type in
// following_types, continue accepting vp[i] as a name
// element and incrementing i.
//
// If a quoted string is encountered, scan the string
// for lexemes, and if there is exactly one, use it
// instead of the quoted string.  Otherwise return
// min::ERROR().
//
// If the returned name has a single element, string or
// number, return the element.  Otherwise return a label
// containing the name elements.
//
min::gen scan_name
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      ll::parser::parser parser,
      min::uns64 initial_types,
      min::uns64 following_types );

} } }

# endif // LL_PARSER_PRIMARY_H
