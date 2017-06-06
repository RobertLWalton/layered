// Layered Languages Lexical Analyzer Testing
//
// File:	ll_lexeme_test.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jun  6 15:12:05 EDT 2017
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

// Testers
// -------

namespace ll { namespace lexeme {

    // Scan the given input and print each lexeme
    // using plexeme.  Erroneous atoms are printed
    // using perroneous_atom, and SCAN_ERROR's are
    // printed and terminate the test function.
    //
    // The ll::lexeme::default_scanner is used and
    // must have all its parameters preset except for
    // erroneous_atom, which are set by this function
    // to LEX::default_erroneous_atom.
    //
    // This function terminates after printing a lexeme
    // of type end_of_file_t.
    //
    // If indent_t != 0, a current lexical master is
    // maintained, and if it is not MISSING_MASTER, the
    // lexical master of the scan is reset to this value
    // after any token of indent_t type.  Initially the
    // current lexical master is MISSING_MASTER.  Any
    // token whose string is `<master-name>' for a
    // valid lexical master name resets the current
    // lexical master.  Any token whose string is
    // `<MISSING_MASTER>' resets the current lexical
    // master to MISSING_MASTER.
    //
    void basic_test_input
	( uns32 end_of_file_t, uns32 indent_t = 0 );

    // Print the given input lines using the scanner
    // print mode, scan each line, and print under each
    // line another line containing the type codes of
    // the lexemes in the line.  Under a character in a
    // lexeme or erroneous atom of type t, print the
    // code type_codes[t].  If two codes, one for a
    // lexeme and one for an erroneous atom, are to be
    // printed in the same place, the erroneous atom
    // code take precedence.
    //
    // The ll::lexeme::default_scanner is used and
    // must have all its parameters preset except for
    // erroneous_atom, which are set by this function.
    // It is assumed that input_file is used to read
    // input and will spool recently read lines for
    // printing.
    //
    // The type_codes are taken from the lexical program
    // of the scanner.  If there is no type code for
    // a type t, a single space is printed.
    //
    // This function terminates after printing a lexeme
    // of type end_of_file_t.
    //
    void test_input ( uns32 end_of_file_t );

} }

# endif // LL_LEXEME_TEST_H
