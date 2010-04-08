// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.h
// Author:	Bob Walton (walton@seas.harvard.edu)
// Date:	Thu Apr  8 09:31:39 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/08 13:35:36 $
//   $RCSfile: ll_lexeme.h,v $
//   $Revision: 1.11 $

// Table of Contents
//
//	Usage and Setup
//	External Interface
//	LL Lexeme Program Construction

// Usage and Setup
// ----- --- -----

# ifndef LL_LEXEME_H
# define LL_LEXEME_H

// External Interface
// -------- ---------

namespace ll { namespace lexeme {

    // Unsigned integer 8-bit and 32-bit types.
    //
    typedef unsigned char uns8;
    typedef unsigned uns32;

    // Characters are stored in uns32 integers.
    // This is more than sufficient for UNICODE.

    // Type of data that records character position
    // of character in input text.
    //
    typedef unsigned long long pos;

    // Allocate a vector of uns32 elements, whose
    // initial elements are a header that cannot be
    // used to store the program.
    //
    uns32 ** allocate_program ( void );

    // Number of uns32 elements in program.
    //
    uns32 program_length ( void );

    // Resize the program.  Length may not be set
    // less than initial length (i.e., the header).
    //
    resize_program ( uns32 new_length );

    // Deallocate the program vector.
    //
    void deallocate_program ( void );

    // Type of data buffer element.
    //
    struct chardatum
    {
        pos	position;
	uns32	character;
    };

    // Allocate a vector of chardatum elements, whose
    // initial elements are a header that cannot be
    // used to store data.
    //
    chardatum ** allocate_data_buffer ( void );

    // Number of chardatum elements in data buffer.
    //
    uns32 data_buffer_length ( void );

    // Resize the data buffer.  Length may not be set
    // less than initial length (i.e., the header).
    //
    resize_data_buffer ( uns32 new_length );

    // Deallocate the data buffer vector.
    //
    void deallocate_data_buffer ( void );

    // Input one or more chardata elements to the end
    // of the data buffer, thereby increasing the length
    // of the buffer.  Return 1 if this is done, and
    // 0 if end of file.
    //
    uns32 input_data_buffer ( void );

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
            ( uns32 & first, & uns32 last,
	      uns32 & label );
} }

// LL Lexeme Program Construction
// -- ------ ------- ------------

namespace ll { namespace lexeme {

    // Create a new program and an atom table, and
    // return the ID of the atom table.  The atom
    // table is the initial table of the program,
    // and has the MASTER mode.
    //
    uns32 create_program ( void );

    // (* program_pointer)[ID] is uns32 element at
    // offset ID in the program.
    //
    // ID's are never 0, so ID == 0 is used to denote
    // a missing ID.
    //
    // header_length is the number of uns32 elements
    // in the header part of the program (which is
    // not under the control of ll::lexeme).
    //
    // length is the current number of uns32 elements
    // of the program that are used.  max_length is
    // the current number of uns32 element in the
    // program.
    //
    // length_increment is the number of unused uns32
    // elements allocated whenever the program is
    // resized.
    //
    extern uns32 ** program_pointer;
    extern uns32 header_length;
    extern uns32 length;
    extern uns32 max_length;
    extern uns32 length_increment;
        
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
    //			The `discarded' end of the atom
    //			is retained as input to be
    //			rescanned.  (Truncate_length
    //			may be 0; it cannot be non-zero
    //			if there is no TRUNCATE flag.)
    //
    //	 TRANSLATE	Translate the atom to the
    //			characters given in the
    //			translation vector which is of
    //			translation_length.  The
    //		        translation replaces the
    //			atom characters in the buffer
    //			immediately after truncation.
    //			The position of each translation
    //			character is set to the position
    //			of the first original character.
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

# endif // LL_LEXEME_H
