// Layered Languages Standard Lexical Analyzer
//
// File:	ll_lexeme_standard.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Sep  4 04:01:51 EDT 2015
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

    // Note: Combining characters can be at any location
    // within a middle lexeme: there is no check to see
    // if they are at the beginning or in other illegal
    // positions.  They cannot be in separators or other
    // non-middle lexemes.

    const uns32 word_t				= 1;
        // Middle lexeme containing a letter before any
	// digit.
    const uns32 natural_t			= 2;
        // Sequence of digits 0..9 not beginning with
	// 0 unless the sequence contains only 1 digit.
    const uns32 numeric_t			= 3;
        // Middle lexeme containing a digit before
	// any letter, and not a natural number.
    const uns32 mark_t				= 4;
        // Middle lexeme not containing a letter or
	// a digit.
    const uns32 quoted_string_t			= 5;
    const uns32 separator_t			= 6;
        // Includes leading and trailing separators.

    const uns32 comment_t			= 7;
        // `//' in lexeme beginning position followed
	// by graphic and horizontal space characters.
    const uns32 horizontal_space_t		= 8;
        // Sequence of horizontal space chracters
	// (characters in category Zs characters and
	// horizontal tabs).
    const uns32 line_break_t			= 9;
        // Sequence of carriage returns, line feeds,
	// vertical tabs, and form feeds containing
	// at least one line feed.
    const uns32 end_of_file_t			= 10;
        // Virtual 0-length lexeme at end of file.

    // Erroneous Lexeme Types:
    //
    const uns32 premature_end_of_string_t	= 11;
        // A line break or end of file was encountered
	// in a quoted string.  The quoted string is
	// ended as if by ", and this 0-length lexeme
	// is output just before the line break or
	// end of file.
    const uns32 premature_end_of_file_t		= 12;
        // End of file not following a line break.
	// Includes case of empty input.  This 0-length
	// lexeme is output just before the end of file
	// lexeme.

    // Erroneous Lexeme AND erroneous Atom Types:
    //
    //     Sequences of various types of illegal charac-
    //     ters.  If appearing outside comments and
    //     quoted strings, these act like horizontal
    //     space lexemes, and are themselves lexemes.
    //     If appearing inside comments or quoted
    //     strings, these act as erroneous atoms, but
    //     are translated to themselves, so a program
    //     that does not output error messages for these
    //     as erroneous atoms in effect allows them
    //     in comments and quoted strings.
    //
    const uns32 misplaced_vertical_t		= 13;
        // Sequence of carriage returns, vertical tabs,
	// and form feeds that does not abutt a line
	// feed.
    const uns32 illegal_control_t		= 14;
        // Sequence of control characters that are not
	// legal in any lexeme.
    const uns32 unrecognized_character_t	= 15;
        // Sequence of unrecognized characters
	// (characters with no recognized general
	// category) that are not legal in any lexeme.

    // Erroneous Atom Types:
    //
    const uns32 unrecognized_escape_t		= 16;
        // Any unrecognized <...> sequence in a quoted
	// string where ... is all upper case letters
	// and digits.  Translated to <UUC>.
    const uns32 misplaced_horizontal_t		= 17;
        // A sequence of horizontal characters other
	// than single space in a quoted string.
	// Translated to itself.  Such a sequence is
	// legal and not erroneous if it is part of a
	// horizontal space or comment lexeme.

    const unsigned MAX_TYPE = 17;

    extern const char * const type_names[MAX_TYPE+1];

    extern const char * type_codes;
        // Type codes for ll::lexeme::test_input.

    // Given a min::gen string that encodes a standard
    // lexeme, returns the type of the lexeme.  Here
    // only words, naturals, numerics, marks, and
    // separators can be encoded as strings, and for
    // the purposes of this function, quoted strings,
    // comments, horizontal space, line breaks, and
    // end of files cannot be encoded as min::gen
    // strings.
    //
    // Given a min::gen value that is not a string,
    // or is a string that cannot be a word, natural,
    // numeric, mark, or separator, returns 0.
    //
    inline uns32 lexical_type_of ( min::gen g )
    {
        min::str_ptr sp ( g );
	if ( ! sp ) return 0;

	min::unsptr len = min::strlen ( sp );
	min::Uchar buffer[len];
	min::Uchar * bp = buffer;
	const char * p = ! min::begin_ptr_of ( sp );
	min::unsptr n =
	    min::utf8_to_unicode
	        ( bp, bp + len, p, p + len );
	min::uns32 str_class =
	    min::standard_str_classifier
	        ( min::standard_char_flags,
		  min::support_all_support_control,
		  n,
		  min::new_ptr<const min::Uchar>
		      ( buffer ) );

	if ( str_class & min::NEEDS_QUOTES )
	    return 0;
	else if ( str_class & min::IS_LETTER )
	    return word_t;
	else if ( str_class & min::IS_NATURAL )
	    return natural_t;
	    // Needs to be BEFORE numeric test.
	else if ( str_class & min::IS_DIGIT )
	    return numeric_t;
	else if ( str_class & (   min::IS_LEADING
	                        | min::IS_TRAILING
	                        | min::IS_SEPARATOR ) )
	    return separator_t;
	    // Needs to be BEFORE mark test.
	else if ( str_class & min::IS_MARK )
	    return mark_t;
	else
	    return 0;
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
