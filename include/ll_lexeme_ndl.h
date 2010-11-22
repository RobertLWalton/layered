// Layers Language Lexeme Nested Description Language
//
// File:	ll_lexeme_ndl.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Nov 21 22:02:31 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Syntax
//	NDL Functions

// Usage and Setup
// ----- --- -----

# ifndef LL_LEXEME_NDL_H
# define LL_LEXEME_NDL_H

# include <ll_lexeme.h>

// Syntax
// ------

// The Lexical Nested Description Language (Lexical NDL)
// is a mid-level language for describing Lexical Pro-
// grams.  It is easier to write and much easier to
// proofread than the low level Program Construction
// functions of ll_lexeme.h.
//
// The syntax of the Lexical Nested Description Language
// is as follows.
//
// Note: in the following `NDL' is a macro that saves
// the current file name and line number for possible
// error messages (only a macro can do this) and ends
// with `ll::lexeme::ndl' to qualify the following name.
//
// Note: None of the NDL::... functions return a value.
//
//   <ndl-program> ::=
//      using ll::lexeme::ndl::MASTER;
//      using ll::lexeme::ndl::ATOM;
//      using ll::lexeme::ndl::OTHER;
//      using ll::lexeme::ndl::uns32;
//
//	NDL::begin_program();
//	<declaration>*
//	<table-definition>*
//	NDL::end_program();
//
//   <declaration> := <table-declaration>
//		    | <atom-pattern-declaration>
//
//   <table-declaration> ::=
//       uns32 <table-name>;
//	 NDL::new_table ( <table-name>, [<mode>] );
//
//   <table-name> ::= C++ variable name
//	// Tables must be declared first to avoid
//	// forward references in go, call, and
//      // match statements.
//
//   <mode> ::= MASTER | ATOM | <type-name>
//	// Mode of table
//
//   <type-name> ::= C++ uns32 expression
//	// The type returned by the scanner for a
//	// lexeme, or the type of a lexeme table.
//
//   <atom-pattern-declaration> ::=
//       uns32 <atom-pattern-name>;
//	 NDL::begin_atom_pattern
//	     ( <atom-pattern-name>,
//             [<included-chars>
//		      [, <excluded-chars>] ] );
//	     <character-adder>*
//	     {
//	       NDL::NEXT
//	           ( [<included-chars>
//		            [, <excluded-chars>] ] );
//	       <character-adder>*
//	     }*
//	 NDL::end_atom_pattern();
//
//   // An atom pattern consists of one or more
//   // character patterns separated by `NDL::NEXT();'.
//
//   // NEXT(incl,excl) is equivalent to
//   // `NEXT(); add_characters(incl,excl)'.
//
//   <included-chars> ::=
//          C++ const char * quoted string expression
//      // List of ASCII characters that are included
//      // in the current character pattern if they are
//	// not also in <excluded-chars>
//
//   <excluded-chars> ::=
//          C++ const char * quoted string expression
//      // See <included-chars>.
//
//   <character-adder> :=
//	  NDL::add_characters
//	      ( [<included-chars>
//		    [, <excluded-chars>] ] );
//	| NDL::add_characters
//	      ( <min-char>, <max-char> );
//
//   <min-char> ::= C++ uns32 UNICODE character code
//   <max-char> ::= C++ uns32 UNICODE character code
//	// Minimum and maximum characters in a range
//      // of UNICODE characters to be added to the
//      // current character pattern.
//
//   <table-definition> ::=
//      NDL::begin_table ( <table-name> );
//	    <dispatch>*
//	    [<instruction-group>]
//      NDL::end_table();
//
//   <dispatch> :=
//	NDL::begin_dispatch
//	     ( [<included-chars>
//	           [, <excluded-chars>] ] );
//	        <character-adder>*
//	        <dispatch>*
//	        [<instruction-group>]
//	NDL::end_dispatch();
//    |
//      NDL::begin_dispatch ( OTHER );
//	        <dispatch>*
//	        [<instruction-group>]
//	NDL::end_dispatch();
//
//   // A <dispatch> tells what to do when the NEXT
//   // character of an atom matches a character
//   // pattern.  There is an optional instruction to
//   // execute if the atom ends with this character
//   // and nested dispatches to be made on the
//   // following character in the atom.
//
//   // begin_dispatch(incl,excl) is equivalent to
//   // `begin_dispatch(); add_characters(incl,excl)'.
//
//   // Two <dispatch>es at the same nesting level
//   // cannot both match the same character.
//
//   // In the OTHER case the pattern includes just
//   // those characters not in the patterns of other
//   // dispatchers for the same atom character (i.e.,
//   // other sybling dispatchers).
//
//   <instruction-group> ::=
//	<instruction> { NDL::ELSE(); <instruction> }*
//
//   <instruction> ::= [<match-component>]
//		       [<keep-component>]
//		       [<translate-component>]
//		       [<require-component>]
//		       [<output-component>]
//		       [<transfer-component>]
//		     | NDL::accept();
//	// An <instruction>s must have at least one
//	// optional component or be `accept();'.
//	// Optional components may be in any order.
//
//   <keep-component> ::=
//	  NDL::keep(<n>);
//	     // Keep only first <n> characters of the
//	     // atom.
//
//   <match-component> ::=
//	  NDL::match ( <table-name> );
//	     // Invoke named atom table, which either
//	     // recognizes and translates one atom, or
//	     // fails.
//
//   <translate-component> ::=
//	  NDL::translate_to
//                  ( <ascii-translation-string> );
//	     // Put <ascii-translation-string> into the
//           // translation buffer instead of the atom.
//      | NDL::translate_to
//                  ( <n>,
//                    <UNICODE-translation-string> );
//	     // Ditto but for <n> UNICODE character
//           // string.
//	| NDL::translate_oct ( <m>, <n> );
//	| NDL::translate_hex ( <m>, <n> );
//	     // Put an oct or hex conversion of the in-
//	     // terior of the atom into the translation
//	     // buffer instead of the atom.  The first
//	     // <m> and last <n> characters of the atom
//	     // are ignored and the rest is the conver-
//	     // ted interior.
//
//   <m> ::= C++ uns32 integer
//   <n> ::= C++ uns32 integer
//   <ascii-translation-string> ::=
//         C++ const char * string
//   <UNICODE-translation-string> ::=
//         C++ const uns32 * string
//
//   <require-component> ::=
//	  NDL::require ( <atom-pattern-name> );
//	     // Fail if the TRANSLATED atom does not
//	     // match the given atom pattern.
//	     //
//           // require() must appear after match(),
//	     // translate_oct(), or translate_hex() in
//	     // an <instruction>.
//
//   <output-component> ::=
//	  NDL::output ( <type-name> );
//	     // After processing atom, output accum-
//	     // ulated lexeme as a lexeme of given type.
//	| NDL::erroneous_atom ( <type-name> );
//	     // Output atom as erroneous atom of the
//	     // given type.  The atom may be in the
//	     // middle of a lexeme.
//
//   <transfer-component> ::=
//	  NDL::go ( <table-name> );
//	     // Go to master or lexeme table.  If
//	     // switching from a lexeme to a master
//	     // table and there is no instruction
//	     // `output()' component and the accum-
//	     // ulated lexeme is of non-zero length,
//	     // output the accumulated lexeme with type
//	     // of the lexeme table.
//	| NDL::call ( table-name> )
//	     // Like `go' but allows return to the
//	     // table containing the call instruction
//	     // via return.  The table called must be
//	     // a lexeme table.  A return stack entry
//	     // holds the ID of the table containing
//	     // the call instruction.  Recursive
//	     // calls are not allowed.
//	| NDL::ret();
//	     // See `call' above.  The return stack is
//	     // popped.  The return stack is cleared
//	     // when a master table is gone to by any
//	     // means.  Otherwise ret() is like go().
//	| NDL::fail();
//	     // In a translation table, causes the
//	     // instruction invoking the translation
//	     // table to fail.
//
//   // The <instruction>s in an <instruction-group>
//   // are separated by `NDL::ELSE();' statements.
//   // These <instruction>s are executed first-to-last
//   // until one of them does NOT fail, and then the
//   // following instructions are ignored.  A failed
//   // <instruction> behaves as a no-operation.


