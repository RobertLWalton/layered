// Layers Language Parser Commands
//
// File:	ll_parser_command.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Nov 23 02:53:50 EST 2013
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_COMMAND_H
# define LL_PARSER_COMMAND_H

# include <ll_parser.h>

namespace ll { namespace parser { namespace command {

// Prints
//
//   ERROR: in <file:line-#> expected <what> <where>:
//     <file-lines-with-pp underscored>
//
// and returns min::ERROR().
//
min::gen expected_error
	( min::printer printer,
	  min::file file,
	  min::phrase_position pp,
	  const char * what,
	  const char * where = "after" );

// Prints `extraneous stuff after <file,pp>' error and
// returns min::ERROR().
//
min::gen extra_stuff_after_error
	( min::printer printer,
	  min::file file,
	  min::phrase_position pp );


// Prints `<what_is> should be <should_be>' warning
// where what_is is at <file,pp>.
//
void misspell_warning
	( min::printer printer,
	  min::file file,
	  min::phrase_position pp,
	  min::gen what_is,
	  min::gen should_be );

// Given an object vector pointer vp pointing at an
// expression, and an index i of an element in the
// object attribute vector, then increment as long
// as the i+1'st element of the object vector has a
// lexical type t such that the bit 1<<t is on in
// accepted_types and this element does not equal
// end_value.  Then if i has been incremented at least
// once, make and return a label from the elements
// scanned over.  If there is only 1 element, return
// just that element.  If there is more than one, return
// the MIN label containing the elements.  If there are
// no elements, return min::MISSING().
//
min::gen scan_simple_label
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  min::uns64 accepted_types,
	  min::gen end_value = min::MISSING() );

// If vp[i] is a []-bracketed subexpression, treat it
// as a flag list, store the specified set of flags in
// the flags argument, increment i, and return
// min::SUCCESS().  If it is not a []-bracketed
// subexpression, just return min::FAILURE().
// If it is a []-bracketed subexpression with an error,
// print error messages to parser->printer, increment i,
// and return min::ERROR().
//
min::gen scan_flags
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  ll::parser::table::flags & flags,
	  ll::parser::table::name_table name_table,
	  ll::parser::parser parser );

// Ditto but scan a flag modifier list into new_flags
// instead of a flag list into flags.  If allow_flag_
// list is true, accept either a flag modifier list or a
// flag list (encode the latter by setting or_flags =
// flags, not_flags = ~ flags, xor_flags = 0).
//
min::gen scan_new_flags
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  ll::parser::table::new_flags & new_flags,
	  ll::parser::table::name_table name_table,
	  ll::parser::parser parser,
	  bool allow_flag_list = false );

// Print flag modifier list.  If allow_flag_list is
// true and new_flags is a flag list (or_flags ==
// ~ not_flags and xor_flags == 0) then a flag list
// is printed instead.  The list is surrounded by [ ]
// brackets and `,' is the separator.  If a modifier
// list is being printed, the operator characters
// `+', `-', and `^' are printed before the flag name.
// Illegal modifier lists result in more than one
// operator for the same flag name (e.g., `+^').
//
void print_new_flags
	( const ll::parser::table::new_flags
	    & new_flags,
	  ll::parser::table::name_table name_table,
	  ll::parser::parser parser,
	  bool allow_flag_list = false );

// Print flag list.
//
void print_flags
	( ll::parser::table::flags flags,
	  ll::parser::table::name_table name_table,
	  ll::parser::parser parser );

// Given a vector pointer vp to an expression, test if
// the expression is a parser command.  Do nothing but
// return min::FAILURE() if no.  If yes, process the
// command, and if there is no error return ll::parser::
// command::PRINTED if the command was printed (as by
// the `parser print ...' commands) or min::SUCCESS()
// otherwise, but if there is an error, print an
// error message to parser->printer and return min::
// ERROR().  Note that only expressions that begin with
// `parser' can be parser commands.
//
extern min::locatable_gen PRINTED;
min::gen parser_execute_command
	( min::obj_vec_ptr & vp,
	  ll::parser::parser parser );

// Print the command stored in vp.  No min::eol or other
// control command is executed.
//
void print_command
	( min::obj_vec_ptr & vp,
	  ll::parser::parser parser );

} } }

# endif // LL_PARSER_COMMAND_H
