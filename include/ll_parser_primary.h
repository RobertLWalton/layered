// Layered Languages Primary Parser
//
// File:	ll_parser_primary.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Oct 31 05:23:09 EDT 2023
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

# include <ll_parser.h>
# include <ll_parser_table.h>

namespace ll { namespace parser { namespace primary {

namespace lexeme {

    extern min::locatable_gen
    	primary,		// primary
	primary_subexpressions,	// primary
				//   subexpressions
	variable,		// variable,
	function,		// function
	level,			// level
	depth,			// depth
	location,		// location
	module,			// module
	parentheses,		// ()
	square_brackets;	// []
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
    // Variable Description.
{
    // Packed_struct subtype is VAR.

    min::uns32 level; // Lexical level.
    min::uns32 depth; // Nesting depth within level.

    min::uns32 location;  // Offset in stack.
    const min::gen module;
        // Module containing location.  For testing,
	// this is a MIN string.  For compiling, it is
        // a mex::module or similar converted to a
        // min::gen.
};

MIN_REF ( min::gen, label,
          ll::parser::primary::var )
MIN_REF ( ll::parser::table::root, next,
          ll::parser::primary::var )
MIN_REF ( min::gen, module,
          ll::parser::primary::var )

// Create a variable definition entry with given
// label, selectors, lexical level, depth, location,
// and module, and return it.
//
ll::parser::primary::var create_var
	( min::gen var_label,
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  min::uns32 level,
	  min::uns32 depth,
	  min::uns32 location,
	  min::gen module );

// Function Definition.
//
struct func_struct;
struct arg_struct;
struct arg_list_struct;

typedef min::packed_vec_insptr<arg_struct>
	args;
typedef min::packed_vec_insptr<arg_list_struct>
	arg_lists;
typedef min::packed_struct_updptr<func_struct>
	func;

extern const uns32 & ARGS;
    // Subtype of min::packed_vec<args_struct>.
extern const uns32 & ARG_LISTS;
    // Subtype of min::packed_vec<arg_lists_struct>.
extern const uns32 & FUNC;
    // Subtype of min::packed_struct_with_base
    //        <func_struct, TAB::root_struct>.

// The output of parsing a function call is a reformat
// of the call that contains a function location
// followed by argument expressions; the list of
// argument expressions is called the argument vector.
//
// The argument vector elements correspond to the
// prototype arguments in left to right order.  A `func'
// contains an argument description (arg_struct) vector
// whose elements correspond to the argument vector.
//
// `func' also contains an argument list description
// (arg_list_struct) vector that has an element for each
// argument list of the prototype, in left to right
// order.  This includes lists preceeding the initial
// function-term-name and immediately following that
// name.
//
// Lastly `func' also contains a term-table that maps
// the non-initial function-term-names onto sequences
// of argument list descriptions.

struct arg_struct
    // Argument descriptor.
{
    min::gen name;
        // Name of argument variable, or MISSING if
	// not relevant.
    min::gen default_value;
        // Default value of argument, or NONE if none.
};
struct arg_list_struct
    // Argument list descriptor.
{
    min::uns32 number_of_args;
    	// Number of arguments in the list.
    min::uns32 first;
        // Index of first argument in the argument
	// vector.
    bool is_square;
        // True if [] list, false if () list.
};

struct func_struct
    : public ll::parser::table::root_struct
    // Function Description
{
    // Packed_struct subtype is FUNC.

    min::uns32 level; // Lexical level.
    min::uns32 depth; // Nesting depth within level.

    min::uns32 location;  // Offset in module code
    			  // vector.
    const min::gen module;
        // Module containing location.  For testing,
	// this is a MIN string.  For compiling, it is
	// a mex::module or similar converted to a
	// min::gen.

    const ll::parser::primary::args args;
        // Argument descriptions in prototype order.
    const ll::parser::primary::arg_lists arg_lists;
        // Argument list descriptions in prototype
	// order.
    const ll::parser::table::key_table term_table;
    	// See term_struc below.

    min::uns32 number_initial_arg_lists;
        // Number of argument lists before the first
	// term name in prototype.  If N>0, the list
	// descriptions are arg_lists[0] thru
	// arg_lists[N-1].
    min::uns32 number_following_arg_lists;
        // Number of argument lists immediately
	// following the first term name in prototype.
	// If M>0 the list descriptions are
	// arg_lists[N] thru arg_lists[N+M-1].
};

MIN_REF ( min::gen, label,
          ll::parser::primary::func )
MIN_REF ( ll::parser::table::root, next,
          ll::parser::primary::func )
MIN_REF ( min::gen, module,
          ll::parser::primary::func )
MIN_REF ( ll::parser::primary::args, args,
          ll::parser::primary::func )
MIN_REF ( ll::parser::primary::arg_lists, arg_lists,
          ll::parser::primary::func )
MIN_REF ( ll::parser::table::key_table, term_table,
          ll::parser::primary::func )

// Create a function definition entry with given
// label, selectors, lexical level, depth, location,
// module, number of initial/following arg lists,
// and return it.  Args and arg_lists vectors and
// term_table are created and initialized as empty.
// Term_table_size must be a power of 2.
//
ll::parser::primary::func create_func
	( min::gen func_label,
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  min::uns32 level,
	  min::uns32 depth,
	  min::uns32 location,
	  min::gen module,
	  min::uns32 number_initial_arg_lists,
	  min::uns32 number_following_arg_lists,
	  min::uns32 term_table_size = 32 );

// Push a new argument description into the args vector
// of a func.
//
inline void push_arg
    ( min::gen name,
      min::gen default_value,
      ll::parser::primary::func func )
{
    ll::parser::primary::arg_struct arg =
        { name, default_value };
    min::push(func->args) = arg;
    min::unprotected::acc_write_update
        ( func->args, name );
    min::unprotected::acc_write_update
        ( func->args, default_value );
}

// Prototype Function Term Definition.
//
struct func_term_struct;
typedef min::packed_struct_updptr<func_term_struct>
	func_term;
extern const uns32 & FUNC_TERM;
    // Subtype of min::packed_struct<func_term_struct>.

struct func_term_struct
    : public ll::parser::table::root_struct
    // Function Term Description
{
    min::uns32 first_arg_list,
               number_arg_lists;
        // Argument list descriptions are in function
	// arg_lists[F] through arg_lists[F+N-1], where
	// F is first_arg_list and N is number_arg_
	// lists.
    bool is_bool;
        // True iff term is a boolean term.  Then
	// number_of_argument_lists == 1 is required.
};

MIN_REF ( min::gen, label,
          ll::parser::primary::func_term )
MIN_REF ( ll::parser::table::root, next,
          ll::parser::primary::func_term )

// Create a function term definition entry with given
// label, arg_list_descriptions, and is_bool value, 
// and push it into the term_table of the given func.
// Selectors and block level are not relevant.
//
void push_func_term
	( min::gen func_term_label,
	  const min::phrase_position & position,
	  min::uns32 first_arg_list,
	  min::uns32 number_arg_lists,
	  bool is_bool,
	  ll::parser::primary::func func );

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
// vector, then if vp[i] is exists, is a string,
// number, or quoted string, and passes the lexical
// type test defined below, scan a name and return it.
// Otherwise return min::NONE().
//
// In the first case accept vp[i] as a name component
// and increment i.  Then as long as vp[i] is a string,
// number, or quoted string passing the lexical type
// test defined below, continue accepting vp[i] as a
// name component and incrementing i.
//
// The lexical type test is defined as follows.  Let
// XXX_types be initial_types for the first component of
// the name, and following_types for subsequent
// components.  Then the test is as follows:
//
//   If vp[i] is NOT a quoted string:
//     The test is passed iff the lexical type of vp[i]
//     is in both XXX_types and outside_quotes_types.
//
//   If vp[i] IS a quoted string and quoted_string_t is
//   NOT in XXX_types:
//     vp[i] is scanned for lexemes.  The test fails if
//     there are 0 or more than one lexeme.  Otherwise
//     the type of the one lexeme L is tested and the
//     test is passed iff the lexical type of L is in
//     both inside_quotes_types and XXX_types.  If the
//     test is passed, L (and NOT vp[i]) is accepted as
//     the next name component.
//
//   If vp[i] IS a quoted string and quoted_string_t IS
//   in XXX_types:
//      The test is passed and the string quoted is
//      accepted as the next name component.
//
// If the returned name has a single component, string
// or number, return the element.  Otherwise return a
// label containing the name components.
//
// If no name components are found, i is not incremented
// and MISSING is returned.
//
const min::uns64 ALL_TYPES = (min::uns64) -1ll;
min::gen scan_name
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      ll::parser::parser parser,
      min::uns64 initial_types,
      min::uns64 following_types,
      min::uns64 outside_quotes_types,
      min::uns64 inside_quotes_types );

// Types specific to a variable name, and an inline
// function to call scan_name with these types.
//
extern min::uns64 var_initial_types;
extern min::uns64 var_following_types;
extern min::uns64 var_outside_quotes_types;
extern min::uns64 var_inside_quotes_types;
inline min::gen scan_var_name
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      ll::parser::parser parser )
{
    return scan_name ( vp, i, parser,
                       var_initial_types,
		       var_following_types,
		       var_outside_quotes_types,
		       var_inside_quotes_types );
}

// Types specific to a function term name, and an inline
// function to call scan_name with these types.
//
extern min::uns64 func_term_initial_types;
extern min::uns64 func_term_following_types;
extern min::uns64 func_term_outside_quotes_types;
extern min::uns64 func_term_inside_quotes_types;
inline min::gen scan_func_term_name
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      ll::parser::parser parser )
{
    return scan_name ( vp, i, parser,
                       func_term_initial_types,
		       func_term_following_types,
		       func_term_outside_quotes_types,
		       func_term_inside_quotes_types );
}

// Scan function prototype and store results in a func
// which is returned.  Specifically vp is scanned
// beginning with vp[i], i is incremented during the
// scan, and i == vp.length at the end of the scan if
// there are no errors.
//
// A prototype argument with a default value must be of
// the form
//
// 	{|<var-name>|} <default-op> {|<default-value>|}
//
// where default-op is a MIN string.  Without a default
// value, the argument may be a variable name or be
// empty.
//
// A bool function term must be have a single argument
// list with a single element the above form that has
// default value equal to bool_value.
//
// The variable names are stored in `variables' in the
// order than they appear in the prototype.  If
// variables is NULL_STUB, it will be created.  If
// no variables are found, `variables' will be created
// but will be empty.  If no function prototype is
// found, `variables' will not be created.
//
// The function name is returned in func_name.  The
// function name consists of argument list tokens
// followed by a function term name.  The argument list
// tokens are the MIN strings "::P" for a parenthesized
// argument list and "::S" for a square bracketted
// argument list.  The function name is a MIN label
// or if that would have just one element, a MIN string
// equal to that element.  It cannot be an empty label,
// and cannot be just "::P".
//
// It is assumed that vp contains a function prototype
// ending at the end of vp.  If a defective prototype is
// found, parse_error is called one or more times, and
// NULL_STUB is returned.
//
// Name_only signifies that only func_name is to be
// returned and modifies the above as follows.  An empty
// label is returned as min::NONE(), a label that is
// just "::P" is allowed, the prototype may end before
// the end of vp, the form of arguments given above is
// not checked (e.g., () or (,) or [a + b] are allowed),
// variables is not set, the return func is NULL_STUB,
// and there are no error messages.
//
typedef min::packed_vec_insptr<min::gen>
    variables_vector;
extern min::locatable_gen func_default_op;
extern min::locatable_gen func_bool_value;
ll::parser::primary::func scan_func_prototype
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      ll::parser::parser parser,
      min::ref<min::gen> func_name,
      min::ref<variables_vector> variables,
      bool name_only = false,
      min::gen default_op = func_default_op,
      min::gen bool_value = func_bool_value );


// Scan a reference expression prototype and return
// the primary_table entry found, either a var if a
// variable is found or a func if a function call is
// found.
//
// Specifically vp is scanned beginning with vp[i] and i
// is incremented during the scan.  The scan stops when
// the next vector element cannot be part of the
// variable name or function call.
//
// True is returned if a primary_table entry was found,
// and false otherwise.
//
// The first call to this function for a particular vp
// and i should have key_prefix == NULL_STUB.  If after
// returning, the result is unsatisfactory because the
// next vp[i] is not a suitable subsequent vector
// element, the result can be rejected and the next
// primary_table entry can be found by re-calling this
// function with i, root, and key_prefix left as they
// were set by the last call.  This function can be
// re-called in this manner until it returns false.
//
// Note that selectors in primary_table entries are
// ignored.
//
typedef min::packed_vec_insptr<min::gen>
    arguments_vector;
extern min::locatable_gen TRUE, FALSE;
bool scan_ref_expression
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      ll::parser::parser parser,
      ll::parser::table::root & root,
      ll::parser::table::key_prefix & key_prefix,
      min::ref<arguments_vector> arguments,
      ll::parser::table::key_table primary_table );

} } }

# endif // LL_PARSER_PRIMARY_H
