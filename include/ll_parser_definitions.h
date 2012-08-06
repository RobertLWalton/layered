// Layers Language Parser Definitions
//
// File:	ll_parser_definitions.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Aug  6 06:18:03 EDT 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_DEFINITIONS_H
# define LL_PARSER_DEFINITIONS_H

# include <ll_parser.h>

namespace ll { namespace parser {
    namespace definition {

// Prints `expected <what> after <file,pp>' error and
// returns min::ERROR().
//
min::gen expected_error
	( min::printer printer,
	  min::file file,
	  min::phrase_position pp,
	  const char * what );

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
// type t such that the bit 1<<t is on in accepted_
// types.  Then if i has been incremented at least
// once, make and return a label from the elements
// scanned over.  If there is only 1 element, return
// just that element.  If there is more than one,
// return the MIN label containing the elements.  If
// there are no elements, return min::MISSING().
//
min::gen scan_simple_label
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  min::uns64 accepted_types );

// If vp[i] is a []-bracketed subexpression, treat it
// as a selectors specification, store the specified
// set of selectors in the selectors argument, increment
// i, and return min::SUCCESS().  If it is not a []-
// bracketed subexpression, just return min::MISSING().
// If it is a []-bracketed subexpression, print error
// messages to parser->printer, do NOT increment i, and
// return min::ERROR().
//
min::gen scan_selectors
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  ll::parser::table::selectors & selectors,
	  ll::parser::parser parser );

// Ditto but scan new_selectors instead of selectors.
//
min::gen scan_new_selectors
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  ll::parser::table::new_selectors &
	      new_selectors,
	  ll::parser::parser parser );

// Given a vector pointer vp to an expression, test if
// the expression is a parser definition.  Do nothing
// but return min::FAILURE() if no.  If yes, process
// the definition, and if there is no error return
// min::SUCCESS(), but if there is an error, print an
// error message too parser->printer and return min::
// ERROR().  Note that only expressions that begin with
// `parser' can be parser definitions.
//
min::gen parser_execute_definition
	( min::obj_vec_ptr & vp,
	  ll::parser::parser parser );

// Ditto but restricted to parser bracket and
// indentation mark definitions.
//
min::gen parser_execute_bracket_definition
	( min::obj_vec_ptr & vp,
	  min::phrase_position_vec ppvec,
	  ll::parser::parser parser );

} } }

# endif // LL_PARSER_DEFINITIONS_H
