// Layers Language Parser Definitions
//
// File:	ll_parser_definitions.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Feb 15 06:41:43 EST 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Parser Definition Functions

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_DEFINITIONS_H
# define LL_PARSER_DEFINITIONS_H

# include <ll_parser.h>

namespace ll { namespace parser {

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
min::gen make_simple_label
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  min::uns64 accepted_types );

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

} }

# endif // LL_PARSER_DEFINITIONS_H
