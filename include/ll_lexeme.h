// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Dec  6 20:35:20 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Data
//	Program Construction
//	Scanning
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
    typedef min::uns8 uns8;
    typedef min::uns32 uns32;
    typedef min::uns64 uns64;

    const min::stub * const NULL_STUB = min::NULL_STUB;

    // Characters are stored in uns32 integers.
    // This is more than sufficient for UNICODE.

    // A buffer is a vector of elements of type T.
    //
    struct buffer_header
    {
        const uns32 type;
	const uns32 length;
	const uns32 max_length;
    };
    template <typename T>
    struct buffer_ptr
        : public min::packed_vec_insptr<buffer_header,T>
    {
        buffer_ptr ( min::gen v )
	    : min::packed_vec_insptr<buffer_header,T>
	        ( v ) {}
        buffer_ptr ( const min::stub * s )
	    : min::packed_vec_insptr<buffer_header,T>
	        ( s ) {}
        buffer_ptr ( void )
	    : min::packed_vec_insptr<buffer_header,T>
	        () {}
    };

    // Allocate n elements in a buffer and return the
    // index of the first element.
    //
    template <typename T>
    inline uns32 allocate ( buffer_ptr<T> p, uns32 n )
    {
        uns32 ID = p->length;
	min::push ( p, n, (T * ) NULL );
	return ID;
    }

    // Deallocate n elements from the end of a buffer
    // and returns the new length of the buffer.
    //
    template <typename T>
    inline uns32 deallocate ( buffer_ptr<T> p, uns32 n )
    {
	min::pop ( p, n, (T *) NULL );
	return p->length;
    }

    // Reset a buffer to zero length.
    //
    template <typename T>
    inline void reset ( buffer_ptr<T> p )
    {
	min::pop ( p, p->length, (T *) NULL );
	min::resize ( p, 1000 );
    }

    typedef buffer_ptr<uns32> program_ptr;
        // Type of a pointer to a program.

    struct inchar
        // Element of input_buffer: see below.
    {
	uns32	line;
	uns32	index;
	uns32	column;
	uns32	character;
    };

    struct file_struct;
    typedef min::packed_vec_insptr<file_struct,uns32>
            file_ptr;
        // See Input Files below.

    struct scanner_struct;
    typedef min::packed_struct_updptr<scanner_struct>
            scanner_ptr;
    const uns32 return_stack_size = 64;
    struct scanner_struct
    {
        const uns32 type;
	    // Packed structure type.

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
	buffer_ptr<inchar> input_buffer;

	// The translation buffer holds the translation
	// of the current lexeme.  For example, if the
	// lexeme is a quoted string lexeme, the quotes
	// may be removed from the translation, and
	// special character representation sequences
	// may be replaced in the translation by the
	// represented characters.  Created by init_
	// scanner.
	//
	buffer_ptr<uns32> translation_buffer;

	// Input one or more inchar elements to the end
	// of the input buffer vector, increasing the
	// length of the buffer as neccessary.  Return 1
	// if this is done, and 0 if there are no more
	// characters because we are at the end of file.
	// Initialized to the default value described
	// below.
	//
	uns32 (*read_input) ( scanner_ptr s );

	// The default value of read_input reads UTF-8
	// characters from the read_input_istream and
	// assigns the UNICODE characters produced the
	// line, index, and column numbers in read_
	// input_inchar.  After putting a UNICODE char-
	// acter into the input_buffer, the line, index,
	// and column numbers in read_input_inchar are
	// updated as follows:
	//
	//   line   Incremented by 1 after a line feed
	//	    is added to the input buffer.
	//
	//   index  Set to 0 after a line feed is added
	//	    to the input buffer; otherwise
	//	    incremented by the number of UTF-8
	//          bytes that encode the UNICODE
	//          character added to the input buffer.
	//
	//   column Set to 0 after adding a line feed,
	//	    form feed, or vertical tab to the
	//          input buffer; set to next multiple
	//          of 8 after adding a tab to the input
	//	    buffer; incremented by the character
	//	    width after adding any other UNICODE
	//	    character to the input buffer.
	//
	// Read_input_istream is initialized to `& cin'
	// and read_input_inchar is initialized to all
	// zeroes.
	//
	// It is possible to save the state of the input
	// and restore it by saving and restoring the
	// contents of:
	//
	//		read_input
	//		read_input_istream
	//		read_input_inchar
	//
	// This may be done when including files.  Care
	// must be taken that the input_buffer ends
	// with the last character of the file include
	// statement, or else input_buffer[next ...]
	// should also be saved and restored.
	//
	std::istream * read_input_istream;
	file_ptr input_file;

	// Output stream for tracing the scan.  If NULL,
	// there is no tracing.  Defaults to NULL.
	//
	std::ostream * scan_trace_out;

	// Function to call with an error atom as per
	// ERRONEOUS_ATOM instruction flag.  The atom is
	// in
	//
	//	input_buffer[first .. last]
	//
	// and the instruction provided type is given as
	// an argument.  If the address of this function
	// is NULL, execution of an instruction with an
	// ERRONEOUS_ATOM flag is a scan error.
	// Defaults to NULL.
	//
	void (* erroneous_atom)
	    ( uns32 first, uns32 last, uns32 type,
	      scanner_ptr s );

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

	// Nominal length and indent for error messages.
	// Line length normally set at the limit for
	// email messages.
	//
	uns32 line_length;  // Default 72
	uns32 indent;       // Default 4

	// Scanner state:

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

	char work[400];
	    // Working buffer for producing components
	    // of error_message.

	// Working data for default_read_input.
	//
	inchar read_input_inchar;
    };

    extern scanner_ptr default_scanner;

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

