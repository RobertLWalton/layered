// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Jan 21 05:27:34 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Data
//	Program Construction
//	Scanner Closures
//	Scanner
//	Input Files
//	Printing

// Usage and Setup
// ----- --- -----

# ifndef LL_LEXEME_H
# define LL_LEXEME_H

# include <min.h>
# include <iostream>
# include <cassert>

// Data
// ----

namespace ll { namespace lexeme {

    // Unsigned integer 8-bit, 32-bit, and 64-bit types.
    //
    using min::uns8;
    using min::uns32;
    using min::uns64;

    const min::stub * const NULL_STUB = min::NULL_STUB;

    // Characters are stored in uns32 integers.
    // This is more than sufficient for UNICODE.

    typedef min::packed_vec_insptr<uns32> program_ptr;
        // Type of a pointer to a program.

    // Print modes.  These determine how a character c
    // in a line is rendered when the line is printed.
    // This in turn determines the `width' of c in the
    // printed line, i.e., the number of columns
    // occupied by the rendering of c.  For example,
    // character 0x01 is rendered as itself and has
    // width 0 in UTF8 mode, is rendered as a UNICODE
    // control picture character of width 1 in UTF8PRINT
    // or UTF8GRAPHIC mode, and is rendered as \01/ and
    // has width 4 in ASCIIGRAPHIC mode.
    //
    // DEFAULT_PRINT_MODE:
    //   The print mode is the default, which is the
    //   print mode of any scanner or parser pass asso-
    //   ciated with the print operation, or is
    //   ll::lexeme::default_print_mode otherwise.
    // UTF8:
    //   Character c is rendered as itself (i.e., is
    //   converted to UTF8).
    // UTF8PRINT:
    //   Character c is rendered as itself, unless the
    //   character is a control character other than
    //   new line, carriage return, form feed, vertical
    //   tab, or horizontal tab, in which case c is
    //   rendered as per UTF8GRAPHIC.
    // UTF8GRAPHIC:
    //   Character c is rendered as itself if it is a
    //   graphic character, and otherwise is rendered
    //   to a UNICODE control code picture character
    //   (0x24yy character).  Characters from 0x00 to
    //   0x1f, single space, and DELETE are the `non-
    //   graphic' characters.
    // ASCIIPRINT:
    //   Character c is rendered as itself, unless the
    //   character is a control character other than
    //   new line, carriage return, form feed, vertical
    //   tab, or horizontal tab, OR c is not an ASCII
    //   character (has code >= 0x80), in which case c
    //   is rendered as per ASCIIGRAPHIC.
    // ASCIIGRAPHIC:
    //   If the uns32 character c is ' ', '\\', '\n',
    //   '\f', '\t', '\v', '\b', or '\r' it is rendered
    //   as "\~/", "\\/", "\lf/", etc.  Otherwise if c
    //   is not an ASCII graphic character (is NOT in
    //   the range 0x21 ... 0x7e), it is rendered as
    //   "\0X...X/", where X...X are hexadecimal digits
    //   representing the uns32 value c.
    //
    // Note: ASCII characters in the range 0x21 .. 0x7E
    // except \ are always rendered as themselves and
    // are always 1 column wide.  This can be used in
    // optimizations.
    //
    enum {
	DEFAULT_PRINT_MODE = 0,
        UTF8 = 1,
        UTF8PRINT = 2,
	UTF8GRAPHIC = 3,
	ASCIIPRINT = 4,
	ASCIIGRAPHIC = 5 };
    //
    extern uns32 default_print_mode;

    // Maximum number of bytes required to represent a
    // UNICODE character in any of the above print
    // modes.
    //
    const unsigned MAX_UNICODE_BYTES = 12;

    struct position
        // Position of an element of the input_buffer:
	// see below.
    {
	uns32	line;
	uns32	index;
	uns32	column;
    };

    struct inchar : public position
        // Element of input_buffer: see below.
    {
	uns32	character;
    };

    struct file_struct;
    typedef min::packed_vec_insptr<uns32,file_struct>
            file_ptr;
        // See Input Files below.

    struct scanner_struct;
    typedef min::packed_struct_updptr<scanner_struct>
            scanner_ptr;
	// See scanner_struct below.

    struct input_struct;
    typedef min::packed_struct_updptr<input_struct>
            input_ptr;
	// See input_struct below.

    struct erroneous_struct;
    typedef min::packed_struct_updptr<erroneous_struct>
            erroneous_ptr;
	// See erroneous_struct below.

    const uns32 return_stack_size = 64;
    struct scanner_struct
    {
        const uns32 control;
	    // Packed structure control word.

	// The program is a sequence of program
	// components.  Defaults to NULL_STUB.
	//
	program_ptr program;

	// The input buffer is a vector of inchar
	// elements each holding a character and the
	// location of that character in the input text.
	// The location, consisting of a line, an index,
	// and a column, is not used by the scanner.
	// Created by init_scanner.
	//
	min::packed_vec_insptr<inchar> input_buffer;

	// The line, index, and column of the character
	// that will be put next at the end of the input
	// buffer.  May be used to delimit the position
	// just after the last character that is to be
	// put into the input buffer, i.e., the position
	// of the end of file.
	//
	position next_position;

