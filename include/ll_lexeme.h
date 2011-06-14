// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jun 13 19:03:05 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Data
//	Program Construction
//	Program Instructions
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

    // (const min::stub *) NULL value.
    //
    using min::NULL_STUB;

    // Trace value with all trace bits on.
    //
    const uns32 TRACE = (uns32) -1;

    // Characters are stored in uns32 integers.
    // This is more than sufficient for UNICODE.

    typedef min::packed_vec_insptr<uns32> program;
        // Type of a pointer to a program.

    extern min::locatable_var<ll::lexeme::program>
           default_program;
        // Default program.

    // ll::lexeme::ptr<T> ( program, index ) returns
    // a min::ptr<T> value pointing at program[index].
    // We use this to point at program components when
    // the program can be relocated.
    //
    template <typename T>
    inline min::ptr<T> ptr
	    ( ll::lexeme::program program, uns32 index )
    {
	return min::ptr<T> ( program + index );
    }

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

    typedef min::packed_vec_insptr<inchar>
            input_buffer;
    typedef min::packed_vec_insptr<uns32>
            translation_buffer;

    struct scanner_struct;
    typedef min::packed_struct_updptr<scanner_struct>
            scanner;
	// See scanner_struct below.

    extern min::locatable_var<ll::lexeme::scanner>
           default_scanner;
        // Default scanner.

    struct input_struct;
    typedef min::packed_struct_updptr<input_struct>
            input;
	// See input_struct below.

    extern min::locatable_var<ll::lexeme::input>
           default_input;
        // Default value for scanner->input.  See
	// scanner->input AND scanner->input_file below.

    struct erroneous_atom_struct;
    typedef min::packed_struct_updptr
		<erroneous_atom_struct>
            erroneous_atom;
	// See erroneous_atom_struct below.

    extern min::locatable_var
	       <ll::lexeme::erroneous_atom>
           default_erroneous_atom;
        // Default value for scanner->erroneous_atom.
	// Prints error message to scanner->printer.

} }

// Program Construction
// ------- ------------

namespace ll { namespace lexeme {

    // A program is an uns32 vector.
    //
    // The program uns32 elements are grouped into
    // program components.  Each component has an ID
    // equal to its offset (the offset of its first
    // element) in the program uns32 vector.
    //
    // See ll_lexeme_program_data.h for more details.

    // The program being constructed defaults to
    //
    //		LEX::default_program.

    // Program component types:
    //
    enum {
	PROGRAM			= 1,
	TABLE			= 2,
	TYPE_MAP		= 3,
	DISPATCHER		= 4,
	INSTRUCTION		= 5
    };

    // Return the type of the component at the given
    // ID.
    //
    inline uns32 component_type
	    ( uns32 ID,
	      ll::lexeme::program program =
	          default_program )
    {
        return program[ID];
    }

    // Create a new program.
    //
    // This function resets program to 0 length and then
    // adds a program header to its beginning.  Subse-
    // quent functions add more program components to
    // the end of the program.  If program does not
    // exist (it equals NULL_STUB), it is created.
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
	    ( uns32 line_number = 0,
	      const char * const * type_name = NULL,
	      uns32 max_type = 0,
	      min::ref<ll::lexeme::program> program =
	          default_program );

    // Table modes and return values.
    //
    // A mode is defined to be either MASTER or ATOM
    // or the type of a lexeme table.
    //
    enum {

	// Return values that are not modes.
	//
	SCAN_ERROR	= 0xFFFFFFFF,

	// Modes that are not types.
	//
        MASTER		= 0xFFFFFFFE,
	ATOM		= 0xFFFFFFFD,
        NONE		= 0xFFFFFFFC
    };

    // Create the table with the given mode and return
    // its ID.  The mode may be MASTER or ATOM or the
    // type of a lexeme table.
    //
    // The first MASTER table created after a create_
    // program becomes the initial table of the program.
    //
    uns32 create_table
	    ( uns32 line_number,
	      uns32 mode,
	      ll::lexeme::program program =
	          default_program );

    // Return the mode of a table with the given ID.
    //
    uns32 table_mode
	    ( uns32 ID,
	      ll::lexeme::program program =
	          default_program );

