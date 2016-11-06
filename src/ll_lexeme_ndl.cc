// Layered Languages Lexical Nested Description Language
//
// File:	ll_lexeme_ndl.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Nov  6 03:58:12 EST 2016
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Data
//	NDL Functions

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_ndl.h>
# include <ll_lexeme_ndl_data.h>
# include <iostream>
# include <cstdlib>
# include <cstdio>
# include <cstring>
# define LEX ll::lexeme
# define LEXNDL ll::lexeme::ndl
# define LEXNDLDATA ll::lexeme::ndl::data
using std::cout;
using std::endl;
using namespace LEXNDLDATA;

// Data
// ----

// Stacks

static min::packed_vec<LEX::uns8>
       uns8_vec_type
           ( "ll::lexeme::ndl::uns8_vec_type" );
static min::packed_vec<LEX::uns32>
       uns32_vec_type
           ( "ll::lexeme::ndl::uns32_vec_type" );
static min::packed_vec<LEXNDLDATA::dispatcher>
       dispatcher_vec_type
           ( "ll::lexeme::ndl::dispatcher_vec_type" );
static min::packed_vec<LEXNDLDATA::instruction>
       instruction_vec_type
           ( "ll::lexeme::ndl::instruction_vec_type" );

min::packed_vec_insptr<LEX::uns8>
    LEXNDLDATA::uns8_stack;
min::packed_vec_insptr<LEX::uns32>
    LEXNDLDATA::uns32_stack;
min::packed_vec_insptr<LEXNDLDATA::dispatcher>
    LEXNDLDATA::dispatchers;
min::packed_vec_insptr<LEXNDLDATA::instruction>
    LEXNDLDATA::instructions;

static void initialize ( void )
{
    LEXNDLDATA::uns8_stack =
        uns8_vec_type.new_gen();
    LEXNDLDATA::uns32_stack =
        uns32_vec_type.new_gen();
    LEXNDLDATA::dispatchers =
	dispatcher_vec_type.new_gen();
    LEXNDLDATA::instructions =
	instruction_vec_type.new_gen();
}
static min::initializer initializer ( ::initialize );

// Other global data.
//
char LEXNDL::OTHERS[1];

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
//	SET_REPEAT_COUNT(...) executes `LEX::set_repeat_
//	    count(...)' and
//
# define FUNCTION(name) function_name = name
# define ASSERT(test,...) \
    ( (test) ? true : \
      ::error ( #test, \
                ( sprintf ( message_buffer, \
		            __VA_ARGS__ ), \
		  message_buffer ) ) )
# define ATTACH(...) \
    ( LEX::attach(__VA_ARGS__) ? 0 : \
    attach_error ( __FILE__, __LINE__ ) )
# define SET_REPEAT_COUNT(...) \
    ( LEX::set_repeat_count(__VA_ARGS__) ? 0 : \
    set_repeat_count_error ( __FILE__, __LINE__ ) )

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
	 << min::error_message
	 << "    NOTE: possible cause:" << endl
	 << "          begin_table ( name ) called"
	             " twice for a given name" << endl;
    exit ( 1 );
}

// Called by SET_REPEAT_COUNT macro to print attach
// failure error message.
//
static int set_repeat_count_error
	( const char * file, uns32 line )
{
    cout << "NDL SYSTEM ERROR: SET_REPEAT_COUNT FAILED:"
            " FILE: "
         << file << " LINE: " << line << endl
	 << "   CALLED FROM FILE: " << LEXNDL::file
	 << " LINE: " << LEXNDL::line << endl
	 << min::error_message
	 << "    NOTE: possible cause:" << endl
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
    ADD_CHARACTERS,	// For non-OTHERS dispatchers
    ADD_REPEAT,		// For OTHERS dispatchers
    DISPATCHERS,
    INSTRUCTION };

static SUBSTATE substate;

// Depth of dispatcher is dispatchers->length.

// Return current or parent dispatcher or instruction.
//
inline min::ref<dispatcher> current_dispatcher
	( void )
{
    uns32 dindex = dispatchers->length;
    MIN_REQUIRE ( dindex >= 1 );
    return dispatchers[dindex-1];
}
inline min::ref<dispatcher> parent_dispatcher
	( void )
{
    uns32 dindex = dispatchers->length;
    MIN_REQUIRE ( dindex >= 2 );
    return dispatchers[dindex-2];
}
inline min::ref<instruction> current_instruction
	( void )
{
    uns32 iindex = instructions->length;
    MIN_REQUIRE ( iindex >= 1 );
    return instructions[iindex-1];
}



