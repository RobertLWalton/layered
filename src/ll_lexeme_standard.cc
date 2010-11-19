// Layers Language Standard Lexical Analyzer
//
// File:	ll_lexeme_standard.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Nov 19 01:30:01 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Program Construction

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_ndl.h>
# include <ll_lexeme_standard.h>
using namespace ll::lexeme::ndl;
# define LEX ll::lexeme

// Program Construction
// ------- ------------

static const uns32 MAX_TYPE = 16;
static const char * type_name[MAX_TYPE+1] = {
    NULL,
    "WORD",
    "NATURAL_NUMBER",
    "NUMBER",
    "MARK",
    "QUOTED_STRING",
    "SEPARATOR",
    "COMMENT",
    "HORIZONTAL_SPACE",
    "LINE_BREAK",
    "END_OF_FILE",
    "BAD_END_OF_LINE",
    "BAD_END_OF_FILE",
    "UNRECOGNIZED_CHARACTER",
    "UNRECOGNIZED_ESCAPE_CHARACTER",
    "UNRECOGNIZED_ESCAPE_SEQUENCE",
    "NON_LETTER_ESCAPE_SEQUENCE" };

struct char_range { uns32 min, max; }
    non_ascii_letters[] =
{
 // { min,		max },
#include <ll_lexeme_non_ascii_letters.h>
    { 0,                0 }
};

static void add_non_ascii_letters ( void )
{
    for ( char_range *p = non_ascii_letters;
	  p->min || p->max != 0; ++ p )
	NDL::add_characters ( p->min, p->max );
}

