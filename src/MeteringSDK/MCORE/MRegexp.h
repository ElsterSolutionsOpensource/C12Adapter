#ifndef MCORE_MREGEXP_H
#define MCORE_MREGEXP_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MRegexp.h
//
// Portions copyright(c) by Guy Gascoigne, Like Zafir and Henry Spencer.
// Used under permission according to copyright notes of the above persons.
//

#include <MCORE/MCOREDefs.h>
#include <MCORE/MException.h>

/// POSIX-like regular expression handler.
///
/// A class could be given a regular expression and from that, return specific substrings (items)
/// from its input. Regular expressions may not be the fastest way to parse input
/// (though with careful anchoring they can be made so that they fail quickly if
/// they are going to) but once you have a working library they do allow for fairly
/// rapid coding. On the whole this is good enough, worry about making it faster
/// once you have it working and actually know that your optimization effort isn't
/// going unnoticed. For example:
/// \code
///    MRegexp re("^[\t ]*(.*)[\t ]*\\((.*)\\)");
///    MStdString str("example.com!david (David)\n");
///    MStdString name, addr;
///    if ( re.Match(str) && re.GetCount() == 2 )
///    {
///       name = re[2];
///       addr = re[1];
///    }
/// \endcode
/// Will give:
/// \code
///    name == "David" and addr == "example.com!david"
/// \endcode
/// If you decompose the regular expression you get:
///
///   - "^"      Beginning of line anchor.
///   - "[\t ]*" Any amount (that is zero or more characters) of tabs or spaces.
///   - "(.*)"   Field 1: A tagged expression matching any string of characters
///              This will be the longest string that will still allow the rest
///              of the pattern to match.
///   - "[\t ]*" Any amount of tabs or spaces.
///   - "\\("    An escaped open parenthesis. The double slash is a C/C++ convention
///              since this is the escape character and we want a literal slash to
///              be passed through to the regular expression code. If the user were
///              typing this sort of thing into your regular expression they would only enter
///              one slash. We escape the parenthesis so that it doesn't get
///              interpreted as a regular expression special character.
///   - "(.*)"   Field 2: A tagged expression matching any string of characters.
///   - "\\)"    An escaped closing parenthesis.
///
/// Note: The phrase tagged regular expression refers to any part of the regular
/// expression that is, because it was surrounded by parenthesis, accessible
/// after a match has been made as a separate item.  
///
/// In English, we are looking for two fields. The first will be all characters
/// from the start of the line through to the second field (without any
/// surrounding white space), and the second will be all characters within
/// parenthesis following the first field.
///
/// \par Regular Expression Syntax
///
/// A regular expression is zero or more branches, separated by '|'. It matches anything that matches one of
/// the branches.
///
/// A branch is zero or more pieces, concatenated. It matches a match for the first, followed by a match for
/// the second, etc.
///
/// A piece is an atom possibly followed by '*', '+', or '?'. An atom followed by '*' matches a sequence of 0
/// or more matches of the atom. An atom followed by '+' matches a sequence of 1 or more matches of the atom.
/// An atom followed by '?' matches a match of the atom, or the empty string.
/// An atom is a regular expression in parentheses (matching a match for the regular expression), a range (see
/// below), '.' (matching any single character), '^' (matching the empty string at the beginning of the input
/// string), '$' (matching the empty string at the end of the input string), a '\' followed by a single
/// character (matching that character), or a single character with no other significance (matching that
/// character).
///
/// A range is a sequence of characters enclosed in '[]'. It normally matches any single character from the
/// sequence. If the sequence begins with '^', it matches any single character not from the rest of the
/// sequence. If two characters in the sequence are separated by '-', this is shorthand for the full list of
/// ASCII characters between them (e.g. '[0-9]' matches any decimal digit). To include a literal ']' in the
/// sequence, make it the first character (following a possible '^'). To include a literal '-', make it the
/// first or last character.
///
/// \par Ambiguity
///
/// If a regular expression could match two different parts of the input string, it will match the one which
/// begins earliest. If both begin in the same place but match different lengths, or match the same length in
/// different ways, life gets messier, as follows.
/// In general, the possibilities in a list of branches are considered in left-to-right order, the
/// possibilities for '*', '+', and '?' are considered longest-first, nested constructs are considered from the
/// outermost in, and concatenated constructs are considered leftmost-first. The match that will be chosen is
/// the one that uses the earliest possibility in the first choice that has to be made. If there is more than
/// one choice, the next will be made in the same manner (earliest possibility) subject to the decision on the
/// first choice. And so forth.
///
/// For example, '(ab|a)b*c' could match 'abc' in one of two ways. The first choice is between 'ab' and 'a';
/// since 'ab' is earlier, and does lead to a successful overall match, it is chosen. Since the 'b' is already
/// spoken for, the 'b*' must match its last possibility--the empty string--since it must respect the earlier
/// choice.
///
/// In the particular case where the regular expression does not use `|' and does not apply `*', `+', or `?' to
/// parenthesized subexpressions, the net effect is that the longest possible match will be chosen. So `ab*',
/// presented with `xabbbby', will match `abbbb'. Note that if `ab*' is tried against `xabyabbbz', it will
/// match `ab' just after `x', due to the begins-earliest rule. (In effect, the decision on where to start the
/// match is the first choice to be made, hence subsequent choices must respect it even if this leads them to
/// less-preferred alternatives.)
///
class M_CLASS MRegexp : public MObject
{
   friend class MRegExecutor; // internal to library

public: // Constants:

