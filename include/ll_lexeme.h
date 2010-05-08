// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.h
// Author:	Bob Walton (walton@seas.harvard.edu)
// Date:	Sat May  8 04:56:36 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/05/08 09:19:59 $
//   $RCSfile: ll_lexeme.h,v $
//   $Revision: 1.37 $

// Table of Contents
//
//	Usage and Setup
//	External Interface
//	Program Construction
//	Scanning
//	Printing

// Usage and Setup
// ----- --- -----

# ifndef LL_LEXEME_H
# define LL_LEXEME_H

# include <iostream>
# include <cassert>

// External Interface
// -------- ---------

namespace ll { namespace lexeme {

    // Unsigned integer 8-bit, 32-bit, and 64-bit types.
    //
    typedef unsigned char uns8;
    typedef unsigned uns32;
    typedef unsigned long long uns64;

    // Characters are stored in uns32 integers.
    // This is more than sufficient for UNICODE.

    // Character positions are stored in uns64 integers.

    // A buffer holds a vector of elements of type T,
    // where T should be a C language number or struct,
    // and must not have a constructor or destructor
    // or any preferred initial value, and must be
    // copyable with memcpy.
    //
    template < typename T >
    struct buffer
    {
        // Members read-only to user.

        uns8 ** base;
	    // Pointer to location storing address of
	    // buffer vector.  Reset by resize function
	    // when max_length is changed to or from 0.
	    // * base is the address of the buffer
	    // vector.

	uns32 header_size;
	    // Size of header at beginning of buffer
	    // in bytes.  The header is not available
	    // to store vector elements.  Reset when
	    // base is reset.

	uns32 length;
	    // Number of elements currently used in
	    // the buffer vector.  Reset by allocate
	    // and deallocate.

	uns32 max_length;
	    // Number of elements in the buffer vector.
	    // max_length - length elements are unused.
	    // Reset by resize function.

        // Members read-write for user.

	uns32 length_increment;
	    // When allocate must call resize, it
	    // provides for length_increment unused
	    // elements after the allocate is done.
	    // Defaults to 1000.

	// b[i] is the i+1'st vector element, if b is
	// of type buffer<T>.  &b[0] is the address of
	// the beginning of the vector and &b[length] is
	// the address of the first location after the
	// end of the vector.
	//
	T & operator[] ( uns32 index )
	{
	    return ( (T *) (* base + header_size) )
	           [index];
	}

	// Allocate n elements from the end of the
	// buffer vector.
	//
	// Adds n to length, and returns the original
	// length, which is the index of the first
	// element allocated.
	//
	// If necessary calls resize (below) to ensure
	// that max_length > length.  In this case sets
	//
	//	max_length = new_length
	//		   + length_increment
	//
	// Returns old value of length, which is the
	// index of the first element allocated.
	//
	uns32 allocate ( uns32 n )
	{
	    uns32 available = max_length - length;
	    if ( available < n )
	    {
		uns32 new_max_length =
		    max_length + n - available
			       + length_increment;
		resize ( new_max_length );
	    }
	    uns32 location = length;
	    length += n;
	    return location;
	}

	// Deallocates n elements from the end of the
	// buffer vector.  Just sets length -= n.
	// Returns new value of length.
	//
	uns32 deallocate ( uns32 n )
	{
	    assert ( length >= n );
	    length -= n;
	}

	// Change the buffer vector max_length.  The
	// max_length can be increased or decreased.
	// The buffer vector will be relocated in
	// memory.
	//
	// Changing the max_length to 0 effectively
	// deallocates the buffer vector, and changing
	// from 0 effectively allocates the buffer
	// vector.
	//
	virtual void resize ( uns32 new_max_length )
	    = 0;

    };

    // The program is a sequence of program components.
    //
    extern buffer<uns32> & program;

    // The input buffer is a vector of inchar elements
    // each holding a character and the position of that
    // character in the input text.
    //
    struct inchar
    {
        uns64	position;
	uns32	character;
    };
    extern buffer<inchar> & input_buffer;

    // The translation buffer holds the translation of
    // the current lexeme.  For example, if the lexeme
    // is a quoted string lexeme, the quotes may be
    // removed from the translation, and special
    // character representation sequences may be
    // replaced in the translation by the represented
    // characters.
    //
    extern buffer<uns32> & translation_buffer;