// Scanning
// --------

namespace ll { namespace lexeme {

    // Initialize scanner.  If scanner == NULL_STUB,
    // creates a new scanner.  Scanner must be
    // initialized before program construction and
    // then again before use as a lexical scanner.
    //
    // Scanner->program is set from program only if
    // program argument is not NULL_STUB.
    //
    void init_scanner
	    ( scanner_ptr & scanner = default_scanner,
              program_ptr program = NULL_STUB );

    // Scan the input and return the next lexeme, END_
    // OF_FILE or SCAN_ERROR.
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
    // If there is an end of file instead of a lexeme,
    // END_OF_FILE is returned instead of a lexeme type.
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
	const uns32 type;
	const uns32 length;
	const uns32 max_length;

        min::gen file_name;
	    // String naming file, or "stdin" etc.

	buffer_ptr<char> data;
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

	uns32 offset;
	    // Offset of first data element of the next
	    // line to be read.  If istream != NULL then
	    // the line has not yet actually been read
	    // from istream and put in data.
    };

    // Create a new input file.
    //
    file_ptr create_file ( void );

    // Read an entire file into an input file.  Previous
    // contents of the input file are lost, the input
    // file istream is set NULL, and the input file
    // spool_length is set to 0.  The file is initial-
    // ized to the first line of the file.
    //
    // If an error occurs, an error message is put into
    // the buffer and std::error is returned.  If there
    // is no error, true is returned.
    //
    bool read_file ( file_ptr file,
                     const char * file_name,
                     char error_message[512] );

    // Initialize an input file to read from the given
    // std::istream.  Set the file name of the input
    // file and the spool_length.  The file is initial-
    // ized to the first line.
    //
    bool init_stream ( file_ptr file,
    		       std::istream & istream,
                       const char * file_name,
		       uns32 spool_length );

    // Initialize file to contain the given data string
    // and have the given file name.  The data is NUL
    // terminated.
    //
    bool init_string ( file_ptr file,
                       const char * file_name,
		       const char * data );

    // Return next line.  Return NULL if end of file
    // or error.
    //
    const char * next_line ( file_ptr file );

    // Return the line with the given line number.
    // If the line is has not yet been read with next_
    // line, return NULL.
    //
    const char * line ( file_ptr file,
    			uns32 line_number );
} }

// Printing
// --------

namespace ll { namespace lexeme {

    // Print an uns32 UNICODE character into the buffer.
    //
    // If the uns32 character c is ' ', '\\', '\n',
    // '\f', '\t', '\v', '\b', or '\r' put "\~/", "\\/",
    // "\lf/", etc. in the buffer.  Otherwise if c is in
    // the range 33 ..  126 put c itself in the buffer.
    // Otherwise "\0X...X/" is put in the buffer, where
    // X...X are hexadecimal digits representing the
    // uns32 value c.  A NUL is put at the end of the
    // characters written into the buffer, and the
    // number of characters written exclusive of the
    // NUL is returned.
    //
    int spchar ( char * buffer, uns32 c );

    // cout << pchar ( c ) does the same thing as spchar
    // but prints to an output stream.
    //
    struct pchar {
        uns32 c;
	pchar ( uns32 c ) : c ( c ) {}
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

    // Print scanner->input_buffer[first .. last] to one
    // or more lines in the buffer.  The \n for the last
    // line is NOT put in the buffer.  Return the number
    // of characters put in the buffer.  Put a NUL
    // character after these characters, NOT including
    // this NUL in the returned count.
    //
    // Preface_with_space is true if the output is to
    // be prefaced with a ` ' character unless it is
    // prefaced with a \n because its beginning will
    // not fit in the remainder of the first line.
    // The next column number to be printed is in the
    // `column' variable; 0 is the first column.  The
    // scanner provides the maximum length of the line
    // and the indent to use after a \n is inserted in
    // the buffer.
    //
    unsigned spinput
	    ( char * buffer,
              uns32 first, uns32 last,
	      unsigned & column,
	      bool preface_with_space = false,
	      scanner_ptr scanner = default_scanner );

    // Ditto but print scanner->translation_buffer in
    // its entirety.
    //
    unsigned sptranslation
	    ( char * buffer,
	      unsigned & column,
	      bool preface_with_space = false,
	      scanner_ptr scanner = default_scanner );

    // Ditto but print the current lexeme, given its
    // first, last, and type.  Include the position and
    // type, and if the translation is inexact, also
    // include the translation.
    //
    unsigned splexeme
	    ( char * buffer,
              uns32 first, uns32 last, uns32 type,
	      unsigned & column,
	      bool preface_with_space = false,
	      scanner_ptr scanner = default_scanner );

    // Ditto but print the current erroneous atom.  The
    // translation is not relevant in this case.
    //
    unsigned sperroneous_atom
	    ( char * buffer,
              uns32 first, uns32 last, uns32 type,
	      unsigned & column,
	      bool preface_with_space = false,
	      scanner_ptr scanner = default_scanner );

    // Ditto but print the given string all on the
    // same line.  n is the length of the string,
    // which need not be NUL terminated.
    //
    unsigned spstring
	    ( char * buffer,
	      const char * string,
	      unsigned n,
	      unsigned & column,
	      bool preface_with_space = false,
	      scanner_ptr scanner = default_scanner );

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