    // Create a dispatcher with given maximum number of
    // breakpoints and maximum ctype (character type).
    // Return the new dispatcher's ID.  Note that legal
    // ctypes are 0 .. max_ctype, and 0 is the default
    // ctype for any character not mapped by a type map.
    //
    uns32 create_dispatcher
	    ( uns32 line_number,
	      uns32 max_breakpointers,
	      uns32 max_ctype,
	      ll::lexeme::program program =
	          default_program );

    // Create a type map for characters in the range
    // cmin .. cmax.  Return the type map ID.  Copy
    // map[0..(cmax-cmin)] into type map, so the map
    // will give character c the ctype map[c-cmin].
    //
    uns32 create_type_map
	    ( uns32 line_number,
	      uns32 cmin, uns32 cmax,
	      uns8 * map,
	      ll::lexeme::program program =
	          default_program );

    // Create a type map for characters in the range
    // cmin .. cmax.  Return the type map ID.  This
    // form of type map will map all characters in the
    // range to the given ctype, which must not be 0.
    //
    uns32 create_type_map
	    ( uns32 line_number,
	      uns32 cmin, uns32 cmax,
	      uns32 ctype,
	      ll::lexeme::program program =
	          default_program );


    // Create an instruction.  Instructions are program
    // components defined below in `Program Instruc-
    // tions'.  Return the ID of the new instruction.
    //
    uns32 create_instruction
	    ( uns32 line_number,
	      uns32 operation,
	      uns32 * translation_vector = NULL,
	      uns32 atom_table_ID = 0,
	      uns32 require_dispatcher_ID = 0,
	      uns32 else_instruction_ID = 0,
	      uns32 erroneous_atom_type = 0,
	      uns32 output_type = 0,
	      uns32 goto_table_ID = 0,
	      uns32 call_table_ID = 0,
	      ll::lexeme::program program =
	          default_program );

    // Attach a dispatcher or an instruction program
    // component to a lexical table target, or a type
    // map component to a dispatcher target.  Return
    // true if no error.  Return false and do nothing
    // but write an error message consisting of one or
    // more complete lines to min::error_message if
    // there is a conflict with a previous attachment.
    //
    bool attach
    	    ( uns32 target_ID,
    	      uns32 component_ID,
	      ll::lexeme::program program =
	          default_program );

    // Attach a dispatcher or an instruction program
    // component to a ctype of a dispatcher target.
    // Return true if no error.  Return false and do
    // nothing but write an error message consisting of
    // one or more complete lines to min::error_message
    // if there is a conflict with a previous attach-
    // ment.
    //
    bool attach
    	    ( uns32 target_ID,
    	      uns32 ctype,
	      uns32 component_ID,
	      ll::lexeme::program program =
	          default_program );

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
    // changing the program's endianhood, an error
    // message consisting of one or more complete lines
    // is written to min::error_message, and false is
    // returned.  Otherwise true is returned.  Most
    // program formatting errors cannot be found this
    // way.
    //
    bool convert_program_endianhood
	    ( ll::lexeme::program program =
	        default_program );
} }

// Program Instructions
// ------- ------------

namespace ll { namespace lexeme {

    // An instruction consists of an uns32 operation,
    // various optional IDs and lexical types, and an
    // uns32 * translation_vector for the TRANSLATE_TO
    // instruction component.  See the create_instruc-
    // tion function above for a list of optional ID's
    // and lexical types.
    //
    // An instruction is composed of instruction compo-
    // nents (not to be confused with program compo-
    // nents).  Each instruction component has a flag
    // and in some cases one or more length parameters
    // in the instruction operation.
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
    //			acters.  The atom must have at
    //			least n characters before it is
    //			truncated.  The discarded end of
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
    //			to the erroneous_atom closure
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

    // Bits 0-15 of an instruction operation are
    // reserved for instruction component flags:
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

