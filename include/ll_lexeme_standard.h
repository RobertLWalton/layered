// Layers Language Standard Lexical Analyzer
//
// File:	ll_lexeme_standard.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Jun 24 04:11:33 EDT 2011
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

# ifndef LL_LEXEME_STANDARD_H
# define LL_LEXEME_STANDARD_H

# include <ll_lexeme.h>


// Lexical Types
// ------- -----

namespace ll { namespace lexeme { namespace standard {

    using ll::lexeme::uns32;

    // Lexeme Types

    const uns32 word_t				= 1;
    const uns32 natural_number_t		= 2;
    const uns32 number_t			= 3;
    const uns32 mark_t				= 4;
    const uns32 quoted_string_t			= 5;
    const uns32 separator_t			= 6;

    const uns32 comment_t			= 7;
    const uns32 horizontal_space_t		= 8;
    const uns32 line_break_t			= 9;
    const uns32 end_of_file_t			= 10;

    // Erroneous Lexeme Types:
    //
    //    These can separated other lexemes in the same
    //    way as horizontal space can, and if these and
    //    horizontal space lexemes are deleted by post-
    //    processing, they will be semantically equiva-
    //    lent to horizontal space.
    //
    const uns32 premature_end_of_line_t		= 11;
        // A line feed or <CR><LF> was encountered in a
	// quoted string.  The quoted string is ended as
	// if by ", and this 0-length lexeme is output
	// before the line break.
    const uns32 premature_end_of_file_t		= 12;
        // A comment or quoted string ended with an end
	// of file.  The quoted string is ended as if by
	// ".  This 0-length lexeme is output before the
	// end of file lexeme.
    const uns32 misplaced_char_t		= 13;
        // A character in a lexeme beginning position
	// that cannot begin a lexeme.
    const uns32 misplaced_space_t		= 14;
        // A sequence of carriage returns, form feed,
	// and vertical tabs that does not contain
	// a line feed.
    const uns32 ascii_escape_seq_t		= 15;
        // A <0...> escape sequence that represents any
	// ASCII character and is outside a quoted
	// string.
    const uns32 non_letter_escape_seq_t		= 16;
        // A <0...> escape sequence in a lexeme begin-
	// ning position that does not represent a
	// letter.

    // Erroneous Atom Types:
    //
    //    These are translated to "" and ignored, so
    //    removal will not change the lexemes produced.
    //
    const uns32 misplaced_control_char_t	= 17;
        // Any control character in a quoted string.
    const uns32 unrecognized_escape_seq_t	= 18;
        // Any unrecognized <...> sequence in a quoted
	// string where ... is at most 12 characters
	// that are all upper case letters, digits,
	// or `-'s.

    const unsigned MAX_TYPE = 18;

    extern const char * const type_name[MAX_TYPE+1];

    extern const char * type_code;
        // Type codes for ll:lexeme::test_input.

} } }


// Program Construction
// ------- ------------

namespace ll { namespace lexeme { namespace standard {

    extern min::locatable_var<ll::lexeme::program>
        default_program;

    // If ll::lexeme::standard::default_program is not
    // NULL_STUB, this function does nothing.  Otherwise
    // it creates the standard program in both
    //
    //		ll::lexeme::default_program
    // and	ll::lexeme::standard::default_program
    //
    void init_standard_program ( void );

} } }

# endif // LL_LEXEME_STANDARD_H