    // Input one or more inchar elements to the end
    // of the input buffer vector, increasing the length
    // of the buffer.  Return 1 if this is done, and 0
    // if there are no more characters because we are
    // at the end of file.
    //
    uns32 read_input ( void );
} }

// Program Construction
// ------- ------------

namespace ll { namespace lexeme {

    // Error message describing the last error.  Can
    // be reset to "" indicating there is no error by
    // ll::lexeme::error_message[0] = 0.  Users should
    // not write into this otherwise.
    //
    // The error message is a sequence of lines each no
    // no longer than LINE characters.  All lines but
    // the last are `\n' terminated: it may be printed
    // with cout << ll::lexeme::error_message << endl.
    // It may be used in conjunction with the output of
    // print_program ( false ) (uncooked program print).
    //
    extern char error_message[];

    // Create a new program and an atom table, and
    // return the ID of the atom table.  The atom
    // table is the initial table of the program,
    // and has the MASTER mode.
    //
    // This function resets the program buffer vector
    // length to 0 and then adds a program header
    // to the beginning of the buffer, followed by
    // the atom table.  Subsequent functions add
    // more program compnents to the end of the
    // program buffer vector.
    //
    uns32 create_program ( void );

    // Atom table types, modes, and return values.
    //
    enum {

	// Return values that are not types or modes.
	//
	END_OF_FILE	= 0,
	SCAN_ERROR	= 1,

        // Standard types:
	//
	ERROR		= 2,
	WHITESPACE	= 3,

	// Modes that are not types or return values.
	//
        MASTER		= 0xFFFFFFFF
    };

    // Create the atom table with the given mode and
    // return its ID.  The mode may be a type, which
    // is a user defined value that is returned to the
    // user when a lexeme is recognized by the analyzer
    // via the atom table.  It can serve to type a
    // lexeme.
    //
    uns32 create_atom_table ( uns32 mode );

    // Create a dispatcher with given maximum number of
    // breakpoints and maximum ctype.  Return the new
    // dispatcher's ID.  Note that legal ctypes are 0 ..
    // max_ctype, and 0 is the default ctype for any
    // character not mapped by a type map.
    //
    uns32 create_dispatcher
	    ( uns32 max_breakpointers,
	      uns32 max_ctype );

    // Create a type map for characters in the range
    // cmin .. cmax.  Return the type map ID.  Copy
    // map[0..(cmax-cmin)] into type map, so the map
    // will give character c the ctype map[c-cmin].
    //
    uns32 create_type_map
	    ( uns32 cmin, uns32 cmax,
	      uns8 * map );

    // Create a type map for characters in the range
    // cmin .. cmax.  Return the type map ID.  This
    // form of type map will map all characters in the
    // range to the given ctype, which must not be 0.
    //
    uns32 create_type_map
	    ( uns32 cmin, uns32 cmax,
	      uns32 ctype );

