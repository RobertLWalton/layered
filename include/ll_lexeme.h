// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.h
// Author:	Bob Walton (walton@seas.harvard.edu)
// Date:	Tue Apr  6 20:26:02 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/07 00:39:09 $
//   $RCSfile: ll_lexeme.h,v $
//   $Revision: 1.3 $

// Table of Contents
//
//	Usage and Setup
//	MIN Interface
//	LL Lexeme Program Construction

// Usage and Setup
// ----- --- -----

# ifndef LL_LEXEME_H
# define LL_LEXEME_H

// MIN Interface
// --- ---------

// If LL_LEXEME_INTERFACE is defined, that is included.
// Otherwise various types and functions are defined
// as follows.
//
#ifdef LL_LEXEME_INTERFACE
#   include <LL_LEXEME_INTERFACE>
#else
#   include <min>
    namespace ll { namespace lexeme {
	// Unsigned integer 8-bit and 32-bit types.
	//
	typedef min::uns8 uns8;
	typedef min::uns32 uns32;

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
    } }
#endif

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
    // header_length is the number of uns32 elements
    // in the header part of the program (which is
    // not under the control of ll::lexeme).  This is
    // always > 0, so no part of the program may have
    // ID == 0.
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
        MASTER		= 1,
	LEXEME		= 2,
	WHITESPACE	= 3,
	ERROR		= 4,

	// Modes that are not classes.
	//
	CONTINUATION	= 5
    };

    // Create the atom table with the given mode and
    // return its ID.
    //
    uns32 create_atom_table ( uns8 mode );

    // Create a sorted dispatcher with given maximum
    // number of breakpoints.  Return the new sorted
    // dispatcher's ID.
    //
    uns32 create_sorted_dispatcher
	    ( uns32 max_breakpointers );

    // Create a type table for characters in the range
    // cmin .. cmax.  Return the type table ID.
    //
    uns32 create_type_table
	    ( uns32 cmin, uns32 cmax );

    // Set type table entries for characters cmin ..
    // cmax to type t.  Return 0 and do nothing if
    // some of these are already set.  Otherwise
    // return 1.
    //
    uns32 set_type_table
	    ( uns32 type_table_ID,
	      uns32 cmin, uns32 cmax, uns8 t );

    // Create a type dispatcher for types 0 .. tmax.
    // Return the type dispatcher ID.
    //
    uns32 create_type_dispatcher
	    ( uns32 tmax );

    // Instruction opcodes:
    //
    enum {
    	ACCEPT		= 1,
	DISCARD		= 2,
	KEEP		= 3
    };

    // Create an instruction.  The instruction has the
    // given operation opcode.  If it has a non-zero
    // goto_atom_table, that is the atom table changed
    // to by the instruction.  If it has a non-zero
    // truncation_size, the atom is truncated to the
    // given number of characters before anything else
    // is done.  If the instruction has a non-NULL
    // translation the atom is replaced by the
    // translation after truncation and before anything
    // else is done.
    //
    // Return 1 if there are no errors.  Return 0 if
    // the instruction cannot be attached because
    // an instruction or type dispatcher is already
    // attached to some of the characters in the
    // cmin .. cmax range.
    //
    uns32 create_instruction
	    ( uns8 operation,
	      uns32 goto_atom_table = 0,
	      uns32 truncation_size = 0,
	      uns32 * translation = NULL,
	      uns32 translation_size = 0 );

    // Attach a sorted dispatcher to an atom table,
    // or a type table to a sorted dispatcher, or
    // a type dispatcher to a sorted dispatcher,
    // or an instruction to a type dispatcher,
    // or a sorted dispatcher to a type dispatcher.
    //
    void attach
    	    ( uns32 attach_to_item_ID,
    	      uns32 attached_item_ID );

# endif // LL_LEXEME_H
