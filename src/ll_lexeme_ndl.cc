// Layers Language Lexical Nested Description Language
//
// File:	ll_lexeme_ndl.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Nov 21 22:05:54 EST 2010
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
# include <iostream>
# include <cstring>
# include <cassert>
# define LEX ll::lexeme
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
//	ASSERT(<test>,<format>,...) signals an error if
//	    the <test> expression is false.  The
//	    error message includes the function name,
//	    file name and line number of the call as
//	    stored in LEXNDL::file/line by the NDL
//	    macro, an error message produced by passing
//	    <format>,... to sprintf, and the submessage
//	    `( <test> FAILED! )'.
//	ATTACH(...) executes `LEX::attach(...)' and
//	    issues an error message if this fails.
//
# define FUNCTION(name) function_name = name
# define ASSERT(test,...) \
    ( (test) ? true : \
      ::error ( #test, \
                ( sprintf ( message_buffer, \
		            __VA_ARGS__ ), \
		  message_buffer ) ) )
# define ATTACH(...) ( LEX::attach(__VA_ARGS__) ? 0 : \
    attach_error ( __FILE__, __LINE__ ) )

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
	 << "    (" << test << " FAILED! )" <<endl;
    exit ( 1 );
}

// Called by ATTACH macro to print attach failure
// error message.
//
static int attach_error
	( const char * file, uns32 line )
{
    cout << "NDL SYSTEM ERROR: ATTACH FAILED: FILE: "
         << file << " LINE: " << line << endl
	 << "   CALLED FROM FILE: " << LEXNDL::file
	 << " LINE: " << LEXNDL::line << endl
	 << LEX::error_message << endl
	 << "    NOTE: possible causes:" << endl
	 << "          begin_table ( name ) called"
	             " twice for a given name" << endl;
    exit ( 1 );
}

// `state' and `substate' are used to verify NDL
// function execution order.

enum STATE {
    OUTSIDE_PROGRAM,
    INSIDE_PROGRAM,
    INSIDE_ATOM_PATTERN,
    INSIDE_TABLE };

static STATE state;

// Substates in the order they appear inside a
// table, atom pattern, or dispatcher.
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

void LEXNDL::new_table
	( uns32 & table_name, uns32 mode )
{
    FUNCTION ( "new_table" );
    ASSERT ( state == INSIDE_PROGRAM,
             "new_table() misplaced" );
    table_name = LEX::create_table ( mode );
}

// Push a new dispatcher and instruction on the
// dispatchers and instructions stacks.  Set substate
// to ADD_CHARACTERS.  If there is a parent dispatcher,
// increment its max_type_code.
//
// However, if is_others is true, set the substate to
// DISPATCHERS (add_characters is not allowed in an
// OTHER's dispatcher), do NOT increment the parent
// max_type_code, and set is_others_dispatcher true
// in the pushed dispatcher.
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
    instruction & ci = current_instruction();

    ci.operation = 0;
    ci.atom_table_ID = 0;
    ci.require_dispatcher_ID = 0;
    ci.output_error_type = 0;
    ci.goto_call_table_ID = 0;
    ci.accept = false;

    substate = is_others ? DISPATCHERS
                         : ADD_CHARACTERS;
}

// Pop an instruction group from the instructions stack
// and return the instruction_ID of its first instruc-
// tion, or return 0 if there is no instruction group.
//
static uns32 pop_instruction_group ( void )
{
    uns32 instruction_ID = 0;
    while ( true )
    {
	instruction & ci = current_instruction();

	uns32 translate_to_length = 0;
	if ( ci.operation & LEX::TRANSLATE_TO_FLAG )
	    translate_to_length =
	        LEX::translate_to_length
		    ( ci.operation );

        uns32 * translation_vector = NULL;
	uns32 length = uns32_stack.length;
	assert ( translate_to_length <= length );
	if ( translate_to_length > 0 )
	    translation_vector =
	        & uns32_stack
		    [length - translate_to_length];

	if ( ci.operation != 0 || ci.accept )
	    instruction_ID = LEX::create_instruction
		( ci.operation,
		  translation_vector,
		  ci.atom_table_ID,
		  ci.require_dispatcher_ID,
		  instruction_ID,
		  ci.output_error_type,
		  ci.goto_call_table_ID );

	if ( translate_to_length > 0 )
	    uns32_stack.deallocate
	        ( translate_to_length );

	instructions.deallocate ( 1 );

	if ( instructions.length == 0
	     ||
	     ! ( current_instruction().operation
	         & LEX::ELSE ) )
	    break;

	ASSERT ( instruction_ID != 0,
		 "no instruction after ELSE()" );
    }
    return instruction_ID;
}