    // Instruction component lengths are stored in bits
    // 16-31 of an instruction operation as per:
    //
    //	    16-20   KEEP_LENGTH		   5 bits
    //	    21-25   TRANSLATE_TO_LENGTH    5 bits
    //	    26-28   PREFIX_LENGTH	   3 bits
    //	    29-31   POSTFIX_LENGTH	   3 bits
    //
    // TRANSLATE_TO_LENGTH overlaps with
    // PREFIX_LENGTH and POSTFIX_LENGTH.
    //
    // Each LENGTH is readable via
    //
    //	    ( operation >> ..._LENGTH_SHIFT )
    //	    &
    //	    ..._LENGTH_MASK
    //
    const uns32 KEEP_LENGTH_SHIFT = 16;
    const uns32 KEEP_LENGTH_MASK = 0x1F;
    const uns32 TRANSLATE_TO_LENGTH_SHIFT = 21;
    const uns32 TRANSLATE_TO_LENGTH_MASK = 0x1F;
    const uns32 PREFIX_LENGTH_SHIFT = 26;
    const uns32 PREFIX_LENGTH_MASK = 0x7;
    const uns32 POSTFIX_LENGTH_SHIFT = 29;
    const uns32 POSTFIX_LENGTH_MASK = 0x7;

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

} }

// Scanner Closures
// ------- --------

namespace ll { namespace lexeme {

    struct input_struct
        // Closure to add inchar elements to the end of
	// the input buffer vector.  See the `input'
	// member of scanner_struct below for details.
    {
    	uns32 control;
	bool (*get)
	    ( ll::lexeme::scanner scanner,
	      ll::lexeme::input input );
    };

    // Set input closure function.  If `input' is NULL_
    // STUB, create closure and set `input' to a pointer
    // to the created closure.  `input' must be loca-
    // table by garbage collector.
    //
    void init
	    ( min::ref<ll::lexeme::input> input,
	      bool (*get) ( ll::lexeme::scanner scanner,
	                    ll::lexeme::input input ) );

    struct erroneous_atom_struct
        // Closure to announce erroneous atom errors.
	// See the `erroneous_atom' member of scanner_
	//. struct below for details.
    {
    	uns32 control;
	void (* announce )
	    ( uns32 first, uns32 next, uns32 type,
	      ll::lexeme::scanner scanner,
	      ll::lexeme::erroneous_atom
		  erroneous_atom );
    };

    // Set erroneous_atom closure function.  If
    // `erroneous_atom' is NULL_STUB, create closure
    // and set `erroneous_atom' to a pointer to the
    // created closure.  `erroneous_atom' must be
    // locatable by garbage collector.
    //
    void init
	    ( min::ref<ll::lexeme::erroneous_atom>
	          erroneous_atom,
	      void (* announce )
		( uns32 first, uns32 next, uns32 type,
		  ll::lexeme::scanner scanner,
		  ll::lexeme::erroneous_atom
		      erroneous_atom ) );
} }

// Scanner
// -------

namespace ll { namespace lexeme {

    // There are several parameters that when set cause
    // a scanner to be (re)initialized.  These are all
    // settable by init_... functions.  For these the
    // scanner is specified by a variable, and if this
    // == NULL_STUB, a new scanner is created and a
    // pointer to it is stored in the variable.  This
    // variable MUST BE locatable by the garbage collec-
    // tor.

    enum {
        // Scanner trace flags:

	TRACE_DISPATCH		= (1 << 0),
	    // Trace character dispatches.
	TRACE_TABLE		= (1 << 2),
	    // Trace table invocations.
	TRACE_INSTRUCTION	= (1 << 3)
	    // Trace instructions executed.
    };

    const uns32 return_stack_size = 16;
    struct scanner_struct
    {
        const uns32 control;
	    // Packed structure control word.

	// The input buffer is a vector of inchar
	// elements each holding a character and the
	// position of that character in the input text.
	// The position, consisting of a line, an index,
	// and a column, is not used by the scanner.
	//
	// Created when the scanner is created, and set
	// empty by scanner initialization functions.
	//
	const ll::lexeme::input_buffer input_buffer;

	// The line, index, and column of the character
	// that will be put next at the end of the input
	// buffer.  May be used as the position just
	// after the last character that was put into
	// the input buffer, e.g., the position of the
	// end of file.
	//
	// Zero'ed by scanner initialization functions.
	//
	position next_position;

	// The translation buffer holds the translation
	// of the current lexeme.  For example, if the
	// lexeme is a quoted string lexeme, the quotes
	// may be removed from the translation, and
	// special character representation sequences
	// may be replaced in the translation by the
	// represented characters.
	//
	// Created when the scanner is created, and set
	// empty by scanner initialization functions.
	//
	const ll::lexeme::translation_buffer
	    translation_buffer;

