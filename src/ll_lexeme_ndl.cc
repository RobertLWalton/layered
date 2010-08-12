// Layers Language Lexical Nested Description Language
//
// File:	ll_lexeme_ndl.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Aug 12 02:04:59 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	NDL Functions

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_ndl.h>
# include <ll_lexeme_ndl_data.h>
# include <ll_lexeme_program_data.h>
# include <iostream>
# include <cstring>
# include <cassert>
# define LEX ll::lexeme
# define LEXDATA ll::lexeme::program_data
# define LEXNDL ll::lexeme::ndl
# define LEXNDLDATA ll::lexeme::ndl::data
using std::cout;
using std::endl;
using namespace LEXNDLDATA;

// Global data (stacks are in ll_lexeme_min.cc).
//
char LEXNDL::OTHER[1];

// Error data.
//
const char * LEXNDL::file = NULL;
uns32 LEXNDL::line = 0;
static const char * function_name = NULL;
static char message_buffer[500];

// Error management.
//
//	FUNCTION(<function-name>) is at the beginning
//	    of each NDL function to register its name
//	    for possible error message use.
//	ASSERT(<test>,<message>) signals an error if
//	    the <test> expression is false.  The
//	    error message includes the function name,
//	    file name and line number of the call as
//	    stored in LEXNDL::file/line by the NDL
//	    macro, the error <message> which is a
//	    const char *, and a copy of the <test>.
//       MESSAGE(format,...) returns the contents of
//	    a static message_buffer for which
//	      sprintf(message_buffer,format,...)
//          has been executed.  This is useful for
//          ASSERT error message, as MESSAGE will
//          not execute in the absence of an error.
//
# define FUNCTION(name) function_name = name
# define ASSERT(test,message) \
    ( (test) ? true : ::error ( #test, message ))
# define MESSAGE(...) \
    ( sprintf ( message_buffer, __VA_ARGS__ ), \
      message_buffer )

// Called by ASSERT macro to print error message and
// exit.
//
static bool error
	( const char * test, const char * message )
{
    cout << "ERROR: FILE: " << LEXNDL::file
         << " LINE: " << LEXNDL::line
	 << " FUNCTION: " << function_name
	 << ":" << endl
	 << "    " << message << endl
	 << "    (" << test << ")" <<endl;
    exit ( 1 );
}

// `state' and `substate' are used to verify NDL
// function execution order.

enum STATE {
    OUTSIDE_PROGRAM,
    INSIDE_PROGRAM,
    INSIDE_CHARACTER_PATTERN,
    INSIDE_ATOM_TABLE };

static STATE state;

// Substates in the order they appear inside an
// atom table, character pattern, or dispatcher.
//
enum SUBSTATE {
    ADD_CHARACTERS,
    DISPATCHERS,
    INSTRUCTION };

static SUBSTATE substate;

// Depth of dispatcher is dispatchers.length.

// Push one uns32 onto the uns32_stack.
//
inline void push_uns32 ( uns32 value )
{
    uns32 i = uns32_stack.allocate ( 1 );
    uns32_stack[i] = value;
}

// Pop one uns32 from the uns32_stack.
//
inline uns32 pop_uns32 ( void )
{
    uns32 i = uns32_stack.length;
    assert ( i > 0 );
    uns32 value = uns32_stack[--i];
    uns32_stack.deallocate ( 1 );
    return value;
}

// Return current or parent dispatcher or instruction.
//
inline dispatcher & current_dispatcher ( void )
{
    uns32 dindex = dispatchers.length;
    assert ( dindex >= 1 );
    return dispatchers[dindex-1];
}
inline dispatcher & parent_dispatcher ( void )
{
    uns32 dindex = dispatchers.length;
    assert ( dindex >= 2 );
    return dispatchers[dindex-2];
}
inline instruction & current_instruction ( void )
{
    uns32 iindex = instructions.length;
    assert ( iindex >= 1 );
    return instructions[iindex-1];
}



// NDL Functions
// --- ---------

void LEXNDL::begin_program ( void )
{
    FUNCTION ( "begin_program" );
    ASSERT ( state == OUTSIDE_PROGRAM,
             "misplaced begin_program()" );
    state = INSIDE_PROGRAM;

    uns32_stack.resize ( 0 );
    dispatchers.resize ( 0 );
    instructions.resize ( 0 );

    LEX::create_program();
}

void LEXNDL::end_program ( void )
{
    FUNCTION ( "end_program" );
    ASSERT ( state == INSIDE_PROGRAM,
             "misplaced end_program()" );
    state = OUTSIDE_PROGRAM;

    assert ( uns32_stack.length == 0 );
    assert ( dispatchers.length == 0 );
    assert ( instructions.length == 0 );
}

uns32 LEXNDL::new_atom_table ( uns32 mode )
{
    FUNCTION ( "new_atom_table" );
    ASSERT ( state == INSIDE_PROGRAM,
             "new_atom_table() misplaced" );
    return LEX::create_atom_table ( mode );
}

// Push a new dispatcher and instruction on the
// dispatchers and instructions stacks.  Set substate
// to ADD_CHARACTERS.  If there is a parent dispatcher,
// increment its max_type_code.
//
// However, if is_others is true, set the substate to
// DISPATCHERS (add_characters is not allowed in an
// OTHER's dispatcher), do NOT increment the parent
// max_type_code, and set is_others_dispatcher true.
//
void push_dispatcher ( bool is_others = false )
{
    if ( dispatchers.length > 0 && ! is_others )
        ++ current_dispatcher().max_type_code;

    dispatchers.allocate ( 1 );
    dispatcher & d = current_dispatcher();

    memset ( d.ascii_map, 0, 128 );
    d.max_type_code = 0;
    d.type_map_count = 0;
    d.others_dispatcher_ID = 0;
    d.others_instruction_ID = 0;
    d.is_others_dispatcher = is_others;

    instructions.allocate ( 1 );
    instruction & i = current_instruction();

    i.operation = 0;
    i.atom_table_ID = 0;
    i.type = 0;
    i.else_dispatcher_ID = 0;
    i.accept = false;

    substate = is_others ? DISPATCHERS
                         : ADD_CHARACTERS;
}

// Pop an instruction (or instructions if there are
// else_not_if instructions) from the instructions
// stack and return its instruction_ID, or return 0 if
// there is no instruction.
//
static uns32 pop_instruction ( void )
{
    uns32 instruction_ID = 0;
    uns32 else_dispatcher_ID = 0;
    while ( true )
    {
	instruction & i = current_instruction();

	uns32 translate_length = 0;
	uns32 call_length = 0;
	if ( i.operation & LEX::TRANSLATE_FLAG )
	    translate_length =
	        LEX::translate_length ( i.operation );
	if ( i.operation & LEX::CALL_FLAG )
	    call_length =
	        LEX::call_length ( i.operation );

        uns32 * translation_vector = NULL;
        uns32 * return_vector = NULL;
	uns32 length = uns32_stack.length;
	assert (    translate_length + call_length
	         <= length );
	if ( translate_length > 0 )
	    translation_vector =
	        & uns32_stack
		    [length - translate_length
		            - call_length];
	if ( call_length > 0 )
	    return_vector =
	        & uns32_stack[length - call_length];

	if ( else_dispatcher_ID != 0 )
	    i.operation |= LEX::ELSE;

	if ( i.operation != 0 || i.accept )
	    instruction_ID = LEX::create_instruction
		( i.operation,
		  i.atom_table_ID,
		  i.type,
		  translation_vector,
		  else_dispatcher_ID,
		  instruction_ID,
		  return_vector );
	else
	    ASSERT ( i.else_dispatcher_ID == 0,
	             "no instruction component after"
		     " else_if_not" );

	if ( translate_length + call_length > 0 )
	    uns32_stack.deallocate
	        ( translate_length + call_length );

	else_dispatcher_ID = i.else_dispatcher_ID;

	instructions.deallocate ( 1 );

	if ( else_dispatcher_ID == 0 ) break;
    }
    return instruction_ID;
}

// Pop a dispatcher from the dispatchers stack.  This
// calls LEX::create_dispatcher which returns the new
// dispatcher_ID.  See stack description in ll_lexeme_
// ndl_data.h.  Set substate to DISPATCHERS.
//
uns32 pop_dispatcher ( void )
{
    uns32 instruction_ID = pop_instruction();

    dispatcher & d = current_dispatcher();

    uns32 cmin = 0;
    uns32 cmax = 127;
    while ( cmin <= cmax && d.ascii_map[cmin] == 0 )
        ++ cmin;
    while ( cmin <= cmax && d.ascii_map[cmax] == 0 )
        -- cmax;
    bool ascii_used = ( cmin <= cmax );

    uns32 dispatcher_ID =
        LEX::create_dispatcher
	    ( 2 * ( d.type_map_count + ascii_used ),
	      d.max_type_code );

    if ( ascii_used )
        LEX::attach ( dispatcher_ID,
	              LEX::create_type_map
		          ( cmin, cmax,
		            d.ascii_map + cmin ) );

    if ( d.others_dispatcher_ID != 0 )
        LEX::attach ( dispatcher_ID, 0,
	              d.others_dispatcher_ID );
    if ( d.others_instruction_ID != 0 )
        LEX::attach ( dispatcher_ID, 0,
	              d.others_instruction_ID );

    for ( uns32 tcode = d.max_type_code;
          0 < tcode; -- tcode )
    {
        uns32 sub_type_map_count = pop_uns32();
	uns32 sub_instruction_ID = pop_uns32();
	uns32 sub_dispatcher_ID = pop_uns32();

	LEX::attach ( dispatcher_ID, tcode,
	              sub_dispatcher_ID );

	if ( sub_instruction_ID != 0 )
	    LEX::attach ( dispatcher_ID, tcode,
		          sub_instruction_ID );

	for ( uns32 i = 0;
	      i < sub_type_map_count; ++ i )
	{
	    uns32 max_char = pop_uns32();
	    uns32 min_char = pop_uns32();
	    LEX::attach ( dispatcher_ID,
	                  LEX::create_type_map
		             ( min_char, max_char,
			       tcode ) );
	}
    }

    if ( d.is_others_dispatcher )
    {
	dispatcher & parent = parent_dispatcher();
	parent.others_dispatcher_ID = dispatcher_ID;
	parent.others_instruction_ID = instruction_ID;
	assert ( d.type_map_count = 0 );
    }
    else
    {
	push_uns32 ( dispatcher_ID );
	push_uns32 ( instruction_ID );
	push_uns32 ( d.type_map_count );
    }

    dispatchers.deallocate ( 1 );

    substate = DISPATCHERS;
}

static uns32 atom_table_name;
void LEXNDL::begin_atom_table ( uns32 atom_table_name )
{
    FUNCTION ( "begin_atom_table" );
    ASSERT ( state == INSIDE_PROGRAM,
             "begin_atom_table() misplaced" );
    state = INSIDE_ATOM_TABLE;
    ::atom_table_name = atom_table_name;

    assert ( dispatchers.length == 0 );
    assert ( instructions.length == 0 );
    push_dispatcher();
    substate = INSTRUCTION;
}

void LEXNDL::end_atom_table ( void )
{
    FUNCTION ( "end_atom_table" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "end_atom_table() misplaced" );
    ASSERT ( dispatchers.length != 1,
             "end_atom_table() misplaced" );

    pop_dispatcher();

    uns32 type_map_count = pop_uns32();
    uns32 instruction_ID = pop_uns32();
    uns32 dispatcher_ID = pop_uns32();

    assert ( dispatchers.length == 0 );
    assert ( instructions.length == 0 );
    assert ( uns32_stack.length == 0 );

    LEX::attach ( atom_table_name, dispatcher_ID );
    if ( instruction_ID != 0 )
	LEX::attach ( atom_table_name, instruction_ID );
    assert ( type_map_count == 0 );

    state = INSIDE_PROGRAM;
}

static void internal_add_characters
	( const char * include_chars,
	  const char * exclude_chars )
{
    dispatcher & d = parent_dispatcher();
    int c;  // use int instead of char to prevent
            // 0 <= c or c < 128 warning message.
    while ( c = * include_chars ++ )
    {
	if ( index ( exclude_chars, c ) ) continue;

	ASSERT ( 0 <= c && c < 128,
	         "non-ASCII character in"
		 " include_chars" );
        ASSERT ( d.ascii_map[c] == 0,
	         MESSAGE ( "character %c in use by"
		           " previous dispatcher",
			   (char) c ) );
	d.ascii_map[c] = d.max_type_code;
    }
}

static uns32 * character_pattern_name_p;
void LEXNDL::begin_character_pattern
	( uns32 & character_pattern_name,
	  const char * include_chars,
	  const char * exclude_chars )
{
    FUNCTION ( "begin_character_pattern" );
    ASSERT ( state == INSIDE_PROGRAM,
             "begin_character_pattern() misplaced" );
    state = INSIDE_CHARACTER_PATTERN;

    push_dispatcher();
    push_dispatcher();
    internal_add_characters
        ( include_chars, exclude_chars );

    character_pattern_name_p = & character_pattern_name;
}

void LEXNDL::end_character_pattern ( void )
{
    FUNCTION ( "end_character_pattern" );
    ASSERT ( state == INSIDE_CHARACTER_PATTERN,
             "end_character_pattern() misplaced" );

    assert ( dispatchers.length == 2 );
    pop_dispatcher();
    pop_dispatcher();

    * character_pattern_name_p = uns32_stack[0];

    uns32_stack.deallocate ( 3 );

    assert ( dispatchers.length == 0 );
    assert ( instructions.length == 0 );
    assert ( uns32_stack.length == 0 );
}

void LEXNDL::add_characters
	( const char * include_chars,
	  const char * exclude_chars )
{
    FUNCTION ( "add_characters" );
    ASSERT ( state == INSIDE_ATOM_TABLE
             ||
	     state == INSIDE_CHARACTER_PATTERN,
             "add_characters() misplaced" );
    ASSERT ( substate == ADD_CHARACTERS,
             "add_characters() misplaced" );

    internal_add_characters
        ( include_chars, exclude_chars );
}

void LEXNDL::add_characters
	( uns32 min_char, uns32 max_char )
{
    FUNCTION ( "add_characters" );
    ASSERT ( state == INSIDE_ATOM_TABLE
             ||
	     state == INSIDE_CHARACTER_PATTERN,
             "add_characters() misplaced" );
    ASSERT ( substate == ADD_CHARACTERS,
             "add_characters() misplaced" );
    ASSERT ( min_char <= max_char,
             "add_characters() min_char > max_char" );

    dispatcher & d = current_dispatcher();
    ++ d.type_map_count;
    push_uns32 ( min_char );
    push_uns32 ( max_char );
}

void LEXNDL::begin_dispatch
	( const char * include_chars,
	  const char * exclude_chars )
{
    FUNCTION ( "begin_dispatch" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "begin_dispatch() misplaced" );
    ASSERT ( substate <= DISPATCHERS,
             "begin_dispatch() misplaced" );

    bool is_others =
        ( include_chars == LEXNDL::OTHER );

    if ( is_others )
    {
        dispatcher & d = current_dispatcher();
	ASSERT ( d.others_dispatcher_ID == 0
	         &&
		 d.others_instruction_ID == 0,
		 "more than one OTHERS dispatchers"
		 " under one parent" );
    }

    push_dispatcher ( is_others );

    if ( ! is_others )
	internal_add_characters
	    ( include_chars, exclude_chars );
}

void LEXNDL::end_dispatch ( void )
{
    FUNCTION ( "end_dispatch" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "end_dispatch() misplaced" );
    ASSERT ( dispatchers.length < 2,
             "end_dispatch() misplaced" );
    pop_dispatcher();
}

void LEXNDL::accept ( void )
{
    FUNCTION ( "accept" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "accept() misplaced" );
    substate = INSTRUCTION;

    instruction & i = current_instruction();
    ASSERT ( ! i.accept,
             "accept() conflicts with another"
	     " accept()" );
    ASSERT ( i.operation == 0,
             "accept() conflicts with another"
	     " instruction component" );

    i.accept = true;
}

void LEXNDL::keep ( uns32 n )
{
    FUNCTION ( "keep" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "accept() misplaced" );
    substate = INSTRUCTION;

    instruction & i = current_instruction();
    ASSERT ( ! i.accept,
             "keep() conflicts with  accept()" );
    ASSERT ( i.operation & LEX::KEEP_FLAG == 0,
             "keep() conflicts with another keep()" );
    ASSERT ( n <= LEX::KEEP_LENGTH_MASK,
             "keep() length too large" );

    i.operation |= LEX::KEEP ( n );
}

void LEXNDL::translate
	( const char * translation_string )
{
    uns32 length = strlen ( translation_string );
    uns32 buffer[length];
    for ( uns32 i = 0; i < length; ++ i )
        buffer[i] = translation_string[i];
    LEXNDL::translate ( length, buffer );
}
void LEXNDL::translate
	( uns32 n, const uns32 * translation_string )
{
    FUNCTION ( "translate" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "translate() misplaced" );
    substate = INSTRUCTION;

    instruction & i = current_instruction();
    ASSERT ( ! i.accept,
             "translate() conflicts with  accept()" );
    ASSERT (   i.operation
             & (   LEX::TRANSLATE_FLAG
	         | LEX::TRANSLATE_OCT_FLAG
	         | LEX::TRANSLATE_HEX_FLAG )
	     == 0,
             "translate() conflicts with another"
	     " translate_...()" );
    ASSERT ( n <= LEX::TRANSLATE_LENGTH_MASK,
             "translate() translation_string length"
	     " too large" );
    if ( n > 0 )
	ASSERT ( translation_string != NULL,
	         "n > 0 but translation_string"
		 " == NULL" );

    i.operation |= LEX::TRANSLATE ( n );
    while ( n -- )
        push_uns32 ( * translation_string ++ );
}

void LEXNDL::translate_oct ( uns32 m, uns32 n )
{
    FUNCTION ( "translate_oct" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "translate_oct() misplaced" );
    substate = INSTRUCTION;

    instruction & i = current_instruction();
    ASSERT ( ! i.accept,
             "translate_oct() conflicts with"
	     " accept()" );
    ASSERT (   i.operation
             & (   LEX::TRANSLATE_FLAG
	         | LEX::TRANSLATE_OCT_FLAG
	         | LEX::TRANSLATE_HEX_FLAG )
	     == 0,
             "translate_oct() conflicts with another"
	     " translate_...()" );
    ASSERT ( m <= LEX::PREFIX_LENGTH_MASK,
             "translate_oct() prefix length"
	     " too large" );
    ASSERT ( n <= LEX::POSTFIX_LENGTH_MASK,
             "translate_oct() postfix length"
	     " too large" );

    i.operation |= LEX::TRANSLATE_OCT ( m, n );
}

void LEXNDL::translate_hex ( uns32 m, uns32 n )
{
    FUNCTION ( "translate_hex" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "translate_hex() misplaced" );
    substate = INSTRUCTION;

    instruction & i = current_instruction();
    ASSERT ( ! i.accept,
             "translate_hex() conflicts with"
	     " accept()" );
    ASSERT (   i.operation
             & (   LEX::TRANSLATE_FLAG
	         | LEX::TRANSLATE_OCT_FLAG
	         | LEX::TRANSLATE_HEX_FLAG )
	     == 0,
             "translate_hex() conflicts with another"
	     " translate_...()" );
    ASSERT ( m <= LEX::PREFIX_LENGTH_MASK,
             "translate_hex() prefix length"
	     " too large" );
    ASSERT ( n <= LEX::POSTFIX_LENGTH_MASK,
             "translate_hex() postfix length"
	     " too large" );

    i.operation |= LEX::TRANSLATE_HEX ( m, n );
}

void LEXNDL::erroneous_atom ( uns32 type_name )
{
    FUNCTION ( "erroneous_atom" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "erroneous_atom() misplaced" );
    substate = INSTRUCTION;

    instruction & i = current_instruction();
    ASSERT ( ! i.accept,
             "erroneous_atom() conflicts with"
	     " accept()" );
    ASSERT ( i.operation & LEX::ERRONEOUS_ATOM == 0,
             "erroneous_atom() conflicts with another"
	     " erroneous_atom()" );
    ASSERT ( i.operation & LEX::OUTPUT == 0,
             "erroneous_atom() conflicts with"
	     " output()" );

    i.operation |= LEX::ERRONEOUS_ATOM;
    i.type = type_name;
}

void LEXNDL::output ( uns32 type_name )
{
    FUNCTION ( "output" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "output() misplaced" );
    substate = INSTRUCTION;

    instruction & i = current_instruction();
    ASSERT ( ! i.accept,
             "output() conflicts with accept()" );
    ASSERT ( i.operation & LEX::OUTPUT == 0,
             "output() conflicts with another"
	     " output()" );
    ASSERT ( i.operation & LEX::ERRONEOUS_ATOM == 0,
             "output() conflicts with"
	     " erroneous_atom()" );

    i.operation |= LEX::OUTPUT;
    i.type = type_name;
}

void LEXNDL::go ( uns32 atom_table_name )
{
    FUNCTION ( "go" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "go() misplaced" );
    substate = INSTRUCTION;

    instruction & i = current_instruction();
    ASSERT ( ! i.accept,
             "go() conflicts with accept()" );
    ASSERT (   i.operation
             & (   LEX::GOTO
	         | LEX::CALL_FLAG
	         | LEX::RETURN_FLAG )
	     == 0,
             "go() conflicts with another go() or"
	     " with call() or ret()" );

    ASSERT (    LEX::program[atom_table_name]
             == LEXDATA::ATOM_TABLE,
             "atom_table_name does not reference an"
	     " atom table" );

    i.operation |= LEX::GOTO;
    i.atom_table_ID = atom_table_name;
}

void LEXNDL::call ( uns32 atom_table_name,
		    uns32 n,
		    const uns32 * return_vector )
{
    FUNCTION ( "call" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "call() misplaced" );
    substate = INSTRUCTION;

    instruction & i = current_instruction();
    ASSERT ( ! i.accept,
             "call() conflicts with accept()" );
    ASSERT (   i.operation
             & (   LEX::GOTO
	         | LEX::CALL_FLAG
	         | LEX::RETURN_FLAG )
	     == 0,
             "call() conflicts with another call() or"
	     " with ret() or go()" );

    ASSERT ( n <= LEX::CALL_LENGTH_MASK,
             "return_vector size n is too large" );
    if ( n > 0 )
	ASSERT ( return_vector != NULL,
	         "n > 0 but return_vector == NULL" );

    ASSERT (    LEX::program[atom_table_name]
             == LEXDATA::ATOM_TABLE,
             "atom_table_name does not reference an"
	     " atom table" );
    for ( uns32 i = 0; i < n; ++ i )
	ASSERT (    LEX::program[return_vector[i]]
		 == LEXDATA::ATOM_TABLE,
		 MESSAGE ( "return_vector[%d] does not"
		           " reference an atom table",
			   i ) );

    i.operation |= LEX::CALL(n);
    i.atom_table_ID = atom_table_name;
    for ( uns32 i = 0; i < n; ++ i )
        push_uns32 ( * return_vector ++ );
}

void LEXNDL::ret ( uns32 i )
{
    FUNCTION ( "ret" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "ret() misplaced" );
    substate = INSTRUCTION;

    instruction & ci = current_instruction();
    ASSERT ( ! ci.accept,
             "ret() conflicts with accept()" );
    ASSERT (   ci.operation
             & (   LEX::GOTO
	         | LEX::CALL_FLAG
	         | LEX::RETURN_FLAG )
	     == 0,
             "ret() conflicts with another ret() or"
	     " with call() or go()" );
    ASSERT ( i <= LEX::RETURN_INDEX_MASK,
             "return index i is too large" );

    ci.operation |= LEX::RETURN(i);
    ci.atom_table_ID = 0;
}

void LEXNDL::else_if_not
	( uns32 character_pattern_name )
{
    FUNCTION ( "else_if_not" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "else_if_not() misplaced" );
    substate = INSTRUCTION;

    instruction & i = current_instruction();
    ASSERT (   i.operation
             & (   LEX::TRANSLATE_OCT_FLAG
	         | LEX::TRANSLATE_HEX_FLAG )
	     != 0,
             "else_if_not() is not after a"
	     " translate_oct/hex()" );
    ASSERT (    LEX::program[character_pattern_name]
             == LEXDATA::DISPATCHER,
             "atom_table_name does not reference a"
	     " character_pattern" );

    instructions.allocate ( 1 );
    instruction & i2 = current_instruction();
    i2.operation = 0;
    i2.atom_table_ID = 0;
    i2.type = 0;
    i2.else_dispatcher_ID = character_pattern_name;
    i2.accept = false;
}
