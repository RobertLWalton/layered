// Layered Languages Standard Lexical Analyzer
//
// File:	ll_lexeme_standard.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun May 24 07:02:40 EDT 2015
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

    // Erroneous Atom Types:
    //
    //    These are translated to "" and ignored, so
    //    removal will not change the lexemes produced.
    //
    const uns32 misplaced_control_char_t	= 15;
        // Any control character in a quoted string.
    const uns32 unrecognized_escape_seq_t	= 16;
        // Any unrecognized <...> sequence in a quoted
	// string where ... is all upper case letters
	// and digits.

    const unsigned MAX_TYPE = 16;

    extern const char * const type_names[MAX_TYPE+1];

    extern const char * type_codes;
        // Type codes for ll::lexeme::test_input.

    namespace internal {

	extern const uns8 * lexeme_type_table;

    }

    // Given a min::gen string that encodes a standard
    // lexeme, returns the type of the lexeme, with
    // exception that number_t is returned for both
    // natural numbers and other numbers.  Note that
    // only words, natural numbers, numbers, marks, and
    // separators can be encoded as strings.  In
    // particular quoted strings, comments, horizontal
    // space, line breaks, and end of files cannot be
    // encoded as min::gen strings (as far as this
    // function is concerned).
    //
    // Given a min:;gen value that is not a string,
    // returns zero.  In all other cases the value
    // returned is undefined.
    //
    inline uns32 lexical_type_of ( min::gen g )
    {
        union { uns64 b; uns8 s[8]; } v;
	v.b = min::strhead ( g );
	uns8 c = v.s[0];
	if ( c == '\'' && v.s[1] != 0 )
	    return word_t;
	else if (     c == '.'
	          && '0' <= v.s[1] && v.s[1] <= '9' )
	    return number_t;
	else
	    return internal::lexeme_type_table[c];
    }

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
