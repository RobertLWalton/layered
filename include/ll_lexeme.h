// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.h
// Author:	Bob Walton (walton@seas.harvard.edu)
// Date:	Fri Apr  9 20:34:07 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/10 01:27:35 $
//   $RCSfile: ll_lexeme.h,v $
//   $Revision: 1.15 $

// Table of Contents
//
//	Usage and Setup
//	External Interface
//	LL Lexeme Program Construction

// Usage and Setup
// ----- --- -----

# ifndef LL_LEXEME_H
# define LL_LEXEME_H

# include <cstring>

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
    // where T should be a C language number or struct.
    //
    template < typename T >
    struct buffer
    {
        uns8 ** base;
	    // Base address of buffer.  Reset by
	    // resize function when max_length is
	    // changed to or from 0.

	uns32 header_size;
	    // Size of header at beginning of buffer
	    // in bytes.  The header is not available
	    // to store vector elements.  Reset when
	    // base is reset.

	uns32 length;
	    // Number of elements currently used in
	    // the buffer vector.  Maintained by user
	    // of buffer.

	uns32 max_length;
	    // Number of elements in the buffer vector.
	    // Reset only by resize function.

	// b[i] is the i+1'st vector element, if b is
	// of type buffer<T>.  &b[0] is the address of
	// the beginning of the buffer and &b[length] is
	// the address of the first location after the
	// end of the buffer.
	//
	T & operator[] ( uns32 index )
	{
	    return * (T *) (* base + header_size);
	}

	// Change the buffer vector max_length.
	//
	// Changing the max_length to 0 effectively
	// deallocates the buffer vector, and changing
	// from 0 effectively allocates the buffer
	// vector.  Other changes may relocate the
	// buffer vector.
	//
	void resize ( uns32 new_max_length );

    };

    struct chardatum
    {
        uns64	position;
	uns32	character;
    };

    extern buffer<uns32> program;
        // Program.
    extern buffer<chardatum> input_buffer;
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

    // Create a new program and an atom table, and
    // return the ID of the atom table.  The atom
    // table is the initial table of the program,
    // and has the MASTER mode.
    //
    // This function resets the program buffer vector
    // length to 0 and then adds a program header
    // to the beginning of the buffer, followed by
    // the atom table.  Subsequent functions add
    // more things to the end of the program buffer
    // vector.
    //
    uns32 create_program ( void );

    // Atom table classes and modes.
    //
    enum {
        // Classes (which are also modes).
	//
	LEXEME		= 1,
	WHITESPACE	= 2,
	ERROR		= 3,

	// Modes that are not classes.
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
    	ACCEPT		= ( 1 << 0 ),
	DISCARD		= ( 1 << 1 ),
	KEEP		= 0,
	TRUNCATE	= ( 1 << 2 ),
	TRANSLATE	= ( 1 << 3 ),
	GOTO		= ( 1 << 4 ),
	SHORTCUT	= ( 1 << 5 ),
    };

    // Create an instruction.  The instruction has the
    // given operation flag bits interpreted as follows:
    //
    //	 ACCEPT:	Move atom to output item.
    //	 DISCARD:	Discard atom.
    //	 KEEP:		Neither of the above.
    //			(ACCEPT and DISCARD are
    //			 exclusive).
    //
    //   TRUNCATE:	Truncate atom to truncate_length
    //			before any other processing.
    //			Truncation is done in the input
    //			buffer before any other process-
    //			ing of the atom.  The discarded
    //			end of the atom is retained as
    //			input to be rescanned.
    //
    //			(Truncate_length may be 0; it
    //			cannot be non-zero if there is
    //			no TRUNCATE flag.)
    //
    //	 TRANSLATE	Instead of copying the atom into
    //			the translation buffer, copy the
    //			characters given in the transla-
    //			tion vector instead.  This vec-
    //			tor has translation_length char-
    //			acters.
    //
    //			(Translation_length may be 0;
    //			if there is no TRANSLATE flag,
    //			translation_length must be 0 and
    //			translation must be NULL.)
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
    //			otherwise it must be non-zero.)
    //
    uns32 create_instruction
	    ( uns32 operation,
	      uns32 atom_table_ID = 0,
	      uns32 truncation_length = 0,
	      uns32 * translation = NULL,
	      uns32 translation_length = 0 );

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
    // stored in the vector returned by allocate_
    // program.  It must have been created by the
    // above functions, but may have been dumped into
    // a file after creation and restored from the file.
    //
    void init_scan ( void );

    // Scan the input and return the next item (lexeme,
    // error string, whitespace, or EOF).
    //
    // The first and last positions in the data buffer
    // are returned, i.e., the item is in
    //
    //		(* p)[first .. last]
    //
    // where p is the value of allocate_data_buffer.
    // The item length, last - first + 1, is always
    // >= 1.  The item class is returned as the
    // value of the scan function, or 0 is returned
    // if there is no item because on an end-of-file.
    // The label of the atom table that generated the
    // item is returned.
    //
    uns32 scan
            ( uns32 & first, uns32 & last,
	      uns32 & label );

} }

# endif // LL_LEXEME_H