// Pop a dispatcher from the dispatchers stack.  This
// calls LEX::create_dispatcher which returns the new
// dispatcher_ID.  See stack description in ll_lexeme_
// ndl_data.h.  Set substate to DISPATCHERS.
//
// If discard_dispatcher is true the dispatcher is not
// created but is discarded, but its added characters
// are mapped to the character type the dispatcher would
// have had (used by end_atom_pattern).
//
static uns32 pop_dispatcher ( bool discard_dispatcher = false )
{
    uns32 instruction_ID = pop_instruction_group();

    dispatcher & d = current_dispatcher();

    uns32 cmin = 0;
    uns32 cmax = 127;
    while ( cmin <= cmax && d.ascii_map[cmin] == 0 )
        ++ cmin;
    while ( cmin <= cmax && d.ascii_map[cmax] == 0 )
        -- cmax;
    bool ascii_used = ( cmin <= cmax );

    if ( discard_dispatcher )
    {
        assert ( d.max_type_code == 0 );
        assert ( instruction_ID == 0 );
	assert ( ! ascii_used );
	assert ( d.others_dispatcher_ID == 0 );
	assert ( d.others_instruction_ID == 0 );

	push_uns32 ( 0 );
	push_uns32 ( instruction_ID );
	push_uns32 ( d.type_map_count );

	dispatchers.deallocate ( 1 );

	substate = DISPATCHERS;
    	return 0;
    }

    uns32 total_type_map_count = 0;
    uns32 * p = & uns32_stack[uns32_stack.length];
    for ( uns32 tcode = d.max_type_code;
          0 < tcode; -- tcode )
    {
        uns32 sub_type_map_count = p[-1];
        total_type_map_count += sub_type_map_count;
	p -= 3 + 2 * sub_type_map_count;
    }

    uns32 dispatcher_ID =
        LEX::create_dispatcher
	    ( 2 * ( total_type_map_count + ascii_used ),
	      d.max_type_code );

    if ( ascii_used )
        ATTACH ( dispatcher_ID,
	         LEX::create_type_map
		     ( cmin, cmax,
		       d.ascii_map + cmin ) );

    if ( d.others_dispatcher_ID != 0 )
        ATTACH ( dispatcher_ID, 0,
	         d.others_dispatcher_ID );
    if ( d.others_instruction_ID != 0 )
        ATTACH ( dispatcher_ID, 0,
	         d.others_instruction_ID );

    for ( uns32 tcode = d.max_type_code;
          0 < tcode; -- tcode )
    {
        uns32 sub_type_map_count = pop_uns32();
	uns32 sub_instruction_ID = pop_uns32();
	uns32 sub_dispatcher_ID = pop_uns32();

	// In the special case of an atom pattern
	// sub_dispatcher_ID == 0 is possible.
	//
	if ( sub_dispatcher_ID != 0 )
	    ATTACH ( dispatcher_ID, tcode,
		     sub_dispatcher_ID );

	if ( sub_instruction_ID != 0 )
	    ATTACH ( dispatcher_ID, tcode,
		     sub_instruction_ID );

	for ( uns32 i = 0;
	      i < sub_type_map_count; ++ i )
	{
	    uns32 max_char = pop_uns32();
	    uns32 min_char = pop_uns32();
	    ATTACH ( dispatcher_ID,
	             LEX::create_type_map
		        ( min_char, max_char, tcode ) );
	}
    }

    if ( d.is_others_dispatcher )
    {
	dispatcher & parent = parent_dispatcher();
	parent.others_dispatcher_ID = dispatcher_ID;
	parent.others_instruction_ID = instruction_ID;
	assert ( d.type_map_count == 0 );
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

static uns32 table_name;
static uns32 table_mode;
void LEXNDL::begin_table ( uns32 table_name )
{
    FUNCTION ( "begin_table" );
    ASSERT ( state == INSIDE_PROGRAM,
             "begin_table() misplaced" );
    state = INSIDE_TABLE;
    ::table_name = table_name;

    ASSERT (    component_type ( table_name )
             == LEX::TABLE,
             "table_name does not reference a table" );
    ::table_mode = LEX::table_mode ( table_name );

    assert ( dispatchers.length == 0 );
    assert ( instructions.length == 0 );
    push_dispatcher();
    substate = DISPATCHERS;
}

void LEXNDL::end_table ( void )
{
    FUNCTION ( "end_table" );
    ASSERT ( state == INSIDE_TABLE,
             "end_table() misplaced" );
    ASSERT ( dispatchers.length == 1,
             "end_table() misplaced" );

    pop_dispatcher();

    uns32 type_map_count = pop_uns32();
    uns32 instruction_ID = pop_uns32();
    uns32 dispatcher_ID = pop_uns32();

    assert ( dispatchers.length == 0 );
    assert ( instructions.length == 0 );
    assert ( uns32_stack.length == 0 );

    ATTACH ( table_name, dispatcher_ID );
    if ( instruction_ID != 0 )
	ATTACH ( table_name, instruction_ID );
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
	         "character %c in use by"
		 " previous dispatcher",
	         (char) c );
	d.ascii_map[c] = d.max_type_code;
    }
}

