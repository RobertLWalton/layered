// Layers Language Standard Lexical Analyzer
//
// File:	ll_lexeme_standard.cc
// Author:	Bob Walton (walton@seas.harvard.edu)
// Date:	Sat May  8 21:39:53 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/05/09 01:40:19 $
//   $RCSfile: ll_lexeme_standard.cc,v $
//   $Revision: 1.3 $

// Table of Contents
//
//	Usage and Setup
//	Data
//	Program Construction

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
using namespace ll::lexeme;
using namespace ll::lexeme::standard;


// Data
// ----

// Number of attach errors.
//
static uns32 error_count = 0;

// Type maps mapping letters to type 1.  The first two
// are for 'a'-'z' and 'A'-'Z' and may be handled
// otherwise in some dispatchers.
//
static uns32 letter_type_map_ID[250]; // type_map IDs
static uns32 letter_type_map_IDs;     // # type_map_IDs

static void create_letter_type_map_IDs ( void )
{
    if ( letter_type_map_IDs != 0 ) return;

#   define LETTER_RANGE(cmin, cmax) \
        letter_type_map_ID \
	    [letter_type_map_IDs ++] = \
		  create_type_map ( cmin, cmax, 1 );

    LETTER_RANGE ( 'a', 'z' );
    LETTER_RANGE ( 'A', 'Z' );
}

// Attach all the letter_type_maps to the given
// dispatcher.  If not_ascii is true, omit the first
// two letter_type_maps which are for ASCII letters.
//
static void attach_letter_type_maps
	( uns32 dispatcher_ID, bool not_ascii = false )
{
    create_letter_type_map_IDs();

    for ( uns32 i = ( not_ascii ? 2, 0 );
          i < letter_type_map_IDs; ++ i )
    {
        if ( ! attach ( letter_dispatcher,
	                letter_type_map_ID[i] ) )
	    ++ error_count;
    }
}


// Program Construction
// ------- ------------

