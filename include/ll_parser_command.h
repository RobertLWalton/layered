// Layered Languages Parser Commands
//
// File:	ll_parser_command.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Dec  9 23:31:15 EST 2016
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

// If vp[i] is a ()-bracketed subexpression, treat it
// as an argument list, store the specified set of
// arguments in the arg_vec vector, increment i, and
// return min::SUCCESS().  If it is not a ()-bracketed
// subexpression, just return min::FAILURE().  If it is
// a ()-bracketed subexpression with an error, print
// error messages to parser->printer, increment i, and
// return min::ERROR().
//
// An argument may be a simple name or a quoted name,
// and arguments are separated by commas.  It is assumed
// that the ()-bracketed subexpression does NOT have a
// separator and any commas are simply elements of the
// subexpression.
//
// If min::FAILURE() is not returned, arg_vec is created
// if it does not exist, emptied if it does exists, and
// then any successfully scanned arguments are pushed
// into it.
//
min::gen scan_args
	( min::obj_vec_ptr & vp, min::uns32 & i,
          min::ref< min::packed_vec_ptr<min::gen> >
	      arg_vec,
	  ll::parser::parser parser );

// If vp[i] is a []-bracketed subexpression, treat it
// as a flag list, store the specified set of flags in
// the flags argument, increment i, and return
// min::SUCCESS().  If it is not a []-bracketed
// subexpression, just return min::FAILURE().
// If it is a []-bracketed subexpression with an error,
// print error messages to parser->printer, increment i,
// and return min::ERROR().
//
// It is an error if any flag is not an allowed_flags.
// It is NOT an error if a flag group contains a flag
// not in allowed_flags.  However, only bits for flags
// in allowed_flags are set in flags.
//
// It is assumed that the []-bracketed subexpression
// does NOT have a separator and any comma separators
// are simply elements of the subexpression.
//
min::gen scan_flags
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  ll::parser::table::flags & flags,
	  ll::parser::table::flags allowed_flags,
	  ll::parser::table::name_table name_table,
	  ll::parser::table::key_table group_name_table,
	  ll::parser::parser parser );

// Ditto but scan a flag modifier list into new_flags
// instead of a flag list into flags.  If allow_flag_
// list is true, accept either a flag modifier list or a
// flag list.  A flag list is encoded by setting
// or_flags = flags, not_flags = ~flags & allowed_flags,
// and xor_flags = 0).
//
// Only bits in allowed_flags are set in or_flags,
// not_flags, and xor_flags.
//
min::gen scan_new_flags
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  ll::parser::table::new_flags & new_flags,
	  ll::parser::table::flags allowed_flags,
	  ll::parser::table::name_table name_table,
	  ll::parser::table::key_table group_name_table,
	  ll::parser::parser parser,
	  bool allow_flag_list = false );

// Print flag modifier list.  If allow_flag_list is
// true and new_flags is a flag list (see below),
// then a flag list is printed instead.  The list is
// surrounded by [ ] brackets and `,' is the separator.
// If a modifier list is being printed, the operator
// characters `+', `-', and `^' are printed before the
// flag name.  Illegal modifier lists result in more
// than one operator for the same flag name (e.g.,
// `+^').
//
// Only the subset of the flags sets (or/not/xor_flags)
// selected by allowed_flags is considered.  The test
// for a flag list is:
//
//        or_flags & allowed_flags
//     == ( ~ not_flags ) & allowed_flags
// and
//     xor_flags & allowed_flags == 0,
//
void print_new_flags
	( const ll::parser::table::new_flags
	    & new_flags,
	  ll::parser::table::flags allowed_flags,
	  ll::parser::table::name_table name_table,
	  ll::parser::parser parser,
	  bool allow_flag_list = false );

// Print flag list.  Flags with min::MISSING() name and
// flags not in allowed_flags are ignored.
//
void print_flags
	( ll::parser::table::flags flags,
	  ll::parser::table::flags allowed_flags,
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