static uns32 * atom_pattern_name_p;
void LEXNDL::begin_atom_pattern
	( uns32 & atom_pattern_name,
	  const char * include_chars,
	  const char * exclude_chars )
{
    FUNCTION ( "begin_atom_pattern" );
    ASSERT ( state == INSIDE_PROGRAM,
             "begin_atom_pattern() misplaced" );
    state = INSIDE_ATOM_PATTERN;

    push_dispatcher();
    push_dispatcher();
    internal_add_characters
        ( include_chars, exclude_chars );

    atom_pattern_name_p = & atom_pattern_name;
}

void LEXNDL::NEXT
	( const char * include_chars,
	  const char * exclude_chars )
{
    FUNCTION ( "NEXT" );
    ASSERT ( state == INSIDE_ATOM_PATTERN,
             "NEXT() misplaced" );

    push_dispatcher();
    internal_add_characters
	( include_chars, exclude_chars );
}

void LEXNDL::end_atom_pattern ( void )
{
    FUNCTION ( "end_atom_pattern" );
    ASSERT ( state == INSIDE_ATOM_PATTERN,
             "end_atom_pattern() misplaced" );

    pop_dispatcher ( true );

    while ( dispatchers.length > 0)
	pop_dispatcher();

    * atom_pattern_name_p = uns32_stack[0];

    uns32_stack.deallocate ( 3 );

    assert ( dispatchers.length == 0 );
    assert ( instructions.length == 0 );
    assert ( uns32_stack.length == 0 );

    state = INSIDE_PROGRAM;
}

