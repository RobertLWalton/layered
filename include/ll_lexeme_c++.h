// Layers Language C++ Lexical Analyzer
//
// File:	ll_lexeme_c++.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jun 11 08:53:09 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Lexical Types
//	Program Construction

// Usage and Setup
// ----- --- -----

# ifndef LL_LEXEME_CPP_H
# define LL_LEXEME_CPP_H

# include <ll_lexeme.h>


// Lexical Types
// ------- -----

namespace ll { namespace lexeme { namespace cpp {

    using ll::lexeme::uns32;

    // Lexeme Types

    const uns32 whitespace_t			= 1;
        // Sequences of whitespace characters other than
	// newline that are outside character and string
	// literals.
    const uns32 comment_t			= 2;
        // Either a /* */ or // comment.
    const uns32 bracketed_header_name_t		= 3;
    const uns32 quoted_header_name_t		= 4;
    const uns32 operator_t			= 5;
        // Includes separators.
    const uns32 identifier_t			= 6;
    const uns32 dec_integer_t			= 7;
    const uns32 oct_integer_t			= 8;
    const uns32 hex_integer_t			= 9;
    const uns32 float_t				= 10;
    const uns32 pp_number_t			= 11;
        // A pp number that is not an integer or float.
    const uns32 char_literal_t			= 12;
    const uns32 u_char_literal_t		= 13;
    const uns32 U_char_literal_t		= 14;
    const uns32 L_char_literal_t		= 15;
    const uns32 string_literal_t		= 16;
    const uns32 u_string_literal_t		= 17;
    const uns32 U_string_literal_t		= 18;
    const uns32 L_string_literal_t		= 19;
    const uns32 newline_t			= 20;
    const uns32 end_of_file_t			= 21;
        // The parser should treat an end-of-file that
	// is not preceeded by a newline as if it
	// was proceeded by a newline.

    // Erroneous Atom Types
    //
    const uns32 premature_newline_t		= 22;
        // 0-length, emitted before newline that
	// terminates a character or string literal.
	// A ' or " is inserted at the end of the
	// literal.
    const uns32 premature_end_of_file_t		= 23;
        // 0-length, emitted before an end-of-file that
	// terminates a character or string literal.
	// A ' or " is inserted at the end of the
	// literal.
    const uns32 illegal_escape_t		= 24;
        // The sequence \C inside a character or string
	// literal where C represents a character such
	// that \C does not begin a legal escape
	// sequence.  Both \ and C are part of the
	// erroneous atom.  The erroneous atom is
	// translated to "" and ignored.
    const uns32 misplaced_char_t		= 25;
        // Backslash or a non-ascii, non-letter charac-
	// ter outside a character or string literal.
	// Note that ## cannot be used to move charac-
	// ters inside a character or string literal,
	// so once misplaced, always misplaced.  Also,
	// we do NOT allow ## to append a combining
	// character to another character.  These
	// erroneous atoms are translated as "" and
	// ignored.
    const uns32 ascii_universal_char_t		= 26;
        // A universal character that represents an
	// ASCII character outside a character or
	// string literal.  These erroneous atoms are
	// translated as "" and ignored.
    const uns32 ill_formed_universal_char_t	= 27;
        // A universal character with non-hexadecimal
	// digits.  These erroneous atoms are translated
	// as "" and ignored.

    const unsigned MAX_TYPE = 27;

    extern const char * const type_name[MAX_TYPE+1];

    extern const char * type_code;
        // Type codes for ll:lexeme::test_input.

} } }


// Program Construction
// ------- ------------

namespace ll { namespace lexeme { namespace cpp {

    extern min::locatable_var<ll::lexeme::program>
        default_program;

    // If ll::lexeme::cpp::default_program is not NULL_
    // STUB, this function does nothing.  Otherwise it
    // creates the c++ lexical program in both
    //
    //		ll::lexeme::default_program
    // and	ll::lexeme::cpp::default_program
    //
    void init_cpp_program ( void );

} } }

# endif // LL_LEXEME_CPP_H
