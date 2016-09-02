#ifndef MCORE_MALGORITHM_H
#define MCORE_MALGORITHM_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MAlgorithm.h

#include <MCORE/MCOREDefs.h>
#include <MCORE/MObject.h>
#include <MCORE/MVariant.h>

/// Set of various, mostly string related algorithms.
///
/// This includes:
///   - Searching in strings and collections
///   - Replacing subsequences in strings and collections
///   - Sorting of collections
///   - Removing (trimming) blanks or other characters from the beginning or the end of the sequence
///   - Splitting strings into collections and joining them back using delimiters
///
class M_CLASS MAlgorithm : public MObject
{
public:

#if !M_NO_VARIANT

   /// Find first occurrence of string in another string.
   ///
   /// Can work with strings, byte strings, or collections of items.
   /// Searches the given sequence in forward direction for the first occurrence of a subsequence.
   ///
   /// \param sequence
   ///    Source string or collection, the one where to find a subSequence
   ///    Can be of types string, byte string, or collection of items, and the type
   ///    should be the same as for subSequence, or an exception is thrown.
   ///
   /// \param subSequence
   ///    String or collection to find in sequence
   ///    Can be of types string, byte string, or collection of items, and the type
   ///    should be the same as for sequence, or an exception is thrown.
   ///
   /// \return When successful, returns zero based index of first character that matches the first substring
   ///    of the given string. When there is no substring found, returns -1.
   ///
   static int Find(const MVariant& sequence, const MVariant& subSequence);

   /// Find last occurrence of string in another string.
   ///
   /// Can work with strings, byte strings, or collections of items.
   /// Searches the given sequence in backward direction for the last occurrence of a subsequence.
   /// Return the index of the first element of subSequence.
   ///
   /// \param sequence
   ///    Source string or collection, the one where to find a subSequence.
   ///    Can be of types string, byte string, or collection of items, and the type
   ///    should be the same as for subSequence, or an exception is thrown.
   ///
   /// \param subSequence
   ///    String or collection to find in sequence
   ///    Can be of types string, byte string, or collection of items, and the type
   ///    should be the same as for sequence, or an exception is thrown.
   ///
   /// \return When successful, returns zero based index of first character that matches the last substring
   ///    of the given string. When there is no substring found, return -1.
   ///
   static int FindReverse(const MVariant& sequence, const MVariant& subSequence);

   /// Replaces all occurrences of 'from' in the source sequence with 'to'.
   ///
   /// Can work with strings, byte strings, or collections of items.
   /// The sequences are not necessarily of the same length, but they should be assignable one to another.
   ///
   /// \param source
   ///    Sequence where to replace the items.
   ///    Can be of types string, byte string, or collection of items, or an exception is thrown.
   ///
   /// \param from
   ///    Sequence to replace.
   ///    Should be of the same type as source parameter, or an exception is thrown.
   ///
   /// \param to
   ///    Replacement sequence.
   ///    Should be of the same type as source parameter, or an exception is thrown.
   ///
   /// \return Result string is returned. If there nothing is replaced, the string is returned unchanged.
   ///
   static MVariant Replace(const MVariant& source, const MVariant& from, const MVariant& to);

#endif // !M_NO_VARIANT

   /// Sort the string vector given, possibly remove duplicates.
   ///
   /// Different from Sort, this method changes the given collection and does not return a new collection.
   /// This is a C++ only method, not available through Reflection interface.
   ///
   /// \param collection
   ///    Sequence of type MStdStringVector to sort.
   ///
   /// \param uniqueOnly
   ///    If true, return only unique items.
   ///    Default value is false, return all items sorted, with possible duplicates.
   ///
   /// \param naturalSort
   ///    Natural sort is case insensitive, and multidigit numbers are compared as numbers.
   ///    This way, the sorted sequence will be such as COM1 COM2 COM11,
   ///    instead of COM1 COM11 COM2 as in the case of lexicographical comparison.
   ///
   /// \see Sort(const MVariant&, bool)
   ///
   static void InplaceSort(MStdStringVector& collection, bool uniqueOnly = false, bool naturalSort = false);

#if !M_NO_VARIANT

   /// Return the given sequence sorted, possibly remove duplicates.
   ///
   /// \param sequence
   ///    What to sort. The method does not change the parameter, but rather returns it.
   ///    Shall be of type string, byte string, or collection, or an exception is thrown.
   ///
   /// \param uniqueOnly
   ///    If true, return only unique items.
   ///    Default value is false, return all items sorted, with possible duplicates.
   ///
   /// \return Sorted sequence is returned.
   ///
   static MVariant Sort(const MVariant& sequence, bool uniqueOnly = false);

#endif // !M_NO_VARIANT