void LEXNDL::add_characters
	( const char * include_chars,
	  const char * exclude_chars )
{
    FUNCTION ( "add_characters" );
    ASSERT ( state == INSIDE_TABLE
             ||
	     state == INSIDE_ATOM_PATTERN,
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
    ASSERT ( state == INSIDE_TABLE
             ||
	     state == INSIDE_ATOM_PATTERN,
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
    ASSERT ( state == INSIDE_TABLE,
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
    ASSERT ( state == INSIDE_TABLE,
             "end_dispatch() misplaced" );
    ASSERT ( dispatchers.length >= 2,
             "end_dispatch() misplaced" );
    pop_dispatcher();
}

void LEXNDL::accept ( void )
{
    FUNCTION ( "accept" );
    ASSERT ( state == INSIDE_TABLE,
             "accept() misplaced" );
    substate = ::INSTRUCTION;

    instruction & ci = current_instruction();
    ASSERT ( ! ci.accept,
             "accept() conflicts with another"
	     " accept()" );
    ASSERT ( ci.operation == 0,
             "accept() conflicts with another"
	     " instruction component" );

    ci.accept = true;
}

void LEXNDL::keep ( uns32 n )
{
    FUNCTION ( "keep" );
    ASSERT ( state == INSIDE_TABLE,
             "accept() misplaced" );
    substate = ::INSTRUCTION;

    instruction & ci = current_instruction();
    ASSERT ( ! ci.accept,
             "keep() conflicts with accept()" );
    ASSERT ( ! ( ci.operation & LEX::KEEP_FLAG ),
             "keep() conflicts with another keep()" );
    ASSERT ( n <= LEX::KEEP_LENGTH_MASK,
             "keep() length (%d) too large (> %d)",
	     n, LEX::KEEP_LENGTH_MASK );

    ci.operation |= LEX::KEEP ( n );
}

void LEXNDL::translate_to
	( const char * translation_string )
{
    uns32 length = strlen ( translation_string );
    uns32 buffer[length];
    for ( uns32 i = 0; i < length; ++ i )
        buffer[i] = (uns8) translation_string[i];
    LEXNDL::translate_to ( length, buffer );
}
void LEXNDL::translate_to
	( uns32 n, const uns32 * translation_string )
{
    FUNCTION ( "translate_to" );
    ASSERT ( state == INSIDE_TABLE,
             "translate_to() misplaced" );
    substate = ::INSTRUCTION;

    instruction & ci = current_instruction();
    ASSERT ( ! ci.accept,
             "translate_to() conflicts with accept()" );
    ASSERT ( ! (   ci.operation
                 & (   LEX::TRANSLATE_TO_FLAG
	             | LEX::TRANSLATE_OCT_FLAG
	             | LEX::TRANSLATE_HEX_FLAG ) ),
             "translate_to() conflicts with another"
	     " translate...()" );
    ASSERT ( n <= LEX::TRANSLATE_TO_LENGTH_MASK,
             "translate_to() translation_string"
	     " length (%d) too large (> %d)",
	     n, LEX::TRANSLATE_TO_LENGTH_MASK );
    if ( n > 0 )
	ASSERT ( translation_string != NULL,
	         "translate_to n > 0 but"
		 " translation_string == NULL" );

    ci.operation |= LEX::TRANSLATE_TO ( n );
    while ( n -- )
        push_uns32 ( * translation_string ++ );
}

void LEXNDL::translate_oct ( uns32 m, uns32 n )
{
    FUNCTION ( "translate_oct" );
    ASSERT ( state == INSIDE_TABLE,
             "translate_oct() misplaced" );
    substate = ::INSTRUCTION;

    instruction & ci = current_instruction();
    ASSERT ( ! ci.accept,
             "translate_oct() conflicts with"
	     " accept()" );
    ASSERT ( ! (   ci.operation
                 & (   LEX::TRANSLATE_TO_FLAG
	             | LEX::TRANSLATE_OCT_FLAG
	             | LEX::TRANSLATE_HEX_FLAG ) ),
             "translate_oct() conflicts with another"
	     " translate...()" );
    ASSERT ( m <= LEX::PREFIX_LENGTH_MASK,
             "translate_oct() prefix length (%d)"
	     " too large (> %d)",
	     m, LEX::PREFIX_LENGTH_MASK );
    ASSERT ( n <= LEX::POSTFIX_LENGTH_MASK,
             "translate_oct() postfix length (%d)"
	     " too large (> %d)",
	     n, LEX::POSTFIX_LENGTH_MASK );

    ci.operation |= LEX::TRANSLATE_OCT ( m, n );
}

void LEXNDL::translate_hex ( uns32 m, uns32 n )
{
    FUNCTION ( "translate_hex" );
    ASSERT ( state == INSIDE_TABLE,
             "translate_hex() misplaced" );
    substate = ::INSTRUCTION;

    instruction & ci = current_instruction();
    ASSERT ( ! ci.accept,
             "translate_hex() conflicts with"
	     " accept()" );
    ASSERT ( ! (   ci.operation
                 & (   LEX::TRANSLATE_TO_FLAG
	             | LEX::TRANSLATE_OCT_FLAG
	             | LEX::TRANSLATE_HEX_FLAG ) ),
             "translate_hex() conflicts with another"
	     " translate...()" );
    ASSERT ( m <= LEX::PREFIX_LENGTH_MASK,
             "translate_hex() prefix length (%d)"
	     " too large (> %d)",
	     m, LEX::PREFIX_LENGTH_MASK );
    ASSERT ( n <= LEX::POSTFIX_LENGTH_MASK,
             "translate_hex() postfix length (%d)"
	     " too large (> %d)",
	     n, LEX::POSTFIX_LENGTH_MASK );

    ci.operation |= LEX::TRANSLATE_HEX ( m, n );
}

void LEXNDL::match ( uns32 table_name )
{
    FUNCTION ( "MATCH" );
    ASSERT ( state == INSIDE_TABLE,
             "match() misplaced" );
    substate = ::INSTRUCTION;

    instruction & ci = current_instruction();

    ASSERT ( ! ci.accept,
             "match() conflicts with accept()" );

    ASSERT (    LEX::component_type ( table_name )
             == LEX::TABLE,
             "table_name does not reference a table" );

    ci.operation |= LEX::MATCH;
    ci.atom_table_ID = table_name;
}

void LEXNDL::require ( uns32 atom_pattern_name )
{
    FUNCTION ( "REQUIRE" );
    ASSERT ( state == INSIDE_TABLE,
             "require() misplaced" );
    substate = ::INSTRUCTION;

    ASSERT (    LEX::component_type
                     ( atom_pattern_name )
             == LEX::DISPATCHER,
             "atom_pattern_name does not reference an"
	     " atom pattern" );

    instruction & ci = current_instruction();
    ASSERT ( ! ci.accept,
             "require() conflicts with accept()" );
    ASSERT ( ! ( ci.operation & REQUIRE ),
             "multiple require()'s in instruction" );
    ASSERT (   ci.operation
             & (   LEX::TRANSLATE_OCT_FLAG
	         | LEX::TRANSLATE_HEX_FLAG
	         | LEX::MATCH ),
             "require() is not after a match() or"
	     " translate_oct/hex()" );

    ci.operation |= LEX::REQUIRE;
    ci.require_dispatcher_ID = atom_pattern_name;
}

void LEXNDL::erroneous_atom ( uns32 type_name )
{
    FUNCTION ( "erroneous_atom" );
    ASSERT ( state == INSIDE_TABLE,
             "erroneous_atom() misplaced" );
    ASSERT ( ::table_mode != LEX::ATOM,
             "erroneous_atom() in atom table" );
    substate = ::INSTRUCTION;

    instruction & ci = current_instruction();
    ASSERT ( ! ci.accept,
             "erroneous_atom() conflicts with"
	     " accept()" );
    ASSERT ( ! ( ci.operation & LEX::ERRONEOUS_ATOM ),
             "erroneous_atom() conflicts with another"
	     " erroneous_atom()" );
    ASSERT ( ! ( ci.operation & LEX::OUTPUT ),
             "erroneous_atom() conflicts with"
	     " output()" );

    ci.operation |= LEX::ERRONEOUS_ATOM;
    ci.output_error_type = type_name;
}

void LEXNDL::output ( uns32 type_name )
{
    FUNCTION ( "output" );
    ASSERT ( state == INSIDE_TABLE,
             "output() misplaced" );
    ASSERT ( ::table_mode != LEX::ATOM,
             "output() in atom table" );
    substate = ::INSTRUCTION;

    instruction & ci = current_instruction();
    ASSERT ( ! ci.accept,
             "output() conflicts with accept()" );
    ASSERT ( ! ( ci.operation & LEX::OUTPUT ),
             "output() conflicts with another"
	     " output()" );
    ASSERT ( ! ( ci.operation & LEX::ERRONEOUS_ATOM ),
             "output() conflicts with"
	     " erroneous_atom()" );

    ci.operation |= LEX::OUTPUT;
    ci.output_error_type = type_name;
}

void LEXNDL::go ( uns32 table_name )
{
    FUNCTION ( "go" );
    ASSERT ( state == INSIDE_TABLE,
             "go() misplaced" );
    ASSERT ( ::table_mode != LEX::ATOM,
             "goto() in atom table" );
    substate = ::INSTRUCTION;

    instruction & ci = current_instruction();
    ASSERT ( ! ci.accept,
             "go() conflicts with accept()" );
    ASSERT ( ! (   ci.operation
                 & (   LEX::GOTO
	             | LEX::CALL
	             | LEX::RETURN ) ),
             "go() conflicts with another go() or"
	     " with call() or ret()" );

    ASSERT (    LEX::component_type ( table_name )
             == LEX::TABLE,
             "table_name does not reference an"
	     " table" );
    ASSERT (    LEX::table_mode ( table_name )
             != LEX::ATOM,
             "table_name references an atom"
	     " table" );

    ci.operation |= LEX::GOTO;
    ci.goto_call_table_ID = table_name;
}

void LEXNDL::call ( uns32 table_name )
{
    FUNCTION ( "call" );
    ASSERT ( state == INSIDE_TABLE,
             "call() misplaced" );
    ASSERT ( ::table_mode != LEX::ATOM,
             "call() in atom table" );
    substate = ::INSTRUCTION;

    instruction & ci = current_instruction();
    ASSERT ( ! ci.accept,
             "call() conflicts with accept()" );
    ASSERT ( ! (   ci.operation
                 & (   LEX::GOTO
	             | LEX::CALL
	             | LEX::RETURN ) ),
             "call() conflicts with another call() or"
	     " with ret() or go()" );

    ASSERT (    LEX::component_type ( table_name )
             == LEX::TABLE,
             "table_name does not reference an"
	     " table" );
    ASSERT (    LEX::table_mode ( table_name )
             != LEX::MASTER,
             "table_name references a MASTER"
	     " table" );
    ASSERT (    LEX::table_mode ( table_name )
             != LEX::ATOM,
             "table_name references an atom"
	     " table" );
    ci.operation |= LEX::CALL;
    ci.goto_call_table_ID = table_name;
}

void LEXNDL::ret ( void )
{
    FUNCTION ( "ret" );
    ASSERT ( state == INSIDE_TABLE,
             "ret() misplaced" );
    ASSERT ( ::table_mode != LEX::ATOM,
             "ret() in atom table" );
    substate = ::INSTRUCTION;

    instruction & ci = current_instruction();
    ASSERT ( ! ci.accept,
             "ret() conflicts with accept()" );
    ASSERT ( ! (   ci.operation
                 & (   LEX::GOTO
	             | LEX::CALL
	             | LEX::RETURN ) ),
             "ret() conflicts with another ret() or"
	     " with call() or go()" );

    ci.operation |= LEX::RETURN;
}

void LEXNDL::fail ( void )
{
    FUNCTION ( "fail" );
    ASSERT ( state == INSIDE_TABLE,
             "fail() misplaced" );
    ASSERT ( ::table_mode == LEX::ATOM,
             "fail() in non-atom table" );
    substate = ::INSTRUCTION;

    instruction & ci = current_instruction();
    ASSERT ( ! ci.accept,
             "fail() conflicts with accept()" );
    ASSERT ( ! ( ci.operation & LEX::FAIL ),
             "fail() conflicts with another fail()" );

    ci.operation |= LEX::FAIL;
}

void LEXNDL::ELSE ( void )
{
    FUNCTION ( "ELSE" );
    ASSERT ( state == INSIDE_TABLE,
             "ELSE() misplaced" );
    substate = ::INSTRUCTION;

    instruction & ci = current_instruction();
    ASSERT (   ci.operation
             & (   LEX::TRANSLATE_OCT_FLAG
	         | LEX::TRANSLATE_HEX_FLAG
	         | LEX::MATCH
		 | LEX::REQUIRE ),
             "ELSE() is not after a match(),"
	     " translate_oct/hex(), or require()" );
    ci.operation |= LEX::ELSE;

    instructions.allocate ( 1 );
    instruction & i2 = current_instruction();
    i2.operation = 0;
    i2.atom_table_ID = 0;
    i2.require_dispatcher_ID = 0;
    i2.output_error_type = 0;
    i2.goto_call_table_ID = 0;
    i2.accept = false;
}
