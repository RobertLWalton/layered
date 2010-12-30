// Layers Language Lexeme Nested Description Language
// Internal Data
//
// File:	ll_lexeme_ndl_data.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Dec 30 09:03:54 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

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
    { namespace ndl { namespace data {

    using ll::lexeme::uns8;
    using ll::lexeme::uns32;

    // Various things are pushed into and popped from
    // the stacks.
    //
    // A begin_table, begin_dispatcher, and begin_atom_
    // pattern push a dispatcher to the dispatchers
    // stack.  Whenever a dispatcher is pushed an
    // instruction is simultaneously pushed into the
    // instructions stack, and ELSE's push additional
    // instructions into that stack.  An end_table,
    // end_dispatcher, and end_atom_pattern pop these
    // stacks.
    //
    // Begin_atom_pattern actually pushes 2 dispatchers
    // into the dispatcher stack, because add_characters
    // edits the second to topmost dispatcher.  NEXT
    // pushes another dispatcher, and end_atom_pattern
    // discards the topmost dispatcher in the dispatcher
    // stack.  Instructions in the instruction stack are
    // discarded by atom patterns.
    //
    // When the dispatchers and instructions stacks are
    // popped, the uns32_stack has the following in
    // order, where d is the dispatcher at the top of
    // the dispatchers stack before popping that stack:
    //    
    //	  d.type_map_count pairs from add_characters
    //        calls for this dispatcher:
    //		char_min
    //		char_max
    //    for each non-OTHERS subdispatcher (child of
    //		  this dispatcher):
    //	      subdispatcher type_map_count pairs:
    //		char_min
    //		char_max
    //	      the triple:
    //		subdispatcher dispatcher_ID
    //		subdispatcher instruction_ID
    //		subdispatcher type_map_count
    //    for each instruction in the instruction stack,
    //	      translate_to_length uns32's giving the
    //	      translate vector in memory order
    //
    // Popping a dispatcher in a table pops an instruc-
    // tion group from the instruction stack and removes
    // the instruction and subdispatcher elements from
    // the uns32_stack and pushes into this stack the
    // new triple:
    //
    //		dispatcher_ID
    //		instruction_ID
    //		type_map_count
    //
    // describing the popped dispatcher and instruction
    // group.  However, if the popped dispatcher is an
    // OTHERS dispatcher, this last triple is not
    // pushed, and the dispatcher_ID and instruction_ID
    // are instead copied to the parent dispatcher (the
    // type_map_count will be zero).
    //
    // Note that `popping an instruction group' means
    // popping all the instructions in a sequence
    //
    //	 <instruction> { ELSE(); <instruction> }*.
    //
    extern min::packed_vec_insptr<uns32> uns32_stack;

    // Accumulated information use to construct a
    // dispatch table for an atom pattern or <dispatch>.
    //
    // Add_characters calls edit the SECOND TO TOPMOST
    // dispatcher on the dispatcher stack to map the
    // characters indicated to the max_type_code.  When
    // the topmost dispatcher is popped, this max_type_
    // code is incremented.
    // 
    struct dispatcher
    {
        uns8 ascii_map[128];
	    // Type codes of ASCII characters.
	    // 0 if none.  Subdispatchers modify this
	    // when they add ASCII characters to their
	    // character patterns via their begin_
	    // dispatcher call or add_characters calls
	    // with ASCII quoted strings.
	uns8 max_type_code;
	    // Last type code assigned to this dispat-
	    // cher.  Incremented when a non-OTHER's
	    // subdispatcher is pushed into the dispat-
	    // chers stack.  This subdispatcher uses the
	    // incremented value as the type code to
	    // attach to.  OTHER's subdispatchers attach
	    // to type code 0.
	uns32 type_map_count;
	    // Number of min_char,max_char pairs in
	    // the uns32_stack for the type code in
	    // this dispatcher's parent that this
	    // dispatcher will be attached to.
	    // Pushed into the uns32_stack with the
	    // other data from this dispatcher for
	    // use by its parent dispatcher.
	    //
	    // Must be 0 for an OTHER's dispatcher.
	uns32 others_dispatcher_ID;
	uns32 others_instruction_ID;
	    // Data from an OTHER's subdispatcher.
	    // Attached to type code 0 of this
	    // dispatcher.
	bool is_others_dispatcher;
	    // True if and only if this is an OTHER's
	    // dispatcher.
    };

    extern min::packed_vec_insptr<dispatcher>
           dispatchers;

    // Accumulated information to use in constructing
    // an instruction.  Pushed into the instructions
    // stack when a new dispatcher is pushed into the
    // dispatcher stack.  Also pushed by ELSE() to form
    // a multi-instruction instruction group.  The top-
    // most instruction group is popped when a dispat-
    // cher is popped.
    //
    struct instruction
    {
	uns32 operation;
	uns32 atom_table_ID;
	uns32 require_dispatcher_ID;
	uns32 erroneous_atom_type;
	uns32 output_type;
	uns32 goto_table_ID;
	uns32 call_table_ID;
	    // Arguments for ll::lexeme::create_
	    // instruction.  If ll::lexeme::translate_
	    // to_length ( operation ) is > 0, then it
	    // is the number of uns32's in the stack
	    // that represent the translation vector.
	    //
	    // Else_instruction_ID is not stored here
	    // but is set from the last instruction
	    // popped from the instruction stack when
	    // an instruction containing an ELSE flag
	    // is popped.
	bool accept;
	    // True if and only if this is an accept
	    // instruction.
    };

    extern min::packed_vec_insptr<instruction>
           instructions;

} } } }

# endif // LL_LEXEME_NDL_DATA_H