// NDL Functions
// --- ---------

void LEXNDL::begin_program
	( const char * const * master_names,
	  uns32 max_master,
	  const char * const * type_names,
	  const char * type_codes,
	  uns32 max_type )
{
    FUNCTION ( "begin_program" );
    ASSERT ( state == OUTSIDE_PROGRAM,
             "misplaced begin_program(...)" );
    state = INSIDE_PROGRAM;

    min::pop ( uns8_stack, uns8_stack->length );
    min::resize ( uns8_stack, 10 * CTYPE_MAP_SIZE );
    min::pop ( uns32_stack, uns32_stack->length );
    min::resize ( uns32_stack, 4000 );
    min::pop ( dispatchers, dispatchers->length );
    min::resize ( dispatchers, 20 );
    min::pop ( instructions, instructions->length );
    min::resize ( instructions, 40 );

    LEX::create_program
        ( LEXNDL::line,
	  master_names, max_master,
	  type_names, type_codes, max_type );
}

void LEXNDL::end_program ( void )
{
    FUNCTION ( "end_program" );
    ASSERT ( state == INSIDE_PROGRAM,
             "misplaced end_program()" );
    state = OUTSIDE_PROGRAM;

    MIN_REQUIRE ( uns8_stack->length == 0 );
    MIN_REQUIRE ( uns32_stack->length == 0 );
    MIN_REQUIRE ( dispatchers->length == 0 );
    MIN_REQUIRE ( instructions->length == 0 );
}

void LEXNDL::new_table
	( uns32 & table_name, uns32 mode )
{
    FUNCTION ( "new_table" );
    ASSERT ( state == INSIDE_PROGRAM,
             "new_table() misplaced" );
    table_name =
        LEX::create_table ( LEXNDL::line, mode );
}

// Push a new dispatcher and instruction on the
// dispatchers and instructions stacks.  Set substate
// to ADD_CHARACTERS.  If there is a parent dispatcher,
// increment its max_type_code.
//
// However, if is_others is true, set the substate to
// ADD_REPEAT (add_characters is not allowed in an
// OTHERS's dispatcher), do NOT increment the parent
// max_type_code, and set is_others_dispatcher true
// in the pushed dispatcher.
//
static void push_dispatcher ( bool is_others = false )
{
    if ( dispatchers->length > 0 && ! is_others )
        ++ (&current_dispatcher())->max_type_code;

    min::ref<dispatcher> d =
        min::push ( dispatchers );

    (&d)->line_number = LEXNDL::line;
    (&d)->max_type_code = 0;
    (&d)->repeat_count = 0;
    (&d)->others_dispatcher_ID = 0;
    (&d)->others_instruction_ID = 0;
    (&d)->others_repeat_count = 0;
    (&d)->is_others_dispatcher = is_others;
    (&d)->ctype_map_offset = uns8_stack->length;

    min::push ( uns8_stack, CTYPE_MAP_SIZE );

    min::ref<instruction> ci =
        min::push ( instructions );

    (&ci)->line_number = 0;
    (&ci)->operation = 0;
    (&ci)->atom_table_ID = 0;
    (&ci)->require_dispatcher_ID = 0;
    (&ci)->erroneous_atom_type = 0;
    (&ci)->output_type = 0;
    (&ci)->goto_table_ID = 0;
    (&ci)->call_table_ID = 0;
    (&ci)->accept = false;

    substate = is_others ? ADD_REPEAT
                         : ADD_CHARACTERS;
}