	// The translation buffer holds the translation
	// of the current lexeme.  For example, if the
	// lexeme is a quoted string lexeme, the quotes
	// may be removed from the translation, and
	// special character representation sequences
	// may be replaced in the translation by the
	// represented characters.  Created by init_
	// scanner.
	//
	min::packed_vec_insptr<uns32>
	    translation_buffer;

	// The scanner parameters are
	//
	//	print_mode
	//	line_length
	//	indent
	//	read_input
	//	input_file
	//	err
	//	scan_trace_out
	//	erroneous_atom
	//
	// These are set to defaults when a scanner is
	// created by init_scanner().

	// Print mode.  One of UTF8, UTF8PRINT, UTF8-
	// GRAPHIC, or ASCIIPRINT, or ASCIIGRAPHIC.
	// See above and also see default read_input
	// function below.
	//
	uns32 print_mode;   // Default:
			    //   ll::lexeme
			    //     ::default_print_mode

	// Nominal length and indent for printing.  Line
	// length defaults to the limit for email mes-
	// sages.
	//
	uns32 line_length;  // Default 72
	uns32 indent;       // Default 4

	// Closure to call to input one or more inchar
	// elements to the end of the input buffer
	// vector, increasing the length of the buffer
	// as neccessary.  Return true if this is done,
	// and false if there are no more characters
	// because we are at the end of file.  Initial-
	// ized to the default value described immedia-
	// tely below.
	//
	input_ptr read_input;

	// ll::lexeme::default_read_input, the default
	// value of read_input, reads UTF-8 lines from
	// the input_file and assigns each UNICODE
	// character a line, index, and column number
	// as follows:
	//
	//   line   input_file->line_number - 1 after
	//	    calling next_line(input_file).
	//
	//   index  Set to 0 at beginning of line, i.e.,
	//	    when `\n' is added to the input
	//	    buffer; otherwise incremented by the
	//	    number of UTF-8 bytes that encode
	//	    the character added to the input
	//	    buffer.
	//
	//   column Set to 0 at the beginning of a line,
	//	    i.e., when `\n' is added to the
	//	    input buffer; otherwise incremented
	//	    by
	//
	//		wchar ( c, print_mode )
	//
	//	    where c is the UNICODE character
	//	    added to the input buffer, unless
	//	    wchar returns 0 and c == `\t', in
	//	    which case the column is set to the
	//	    next multiple of 8.
	//
	// Input_file should be set using init_file.
	// Its default value is set to input from cin
	// with file name "standard input", and spool_
	// length == 0.
	//
	file_ptr input_file;

	// Output stream for default_erroneous.
	// Defaults to & std::cerr.
	//
	std::ostream * err;

	// Output stream for tracing the scan.  If NULL,
	// there is no tracing.  Defaults to NULL.
	//
	std::ostream * scan_trace_out;

	// Closure to call with an error atom as per
	// ERRONEOUS_ATOM instruction flag.  The atom is
	// in
	//
	//	input_buffer[first .. last]
	//
	// and the instruction provided type is given as
	// an argument.  If the address of this function
	// is NULL, execution of an instruction with an
	// ERRONEOUS_ATOM flag is a scan error.
	// Defaults to NULL_STUB.
	//
	erroneous_ptr erroneous_atom;

	// Scanner state:

	// Error message describing the last error.  Can
	// be reset to "" indicating there is no error
	// by error_message[0] = 0.  Users should not
	// write into this otherwise.
	//
	// The error message is a sequence of lines each
	// no longer than line_length characters.  All
	// lines but the last are `\n' terminated: it
	// may be printed with cout << error_message
	// << endl.
	//
	// Program construction error messages may be
	// used in conjunction with the output of print_
	// program ( false ) (uncooked program print).
	//
	char error_message [2000];

	bool reinitialize;
	    // Set to true if scanner is to be
	    // reinitialized on the next call to scan.
	uns32 next;
	    // input_buffer[next] is the first character
	    // of the first yet unscanned atom.
	uns32 current_table_ID;
	    // Current table ID.
	uns32 return_stack[return_stack_size];
	uns32 return_stack_p;
	    // Return stack containing return_stack_p
	    // elements (0 is first and return_stack_p
	    // - 1 element is top).

	// Work areas:

	char work[400];
	    // Working buffer for producing components
	    // of error_message.
    };

    extern scanner_ptr & default_scanner;
        // Default scanner.  This variable is locatable
	// by the garbage collector.

} }

// Program Construction
// ------- ------------

namespace ll { namespace lexeme {

    // The program being constructed is
    //
    //		scanner->program
    //
    // which defaults to
    //
    //		LEX::default_scanner->program.
    //
    // The scanner also provides scanner->error_message
    // as a place to put error messages.
    //
    // The scanner must be initialized with init_scanner
    // before constructing a program.  After construc-
    // ting a program, init_scanner must be recalled
    // before using the scanner to scan lexemes.

    // Program component types:
    //
    enum {
	PROGRAM			= 1,
	TABLE			= 2,
	TYPE_MAP			= 3,
	DISPATCHER			= 4,
	INSTRUCTION			= 5
    };

