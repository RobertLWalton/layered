// Layers Language Lexeme Nested Description Language
//
// File:	ll_lexeme_ndl.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Aug 11 16:28:41 EDT 2010
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
// proofread then the low level Program Construction
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
//   <ndl-program> ::=
//      using ll::lexeme::ndl::MASTER;
//      using ll::lexeme::ndl::OTHER;
//      using ll::lexeme::ndl::uns32;
//
//	NDL::begin_program();
//	<declaration>*
//	<atom-table-definition>*
//	NDL::end_program();
//
//   <declaration> := <atom-table-declaration>
//		    | <character-pattern-declaration>
//
//   <atom-table-declaration> ::=
//	uns32 <atom-table-name> =
//	    NDL::new_atom_table ( <mode> );
//
//   <atom-table-name> ::= C++ variable name
//	// Atom tables must be declared first to
//      // avoid forward references in go and
//      // call statements.
//
//   <mode> ::= MASTER | <type-name>
//	// Mode of atom table
//
//   <type-name> ::= C++ uns32 expression
//	// The type returned by the scanner for a
//	// lexeme.
//
//   <character-pattern-declaration> ::=
//       uns32 <character-pattern-name>;
//	 NDL::begin_character_pattern
//	     ( <character-pattern-name>,
//             [<included-chars>
//		      [, <excluded-chars>] ] );
//	     <character-adder>*
//	 NDL::end_character_pattern();
//
//   <included-chars> ::=
//           C++ const char * quoted string expression
//       // List of ASCII characters that are included
//       // in the // character pattern if they are not
//       // also in <excluded-chars>
//
//   <excluded-chars> ::=
//           C++ const char * quoted string expression
//       // See <included-chars>.
//
//   <character-adder> :=
//	   NDL::add_characters
//	       ( [<included-chars>
//		     [, <excluded-chars>] ] );
//	 | NDL::add_characters
//	       ( <min-char>, <max-char> );
//
//   <min-char> ::= C++ uns32 UNICODE character code
//   <max-char> ::= C++ uns32 UNICODE character code
//	// Minimum and maximum characters in a range
//      // of UNICODE characters to be added to a
//      // character pattern.
//
//   <atom-table-definition> ::=
//       NDL::begin_atom_table ( <atom-table-name> );
//	     <dispatch>*
//	     [<instruction>]
//       NDL::end_atom_table();
//
//   <dispatch> :=
//	 NDL::begin_dispatch
//	     ( [<included-chars>
//	           [, <excluded-chars>] ] );
//	        <character-adder>*
//	        <dispatch>*
//	        [<instruction>]
//	 NDL::end_dispatch();
//     |
//       NDL::begin_dispatch ( OTHER );
//	        <dispatch>*
//	        [<instruction>]
//	 NDL::end_dispatch();
//
//    // A <dispatch> tells what to do when the NEXT
//    // character of an atom matches a character
//    // pattern.  There is an optional instruction to
//    // execute if the atom ends with this character
//    // and nested dispatches to be made on the
//    // following character in the atom.
//
//    // In the OTHER case the pattern includes just
//    // those characters not in the patterns of other
//    // dispatchers for the same atom character (i.e.,
//    // other sybling dispatchers).
//
//    <instruction> ::= <non-else-instruction>
//			[<else-instruction>]
//
//    <non-else-instruction> ::= [<keep-component>]
//			         [translate-component]
//			         [output-component]
//			         [transfer-component]
//			       | NDL::accept();
//	  // An <instruction>s must have at least one
//	  // optional component or be `accept();'.
//	  // Optional components may be in any order.
//
//    <keep-component> ::=
//	  NDL::keep(<n>);
//		// Keep only first <n> characters of
//		// the atom.
//
//    <translate-component> ::=
//	    NDL::translate
//                  ( <ascii-translation-string> );
//		// Put <ascii-translation-string> into
//              // the translation buffer instead of
//		// the atom.
//	  | NDL::translate
//                  ( <n>, <UNICODE-translation-string> );
//		// Ditto but for <n> UNICODE character
//              // string.
//	  | NDL::translate_oct ( <m>, <n> );
//	  | NDL::translate_hex ( <m>, <n> );
//		// Put an oct or hex conversion of
//		// interior of atom into the translation
//		// buffer instead of the atom.  The
//		// first <m> and last <n> characters of
//		// the atom are ignored and the rest is
//		// the converted interior.
//
//     <m> ::= C++ uns32 integer
//     <n> ::= C++ uns32 integer
//     <ascii-translation-string> ::=
//         C++ const char * string
//     <UNICODE-translation-string> ::=
//         C++ const uns32 * string
//
//
//    <output-component> ::=
//	  NDL::output ( <type-name> );
//		// After processing atom, output accum-
//		// ulated lexeme as a lexeme of given
//		// type.
//	| NDL::erroneous_atom ( <type-name> );
//		// Output atom as erroneous atom of the
//		// given type.  The atom may be in the
//		// middle of a lexeme.
//
//    <transfer-component> ::=
//	  | NDL::go ( <atom-table-name> );
//		// Go to atom table.  If switching from
//		// non-master to master table and there
//		// is no instruction `output()'
//		// component, output the accumulated
//		// lexeme with type of the table being
//		// gone from.
//	  | NDL::call ( <atom-table-name>,
//			<atom-table-name-1>,
//			<atom-table-name-2>, ... );
//		// Like `go' but allows return to the
//		// atom table containing the instruction
//		// with the `call' component via
//		// return(0), or to one of the atom-
//		// table-n values via return(n).  The
//		// atom table called cannot have mode
//		// MASTER.  A return stack entry holds
//		// the ID of the atom table containing
//		// the instruction with the `call' com-
//		// ponent, plus the ID of that instruc-
//		// tion.
//	  | NDL::ret ( <n> );
//		// See `call' above.  The return stack
//		// is popped.  The return stack is
//		// cleared when a MASTER table is gone
//		// to by any means.
//
//    <else-instruction> ::=
//	  NDL::else_if_not ( <character-pattern-name> );
//        <instruction>
//            // Only permitted if last instruction had
//	      // a translate_oct/hex component.  The
//            // preceding <instruction> is turned into
//            // a no-operation and the following
//            // <instruction> is executed if the
//            // UNICODE character produced by the
//            // preceding <instruction>'s translate_
//            // oct/hex is NOT in the character
//            // pattern.