void ll::lexeme::standard::create_standard_program
	( void )
{
    error_count = 0;
#   define ATTACH(x,y) \
        if ( ! attach ( x, y ) ) ++ error_count

    // begin standard lexical program;
    //
    // "<ascii-letter>" = "a-z" | "A-Z";
    // "<digit>" = "0-9";
    // "<oct-digit>" = "0-7";
    // "<hex-digit>" = "0-9" | "a-f" | "A-F";
    //
    // "<non-digit>" = ~ "<digit>";
    //
    // The following list of language specific letters
    // is taken from Annex E of the Working Paper for
    // Draft Proposed International Standard for
    // Information Systems—Programming Language C++,
    // 1996.
    //
    // "<latin-letter>" = "\u00c0-\n00d6"
    //                  | "\u00d8-\u00f6"
    //                  | "\u00f8-\u01f5"
    //                  | "\u01fa-\u0217"
    //                  | "\u0250-\u02a8"
    //                  | "\u1ea0-\u1ef9";
    //
    // . . . . . letter character pattern definitions
    //           omitted . . . . .
    //
    // "<CJK-letter>" = "\uf900-\ufa2d"
    //                | . . .  // Details omitted
    //
    // Context sensitive character classifications,
    // e.g., classifying ' as a word character if it is
    // followed by a letter, cannot be included in
    // character pattern definitions, but are noted in
    // comments.  They are accounted for by separate atom
    // table entries below.
    //
    // "<letter>" = "<ascii-letter>" | "<latin-letter>"
    //            | . . . | "<CJK-letter>";
    //            // Details omitted
    //
    // "<non-letter>" = ~ "<letter>";
    //
    // "<mark-char>" = "+" | "-" | "*" | "~" | "@" | "#"
    //               | "$" | "%" | "^" | "&" | "=" | "|"
    //               | "<" | ">" | "_" | "!" | "?"
    //               | ":";
    //               //
    //               // Also \ not followed by u or U,
    //               //      . not followed by a digit
    //               //      / not surrounded by digits
    //
    // "<non-u-char>" = ~ "u" & ~ "U";
    // "<non-slash-char>" = ~ "/";
    //
    // "<separator-char>" = "(" | ")" | "[" | "]" | "{"
    //                    | "}" | ";" | "`";
    //               //
    //               // Also , not surrounded by digits
    //               //      ' not followed by a letter
    //
    // "<line-break-char>" = "\n" | "\v" | "\f" | "\r";
    //
    // "<non-line-break-char>" =
    //     ~ "<line-break-character>";
    //
    // "<horizontal-space-char>" = " " | "\t";

    // Dispatcher that maps letters to 1 and other
    // characters to 0.  Implements letter character
    // pattern for \uXXXX and \UXXXXXXXX escape
    // sequences.
    //
    uns32 letter_dispatcher =
        create_dispatcher ( 500, 2 );
    attach_letter_type_maps ( letter_dispatcher );

    // All the atom tables are created first so
    // instructions can be created which do not do
    // forward references.
    //
    uns32 master = create_atom_table ( MASTER );
    uns32 comment = create_atom_table ( comment_t );
    uns32 horizontal_space =
	create_atom_table ( horizontal_space_t );
    uns32 line_break =
        create_atom_table ( line_break_t );
    uns32 word = create_atom_table ( word_t );
    uns32 mark = create_atom_table ( mark_t );
    uns32 natural_number =
        create_atom_table ( natural_number_t );
    uns32 number = create_atom_table ( number_t );

    // begin master atom table;
    //
    //   // Whitespace is separated out into 3 kinds of
    //   // lexeme:
    //   //
    //   //   `horizontal space'  spaces and horizontal
    //   //                       tabs
    //   //   `line break'	  linefeeds, carriage
    //   //                       returns, form feeds,
    //   //                       and vertical tabs
    //   //   `comments'          `//' up to just before
    //   //                       next line break
    //   //
    //    "//" call comment;
    //    "<horizontal-space-char>" call horizontal space;
    //    "<line-break-char>" call line break;
    //
    //    "<word-char>" call word;
    //    "'<letter>" call word;
    //     "\\u<hex-digit><hex-digit>"
    //        "<hex-digit><hex-digit>"
    //         translate hex 2 0 "<letter"> call word
    //         else keep 2 error bad escape sequence;
    //     "\\U<hex-digit><hex-digit>"
    //        "<hex-digit><hex-digit>"
    //        "<hex-digit><hex-digit>"
    //        "<hex-digit><hex-digit>"
    //         translate hex 2 0 "<letter"> call word
    //         else keep 2 error bad escape sequence;
    //
    //     "\\u" error bad escape sequence;
    //     "\\U" error bad escape sequence;
    //
    //    "<mark-char>" call mark;
    //    "." call mark;        // ".<non-digit>"
    //    "/" call mark;        // "/<non-slash-char>"
    //    "\\" call mark;       // "\\<non-u-char>"
    //
    //    // We assume that the preceding text is not a
    //    // digit or the current input is not / or ,
    //    // followed by a digit.
    //    //
    //    "<digit>" keep 0 call natural_number;
    //    ".<digit>" keep 1 call number;
    //
    //    "<separator-char>" output separator;
    //    "," output separator;
    //    "'" output separator; // "'<non-letter>"
    //
    //    "\"" translate "" call quoted string;
    //
    //    "<other>" output bad character;
    //    output end of file;
    //
    // end master atom table;

    uns32 end_of_file_instruction =
        create_instruction ( OUTPUT, end_of_file_t );
    ATTACH ( master, end_of_file_instruction );

    uns32 master_dispatcher1 =
        create_dispatcher ( 500, 20 );
    ATTACH ( master, master_dispatcher1 );

    // Type map for master_dispatcher_1 first 128
    // characters.
    //
    const uns32 letter_c     = 1;  // Must be 1.
    const uns32 digit_c      = 2;
    const uns32 mark_c       = 3;
    const uns32 sep_c        = 4;
    const uns32 line_break_c = 5;
    const uns32 h_space_c    = 6;
    const uns32 point_c      = 7;
    const uns32 slash_c      = 8;
    const uns32 backslash_c  = 9;
    const uns32 single_q_c   = 10;
    const uns32 double_q_c   = 11;

    uns8 master_cmap[128] =
        {
            0,              // NUL '\0'
            0,		    // SOH (start of heading)
            0,		    // STX (start of text)
            0,		    // ETX (end of text)
            0,		    // EOT (end of transmission)
            0,		    // ENQ (enquiry)
            0,		    // ACK (acknowledge)
            0,		    // BEL '\a' (bell)
            0,		    // BS  '\b' (backspace)
            h_space_c,	    // HT  '\t' (horizontal tab)
            line_break_c,   // LF  '\n' (new line)
            line_break_c,   // VT  '\v' (vertical tab)
            line_break_c,   // FF  '\f' (form feed)
            line_break_c,   // CR  '\r' (carriage ret)
            0,		    // SO  (shift out)
            0,		    // SI  (shift in)
            0,		    // DLE (data link escape)
            0,		    // DC1 (device control 1)
            0,		    // DC2 (device control 2)
            0,		    // DC3 (device control 3)
            0,		    // DC4 (device control 4)
            0,		    // NAK (negative ack.)
            0,		    // SYN (synchronous idle)
            0,		    // ETB (end of trans. blk)
            0,		    // CAN (cancel)
            0,		    // EM  (end of medium)
            0,		    // SUB (substitute)
            0,		    // ESC (escape)
            0,		    // FS  (file separator)
            0,		    // GS  (group separator)
            0,		    // RS  (record separator)
            0,		    // US  (unit separator)
            h_space_c,      // SPACE
            mark_c,	    // !
            double_q_c,	    // "
            mark_c,	    // #
            mark_c,	    // $
            mark_c,	    // %
            mark_c,	    // &
            single_q_c,	    // '
            sep_c,	    // (
            sep_c,	    // )
            mark_c,	    // *
            mark_c,	    // +
            sep_c,	    // ,
            mark_c,	    // -
            point_c,	    // .
            slash_c,	    // /
            digit_c,	    // 0
            digit_c,	    // 1
            digit_c,	    // 2
            digit_c,	    // 3
            digit_c,	    // 4
            digit_c,	    // 5
            digit_c,	    // 6
            digit_c,	    // 7
            digit_c,	    // 8
            digit_c,	    // 9
            mark_c,	    // :
            sep_c,	    // ;
            mark_c,	    // <
            mark_c,	    // =
            mark_c,	    // >
            mark_c,	    // ?
            mark_c,	    // @
            letter_c,	    // A
            letter_c,	    // B
            letter_c,	    // C
            letter_c,	    // D
            letter_c,	    // E
            letter_c,	    // F
            letter_c,	    // G
            letter_c,	    // H
            letter_c,	    // I
            letter_c,	    // J
            letter_c,	    // K
            letter_c,	    // L
            letter_c,	    // M
            letter_c,	    // N
            letter_c,	    // O
            letter_c,	    // P
            letter_c,	    // Q
            letter_c,	    // R
            letter_c,	    // S
            letter_c,	    // T
            letter_c,	    // U
            letter_c,	    // V
            letter_c,	    // W
            letter_c,	    // X
            letter_c,	    // Y
            letter_c,	    // Z
            sep_c,	    // [
            backslash_c,    // \  '\\'
            sep_c,	    // ]
            mark_c,	    // ^
            mark_c,	    // _
            mark_c,	    // `
            letter_c,	    // a
            letter_c,	    // b
            letter_c,	    // c
            letter_c,	    // d
            letter_c,	    // e
            letter_c,	    // f
            letter_c,	    // g
            letter_c,	    // h
            letter_c,	    // i
            letter_c,	    // j
            letter_c,	    // k
            letter_c,	    // l
            letter_c,	    // m
            letter_c,	    // n
            letter_c,	    // o
            letter_c,	    // p
            letter_c,	    // q
            letter_c,	    // r
            letter_c,	    // s
            letter_c,	    // t
            letter_c,	    // u
            letter_c,	    // v
            letter_c,	    // w
            letter_c,	    // x
            letter_c,	    // y
            letter_c,	    // z
            sep_c,	    // {
            mark_c,	    // |
            sep_c,	    // }
            mark_c,	    // ~
            0	    	    // DEL
        };

    uns32 master_tmap1 =
        create_type_map ( 0, 127, master_cmap );
    ATTACH ( master_dispatcher1, master_tmap1 );
    attach_letter_type_maps
        ( master_dispatcher1, true );

    uns32 call_horizontal_space =
        create_instruction
	    ( CALLRETURN, horizontal_space );
    ATTACH ( master_dispatcher1, h_space_c,
             call_horizontal_space );

    uns32 call_line_break =
        create_instruction
	    ( CALLRETURN, line_break );
    ATTACH ( master_dispatcher1, line_break_c,
             call_line_break );

    uns32 call_word =
        create_instruction
	    ( CALLRETURN, word );
    ATTACH ( master_dispatcher1, word_c,
             call_word );

    uns32 call_mark =
        create_instruction
	    ( CALLRETURN, mark );
    ATTACH ( master_dispatcher1, mark_c,
             call_mark );
    ATTACH ( master_dispatcher1, point_c,
             call_mark );
    ATTACH ( master_dispatcher1, slash_c,
             call_mark );
    ATTACH ( master_dispatcher1, backslash_c,
             call_mark );

    uns32 call_natural_number =
        create_instruction
	    ( CALLRETURN, natural_number );
    ATTACH ( master_dispatcher1, digit_c,
             call_natural_number );

    uns32 output_separator =
        create_instruction
	    ( OUTPUT, separator_t );
    ATTACH ( master_dispatcher1, sep_c,
             output_separator );
    ATTACH ( master_dispatcher1, single_q_c,
             output_separator );

    uns32 call_quoted_string =
        create_instruction
	    ( CALLRETURN, quoted_string );
    ATTACH ( master_dispatcher1, double_q_c,
             call_quoted_string );

    uns32 output_bad_character =
        create_instruction
	    ( OUTPUT, bad_character_t );
    ATTACH ( master_dispatcher1, 0,
             output_bad_character );

    uns32 master_dispatcher2_slash =
        create_dispatcher ( 3, 2 );
    uns32 slash_tmap =
        create_type_map ( '/', '/', 1 );
    ATTACH ( master_dispatcher2_slash,
             slash_tmap );
    ATTACH ( master_dispatcher1,
             master_dispatcher2_slash,
	     slash_c );
    uns32 call_comment =
        create_instruction
	    ( CALLRETURN, comment );
    ATTACH ( master_dispatcher2_slash,
             1, call_comment );

    uns32 master_dispatcher2_single_q =
        create_dispatcher ( 500, 2 );
    attach_letter_type_maps
        ( master_dispatcher2_single_q );
    ATTACH ( master_dispatcher1,
             master_dispatcher2_single_q,
	     single_q_c );
    ATTACH ( master_dispatcher2_single_q,
             1, call_word );

    uns32 master_dispatcher2_point =
        create_dispatcher ( 3, 2 );
    uns32 digit_tmap =
        create_type_map ( '0', '9', 1 );
    ATTACH ( master_dispatcher2_point,
             digit_tmap );
    ATTACH ( master_dispatcher1,
             master_dispatcher2_point,
	     point_c );
    uns32 call_number =
        create_instruction
	    ( CALLRETURN, number );
    ATTACH ( master_dispatcher2_point,
             1, call_number );

    uns8 u_c = 1;
    uns8 U_c = 2;
    uns8 Uu_cmap[128] =
        {
            0,              // NUL '\0'
            0,		    // SOH (start of heading)
            0,		    // STX (start of text)
            0,		    // ETX (end of text)
            0,		    // EOT (end of transmission)
            0,		    // ENQ (enquiry)
            0,		    // ACK (acknowledge)
            0,		    // BEL '\a' (bell)
            0,		    // BS  '\b' (backspace)
            0,		    // HT  '\t' (horizontal tab)
            0,		    // LF  '\n' (new line)
            0,		    // VT  '\v' (vertical tab)
            0,		    // FF  '\f' (form feed)
            0,		    // CR  '\r' (carriage ret)
            0,		    // SO  (shift out)
            0,		    // SI  (shift in)
            0,		    // DLE (data link escape)
            0,		    // DC1 (device control 1)
            0,		    // DC2 (device control 2)
            0,		    // DC3 (device control 3)
            0,		    // DC4 (device control 4)
            0,		    // NAK (negative ack.)
            0,		    // SYN (synchronous idle)
            0,		    // ETB (end of trans. blk)
            0,		    // CAN (cancel)
            0,		    // EM  (end of medium)
            0,		    // SUB (substitute)
            0,		    // ESC (escape)
            0,		    // FS  (file separator)
            0,		    // GS  (group separator)
            0,		    // RS  (record separator)
            0,		    // US  (unit separator)
            0,		    // SPACE
            0,		    // !
            0,		    // "
            0,		    // #
            0,		    // $
            0,		    // %
            0,		    // &
            0,		    // '
            0,		    // (
            0,		    // )
            0,		    // *
            0,		    // +
            0,		    // ,
            0,		    // -
            0,		    // .
            0,		    // /
            0,		    // 0
            0,		    // 1
            0,		    // 2
            0,		    // 3
            0,		    // 4
            0,		    // 5
            0,		    // 6
            0,		    // 7
            0,		    // 8
            0,		    // 9
            0,		    // :
            0,		    // ;
            0,		    // <
            0,		    // =
            0,		    // >
            0,		    // ?
            0,		    // @
            0,		    // A
            0,		    // B
            0,		    // C
            0,		    // D
            0,		    // E
            0,		    // F
            0,		    // G
            0,		    // H
            0,		    // I
            0,		    // J
            0,		    // K
            0,		    // L
            0,		    // M
            0,		    // N
            0,		    // O
            0,		    // P
            0,		    // Q
            0,		    // R
            0,		    // S
            0,		    // T
            U_c,	    // U
            0,		    // V
            0,		    // W
            0,		    // X
            0,		    // Y
            0,		    // Z
            0,		    // [
            0,	    	    // \  '\\'
            0,		    // ]
            0,		    // ^
            0,		    // _
            0,		    // `
            0,		    // a
            0,		    // b
            0,		    // c
            0,		    // d
            0,		    // e
            0,		    // f
            0,		    // g
            0,		    // h
            0,		    // i
            0,		    // j
            0,		    // k
            0,		    // l
            0,		    // m
            0,		    // n
            0,		    // o
            0,		    // p
            0,		    // q
            0,		    // r
            0,		    // s
            0,		    // t
            u_c,	    // u
            0,		    // v
            0,		    // w
            0,		    // x
            0,		    // y
            0,		    // z
            0,		    // {
            0,		    // |
            0,		    // }
            0,		    // ~
            0	    	    // DEL
        };

    uns32 master_dispatcher2_backslash =
        create_dispatcher ( 2, 3 );
    uns32 Uu_tmap =
        create_type_map ( 0, 127, Uu_cmap );
    ATTACH ( master_dispatcher2_backslash, Uu_tmap );
    ATTACH ( master_dispatcher1,
             master_dispatcher2_backslash,
	     backslash_c );
    uns32 error_bad_escape_sequence =
        create_instruction
	    ( ERRONEOUS_ATOM, bad_escape_sequence_t );
    ATTACH ( master_dispatcher2_backslash,
             0, error_bad_escape_sequence );

    uns8 hex_digit_cmap[128] =
        {
            0,              // NUL '\0'
            0,		    // SOH (start of heading)
            0,		    // STX (start of text)
            0,		    // ETX (end of text)
            0,		    // EOT (end of transmission)
            0,		    // ENQ (enquiry)
            0,		    // ACK (acknowledge)
            0,		    // BEL '\a' (bell)
            0,		    // BS  '\b' (backspace)
            0,		    // HT  '\t' (horizontal tab)
            0,		    // LF  '\n' (new line)
            0,		    // VT  '\v' (vertical tab)
            0,		    // FF  '\f' (form feed)
            0,		    // CR  '\r' (carriage ret)
            0,		    // SO  (shift out)
            0,		    // SI  (shift in)
            0,		    // DLE (data link escape)
            0,		    // DC1 (device control 1)
            0,		    // DC2 (device control 2)
            0,		    // DC3 (device control 3)
            0,		    // DC4 (device control 4)
            0,		    // NAK (negative ack.)
            0,		    // SYN (synchronous idle)
            0,		    // ETB (end of trans. blk)
            0,		    // CAN (cancel)
            0,		    // EM  (end of medium)
            0,		    // SUB (substitute)
            0,		    // ESC (escape)
            0,		    // FS  (file separator)
            0,		    // GS  (group separator)
            0,		    // RS  (record separator)
            0,		    // US  (unit separator)
            0,		    // SPACE
            0,		    // !
            0,		    // "
            0,		    // #
            0,		    // $
            0,		    // %
            0,		    // &
            0,		    // '
            0,		    // (
            0,		    // )
            0,		    // *
            0,		    // +
            0,		    // ,
            0,		    // -
            0,		    // .
            0,		    // /
            1,		    // 0
            1,		    // 1
            1,		    // 2
            1,		    // 3
            1,		    // 4
            1,		    // 5
            1,		    // 6
            1,		    // 7
            1,		    // 8
            1,		    // 9
            0,		    // :
            0,		    // ;
            0,		    // <
            0,		    // =
            0,		    // >
            0,		    // ?
            0,		    // @
            1,		    // A
            1,		    // B
            1,		    // C
            1,		    // D
            1,		    // E
            1,		    // F
            0,		    // G
            0,		    // H
            0,		    // I
            0,		    // J
            0,		    // K
            0,		    // L
            0,		    // M
            0,		    // N
            0,		    // O
            0,		    // P
            0,		    // Q
            0,		    // R
            0,		    // S
            0,		    // T
            0,		    // U
            0,		    // V
            0,		    // W
            0,		    // X
            0,		    // Y
            0,		    // Z
            0,		    // [
            0,	    	    // \  '\\'
            0,		    // ]
            0,		    // ^
            0,		    // _
            0,		    // `
            1,		    // a
            1,		    // b
            1,		    // c
            1,		    // d
            1,		    // e
            1,		    // f
            0,		    // g
            0,		    // h
            0,		    // i
            0,		    // j
            0,		    // k
            0,		    // l
            0,		    // m
            0,		    // n
            0,		    // o
            0,		    // p
            0,		    // q
            0,		    // r
            0,		    // s
            0,		    // t
            0,		    // u
            0,		    // v
            0,		    // w
            0,		    // x
            0,		    // y
            0,		    // z
            0,		    // {
            0,		    // |
            0,		    // }
            0,		    // ~
            0	    	    // DEL
        };
    uns32 hex_digit_tmap =
        create_type_map ( 0, 127, hex_digit_cmap );

    uns32 master_dispatcher_u =
        master_dispatcher2_backslash;
    uns32 u_type = u_c;
    for ( uns32 i = 0; i < 4; ++ i )
    {
        uns32 hex_digit_dispatcher =
	    create_dispatcher ( 2, 2 );
	ATTACH ( hex_digit_dispatcher,
	         hex_digit_tmap );
	ATTACH ( master_dispatcher_u,
	         hex_digit_dispatcher,
		 u_type );
	master_dispatcher_u = hex_digit_dispatcher;
	u_type = 1;
    }
    uns32 keep_2_error_bad_escape_sequence =
        create_instruction
	    ( ERRONEOUS_ATOM + KEEP(2),
	      0, bad_escape_sequence_t );
    uns32 master_translate_uU =
        create_instruction
	    ( TRANSLATE_HEX(2,0) + CALLRETURN + ELSE,
	      word, 0, NULL,
	      letter_dispatcher,
	      keep_2_error_bad_escape_sequence );
    ATTACH ( master_dispatcher_u,
             1, master_translate_uU );

    uns32 master_dispatcher_U =
        master_dispatcher2_backslash;
    uns32 U_type = U_c;
    for ( uns32 i = 0; i < 8; ++ i )
    {
        uns32 hex_digit_dispatcher =
	    create_dispatcher ( 2, 2 );
	ATTACH ( hex_digit_dispatcher,
	         hex_digit_tmap );
	ATTACH ( master_dispatcher_U,
	         hex_digit_dispatcher,
		 U_type );
	master_dispatcher_U = hex_digit_dispatcher;
	U_type = 1;
    }
    ATTACH ( master_dispatcher_U,
             1, master_translate_uU );

    // The below tables are entered from the master table
    // with the first one or two characters scanned.

    // begin comment atom table;
    //    "<non-line-break-char>" accept;
    //    return;
    // end comment atom table;

    uns32 comment_dispatcher =
        create_dispatcher ( 2, 2 );
    ATTACH ( comment, comment_dispatcher );

    uns8 line_break_cmap[128] =
        {
            0,              // NUL '\0'
            0,		    // SOH (start of heading)
            0,		    // STX (start of text)
            0,		    // ETX (end of text)
            0,		    // EOT (end of transmission)
            0,		    // ENQ (enquiry)
            0,		    // ACK (acknowledge)
            0,		    // BEL '\a' (bell)
            0,		    // BS  '\b' (backspace)
            0,		    // HT  '\t' (horizontal tab)
            1,		    // LF  '\n' (new line)
            1,		    // VT  '\v' (vertical tab)
            1,		    // FF  '\f' (form feed)
            1,		    // CR  '\r' (carriage ret)
            0,		    // SO  (shift out)
            0,		    // SI  (shift in)
            0,		    // DLE (data link escape)
            0,		    // DC1 (device control 1)
            0,		    // DC2 (device control 2)
            0,		    // DC3 (device control 3)
            0,		    // DC4 (device control 4)
            0,		    // NAK (negative ack.)
            0,		    // SYN (synchronous idle)
            0,		    // ETB (end of trans. blk)
            0,		    // CAN (cancel)
            0,		    // EM  (end of medium)
            0,		    // SUB (substitute)
            0,		    // ESC (escape)
            0,		    // FS  (file separator)
            0,		    // GS  (group separator)
            0,		    // RS  (record separator)
            0,		    // US  (unit separator)
            0,		    // SPACE
            0,		    // !
            0,		    // "
            0,		    // #
            0,		    // $
            0,		    // %
            0,		    // &
            0,		    // '
            0,		    // (
            0,		    // )
            0,		    // *
            0,		    // +
            0,		    // ,
            0,		    // -
            0,		    // .
            0,		    // /
            0,		    // 0
            0,		    // 1
            0,		    // 2
            0,		    // 3
            0,		    // 4
            0,		    // 5
            0,		    // 6
            0,		    // 7
            0,		    // 8
            0,		    // 9
            0,		    // :
            0,		    // ;
            0,		    // <
            0,		    // =
            0,		    // >
            0,		    // ?
            0,		    // @
            0,		    // A
            0,		    // B
            0,		    // C
            0,		    // D
            0,		    // E
            0,		    // F
            0,		    // G
            0,		    // H
            0,		    // I
            0,		    // J
            0,		    // K
            0,		    // L
            0,		    // M
            0,		    // N
            0,		    // O
            0,		    // P
            0,		    // Q
            0,		    // R
            0,		    // S
            0,		    // T
            0,		    // U
            0,		    // V
            0,		    // W
            0,		    // X
            0,		    // Y
            0,		    // Z
            0,		    // [
            0,	    	    // \  '\\'
            0,		    // ]
            0,		    // ^
            0,		    // _
            0,		    // `
            0,		    // a
            0,		    // b
            0,		    // c
            0,		    // d
            0,		    // e
            0,		    // f
            0,		    // g
            0,		    // h
            0,		    // i
            0,		    // j
            0,		    // k
            0,		    // l
            0,		    // m
            0,		    // n
            0,		    // o
            0,		    // p
            0,		    // q
            0,		    // r
            0,		    // s
            0,		    // t
            0,		    // u
            0,		    // v
            0,		    // w
            0,		    // x
            0,		    // y
            0,		    // z
            0,		    // {
            0,		    // |
            0,		    // }
            0,		    // ~
            0	    	    // DEL
        };
    uns32 line_break_tmap =
        create_type_map ( 0, 127, line_break_cmap );
    ATTACH ( comment_dispatcher, line_break_tmap );

    // begin horizontal space atom table;
    //    "<horizontal-space-char>" accept;
    //    return;
    // end horizontal space atom table;

    uns32 horizontal_space_dispatcher =
        create_dispatcher ( 2, 2 );
    ATTACH ( horizontal_space,
             horizontal_space_dispatcher );

    uns8 horizontal_space_cmap[128] =
        {
            0,              // NUL '\0'
            0,		    // SOH (start of heading)
            0,		    // STX (start of text)
            0,		    // ETX (end of text)
            0,		    // EOT (end of transmission)
            0,		    // ENQ (enquiry)
            0,		    // ACK (acknowledge)
            0,		    // BEL '\a' (bell)
            0,		    // BS  '\b' (backspace)
            1,		    // HT  '\t' (horizontal tab)
            0,		    // LF  '\n' (new line)
            0,		    // VT  '\v' (vertical tab)
            0,		    // FF  '\f' (form feed)
            0,		    // CR  '\r' (carriage ret)
            0,		    // SO  (shift out)
            0,		    // SI  (shift in)
            0,		    // DLE (data link escape)
            0,		    // DC1 (device control 1)
            0,		    // DC2 (device control 2)
            0,		    // DC3 (device control 3)
            0,		    // DC4 (device control 4)
            0,		    // NAK (negative ack.)
            0,		    // SYN (synchronous idle)
            0,		    // ETB (end of trans. blk)
            0,		    // CAN (cancel)
            0,		    // EM  (end of medium)
            0,		    // SUB (substitute)
            0,		    // ESC (escape)
            0,		    // FS  (file separator)
            0,		    // GS  (group separator)
            0,		    // RS  (record separator)
            0,		    // US  (unit separator)
            1,		    // SPACE
            0,		    // !
            0,		    // "
            0,		    // #
            0,		    // $
            0,		    // %
            0,		    // &
            0,		    // '
            0,		    // (
            0,		    // )
            0,		    // *
            0,		    // +
            0,		    // ,
            0,		    // -
            0,		    // .
            0,		    // /
            0,		    // 0
            0,		    // 1
            0,		    // 2
            0,		    // 3
            0,		    // 4
            0,		    // 5
            0,		    // 6
            0,		    // 7
            0,		    // 8
            0,		    // 9
            0,		    // :
            0,		    // ;
            0,		    // <
            0,		    // =
            0,		    // >
            0,		    // ?
            0,		    // @
            0,		    // A
            0,		    // B
            0,		    // C
            0,		    // D
            0,		    // E
            0,		    // F
            0,		    // G
            0,		    // H
            0,		    // I
            0,		    // J
            0,		    // K
            0,		    // L
            0,		    // M
            0,		    // N
            0,		    // O
            0,		    // P
            0,		    // Q
            0,		    // R
            0,		    // S
            0,		    // T
            0,		    // U
            0,		    // V
            0,		    // W
            0,		    // X
            0,		    // Y
            0,		    // Z
            0,		    // [
            0,	    	    // \  '\\'
            0,		    // ]
            0,		    // ^
            0,		    // _
            0,		    // `
            0,		    // a
            0,		    // b
            0,		    // c
            0,		    // d
            0,		    // e
            0,		    // f
            0,		    // g
            0,		    // h
            0,		    // i
            0,		    // j
            0,		    // k
            0,		    // l
            0,		    // m
            0,		    // n
            0,		    // o
            0,		    // p
            0,		    // q
            0,		    // r
            0,		    // s
            0,		    // t
            0,		    // u
            0,		    // v
            0,		    // w
            0,		    // x
            0,		    // y
            0,		    // z
            0,		    // {
            0,		    // |
            0,		    // }
            0,		    // ~
            0	    	    // DEL
        };
    uns32 horizontal_space_tmap =
        create_type_map
	    ( 0, 127, horizontal_space_cmap );
    ATTACH ( horizontal_space_dispatcher,
             horizontal_space_tmap );

    // begin line break atom table;
    //    "<line-break-char>" accept;
    //    return;
    // end line break atom table;

    uns32 line_break_dispatcher =
        create_dispatcher ( 2, 2 );
    ATTACH ( line_break, line_break_dispatcher );
    ATTACH ( line_break_dispatcher, line_break_tmap );

    // begin word atom table;
    //    "<word-char>" accept;
    //    "'<letter>" accept;
    //     "\\u<hex-digit><hex-digit><hex-digit><hex-digit>"
    //         translate hex 2 0 "<letter">
    //         else keep 0 return
    //    "\\U<hex-digit><hex-digit><hex-digit><hex-digit>"
    //       "<hex-digit><hex-digit><hex-digit><hex-digit>"
    //        translate hex 2 0 "<letter">
    //         else keep 0 return
    //   return;
    // end word atom table;

    uns32 word_dispatcher = create_dispatcher ( 500, 4 );
    ATTACH ( word, word_break_dispatcher );

    const uns32 word_c            = 1;  // Must be 1.
    const uns32 word_q_c          = 2;
    const uns32 word_backslash_c  = 3;
    uns8 word_cmap[128] =
        {
            0,              // NUL '\0'
            0,		    // SOH (start of heading)
            0,		    // STX (start of text)
            0,		    // ETX (end of text)
            0,		    // EOT (end of transmission)
            0,		    // ENQ (enquiry)
            0,		    // ACK (acknowledge)
            0,		    // BEL '\a' (bell)
            0,		    // BS  '\b' (backspace)
            0,		    // HT  '\t' (horizontal tab)
            0,		    // LF  '\n' (new line)
            0,		    // VT  '\v' (vertical tab)
            0,		    // FF  '\f' (form feed)
            0,		    // CR  '\r' (carriage ret)
            0,		    // SO  (shift out)
            0,		    // SI  (shift in)
            0,		    // DLE (data link escape)
            0,		    // DC1 (device control 1)
            0,		    // DC2 (device control 2)
            0,		    // DC3 (device control 3)
            0,		    // DC4 (device control 4)
            0,		    // NAK (negative ack.)
            0,		    // SYN (synchronous idle)
            0,		    // ETB (end of trans. blk)
            0,		    // CAN (cancel)
            0,		    // EM  (end of medium)
            0,		    // SUB (substitute)
            0,		    // ESC (escape)
            0,		    // FS  (file separator)
            0,		    // GS  (group separator)
            0,		    // RS  (record separator)
            0,		    // US  (unit separator)
            0,		    // SPACE
            0,		    // !
            0,		    // "
            0,		    // #
            0,		    // $
            0,		    // %
            0,		    // &
            word_q_c,	    // '
            0,		    // (
            0,		    // )
            0,		    // *
            0,		    // +
            0,		    // ,
            0,		    // -
            0,		    // .
            0,		    // /
            word_c,	    // 0
            word_c,	    // 1
            word_c,	    // 2
            word_c,	    // 3
            word_c,	    // 4
            word_c,	    // 5
            word_c,	    // 6
            word_c,	    // 7
            word_c,	    // 8
            word_c,	    // 9
            0,		    // :
            0,		    // ;
            0,		    // <
            0,		    // =
            0,		    // >
            0,		    // ?
            0,		    // @
            word_c,	    // A
            word_c,	    // B
            word_c,	    // C
            word_c,	    // D
            word_c,	    // E
            word_c,	    // F
            word_c,	    // G
            word_c,	    // H
            word_c,	    // I
            word_c,	    // J
            word_c,	    // K
            word_c,	    // L
            word_c,	    // M
            word_c,	    // N
            word_c,	    // O
            word_c,	    // P
            word_c,	    // Q
            word_c,	    // R
            word_c,	    // S
            word_c,	    // T
            word_c,	    // U
            word_c,	    // V
            word_c,	    // W
            word_c,	    // X
            word_c,	    // Y
            word_c,	    // Z
            0,		    // [
            word_backslash_c,
			    // \  '\\'
            0,		    // ]
            0,		    // ^
            0,		    // _
            0,		    // `
            word_c,	    // a
            word_c,	    // b
            word_c,	    // c
            word_c,	    // d
            word_c,	    // e
            word_c,	    // f
            word_c,	    // g
            word_c,	    // h
            word_c,	    // i
            word_c,	    // j
            word_c,	    // k
            word_c,	    // l
            word_c,	    // m
            word_c,	    // n
            word_c,	    // o
            word_c,	    // p
            word_c,	    // q
            word_c,	    // r
            word_c,	    // s
            word_c,	    // t
            word_c,	    // u
            word_c,	    // v
            word_c,	    // w
            word_c,	    // x
            word_c,	    // y
            word_c,	    // z
            0,		    // {
            0,		    // |
            0,		    // }
            0,		    // ~
            0	    	    // DEL
        };
    uns32 word_tmap =
        create_type_map ( 0, 127, word_cmap );
    ATTACH ( word_dispatcher, word_tmap );
    attach_letter_tmaps ( word_dispatcher, true );

    uns32 word_dispatcher2_q =
        create_dispatcher ( 500, 2 );
    attach_letter_type_maps ( word_dispatcher2_q );
    ATTACH ( word_dispatcher1,
             word_dispatcher2_q,
	     word_q_c );

    uns32 word_dispatcher2_backslash =
        create_dispatcher ( 2, 3 );
    ATTACH ( word_dispatcher2_backslash,
             Uu_tmap );
    ATTACH ( word_dispatcher1,
             word_dispatcher2_backslash,
	     word_backslash_c );

    uns32 word_dispatcher_u =
        word_dispatcher2_backslash;
    uns32 word_u_type = u_c;
    for ( uns32 i = 0; i < 4; ++ i )
    {
        uns32 hex_digit_dispatcher =
	    create_dispatcher ( 2, 2 );
	ATTACH ( hex_digit_dispatcher,
	         hex_digit_tmap );
	ATTACH ( word_dispatcher_u,
	         hex_digit_dispatcher,
		 word_u_type );
	word_dispatcher_u = hex_digit_dispatcher;
	word_u_type = 1;
    }

    uns32 word_dispatcher_U =
        word_dispatcher2_backslash;
    uns32 word_U_type = U_c;
    for ( uns32 i = 0; i < 8; ++ i )
    {
        uns32 hex_digit_dispatcher =
	    create_dispatcher ( 2, 2 );
	ATTACH ( hex_digit_dispatcher,
	         hex_digit_tmap );
	ATTACH ( word_dispatcher_U,
	         hex_digit_dispatcher,
		 word_U_type );
	word_dispatcher_U = hex_digit_dispatcher;
	word_U_type = 1;
    }

    // begin mark atom table;
    //    "<mark-char>" accept;
    //    ".<non-digit>" keep 1;
    //    "/<non-slash-char>" keep 1;
    //    "\\<non-u-char>" keep 1;
    //    return;
    // end mark atom table;

    uns32 mark_dispatcher1 =
        create_dispatcher ( 2, 2 );
    ATTACH ( mark, mark_dispatcher1 );

    const uns32 mark_m_c          = 1;
    const uns32 mark_point_c      = 2;
    const uns32 mark_backslash_c  = 3;
    uns8 mark_cmap[128] =
        {
            0,              // NUL '\0'
            0,		    // SOH (start of heading)
            0,		    // STX (start of text)
            0,		    // ETX (end of text)
            0,		    // EOT (end of transmission)
            0,		    // ENQ (enquiry)
            0,		    // ACK (acknowledge)
            0,		    // BEL '\a' (bell)
            0,		    // BS  '\b' (backspace)
            0,		    // HT  '\t' (horizontal tab)
            0,		    // LF  '\n' (new line)
            0,		    // VT  '\v' (vertical tab)
            0,		    // FF  '\f' (form feed)
            0,		    // CR  '\r' (carriage ret)
            0,		    // SO  (shift out)
            0,		    // SI  (shift in)
            0,		    // DLE (data link escape)
            0,		    // DC1 (device control 1)
            0,		    // DC2 (device control 2)
            0,		    // DC3 (device control 3)
            0,		    // DC4 (device control 4)
            0,		    // NAK (negative ack.)
            0,		    // SYN (synchronous idle)
            0,		    // ETB (end of trans. blk)
            0,		    // CAN (cancel)
            0,		    // EM  (end of medium)
            0,		    // SUB (substitute)
            0,		    // ESC (escape)
            0,		    // FS  (file separator)
            0,		    // GS  (group separator)
            0,		    // RS  (record separator)
            0,		    // US  (unit separator)
            0,		    // SPACE
            mark_m_c,	    // !
            0,		    // "
            mark_m_c,	    // #
            mark_m_c,	    // $
            mark_m_c,	    // %
            mark_m_c,	    // &
            0,		    // '
            0,		    // (
            0,		    // )
            mark_m_c,	    // *
            mark_m_c,	    // +
            0,		    // ,
            mark_m_c,	    // -
            mark_point_c,   // .
            mark_m_c,	    // /
            0,		    // 0
            0,		    // 1
            0,		    // 2
            0,		    // 3
            0,		    // 4
            0,		    // 5
            0,		    // 6
            0,		    // 7
            0,		    // 8
            0,		    // 9
            mark_m_c,	    // :
            0,		    // ;
            mark_m_c,	    // <
            mark_m_c,	    // =
            mark_m_c,	    // >
            mark_m_c,	    // ?
            mark_m_c,	    // @
            0,		    // A
            0,		    // B
            0,		    // C
            0,		    // D
            0,		    // E
            0,		    // F
            0,		    // G
            0,		    // H
            0,		    // I
            0,		    // J
            0,		    // K
            0,		    // L
            0,		    // M
            0,		    // N
            0,		    // O
            0,		    // P
            0,		    // Q
            0,		    // R
            0,		    // S
            0,		    // T
            0,		    // U
            0,		    // V
            0,		    // W
            0,		    // X
            0,		    // Y
            0,		    // Z
            0,		    // [
            mark_backslash_c,
	    		    // \  '\\'
            0,		    // ]
            mark_m_c,	    // ^
            mark_m_c,	    // _
            0,		    // `
            0,		    // a
            0,		    // b
            0,		    // c
            0,		    // d
            0,		    // e
            0,		    // f
            0,		    // g
            0,		    // h
            0,		    // i
            0,		    // j
            0,		    // k
            0,		    // l
            0,		    // m
            0,		    // n
            0,		    // o
            0,		    // p
            0,		    // q
            0,		    // r
            0,		    // s
            0,		    // t
            0,		    // u
            0,		    // v
            0,		    // w
            0,		    // x
            0,		    // y
            0,		    // z
            0,		    // {
            mark_m_c,	    // |
            0,		    // }
            mark_m_c,	    // ~
            0	    	    // DEL
        };
    uns32 mark_tmap =
        create_type_map ( 0, 127, mark_cmap );
    ATTACH ( mark_dispatcher1, mark_tmap );

    uns32 mark_dispatcher2_point =
        create_dispatcher ( 3, 2 );
    ATTACH ( mark_dispatcher2_point,
             digit_tmap );
    ATTACH ( mark_dispatcher1,
             mark_dispatcher2_point,
	     mark_point_c );

    uns32 mark_dispatcher2_backslash =
        create_dispatcher ( 3, 2 );
    ATTACH ( mark_dispatcher2_backslash,
             Uu_tmap );
    ATTACH ( mark_dispatcher1,
             mark_dispatcher2_backslash,
	     mark_backslash_c );

    // begin natural number atom table;
    //
    //    "<digit>" accept;
    //    ".<digit>" keep 1 goto number;
    //    "<digit>/<digit>" keep 2 goto number;
    //    "<digit>,<digit>" keep 2 goto number;
    //    return;
    //
    // end natural number atom table;

    uns32 natural_number_dispatcher1 =
        create_dispatcher ( 2, 2 );
    ATTACH ( natural_number,
             natural_number_dispatcher1 );
    const uns32 number_digit_c      = 1;
    const uns32 number_point_c      = 2;
    uns8 number_cmap[128] =
        {
            0,              // NUL '\0'
            0,		    // SOH (start of heading)
            0,		    // STX (start of text)
            0,		    // ETX (end of text)
            0,		    // EOT (end of transmission)
            0,		    // ENQ (enquiry)
            0,		    // ACK (acknowledge)
            0,		    // BEL '\a' (bell)
            0,		    // BS  '\b' (backspace)
            0,		    // HT  '\t' (horizontal tab)
            0,		    // LF  '\n' (new line)
            0,		    // VT  '\v' (vertical tab)
            0,		    // FF  '\f' (form feed)
            0,		    // CR  '\r' (carriage ret)
            0,		    // SO  (shift out)
            0,		    // SI  (shift in)
            0,		    // DLE (data link escape)
            0,		    // DC1 (device control 1)
            0,		    // DC2 (device control 2)
            0,		    // DC3 (device control 3)
            0,		    // DC4 (device control 4)
            0,		    // NAK (negative ack.)
            0,		    // SYN (synchronous idle)
            0,		    // ETB (end of trans. blk)
            0,		    // CAN (cancel)
            0,		    // EM  (end of medium)
            0,		    // SUB (substitute)
            0,		    // ESC (escape)
            0,		    // FS  (file separator)
            0,		    // GS  (group separator)
            0,		    // RS  (record separator)
            0,		    // US  (unit separator)
            0,		    // SPACE
            0,		    // !
            0,		    // "
            0,		    // #
            0,		    // $
            0,		    // %
            0,		    // &
            0,		    // '
            0,		    // (
            0,		    // )
            0,		    // *
            0,		    // +
            0,		    // ,
            0,		    // -
            number_point_c, // .
            0,		    // /
            number_digit_c, // 0
            number_digit_c, // 1
            number_digit_c, // 2
            number_digit_c, // 3
            number_digit_c, // 4
            number_digit_c, // 5
            number_digit_c, // 6
            number_digit_c, // 7
            number_digit_c, // 8
            number_digit_c, // 9
            0,		    // :
            0,		    // ;
            0,		    // <
            0,		    // =
            0,		    // >
            0,		    // ?
            0,		    // @
            0,		    // A
            0,		    // B
            0,		    // C
            0,		    // D
            0,		    // E
            0,		    // F
            0,		    // G
            0,		    // H
            0,		    // I
            0,		    // J
            0,		    // K
            0,		    // L
            0,		    // M
            0,		    // N
            0,		    // O
            0,		    // P
            0,		    // Q
            0,		    // R
            0,		    // S
            0,		    // T
            0,		    // U
            0,		    // V
            0,		    // W
            0,		    // X
            0,		    // Y
            0,		    // Z
            0,		    // [
            0,		    // \  '\\'
            0,		    // ]
            0,		    // ^
            0,		    // _
            0,		    // `
            0,		    // a
            0,		    // b
            0,		    // c
            0,		    // d
            0,		    // e
            0,		    // f
            0,		    // g
            0,		    // h
            0,		    // i
            0,		    // j
            0,		    // k
            0,		    // l
            0,		    // m
            0,		    // n
            0,		    // o
            0,		    // p
            0,		    // q
            0,		    // r
            0,		    // s
            0,		    // t
            0,		    // u
            0,		    // v
            0,		    // w
            0,		    // x
            0,		    // y
            0,		    // z
            0,		    // {
            0,		    // |
            0,		    // }
            0,		    // ~
            0	    	    // DEL
        };
    uns32 number_tmap =
        create_type_map ( 0, 127, number_cmap );
    ATTACH ( number_dispatcher1, number_tmap );

    uns32 natural_number_dispatcher2_point =
        create_dispatcher ( 3, 2 );
    ATTACH ( natural_number_dispatcher2_point,
             digit_tmap );
    ATTACH ( natural_number_dispatcher1,
             natural_number_dispatcher2_point,
	     natural_number_point_c );

    uns32 natural_number_dispatcher2 =
        create_dispatcher ( 2, 2 );
    ATTACH ( natural_number_dispatcher1,
             natural_number_dispatcher2,
	     number_digit_c );
    uns8 slash_comma_cmap[128] =
        {
            0,              // NUL '\0'
            0,		    // SOH (start of heading)
            0,		    // STX (start of text)
            0,		    // ETX (end of text)
            0,		    // EOT (end of transmission)
            0,		    // ENQ (enquiry)
            0,		    // ACK (acknowledge)
            0,		    // BEL '\a' (bell)
            0,		    // BS  '\b' (backspace)
            0,		    // HT  '\t' (horizontal tab)
            0,		    // LF  '\n' (new line)
            0,		    // VT  '\v' (vertical tab)
            0,		    // FF  '\f' (form feed)
            0,		    // CR  '\r' (carriage ret)
            0,		    // SO  (shift out)
            0,		    // SI  (shift in)
            0,		    // DLE (data link escape)
            0,		    // DC1 (device control 1)
            0,		    // DC2 (device control 2)
            0,		    // DC3 (device control 3)
            0,		    // DC4 (device control 4)
            0,		    // NAK (negative ack.)
            0,		    // SYN (synchronous idle)
            0,		    // ETB (end of trans. blk)
            0,		    // CAN (cancel)
            0,		    // EM  (end of medium)
            0,		    // SUB (substitute)
            0,		    // ESC (escape)
            0,		    // FS  (file separator)
            0,		    // GS  (group separator)
            0,		    // RS  (record separator)
            0,		    // US  (unit separator)
            0,		    // SPACE
            0,		    // !
            0,		    // "
            0,		    // #
            0,		    // $
            0,		    // %
            0,		    // &
            0,		    // '
            0,		    // (
            0,		    // )
            0,		    // *
            0,		    // +
            1,		    // ,
            0,		    // -
            0,		    // .
            1,		    // /
            0,		    // 0
            0,		    // 1
            0,		    // 2
            0,		    // 3
            0,		    // 4
            0,		    // 5
            0,		    // 6
            0,		    // 7
            0,		    // 8
            0,		    // 9
            0,		    // :
            0,		    // ;
            0,		    // <
            0,		    // =
            0,		    // >
            0,		    // ?
            0,		    // @
            0,		    // A
            0,		    // B
            0,		    // C
            0,		    // D
            0,		    // E
            0,		    // F
            0,		    // G
            0,		    // H
            0,		    // I
            0,		    // J
            0,		    // K
            0,		    // L
            0,		    // M
            0,		    // N
            0,		    // O
            0,		    // P
            0,		    // Q
            0,		    // R
            0,		    // S
            0,		    // T
            0,		    // U
            0,		    // V
            0,		    // W
            0,		    // X
            0,		    // Y
            0,		    // Z
            0,		    // [
            0,		    // \  '\\'
            0,		    // ]
            0,		    // ^
            0,		    // _
            0,		    // `
            0,		    // a
            0,		    // b
            0,		    // c
            0,		    // d
            0,		    // e
            0,		    // f
            0,		    // g
            0,		    // h
            0,		    // i
            0,		    // j
            0,		    // k
            0,		    // l
            0,		    // m
            0,		    // n
            0,		    // o
            0,		    // p
            0,		    // q
            0,		    // r
            0,		    // s
            0,		    // t
            0,		    // u
            0,		    // v
            0,		    // w
            0,		    // x
            0,		    // y
            0,		    // z
            0,		    // {
            0,		    // |
            0,		    // }
            0,		    // ~
            0	    	    // DEL
        };
    uns32 slash_comma_tmap =
        create_type_map ( 0, 127, slash_comma_cmap );
    ATTACH ( number_dispatcher2, slash_comma_tmap );

    uns32 natural_number_dispatcher3 =
        create_dispatcher ( 3, 2 );
    ATTACH ( natural_number_dispatcher3,
             digit_tmap );
    ATTACH ( natural_number_dispatcher1,
             natural_number_dispatcher3,
	     1 );

    // begin number atom table;
    //
    //    // In order to recognize , and / surrounded by digits
    //    // as number atoms, entries to this table upon
    //    // recognizing "X<digit>" must do a `keep 1' so the
    //    // digit will be left to be recognized by
    //    // <digit>/<digit> or <digit>,<digit>.
    //
    //    // Alternatively we could have a separate table
    //    // for the state where the last atom ended with
    //    // a digit.
    //
    //    "<digit>" accept;
    //    ".<digit>" keep 1;
    //    "<digit>/<digit>" keep 2;
    //    "<digit>,<digit>" keep 2;
    //    return;
    //
    // end number atom table;

    uns32 number_dispatcher1 =
        create_dispatcher ( 2, 2 );
    ATTACH ( number,
             number_dispatcher1 );
    ATTACH ( number_dispatcher1, number_tmap );

    uns32 number_dispatcher2_point =
        create_dispatcher ( 3, 2 );
    ATTACH ( number_dispatcher2_point,
             digit_tmap );
    ATTACH ( number_dispatcher1,
             number_dispatcher2_point,
	     number_point_c );

    uns32 number_dispatcher2 =
        create_dispatcher ( 2, 2 );
    ATTACH ( number_dispatcher1,
             number_dispatcher2,
	     number_digit_c );
    ATTACH ( number_dispatcher2, slash_comma_tmap );

    uns32 number_dispatcher3 =
        create_dispatcher ( 3, 2 );
    ATTACH ( number_dispatcher3,
             digit_tmap );
    ATTACH ( number_dispatcher1,
             number_dispatcher3,
	     1 );

    // begin quoted string atom table;
    //
    //     "\"" translate "" return;   // End quoted string.
    //
    //     "\\\"" translate "\"";
    //     "\\\n" translate "\n";
    //     "\\\r" translate "\r";
    //     "\\\t" translate "\t";
    //     "\\\b" translate "\b";
    //     "\\\f" translate "\f";
    //     "\\\v" translate "\v";
    //     "\\\\" translate "\\";
    //     "\\~"  translate " " ;
    //     "\\x<hex-digit><hex-digit>" translate hex 2 0;
    //     "\\<oct-digit><oct-digit><oct-digit>" translate oct 1 0;
    //     "\\u<hex-digit><hex-digit><hex-digit><hex-digit>"
    //                                 translate hex 2 0;
    //     "\\U<hex-digit><hex-digit><hex-digit><hex-digit>"
    //        "<hex-digit><hex-digit><hex-digit><hex-digit>"
    //                                 translate hex 2 0;
    //
    //     "<line-break-char>"
    //         goto bad end of line;
    //     "\\<other>" error bad escape sequence;
    //
    //     "<other>" accept;
    //
    //     goto bad end of file;
    //
    // end quoted string atom table;


    // begin bad end of line;
    //     mode master;
    //     output bad end of line
    //     	   goto master;
    // end bad end of line;


    // begin bad end of file;
    //     mode master;
    //     output bad end of file
    //     	   goto master;
    // end bad end of file;


    // end standard lexical program;
