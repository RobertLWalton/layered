// Layers Language Lexeme Nested Description Language
// Internal Data
//
// File:	ll_lexeme_ndl_data.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Aug 11 09:38:19 EDT 2010
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
    // A begin_atom_table, begin_dispatcher, and begin_
    // character_pattern push a dispatcher to the
    // dispatchers stack and an instruction to the
    // instructions stack.  The corresponding ends
    // pop these stacks.  Else_no_if's push additional
    // instructions to the instructions stack.
    //
    // When the dispatchers and instructions stacks are
    // popped, the uns32_stack has the following in order,
    // where d is the dispatcher at the top of the
    // dispatchers stack before popping that stack:
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
    // Popping a dispatcher pops an instruction from
    // the instruction stack and removes the instruction
    // and subdispatcher elements from the uns32_stack
    // and pushes into this stack the new triple:
    //
    //		dispatcher_ID
    //		instruction_ID
    //		type_map_count
    //
    // describing the popped dispatcher and instruction.
    // However, if the popped dispatcher is an OTHERS
    // dispatcher, this last triple is not pushed, and
    // the dispatcher_ID adn instruction_ID are instead
    // copied to the parent dispatcher (the type_map_
    // count will be zero).
    //
    // Note that `popping an instruction' means popping
    // all the instructions in a sequence
    //
    //	 <non-else-instruction>
    //   { else_if_not(...) <non-else-instruction> }*.
    //
    // Note that begin/end_character_pattern pushes and
    // pops two dispatchers as if it were a begin/end_
    // atom_table with a single nested begin/end_dis-
    // patcher.
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
	uns32 atom_table_ID;
	uns32 type;
	    // Arguments for ll::lexeme::create_
	    // instruction.  If ll::lexeme::translate_
	    // length ( operation ) is > 0, then it is
	    // the number of uns32's in the stack that
	    // represent the translation vector.  Note
	    // that the ELSE flag is NOT included but
	    // must be added when an instruction with
	    // a non-zero else_dispatcher_ID is created.
	uns32 else_dispatcher_ID;
	    // Else_dispatcher_ID argument for the
	    // PREVIOUS instruction in the instructions
	    // stack.  Non-zero only if this instruction
	    // follows a else_not_if, and in this case
	    // is the dispatcher ID given by the char-
	    // acter pattern name.
	bool accept;
	    // True if and only if this is an accept
	    // instruction.
    };

    extern ll::lexeme::buffer<instruction>
           & instructions;

} } } }

# endif // LL_LEXEME_NDL_DATA_H