// Pop an instruction group from the instructions stack
// and return the instruction_ID of its first instruc-
// tion, or return 0 if there is no instruction group.
//
static uns32 pop_instruction_group ( uns32 line_number )
{
    uns32 instruction_ID = 0;
    while ( true )
    {
	min::ref<instruction> ci =
	    current_instruction();

	uns32 translate_to_length = 0;
	if ( (&ci)->operation & LEX::TRANSLATE_TO_FLAG )
	    translate_to_length =
	        LEX::translate_to_length
		    ( (&ci)->operation );

        min::ptr<uns32> translation_vector =
	    LEX::NULL_TV();
	uns32 length = uns32_stack->length;
	MIN_REQUIRE ( translate_to_length <= length );
	if ( translate_to_length > 0 )
	    translation_vector =
	        & uns32_stack
		    [length - translate_to_length];

	if ( (&ci)->operation != 0 || (&ci)->accept )
	    instruction_ID = LEX::create_instruction
		( (&ci)->line_number,
		  (&ci)->operation,
		  translation_vector,
		  (&ci)->atom_table_ID,
		  (&ci)->require_dispatcher_ID,
		  instruction_ID,
		  (&ci)->erroneous_atom_type,
		  (&ci)->output_type,
		  (&ci)->goto_table_ID,
		  (&ci)->call_table_ID );

	if ( translate_to_length > 0 )
	    min::pop ( uns32_stack,
	               translate_to_length );

	min::pop ( instructions );

	if ( instructions->length == 0
	     ||
	     ! ( (&current_instruction())->operation
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
static void pop_dispatcher
	( bool discard_dispatcher = false )
{
    min::ref<dispatcher> d = current_dispatcher();

    uns32 instruction_ID =
        pop_instruction_group ( (&d)->line_number );

    min::ptr<const uns8> ctype_map =
        & uns8_stack[(&d)->ctype_map_offset];

    uns32 ctype_map_size = CTYPE_MAP_SIZE;
    while (    ctype_map_size > 0
            && ctype_map[ctype_map_size-1] == 0 )
        -- ctype_map_size;

    if ( discard_dispatcher )
    {
        MIN_REQUIRE ( (&d)->max_type_code == 0 );
        MIN_REQUIRE ( instruction_ID == 0 );
	MIN_REQUIRE ( ctype_map_size == 0 );
	MIN_REQUIRE ( (&d)->others_dispatcher_ID == 0 );
	MIN_REQUIRE
	    ( (&d)->others_instruction_ID == 0 );
	MIN_REQUIRE
	    ( (&d)->others_repeat_count == 0 );

	min::push(uns32_stack) = 0;
	min::push(uns32_stack) = instruction_ID;
	min::push(uns32_stack) = (&d)->repeat_count;

	min::pop ( dispatchers );
	min::pop ( uns8_stack, CTYPE_MAP_SIZE );

	substate = DISPATCHERS;
    	return;
    }

    uns32 dispatcher_ID =
        LEX::create_dispatcher
	    ( (&d)->line_number,
	      (&d)->max_type_code,
	      ctype_map_size,
	      ctype_map );

    if ( (&d)->others_dispatcher_ID != 0 )
        ATTACH ( dispatcher_ID, 0,
	         (&d)->others_dispatcher_ID );
    if ( (&d)->others_instruction_ID != 0 )
        ATTACH ( dispatcher_ID, 0,
	         (&d)->others_instruction_ID );
    if ( (&d)->others_repeat_count != 0 )
	SET_REPEAT_COUNT
	    ( dispatcher_ID, 0,
	      (&d)->others_repeat_count );

    for ( uns32 tcode = (&d)->max_type_code;
          0 < tcode; -- tcode )
    {
        uns32 sub_repeat_count =
	    min::pop ( uns32_stack );
	uns32 sub_instruction_ID =
	    min::pop ( uns32_stack );
	uns32 sub_dispatcher_ID =
	    min::pop ( uns32_stack );

	// In the special case of an atom pattern
	// sub_dispatcher_ID == 0 is possible.
	//
	if ( sub_dispatcher_ID != 0 )
	    ATTACH ( dispatcher_ID, tcode,
		     sub_dispatcher_ID );

	if ( sub_instruction_ID != 0 )
	    ATTACH ( dispatcher_ID, tcode,
		     sub_instruction_ID );

	if ( sub_repeat_count != 0 )
	    SET_REPEAT_COUNT ( dispatcher_ID, tcode,
		               sub_repeat_count );
    }

    if ( (&d)->is_others_dispatcher )
    {
	min::ref<dispatcher> parent =
	    parent_dispatcher();
	(&parent)->others_dispatcher_ID =
	    dispatcher_ID;
	(&parent)->others_instruction_ID =
	    instruction_ID;
	(&parent)->others_repeat_count =
	    (&d)->repeat_count;
    }
    else
    {
	min::push(uns32_stack) = dispatcher_ID;
	min::push(uns32_stack) = instruction_ID;
	min::push(uns32_stack) = (&d)->repeat_count;
    }

    min::pop ( dispatchers );
    min::pop ( uns8_stack, CTYPE_MAP_SIZE );

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

    MIN_REQUIRE ( dispatchers->length == 0 );
    MIN_REQUIRE ( instructions->length == 0 );
    push_dispatcher();
    substate = DISPATCHERS;
}

void LEXNDL::end_table ( void )
{
    FUNCTION ( "end_table" );
    ASSERT ( state == INSIDE_TABLE,
             "end_table() misplaced" );
    ASSERT ( dispatchers->length == 1,
             "end_table() misplaced" );

    pop_dispatcher();

    uns32 repeat_count = min::pop ( uns32_stack );
    uns32 instruction_ID = min::pop ( uns32_stack );
    uns32 dispatcher_ID  = min::pop ( uns32_stack );

    MIN_REQUIRE ( dispatchers->length == 0 );
    MIN_REQUIRE ( instructions->length == 0 );
    MIN_REQUIRE ( uns32_stack->length == 0 );
    MIN_REQUIRE ( uns8_stack->length == 0 );

    ATTACH ( table_name, dispatcher_ID );
    if ( instruction_ID != 0 )
	ATTACH ( table_name, instruction_ID );
    MIN_REQUIRE ( repeat_count == 0 );

    state = INSIDE_PROGRAM;
}

static void internal_add_sub_chars
	( const char * ASCII_chars, bool sub = false )
{
    if ( ASCII_chars == NULL ) return;

    min::ref<dispatcher> d = parent_dispatcher();
    int c;  // use int instead of char to prevent
            // 0 <= c or c < 128 warning message.
    uns32 t = (&d)->max_type_code;
    min::ptr<uns8> ctype_map =
        & uns8_stack[(&d)->ctype_map_offset];
    while ( ( c = * ASCII_chars ++ ) != 0 )
    {
	ASSERT ( 0 <= c && c < 128,
	         "non-ASCII character in"
		 " ASCII_chars" );

	if ( sub )
	{
	    if ( ctype_map[c] == 0 ) continue;
	    if ( ctype_map[c] != t ) continue;
	    ctype_map[c] = 0;
	}
	else
	{
	    if ( ctype_map[c] == t ) continue;
	    if ( ctype_map[c] != 0 ) continue;
	    ctype_map[c] = t;
	}

    }
}

static uns32 * atom_pattern_name_p;
void LEXNDL::begin_atom_pattern
	( uns32 & atom_pattern_name,
	  const char * ASCII_chars )
{
    FUNCTION ( "begin_atom_pattern" );
    ASSERT ( state == INSIDE_PROGRAM,
             "begin_atom_pattern() misplaced" );
    state = INSIDE_ATOM_PATTERN;

    push_dispatcher();
    push_dispatcher();
    internal_add_sub_chars ( ASCII_chars );

    atom_pattern_name_p = & atom_pattern_name;
}

void LEXNDL::NEXT ( const char * ASCII_chars )
{
    FUNCTION ( "NEXT" );
    ASSERT ( state == INSIDE_ATOM_PATTERN,
             "NEXT() misplaced" );

    push_dispatcher();
    internal_add_sub_chars ( ASCII_chars );
}

void LEXNDL::end_atom_pattern ( void )
{
    FUNCTION ( "end_atom_pattern" );
    ASSERT ( state == INSIDE_ATOM_PATTERN,
             "end_atom_pattern() misplaced" );

    pop_dispatcher ( true );

    while ( dispatchers->length > 0)
	pop_dispatcher();

    * atom_pattern_name_p = uns32_stack[0];

    min::pop ( uns32_stack, 3 );

    MIN_REQUIRE ( dispatchers->length == 0 );
    MIN_REQUIRE ( instructions->length == 0 );
    MIN_REQUIRE ( uns32_stack->length == 0 );
    MIN_REQUIRE ( uns8_stack->length == 0 );

    state = INSIDE_PROGRAM;
}

void LEXNDL::add_chars ( const char * ASCII_chars )
{
    FUNCTION ( "add_chars" );
    ASSERT ( state == INSIDE_TABLE
             ||
	     state == INSIDE_ATOM_PATTERN,
             "add_chars() misplaced" );
    ASSERT ( substate == ADD_CHARACTERS,
             "add_chars() misplaced" );

    internal_add_sub_chars ( ASCII_chars );
}

void LEXNDL::sub_chars ( const char * ASCII_chars )
{
    FUNCTION ( "sub_chars" );
    ASSERT ( state == INSIDE_TABLE
             ||
	     state == INSIDE_ATOM_PATTERN,
             "sub_chars() misplaced" );
    ASSERT ( substate == ADD_CHARACTERS,
             "sub_chars() misplaced" );

    internal_add_sub_chars ( ASCII_chars, true );
}

void LEXNDL::add_char ( uns32 c )
{
    FUNCTION ( "add_char" );
    ASSERT ( state == INSIDE_TABLE
             ||
	     state == INSIDE_ATOM_PATTERN,
             "add_char() misplaced" );
    ASSERT ( substate == ADD_CHARACTERS,
             "add_char() misplaced" );
    ASSERT ( c < 256,
             "add_char() c is not LATIN1 or ASCII" );

    min::ref<dispatcher> d = parent_dispatcher();
    uns32 t = (&d)->max_type_code;
    min::ptr<uns8> ctype_map =
        & uns8_stack[(&d)->ctype_map_offset];
    if ( ctype_map[c] == t ) return;
    if ( ctype_map[c] != 0 ) return;
    ctype_map[c] = t;
}

void LEXNDL::sub_char ( uns32 c )
{
    FUNCTION ( "sub_char" );
    ASSERT ( state == INSIDE_TABLE
             ||
	     state == INSIDE_ATOM_PATTERN,
             "sub_char() misplaced" );
    ASSERT ( substate == ADD_CHARACTERS,
             "sub_char() misplaced" );
    ASSERT ( c < 256,
             "sub_char() c is not LATIN1 or ASCII" );

    min::ref<dispatcher> d = parent_dispatcher();
    uns32 t = (&d)->max_type_code;
    min::ptr<uns8> ctype_map =
        & uns8_stack[(&d)->ctype_map_offset];
    if ( ctype_map[c] == 0 ) return;
    if ( ctype_map[c] != t ) return;
    ctype_map[c] = 0;
}

void LEXNDL::add_category ( const char * category )
{
    FUNCTION ( "add_category" );
    ASSERT ( state == INSIDE_TABLE
             ||
	     state == INSIDE_ATOM_PATTERN,
             "add_category() misplaced" );
    ASSERT ( substate == ADD_CHARACTERS,
             "add_category() misplaced" );

    min::ref<dispatcher> d = parent_dispatcher();
    uns32 t = (&d)->max_type_code;
    min::ptr<uns8> ctype_map =
        & uns8_stack[(&d)->ctype_map_offset];
    unsigned len = strlen ( category );
    for ( uns32 cindex = 0; cindex < CTYPE_MAP_SIZE;
                            ++ cindex )
    {
	const char * cat =
	    min::unicode::category[cindex];
        if (    cat == NULL
	     || strncmp ( category, cat, len ) != 0 )
	    continue;
	if ( ctype_map[cindex] == t ) continue;
	if ( ctype_map[cindex] != 0 ) continue;
	ctype_map[cindex] = t;
    }
}

void LEXNDL::sub_category ( const char * category )
{
    FUNCTION ( "sub_category" );
    ASSERT ( state == INSIDE_TABLE
             ||
	     state == INSIDE_ATOM_PATTERN,
             "sub_category() misplaced" );
    ASSERT ( substate == ADD_CHARACTERS,
             "sub_category() misplaced" );

    min::ref<dispatcher> d = parent_dispatcher();
    uns32 t = (&d)->max_type_code;
    min::ptr<uns8> ctype_map =
        & uns8_stack[(&d)->ctype_map_offset];
    unsigned len = strlen ( category );
    for ( uns32 cindex = 0; cindex < CTYPE_MAP_SIZE;
                            ++ cindex )
    {
	const char * cat =
	    min::unicode::category[cindex];
        if (    cat == NULL
	     || strncmp ( category, cat, len ) != 0 )
	    continue;
	if ( ctype_map[cindex] == 0 ) continue;
	if ( ctype_map[cindex] != t ) continue;
	ctype_map[cindex] = 0;
    }
}

void LEXNDL::REPEAT ( uns32 repeat_count )
{
    FUNCTION ( "REPEAT" );
    ASSERT ( state == INSIDE_TABLE
             ||
	     state == INSIDE_ATOM_PATTERN,
             "REPEAT() misplaced" );
    ASSERT ( substate == ADD_CHARACTERS
             ||
	     substate == ADD_REPEAT,
             "REPEAT() misplaced" );
    ASSERT ( repeat_count > 0,
             "REPEAT(0) with 0 repeat_count illegal" );

    min::ref<dispatcher> d = current_dispatcher();

    ASSERT ( (&d)->repeat_count == 0,
             "REPEAT() previously called for this"
	     " dispatcher" );

    (&d)->repeat_count = repeat_count;
}

void LEXNDL::begin_dispatch ( const char * ASCII_chars )
{
    FUNCTION ( "begin_dispatch" );
    ASSERT ( state == INSIDE_TABLE,
             "begin_dispatch() misplaced" );
    ASSERT ( substate <= DISPATCHERS,
             "begin_dispatch() misplaced" );

    bool is_others =
        ( ASCII_chars == LEXNDL::OTHERS );

    if ( is_others )
    {
	min::ref<dispatcher> d = current_dispatcher();
	ASSERT ( (&d)->others_dispatcher_ID == 0
	         &&
		 (&d)->others_instruction_ID == 0
	         &&
		 (&d)->others_repeat_count == 0,
		 "more than one OTHERS dispatchers"
		 " under one parent" );
    }

    push_dispatcher ( is_others );

    if ( ! is_others )
	internal_add_sub_chars ( ASCII_chars );
}

void LEXNDL::end_dispatch ( void )
{
    FUNCTION ( "end_dispatch" );
    ASSERT ( state == INSIDE_TABLE,
             "end_dispatch() misplaced" );
    ASSERT ( dispatchers->length >= 2,
             "end_dispatch() misplaced" );
    pop_dispatcher();
}

# define INSTRUCTION_LINE \
    if ( (&ci)->line_number == 0 ) \
	(&ci)->line_number = LEXNDL::line

void LEXNDL::accept ( void )
{
    FUNCTION ( "accept" );
    ASSERT ( state == INSIDE_TABLE,
             "accept() misplaced" );
    substate = ::INSTRUCTION;

    min::ref<instruction> ci = current_instruction();
    ASSERT ( ! (&ci)->accept,
             "accept() conflicts with another"
	     " accept()" );
    ASSERT ( (&ci)->operation == 0,
             "accept() conflicts with another"
	     " instruction component" );

    INSTRUCTION_LINE;
    (&ci)->accept = true;
}

void LEXNDL::keep ( uns32 n )
{
    FUNCTION ( "keep" );
    ASSERT ( state == INSIDE_TABLE,
             "accept() misplaced" );
    substate = ::INSTRUCTION;

    min::ref<instruction> ci = current_instruction();
    ASSERT ( ! (&ci)->accept,
             "keep() conflicts with accept()" );
    ASSERT ( ! ( (&ci)->operation & LEX::KEEP_FLAG ),
             "keep() conflicts with another keep()" );
    ASSERT ( n <= LEX::KEEP_LENGTH_MASK,
             "keep() length (%d) too large (> %d)",
	     n, LEX::KEEP_LENGTH_MASK );

    INSTRUCTION_LINE;
    (&ci)->operation |= LEX::KEEP ( n );
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

    min::ref<instruction> ci = current_instruction();
    ASSERT ( ! (&ci)->accept,
             "translate_to() conflicts with accept()" );
    ASSERT ( ! (   (&ci)->operation
                 & (   LEX::TRANSLATE_TO_FLAG
	             | LEX::TRANSLATE_OCT_FLAG
	             | LEX::TRANSLATE_NAME_FLAG
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

    INSTRUCTION_LINE;
    (&ci)->operation |= LEX::TRANSLATE_TO ( n );
    while ( n -- )
        min::push(uns32_stack) =
	    * translation_string ++;
}

void LEXNDL::translate_oct ( uns32 m, uns32 n )
{
    FUNCTION ( "translate_oct" );
    ASSERT ( state == INSIDE_TABLE,
             "translate_oct() misplaced" );
    substate = ::INSTRUCTION;

    min::ref<instruction> ci = current_instruction();
    ASSERT ( ! (&ci)->accept,
             "translate_oct() conflicts with"
	     " accept()" );
    ASSERT ( ! (   (&ci)->operation
                 & (   LEX::TRANSLATE_TO_FLAG
	             | LEX::TRANSLATE_OCT_FLAG
	             | LEX::TRANSLATE_NAME_FLAG
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

    INSTRUCTION_LINE;
    (&ci)->operation |= LEX::TRANSLATE_OCT ( m, n );
}

void LEXNDL::translate_hex ( uns32 m, uns32 n )
{
    FUNCTION ( "translate_hex" );
    ASSERT ( state == INSIDE_TABLE,
             "translate_hex() misplaced" );
    substate = ::INSTRUCTION;

    min::ref<instruction> ci = current_instruction();
    ASSERT ( ! (&ci)->accept,
             "translate_hex() conflicts with"
	     " accept()" );
    ASSERT ( ! (   (&ci)->operation
                 & (   LEX::TRANSLATE_TO_FLAG
	             | LEX::TRANSLATE_OCT_FLAG
	             | LEX::TRANSLATE_NAME_FLAG
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

    INSTRUCTION_LINE;
    (&ci)->operation |= LEX::TRANSLATE_HEX ( m, n );
}
void LEXNDL::translate_name ( uns32 m, uns32 n )
{
    FUNCTION ( "translate_name" );
    ASSERT ( state == INSIDE_TABLE,
             "translate_name() misplaced" );
    substate = ::INSTRUCTION;

    min::ref<instruction> ci = current_instruction();
    ASSERT ( ! (&ci)->accept,
             "translate_name() conflicts with"
	     " accept()" );
    ASSERT ( ! (   (&ci)->operation
                 & (   LEX::TRANSLATE_TO_FLAG
	             | LEX::TRANSLATE_OCT_FLAG
	             | LEX::TRANSLATE_NAME_FLAG
	             | LEX::TRANSLATE_HEX_FLAG ) ),
             "translate_name() conflicts with another"
	     " translate...()" );
    ASSERT ( m <= LEX::PREFIX_LENGTH_MASK,
             "translate_name() prefix length (%d)"
	     " too large (> %d)",
	     m, LEX::PREFIX_LENGTH_MASK );
    ASSERT ( n <= LEX::POSTFIX_LENGTH_MASK,
             "translate_name() postfix length (%d)"
	     " too large (> %d)",
	     n, LEX::POSTFIX_LENGTH_MASK );

    INSTRUCTION_LINE;
    (&ci)->operation |= LEX::TRANSLATE_NAME ( m, n );
}

void LEXNDL::match ( uns32 table_name )
{
    FUNCTION ( "MATCH" );
    ASSERT ( state == INSIDE_TABLE,
             "match() misplaced" );
    substate = ::INSTRUCTION;

    min::ref<instruction> ci = current_instruction();

    ASSERT ( ! (&ci)->accept,
             "match() conflicts with accept()" );

    ASSERT (    LEX::component_type ( table_name )
             == LEX::TABLE,
             "table_name does not reference a table" );

    INSTRUCTION_LINE;
    (&ci)->operation |= LEX::MATCH;
    (&ci)->atom_table_ID = table_name;
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

    min::ref<instruction> ci = current_instruction();
    ASSERT ( ! (&ci)->accept,
             "require() conflicts with accept()" );
    ASSERT ( ! ( (&ci)->operation & REQUIRE ),
             "multiple require()'s in instruction" );
    ASSERT (   (&ci)->operation
             & (   LEX::TRANSLATE_OCT_FLAG
	         | LEX::TRANSLATE_NAME_FLAG
	         | LEX::TRANSLATE_HEX_FLAG
	         | LEX::MATCH ),
             "require() is not after a match() or"
	     " translate_oct/hex/name()" );

    INSTRUCTION_LINE;
    (&ci)->operation |= LEX::REQUIRE;
    (&ci)->require_dispatcher_ID = atom_pattern_name;
}

void LEXNDL::erroneous_atom ( uns32 type_name )
{
    FUNCTION ( "erroneous_atom" );
    ASSERT ( state == INSIDE_TABLE,
             "erroneous_atom() misplaced" );
    substate = ::INSTRUCTION;

    min::ref<instruction> ci = current_instruction();
    ASSERT ( ! (&ci)->accept,
             "erroneous_atom() conflicts with"
	     " accept()" );
    ASSERT ( ! (   (&ci)->operation
                 & LEX::ERRONEOUS_ATOM ),
             "erroneous_atom() conflicts with another"
	     " erroneous_atom()" );

    INSTRUCTION_LINE;
    (&ci)->operation |= LEX::ERRONEOUS_ATOM;
    (&ci)->erroneous_atom_type = type_name;
}

void LEXNDL::output ( uns32 type_name )
{
    FUNCTION ( "output" );
    ASSERT ( state == INSIDE_TABLE,
             "output() misplaced" );
    ASSERT ( ::table_mode != LEX::ATOM,
             "output() in atom table" );
    substate = ::INSTRUCTION;

    min::ref<instruction> ci = current_instruction();
    ASSERT ( ! (&ci)->accept,
             "output() conflicts with accept()" );
    ASSERT ( ! ( (&ci)->operation & LEX::OUTPUT ),
             "output() conflicts with another"
	     " output()" );

    INSTRUCTION_LINE;
    (&ci)->operation |= LEX::OUTPUT;
    (&ci)->output_type = type_name;
}

void LEXNDL::go ( uns32 table_name )
{
    FUNCTION ( "go" );
    ASSERT ( state == INSIDE_TABLE,
             "go() misplaced" );
    ASSERT ( ::table_mode != LEX::ATOM,
             "goto() in atom table" );
    substate = ::INSTRUCTION;

    min::ref<instruction> ci = current_instruction();
    ASSERT ( ! (&ci)->accept,
             "go() conflicts with accept()" );
    ASSERT ( ! (   (&ci)->operation
                 & (   LEX::GOTO
	             | LEX::RETURN ) ),
             "go() conflicts with another go() or"
	     " with ret()" );

    ASSERT (    LEX::component_type ( table_name )
             == LEX::TABLE,
             "table_name does not reference an"
	     " table" );
    ASSERT (    LEX::table_mode ( table_name )
             != LEX::ATOM,
             "table_name references an atom"
	     " table" );

    INSTRUCTION_LINE;
    (&ci)->operation |= LEX::GOTO;
    (&ci)->goto_table_ID = table_name;
}

void LEXNDL::call ( uns32 table_name )
{
    FUNCTION ( "call" );
    ASSERT ( state == INSIDE_TABLE,
             "call() misplaced" );
    ASSERT ( ::table_mode != LEX::ATOM,
             "call() in atom table" );
    substate = ::INSTRUCTION;

    min::ref<instruction> ci = current_instruction();
    ASSERT ( ! (&ci)->accept,
             "call() conflicts with accept()" );
    ASSERT ( ! (   (&ci)->operation
                 & (   LEX::CALL
	             | LEX::RETURN ) ),
             "call() conflicts with another call() or"
	     " with ret()" );

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

    INSTRUCTION_LINE;
    (&ci)->operation |= LEX::CALL;
    (&ci)->call_table_ID = table_name;
}

void LEXNDL::ret ( void )
{
    FUNCTION ( "ret" );
    ASSERT ( state == INSIDE_TABLE,
             "ret() misplaced" );
    ASSERT ( ::table_mode != LEX::ATOM,
             "ret() in atom table" );
    substate = ::INSTRUCTION;

    min::ref<instruction> ci = current_instruction();
    ASSERT ( ! (&ci)->accept,
             "ret() conflicts with accept()" );
    ASSERT ( ! (   (&ci)->operation
                 & (   LEX::GOTO
	             | LEX::CALL
	             | LEX::RETURN ) ),
             "ret() conflicts with another ret() or"
	     " with call() or go()" );

    INSTRUCTION_LINE;
    (&ci)->operation |= LEX::RETURN;
}

void LEXNDL::fail ( void )
{
    FUNCTION ( "fail" );
    ASSERT ( state == INSIDE_TABLE,
             "fail() misplaced" );
    ASSERT ( ::table_mode == LEX::ATOM,
             "fail() in non-atom table" );
    substate = ::INSTRUCTION;

    min::ref<instruction> ci = current_instruction();
    ASSERT ( ! (&ci)->accept,
             "fail() conflicts with accept()" );
    ASSERT ( ! ( (&ci)->operation & LEX::FAIL ),
             "fail() conflicts with another fail()" );

    INSTRUCTION_LINE;
    (&ci)->operation |= LEX::FAIL;
}

void LEXNDL::ELSE ( void )
{
    FUNCTION ( "ELSE" );
    ASSERT ( state == INSIDE_TABLE,
             "ELSE() misplaced" );
    substate = ::INSTRUCTION;

    min::ref<instruction> ci = current_instruction();
    ASSERT (   (&ci)->operation
             & (   LEX::TRANSLATE_OCT_FLAG
	         | LEX::TRANSLATE_NAME_FLAG
	         | LEX::TRANSLATE_HEX_FLAG
	         | LEX::MATCH
		 | LEX::REQUIRE ),
             "ELSE() is not after a match(),"
	     " translate_oct/hex/name(), or"
	     " require()" );

    INSTRUCTION_LINE;
    (&ci)->operation |= LEX::ELSE;

    min::ref<instruction> i2 =
        min::push ( instructions );

    (&i2)->operation = 0;
    (&i2)->atom_table_ID = 0;
    (&i2)->require_dispatcher_ID = 0;
    (&i2)->erroneous_atom_type = 0;
    (&i2)->output_type = 0;
    (&i2)->goto_table_ID = 0;
    (&i2)->call_table_ID = 0;
    (&i2)->accept = false;
}
