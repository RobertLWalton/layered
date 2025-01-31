// Layered Languages Primary Parser
//
// File:	ll_parser_primary.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Jan 29 02:32:51 AM EST 2025
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
	square_brackets;	// ;;S
}


// Primary Table Entries
// -------- ----- -------

// Variable Definition.
//
struct var_struct;
typedef min::packed_struct_updptr<var_struct> var;
extern const uns32 & VAR;
    // Subtype of min::packed_struct<var_struct>.

enum var_flags {
    WRITABLE_VAR	= ( 1 << 0 ),
    NEXT_VAR		= ( 1 << 1 )
};
struct var_struct
    : public ll::parser::table::root_struct
    // Variable Description.
{
    // Packed_struct subtype is VAR.

    min::uns32 flags;     // Variable flags.
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
	  const min::phrase_position & position,
	  min::uns16 level,
	  min::uns16 depth,
	  min::uns32 flags,
	  min::uns32 location,
	  min::gen module );

// Push var into symbol table.
//
inline void push_var
	( ll::parser::table::key_table symbol_table,
	  ll::parser::primary::var var )
{
    ll::parser::table::push
        ( symbol_table, (ll::parser::table::root) var );
}

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
	// not available due to error.
    min::gen default_value;
        // Default value of argument, or NONE if none.
};
struct arg_list_struct
    // Argument list descriptor.
{
    min::gen term_name;
        // Term name to be printed just before argument
	// list when printing prototype or call,
	// or min::NONE() if none.
    min::uns32 number_of_args;
    	// Number of arguments in the list.
    min::uns32 first;
        // Index of first argument in the argument
	// vector.
    bool is_square;
        // True if [] list, false if () list.
};

enum func_flags {
    BUILTIN_FUNCTION		= ( 1 << 0 ),
    VALUE_OPERATOR		= ( 1 << 1 ),
    LOGICAL_OPERATOR		= ( 1 << 2 )
};

enum logical_op_codes {
    COMPARE			= 1,
    AND				= 2,
    OR				= 3,
    NOT				= 4,
    BUT_NOT			= 5,
    IF				= 6,
    JMP				= 7
};