    // Return the type of the component at the given
    // ID.
    //
    inline uns32 component_type
	    ( uns32 ID,
	      scanner_ptr scanner = default_scanner )
    {
        return scanner->program[ID];
    }

    // Create a new program.
    //
    // This function resets scanner->program to 0 length
    // and then adds a program header to its beginning.
    // Subsequent functions add more program components
    // to the end of the scanner->program.  If scanner->
    // program does not exist (it equals NULL_STUB), it
    // is created.
    //
    // The header contains a map of lexeme types to type
    // names that is used for printouts.  Lexeme type
    // t maps to name type_name[t].  This may be NULL
    // if t is not used.
    //
    // If type_name is NULL, no type names are given.
    // If max_type != 0, all lexeme types are required
    // to be <= max_type.
    //
    void create_program
	    ( const char * const * type_name = NULL,
	      uns32 max_type = 0,
	      scanner_ptr scanner = default_scanner );

    // Table modes and return values.
    //
    // A mode is defined to be either the kind MASTER
    // or ATOM or the type of a lexeme table.
    //
    enum {

	// Return values that are not types or kinds.
	//
	SCAN_ERROR	= 0xFFFFFFFF,

	// Kinds that are not types or return values.
	//
        MASTER		= 0xFFFFFFFE,
	ATOM		= 0xFFFFFFFD
    };

    // Create the table with the given mode and return
    // its ID.  The mode may be MASTER or ATOM or the
    // type of a lexeme table.
    //
    // The first MASTER table created after a create_
    // program becomes the initial table of the program.
    //
    uns32 create_table
	    ( uns32 mode,
	      scanner_ptr scanner = default_scanner );

    // Return the mode of a table with the given ID.
    //
    uns32 table_mode
	    ( uns32 ID,
	      scanner_ptr scanner = default_scanner );

    // Create a dispatcher with given maximum number of
    // breakpoints and maximum ctype.  Return the new
    // dispatcher's ID.  Note that legal ctypes are 0 ..
    // max_ctype, and 0 is the default ctype for any
    // character not mapped by a type map.
    //
    uns32 create_dispatcher
	    ( uns32 max_breakpointers,
	      uns32 max_ctype,
	      scanner_ptr scanner = default_scanner );

    // Create a type map for characters in the range
    // cmin .. cmax.  Return the type map ID.  Copy
    // map[0..(cmax-cmin)] into type map, so the map
    // will give character c the ctype map[c-cmin].
    //
    uns32 create_type_map
	    ( uns32 cmin, uns32 cmax,
	      uns8 * map,
	      scanner_ptr scanner = default_scanner );

    // Create a type map for characters in the range
    // cmin .. cmax.  Return the type map ID.  This
    // form of type map will map all characters in the
    // range to the given ctype, which must not be 0.
    //
    uns32 create_type_map
	    ( uns32 cmin, uns32 cmax,
	      uns32 ctype,
	      scanner_ptr scanner = default_scanner );

    // An instruction consists of an uns32 operation,
    // various optional IDs, and an uns32 * translation_
    // vector for the TRANSLATE_TO flag.
    // 
    // The operation is the sum of some of the follow-
    // ing:
    //
    //   ACCEPT		Equals 0; use if operation has
    //			no other components.
    //
    //	 MATCH		Invoke the atom table specified
    //			by the atom_table_ID.  This may
    //			succeed or fail.  If it suc-
    //			ceeds, it replaces the matched
    //			atom and provides a translation.
    //			The matched atom can be shorten-
    //			ed by KEEP and the translation
    //			can be overriden by TRANSLATE_
    //			{TO/HEX/OCT}.
    //
    //   KEEP(n):	Truncate atom to n uns32 char-
    //			acters.  The discarded end of
    //			the atom is retained as input to
    //			be rescanned.  0 <= n < 32.
    //
    //	 TRANSLATE_TO(n)
    //			Instead of copying the atom into
    //			the translation buffer, copy the
    //			characters given in the transla-
    //			tion_vector instead.  This vec-
    //			tor has n characters, where n
    //			may be 0 (if n is 0, the trans-
    //			lation vector address should be
    //			NULL).  0 <= n < 32.
    //
    //   TRANSLATE_HEX(prefix,postfix)
    //   TRANSLATE_OCT(prefix,postfix)
    //			Instead of copying the atom into
    //   		the translation buffer, convert
    //		        some of the atom characters from
    //			a hexadecimal or octal number
    //			representation to an uns32 value
    //			equal to one UNICODE character,
    //			and put that character into the
    //			translation buffer.
    //
    //			If p points at the atom, the
    //			characters converted are
    //			p[begin .. end] where
    //			    begin = prefix
    //			    end = atom length
    //				- postfix - 1
    //
    //			0 <= prefix,postfix < 32.  
    //
    //			If conversion fails because one
    //			of the digits is not hexa-deci-
    //			mal or octal, the instruction
    //			fails.
    //
    //   REQUIRE	Require the atom translation to
    //			match the atom pattern defined
    //			by the require_dispatcher_ID.
    //			The match is done the same way
    //			as matching for lexical tables,
    //			with success being defined as
    //			getting to a point with no
    //			further dispatcher table, and
    //			failure as failing match in
    //			one of the dispatcher tables.
    //			If the match fails, the instruc-
    //			tion fails.
    //
    //   ERRONEOUS_ATOM	Indicates the current atom is
    //			erroneous and is to be delivered
    //			to the erroneous_atom function
    //			with the erroneous_atom_type.
    //
    //   OUTPUT		After finishing atom processing
    //			and translation, output the
    //			current lexeme with the output_
    //			type.  The next lexical table
    //			must be a master table.  The
    //			lexeme may be zero length if the
    //			instruction is in the default
    //			instruction group of a table.
    //
    //   GOTO		After all other atom processing,
    //			switch the current table to the
    //			goto_table_ID table, which must
    //			be a master or lexeme table.
    //
    //   CALL		Ditto but also push the current
    //			current table ID into the return
    //			stack and switch to the call_
    //			table_ID table.  The target of
    //			the CALL (unlike GOTO) must be a
    //			lexeme table.
    //
    //			If GOTO and CALL are BOTH in the
    //			instruction, the goto_table_ID
    //			is pushed into the return stack
    //			in place of the current table
    //			ID.
    //
    //	 RETURN		Like GOTO but gets the new table
    //			ID by popping the return stack.
    //			CANNOT be used with CALL or
    //			GOTO.
    //
    //	 FAIL		Used in a translation table to
    //			indicate the table failed to
    //			find an atom.
    //
    //	 ELSE		If the instruction fails, then
    //			execute the instruction at the
    //			else_instruction_ID.
    //

