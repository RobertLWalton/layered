// MIN Programming System Lexical Analyzer
//
// File:	min_lexeme.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sat Mar 27 09:15:13 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/03/27 16:20:40 $
//   $RCSfile: ll_lexeme.h,v $
//   $Revision: 1.1 $

// Table of Contents
//
//	Usage and Setup
//	MIN Interface

// Usage and Setup
// ----- --- -----

# ifndef MIN_LEXEME_H
# define MIN_LEXEME_H

// MIN Interface
// --- ---------

// If MIN_LEXEME_INTERFACE is defined, that is included.
// Otherwise various types and functions are defined
// as follows.
//
#ifdef MIN_LEXEME_INTERFACE
#   include <MIN_LEXEME_INTERFACE>
#else
    namespace min { namespace lexeme {
	typedef min::uns8 uns8;
	typedef min::uns32 uns32;
	typedef min::gen atom_table;
	    inline atom_table new_atom_table ( void )
	    {
	        return min::insertable_raw_vec_pointer
		               <uns32>::new_gen();
	    }
	typedef min::gen atom_table_name;
	    inline atom_table new_atom_table_name
		    ( const char * name )
	    {
	        return min::new_str_gen ( name );
	    }
	typedef min::insertable_raw_vec_pointer<uns32>
		    atom_table_pointer;
	    // The following functions are used without
	    // redefinition:
	    //
	    //	(constructor) atom_table_pointer
	    //			  ( atom_table )
	    //	
	    //  uns32 length_of ( atom_table_pointer & )
	    //  uns32 & operator[] ( atom_table_pointer &,
	    //			     uns32 index )
	    //  void push ( atom_table_pointer &, uns32 )
	    //
	    inline void compact
		    ( atom_table_pointer & atp )
	    {
	        min::resize
		    ( atp, min::length_of ( atp ) );
	    }
		    


    } }
#endif

# endif // MIN_LEXEME_H
