// Layered Languages Standard Lexical Analyzer
//
// File:	ll_lexeme_standard.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Dec  8 23:51:59 EST 2021
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

        // Index  Name/Function
	//
	// 0	  START
	//          Start of file.
	//
	// 1	  DEFAULT
	//          Any part of line after indent
	//          lexeme.
	//
	// 2	  DATA-CHECK
	//	    Ditto if the special data_t lexeme
	//	    and raw_data_t lexemes are to be
	//	    recognized at the current location,
	//	    after which control passes to
	//	    DEFAULT.
	//
	// 3	  TABLE-CHECK
	//	    Ditto if the special table_t lexeme
	//	    is to be recognized at the current
	//	    location, after which control passes
	//	    to DEFAULT.
	//
	// 4	  PARAGRAPH-CHECK
	//	    Ditto if the special data_t, raw_
	//	    data_t, and table_t lexemes are to
	//	    be recognized at the current
	//	    location, after which control passes
	//	    to DEFAULT.
	//
	// 5	  ROW-CHECK
	//	    Ditto if the special row_t lexeme is
	//	    to be recognized at the current
	//	    location, after which control passes
	//	    to DEFAULT.
	//
    const uns32 MAX_MASTER			= 5;
    extern const char * const master_names
    				[MAX_MASTER+1];

    // Lexeme Types

    // Note: Combining characters can be at any location
    // within a middle lexeme: there is no check to see
    // if they are at the beginning or in other illegal
    // positions.  They cannot be in separators or other
    // non-middle lexemes.

    const uns32 word_t				= 1;
        // Middle lexeme containing a letter before any
	// digit, but not a numeric_word_t.
    const uns32 numeric_word_t			= 2;
        // One of:
	//     sign? NaN
	//     sign? Inf
    const uns32 natural_t			= 3;
        // Sequence of digits 0..9 not beginning with
	// 0 unless the sequence contains only 1 digit.
    const uns32 number_t			= 4;
        // One of:
	//   sign? integer fraction? exponent?
	//   sign? fraction exponent?
	// where
	//   integer :::= digit+
	//   fraction :::= . digit+ 
	//   exponent :::= {e|E} sign? digit+
	// (i.e., all floats allowed by C/C++ except
	// `digit+ .'.)
	//
	// but not a natural_t.
    const uns32 numeric_t			= 5;
        // Middle lexeme containing a digit before
	// any letter, and not natural_t or number_t.
    const uns32 mark_t				= 6;
        // Middle lexeme not containing a letter or
	// a digit.

    const uns32 middle_mask =
          ( 1 << word_t )
	+ ( 1 << numeric_t ) + ( 1 << mark_t );
	// Middle lexemes that can be parts of
	// broken middle lexemes.  natural_t
	// cannot be a broken part.

    const uns32 quoted_string_t			= 7;

    const uns32 convert_mask =
        ( 1 << quoted_string_t );
	// Lexemes standardly converted to expressions
	// of the form {" ... "}.

    const uns32 separator_t			= 8;
        // Includes leading and trailing separators.

    const uns32 symbol_mask =
          ( 1 << word_t ) + ( 1 << numeric_t )
	+ ( 1 << natural_t ) + ( 1 << number_t )
	+ ( 1 << mark_t ) + ( 1 << separator_t );
	// Lexemes that are `symbols' for parser
	// symbol tables.

    const uns32 comment_t			= 9;
        // `//' in lexeme beginning position followed
	// by graphic and horizontal space characters.
    const uns32 horizontal_space_t		= 10;
        // Sequence of horizontal space chracters
	// (characters in category Zs characters and
	// horizontal tabs).  May not be empty.  Also
	// the lexeme must not be in the following
	// category.
    const uns32 indent_t			= 11;
	// Ditto, but just before the first graphic
	// character on a line that contains a graphic
	// character.  There is always one of these
	// lexemes on such a line.  Used to communicate
	// the indentation of the line and to allow
	// the lexeme table to be switched just before
	// the first graphic character on the line.
	// Also may be empty.
    const uns32 line_break_t			= 12;
        // Sequence of carriage returns, line feeds,
	// vertical tabs, and form feeds containing
	// at least one line feed.
    const uns32 start_of_file_t			= 13;
        // Virtual 0-length lexeme at start of file.
    const uns32 end_of_file_t			= 14;
        // Virtual 0-length lexeme at end of file.

    // Special lexemes:
    //
    const uns32 data_t				= 15;
        // @ at begining of line.
    const uns32 raw_data_t			= 16;
        // !@ at begining of line.
    const uns32 table_t				= 17;
        // ----- or ===== at beginning of line.
    const uns32 row_t				= 18;
        // |, -----, or ===== at beginning of line.

    // Erroneous Lexeme Types:
    //
    const uns32 premature_end_of_string_t	= 19;
        // A line break or end of file was encountered
	// in a quoted string.  The quoted string is
	// ended as if by ", and this 0-length lexeme
	// is output just before the line break or
	// end of file.
    const uns32 premature_end_of_file_t		= 20;
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
    const uns32 misplaced_vertical_t		= 21;
        // Sequence of carriage returns, vertical tabs,
	// and form feeds that does not abutt a line
	// feed.
    const uns32 illegal_control_t		= 22;
        // Sequence of control characters that are not
	// legal in any lexeme.
    const uns32 unrecognized_character_t	= 23;
        // Sequence of unrecognized characters
	// (characters with no recognized general
	// category) that are not legal in any lexeme.

    // Erroneous Atom Types:
    //
    const uns32 unrecognized_escape_t		= 24;
        // Any unrecognized <...> sequence in a quoted
	// string where ... is all upper case letters
	// and digits.  Translated to <UUC>.
    const uns32 misplaced_horizontal_t		= 25;
        // A sequence of horizontal characters other
	// than single space in a quoted string.
	// Translated to itself.  Such a sequence is
	// legal and not erroneous if it is part of a
	// horizontal space or comment lexeme.

    const unsigned MAX_TYPE = 25;

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
	const char * p = ~ min::begin_ptr_of ( sp );
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
