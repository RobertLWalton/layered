// Layered Languages Lexeme Nested Description Language
// Internal Data
//
// File:	ll_lexeme_ndl_data.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Jul 14 05:09:08 EDT 2017
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
    using ll::lexeme::Uchar;

    // Various things are pushed into and popped from
    // the following stacks:
    //
    // 		dispatchers stack
    // 		instructions stack
    // 		uns32_stack
    // 		uns8_stack
    //
    // A begin_table, begin_dispatcher, and begin_atom_
    // pattern push a dispatcher to the dispatchers
    // stack.  Whenever a dispatcher is pushed an
    // instruction is simultaneously pushed into the
    // instructions stack, and ELSE's push additional
    // instructions into that stack forming an instru-
    // tion group associated with the dispatcher.  Dis-
    // patchers and instruction groups are in 1-1
    // correspondence.  An end_table, end_dispatcher,
    // and end_atom_pattern pop these stacks.
    //
    // Whenever a dispatcher is pushed a CTYPE_MAP_SIZE
    // vector is allocated to (pushed to the end of) the
    // uns8_stack to hold the character type map (ctype_
    // map) of the dispatcher.  Popping the dispatcher
    // pops its ctype_map vector.  The only reason there
    // is a separate vector is that CTYPE_MAP_SIZE is
    // not a compile constant so its awkward to include
    // a dispatcher's ctype_map inside the dispatcher
    // struct.
    //
    // A dispatcher D has a character type counter (max_
    // type_code) initialized to 0.  When a subdispatch-
    // er of dispatcher D is pushed into the dispatcher
    // stack, this counter is incremented and its new
    // value becomes the character type associated with
    // the subdispatcher.  When <character-adder>s are
    // executed for the subdispatcher, they associate
    // characters the ctype_map of D (D is now the
    // SECOND from topmost dispatcher in the stack) with
    // this character type.  When the subdispatcher is
    // popped from the stack, it and its associated
    // instruction and repetition count are associated
    // with this character type in dispatcher D.
    //
    // An OTHERS subdispatcher is a special case and
    // is associated with character type 0.  A table is
    // treated as a dispatcher that has no parent.
    //
    // Popping a dispatcher stack entry pushes the
    // following into the uns32_stack:
    //
    //	    dispatcher ID of created dispatcher
    //	    instruction ID of created instruction
    //	    repetition count from popped dispatcher
    //
    // The popped dispatcher stack entry only creates
    // a dispatcher if it had subdispatchers (so its
    // max_type_code character type counter became > 0).
    // Otherwise the dispatcher ID above is 0.
    //
    // The instruction group for a dispatcher D is
    // created by NDL code executed after the NDL code
    // that creates subdispatchers of D.  It is recorded
    // in the topmost instruction stack entry, and in
    // the case of an instruction group, in entries
    // pushed after that entry.  When a dispatcher stack
    // entry is popped, the instruction group stack
    // entries are popped and generate an instruction ID
    // that is pushed into the uns32_stack as indicated
    // above.  If no instruction is created for a dis-
    // patcher, the sole popped instruction stack entry
    // will be `unused', and the instruction ID will be
    // 0.
    //
    // Note that `popping an instruction group' means
    // popping all the instructions in a sequence
    //
    //	 <instruction> { ELSE(); <instruction> }*.
    //
    // and returning the instruction_ID of the first
    // instruction (last instruction popped) as the
    // instruction_ID of the group.
    //
    // When an instruction with a translation string
    // is created, the translation string is pushed
    // into the uns32_stack.  This translation string
    // is popped when its corresponding instruction
    // stack entry is popped.
    //
    // After creating an instruction ID for a dispatcher
    // stack entry being popped, the uns32_stack will
    // contain the information given above for all the
    // non-OTHERS subdispatchers of the entry being pop-
    // ped, and this information is used to create a
    // dispatcher.  At this point the number of non-
    // OTHERS subdispatchers of the dispatcher being
    // popped is the max_type_count of the dispatcher
    // being popped.
    //
    // Exceptions are the instruction ID and dispatcher
    // ID of an OTHERS subdispatcher and the repetition
    // count of the dispatcher being popped.  These
    // are recorded in the dispatcher stack entry being
    // popped.
    //
    // A dispatcher with an OTHERS subdispatcher MUST
    // also have some non-OTHERS subdispatcher (i.e.,
    // max_type_count > 0).
    //
    // Begin_atom_pattern pushes two entries into the
    // dispatchers stack.  Each NEXT() pushes one more
    // entry.  End_atom_pattern pops all these entries
    // and generates a dispatcher for each but the
    // topmost.  All characters added have character
    // type 1, and there are no instructions.
    //
    extern min::packed_vec_insptr<uns8> uns8_stack;
    extern min::packed_vec_insptr<uns32> uns32_stack;

    // Accumulated information use to construct a
    // dispatch table for an atom pattern or <dispatch>.
    //
    // Add_characters calls edit the SECOND TO TOPMOST
    // dispatcher on the dispatchers stack to map the
    // characters indicated to this dispatcher's max_
    // type_code.
    // 
    const unsigned CTYPE_MAP_SIZE =
        min::unicode::index_limit;
    struct dispatcher
    {
        uns32 line_number;
	    // Line number of begin_dispatch,
	    // begin_atom_pattern, begin_table,
	    // or NEXT.
	uns8 max_type_code;
	    // Character type code counter.  Last type
	    // code assigned to this dispatcher.  Initi-
	    // aly 0.  Incremented when a non-OTHER's
	    // subdispatcher is pushed into the dispat-
	    // chers stack.
	uns32 repeat_count;
	    // Repeat_count value for the character
	    // type in this dispatcher's parent that
	    // this dispatcher will be attached to.
	    // Pushed into the uns32_stack with the
	    // other data from this dispatcher for
	    // use by its parent dispatcher.
	uns32 others_dispatcher_ID;
	uns32 others_instruction_ID;
	uns32 others_repeat_count;
	    // Data from an OTHER's subdispatcher.
	    // Attached to type code 0 of this
	    // dispatcher.
	bool is_others_dispatcher;
	    // True if and only if this is an OTHER's
	    // dispatcher.
        uns32 ctype_map_offset;
	    // Offset of dispatcher ctype_map in
	    // uns8_stack.  The ctype_map has size
	    // CTYPE_MAP_SIZE.
	    //
	    // Subdispatchers modify this ctype_map when
	    // they add characters to their character
	    // patterns via their begin_dispatcher call
	    // or add_char... calls.
	    //
	    // The ctype_map is truncated when it is
	    // used to create the actual dispatcher by
	    // removing 0 elements from its end.
    };

    extern min::packed_vec_insptr<dispatcher>
           dispatchers;

    // Accumulated information to use in constructing
    // an instruction.  Pushed into the instructions
    // stack when a new dispatcher is pushed into the
    // dispatchers stack.  Also pushed by ELSE() to form
    // a multi-instruction instruction group.  The top-
    // most instruction group is popped when a dispat-
    // cher is popped.
    //
    struct instruction
    {
	uns32 line_number;
	    // Line number of first function to set
	    // instruction operation.  0 if none yet.
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
