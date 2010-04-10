// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.h
// Author:	Bob Walton (walton@seas.harvard.edu)
// Date:	Sat Apr 10 12:13:15 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/10 16:30:24 $
//   $RCSfile: ll_lexeme.h,v $
//   $Revision: 1.17 $

// Table of Contents
//
//	Usage and Setup
//	External Interface
//	LL Lexeme Program Construction

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
	    return * (T *) (* base + header_size);
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
	uns32 allocate ( uns32 n );

	// Deallocates n elements from the end of the
	// buffer vector.  Just sets length -= n.
	// Returns new value of length.
	//
	uns32 deallocate ( uns32 n );

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
	void resize ( uns32 new_max_length );

    };

    struct inchar
    {
        uns64	position;
	uns32	character;
    };

    extern buffer<uns32> program;
        // Program.
    extern buffer<inchar> input_buffer;
        // Scanner input buffer.
    extern buffer<uns32> translation_buffer;
        // Scanner translation buffer.

    // Input one or more chardata elements to the end
    // of the input buffer vector, increasing the length
    // of the buffer.  Return 1 if this is done, and 0
    // if there are no more characters because we are
    // at the end of file.
    //
    uns32 input_data_buffer ( void );
} }

// LL Lexeme Program Construction
// -- ------ ------- ------------

namespace ll { namespace lexeme {

    // Error message describing the last error.  Can
    // be reset to "" indicating there is no error by
    // ll::lexeme::error_message[0] = 0.  Users should
    // not write into this otherwise.
    //
    // The error message is a sequence of '\n' termina-
    // ted lines each no longer than 72 characters.  It
    // may be used in conjunction with the print_program
    // output.
    //
    extern char * error_message;

    // Create a new program and an atom table, and
    // return the ID of the atom table.  The atom
    // table is the initial table of the program,
    // and has the MASTER mode and 0 label.
    //
    // This function resets the program buffer vector
    // length to 0 and then adds a program header
    // to the beginning of the buffer, followed by
    // the atom table.  Subsequent functions add
    // more things to the end of the program buffer
    // vector.
    //
    uns32 create_program ( void );

    // Atom table kinds and modes.
    //
    enum {
        // Kinds (which are also modes).
	//
	LEXEME		= 1,
	WHITESPACE	= 2,
	ERROR		= 3,

	// Modes that are not kinds.
	//
        MASTER		= 4,
	CONTINUATION	= 5
    };

    // Create the atom table with the given mode and
    // label and return its ID.  The label is a user
    // provided value that is returned to the user when
    // an item (LEXEME, ERROR, or WHITESPACE) is
    // recognized by the analyzer via the atom table.
    // It can serve to type a lexeme, for example.
    //
    uns32 create_atom_table ( uns8 mode, uns32 label );

    // Create a dispatcher with given maximum numbers of
    // breakpoints and the maximum type.  Return the new
    // dispatcher's ID.  Note that legal types are 0 ..
    // max_type, and 0 is the default type for any
    // character not mapped by a type map.
    //
    uns32 create_dispatcher
	    ( uns32 max_breakpointers,
	      uns32 max_type );

    // Create a type map for characters in the range
    // cmin .. cmax.  Return the type map ID.  Copy
    // map[0..(cmax-cmin-1)] into type map, so the map
    // will give character c the type map[c-cmin].
    //
    uns32 create_type_map
	    ( uns32 cmin, uns32 cmax,
	      uns8 * map );

    // Create a type map for characters in the range
    // cmin .. cmax.  Return the type map ID.  This
    // form of type map will map all characters in the
    // range to the given type, which must not be 0.
    //
    uns32 create_type_map
	    ( uns32 cmin, uns32 cmax,
	      uns32 type );

    // Instruction opcodes:
    //
    enum {
    	ACCEPT			= ( 1 << 0 ),
	DISCARD			= ( 1 << 1 ),
	KEEP			= 0,
	TRUNCATE_FLAG		= ( 1 << 2 ),
	TRANSLATE_FLAG		= ( 1 << 3 ),
	TRANSLATE_HEX_FLAG	= ( 1 << 4 ),
	TRANSLATE_OCT_FLAG	= ( 1 << 5 ),
	GOTO			= ( 1 << 6 ),
	SHORTCUT		= ( 1 << 7 ),
    };