    // Instruction operation flags:
    //
    enum {
	ACCEPT			= 0,
	MATCH			= ( 1 << 0 ),
	KEEP_FLAG		= ( 1 << 1 ),
	TRANSLATE_TO_FLAG	= ( 1 << 2 ),
	TRANSLATE_HEX_FLAG	= ( 1 << 3 ),
	TRANSLATE_OCT_FLAG	= ( 1 << 4 ),
	REQUIRE			= ( 1 << 5 ),
	ERRONEOUS_ATOM		= ( 1 << 6 ),
	OUTPUT			= ( 1 << 7 ),
	GOTO			= ( 1 << 8 ),
	CALL			= ( 1 << 9 ),
	RETURN			= ( 1 << 10 ),
	FAIL			= ( 1 << 11 ),
	ELSE			= ( 1 << 12 ),
    };

    // Instruction shifts and masks
    //
    const uns32 KEEP_LENGTH_SHIFT = 16;
    const uns32 KEEP_LENGTH_MASK = 0x3F;
    const uns32 TRANSLATE_TO_LENGTH_SHIFT = 22;
    const uns32 TRANSLATE_TO_LENGTH_MASK = 0x3F;
    const uns32 PREFIX_LENGTH_SHIFT = 22;
    const uns32 PREFIX_LENGTH_MASK = 0x1F;
    const uns32 POSTFIX_LENGTH_SHIFT = 27;
    const uns32 POSTFIX_LENGTH_MASK = 0x1F;
        // TRANSLATE_TO_LENGTH overlaps with
	// PREFIX_LENGTH and POSTFIX_LENGTH.

    inline uns32 keep_length ( uns32 operation )
    {
        return ( operation >> KEEP_LENGTH_SHIFT )
	       &
	       KEEP_LENGTH_MASK;
    }
    inline uns32 translate_to_length ( uns32 operation )
    {
        return (    operation
	         >> TRANSLATE_TO_LENGTH_SHIFT )
	       &
	       TRANSLATE_TO_LENGTH_MASK;
    }
    inline uns32 prefix_length ( uns32 operation )
    {
        return ( operation >> PREFIX_LENGTH_SHIFT )
	       &
	       PREFIX_LENGTH_MASK;
    }
    inline uns32 postfix_length ( uns32 operation )
    {
        return ( operation >> POSTFIX_LENGTH_SHIFT )
	       &
	       POSTFIX_LENGTH_MASK;
    }

    // Composite operations.
    //
    inline uns32 KEEP ( uns32 keep_length )
    {
        assert
	  ( keep_length <= KEEP_LENGTH_MASK );
	return KEEP_FLAG
	     + (    keep_length
	         << KEEP_LENGTH_SHIFT );
    }
    inline uns32 TRANSLATE_TO
	    ( uns32 translate_to_length )
    {
        assert (    translate_to_length
	         <= TRANSLATE_TO_LENGTH_MASK );
	return TRANSLATE_TO_FLAG
	     + (    translate_to_length
	         << TRANSLATE_TO_LENGTH_SHIFT );
    }
    inline uns32 TRANSLATE_HEX
    	( uns32 prefix_length, uns32 postfix_length )
    {
        assert
	  ( prefix_length <= PREFIX_LENGTH_MASK );
        assert
	  ( postfix_length <= POSTFIX_LENGTH_MASK );
	return TRANSLATE_HEX_FLAG
	     + ( prefix_length << PREFIX_LENGTH_SHIFT )
	     + (    postfix_length
	         << POSTFIX_LENGTH_SHIFT );
    }
    inline uns32 TRANSLATE_OCT
    	( uns32 prefix_length, uns32 postfix_length )
    {
        assert
	  ( prefix_length <= PREFIX_LENGTH_MASK );
        assert
	  ( postfix_length <= POSTFIX_LENGTH_MASK );
	return TRANSLATE_OCT_FLAG
	     + ( prefix_length << PREFIX_LENGTH_SHIFT )
	     + (    postfix_length
	         << POSTFIX_LENGTH_SHIFT );
    }