   /// Trim the leading characters from the string given, modify this string.
   ///
   /// This is a non-reflective C++ version of TrimLeft.
   ///
   /// \param str
   ///    String to remove characters from, the given string will be modified.
   ///
   /// \param trimCharacters
   ///    Set of characters to remove. If NULL is given, or if the parameter is not present, control characters and  blanks are removed.
   ///    If given, this should be a zero terminated string of what to remove, such as "\n\t ".
   ///
   /// \see TrimLeft - reflection-enabled version that returns a new sequence rather than modifies the current
   /// \see InplaceTrim(MStdString&, MConstChars)
   /// \see InplaceTrimRight(MStdString&, MConstChars)
   ///
   static void InplaceTrimLeft(MStdString& str, MConstChars trimCharacters = NULL);

   /// Trim the trailing characters from the string given, modify this string.
   ///
   /// This is a non-reflective C++ version of TrimRight.
   ///
   /// \param str
   ///    String to remove characters from, the given string will be modified.
   ///
   /// \param trimCharacters
   ///    Set of characters to remove. If NULL is given, or if the parameter is not present, control characters and blanks are removed.
   ///    If given, this should be a zero terminated string of what to remove, such as "\n\t ".
   ///
   /// \see TrimRight - reflection-enabled version that returns a new sequence rather than modifies the current
   /// \see InplaceTrim(MStdString&, MConstChars)
   /// \see InplaceTrimLeft(MStdString&, MConstChars)
   ///
   static void InplaceTrimRight(MStdString& str, MConstChars trimCharacters = NULL);

   /// Trim the leading and trailing characters from the string given, modify this string.
   ///
   /// This is a non-reflective C++ version of Trim.
   ///
   /// \param str
   ///    String to remove characters from, the given string will be modified.
   ///
   /// \param trimCharacters
   ///    Set of characters to remove. If NULL is given, or if the parameter is not present, the control characters and blanks are removed.
   ///    If given, this should be a zero terminated string of what to remove, such as "\n\t ".
   ///
   /// \see Trim - reflection-enabled version that returns a new sequence rather than modifies the current
   /// \see InplaceTrimRight(MStdString&, MConstChars)
   /// \see InplaceTrimLeft(MStdString&, MConstChars)
   ///
   static void InplaceTrim(MStdString& str, MConstChars trimCharacters = NULL);

#if !M_NO_VARIANT

   /// Trim the leading characters or bytes from the string given, return the result.
   ///
   /// Either string or byte string can be given as parameter.
   ///
   /// \param str
   ///    String or byte string to remove characters from, the given string will not be modified.
   ///
   /// \param trimCharacters
   ///    Set of characters to remove. If the set is empty, for example the variant is empty, the control characters and blanks are removed.
   ///    If given, this should be a zero terminated string of what to remove, such as "\n\t ".
   ///
   /// \return The result trimmed string.
   ///
   /// \see Trim
   /// \see TrimRight
   ///
   static MVariant TrimLeft(const MVariant& str, const MVariant& trimCharacters);

   /// Trim the trailing characters or bytes from the string given, return the result.
   ///
   /// Either string or byte string can be given as parameter.
   ///
   /// \param str
   ///    String or byte string to remove characters from, the given string will not be modified.
   ///
   /// \param trimCharacters
   ///    Set of characters to remove. If the set is empty, for example the variant is empty, the control characters and blanks are removed.
   ///    If given, this should be a zero terminated string of what to remove, such as "\n\t ".
   ///
   /// \return The result trimmed string.
   ///
   /// \see Trim
   /// \see TrimLeft
   ///
   static MVariant TrimRight(const MVariant& str, const MVariant& trimCharacters);

   /// Trim the leading and trailing characters or bytes from the string given, return the result.
   ///
   /// Either string or byte string can be given as parameter.
   ///
   /// \param str
   ///    String or byte string to remove characters from, the given string will not be modified.
   ///
   /// \param trimCharacters
   ///    Set of characters to remove. If the set is empty, for example the variant is empty, the control characters and blanks are removed.
   ///    If given, this should be a zero terminated string of what to remove, such as "\n\t ".
   ///
   /// \return The result trimmed string
   ///
   /// \see Trim
   /// \see TrimLeft
   ///
   static MVariant Trim(const MVariant& str, const MVariant& trimCharacters);

#endif // !M_NO_VARIANT

   /// Specialized faster version of Trim that works on string type rather than on MVariant.
   ///
   /// \see Trim
   ///
   static MStdString TrimString(const MStdString& str, MConstChars trimCharacters = NULL);