	// The scanner parameters are
	//
	//	program
	//	input
	//	input_file
	//	erroneous_atom
	//	printer
	//	trace
	//
	// All of these but `trace' are set to NULL_STUB
	// when the scanner is created, and if still
	// NULL_STUB when the scanner is first used by
	// the `scan' function, are set to defaults at
	// that time.

	// The program is a sequence of program compo-
	// nents.  Defaults to NULL_STUB.  Must NOT be
	// NULL_STUB when the `scan' function is called.
	//
	const ll::lexeme::program program;

	// Closure to call to input one or more inchar
	// elements to the end of the input buffer
	// vector, increasing the length of the buffer
	// as neccessary.  Return true if this is done,
	// and false if there are no more characters
	// because we are at the end of file.
	//
	// Set to NULL_STUB when the scanner is created.
	// Set to `default_input' if still NULL_STUB
	// when `scan' is first called after scanner
	// initialization.
	//
	const ll::lexeme::input input;

	// ll::lexeme::default_input, the default value
	// of `input', reads UTF-8 lines from the input_
	// file and assigns each UNICODE character a
	// line, index, and column number as follows:
	//
	//   line   input_file->line_number - 1 after
	//	    calling next_line(input_file).
	//
	//   index  Set to 0 at beginning of line;
	//	    incremented by the number of UTF-8
	//	    bytes that encode the character
	//	    added to the input buffer.
	//
	//   column Set to 0 at the beginning of a line;
	//	    incremented by 
	//
	//		min::width
	//		    ( column, c,
	//                    scanner->input_file
	//                           ->print_flags )
	//
	//	    where c is the UNICODE character
	//	    added to the input buffer.
	//
	// Set to NULL_STUB when the scanner is created.
	// Set by
	//
	//	min::init_input_stream
	//	    ( input_file_ref(scanner),
	//	      std::cin );
	//
	//      min::init_file_name
	//	    ( input_file_ref(scanner),
	//	      min::new_str_gen
	//	          ( "standard input" ) );
	//
	// if NULL_STUB when `scan' is first called
	// after scanner initialization.
	//
	const min::file input_file;

	// Closure to call with an erroneous atom as per
	// ERRONEOUS_ATOM instruction flag.  The atom is
	// in
	//
	//	input_buffer[first .. next-1]
	//
	// and the instruction provided type is given as
	// an argument.
	//
	// Set to NULL_STUB when the scanner is created.
	// Set to `default_erroneous_atom' if still
	// NULL_STUB when `scan' is first called after
	// scanner initialization.
	//
	const ll::lexeme::erroneous_atom erroneous_atom;

	// Printer for default erroneous atom error
	// messages and tracing.
	//
	// Set to NULL_STUB when the scanner is created.
	// Set by
	//
	//	min::init ( printer_ref(scanner) );
	//
	// if it is still NULL_STUB when `scan' is
	// called and scanner->trace is non-zero.
	//
	const min::printer printer;

	// Scanner trace flags (see above for values).
	//
	uns32 trace;

	// Scanner state:

	bool reinitialize;
	    // Set to true if scanner is to be
	    // reinitialized on the next call to scan.
	bool scan_error;
	    // Set to true if last call to scan produced
	    // a scan error.
	uns32 next;
	    // input_buffer[next] is the first character
	    // of the first yet unscanned atom.
	uns32 current_table_ID;
	    // Current table ID.
    };

    MIN_REF ( ll::lexeme::input_buffer, input_buffer,
              ll::lexeme::scanner )
    MIN_REF ( ll::lexeme::translation_buffer,
              translation_buffer,
              ll::lexeme::scanner )

    MIN_REF ( ll::lexeme::program, program,
              ll::lexeme::scanner )
    MIN_REF ( ll::lexeme::input, input,
              ll::lexeme::scanner )
    MIN_REF ( min::file, input_file,
              ll::lexeme::scanner )
    MIN_REF ( ll::lexeme::erroneous_atom,
              erroneous_atom,
              ll::lexeme::scanner )
    MIN_REF ( min::printer, printer,
              ll::lexeme::scanner )

    // Simply (re)initialize a scanner.
    //
    void init ( min::ref<ll::lexeme::scanner> scanner );

    // Initialize the scanner and set the scanner
    // program.
    //
    void init_program
	    ( min::ref<ll::lexeme::scanner> scanner,
              ll::lexeme::program program );