    // Create an instruction.
    //
    uns32 create_instruction
	    ( uns32 operation,
	      uns32 * translation_vector = NULL,
	      uns32 atom_table_ID = 0,
	      uns32 require_dispatcher_ID = 0,
	      uns32 else_instruction_ID = 0,
	      uns32 erroneous_atom_type = 0,
	      uns32 output_type = 0,
	      uns32 goto_table_ID = 0,
	      uns32 call_table_ID = 0,
	      scanner_ptr scanner = default_scanner );

    // Attach a dispatcher or an instruction component
    // to a lexical table target, or a type map compo-
    // nent to a dispatcher target.  Return 1 if no
    // error.  Return 0 and do nothing but write error_
    // message if there is a conflict with a previous
    // attachment.
    //
    uns32 attach
    	    ( uns32 target_ID,
    	      uns32 component_ID,
	      scanner_ptr scanner = default_scanner );

    // Attach a dispatcher or an instruction component
    // to a ctype of a dispatcher target.  Return 1 if
    // no error.  Return 0 and do nothing but write
    // error_message if there is a conflict with a
    // previous attachment.
    //
    uns32 attach
    	    ( uns32 target_ID,
    	      uns32 ctype,
	      uns32 component_ID,
	      scanner_ptr scanner = default_scanner );

    // Convert the program to the endianhood of this
    // computer.  This is necessary when the program is
    // read from a binary file.  The first uns32 element
    // of the program determines the program's endian-
    // hood (it is a known constant that appears correct
    // if and only if the program's current endianhood
    // is correct).  Note that the program contains
    // embedded byte vectors which must not be changed
    // by endianhood conversion, so one cannot simply
    // convert all the uns32 elements of the program.
    //
    // If a program formatting error is found while
    // changing the program's endianhood, false is
    // returned.  Otherwise true is returned.  Many
    // program formatting errors cannot be found this
    // way.
    //
    bool convert_program_endianhood
	    ( scanner_ptr scanner = default_scanner );
} }

// Scanner Closures
// ------- --------

namespace ll { namespace lexeme {

    struct input_struct
        // Closure to add inchar elements to the end of
	// the input buffer vector.
    {
    	uns32 control;
	bool (*get) ( scanner_ptr scanner,
	              input_ptr input );
	    // See scanner->read_input.
    };

    extern input_ptr & default_read_input;
        // Default value for scanner->read_input.
	// See scanner->read_input AND scanner->input_
	// file below.
	//
	// Note: This variable is not set until the first
	// scanner is created (first call to init_
	// scanner).

    // Set input closure function.  If `input' is NULL_
    // STUB, create closure and set `input' to a pointer
    // to the created closure.  `input' must be loca-
    // table by garbage collector.
    //
    void init_input
	    ( bool (*get) ( scanner_ptr scanner,
	                    input_ptr input ),
	      input_ptr & input = default_read_input );

    struct erroneous_struct
        // Closure to add announce errors, such as erro-
	// neous atoms.
    {
    	uns32 control;
	void (* announce )
	    ( uns32 first, uns32 last, uns32 type,
	      scanner_ptr scanner,
	      erroneous_ptr erroneous );
	    // See scanner->erroneous_atom.
    };

    extern erroneous_ptr & default_erroneous_atom;
        // Default value for scanner->erroneous_atom.
	// Prints error message to scanner->err.
	//
	// Note: This variable is not set until the first
	// scanner is created (first call to init_
	// scanner).

    // Set erroneous closure function.  If `erroneous'
    // is NULL_STUB, create closure and set `erroneous'
    // to a pointer to the created closure.  `erroneous'
    // must be locatable by garbage collector.
    //
    void init_erroneous
	    ( void (* announce )
		( uns32 first, uns32 last, uns32 type,
		  scanner_ptr scanner,
		  erroneous_ptr erroneous ),
	      erroneous_ptr & erroneous =
	          default_erroneous_atom );
} }

// Scanner
// -------

namespace ll { namespace lexeme {

    // There are several parameters that when set cause
    // a scanner to be (re)initialized.  These are all
    // settable by init_scanner functions.  For these
    // the scanner is specified by a variable, and if
    // this == NULL_STUB, a new scanner is created and a
    // pointer to it is stored in the variable.  This
    // variable MUST BE locatable by the garbage collec-
    // tor.
    //
    // Note that a scanner must be created before a new
    // program can be constructed using the scanner.
    //
    // When a new scanner is created, scanner parameters
    // such as print_mode, line_length, indent, read_
    // input, input_file, etc. are set to defaults.
    // Otherwise these are left untouched, and can be
    // set either before or immediately after the call
    // to init_scanner.