// NDL Functions
// --- ---------

namespace ll { namespace lexeme { namespace ndl {

    using ll::lexeme::uns32;
    using ll::lexeme::MASTER;
    using ll::lexeme::ATOM;

    extern char OTHER[];

    extern const char * file;
    extern uns32 line;

#   define NDL (ll::lexeme::ndl::file = __FILE__, \
                ll::lexeme::ndl::line = __LINE__), \
	       ll::lexeme::ndl

    // Warning: NDL does NOT permit return values, so
    // all of the below must have void return type.

    void begin_program ( void );
    void end_program ( void );

    void new_table
        ( uns32 & table_name, uns32 mode = 0 );

    void begin_atom_pattern
	( uns32 & atom_pattern_name,
	  const char * included_chars = "",
	  const char * excluded_chars = "" );
    void NEXT
	( const char * included_chars = "",
	  const char * excluded_chars = "" );
    void end_atom_pattern ( void );

    void add_characters
	( const char * included_chars = "",
	  const char * excluded_chars = "" );
    void add_characters
        ( uns32 min_char, uns32 max_char );

    void begin_table ( uns32 table_name );
    void end_table ( void );

    void begin_dispatch
	( const char * included_chars = "",
	  const char * excluded_chars = "" );
    void end_dispatch ( void );

    void accept ( void );
    void keep ( uns32 n );

    void translate_to
        ( const char * translation_string );
    void translate_to
        ( uns32 n, const uns32 * translation_string );
    void translate_oct ( uns32 m, uns32 n );
    void translate_hex ( uns32 m, uns32 n );
    void match ( uns32 table_name );

    void require ( uns32 atom_pattern_name );

    void ELSE ( void );

    void erroneous_atom ( uns32 type_name );
    void output ( uns32 type_name );

    void go ( uns32 table_name );
    void call ( uns32 table_name );
    void ret ( void );
    void fail ( void );

} } }

# endif // LL_LEXEME_NDL_H