// NDL Functions
// --- ---------

namespace ll { namespace lexeme { namespace ndl {

    using ll::lexeme::uns32;
    using ll::lexeme::MASTER;

    extern char OTHER[];

    extern const char * file;
    extern uns32 line;

#   define NDL (ll::lexeme::ndl::file = __FILE__, \
                ll::lexeme::ndl::line = __LINE__), \
	       lexeme::ndl

    void begin_program ( void );
    void end_program ( void );

    uns32 new_atom_table ( uns32 mode );

    void begin_character_pattern
	( uns32 & character_pattern_name,
	  const char * included_chars = "",
	  const char * excluded_chars = "" );
    void end_character_pattern ( void );

    void add_characters
	( const char * included_chars = "",
	  const char * excluded_chars = "" );
    void add_characters
        ( uns32 min_char, uns32 max_char );

    void begin_atom_table ( uns32 atom_table_name );
    void end_atom_table ( void );

    void begin_dispatch
	( const char * included_chars = "",
	  const char * excluded_chars = "" );
    void end_dispatch ( void );

    void accept ( void );
    void keep ( uns32 n );

    void translate ( const char * translation_string );
    void translate ( uns32 n,
                     const uns32 * translation_string );
    void translate_oct ( uns32 m, uns32 n );
    void translate_hex ( uns32 m, uns32 n );

    void erroneous_atom ( uns32 type_name );

    void output ( uns32 type_name );

    void go ( uns32 atom_table_name );
    void call ( uns32 atom_table_name );
    void call ( uns32 atom_table_name,
                uns32 atom_table_name );
    void call ( uns32 atom_table_name,
                uns32 atom_table_name,
                uns32 atom_table_name );
    void call ( uns32 atom_table_name,
                uns32 atom_table_name,
                uns32 atom_table_name,
                uns32 atom_table_name );
    void call ( uns32 atom_table_name,
                uns32 atom_table_name,
                uns32 atom_table_name,
                uns32 atom_table_name,
                uns32 atom_table_name );
    void ret ( uns32 return_index );

    void else_if_not ( uns32 character_pattern_name );

} } }

# endif // LL_LEXEME_NDL_H
