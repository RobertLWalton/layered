// Layered Languages Primary Parser
//
// File:	ll_parser_primary.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Nov  4 17:36:50 EDT 2023
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
	parentheses,		// ;;P
	square_brackets,	// ;;S
	TRUE;			// TRUE
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
	( ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  min::uns32 level,
	  min::uns32 depth,
	  min::uns32 location,
	  min::gen module,
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
// The lexical type of a number is number_t, natural_t,
// or numeric_word_t.  The lexical type of a string is
// word_t, numeric_t, mark_t, separator_t, or other.
// Strings containing multiple lexemes have other type.
// A quoted string vp element is treated the same as a
// string vp element.
//
// Let XXX_types be initial_types for the first
// component of the name, and following_types for
// subsequent components.  Then the lexical type test
// is as follows:
//
//   A string or quoted string whose lexical type is
//     other always fails the test.
//
//   A number or string that is not quoted passes the
//     test is its type is in both both XXX_types and
//     outside_quotes_types.
//
//   A quoted string passes the test is its type is in
//     both both XXX_types and inside_quotes_types.
//
// If the returned name has a single component, string
// or number, return the element.  Otherwise return a
// label containing the name components.
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

// Scan function label.  vp[i] should be the beginning
// of a function prototype that may include () and []
// argument lists before the first function term name.
// The scan stops at the end of vp or at the first
// element that cannot be part of a function prototype.
//
// Specifically, a (...) at the beginning of the scan
// produces the MIN string ";;P" in the label, and a
// [...] produces a ";;S".  These are followed by the
// first function term in the prototype.
//
// The contents of (...) and [...] are NOT checked.
// Partial labels are allowed, but the empty label
// returns min::NONE().  A label with one element is
// returned as that element, and labels with more
// elements are returned as MIN labels.
//
min::gen scan_func_label
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      ll::parser::parser parser );

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
// It is assumed that vp contains a function prototype
// ending at the end of vp.  If a defective prototype is
// found, parse_error is called one or more times, and
// NULL_STUB is returned.
//
typedef min::packed_vec_insptr<min::gen>
    variables_vector;

extern min::locatable_gen func_default_op;
extern min::locatable_gen func_bool_value;
extern min::uns32 func_term_table_size;

ll::parser::primary::func scan_func_prototype
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      min::phrase_position_vec ppvec,
      ll::parser::parser parser,
      ll::parser::table::flags selectors,
      min::uns32 block_level,
      min::uns32 level,
      min::uns32 depth,
      min::uns32 location,
      min::gen module,
      min::gen default_op = func_default_op,
      min::gen bool_value = func_bool_value,
      min::uns32 term_table_size =
          func_term_table_size );


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
bool scan_ref_expression
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      ll::parser::parser parser,
      ll::parser::table::root & root,
      ll::parser::table::key_prefix & key_prefix,
      min::ref<arguments_vector> arguments,
      ll::parser::table::key_table primary_table );

} } }

# endif // LL_PARSER_PRIMARY_H
