// Layers Language Lexical Analyzer
//
// File:	ll_lexeme.h
// Author:	Bob Walton (walton@seas.harvard.edu)
// Date:	Wed Apr  7 06:14:16 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/07 10:15:24 $
//   $RCSfile: ll_lexeme.h,v $
//   $Revision: 1.5 $

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

    // Create a dispatcher with given maximum numbers of
    // breakpoints and types.  Return the new
    // dispatcher's ID.
    //
    uns32 create_dispatcher
	    ( uns32 max_breakpointers,
	      uns32 max_types );

    // Create a type table for characters in the range
    // cmin .. cmax.  Return the type table ID.
    //
    uns32 create_type_table
	    ( uns32 cmin, uns32 cmax );

    // Set type table entries for characters cmin ..
    // cmax to type t.  Return 0 and do nothing if
    // some of these are already set of if cmin or
    // cmax are out of range of the type table.
    // Otherwise return 1.
    //
    uns32 set_type_table
	    ( uns32 type_table_ID,
	      uns32 cmin, uns32 cmax, uns8 t );

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
    // truncation_length, the atom is truncated to the
    // given number of characters before anything else
    // is done.  If the instruction has a non-NULL
    // translation the atom is replaced by the
    // translation after truncation and before anything
    // else is done.
    //
    uns32 create_instruction
	    ( uns8 operation,
	      uns32 goto_atom_table = 0,
	      uns32 truncation_length = 0,
	      uns32 * translation = NULL,
	      uns32 translation_length = 0 );

    // Attach a dispatcher to an atom table, or a type
    // table to a dispatcher.  Return 1 if no error.
    // Return 0 and do nothing if there is a conflict
    // with the desired attachment.
    //
    uns32 attach
    	    ( uns32 item_being_attached_to_ID,
    	      uns32 attached_item_ID );

    // Attach a dispatcher or an instruction to type t
    // of a dispatcher.  Return 1 if no error.
    // Return 0 and do nothing if there is a conflict
    // with the desired attachment.
    //
    uns32 attach
    	    ( uns32 item_being_attached_to_ID,
    	      uns32 attached_item_ID,
	      uns32 t );

# endif // LL_LEXEME_H
