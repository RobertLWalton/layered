// Layers Language Lexical Analyzer Testing
//
// File:	ll_lexeme_test.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jan  1 08:27:48 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Testers

// Usage and Setup
// ----- --- -----

# ifndef LL_LEXEME_TEST_H
# define LL_LEXEME_TEST_H

# include <min.h>
# include <iostream>
# include <cassert>

// Testers
// -------

namespace ll { namespace lexeme {

    // Scan the given input and print each lexeme
    // using splexeme.  Erroneous atoms are printed
    // using sperroneous_atom, and SCAN_ERROR's are
    // printed and terminate the test function.
    //
    // The ll::lexeme::default_scanner is used and
    // must have its program, max_type, and type_name
    // members preset.
    //
    // The `in' stream and file_name are passed to
    // ll::lexeme::init_stream.  This function termina-
    // tes after printing a lexeme of type end_of_
    // file_t.
    //
    void basic_test_input
	    ( std::istream & in,
	      const char * file_name,
	      uns32 end_of_file_t );

    // Print the given input lines using the scanner
    // print mode, scan each line, and print under each
    // line another line containing the type codes of
    // the lexemes in the line.  Under a character in a
    // lexeme or erroneous atom of type t, print the
    // code type_code[t].  If two codes, one for a
    // lexeme and one for an erroneous atom, are to be
    // printed in the same place, the erroneous atom
    // code take precedence.
    //
    // The ll::lexeme::default_scanner is used and must
    // have its program preset.  The `in', file_name,
    // and end_of_file_t arguments are as for basic_
    // test_input above.
    //
    void test_input
	    ( const char * type_code,
	      std::istream & in,
	      const char * file_name,
	      uns32 end_of_file_t );

} }

# endif // LL_LEXEME_TEST_H
