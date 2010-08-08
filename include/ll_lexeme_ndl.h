// Layers Language Lexeme Nested Description Language
//
// File:	ll_lexeme_ndl.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Aug  8 08:38:22 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/05/09 01:41:51 $
//   $RCSfile: ll_lexeme.h,v $
//   $Revision: 1.38 $

// Table of Contents
//
//	Usage and Setup
//	Syntax
//	Declarations

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
// The syntax of Lexical NDL is:
//
//   <ndl-program> ::=
//	begin_program();
//	<declaration>*
//	<atom-table-definition>*
//	end_program();
//
//   <declaration> := <atom-table-declaration>
//		    | <character-pattern-declaration>
//
//   <atom-table-declaration> ::=
//	uns32 <atom-table-name> =
//	    new_atom_table ( <mode> );
//
//   <atom-table-name> ::= C++ variable name
//	// Atom tables must be declared first to
//      // avoid forward references in goto and
//      // call statements.
//
//   <mode> ::= MASTER | <type-name>
//	// Mode of atom table
//
//   <type-name> ::= C++ variable name
//	// The variable value is the type returned for
//	// a lexeme by the scanner
//
//   <character-pattern-declaration> ::=
//	uns32 <character-pattern-name> =
//	    begin_character_pattern
//	        ( [<included-chars>
//		      [, <excluded-chars>] ] );
//	        <character-adder>*
//	    end_character_pattern();
//
//   <included-chars> ::=
//           C++ const char * quoted string expression
//       // List of characters that are included in the
//       // character pattern if they are not also in
//	 // <excluded-chars>
//
//   <included-chars> ::=
//           C++ const char * quoted string expression
//       // See <included-chars>.
//
//   <character-adder> :=
//	   add_characters
//	       ( [<included-chars>
//		     [, <excluded-chars>] ] );
//	 | add_characters
//	       ( <min-char>, <max-char> );
//
//   <min-char> ::= C++ uns32 UNICODE character code
//   <max-char> ::= C++ uns32 UNICODE character code
//	// Minimum and maximum characters in a range
//      // of UNICODE characters
//
//   <atom-table-definition> ::=
//       begin_atom_table ( <atom-table-name> );
//	     [<instruction>]
//	     <dispatch>*
//       end_atom_table();
//
//   <dispatch> :=
//	 begin_dispatch
//	     ( [<included-chars>
//	           [, <excluded-chars>] ] );
//	        <character-adder>*
//	        [<instruction>]
//	        <dispatch>*
//	 end_dispatch();
//
//    // A <dispatch> tells what to do when the NEXT
//    // character of an atom matches a character
//    // pattern.  There is an optional instruction to
//    // execute if the atom ends with this character
//    // and nested dispatches to be made on the
//    // following character in the atom.
//
//    <instruction> ::= <non-else-instruction>
//			[<else-instruction>]
//
//    <non-else-instruction> ::= [<keep-component>]
//			         [translate-component]
//			         [atom-error-component]
//			         [output-component]
//			         [transfer-component]
//			       | accept();
//	  // An <instruction> must have at least one
//	  // optional component or be `accept();'.
//
//    <keep-component> ::=
//	  keep(<n>);
//		// Keep only first <n> characters of
//		// the atom.
//
//    <translate-component> ::=
//	    translate ( <translation-string> );
//		// Put <translation-string> into the
//		// translation buffer instead of the
//		// atom.
//	  | translate_oct ( <m>, <n> );
//	  | translate_hex ( <m>, <n> );
//		// Put an oct or hex conversion of
//		// interior of atom into the translation
//		// buffer instead of the atom.  The
//		// first <m> and last <n> characters of
//		// the atom are ignored and the rest is
//		// the converted interior.
//
//     <m> ::= C++ uns32 integer
//     <n> ::= C++ uns32 integer
//     <translation-string> ::= C++ const char * string
//                            | C++ const uns32 * string
//
//    <atom-error-component> ::=
//	  erroneous_atom ( <type-name> );
//		// Output atom as erroneous atom of the
//		// given type.
//
//    <output-component> ::=
//	  output ( <type-name> );
//		// After processing atom, output accum-
//		// ulated lexeme as a lexeme of given
//		// type.
//
//    <transfer-component> ::=
//	  | goto ( <atom-table-name> );
//		// Go to atom table.  If switching from
//		// non-master to master table and there
//		// is no instruction `output()'
//		// component, output the accumulated
//		// lexeme with type of the table being
//		// gone from.
//	  | call ( <atom-table-name> );
//		// Like goto but allows return.
//		// Atom table gone to cannot be MASTER.
//	  | return();
//		// Goto to table at top of return stack.
//		// Return stack is cleard when a MASTER
//		// table is gone to.
//
//    <else-instruction> ::=
//	  else_if_not ( <character-pattern-name> );
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


// Declarations
// ------------

namespace ll { namespace lexeme { namespace ndl {


} }

# endif // LL_LEXEME_NDL_H