struct func_struct
    : public ll::parser::table::root_struct
    // Function Description
{
    // Packed_struct subtype is FUNC.
    //
    // root->block_level is set to
    //     (lexical_level << 16) + depth

    min::uns32 flags;	  // Function flags.
    min::uns32 location;  // Offset in module or
    			  // instruction table.
    const min::gen module;
        // Module containing location.  For testing,
	// this is a MIN string.  For compiling, it is
	// a mex::module or similar converted to a
	// min::gen, or NONE for BUILTIN_FUNCTION or
	// VALUE_OPERATOR or LOGICAL_OPERATOR.

    const ll::parser::primary::args args;
        // Argument descriptions in prototype order.
    const ll::parser::primary::arg_lists arg_lists;
        // Argument list descriptions in prototype
	// order.
    const ll::parser::table::key_table term_table;
    	// See funct_term_struc below.

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
// selectors, position, lexical level, depth, flags,
// location, module, and term table size and return it.
// Args and arg_lists vectors and term_table are created
// and initialized as empty, and number_of_{initial,
// following}_arg_lists are set to 0.  Term_table_size
// must be a power of 2.
//
ll::parser::primary::func create_func
	( ll::parser::table::flags selectors,
	  const min::phrase_position & position,
	  min::uns16 level,
	  min::uns16 depth,
	  min::uns32 flags,
	  min::uns32 location,
	  min::gen module,
	  min::uns32 term_table_size );

// Push into a symbol_table a top level VALUE_OPERATOR
// or LOGICAL_OPERATOR func describing an infix or
// postfix operator with given op_name and op_codes.
//
// Level and depth are set to 0, position is set to
// PAR::top_level_position, selectors are set to
// PAR::ALL_SELECTORS, module is set to NONE.
//
ll::parser::primary::func push_op
	( min::gen op_name,
	  ll::parser::table::key_table symbol_table,
	  min::uns8 op_code_1,
	  min::uns8 op_code_2,
	  bool is_infix,
	  min::uns32 base_flags );

// Ditto but make func a VALUE_OPERATOR and by default
// make it infix.
//
// For infix operators, if two MEX op_codes are given
// (e.g., for + and -), the first (_1) is for the
// op_name, and the second (_2) is for the alternate
// operator name (e.g., if op_name is + op_code_1 is ADD
// and op_code_2 is SUB for -).
//
inline ll::parser::primary::func push_value_op
	( min::gen op_name,
	  ll::parser::table::key_table symbol_table,
	  min::uns8 op_code_1,
	  min::uns8 op_code_2 = 0,
	  bool is_infix = true )
{
    return ll::parser::primary::push_op
    	( op_name, symbol_table,
	  op_code_1, op_code_2, is_infix,
	  ll::parser::primary::VALUE_OPERATOR );
}

// Ditto but make LOGICAL_OPERATOR instead of
// VALUE_OPERATOR.
//
// For infix operators the first op_code (_1) is the 
// logical_op_code and the second op_code is unused.
// For JMP postfix operators, the first op code (_1) is
// JMP and the second is the JMP... op code.
//
inline ll::parser::primary::func push_logical_op
	( min::gen op_name,
	  ll::parser::table::key_table symbol_table,
	  min::uns8 op_code_1,
	  min::uns8 op_code_2 = 0,
	  bool is_infix = true )
{
    return ll::parser::primary::push_op
    	( op_name, symbol_table,
	  op_code_1, op_code_2, is_infix,
	  ll::parser::primary::LOGICAL_OPERATOR );
}

// Push into a symbol_table a top level BUILTIN_FUNCTION
// func describing a function operator with given
// func_name, MEX instruction op_code, and number of
// arguments.
//
// Level and depth are set to 0, position is set to
// PAR::top_level_position, and selectors are set to
// PAR::ALL_SELECTORS.
//
ll::parser::primary::func push_builtin_func
	( min::gen func_name,
	  ll::parser::table::key_table symbol_table,
	  min::uns8 op_code,
	  min::uns32 number_arguments = 1,
	  min::uns32 base_flags =
	      ll::parser::primary::BUILTIN_FUNCTION );

// Ditto but make LOGICAL_OPERATOR instead of
// BUILTIN_FUNCTION.
//
inline ll::parser::primary::func push_logical_func
	( min::gen func_name,
	  ll::parser::table::key_table symbol_table,
	  min::uns8 op_code,
	  min::uns32 number_arguments = 1 )
{
    return ll::parser::primary::push_builtin_func
        ( func_name, symbol_table, op_code,
	  number_arguments,
	  ll::parser::primary::LOGICAL_OPERATOR );
}

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

// Push a new argument list description into the
// arg_lists vector of a func.
//
inline void push_arg_list
    ( min::gen term_name,
      min::uns32 number_of_args,
      min::uns32 first,
      bool is_square,
      ll::parser::primary::func func )
{
    ll::parser::primary::arg_list_struct arg_list =
        { term_name, number_of_args, first, is_square };
    min::push(func->arg_lists) = arg_list;
    min::unprotected::acc_write_update
        ( func->arg_lists, term_name );
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
// label.  Selectors are set to PAR::ALL_SELECTORS.
// Position, first_arg_list, number_arg_lists, is_bool
// must be set later.
//
ll::parser::primary::func_term create_func_term
	( min::gen func_term_label );

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

    const min::gen modifying_ops;
        // min::get ( modifying_ops, op ) is the mex op
	// code (e.g., mex::DIV ) of the instruction
	// that combines the top two stack elements to
	// implement a modifying assignment (e.g., /=).
	// The topmost stack element is the rightside
	// of the modifying assignment, and the next
	// to topmost stack element is the left side
	// (the value of the variable being modified).

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
MIN_REF ( min::gen, modifying_ops,
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
// A quoted string vp element is treated as a string vp
// element.
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
    ( min::obj_vec_ptr & vp, min::uns32 & i )
{
    return scan_name ( vp, i,
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
    ( min::obj_vec_ptr & vp, min::uns32 & i )
{
    return scan_name ( vp, i,
                       func_term_initial_types,
		       func_term_following_types,
		       func_term_outside_quotes_types,
		       func_term_inside_quotes_types );
}

// Scan function label.  vp[i] should be the beginning
// of a function prototype that may include () and []
// argument lists before the first function term name.
// The scan stops at the end of vp or at the first
// element after initial argument lists that cannot be
// part of a function term name.
//
// Specifically, a (...) at the beginning of the scan
// produces the MIN string ";;P" in the label, and a
// [...] produces a ";;S".  These are followed by the
// first function term name in the prototype, if there
// is one.
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

extern min::locatable_gen func_default_op;
    // Operator separating variable name from default
    // value.  Defaults to `?='.
extern min::locatable_gen func_bool_values;
    // MIN label listing boolean values.  The first
    // is the false value and the second is the
    // true value.   Defaults to [< FALSE TRUE >].
extern min::locatable_gen func_negators;
    // MIN label listing function call negators.
    // Defaults to [< no NO not NOT >].
extern min::uns32 func_term_table_size;
    // Size of func term_table.  Defaults to 16.

// Scan function prototype and store results in a func
// which is returned.  Specifically vp is scanned
// beginning with vp[i] and i is incremented during the
// scan.  The scan ends when vp ends or when the next
// vp element after an argument list is neither an
// argument list of a function term name component.
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
// list containing a single argument of the above form
// that has its default value equal to one of the
// elements of bool_values (which is a MIN label of zero
// or elements).
//
// It is assumed that vp contains a function prototype
// ending as indicated above.  If a defective prototype
// is found, parse_error is called one or more times,
// and NULL_STUB is returned (i is then undefined).

ll::parser::primary::func scan_func_prototype
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      min::phrase_position_vec ppvec, // of vp
      ll::parser::parser parser,
      ll::parser::table::flags selectors,
      min::uns16 level,
      min::uns16 depth,
      min::uns32 flags,
      min::uns32 location,
      min::gen module,
      min::gen default_op = func_default_op,
      min::gen bool_values = func_bool_values,
      min::uns32 term_table_size =
          func_term_table_size );


// Scan a primary expression and return the symbol_table
// entry found, either a var if a variable is found or a
// func if a function call is found.
//
// Specifically vp is scanned beginning with vp[i] and i
// is incremented during the scan.  For variables, the
// scan fails if the expression scanned is not a
// variable name followed by [] bracketed expressions.
// For function calls, the scan fails if the the call
// prototype match fails.  In either case i must scan to
// the end of vp, else the scan fails.
//
// True is returned if a primary_table entry was found,
// and false otherwise.
//
// The first call to this function for a particular vp
// and i should have key_prefix = NULL_STUB.  If after
// returning, the result is unsatisfactory because the
// next vp[i] is not a suitable subsequent vector
// element, the result can be rejected and the next
// primary_table entry can be found by re-calling this
// function with i, root, and key_prefix left as they
// were set by the last call.  This function can be
// re-called in this manner until the result is
// satisfactory or the function returns false.
//
// The argument_vector is always allocated if it is
// initially NULL_STUB.  For variables, the [] bracketed
// expressions following the variable name are returned
// in order in argument_vector.
//
// For functions, the call argument expressions are
// returned in the argument_vector, in left to right
// prototype order.  This vector will NOT have any
// elements with value NONE: all arguments must be
// provided or defaulted.  A prototype that would
// produce an arguments vector with a NONE element
// is automatically rejected just as if it had the
// wrong selectors.
//
// This function treats quoted string elements of vp
// as if they were MIN strings.  Should they not be
// legal in a variable or function term name, they will
// not be found in the symbol table.
//
// Quoted_i is set to the smallest i value for which
// vp[i] is a quoted string, or to the size of vp if
// there is no quoted string.  It can be used, for
// example, to reject variable names containing a quoted
// string.  Like i, it must be left untouched when
// re-calling this function.
//
// If print_rejections is true, rejection of a function
// prototype because of argument structure causes a
// REJECT: ... message to be printed.
//
// In addition to standard function calls, this function
// supports `operator calls'.  An operator call is an
// expression containing only one variety of operator
// outside bracketed subexpressions.  An operator
// variety is a set of operators with the same prece-
// dence, most commonly a set containing a single
// operator such as { * }, but also including some
// varieties that contain multiple operators, such as
// { +, - } and { <, >, <=, >=, ==, != }.
//
// When an operator call is scanned, a func is returned
// as soon as the first function-term (first operator)
// in the expression is recognized, and the rest of the
// expression is NOT scanned.  To qualify as an operator
// call, the first operator in the expression must NOT
// be quoted and must have the VALUE_OPERATOR or
// LOGICAL_OPERATOR flag.  Only argument expressions
// before the first function term (first operator) are
// returned in the argument_vector.
//
// Parser reformatters must be used to ensure that
// operator calls in fact have only operators of the
// same variety outside bracketed subexpressions.
// Alternatively, parser reformatters can be used to
// reformat operator calls so that they can be processed
// as ordinary calls: e.g., reformat x + y + z to
// {| x + y |} + z.  In this case there will will be
// no operator calls and the VALUE_OPERATOR and
// LOGICAL_OPERATOR flags will be unused.
//
typedef min::packed_vec_insptr<min::gen>
    argument_vector;
bool scan_primary
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      min::phrase_position_vec ppvec, // of vp
      ll::parser::parser parser,
      ll::parser::table::flags selectors,
      ll::parser::table::key_prefix & key_prefix,
      ll::parser::table::root & root,
      min::uns32 & quoted_i,
      min::ref<argument_vector> argument_vector,
      ll::parser::table::key_table symbol_table,
      bool print_rejections = false,
      min::gen bool_values = func_bool_values,
      min::gen negators = func_negators );

} } }

# endif // LL_PARSER_PRIMARY_H