    // Instruction shifts and masks
    //
    const uns32 TRUNCATE_LENGTH_SHIFT = 16;
    const uns32 TRUNCATE_LENGTH_MASK = 0x3F;
    const uns32 TRANSLATE_LENGTH_SHIFT = 22;
    const uns32 TRANSLATE_LENGTH_MASK = 0x3F;
    const uns32 PREFIX_LENGTH_SHIFT = 22;
    const uns32 PREFIX_LENGTH_MASK = 0x1F;
    const uns32 POSTFIX_LENGTH_SHIFT = 27;
    const uns32 POSTFIX_LENGTH_MASK = 0x1F;

    // Composite operations.
    //
    inline uns32 TRUNCATE ( uns32 truncate_length )
    {
        assert
	  ( truncate_length <= TRUNCATE_LENGTH_MASK );
	return TRUNCATE_FLAG
	     + (    truncate_length
	         << TRUNCATE_LENGTH_SHIFT );
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

    // Create an instruction.  The instruction is the
    // some of some of the following:
    //
    //	 ACCEPT:	Move atom to output item and to
    //			translation buffer.
    //	 DISCARD:	Discard atom.
    //	 KEEP:		Neither of the above.
    //			(ACCEPT and DISCARD are
    //			 exclusive).
    //
    //   TRUNCATE(n):	Truncate atom to n uns32 char-
    //			acters before any other proces-
    //			sing.  Truncation is done in the
    //			input buffer before any other
    //			processing of the atom.  The
    //			discarded end of the atom is re-
    //			tained as input to be rescanned.
    //			n may be 0.
    //
    //	 TRANSLATE(n)	Instead of copying the atom into
    //			the translation buffer, copy the
    //			characters given in the transla-
    //			tion vector instead.  This vec-
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
    //	 GOTO		After all other processing,
    //			switch the current atom table
    //			to that indicated by atom_table_
    //			ID.
    //
    //    SHORTCUT	Process as if analyzer had
    //			switched to the atom table
    //			specified by atom_table_ID
    //			just before inputting the atom,
    //			and then did a GOTO back to the
    //			original atom table at the end
    //			of processing the atom.  This
    //			speeds handling of one-atom
    //			items, e.g., separators.
    //
    //			(GOTO and SHORTCUT are
    //			exclusive.  If neither is given
    //			atom_table_ID must be zero;
    //			otherwise it must be non-zero.
    //			Note that ID's of program compo-
    //			nents are never zero.)
    //
    uns32 create_instruction
	    ( uns32 operation,
	      uns32 atom_table_ID = 0,
	      uns32 * translation_vector = NULL );

    // Attach a dispatcher or an instruction item to an
    // atom table target, or a type map item to a
    // dispatcher target.  Return 1 if no error.  Return
    // 0 and do nothing if there is a conflict with a
    // previous attachment.
    //
    uns32 attach
    	    ( uns32 target_ID,
    	      uns32 item_ID );

    // Attach a dispatcher or an instruction item to
    // type t of a dispatcher target.  Return 1 if no
    // error.  Return 0 and do nothing if there is a
    // conflict with a previous attachment.
    //
    uns32 attach
    	    ( uns32 target_ID,
    	      uns32 item_ID,
	      uns32 t );

    // Initialize lexical scan.  The program must be
    // stored in the program buffer.  It must have been
    // created by the above functions, but may have been
    // dumped into a file after creation and restored
    // from the file.
    //
    void init_scan ( void );

    // Scan the input and return the next item (lexeme,
    // error string, whitespace, or EOF).
    //
    // The first and last positions in the input buffer
    // are returned, i.e., the item is in
    //
    //		input_buffer[first .. last]
    //
    // The item length, last - first + 1, is always
    // >= 1.  The item kind is returned as the
    // value of the scan function, or 0 is returned
    // if there is no item because of an end-of-file.
    // The label of the atom table that generated the
    // item is returned.  The translated item is
    // returned in the translation buffer if there is
    // no end-of-file.
    //
    uns32 scan
            ( uns32 & first, uns32 & last,
	      uns32 & label );

    // Print a representation of the program to the
    // output stream.
    //
    void print_program ( std::ostream & out );

    // Convert the program to the endianhood of this
    // computer.  This is necessary when the program is
    // read from a binary file.  The first uns32 element
    // of the program determines the program's endian-
    // hood (it is known constant that appears correct
    // if and only if the program's current endianhood
    // is correct).  Note that the program contains
    // embedded byte vectors which must not be changed
    // by endianhood conversion, so one cannot simply
    // convert all the uns32 elements of the program.
    //
    void convert_program_endianhood ( void );

} }

# endif // LL_LEXEME_H
