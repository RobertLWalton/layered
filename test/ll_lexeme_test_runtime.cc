// Layers Language Lexical Analyzer Test Runtime
//
// File:	ll_lexeme_test_runtime.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sun Apr 11 11:02:04 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/11 15:52:08 $
//   $RCSfile: ll_lexeme_test_runtime.cc,v $
//   $Revision: 1.1 $

// Table of Contents
//
//	Usage and Setup
//	Test Runtime

// Usage and Setup
// ----- --- -----

# include <ll_lexeme.h>
# include <iostream>
# include <iomanip>
# include <cstdlib>
# include <cstring>
# include <cassert>
# define LLLEX ll::lexeme
using std::cout;
using std::endl;
using std::setw;
using std::ios;
using std::ostream;
using LLLEX::uns32;

// Test Runtime
// ---- -------

struct ext_buffer : public LLLEX buffer
{
    ext_buffer ( void )
    {
	base = NULL;
	header_size = 0;
	length = 0;
	max_length = 0;
	length_increment = 1000;
    }
    ~ ext_buffer ( void )
    {
        delete [] base;
    }
    void resize ( uns32 new_max_length );
};