    // Reinitialized the scanner and set the scanner->
    // input_file as per min:: function of the same
    // name.
    //
    bool init_input_named_file
	    ( min::ref<ll::lexeme::scanner> scanner,
	      min::gen file_name,
	      uns32 print_flags = 0,
	      uns32 spool_lines = min::ALL_LINES );

    void init_input_stream
	    ( min::ref<ll::lexeme::scanner> scanner,
	      std::istream & istream,
	      uns32 print_flags = 0,
	      uns32 spool_lines = min::ALL_LINES );

    void init_input_string
	    ( min::ref<ll::lexeme::scanner> scanner,
	      const char * data,
	      uns32 print_flags = 0,
	      uns32 spool_lines = min::ALL_LINES );

    void init_input
	    ( min::ref<ll::lexeme::scanner> scanner );

    void init_print_flags
	    ( min::ref<ll::lexeme::scanner> scanner,
	      uns32 print_flags );

    void init_spool_lines
	    ( min::ref<ll::lexeme::scanner> scanner,
	      uns32 spool_lines );

    // Reinitialized the scanner and set the scanner->
    // printer as per min:: function of the same
    // name.
    //
    min::printer init_output_stream
	    ( min::ref<ll::lexeme::scanner> scanner,
	      std::ostream & out );

    // Reinitialize the scanner and set the scanner
    // printer.
    //
    void init_printer
	    ( min::ref<ll::lexeme::scanner> scanner,
              min::printer printer );

    // Reinitialize the scanner and set the scanner
    // input_file.
    //
    void init_input_file
	    ( min::ref<ll::lexeme::scanner> scanner,
              min::file input_file );

    // Scan the input and return the next lexeme or
    // SCAN_ERROR.
    //
    // When a lexeme is returned, the first and next
    // positions in the input buffer are returned, i.e.,
    // the lexeme is in
    //
    //		input_buffer[first .. next-1]
    //
    // The lexeme length, next - first, is always >= 0,
    // The lexeme type is returned as the value of the
    // scan function.  The translated lexeme is returned
    // in the translation buffer.
    //
    // If there is an error in the lexical scanning
    // program, SCAN_ERROR is returned instead of a
    // lexeme type, and an error message consisting of
    // one or more complete lines is written to min::
    // error_message.  In this case first, next, and the
    // translation buffer are not set.
    //
    uns32 scan
            ( uns32 & first, uns32 & next,
	      ll::lexeme::scanner scanner =
	          default_scanner );
} }

// Printing
// --------

namespace ll { namespace lexeme {

    // printer << pgraphic ( c ) does the same thing as
    //	    printer << min::push_parameters
    //              << min::graphic
    //              << min::punicode ( c )
    //              << min::pop_parameters
    //
    struct pgraphic {
        uns32 c;
	pgraphic ( uns32 c ) : c ( c ) {}
    };

    // printer << pmode ( program, m ) prints the mode
    // m of the program.
    //
    struct pmode {
	ll::lexeme::program program;
        uns32 mode;
	pmode ( ll::lexeme::program program,
	        uns32 mode )
	    : program ( program ), mode ( mode ) {}
    };

    // printer << pinput ( scanner, first, next ) prints
    // the characters of scanner->input_buffer[first ..
    // next-1] using pgraphic.  If first >= next,
    // "<empty>" is printed instead.
    //
    struct pinput
    {
	ll::lexeme::scanner scanner;
        uns32 first, next;

        pinput ( ll::lexeme::scanner scanner,
	         uns32 first, uns32 next )
	    : scanner ( scanner ),
	      first ( first ), next ( next ) {}
    };

    // printer << ptranslation ( scanner ) prints the
    // characters of scanner->translation_buffer using
    // pgraphic.  If the buffer is empty, "<empty>" is
    // printed instead.
    //
    struct ptranslation
    {
    	ll::lexeme::scanner scanner;
	ptranslation
	    ( ll::lexeme::scanner scanner )
	    : scanner ( scanner ) {}
    };

    // Ditto but print the current lexeme, given its
    // first, next, and type.  Include the position and
    // type, and if the translation is inexact, also
    // include the translation buffer.
    //
    struct plexeme
    {
    	ll::lexeme::scanner scanner;
        uns32 first, next, type;
	plexeme
	    ( ll::lexeme::scanner scanner,
              uns32 first, uns32 next, uns32 type )
	    : scanner ( scanner ),
	      first ( first ), next ( next ),
	      type ( type ) {}
    };

