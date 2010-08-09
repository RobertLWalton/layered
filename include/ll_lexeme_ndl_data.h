// Layers Language Lexeme Nested Description Language
// Internal Data
//
// File:	ll_lexeme_ndl_data.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Aug  8 12:43:54 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/05/09 01:41:51 $
//   $RCSfile: ll_lexeme.h,v $
//   $Revision: 1.38 $

// Table of Contents
//
//	Usage and Setup
//	Data

// Usage and Setup
// ----- --- -----

# ifndef LL_LEXEME_NDL_DATA_H
# define LL_LEXEME_NDL_DATA_H

# include <ll_lexeme.h>

// Data
// ----

namespace ll { namespace lexeme
    { namespace ndl { namespace data

    using ll::lexeme::uns8;

    // Various things are pushed into and popped from
    // the stacks.
    //
    // A begin_atom_table, begin_dispatcher, and begin_
    // character_pattern push a dispatcher to the
    // dispatchers stack and in instruction to the
    // instructions stack.  The corresponding ends
    // pop these stacks.  Else_no_if's push additional
    // instructions to the instructions stack.
    //
    // When the dispatchers and instructions stacks are
    // popped, the uns32_stack has the following in order,
    // where d is the dispatcher at the top of the
    // dispatchers stack before popping that stack:
    //    
    //	  d.type_map count pairs:
    //		char_min
    //		char_max
    //    for each instruction in the instruction stack,
    //	      translate_length uns32's giving the
    //	      translate vector in memory order
    //    for each subdispatcher:
    //	      sub_type_map_count pairs:
    //		char_min
    //		char_max
    //	      the triple:
    //		sub_dispatcher_ID
    //		sub_instruction_ID
    //		sub_type_map_count
    //
    // Popping the dispatchers and instructions stacks
    // removes the instruction and subdispatcher
    // elements and pushes a new triple:
    //		dispatcher_ID
    //		instruction_ID
    //		type_map_count
    //
    // describing the popped dispatcher and instruction.
    extern ll::lexeme::buffer<uns32> & uns_stack;


    // Accumulated information use to construct a
    // dispatch table for an atom table or <dispatch>.
    // Also the type code and instruction that goes
    // with the dispatch table.
    //
    // Pushed into the dispatchers stack by begin_atom_
    // table() or begin_dispatcher(), and popped by the
    // corresponding end_atom_table() or end_
    // dispatcher().
    // 
    struct dispatcher
    {
        uns8 ascii_map[128];
	    // Type codes of ASCII characters.
	    // 0 if none.  Added to be the begin_
	    // dispatcher that starts a child dispat-
	    // cher, and by the add_characters for
	    // that child.
	uns8 max_type_code;
	    // Last type code assigned to this dispat-
	    // cher.  Incremented by the begin_dispat-
	    // ch() that starts a sub-dispatcher.
	uns32 type_map_count;
	    // Number of min_char,max_char pairs in
	    // the stack for the type code this
	    // dispatcher will be attached to.
	    // Used by parent of this dispatcher.
    };

    extern ll::lexeme::buffer<dispatcher> & dispatchers;

    // Accumulated information to use in constructing
    // an instruction.  Pushed into the instructions
    // stack by begin_atom_table() or begin_dispatcher()
    // and by else_if_not().  Popped from the stack by
    // implicit end_instruction().
    //
    struct instruction
    {
	uns32 operation;
	uns32 atom_table_ID;
	uns32 type;
	    // Arguments for ll::lexeme::create_
	    // instruction.  If ll::lexeme::translate_
	    // length ( operation ) is > 0, then it is
	    // the number of uns32's in the stack that
	    // represent the translation vector.
	uns32 else_dispatcher_ID;
	    // Else_dispatcher_ID argument for the
	    // PREVIOUS instruction in the instructions
	    // stack.  Non-zero only if this instruction
	    // follows a else_not_if, and in this case
	    // is the dispatcher ID given by the char-
	    // acter pattern name.
	bool accept;
	    // True if accept instruction.
    };

    extern ll::lexeme::buffer<instruction> & instructions;

} } } }

# endif // LL_LEXEME_NDL_DATA_H