    // Simply (re)initialize a scanner.
    //
    void init_scanner
	    ( scanner_ptr & scanner = default_scanner );

    // Set the scanner program and reinitialize the
    // scanner.
    //
    void init_scanner
            ( program_ptr program,
	      scanner_ptr & scanner = default_scanner );

    // Set the scanner input_file to equal the contents
    // of the named file.  Return true if no error and
    // false if error.  If there is an error, an error
    // message is put in scanner->error_message.
    // (Re)initialize scanner.
    //
    // See init_file below for more details.
    //
    bool init_scanner
	    ( const char * file_name,
	      scanner_ptr & scanner = default_scanner );

    // Ditto but initialize input_file to input from an
    // istream and have the given spool_length.  See
    // init_file below for details.
    //
    void init_scanner
	    ( std::istream & istream,
	      const char * file_name,
	      uns32 spool_length,
	      scanner_ptr & scanner = default_scanner );

    // Ditto but initialize input_file to the contents
    // of a data string.  See init_file below for
    // details.
    //
    void init_scanner
	    ( const char * file_name,
	      const char * data,
	      scanner_ptr & scanner = default_scanner );

    // Scan the input and return the next lexeme or
    // SCAN_ERROR.
    //
    // When a lexeme is returned, the first and last
    // positions in the input buffer are returned, i.e.,
    // the lexeme is in
    //
    //		input_buffer[first .. last]
    //
    // The lexeme length, last - first + 1, is always
    // >= 1.  The lexeme type is returned as the value
    // of the scan function.  The translated lexeme is
    // returned in the translation buffer.
    //
    // If there is an error in the lexical scanning
    // program, SCAN_ERROR is returned instead of a
    // lexeme type, and an error message diagnostic is
    // placed in error_message.  In these two cases
    // first and last and the translation buffer are not
    // set.
    //
    uns32 scan
            ( uns32 & first, uns32 & last,
	      scanner_ptr scanner = default_scanner );
} }

// Input Files
// ----- -----

namespace ll { namespace lexeme {

    // A file is data char vector containing lines as
    // NUL terminated UTF-8 character strings, plus
    // another vector containing the offsets in the
    // data vector of each file line.  A file may be
    // presented in its entirety, or it may be presented
    // as an std::istream that is read in one line at a
    // time.  In the later case all the lines read so
    // far may be saved in the file data, or only the
    // last N lines may be saved, where N is called the
    // `spool length'.
    //
    struct file_struct
        // Header for vector containing line offsets.
	// All other file information is stored in this
	// header or in the data vector it points to.
    {
	const uns32 control;
	const uns32 length;
	const uns32 max_length;

        min::gen file_name;
	    // String naming file, or "stdin" etc.

	min::packed_vec_insptr<char> data;
	    // File data.

	std::istream * istream;
	    // If not NULL, data is read from this
	    // istream and appended to the file data,
	    // as necessary.  If NULL, entire file must
	    // be stored in file data when file is
	    // initialized.

	uns32 spool_length;
	    // If istream is not NULL, then if this is
	    // 0 all file lines are stored in file data.
	    // Otherwise must the last spool_length
	    // lines are stored, and data is a circular
	    // buffer (but lines are not split across
	    // boundaries).  In this last case max_
	    // length == spool_length.

	uns32 line_number;
	    // Line number of next line to be read.
	    // Equals number of lines read so far by
	    // next_line.

	uns32 offset;
	    // Offset of first data element of the next
	    // line to be read.  If istream != NULL then
	    // this equals data->length and the line has
	    // not yet actually been read from istream
	    // and put in data.
	    //
	    // Set to NO_LINE by next_line() when it
	    // returns NO_LINE because there are no more
	    // lines to be read from the file.  In this
	    // case line_number is the total number of
	    // lines in the file.
    };

    // NO_LINE denotes a missing line, possibly caused
    // by an end of file, for char vector offset values.
    //
    const uns32 NO_LINE = 0xFFFFFFFF;

    // Init_file functions are used to initialize an
    // input file.  For these the file is specified by
    // a variable, and if this == NULL_STUB, a new
    // input_file is created and a pointer to it is
    // stored in the variable.  This variable MUST BE
    // locatable by the garbage collector.

    // Read an entire file into an input file.  Previous
    // contents of the input file are lost, the input
    // file istream is set NULL, and the input file
    // spool_length is set to 0.  The file is initial-
    // ized to the first line of the file.
    //
    // If an error occurs, an error message is put into
    // the buffer and false is returned.  If there is no
    // error, true is returned.
    //
    bool init_file ( const char * file_name,
                     char error_message[512],
		     file_ptr & file );

    // Initialize an input file to read from the given
    // std::istream.  Set the file name of the input
    // file and the spool_length.  The file is initial-
    // ized to the first line.
    //
    void init_file ( std::istream & istream,
                       const char * file_name,
		       uns32 spool_length,
		       file_ptr & file );

    // Initialize file to contain the given data string
    // and have the given file name.  The data is NUL
    // terminated.
    //
    // WARNING: this function is distinguished from the
    // function that reads an entire file ONLY by the
    // fact that the second argument is `CONST char'.
    //
    void init_file ( const char * file_name,
		     const char * data,
		     file_ptr & file );

