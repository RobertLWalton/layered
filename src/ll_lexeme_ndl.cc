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

// Implicit end instruction.
//
static void end_instruction ( void );

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
    OUTSIDE,
    INSIDE_PROGRAM,
    INSIDE_CHARACTER_PATTERN,
    INSIDE_ATOM_TABLE };

static STATE state;
static uns32 atom_table_name;
    // As set by begin_atom_table.

// Depth of dispatcher is dispatchers.length.

inline void FUNCTION ( const char * name )
{
    ::function = name;
}



// NDL Functions
// --- ---------

void LEXNDL::begin_program ( void )
{
    FUNCTION ( "begin_program" );
    ASSERT ( state == OUTSIDE,
             "misplaced begin_program()" );
    state = INSIDE_PROGRAM;

    DATA::stack.resize ( 0 );
    DATA::dispatchers.resize ( 0 );
    DATA::instructions.resize ( 0 );

    LEX::create_program();
}

void LEXNDL::end_program ( void )
{
    FUNCTION ( "end_program" );
    ASSERT ( state == INSIDE_PROGRAM,
             "misplaced end_program()" );
    state = OUTSIDE;

    assert ( stack.length == 0 );
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

void push_dispatcher ( void )
{
    uns32 dindex = dispatchers.allocate ( 1 );
    dispatcher & d = dispatchers[dindex];
    memset ( d.ascii_map, 0, 128 );
    d.max_type_code = 0;
    d.type_map_count = 0;
    d.instruction_ID = 0;

    uns32 iindex = instructions.allocate ( 1 );
    instruction & i = instructions[iindex];
    i.operation = 0;
    i.atom_table_ID = 0;
    i.type = 0;
    i.else_dispatcher_ID = 0;
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