   ///@{
   /// Split the string into a vector of strings using the given delimiter sequence.
   ///
   /// This is a fast C++ variation of split for strings, not reflected.
   /// Empty items are not put into collection, like if the list 
   /// has a separator at the end, it will not be part of the output vector.
   /// If the string has no separators, only one item is returned.
   ///
   /// \param str
   ///    String to split
   ///
   /// \param delimiter
   ///    String separator, can be one character such as ";",
   ///    or multiple characters like ", " (there is a blank at the end in this example)
   ///
   /// \param trimBlanks
   ///    Boolean value, whether to remove blanks from each result string in the collection.
   ///    If false, or if the parameter is not given, the blanks will be retained in the result string.
   ///
   /// \param allowEmpty
   ///    Boolean value, whether to retain empty strings in the result collection.
   ///    For example, if allowEmpty is false, three empty strings will result from string ",," if separator is ",".
   ///    When allowEmpty is true, the result will be an empty collection.
   ///    If not given, this parameter defaults to false, and the result collection can have empty strings.
   ///
   /// \return The vector of items is returned.
   ///
   /// \ifnot M_NO_VARIANT
   /// \see \ref Split - reflection enabled universal function.
   /// \see \ref Join - Reverse function.
   /// \endif
   ///
   static MStdStringVector SplitWithDelimiter(const MStdString& str, MChar delimiter, bool trimBlanks = false, bool allowEmpty = false);
   static MStdStringVector SplitWithDelimiter(const MStdString& str, const MStdString& delimiter, bool trimBlanks = false, bool allowEmpty = false);
   ///@}

#if !M_NO_VARIANT

   /// Split the string separated by delimiter into a collection of strings.
   ///
   /// Empty items are not put into collection, like if the list 
   /// has a separator at the end, it will not be part of the output vector.
   /// If the string has no separators, only one item will be returned.
   ///
   /// \param str
   ///    String or byte string to split
   ///
   /// \param delimiter
   ///    String or byte string separator, can be one character such as ";",
   ///    or multiple characters like ", " (there is a blank at the end).
   ///    The type of delimiter shall be the same as the type of source, or an exception is thrown.
   ///
   /// \param trimBlanks
   ///    Boolean value, whether to remove blanks from each result string in the collection.
   ///    If false, or if the parameter is not given, the blanks will be retained in the result string.
   ///
   /// \param allowEmpty
   ///    Boolean value, whether to retain empty strings in the result collection.
   ///    For example, if allowEmpty is false, three empty strings will result from string ",," if separator is ",".
   ///    When allowEmpty is true, the result will be an empty collection.
   ///    If not given, this parameter defaults to false, and the result collection can have empty strings.
   ///
   /// \return The vector of items is returned.
   ///
   /// \see \ref Join - reverse function of Split.
   ///
   static MVariant Split(const MVariant& str, const MVariant& delimiter, bool trimBlanks = false, bool allowEmpty = false);

   /// Join the given array of items into a string or byte string using the given delimiter.
   ///
   /// If delimiter is of byte, or byte string type, the result string will be a byte string.
   /// Otherwise the result will be a string.
   /// All items in source should be convertible into the result type.
   ///
   /// \param source
   ///    Sequence of items to join together.
   ///    This should be a collection of items convertible into either string or to byte string.
   ///
   /// \param delimiter
   ///    Delimiter to place in between source strings.
   ///
   /// \return String or byte string is returned, depending on the type of delimiter.
   ///
   /// \see \ref Split - reverse function of Join.
   ///
   static MVariant Join(const MVariant& source, const MVariant& delimiter);

#endif // !M_NO_VARIANT

   /// Add string value to string collection if it is not there yet.
   ///
   /// This is a C++ only method.
   ///
   /// \param source
   ///    Source string collection into which the value will be attempted to be added
   ///
   /// \param value
   ///    Value to be present in collection after the call succeeds
   ///
   static void AddUnique(MStdStringVector& source, const MStdString& value);

#if MCORE_PROJECT_COMPILING
public: // Semi-private services, reflection helpers:
/// \cond SHOW_INTERNAL

#if !M_NO_VARIANT

   /// \see Sort
   static MVariant DoSort(const MVariant& coll);

   /// \see Split
   static MVariant DoSplit2(const MVariant& source, const MVariant& delimiter);

   /// \see Split
   static MVariant DoSplit3(const MVariant& source, const MVariant& delimiter, bool trimBlanks);

   /// \see Join
   static MVariant DoJoin(const MVariant& source);

   /// \see TrimLeft
   static MVariant DoTrimLeft(const MVariant& str);

   /// \see TrimRight
   static MVariant DoTrimRight(const MVariant& str);

   /// \see Trim
   static MVariant DoTrim(const MVariant& str);

#endif // !M_NO_VARIANT

/// \endcond SHOW_INTERNAL
#endif // MCORE_PROJECT_COMPILING

private:

   M_DECLARE_CLASS(Algorithm)
};

///@}
#endif