    // An instruction consists of an uns32 operation,
    // an atom_table_ID for GOTO or CALL (which may be 0
    // if unused), a type for OUTPUT or ERRONEOUS_ATOM
    // (which may be 0 if unused), an uns32 *
    // translation_vector for TRANSLATE_FLAG (which may
    // be NULL if unused), and an uns32 else_dispatcher_
    // ID and uns32 else_instruction_ID for ELSE (which
    // may be 0 if unused).  ELSE requires a TRANSLATE_
    // {HEX/OCT}_FLAG and these are exclusive of
    // TRANSLATE_FLAG.
    // 
    // The operation is the sum of some of the
    // following:
    //
    //   KEEP(n):	Truncate atom to n uns32 char-
    //			acters before any other proces-
    //			sing.  Truncation is done in the
    //			input buffer before any other
    //			processing of the atom.  The
    //			discarded end of the atom is re-
    //			tained as input to be rescanned.
    //			n may be 0.
    //
    //   ACCEPT		Equals 0; use if operation is
    //			completely 0 (has no other
    //			components).
    //
    //	 TRANSLATE(n)	Instead of copying the atom into
    //			the translation buffer, copy the
    //			characters given in the transla-
    //			tion_vector instead.  This vec-
    //			tor has n characters, where n
    //			may be 0 (if n is 0, the trans-
    //			lation vector address may be
    //			NULL).
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
    //			The prefix and/or postfix may be
    //			0.
    //
    //			(TRANSLATE, TRANSLATE_HEX, and
    //			TRANSLATE_OCT are mutually ex-
    //			clusive.  The translate_vector
    //			address must be non-NULL for
    //			TRANSLATE(n) with n > 0, and
    //			must be NULL if there is no
    //			TRANSLATE.)
    //
    //	 ELSE		Use with TRANSLATE_{HEX/OCT} to
    //			indicate the presence of an
    //			else_dispatcher_ID and an else_
    //			instruction_ID.  These must be
    //			zero if ELSE is not given.  ELSE
    //			requires that TRANSLATE_{HEX/
    //			OCT} be given.
    //
    //			If an else flag is present, the
    //			character produced by TRANSLATE_
    //			{HEX/OCT} is put into the else_
    //			dispatcher to determine a ctype.
    //			If the ctype is 0 the current
    //			instruction is turned into a
    //			no-operation and is replaced
    //			by the else_instruction.
    //
    //   ERRONEOUS_ATOM	Indicates the current atom is
    //			erroneous and is to be delivered
    //			to the erroneous_atom function
    //			with the instruction provided
    //			type.
    //
    //   GOTO		After all other atom processing,
    //			switch the current atom table
    //			to that whose ID equals the
    //			instruction atom_table_ID.
    //
    //   CALL		Ditto but also push the current
    //			atom table ID (i.e., the ID
    //			before the switch) into the
    //			return stack.
    //
    //     Note: CALL is encoded as a CALLRETURN with
    //		 non-zero atom_table_ID.
    //
    //	 RETURN		Like GOTO but get the new atom
    //			table ID by popping the return
    //			stack.
    //
    //     Note: RETURN is encoded as a RETURN with a
    //		 zero atom_table_ID.
    //
    //   OUTPUT		After KEEP and TRANSLATE...
    //			atom processing, output the
    //			current lexeme with the
    //			instruction provided type.
    //			The current atom table must
    //			be or become a master mode
    //			table.
    //
    //			(ERRONEOUS_ATOM and OUTPUT
    //			are exclusive.  If neither is
    //			given type must be zero; other-
    //			wise it must be non-zero.  Note
    //			real types are never zero.)

    // Instruction operation flags:
    //
    enum {
	ACCEPT			= 0,
	KEEP_FLAG		= ( 1 << 0 ),
	TRANSLATE_FLAG		= ( 1 << 1 ),
	TRANSLATE_HEX_FLAG	= ( 1 << 2 ),
	TRANSLATE_OCT_FLAG	= ( 1 << 3 ),
	ELSE			= ( 1 << 4 ),
	ERRONEOUS_ATOM		= ( 1 << 5 ),
	GOTO			= ( 1 << 6 ),
	CALLRETURN		= ( 1 << 7 ),
	OUTPUT			= ( 1 << 8 ),
    };

    // Instruction shifts and masks
    //
    const uns32 KEEP_LENGTH_SHIFT = 16;
    const uns32 KEEP_LENGTH_MASK = 0x3F;
    const uns32 TRANSLATE_LENGTH_SHIFT = 22;
    const uns32 TRANSLATE_LENGTH_MASK = 0x3F;
    const uns32 PREFIX_LENGTH_SHIFT = 22;
    const uns32 PREFIX_LENGTH_MASK = 0x1F;
    const uns32 POSTFIX_LENGTH_SHIFT = 27;
    const uns32 POSTFIX_LENGTH_MASK = 0x1F;