    // Return offset of next line in file->data.  Return
    // NO_LINE if end of file or input error.
    //
    // If file->instream != NULL and file->spool_length
    // > 0, the returned value remains valid ONLY until
    // the next call to next_line, as these calls shift
    // lines in file->data.
    //
    uns32 next_line ( file_ptr file );

    // Return offset in file->data of the line with the
    // given line number.  If the line is has not yet
    // been read with next_line, or is no longer in the
    // spool when istream != NULL and spool_length > 0,
    // return NO_LINE.
    //
    // If file->instream != NULL and file->spool_length
    // > 0, the returned value remains valid ONLY until
    // the next call to next_line, as these calls shift
    // lines in file->data.
    //
    uns32 line ( file_ptr file, uns32 line_number );

    // Print a line from scanner->input_file to an
    // output stream using scanner->print_mode.  If
    // append_line_feed is true, append a line feed
    // if the scanner->print_mode is xxxGRAPHIC (so
    // the line feed prints as graphic characters).
    // Return the number of columns printed.
    //
    // If the line_number designates the line just after
    // the file, print "<END-OF-FILE>".
    //
    // If the line_number designates a line not
    // available, do nothing but return NO_LINE.
    //
    uns32 print_line
        ( std::ostream & out,
	  scanner_ptr scanner,
	  uns32 line_number,
	  bool append_line_feed = false );

    // Print the lines containing the lexeme identified
    // by first and last, and print the given mark char-
    // acter (default '^') under the columns containing
    // the lexeme.  Multi-line lexemes are allowed.
    //
    // Return 0 and do nothing if the first line is not
    // available.  It is required that if the first line
    // is available, all lines before the end of file
    // must be available (else this function crashes).
    // Otherwise store the first and last column of the
    // part of the lexeme that is in the last line and
    // return the number of lines printed, which is > 0.
    //
    uns32 print_lexeme_lines
        ( std::ostream & out,
	  uns32 & first_column, uns32 & last_column,
	  scanner_ptr scanner,
	  uns32 first, uns32 last,
	  bool append_line_feed = false,
	  char mark = '+' );

    // Ditto but print the lines containing an item
    // identified by begin and end.  Begin is the posi-
    // tion of the first column to be marked, and end is
    // the position JUST AFTER the last column to be
    // marked.
    //
    uns32 print_item_lines
        ( std::ostream & out,
	  uns32 & first_column, uns32 & last_column,
	  scanner_ptr scanner,
	  const position & begin,
	  const position & end,
	  bool append_line_feed = false,
	  char mark = '+' );

    // Print the given message in a format suitable for
    // use as a message about a lexeme or item whose
    // marked lines were just printed by print_lexeme_
    // lines or print_item_lines.
    //
    // The message must contain just ASCII graphic
    // characters; single space is allowed but horizon-
    // tal tab is not.  The message will be broken
    // across lines so it can be indented by scanner->
    // indent and constrained by scanner->line_length.
    //
    // If the part of the message on the first line
    // begins after first_column or ends before last_
    // column, the part on the first line is extended
    // with the mark character, so the extended part
    // is under all the columns from first_column
    // through last_column.
    //
    void print_message
        ( std::ostream & out,
	  const uns32 & first_column,
	  const uns32 & last_column,
	  scanner_ptr scanner,
	  const char * message,
	  char mark = '+' );
} }

// Printing
// --------

namespace ll { namespace lexeme {

    // Print an uns32 UNICODE character into the buffer,
    // rendering the character in UTF-8 according to the
    // print mode.  See above for the possible print
    // modes.  NUL is put into the buffer at the end of
    // the rendering and a count of the bytes in the
    // rendering is returned (the NUL is not included in
    // this count).  The buffer should be able to hold
    // at least 12 characters (for rendering a maximum
    // 32 bit character in ASCIIGRAPHIC).
    //
    int spchar
	    ( char * buffer, uns32 c,
              uns32 print_mode = default_print_mode );

    // Return the `width' of the character in the given
    // print_mode.  The `width' is the number of columns
    // required by the character rendering.
    //
    // If the returned width is 0, the character is a
    // control character rendered as itself.  In parti-
    // cular if the character is `\t' and the width is 0
    // the number of columns is 8 - (starting column)%8.
    //
    int wchar ( uns32 c,
                uns32 print_mode = default_print_mode );

    // cout << pchar ( c, print_mode ) does the same
    // thing as spchar but prints to an output stream.
    //
    struct pchar {
        uns32 c, print_mode;
	pchar ( uns32 c,
	        uns32 print_mode = default_print_mode )
	    : c ( c ), print_mode ( print_mode ) {}
    };

    // Print the atom table mode or scanner return value
    // into the buffer, and return the number of
    // characters used.
    //
    int spmode
	    ( char * buffer, uns32 mode,
	      scanner_ptr scanner = default_scanner );

    // cout << pmode ( m ) does the same thing as spmode
    // but prints to an output stream.
    //
    struct pmode {
        uns32 mode;
	scanner_ptr scanner;
	pmode ( uns32 mode,
	        scanner_ptr scanner = default_scanner )
	    : mode ( mode ), scanner ( scanner ) {}
    };
} }