    // Ditto but print the current erroneous atom.  The
    // translation is not relevant in this case.
    //
    struct perroneous_atom
    {
    	ll::lexeme::scanner scanner;
        uns32 first, next, type;
	perroneous_atom
	    ( ll::lexeme::scanner scanner,
              uns32 first, uns32 next, uns32 type )
	    : scanner ( scanner ),
	      first ( first ), next ( next ),
	      type ( type ) {}
    };

    // Just like min::pline_numbers but takes positions
    // instead of line numbers.  If end.column == 0
    // and end.line > begin.line, then last line number
    // equals end.line - 1 instead of end.line.
    //
    min::pline_numbers pline_numbers
	    ( min::file file,
              const ll::lexeme::position & begin,
              const ll::lexeme::position & end );

    // Just line min::pline_numbers but takes scanner,
    // first, and next as arguments and uses line
    // numbers of scanner->input_buffer[first/next],
    // or scanner->next_position if these do not
    // exist, and decrements end line by 1 if end column
    // is 0.
    //
    min::pline_numbers pline_numbers
	    ( ll::lexeme::scanner scanner,
              uns32 first, uns32 next );

    // Return true if the translation buffer holds a
    // copy of scanner->input_buffer[first .. next-1].
    //
    bool translation_is_exact
	    ( ll::lexeme::scanner scanner,
	      uns32 first, uns32 next );

    // Print the lines and put marks (default '^')
    // underneath columns from `begin' to just before
    // `end'.  `file' is usually scanner->input_file.
    // Lines that are all whitespace are replaced by
    // blank_line.
    //
    void print_item_lines
	    ( min::printer printer,
	      min::file file,
	      const ll::lexeme::position & begin,
	      const ll::lexeme::position & end,
	      char mark = '^',
	      const char * blank_line =
	          "<BLANK-LINE>" );

    // Ditto but for lexeme in scanner->input_buffer
    // [first .. next-1].  If input_buffer[first] or
    // input_buffer[next-1] does not exist, use
    // scanner->next_position instead.
    //
    void print_item_lines
	    ( min::printer,
	      ll::lexeme::scanner scanner,
	      uns32 first, uns32 next,
	      char mark = '^',
	      const char * blank_line =
	          "<BLANK-LINE>" );

    // Print a representation of the program to the
    // printer.  There are two output formats: cooked
    // which prints dispatcher table maps from character
    // ranges to instruction/dispatcher IDs but does not
    // separately print out type maps and instructions,
    // and raw, that prints out everything separately.
    //
    void print_program
    	    ( min::printer printer,
	      ll::lexeme::program program,
	      bool cooked );

    // Ditto but just print the program component with
    // the given ID.  The size of the program component
    // is returned, and can be added to ID to get the
    // ID of the next program component in memory.
    //
    // If the program component is illegal, print an
    // error message and return scanner->program->length
    // + 1.
    //
    min::uns32 print_program_component
    	    ( min::printer printer,
	      ll::lexeme::program program,
	      min::uns32 ID,
	      bool cooked );

    // Return the length of the program component with
    // the given ID.  Used to skip over components.
    //
    uns32 component_length
	    ( uns32 ID,
	      ll::lexeme::program program =
	          default_program );

} }

inline min::printer operator <<
	( min::printer printer,
	  const ll::lexeme::pgraphic & pgraphic )
{
    return printer << min::push_parameters
                   << min::graphic
		   << min::punicode ( pgraphic.c )
                   << min::pop_parameters;
}

min::printer operator <<
	( min::printer printer,
          const ll::lexeme::pmode & pmode );

min::printer operator <<
	( min::printer printer,
          const ll::lexeme::pinput & pinput );

min::printer operator <<
	( min::printer printer,
          const ll::lexeme::ptranslation &
	      ptranslation );

min::printer operator <<
	( min::printer printer,
          const ll::lexeme::plexeme & plexeme );

min::printer operator <<
	( min::printer printer,
          const ll::lexeme::perroneous_atom &
	      perroneous_atom );

# endif // LL_LEXEME_H