    inline uns32 keep_length ( uns32 operation )
    {
        return ( operation >> KEEP_LENGTH_SHIFT )
	       &
	       KEEP_LENGTH_MASK;
    }
    inline uns32 translate_length ( uns32 operation )
    {
        return ( operation >> TRANSLATE_LENGTH_SHIFT )
	       &
	       TRANSLATE_LENGTH_MASK;
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
    inline uns32 TRANSLATE ( uns32 translate_length )
    {
        assert
	  ( translate_length <= TRANSLATE_LENGTH_MASK );
	return TRANSLATE_FLAG
	     + (    translate_length
	         << TRANSLATE_LENGTH_SHIFT );
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
	      uns32 atom_table_ID = 0,
	      uns32 type = 0,
	      uns32 * translation_vector = NULL,
	      uns32 else_dispatcher_ID = 0,
	      uns32 else_instruction_ID = 0 );

    // Attach a dispatcher or an instruction component
    // to an atom table target, or a type map component
    // to a dispatcher target.  Return 1 if no error.
    // Return 0 and do nothing but write error_message
    // if there is a conflict with a previous
    // attachment.
    //
    uns32 attach
    	    ( uns32 target_ID,
    	      uns32 component_ID );

    // Attach a dispatcher or an instruction component
    // to a ctype of a dispatcher target.  Return 1 if
    // no error.  Return 0 and do nothing but write
    // error_message if there is a conflict with a
    // previous attachment.
    //
    uns32 attach
    	    ( uns32 target_ID,
    	      uns32 ctype,
	      uns32 component_ID );

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
    void convert_program_endianhood ( void );
} }

// Scanning
// --------

namespace ll { namespace lexeme {

    // Initialize lexical scan.  The program must be
    // stored in the program buffer.  It must have been
    // created by the above functions, but may have been
    // dumped into a file after creation and restored
    // from the file.
    //
    void init_scan ( void );

    // Output stream for tracing the scan.  Set by
    // user.  If NULL, there is no tracing.
    //
    extern std::ostream * scan_trace_out;

    // Function to call with an error atom as per
    // ERRONEOUS_ATOM instruction flag.  The atom is in
    //
    //		input_buffer[first .. last]
    //
    // and the instruction provided type is given as an
    // argument.  If the address of this function is
    // NULL, execution of an instruction with an
    // ERRONEOUS_ATOM flag is a scan error.
    //
    extern void (* erroneous_atom)
	( uns32 first, uns32 last, uns32 type );

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
            ( uns32 & first, uns32 & last );
} }

// Printing
// --------

namespace ll { namespace lexeme {

    // Nominal length of a line for diagnostic messages.
    // Normally set at the limit for email messages.
    //
    const int LINE = 72;

    // Print an uns32 UNICODE character into the buffer.
    //
    // If the uns32 character c is ' ', '\\', '\n',
    // '\f', '\t', '\v', '\b', or '\r' put "\~", "\\",
    // etc in the buffer.  Otherwise if c is in the
    // range 33 ..  126 put c itself in the buffer.
    // Otherwise if c is <= 0xFFFF then \uXXXX is put in
    // the buffer, where XXXX is the hexadecimal
    // representation of the uns32 value c.  Otherwise
    // \UXXXXXXXX is put in the buffer where XXXXXXXX is
    // the hexadecimal representation of c.  A NUL is
    // put at the end of the characters written into the
    // buffer, and the number of characters written
    // exclusive of the NUL is returned.
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
    int spmode ( char * buffer, uns32 mode );

    // cout << pmode ( m ) does the same thing as spmode
    // but prints to an output stream.
    //
    struct pmode {
        uns32 mode;
	pmode ( uns32 mode ) : mode ( mode ) {}
    };
} }

std::ostream & operator <<
    ( std::ostream & out,
      const ll::lexeme::pchar & pc );
std::ostream & operator <<
    ( std::ostream & out,
      const ll::lexeme::pmode & pm );

namespace ll { namespace lexeme {

    // Print ll::lexeme::input_buffer[first .. last]
    // to one or more lines in the buffer.  The \n for
    // the last line is NOT put in the buffer.  Return
    // the number of characters put in the buffer.
    //
    int spinput ( char * buffer,
                  uns32 first, uns32 last );

    // Print a representation of the program to the
    // output stream.  There are two output formats:
    // cooked which prints dispatcher table maps
    // from character ranges to instruction/dispatcher
    // IDs but does not separately print out type
    // maps and instructions, and raw, that prints
    // out everything separately.
    //
    void print_program
        ( std::ostream & out, bool cooked = true );

    // Ditto but just print the program component with
    // the given ID.  Return the length of the component
    // in uns32 vector elements.
    //
    uns32 print_program_component
        ( std::ostream & out,
	  uns32 ID, bool cooked = true );

} }

# endif // LL_LEXEME_H
