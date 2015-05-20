// Layered Languages C++ Lexical Analyzer
//
// File:	ll_lexeme_c++.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Jun 17 05:26:15 EDT 2011
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
    const uns32 decimal_integer_t		= 7;
        // A pp number with decimal integer form.
    const uns32 octal_integer_t			= 8;
        // A pp number with octal integer form.
    const uns32 hexadecimal_integer_t		= 9;
        // A pp number with hexadecimal integer form.
    const uns32 float_t				= 10;
        // A pp number with floating point form.
    const uns32 pp_number_t			= 11;
        // Other pp numbers.
    const uns32 char_literal_t			= 12;
    const uns32 u_char_literal_t		= 13;
    const uns32 U_char_literal_t		= 14;
    const uns32 L_char_literal_t		= 15;
    const uns32 string_literal_t		= 16;
    const uns32 u_string_literal_t		= 17;
    const uns32 U_string_literal_t		= 18;
    const uns32 L_string_literal_t		= 19;
    const uns32 newline_t			= 20;
        // A single newline charater by itself.
    const uns32 end_of_file_t			= 21;
        // The parser should treat an end-of-file that
	// is not preceeded by a newline as if it
	// was proceeded by a newline.

    // Error Lexeme Types
    //
    // These lexemes appear outside character literal
    // ''s and and quoted string literal ""s and are
    // treated syntactically like whitespace (if they
    // are replaced by a space the code will have the
    // same semantics).
    //
    // Note that ## cannot be used to move characters
    // inside a character or string literal '' or "".
    // Also, we do NOT allow ## to append a combining
    // character to another character.  So ## cannot be
    // used to create a legal pp token by concatenating
    // a legal pp token with a stray character that is
    // not part of a legal pp token.  Therefore we can
    // treat all stay characters as errors, instead of
    // making them into pp tokens.
    //
    const uns32 misplaced_char_t		= 22;
        // Backslash or a non-ascii, non-letter charac-
	// ter.
    const uns32 ascii_universal_char_t		= 23;
        // A universal character that represents an
	// ASCII character.
    const uns32 misplaced_universal_char_t	= 24;
        // A universal character that represents a
	// non-ASCII, non-letter character.
    const uns32 short_universal_char_t		= 25;
        // A universal character with too few hexa-
	// decimal digits.
    const uns32 premature_newline_t		= 26;
        // 0-length, emitted before newline that
	// terminates a character or string literal
	// or #include header.
    const uns32 premature_end_of_file_t		= 27;
        // 0-length, emitted before an end-of-file that
	// terminates a character or string literal or
	// a /* comment or an #include header.  A */ is
	// inserted at the end of a comment.


    // Erroneous atoms.  These are translated to "" and
    // are ignored as if they did not exist.  Removing
    // them will not change program semantics.
    //
    const uns32 ill_formed_escape_t		= 28;
        // The sequence \C inside a character or string
	// literal where C represents a character such
	// that \C does not begin a legal escape
	// sequence.  Both \ and C are part of the
	// erroneous atom.
    const uns32 too_long_hex_escape_t		= 29;
        // A \x or \X escape sequence with 9 hexadecimal
	// digits.  This would be allowed in standard
	// C++, but due to the limitations of our lexeme
	// translator, we must bound the length of any
	// escape sequence.
    // const uns32 short_universal_char_t	= 25;
        // A universal character with to few digits.
	// Has the same type code as error lexeme above.

    const unsigned MAX_TYPE = 29;

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