void ll::lexeme::standard::create_standard_program
	( void )
{
    // Set up type names.
    //
    LEX::type_name = ::type_name;
    LEX::max_type = ::MAX_TYPE;

    /// begin standard lexical program;
    ///
    begin_program();

    uns32 main;
    NDL::new_table ( main, MASTER );
    uns32 word;
    NDL::new_table ( main, word_t );
    uns32 natural_number;
    NDL::new_table ( main, natural_number_t );
    uns32 number;
    NDL::new_table ( main, number_t );
    uns32 mark;
    NDL::new_table ( main, mark_t );
    uns32 quoted_string;
    NDL::new_table ( main, quoted_string_t );
    uns32 separator;
    NDL::new_table ( main, separator_t );
    uns32 comment;
    NDL::new_table ( main, comment_t );
    uns32 horizontal_space;
    NDL::new_table ( main, horizontal_space_t );
    uns32 line_break;
    NDL::new_table ( main, line_break_t );
    uns32 bad_end_of_line;
    NDL::new_table ( main, MASTER );
    uns32 bad_end_of_file;
    NDL::new_table ( main, MASTER );
    uns32 escaped_character;
    NDL::new_table ( main, TRANSLATION );
    uns32 bad_escape_sequence;
    NDL::new_table ( main, TRANSLATION );


    /// "<ascii-letter>" = "a-z" | "A-Z";
    //
    const char * ascii_letter_char =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    /// "<digit>" = "0-9";
    //
    const char * digit_char = "0123456789";

    /// "<oct-digit>" = "0-7";
    //
    const char * oct_digit_char = "01234567";

    /// "<hex-digit>" = "0-9" | "a-f" | "A-F";
    ///
    const char * hex_digit_char = "0123456789abcdefABCDEF";

    /// "<non-digit>" = ~ "<digit>";
    ///
    /// // The following list of language specific letters is taken
    /// // from Annex E of the Working Paper for Draft Proposed
    /// // International Standard for Information Systems—Programming
    /// // Language C++, 1996.
    ///
    /// "<latin-letter>" = "\00c0/-\00d6/" | "\00d8/-\00f6/"
    ///                  | "\00f8/-\01f5/" | "\01fa/-\0217/"
    ///                  | "\0250/-\02a8/" | "\01ea0/-\01ef9/";
    ///
    /// . . . . . letter character pattern definitions omitted . . . . .
    ///
    /// "<CJK-letter>" = "\0f900/-\0fa2d/" | . . .  // Details omitted
    ///
    /// // Context sensitive character classifications, e.g.,
    /// // classifying ' as a word character if it is followed by
    /// // a letter, cannot be included in character pattern
    /// // definitions, but are noted in comments.  They are
    /// // accounted for by separate lexeme table entries below.
    ///
    /// "<letter>" = "<ascii-letter>" | "<latin-letter>"
    ///            | . . . | "<CJK-letter>";   // Details omitted
    ///
    uns32 letter;
    NDL::begin_atom_pattern ( letter );
    	NDL::add_characters ( ascii_letter_char );
	add_non_ascii_letters();
    NDL::end_atom_pattern();

    /// "<non-letter>" = ~ "<letter>";
    ///
    /// "<mark-char>" = "+" | "-" | "*" | "~" | "@" | "#" | "$" | "%"
    ///               | "^" | "&" | "=" | "|" | "<" | ">" | "_" | "!"
    ///               | "?" | ":";
    ///               //
    ///               // Also . not followed by a digit
    ///               //      / not surrounded by digits
    ///
    const char * mark_char = "+-*~@#$%^&=|<>_!?:";

    /// "<non-slash-char>" = ~ "/";
    ///
    /// "<separator-char>" = "(" | ")" | "[" | "]" | "{" | "}" | ";" | "`";
    ///               //
    ///               // Also , not surrounded by digits
    ///               //      ' not followed by a letter
    ///
    const char * separator_char = "()[]{};`";

    /// "<line-break-char>" = "\lf/" | "\vt/" | "\ff/" | "\cr/";
    ///
    const char * line_break_char = "\n\v\f\r";

    /// "<non-line-break-char>" = ~ "<line-break-character>";
    ///
    /// "<horizontal-space-char>" = " " | "\ht/";
    ///
    const char * horizontal_space_char = " \t";

    /// begin main master table;
    ///
    NDL::begin_table ( main );

    ///    // Whitespace is separated out into 3 types of lexeme:
    ///    //
    ///    //   `horizontal space'  spaces and horizontal tabs
    ///    //   `line break'        linefeeds, carriage returns,
    ///    //                       form feeds, and vertical
    ///    //                       tabs
    ///    //   `comments'          `//' up to just before next
    ///    //                       line break
    ///    //
    ///    "//" call comment;
    //
	   NDL::begin_dispatch ( "/" );
	      NDL::begin_dispatch ( "/" );
		 NDL::call ( comment );
	      NDL::end_dispatch();

    	      // See "/" call mark;  below.
	      //
	      NDL::begin_dispatch ( OTHER );
		 NDL::keep ( 1 );
		 NDL::call ( mark );
	      NDL::end_dispatch();
	   NDL::end_dispatch();

    ///    "<horizontal-space-char>" call horizontal space;
    //
	   NDL::begin_dispatch ( horizontal_space_char );
	      NDL::call ( horizontal_space );
	   NDL::end_dispatch();

    ///    "<line-break-char>" call line break;
    ///
	   NDL::begin_dispatch ( line_break_char );
	      NDL::call ( line_break );
	   NDL::end_dispatch();

    ///    "<letter>" call word;
    //
	   NDL::begin_dispatch ( ascii_letter_char );
	      add_non_ascii_letters ();
	      NDL::call ( word );
	   NDL::end_dispatch();

    ///    "'<letter>" call word;
    //
	   NDL::begin_dispatch ( "'" );
	      NDL::begin_dispatch ( ascii_letter_char );
		 add_non_ascii_letters ();
		 NDL::call ( word );
	      NDL::end_dispatch();

    	      // See "'" output separator;  below.
	      //
	      NDL::begin_dispatch ( OTHER );
		 NDL::keep ( 1 );
		 NDL::output ( separator_t );
	      NDL::end_dispatch();
	   NDL::end_dispatch();

    ///   "\\/" translate escaped character
    ///          require "<letter>" call word
    ///          else translate bad escape sequence
    /// 	      output unrecognized escape sequence
    ///          else output unrecognized escape character;
    ///
    	   NDL::begin_dispatch ( "\\" );
		 NDL::translate ( escaped_character );
		 NDL::require ( letter );
		 NDL::call ( word );
	      NDL::ELSE();
	      	 NDL::translate ( bad_escape_sequence );
		 NDL::output ( unrecognized_escape_sequence_t );
	      NDL::ELSE();
	         NDL::output ( unrecognized_escape_character_t );
    	   NDL::end_dispatch();

    ///    "<mark-char>" call mark;
    //
    	   NDL::begin_dispatch ( mark_char );
	      NDL::call ( mark );
    	   NDL::end_dispatch();

    ///    "." call mark;  // ".<non-digit>"
    //     	See ".<digit>" keep 1 call number; below.
    ///    "/" call mark;  // "/<non-slash-char>"
    //     	See "//" call comment; above.
    ///
    ///    // We assume that the preceding text is not a digit
    ///    // or the current input is not / or , followed by a digit.
    ///    //
    ///    "<digit>" keep 0 call natural number;
    //
    	   NDL::begin_dispatch ( digit_char );
	      NDL::call ( natural_number );
    	   NDL::end_dispatch();

    ///    ".<digit>" keep 1 call number;
    ///
    	   NDL::begin_dispatch ( "." );
	      NDL::begin_dispatch ( digit_char );
		 NDL::keep ( 1 );
		 NDL::call ( number );
	      NDL::end_dispatch();

	      // See "." call mark;  above.
	      NDL::begin_dispatch ( OTHER );
		 NDL::keep ( 1 );
		 NDL::call ( mark );
	      NDL::end_dispatch();
    	   NDL::end_dispatch();

    ///    "<separator-char>" output separator;
    //
    	   NDL::begin_dispatch ( separator_char );
	      NDL::output ( separator_t );
    	   NDL::end_dispatch();

    ///    "," output separator;	// "<non-digit>,"
    //
    	   NDL::begin_dispatch ( "," );
	      NDL::output ( separator_t );
    	   NDL::end_dispatch();

    ///    "'" output separator;        // "'<non-letter>"
    ///
    //         See "'<letter>" call word; above.

    ///    "\"/" translate "" call quoted string;
    ///
    	   NDL::begin_dispatch ( "\"" );
	      NDL::call ( quoted_string );
    	   NDL::end_dispatch();

    ///    "<other>" output unrecognized character;
    //
    	   NDL::begin_dispatch ( OTHER );
	      NDL::output ( unrecognized_character_t );
    	   NDL::end_dispatch();

    ///    output end of file;
    ///
	   NDL::output ( end_of_file_t );

    /// end main master table;
    ///
    ///
    	NDL::end_table();

    /// // The below tables are entered from the master table
    /// // with the first one or two characters scanned.
    ///
    ///
    /// begin comment lexeme table;
    //
    NDL::begin_table ( comment );
    	   
    ///    "<non-line-break-char>" accept;
    ///    return;
    //
           NDL::begin_dispatch ( line_break_char );
	       NDL::keep ( 0 );
	       NDL::ret();
	   NDL::end_dispatch();
           NDL::begin_dispatch ( OTHER );
	       NDL::accept();
	   NDL::end_dispatch();

    /// end comment lexeme table;
    ///
    ///
    NDL::end_table();

    /// begin horizontal space lexeme table;
    //
    NDL::begin_table ( horizontal_space );

    ///    "<horizontal-space-char>" accept;
    //
           NDL::begin_dispatch ( horizontal_space_char );
	       NDL::accept();
	   NDL::end_dispatch();

    ///    return;
    //
    	   NDL::ret();

    /// end horizontal space lexeme table;
    ///
    ///
    NDL::end_table();

    /// begin line break lexeme table;
    NDL::begin_table ( line_break );

    ///    "<line-break-char>" accept;
    //
           NDL::begin_dispatch ( line_break_char );
	       NDL::accept();
	   NDL::end_dispatch();

    ///    return;
    //
    	   NDL::ret();

    /// end line break lexeme table;
    ///
    ///
    NDL::end_table();

    /// begin word lexeme table;
    ///
    ///    "<letter>" accept;
    ///    "'<letter>" accept;
    ///     "\\/" translate escaped character
    ///           require "<letter>"
    ///           else keep 0 return;
    ///    return;
    ///
    /// end word lexeme table;
    ///
    /// begin mark lexeme table;
    ///    "<mark-char>" accept;
    ///    ".<non-digit>" keep 1;
    ///    "/<non-slash-char>" keep 1;
    ///    return;
    /// end mark lexeme table;
    ///
    /// begin natural number lexeme table;
    ///    "<digit>" accept;
    ///    ".<digit>" keep 1 goto number;
    ///    "<digit>/<digit>" keep 2 goto number;
    ///    "<digit>,<digit>" keep 2 goto number;
    ///    return;
    /// end natural number lexeme table;
    ///
    /// begin number lexeme table;
    ///
    ///    // In order to recognize , and / surrounded by digits
    ///    // as number atoms, entries to this table upon
    ///    // recognizing "X<digit>" must do a `keep 1' so the
    ///    // digit will be left to be recognized by
    ///    // <digit>/<digit> or <digit>,<digit>.
    ///
    ///    // Alternatively we could have a separate table
    ///    // for the state where the last atom ended with
    ///    // a digit.
    ///
    ///    "<digit>" accept;
    ///    ".<digit>" keep 1;
    ///    "<digit>/<digit>" keep 2;
    ///    "<digit>,<digit>" keep 2;
    ///    return;
    ///
    /// end number lexeme table;
    ///
    ///
    /// begin quoted string lexeme table;
    ///
    ///     "\"/" translate "" return;   // End quoted string.
    ///
    ///     "\\/" translate escaped character
    ///           else translate bad escape sequence
    /// 	       error unrecognized escape sequence
    ///           else error unrecognized character;
    ///
    ///     "<line-break-char>"
    ///         goto bad end of line;
    ///
    ///     "<other>" accept;
    ///
    ///     goto bad end of file;
    ///
    /// end quoted string lexeme table;
    ///
    /// begin escaped character translation table;
    ///
    ///     "\\/\"/" translate "\"/";
    ///     "\\/\lf/" translate "\lf/";
    ///     "\\/\cr/" translate "\cr/";
    ///     "\\/\ht/" translate "\ht/";
    ///     "\\/\bs/" translate "\bs/";
    ///     "\\/\ff/" translate "\ff/";
    ///     "\\/\vt/" translate "\vt/";
    ///     "\\/\\//" translate "\\/";
    ///     "\\/~"  translate " " ;
    ///     "\\/0/" translate "\0/";
    ///     "\\/0<hex-digit>/"
    ///               translate hex 2 1;
    ///     "\\/0<hex-digit><hex-digit>/"
    ///               translate hex 2 1;
    ///     "\\/0<hex-digit><hex-digit><hex-digit>/"
    ///               translate hex 2 1;
    ///     "\\/0<hex-digit><hex-digit><hex-digit><hex-digit>/"
    ///               translate hex 2 1;
    ///     "\\/0<hex-digit><hex-digit><hex-digit><hex-digit>"
    ///         "<hex-digit>/"
    ///               translate hex 2 1;
    ///     "\\/0<hex-digit><hex-digit><hex-digit><hex-digit>"
    ///         "<hex-digit><hex-digit>/"
    ///               translate hex 2 1;
    ///     "\\/0<hex-digit><hex-digit><hex-digit><hex-digit>"
    ///         "<hex-digit><hex-digit><hex-digit>/"
    ///               translate hex 2 1;
    ///     "\\/0<hex-digit><hex-digit><hex-digit><hex-digit>"
    ///         "<hex-digit><hex-digit><hex-digit><hex-digit>/"
    ///               translate hex 2 1;
    ///     fail;
    /// end escaped character translation table;
    ///
    /// "<escaped-char>" = ~ "/" & ~ "<line-break-char>";
    ///
    /// begin bad escape sequence translation table;
    ///
    ///     "\\//";
    ///     "\\/<escaped-char>/";
    ///     "\\/<escaped-char><escaped-char>/";
    ///     "\\/<escaped-char><escaped-char>"
    ///        "<escaped-char>/";
    ///     "\\/<escaped-char><escaped-char>"
    ///        "<escaped-char><escaped-char>/";
    ///     "\\/<escaped-char><escaped-char>"
    ///        "<escaped-char><escaped-char>"
    ///        "<escaped-char>/";
    ///     "\\/<escaped-char><escaped-char>"
    ///        "<escaped-char><escaped-char>"
    ///        "<escaped-char><escaped-char>/";
    ///     "\\/<escaped-char><escaped-char>"
    ///        "<escaped-char><escaped-char>"
    ///        "<escaped-char><escaped-char>"
    ///        "<escaped-char>/";
    ///     "\\/<escaped-char><escaped-char>"
    ///        "<escaped-char><escaped-char>"
    ///        "<escaped-char><escaped-char>"
    ///        "<escaped-char><escaped-char>/";
    ///     "\\/<escaped-char><escaped-char>"
    ///        "<escaped-char><escaped-char>"
    ///        "<escaped-char><escaped-char>"
    ///        "<escaped-char><escaped-char>"
    ///        "<escaped-char>/";
    ///     "\\/<escaped-char><escaped-char>"
    ///        "<escaped-char><escaped-char>"
    ///        "<escaped-char><escaped-char>"
    ///        "<escaped-char><escaped-char>"
    ///        "<escaped-char><escaped-char>/";
    ///     fail;
    /// end bad escape sequence translation table;
    ///
    /// begin bad end of line master table;
    ///     output bad end of line goto main;
    /// end bad end of line master table;
    ///
    ///
    /// begin bad end of file master table;
    ///     output bad end of file goto main;
    /// end bad end of file master table;
    ///
    ///
    /// end standard lexical program;
    //
    NDL::end_program();
}
