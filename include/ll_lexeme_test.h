// Layers Language Lexical Analyzer Testing
//
// File:	ll_lexeme_test.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Dec 30 00:13:06 EST 2010
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
    // The LEX::default_scanner is used and must
    // have its program, max_type, and type_name
    // members preset.
    //
    void basic_test_input ( std::istream & in );

    // Print the given input lines using UTF8GRAPHIC
    // print mode, scan each line, and print under each
    // line another line containing the type codes of
    // the lexemes in the line.  Under a character in a
    // lexeme or erroneous atom of type t, print the
    // code type_code[t].  If two codes, one for a
    // lexeme and one for an erroneous atom, are to be
    // printed in the same place, the erroneous atom
    // code take precedence.
    //
    // The LEX::default_scanner is used and must have
    // its program preset.
    //
    void test_input ( std::istream & in,
                      const char * type_code );

} }

# endif // LL_LEXEME_TEST_H