   enum 
   {
      /// How many subexpressions that the library will support,
      /// attempting to use a regular expression with more than this number will generate an error.
      ///
      NUMBER_OF_SUBEXPRESSIONS = 10 
   };

public: // Constructors, destructor:

   /// Default constructor
   ///
   MRegexp();

   /// Constructor of the regular expression that takes an expression as standard string.
   ///
   /// \param exp
   ///     Regular expression
   ///
   /// \param caseInsensitive
   ///     When true, the match shall be case insensitive, false by default.
   ///
   /// \pre The expression has to correspond to the valid syntax definition
   /// as presented in the header of the file. Otherwise MERegexp is thrown
   /// with the type and string that corresponds to the error.
   ///
   MRegexp(const MStdString& exp, bool caseInsensitive = false);

   /// Constructor of the regular expression that takes an expression as a pointer to
   /// a zero terminated string. 
   ///
   /// \param exp
   ///     Regular expression
   ///
   /// \param caseInsensitive
   ///     When true, the match shall be case insensitive, false by default.
   ///
   /// \pre The expression should not be NULL, otherwise the behavior is
   /// undefined (the debug version has an assertion operator).
   /// The expression has to correspond to the valid syntax definition
   /// as presented in the header of the file. Otherwise MERegexp is thrown
   /// with the type and string that corresponds to the error.
   ///
   MRegexp(MConstChars exp, bool caseInsensitive = false); // SWIG_HIDE due to the above prototype

   /// Copy constructor
   ///
   /// \pre If the object given had a compilation error, the new object
   /// has it too.
   ///
   MRegexp(const MRegexp& r);

   /// Object destructor
   ///
   virtual ~MRegexp();

public: // Services:

   /// Check whether a valid regular expression was supplied.
   ///
   bool IsCompiled() const
   {
      return m_program != NULL;
   }

   /// Return the number of items found after a successful Match.
   ///
   int GetCount() const;

   /// Get the pattern, as it was set at compile method.
   ///
   const MStdString& GetPattern() const
   {
      return m_pattern;
   }

   /// Assignment operator.
   ///
   /// \pre If the object given had a compilation error, the new object
   /// has it too.
   ///
   MRegexp& operator=(const MRegexp& r)
   {
      if ( this != &r )
      {
         if ( r.IsCompiled() )
            Compile(r.m_pattern);
         else
            Clear();
      }
      return *this;
   }

   /// Compile the regular expression given as standard string.
   /// The format of the regular expression is defined in the class header.
   ///
   /// \param exp
   ///     Regular expression
   ///
   /// \param caseInsensitive
   ///     When true, the match shall be case insensitive, false by default.
   ///
   /// \pre The expression has to correspond to the valid syntax definition
   /// as presented in the header of the file. Otherwise the exception MERegexp is thrown.
   ///
   void Compile(const MStdString& exp, bool caseInsensitive = false);

   /// Clear the regular expression, possibly reclaim memory.
   ///
   void Clear();

   /// Examine the character string with this regular expression, returning true
   /// if there is a match. This match updates the state of this MRegexp object
   /// so that the items of the match can be obtained. The 0th item
   /// is the item of string that matched the whole regular expression.
   /// The others are those items that matched parenthesized expressions
   /// within the regular expression, with parenthesized expressions numbered
   /// in left-to-right order of their opening parentheses. If a parenthesized
   /// expression does not participate in the match at all, its length is 0.
   ///
   /// \pre MRegexp has been successfully initialized. Otherwise the
   /// match will return false in any case.
   ///
   bool Match(const MStdString&);
   
   /// Do a match using the given regular expression and string without creating MRegexp object
   ///
   /// \param regexp
   ///    Regular expression to match
   ///
   /// \param str
   ///    String in which the regular expression shall be matched.
   ///
   /// \param caseInsensitive
   ///     When true, the match shall be case insensitive, false by default.
   ///
   /// \see Match - non-static version of this call
   ///
   static bool StaticMatch(MConstChars regexp, const MStdString& str, bool caseInsensitive = false);

