// Layers Language Standard Lexical Analyzer
//
// File:	ll_lexeme_standard.cc
// Author:	Bob Walton (walton@seas.harvard.edu)
// Date:	Thu May  6 07:49:01 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/05/06 12:51:48 $
//   $RCSfile: ll_lexeme_standard.cc,v $
//   $Revision: 1.1 $

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
    assert ( letter_type_map_IDs == 0 );

#   define LETTER_RANGE(cmin, cmax) \
        letter_type_map_ID \
	    [letter_type_map_IDs ++] = \
		  create_type_map ( cmin, cmax, 1 );

    LETTER_RANGE ( 'a', 'z' );
    LETTER_RANGE ( 'A', 'Z' );
}

// Attach all the letter_type_maps to the given
// dispatcher.  If not_ascii is true, omit the first
// two letter_type_maps.
//
static void attach_letter_type_maps
	( uns32 dispatcher_ID, bool not_ascii = false )
{
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

void ll::lexeme::standard:;create_standard_program
	( void )
{
    error_count = 0;

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
    //			| "\u00d8-\u00f6"
    //                  | "\u00f8-\u01f5"
    //			| "\u01fa-\u0217"
    //                  | "\u0250-\u02a8"
    //			| "\u1ea0-\u1ef9";
    //
    // . . . . . letter character pattern definitions
    //           omitted . . . . .
    //
    // "<CJK-letter>" = "\uf900-\ufa2d"
    //		      | . . .  // Details omitted
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
    //    "//" goto comment;
    //    "<horizontal-space-char>" goto horizontal space;
    //    "<line-break-char>" goto line break;
    //
    //    "<word-char>" goto word;
    //    "'<letter>" goto word;
    //     "\\u<hex-digit><hex-digit>"
    //        "<hex-digit><hex-digit>"
    //         translate hex 2 0 "<letter"> goto word
    //         else keep 1 output bad escape sequences;
    //     "\\U<hex-digit><hex-digit>"
    //        "<hex-digit><hex-digit>"
    //        "<hex-digit><hex-digit>"
    //        "<hex-digit><hex-digit>"
    //         translate hex 2 0 "<letter"> goto word
    //         else keep 1 output bad escape sequences;
    //
    //    "<mark-char>" goto mark;
    //    ".<non-digit>" keep 1 goto mark;
    //    "/" goto mark;
    //    "\\<non-u-char>" keep 1 goto mark;
    //
    //    // We assume that the preceding text is not a
    //    // digit or the current input is not / or ,
    //    // followed by a digit.
    //    //
    //    "<digit>" keep 0 goto number;
    //    ".<digit>" keep 1 goto number;
    //
    //    "<separator-char>" output separator;
    //    "," output separator;
    //    "'<non-letter>" keep 1 output separator;
    //
    //    "\"" translate "" goto quoted string;
    //
    //    "<other>" output bad character;
    //    output end of file;
    //
    // end master atom table;

    uns32 master = create_atom_table ( MASTER );
    uns32 master_dispatcher1 =
        create_dispatcher ( 500, 20 );
    uns32 attach ( master, master_dispatcher1 );

    // Type map for master_dispatcher_1 first 128
    // characters.
    //
    const uns32 letter_c     = 1;  // Must be 1.
    const uns32 digit_c      = 2;
    const uns32 mark_c       = 3;
    const uns32 sep_c        = 4;
    const uns32 line_break_c = 5;
    const uns32 h_space_c    = 6;
    const uns32 comma_c      = 7;
    const uns32 point_c      = 8;
    const uns32 slash_c      = 9;
    const uns32 backslash_c  = 10;
    const uns32 single_q_c   = 11;
    const uns32 double_q_c   = 12;

    uns8 master_cmap1[128] =
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
            comma_c,	    // ,
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
        LEX::create_type_map ( 0, 127, master_cmap1 );
    uns32 attach ( master_dispatcher1, master_tmap1 );

    // Type map for


    // The below tables are entered from the master table
    // with the first one or two characters scanned.


    // begin comment atom table;
    //    "<non-line-break-char>" accept;
    //    goto master;
    // end comment atom table;


    // begin horizontal space atom table;
    //    "<horizontal-space-char>" accept;
    //    goto master;
    // end horizontal space atom table;


    // begin line break atom table;
    //    "<line-break-char>" accept;
    //    goto master;
    // end line break atom table;


    // begin word atom table;
    //    "<word-char>" accept;
    //    "'<letter>" accept;
    //     "\\u<hex-digit><hex-digit><hex-digit><hex-digit>"
    //         translate hex 2 0 "<letter">
    //         else keep 0 goto master
    //    "\\U<hex-digit><hex-digit><hex-digit><hex-digit>"
    //       "<hex-digit><hex-digit><hex-digit><hex-digit>"
    //        translate hex 2 0 "<letter">
    //         else keep 0 goto master
    //   goto master;
    // end word atom table;

    // begin mark atom table;
    //    "<mark-char>" accept;
    //    ".<non-digit>" keep 1;
    //    "/" accept;
    //    "\\<non-u-char>" keep 1;
    //    goto master;
    // end mark atom table;


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
    //    goto master;
    //
    // end number atom table;


    // begin quoted string atom table;
    //
    //     "\"" translate "" goto master;   // End quoted string.
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
    //     "\\" error bad escape sequence;
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
