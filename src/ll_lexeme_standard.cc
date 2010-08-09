    //// begin standard lexical program;
    //// 
    NDL::begin_program();

    uns32 master_atom_table =
        NDL::new_atom_table ( MASTER );
    uns32 word_atom_table =
        NDL::new_atom_table ( word_t );
    uns32 natural_number_atom_table =
        NDL::new_atom_table ( natural_number_t );
    uns32 mark_atom_table =
        NDL::new_atom_table ( mark_t );
    uns32 quoted_string_atom_table =
        NDL::new_atom_table ( quoted_string_t );
    uns32 comment_atom_table =
        NDL::new_atom_table ( comment_t );
    uns32 horizontal_space_atom_table =
        NDL::new_atom_table ( horizontal_space_t );
    uns32 line_break_atom_table =
        NDL::new_atom_table ( line_break_t );
    uns32 end_of_file_atom_table =
        NDL::new_atom_table ( end_of_file_t );
    uns32 bad_end_of_line_atom_table =
        NDL::new_atom_table ( bad_end_of_line_t );
    uns32 bad_end_of_file_atom_table =
        NDL::new_atom_table ( bad_end_of_file_t );
    uns32 bad_character_atom_table =
        NDL::new_atom_table ( bad_character_t );
    uns32 bad_escape_sequence_atom_table =
        NDL::new_atom_table ( bad_escape_sequence_t );

    //// "<ascii-letter>" = "a-z" | "A-Z";
    //
    static const char * cp_ascii_letter =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    //// "<digit>" = "0-9";
    //
    static const char * cp_digit = "0123456789";

    //// "<oct-digit>" = "0-7";
    //
    static const char * cp_oct_digit = "01234567";

    //// "<hex-digit>" = "0-9" | "a-f" | "A-F";
    //// 
    static const char * cp_hex_digit =
        "0123456789abcdefABCDEF";

    //// "<non-digit>" = ~ "<digit>";
    //// 
    //// // The following list of language specific letters is taken
    //// // from Annex E of the Working Paper for Draft Proposed
    //// // International Standard for Information Systems—Programming
    //// // Language C++, 1996.
    //// 
    //// "<latin-letter>" = "\u00c0-\n00d6" | "\u00d8-\u00f6"
    ////                  | "\u00f8-\u01f5" | "\u01fa-\u0217"
    ////                  | "\u0250-\u02a8" | "\u1ea0-\u1ef9";
    //// 
    //// . . . . . letter character pattern definitions omitted . . . . .
    //// 
    //// "<CJK-letter>" = "\uf900-\ufa2d" | . . .  // Details omitted
    //// 
    //// // Context sensitive character classifications, e.g.,
    //// // classifying ' as a word character if it is followed by
    //// // a letter, cannot be included in character pattern
    //// // definitions, but are noted in comments.  They are
    //// // accounted for by separate atom table entries below.
    //// 
    //// "<letter>" = "<ascii-letter>" | "<latin-letter>"
    ////            | . . . | "<CJK-letter>";   // Details omitted
    //// 
    void add_non_ascii_letters ( void );

    //// "<non-letter>" = ~ "<letter>";
    //// 
    //// "<mark-char>" = "+" | "-" | "*" | "~" | "@" | "#" | "$" | "%"
    ////               | "^" | "&" | "=" | "|" | "<" | ">" | "_" | "!"
    ////               | "?" | ":";
    ////               //
    ////               // Also \ not followed by u or U,
    ////               //      . not followed by a digit
    ////               //      / not surrounded by digits
    ////               //        or part of // comment
    ////	       //        beginning
    //// 
    static const char * cp_mark = "+-*~@#$%^&=|<>_!?:";

    //// "<non-u-char>" = ~ "u" & ~ "U";
    //// "<non-slash-char>" = ~ ";";
    //// 
    //// "<separator-char>" = "(" | ")" | "[" | "]" | "{" | "}" | ";" | "`";
    ////               //
    ////               // Also , not surrounded by digits
    ////               //      ' not followed by a letter
    //// 
    static const char * cp_separator = "()[]{};`";

    //// "<line-break-char>" = "\n" | "\v" | "\f" | "\r";
    //// 
    static const char * cp_line_break = "\n\v\f\r";

    //// "<non-line-break-char>" = ~ "<line-break-character>";
    //// 
    //// "<horizontal-space-char>" = " " | "\t";
    //// 
    static const char * cp_horizontal_space = " \t";

    uns32 letter_pattern;
    NDL::begin_character_pattern ( letter_pattern,
    				   cp_ascii_letter );
        add_non_ascii_letters();
    NDL::end_character:pattern();

    //// begin master atom table;
    //// 
    NDL::begin_atom_table ( master_atom_table );

    ////    // Whitespace is separated out into 3 types
    ////    // of lexeme:
    ////    //
    ////    //   `horizontal space'  spaces and horizon-
    ////    //                       tal tabs
    ////    //   `line break'        linefeeds, carriage
    ////    //                       returns, form
    ////    //                       feeds, and vertical
    ////    //                       tabs
    ////    //   `comments'          `//' up to just
    ////    //                       before next line
    ////    //                       break
    ////    //
    ////    "//" call comment;
    //
    	NDL::begin_dispatch ( "/" );
	    NDL::begin_dispatch ( "/" );
		NDL::call ( comment_atom_table );
	    NDL::end_dispatch();

	    NDL::call ( mark_atom_table );
	NDL::end_dispatch();

    ////    "<horizontal-space-char>" call horizontal space;
    //
    	NDL::begin_dispatch ( cp_horizontal_space );
	    NDL::call ( horizontal_space_atom_table );
	NDL::end_dispatch();

    ////    "<line-break-char>" call line break;
    //// 
    	NDL::begin_dispatch ( cp_line_break );
	    NDL::call ( line_break_atom_table );
	NDL::end_dispatch();

    ////    "<letter>" call word;
    //
    	NDL::begin_dispatch ( cp_ascii_letter );
	    add_non_ascii_letters();
	    NDL::call ( word_atom_table );
	NDL::end_dispatch();

    ////    "'<letter>" call word;
    //
	NDL::begin_dispatch ( "'" );
	    NDL::begin_dispatch ( cp_ascii_letter );
		add_non_ascii_letters();
		NDL::call ( word_atom_table );
	    NDL::end_dispatch();

	    NDL::output ( sep_t );
	NDL::end_dispatch();

    ////     "\\u<hex-digit><hex-digit><hex-digit><hex-digit>"
    ////         translate hex 2 0 "<letter"> call word
    ////         else keep 2 output bad escape sequence;
    ////     "\\U<hex-digit><hex-digit><hex-digit><hex-digit>"
    ////        "<hex-digit><hex-digit><hex-digit><hex-digit>"
    ////         translate hex 2 0 "<letter"> call word
    ////         else keep 2 output bad escape sequence;
    //// 
    ////     "\\u" output bad escape sequence;
    ////     "\\U" output bad escape sequence;
    //// 
	NDL::begin_dispatch ( "\\" );
	  NDL::begin_dispatch ( "u" );
	    NDL::begin_dispatch ( cp_hex_digit );
	    NDL::begin_dispatch ( cp_hex_digit );
	    NDL::begin_dispatch ( cp_hex_digit );
	    NDL::begin_dispatch ( cp_hex_digit );
	      NDL::translate_hex(2,0);
	      NDL::call ( word_atom_table );
	      NDL::else_if_not ( letter_pattern );
	        NDL::keep(2);
	  	NDL::output ( bad_escape_sequence_t );
	    NDL::end_dispatch();
	    NDL::end_dispatch();
	    NDL::end_dispatch();
	    NDL::end_dispatch();

	    NDL::output ( bad_escape_sequence_t );
	  NDL::end_dispatch();

	  NDL::begin_dispatch ( "U" );
	    NDL::begin_dispatch ( cp_hex_digit );
	    NDL::begin_dispatch ( cp_hex_digit );
	    NDL::begin_dispatch ( cp_hex_digit );
	    NDL::begin_dispatch ( cp_hex_digit );
	    NDL::begin_dispatch ( cp_hex_digit );
	    NDL::begin_dispatch ( cp_hex_digit );
	    NDL::begin_dispatch ( cp_hex_digit );
	    NDL::begin_dispatch ( cp_hex_digit );
	      NDL::translate_hex(2,0);
	      NDL::call ( word_atom_table );
	      NDL::else_if_not ( letter_pattern );
	        NDL::keep(2);
	  	NDL::output ( bad_escape_sequence_t );
	    NDL::end_dispatch();
	    NDL::end_dispatch();
	    NDL::end_dispatch();
	    NDL::end_dispatch();
	    NDL::end_dispatch();
	    NDL::end_dispatch();
	    NDL::end_dispatch();
	    NDL::end_dispatch();

	    NDL::output ( bad_escape_sequence_t );
	  NDL::end_dispatch();

	  NDL::call ( mark_atom_table );

	NDL::end_dispatch();

    ////    "<mark-char>" call mark;
    //
    	NDL::begin_dispatch ( cp_mark );
	    NDL::call ( mark_atom_table );
	NDL::end_dispatch();

    ////    "." call mark;  // ".<non-digit>"
    ////    "/" call mark;  // "/<non-slash-char>"
    ////    "\\" call mark; // "\\<non-u-char>"
    //// 
    ////    // We assume that the preceding text is not a digit
    ////    // or the current input is not / or , followed by a digit.
    ////    //
    ////    "<digit>" keep 0 call natural number;
    //
    	NDL::begin_dispatch ( cp_mark );
	    NDL::keep(0);
	    NDL::call ( natural_number_atom_table );
	NDL::end_dispatch();

    ////    ".<digit>" keep 1 call number;
    //// 
    	NDL::begin_dispatch ( "." );
	    NDL::call ( mark_atom_table );
	    NDL::begin_dispatch ( "." );
		NDL::keep(1);
		NDL::call ( number_atom_table );
	    NDL::end_dispatch();
	NDL::end_dispatch();

    ////    "<separator-char>" output separator;
    ////    "," output separator;
    //
    	NDL::begin_dispatch ( cp_separator );
	    NDL::add_character ( "," );
	    NDL::output ( sep_t );
	NDL::end_dispatch();

    ////    "'" output separator;        // "'<non-letter>"
    //// 
    ////    "\"" translate "" call quoted string;
    //// 
    	NDL::begin_dispatch ( "\"" );
	    NDL::translate ( "" );
	    NDL::call ( quoted_string_atom_table );
	NDL::end_dispatch();

    ////    "<other>" output bad character;
    //
    	NDL::begin_dispatch ( OTHER );
	    NDL::output ( bad_character_t );
	NDL::end_dispatch();

    ////    output end of file;
    //// 
        NDL::output ( end_of_file_t );
    //// end master atom table;
    //// 
    NDL::end_atom_table();

    //// 
    //// // The below tables are entered from the master table
    //// // with the first one or two characters scanned.
    //// 
    //// 
    //// begin comment atom table;
    ////    "<non-line-break-char>" accept;
    ////    return;
    //// end comment atom table;
    //// 
    //// 
    //// begin horizontal space atom table;
    ////    "<horizontal-space-char>" accept;
    ////    return;
    //// end horizontal space atom table;
    //// 
    //// 
    //// begin line break atom table;
    ////    "<line-break-char>" accept;
    ////    return;
    //// end line break atom table;
    //// 
    //// 
    //// begin word atom table;
    //// 
    ////    "<word-char>" accept;
    ////    "'<letter>" accept;
    //// 
    ////     "\\u<hex-digit><hex-digit><hex-digit><hex-digit>"
    ////         translate hex 2 0 "<letter">
    ////         else keep 0 return
    ////     "\\U<hex-digit><hex-digit><hex-digit><hex-digit>"
    ////        "<hex-digit><hex-digit><hex-digit><hex-digit>"
    ////         translate hex 2 0 "<letter">
    ////         else keep 0 return
    //// 
    ////    return;
    //// 
    //// end word atom table;
    //// 
    //// begin mark atom table;
    ////    "<mark-char>" accept;
    ////    ".<non-digit>" keep 1;
    ////    "/<non-slash-char>" keep 1;
    ////    "\\<non-u-char>" keep 1;
    ////    return;
    //// end mark atom table;
    //// 
    //// begin natural number atom table;
    //// 
    ////    "<digit>" accept;
    ////    ".<digit>" keep 1 goto number;
    ////    "<digit>/<digit>" keep 2 goto number;
    ////    "<digit>,<digit>" keep 2 goto number;
    ////    return;
    //// 
    //// end natural number atom table;
    //// 
    //// begin number atom table;
    //// 
    ////    // In order to recognize , and / surrounded by digits
    ////    // as number atoms, entries to this table upon
    ////    // recognizing "X<digit>" must do a `keep 1' so the
    ////    // digit will be left to be recognized by
    ////    // <digit>/<digit> or <digit>,<digit>.
    //// 
    ////    // Alternatively we could have a separate table
    ////    // for the state where the last atom ended with
    ////    // a digit.
    //// 
    ////    "<digit>" accept;
    ////    ".<digit>" keep 1;
    ////    "<digit>/<digit>" keep 2;
    ////    "<digit>,<digit>" keep 2;
    ////    return;
    //// 
    //// end number atom table;
    //// 
    //// 
    //// begin quoted string atom table;
    //// 
    ////     "\"" translate "" return;   // End quoted string.
    //// 
    ////     "\\\"" translate "\"";
    ////     "\\\n" translate "\n";
    ////     "\\\r" translate "\r";
    ////     "\\\t" translate "\t";
    ////     "\\\b" translate "\b";
    ////     "\\\f" translate "\f";
    ////     "\\\v" translate "\v";
    ////     "\\\\" translate "\\";
    ////     "\\~"  translate " " ;
    ////     "\\u<hex-digit><hex-digit><hex-digit><hex-digit>"
    ////                                 translate hex 2 0;
    ////     "\\U<hex-digit><hex-digit><hex-digit><hex-digit>"
    ////        "<hex-digit><hex-digit><hex-digit><hex-digit>"
    ////                                 translate hex 2 0;
    ////     "\\<line-break-char>"
    ////         keep 1 error bad escape sequence;
    //// 
    ////     "<line-break-char>"
    ////         goto bad end of line;
    ////     "\\<other>" error bad escape sequence;
    //// 
    ////     "<other>" accept;
    //// 
    ////     goto bad end of file;
    //// 
    //// end quoted string atom table;
    //// 
    //// 
    //// begin bad end of line;
    ////     mode master;
    ////     output bad end of line
    ////            goto master;
    //// end bad end of line;
    //// 
    //// 
    //// begin bad end of file;
    ////     mode master;
    ////     output bad end of file
    ////            goto master;
    //// end bad end of file;
    //// 
    //// 
    //// end standard lexical program;