std::ostream & operator <<
    ( std::ostream & out,
      const ll::lexeme::pchar & pc );
std::ostream & operator <<
    ( std::ostream & out,
      const ll::lexeme::pmode & pm );

namespace ll { namespace lexeme {

    // Print character as per spchar above, but use the
    // space_mode and scanner to limit line length.
    //
    // The possible space_mode values are the sum of:
    //
    enum {
    	ENFORCE_LINE_LENGTH = 1,
	    // If output column would exceed scanner->
	    // line_length, precede output by a new
	    // line followed by scanner->indent
	    // spaces.
	PREFACE_WITH_SPACE = 2,
	    // Output is preceded by a single space
	    // unless ENFORCE_LINE_LENGTH is also
	    // present and the output column with the
	    // single space included would excede
	    // scanner->line_length, in which case the
	    // output is precded by a new line followed
	    // by scanner->indent spaces.
    };
    //
    // Note: you must give space_mode, as it is
    // necessary to avoid calls ambiguous with
    // spchar above.
    //
    int spchar
	    ( char * buffer, uns32 c,
	      unsigned & column,
	      uns32 space_mode,
	      scanner_ptr scanner = default_scanner,
	      uns32 print_mode = DEFAULT_PRINT_MODE );

    // Like spchar but print the `word' as if it were
    // the translation of a character.  That is, `word'
    // will not be broken between lines.  Here `word'
    // must be an ASCII string of non-control charac-
    // ters (single space IS ALLOWED) and is printed in
    // UTF-8 mode (which is equivalent to ASCIIPRINT in
    // this case except for \).
    //
    int spword
	    ( char * buffer, const char * word,
	      unsigned & column,
	      uns32 space_mode = ENFORCE_LINE_LENGTH,
	      scanner_ptr scanner = default_scanner );

    // Print the given NUL terminated UTF-8 string.
    // The string is copied to the buffer as if it were
    // converted to UNICODE characters that are then
    // printed with spchar.
    //
    // PREFACE_WITH_SPACE is applied only to the first
    // character output.
    //
    // The number of bytes added to the buffer is
    // returned, not counting the NUL that is the last
    // byte added.
    //
    int spstring
	    ( char * buffer,
	      const char * string,
	      unsigned & column,
	      uns32 space_mode = ENFORCE_LINE_LENGTH,
	      scanner_ptr scanner = default_scanner,
	      uns32 print_mode = DEFAULT_PRINT_MODE );

    // Ditto but print the characters of scanner->input_
    // buffer[first .. last] instead of printing a UTF-8
    // string.  If first > last use spword to print
    // "<empty>" instead.
    //
    int spinput
	    ( char * buffer,
              uns32 first, uns32 last,
	      unsigned & column,
	      uns32 space_mode = ENFORCE_LINE_LENGTH,
	      scanner_ptr scanner = default_scanner,
	      uns32 print_mode = DEFAULT_PRINT_MODE );

    // Ditto but print scanner->translation_buffer in
    // its entirety.  If translation_buffer is empty,
    // print "<empty>" instead.
    //
    int sptranslation
	    ( char * buffer,
	      unsigned & column,
	      uns32 space_mode = ENFORCE_LINE_LENGTH,
	      scanner_ptr scanner = default_scanner,
	      uns32 print_mode = DEFAULT_PRINT_MODE );

    // Ditto but print the current lexeme, given its
    // first, last, and type.  Include the position and
    // type, and if the translation is inexact, also
    // include the translation.
    //
    int splexeme
	    ( char * buffer,
              uns32 first, uns32 last, uns32 type,
	      unsigned & column,
	      uns32 space_mode = ENFORCE_LINE_LENGTH,
	      scanner_ptr scanner = default_scanner,
	      uns32 print_mode = DEFAULT_PRINT_MODE );

    // Ditto but print the current erroneous atom.  The
    // translation is not relevant in this case.
    //
    int sperroneous_atom
	    ( char * buffer,
              uns32 first, uns32 last, uns32 type,
	      unsigned & column,
	      uns32 space_mode = ENFORCE_LINE_LENGTH,
	      scanner_ptr scanner = default_scanner,
	      uns32 print_mode = DEFAULT_PRINT_MODE );

    // Return true if the translation buffer holds a
    // copy of scanner->input_buffer[first .. last].
    //
    bool translation_is_exact
	    ( uns32 first, uns32 last,
	      scanner_ptr scanner = default_scanner );

    // Print a representation of the program to the
    // output stream.  There are two output formats:
    // cooked which prints dispatcher table maps
    // from character ranges to instruction/dispatcher
    // IDs but does not separately print out type
    // maps and instructions, and raw, that prints
    // out everything separately.
    //
    void print_program
        ( std::ostream & out, bool cooked = true,
	  scanner_ptr scanner = default_scanner );

    // Ditto but just print the program component with
    // the given ID.  Return the length of the component
    // in uns32 vector elements.
    //
    uns32 print_program_component
        ( std::ostream & out,
	  uns32 ID, bool cooked = true,
	  scanner_ptr scanner = default_scanner );

} }

# endif // LL_LEXEME_H
