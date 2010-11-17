// Layers Language Lexeme Nested Description Language
// Internal Data
//
// File:	ll_lexeme_ndl_data.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Nov 17 12:39:34 EST 2010
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
    // stack.  Begin_table and begin_dispatcher push
    // an instruction to the instructions stack, and
    // ELSE's push additional instructions to the
    // instructions stack.  An end_table, end_dispat-
    // cher, and end_atom_pattern pop these stacks.
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
    //	      translate_length uns32's giving the
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
    // OTHERS dispatcher, this last triple is not pushed,
    // and the dispatcher_ID and instruction_ID are
    // instead copied to the parent dispatcher (the
    // type_map_count will be zero).
    //
    // Note that `popping an instruction group' means
    // popping all the instructions in a sequence
    //
    //	 <instruction> { ELSE(); <instruction> }*.
    //
    // Note that begin/end_atom_pattern pushes and pops
    // two dispatchers as if it were a begin/end_table
    // with a single nested begin/end_dispatcher.  Each
    // { NEXT(...); <add-characters>*} creates a single
    // nested subdispatcher.
    //
    extern ll::lexeme::buffer<uns32> & uns32_stack;

    // Accumulated information use to construct a
    // dispatch table for an atom table or <dispatch>.
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
	    // cher.  Incremented by the begin_dispat-
	    // ch() that starts a non-OTHER's sub-
	    // dispatcher; the subdispatcher that
	    // increments this will use the new value
	    // of this as the type code to attach to.
	    // OTHER's subdispatchers attach to type
	    // code 0.
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
	bool in_atom_pattern;
	    // True if this dispatcher is in an atom
	    // pattern and not part of a table.
    };

    extern ll::lexeme::buffer<dispatcher> & dispatchers;

    // Accumulated information to use in constructing
    // an instruction.  Pushed into the instructions
    // stack when a new dispatcher is pushed into the
    // dispatcher stack, and also pushed by else_if_
    // not().  Popped when a dispatcher is popped.
    //
    struct instruction
    {
	uns32 operation;
	uns32 translation_table_ID;
	uns32 require_table_ID;
	uns32 else_instruction_ID;
	uns32 output_error_type;
	uns32 goto_call_table_ID;
	    // Arguments for ll::lexeme::create_
	    // instruction.  If ll::lexeme::translate_
	    // length ( operation ) is > 0, then it is
	    // the number of uns32's in the stack that
	    // represent the translation vector.
	bool accept;
	    // True if and only if this is an accept
	    // instruction.
    };

    extern ll::lexeme::buffer<instruction>
           & instructions;

} } } }

# endif // LL_LEXEME_NDL_DATA_H