   /// Return the I-th matched item after a successful Match.
   /// As in the classic regexp, the zeroth element is the whole string,
   /// and the last allowed index is equal to GetCount.
   /// Look at operator[] for convenience.
   ///
   /// \pre The index has to be within zero and GetCount.
   /// There is a check.
   ///
   MStdString Item(int i) const;

   /// Return the I-th matched item after a successful Match.
   /// As in the classic regexp, the zeroth element is the whole string,
   /// and the last allowed index is equal to GetCount.
   /// This is a more convenient C++ way of accessing an item.
   ///
   /// \pre The index has to be within zero and GetCount.
   /// There is a check.
   ///
   MStdString operator[](int i) const
   {
      return Item(i);
   }

   /// Return the starting offset of the I-th matched item
   /// from the beginning of the character array used in Match.
   ///
   /// \pre The index has to be within zero and GetCount
   /// minus one. Otherwise the "" string is returned, and the object is put into
   /// an erroneous state, the error string is available.
   ///
   int GetItemStart(int i) const;

   /// Return the length of the I-th matched item as used in Match.
   ///
   /// Along with the GetItemStart, this service can be used as follows:
   ///
   /// \code
   ///   MRegexp re("^[\t ]*(.*)[\t ]*\\((.*)\\)");
   ///   MStdString str( "example.com!david (David)\n" );
   ///   assert(re.Match(str));
   ///   assert(re.GetCount() == 2);
   ///   assert(re.GetItemStart(0) == 0);
   ///   assert(re.GetItemLength(0) == 26);
   ///   assert(re.GetItemStart(1) == 0);
   ///   assert(re.GetItemLength(1) == 19);
   ///   assert(re.GetItemStart(2) == 20);
   ///   assert(re.GetItemLength(2) == 5);
   /// \endcode
   ///
   /// \pre The index has to be within zero and GetCount
   /// minus one. Otherwise the "" string is returned, and the object is put into
   /// an erroneous state, the error string is available.
   ///
   int GetItemLength(int i) const;

   /// Get the string for replacement, use source as standard string.
   ///
   /// After a successful Match one can retrieve a replacement string as an
   /// alternative to building up the various items by hand.
   ///
   /// Each character in the source string will be copied to the return value
   /// except for the following special characters:
   ///
   /// \code
   ///    &    The complete matched string (item 0).
   ///    \1   Item 1
   ///    ... and so on until...
   ///    \9   Item 9
   /// \endcode
   /// So:
   /// \code
   ///     MStdString repl = re.GetReplacementString("\2 == \1");
   /// \endcode
   /// Will give: "David == example.com!david"
   ///
   /// \pre The items given as parameters should be within range
   /// zero and GetCount minus one. Otherwise the "" string
   /// is returned, and the object is put into an erroneous state.
   ///
   MStdString GetReplaceString(const MStdString& source) const;

   /// Check if the regular expression is compiled, throw error if not.
   ///
   /// \pre The regular expression needs to be compiled.
   /// Otherwise an error is thrown.
   ///
   void CheckIsCompiled() const;

public: // Semi-private reflection method:
/// \cond SHOW_INTERNAL

   /// Reflection helper method that compiles the given regular expression with the second
   /// parameter taken as default. See Compile method for details.
   ///
   /// \pre The expression has to correspond to the valid syntax definition
   /// as presented in the header of the file. Otherwise the exception MERegexp is thrown.
   ///
   void DoCompile1(const MStdString& str)
   {
      Compile(str);
   }

/// \endcond SHOW_INTERNAL
private: // Attributes:

   // Pattern, that is used for compilation. In case of insensitive search, it will be modified from one given by the user.
   //
   MStdString m_pattern;

   // A temporary which is used to return substring offsets only
   //
   MStdString m_str;

   // Char that must begin a match, irrelevant if m_regstartExists is false
   //
   char m_regstart;

   // String (pointer into m_program) that the match must include, or NULL
   //
   MChars m_regmust;

   // Program that holds the internal regexp state machine
   //
   MChars m_program;

   // Length of m_regmust string
   //
   int m_regmlen;

   // Number of matches subexpressions
   //
   int m_count;

   // Start pointer of the subexpression
   //
   MChars m_startp [ NUMBER_OF_SUBEXPRESSIONS ];

   // End pointer of the subexpression
   //
   MChars m_endp [ NUMBER_OF_SUBEXPRESSIONS ];

   // Whether the m_regstart exists
   //
   bool m_regstartExists;

   // Is the match anchored (at beginning-of-line only)?
   //
   bool m_reganch;

   M_DECLARE_CLASS(Regexp)
};

///@}
#endif
