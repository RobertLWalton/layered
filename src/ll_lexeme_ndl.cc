// Layers Language Lexical Nested Description Language
//
// File:	ll_lexeme_ndl.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Aug  8 20:39:32 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/05/08 09:23:39 $
//   $RCSfile: ll_lexeme.cc,v $
//   $Revision: 1.38 $

// Table of Contents
//
//	Usage and Setup
//	NDL Functions

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_ndl.h>
# include <ll_lexeme_ndl_data.h>
# include <iostream>
# include <iomanip>
# include <cstdlib>
# include <cstring>
# include <cassert>
# define LEX ll::lexeme
# define LEXNDL ll::lexeme::ndl
# define LEXNDLDATA ll::lexeme::ndl::data
using std::cout;
using std::endl;
using std::setw;
using std::ios;
using std::ostream;
using namespace LEXNDLDATA;

const char * LEXNDL::file = NULL;
uns32 LEXNDL::line = 0;
static const char * function = NULL;

# define FUNCTION(name) ::function = name
# define ASSERT(test,message) \
    ( (test) ? true : ::error ( #test, message ))

static bool error
	( const char * test, const char * message )
{
    cout << "ERROR: FILE: " << LEXNDL::file
         << " LINE: " << LEXNDL::line
	 << " FUNCTION: " << ::function
	 << ":" << endl
	 << "    " << message << endl
	 << "    (" << test << ")" <<endl;
    exit ( 1 );
}

enum STATE {
    OUTSIDE_PROGRAM,
    INSIDE_PROGRAM,
    INSIDE_CHARACTER_PATTERN,
    INSIDE_ATOM_TABLE };

static STATE state;

enum SUBSTATE {
    ADD_CHARACTERS,
    INSTRUCTION,
    SUBDISPATCHERS };

static SUBSTATE substate;

static uns32 atom_table_name;
    // As set by begin_atom_table.

// Depth of dispatcher is dispatchers.length.

inline void FUNCTION ( const char * name )
{
    ::function = name;
}

inline void push_uns32 ( uns32 value )
{
    uns32 i = uns32_stack.allocate ( 1 );
    uns32_stack[i] = value;
}
inline uns32 pop_uns32 ( void )
{
    uns32 i = uns32_stack.length;
    assert ( i > 0 );
    uns32 value = uns32_stack[--i];
    uns32_stack.deallocate ( 1 );
    return value;
}


// NDL Functions
// --- ---------

void LEXNDL::begin_program ( void )
{
    FUNCTION ( "begin_program" );
    ASSERT ( state == OUTSIDE_PROGRAM,
             "misplaced begin_program()" );
    state = INSIDE_PROGRAM;

    DATA::uns32_stack.resize ( 0 );
    DATA::dispatchers.resize ( 0 );
    DATA::instructions.resize ( 0 );

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
// dispatchers and instructions stacks.  Set
// substate to ADD_CHARACTERS.
//
void push_dispatcher ( void )
{
    uns32 dindex = dispatchers.allocate ( 1 );
    dispatcher & d = dispatchers[dindex];
    memset ( d.ascii_map, 0, 128 );
    d.max_type_code = 0;
    d.type_map_count = 0;

    uns32 iindex = instructions.allocate ( 1 );
    instruction & i = instructions[iindex];
    i.operation = 0;
    i.atom_table_ID = 0;
    i.type = 0;
    i.else_dispatcher_ID = 0;
    i.accept = false;

    substate = ADD_CHARACTERS;
}

// Pop an instruction (or instructions if there are
// else_not_if instructions) from the instructions
// stack and return its instruction_ID, or return 0 if
// there is no instruction.
//
static uns32 pop_instruction ( void )
{
    // TBD
}

// Pop a dispatcher from the dispatchers stack.  This
// calls LEX::create_dispatcher which returns the new
// dispatcher_ID.  See stack description in ll_lexeme_
// ndl_data.h.
//
uns32 pop_dispatcher ( void )
{
    uns32 dindex = dispatchers.length;
    assert ( dindex > 0 );
    dispatcher & d = dispatchers[--dindex];

    uns32 cmin = 0;
    uns32 cmax = 127;
    while ( cmin <= cmax && d.ascii_map[cmin] == 0 )
        ++ cmin;
    while ( cmin <= cmax && d.ascii_map[cmax] == 0 )
        -- cmax;
    bool ascii_used = ( cmin < cmax );

    uns32 dispatcher_ID =
        LEX::create_dispatcher
	    ( 2 * ( d.type_map_count + ascii_used ),
	      d.max_type_code );

    if ( ascii_used )
        ATTACH ( dispatcher_ID,
	         LEX::create_type_map
		     ( cmin, cmax,
		       d.ascii_map + cmin ) );

    for ( uns32 tcode = d.max_type_code;
          0 < tcode; -- tcode )
    {
        uns32 sub_type_map_count = pop_uns32();
	uns32 sub_dispatcher_ID = pop_uns32();
	uns32 sub_instruction_ID = pop_uns32();
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
		        ( min_char, max_char,
			  tcode ) );
	}
    }
    uns32 instruction_ID = pop_instruction();

    push_uns32 ( dispatcher_ID );
    push_uns32 ( instruction_ID );
    push_uns32 ( d.type_map_count );
    dispatchers.deallocate ( 1 );
}

void LEXNDL::begin_atom_table ( uns32 name )
{
    FUNCTION ( "begin_atom_table" );
    ASSERT ( state == INSIDE_PROGRAM,
             "begin_atom_table() misplaced" );
    state = INSIDE_ATOM_TABLE;
    atom_table_name = name;

    assert ( dispatchers.length == 0 );
    assert ( instructions.length == 0 );
    dispatcher_push();
}

void LEXNDL::end_atom_table ( void )
{
    FUNCTION ( "end_atom_table" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "end_atom_table() misplaced" );
    ASSERT ( dispatchers.length != 1,
             "end_atom_table() misplaced" );
    dispatcher_pop();
    assert ( dispatchers.length == 0 );
    assert ( instructions.length == 0 );
    uns32 type_map_count == pop_uns32();
    uns32 instruction_ID == pop_uns32();
    uns32 dispatcher_ID == pop_uns32();
    assert ( uns32_stack.length == 0 );
    ATTACH ( atom_table_name, dispatcher_ID );
    if ( instruction_DID != 0 )
	ATTACH ( atom_table_name, instruction_ID );
    assert ( type_map_count == 0 );
    state = INSIDE_PROGRAM;
}

void LEXNDL::accept ( void )
{
    FUNCTION ( "accept" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "accept() misplaced" );
    ASSERT ( substate <= INSTRUCTION,
             "accept() misplaced" );
    substate = INSTRUCTION;
    uns32 iindex = instructions.length;
    assert ( iindex > 0 );
    instruction & i = instructions[--iindex];
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
    ASSERT ( substate <= INSTRUCTION,
             "accept() misplaced" );
    substate = INSTRUCTION;
    uns32 iindex = instructions.length;
    assert ( iindex > 0 );
    instruction & i = instructions[--iindex];
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
    for ( uns32 i = 0; i < length; ++ i );
        buffer[i] = translation_string[i];
    LEXNDL::translate ( length, buffer );
}
void LEXNDL::translate
	( uns32 n, const uns32 * translation_string )
{
    FUNCTION ( "translate" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "translate() misplaced" );
    ASSERT ( substate <= INSTRUCTION,
             "accept() misplaced" );
    substate = INSTRUCTION;
    uns32 iindex = instructions.length;
    assert ( iindex > 0 );
    instruction & i = instructions[--iindex];
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
    i.operation |= LEX::TRANSLATE ( n );
    uns32 index = uns32_stack.allocate ( n );
    memcpy ( & uns32_stack[index], translation_string,
             sizeof ( uns32 ) * n );
}

void LEXNDL::translate_oct ( uns32 m, uns32 n )
{
    FUNCTION ( "translate_oct" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "translate_oct() misplaced" );
    ASSERT ( substate <= INSTRUCTION,
             "translate_oct() misplaced" );
    substate = INSTRUCTION;
    uns32 iindex = instructions.length;
    assert ( iindex > 0 );
    instruction & i = instructions[--iindex];
    ASSERT ( ! i.accept,
             "translate_oct() conflicts with  accept()" );
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
    ASSERT ( substate <= INSTRUCTION,
             "translate_hex() misplaced" );
    substate = INSTRUCTION;
    uns32 iindex = instructions.length;
    assert ( iindex > 0 );
    instruction & i = instructions[--iindex];
    ASSERT ( ! i.accept,
             "translate_hex() conflicts with  accept()" );
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
    ASSERT ( substate <= INSTRUCTION,
             "erroneous_atom() misplaced" );
    substate = INSTRUCTION;
    uns32 iindex = instructions.length;
    assert ( iindex > 0 );
    instruction & i = instructions[--iindex];
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
    ASSERT ( substate <= INSTRUCTION,
             "output() misplaced" );
    substate = INSTRUCTION;
    uns32 iindex = instructions.length;
    assert ( iindex > 0 );
    instruction & i = instructions[--iindex];
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

void LEXNDL::jump ( uns32 atom_table_name )
{
    FUNCTION ( "jump" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "jump() misplaced" );
    ASSERT ( substate <= INSTRUCTION,
             "jump() misplaced" );
    substate = INSTRUCTION;
    uns32 iindex = instructions.length;
    assert ( iindex > 0 );
    instruction & i = instructions[--iindex];
    ASSERT ( ! i.accept,
             "jump() conflicts with accept()" );
    ASSERT ( i.operation & LEX::GOTO == 0,
             "jump() conflicts with another"
	     " jump()" );
    ASSERT ( i.operation & LEX::CALLRETURN == 0,
             "jump() conflicts with"
	     " call() or ret()" );
    ASSERT ( atom_table_name != 0,
             "jump() has zero atom_table_name" );
    i.operation |= LEX::GOTO;
    i.atom_table_ID = atom_table_name;
}

void LEXNDL::call ( uns32 atom_table_name )
{
    FUNCTION ( "call" );
    ASSERT ( state == INSIDE_ATOM_TABLE,
             "call() misplaced" );
    ASSERT ( substate <= INSTRUCTION,
             "call() misplaced" );
    substate = INSTRUCTION;
    uns32 iindex = instructions.length;
    assert ( iindex > 0 );
    instruction & i = instructions[--iindex];
    ASSERT ( ! i.accept,
             "call() conflicts with accept()" );
    ASSERT ( i.operation & LEX::GOTO == 0,
             "call() conflicts with goto()" );
    ASSERT ( i.operation & LEX::CALLRETURN == 0,
             "call() conflicts with another"
	     " or with ret()" );
    ASSERT ( atom_table_name != 0,
             "call() has zero atom_table_name" );
    i.operation |= LEX::CALLRETURN;
    i.atom_table_ID = atom_table_name;
}
