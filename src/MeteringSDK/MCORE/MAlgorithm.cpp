// File MCORE/MAlgorithm.cpp

#include "MCOREExtern.h"
#include "MException.h"
#include "MAlgorithm.h"

M_START_PROPERTIES(Algorithm)
M_START_METHODS(Algorithm)
   M_CLASS_SERVICE           (Algorithm, Find,                      ST_int_S_constMVariantA_constMVariantA)
   M_CLASS_SERVICE           (Algorithm, FindReverse,               ST_int_S_constMVariantA_constMVariantA)
   M_CLASS_SERVICE_OVERLOADED(Algorithm, Sort,      Sort,        2, ST_MVariant_S_constMVariantA_bool)
   M_CLASS_SERVICE_OVERLOADED(Algorithm, Sort,      DoSort,      1, ST_MVariant_S_constMVariantA)  // SWIG_HIDE
   M_CLASS_SERVICE           (Algorithm, Replace,                   ST_MVariant_S_constMVariantA_constMVariantA_constMVariantA)
   M_CLASS_SERVICE_OVERLOADED(Algorithm, TrimLeft,  TrimLeft,    2, ST_MVariant_S_constMVariantA_constMVariantA)
   M_CLASS_SERVICE_OVERLOADED(Algorithm, TrimLeft,  DoTrimLeft,  1, ST_MVariant_S_constMVariantA)  // SWIG_HIDE
   M_CLASS_SERVICE_OVERLOADED(Algorithm, TrimRight, TrimRight,   2, ST_MVariant_S_constMVariantA_constMVariantA)
   M_CLASS_SERVICE_OVERLOADED(Algorithm, TrimRight, DoTrimRight, 1, ST_MVariant_S_constMVariantA)  // SWIG_HIDE
   M_CLASS_SERVICE_OVERLOADED(Algorithm, Trim,      Trim,        2, ST_MVariant_S_constMVariantA_constMVariantA)
   M_CLASS_SERVICE_OVERLOADED(Algorithm, Trim,      DoTrim,      1, ST_MVariant_S_constMVariantA)  // SWIG_HIDE
   M_CLASS_SERVICE_OVERLOADED(Algorithm, Split,     Split,       4, ST_MVariant_S_constMVariantA_constMVariantA_bool_bool)
   M_CLASS_SERVICE_OVERLOADED(Algorithm, Split,     DoSplit3,    3, ST_MVariant_S_constMVariantA_constMVariantA_bool) // SWIG_HIDE
   M_CLASS_SERVICE_OVERLOADED(Algorithm, Split,     DoSplit2,    2, ST_MVariant_S_constMVariantA_constMVariantA)      // SWIG_HIDE
   M_CLASS_SERVICE_OVERLOADED(Algorithm, Join,      Join,        2, ST_MVariant_S_constMVariantA_constMVariantA)
   M_CLASS_SERVICE_OVERLOADED(Algorithm, Join,      DoJoin,      1, ST_MVariant_S_constMVariantA)  // SWIG_HIDE
M_END_CLASS(Algorithm, Object)

   using namespace std;

#if !M_NO_VARIANT

int MAlgorithm::Find(const MVariant& str, const MVariant& subStr)
{
   return str.FindIndexOf(subStr, false);
}

int MAlgorithm::FindReverse(const MVariant& str, const MVariant& subStr)
{
   return str.FindIndexOf(subStr, true);
}

   template
      <class Vect, class Val>
   inline void DoStingReplace(Vect& result, const Vect& source, const Val& from, const Val& to)
   {
      M_ASSERT(result.empty());
      const typename Vect::size_type sourceSize = source.size();
      typename Vect::size_type fromSize = from.size();
      if ( fromSize == 0u ) // by convention if 'from' is an empty string, then nothing to replace in string
         result = source;
      else
      {
         for ( typename Vect::size_type curr = 0u; curr < sourceSize; )
         {
            typename Vect::size_type pos = source.find(from, curr);
            if ( pos != Vect::npos ) // pos is offset of the string to replace
            {
               typename Vect::size_type delta = pos - curr;
               result.append(source, curr, delta); // copy leading substring
               result.append(to);
               curr += delta + fromSize;
            }
            else // if no more occurrences were found, copy the rest
            {
               result.append(source, curr, sourceSize - curr);
               break;
            }
         }
      }
   }

MVariant MAlgorithm::Replace(const MVariant& source, const MVariant& from, const MVariant& to)
{
   MVariant result;
   MVariant::Type type = source.GetType();
   switch ( type )
   {
   default:
      MException::ThrowUnsupportedType(type);
      M_ENSURED_ASSERT(0);
   case MVariant::VAR_STRING:
      {
         MStdString tmp;
         DoStingReplace(tmp, source.DoInterpretAsString(), from.AsString(), to.AsString());
         result.DoAssignToEmpty(tmp);
      }
      break;
   case MVariant::VAR_BYTE_STRING:
      {
         MByteString tmp;
         DoStingReplace(tmp, source.DoInterpretAsByteString(), from.AsByteString(), to.AsByteString());
         result.DoAssignByteStringToEmpty(tmp);
      }
      break;
   case MVariant::VAR_STRING_COLLECTION:
   case MVariant::VAR_VARIANT_COLLECTION:
      {
         result = source;
         const int count = result.GetCount();
         for ( int i = 0; i < count; ++i )
         {
            MVariant& it = result.AccessItem(i);
            if ( it == from )
               it = to;
         }
      }
      break;
   }
   return result;
}

#endif // !M_NO_VARIANT

   /*
   Alphanumeric comparator algorithm is taken from here:   http://www.davekoelle.com/files/alphanum.hpp
   !!! UNFORTUNATELY CURRENTLY THIS IS NOT HANDLING UTF-8 PROPERLY.

   The Alphanum Algorithm is an improved sorting algorithm for strings
   containing numbers. Instead of sorting numbers in ASCII order like a
   standard sort, this algorithm sorts numbers in numeric order.

   The Alphanum Algorithm is discussed at http://www.DaveKoelle.com

   This implementation is Copyright (c) 2008 Dirk Jagdmann <doj@cubic.org>.
   It is a cleanroom implementation of the algorithm and not derived by
   other's works. In contrast to the versions written by Dave Koelle this
   source code is distributed with the libpng/zlib license.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you
   must not claim that you wrote the original software. If you use
   this software in a product, an acknowledgement in the product
   documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and
   must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
   */
   inline bool alphanum_isdigit(const char c)
   {
      return c >= '0' && c <= '9';
   }

   static int alphanum_impl(const char *l, const char *r)
   {
      enum mode_t { STRING, NUMBER } mode = STRING;

      while ( *l && *r )
      {
         if ( mode == STRING )
         {
            char l_char, r_char;
            while ( (l_char = *l) != 0 && (r_char = *r) != 0 )
            {
               // check if there are digit characters
               const bool l_digit = alphanum_isdigit(l_char);
               const bool r_digit = alphanum_isdigit(r_char);
               // if both characters are digits, we continue in NUMBER mode
               if ( l_digit && r_digit )
               {
                  mode = NUMBER;
                  break;
               }
               // if only the left character is a digit, we have a result
               if ( l_digit ) return -1;
               // if only the right character is a digit, we have a result
               if ( r_digit ) return +1;
               // compute the difference of both characters
               int diff = l_char - r_char;
               // if they differ we have a result
               if ( diff != 0 )
               {
                  diff = toupper(l_char) - toupper(r_char);
                  if ( diff != 0 )
                     return diff;
               }
               // otherwise process the next characters
               ++l;
               ++r;
            }
         }
         else // mode==NUMBER
         {
            // get the left number
            unsigned long l_int = 0;
            while ( *l && alphanum_isdigit(*l) )
            {
               // TODO: this can overflow
               l_int = l_int * 10 + *l - '0';
               ++l;
            }

            // get the right number
            unsigned long r_int = 0;
            while ( *r && alphanum_isdigit(*r) )
            {
               // TODO: this can overflow
               r_int = r_int * 10 + *r - '0';
               ++r;
            }

            // if the difference is not equal to zero, we have a comparison result
            const long diff = l_int - r_int;
            if ( diff != 0 )
               return diff;

            // otherwise we process the next substring in STRING mode
            mode = STRING;
         }
      }

      if ( *r ) return -1;
      if ( *l ) return +1;
      return 0;
   }

   static bool alphanumericLessThanComparator(const MStdString& left, const MStdString& right)
   {
      return alphanum_impl(left.c_str(), right.c_str()) < 0;
   }

   static bool alphanumericEqualComparator(const MStdString& left, const MStdString& right)
   {
      return alphanum_impl(left.c_str(), right.c_str()) == 0;
   }

void MAlgorithm::InplaceSort(MStdStringVector& vec, bool uniqueOnly, bool naturalSort)
{
   if ( naturalSort )
   {
      std::sort(vec.begin(), vec.end(), alphanumericLessThanComparator);
      if ( uniqueOnly )
      {
         MStdStringVector::iterator last = std::unique(vec.begin(), vec.end(), alphanumericEqualComparator);
         vec.erase(last, vec.end());
      }
   }
   else
   {
      std::sort(vec.begin(), vec.end());
      if ( uniqueOnly )
      {
         MStdStringVector::iterator last = std::unique(vec.begin(), vec.end());
         vec.erase(last, vec.end());
      }
   }
}

#if !M_NO_VARIANT

   static void DoQuickSort(MVariant& a, int low, int high)
   {
      MVariant z = a.GetItem((low + high) / 2);
      int i = low;
      int j = high;
      do
      {
         while ( a.GetItem(i) < z )
            ++i;
         while ( a.GetItem(j) > z )
            --j;
         if ( i <= j )
         {
            a.SwapItems(i, j);
            ++i;
            --j;
         }
      } while ( i <= j );

      if ( low < j )
         DoQuickSort(a, low, j);
      if ( i < high )
         DoQuickSort(a, i, high);
   }

MVariant MAlgorithm::Sort(const MVariant& coll, bool uniqueOnly)
{
   MVariant result;
   if ( coll.GetType() == MVariant::VAR_MAP )
   {
      result.SetToNull(MVariant::VAR_MAP);
      const MVariant& keys = Sort(coll.GetAllMapKeys(), true);
      const int count = keys.GetCount();
      for ( int i = 0; i < count; ++i )
      {
         const MVariant& key = keys.GetItem(i);
         result.SetItem(key, coll.GetItem(key));
      }
   }
   else
   {
      result = coll;
      if ( coll.IsIndexed() )
      {
         int count = coll.GetCount();
         if ( count > 1 )
         {
            DoQuickSort(result, 0, count - 1);
            if ( uniqueOnly )
            {
               // For efficiency, find the first duplicate without copying anything
               MVariant i = 0;
               MVariant j = 1;
               for ( ; ; )
               {
                  if ( result.GetItem(i) == result.GetItem(j) )
                     break;
                  ++i;
                  ++j;
                  if ( j == count )
                     goto DONE_UNIQUE; // every item is unique
               }

               // Here we know that i and j point to equal elements with value tmp, start to move
               while ( ++j < count )
                  if ( result.GetItem(i) != result.GetItem(j))
                     result.SetItem(++i, result.GetItem(j));
               result.SetCount(i.AsInt() + 1);

            DONE_UNIQUE:
               ;
            }
         }
      }
   }
   return result;
}

#endif // !M_NO_VARIANT

void MAlgorithm::InplaceTrimLeft(MStdString& str, MConstChars trimCharacters)
{
   MStdString::iterator it = str.begin();
   MStdString::iterator itEnd = str.end();
   if ( trimCharacters == NULL || *trimCharacters == '\0' )
   {
      for ( ; it != itEnd; ++it )
         if ( *it < '\0' || *it > ' ' ) // do not use isspace here
            break;
   }
   else
   {
      for ( ; it != itEnd; ++it )
         if ( m_strchr(trimCharacters, *it) == NULL )
            break;
   }
   str.erase(str.begin(), it);
}

void MAlgorithm::InplaceTrimRight(MStdString& str, MConstChars trimCharacters)
{
   MStdString::iterator it = str.end();
   MStdString::iterator itBegin = str.begin();
   if ( trimCharacters == NULL || *trimCharacters == '\0' )
   {
      for ( ; it != itBegin; --it )
         if ( *(it - 1) < '\0' || *(it - 1) > ' ' ) // do not use isspace here
            break;
   }
   else
   {
      for ( ; it != itBegin; --it )
         if ( m_strchr(trimCharacters, *(it - 1)) == NULL )
            break;
   }
   str.erase(it, str.end());
}

void MAlgorithm::InplaceTrim(MStdString& str, MConstChars trimCharacters)
{
   InplaceTrimRight(str, trimCharacters); // having "right" first is more efficient
   InplaceTrimLeft(str, trimCharacters);
}

#if !M_NO_VARIANT

   const unsigned s_maskTrimLeft = 1;
   const unsigned s_maskTrimRight = 2;

   static void DoInternalTrim(MVariant& result, const MVariant& str, const MVariant& trimCharacters, unsigned leftRight)
   {
      M_ASSERT(leftRight > 0 && leftRight < 4);
      if ( !str.IsIndexed() )
      {
         MException::ThrowUnsupportedType(str.GetType());
         M_ENSURED_ASSERT(0);
      }

      int count = str.GetCount();
      int i = count - 1;
      int j = 0;
      if ( trimCharacters.IsEmpty() || (trimCharacters.IsIndexed() && trimCharacters.GetCount() == 0) )
      {
         if ( (leftRight & s_maskTrimRight) != 0 )
         {
            for ( ; i >= 0; --i )
               if ( str.GetItem(i).AsDWord() > static_cast<unsigned>(' ') ) // do not use isspace here
                  break;
         }
         if ( (leftRight & s_maskTrimLeft) != 0 )
         {
            for ( ; j <= i; ++j )
               if ( str.GetItem(j).AsDWord() > static_cast<unsigned>(' ') ) // do not use isspace here
                  break;
         }
      }
      else
      {
         if ( (leftRight & s_maskTrimRight) != 0 )
         {
            for ( ; i >= 0; --i )
               if ( !trimCharacters.IsPresent(str.GetItem(i)) )
                  break;
         }
         if ( (leftRight & s_maskTrimLeft) != 0 )
         {
            for ( ; j <= i; ++j )
               if ( !trimCharacters.IsPresent(str.GetItem(j)) )
                  break;
         }
      }
      if ( j == 0 && i == count - 1 )
         result = str;
      else
         result = str.GetSlice(j, i + 1);
   }

MVariant MAlgorithm::TrimLeft(const MVariant& str, const MVariant& trimCharacters)
{
   MVariant result;
   DoInternalTrim(result, str, trimCharacters, s_maskTrimLeft);
   return result;
}

MVariant MAlgorithm::TrimRight(const MVariant& str, const MVariant& trimCharacters)
{
   MVariant result;
   DoInternalTrim(result, str, trimCharacters, s_maskTrimRight);
   return result;
}

MVariant MAlgorithm::Trim(const MVariant& str, const MVariant& trimCharacters)
{
   MVariant result;
   DoInternalTrim(result, str, trimCharacters, (s_maskTrimLeft | s_maskTrimRight));
   return result;
}

#endif // !M_NO_VARIANT

MStdString MAlgorithm::TrimString(const MStdString& str, MConstChars trimCharacters)
{
   MStdString result = str;
   InplaceTrim(result, trimCharacters);
   return result;
}

   template
      <typename Vector, typename Str>
   inline Vector DoSplitWithDelimiterHelper(const Str& str, const Str& separator, bool trimBlanks, bool allowEmpty)
   {
      Vector result;
      result.reserve(8);
      typename Str::size_type separatorPos;
      typename Str::size_type startPos = 0u;
      for ( ; ; startPos = separatorPos + separator.size())
      {
         separatorPos = str.find(separator, startPos);
         if ( separatorPos == Str::npos )
            separatorPos = str.size();
         Str element(str, startPos, separatorPos - startPos);
         if ( trimBlanks )
            MAlgorithm::InplaceTrim(element);
         if ( allowEmpty || !element.empty() )
            result.push_back(element);
         if ( separatorPos == str.size() )
            break;
      }
      return result;
   }

MStdStringVector MAlgorithm::SplitWithDelimiter(const MStdString& str, MChar separator, bool trimBlanks, bool allowEmpty)
{
   MStdString separatorString(1, separator);
   return SplitWithDelimiter(str, separatorString, trimBlanks, allowEmpty);
}

MStdStringVector MAlgorithm::SplitWithDelimiter(const MStdString& str, const MStdString& separator, bool trimBlanks, bool allowEmpty)
{
   return DoSplitWithDelimiterHelper<MStdStringVector, MStdString>(str, separator, trimBlanks, allowEmpty);
}

#if !M_NO_VARIANT

MVariant MAlgorithm::Split(const MVariant& source, const MVariant& separator, bool trimBlanks, bool allowEmpty)
{
   MVariant::Type type = source.GetType();
   if ( type != MVariant::VAR_BYTE_STRING && type != MVariant::VAR_STRING )
   {
      MException::ThrowUnsupportedType(type);
      M_ENSURED_ASSERT(0);
   }
   return SplitWithDelimiter(source.AsString(), separator.AsString(), trimBlanks, allowEmpty);
}

MVariant MAlgorithm::Join(const MVariant& source, const MVariant& delimiter)
{
   if ( !source.IsCollection() )
   {
      MException::ThrowUnsupportedType(source.GetType());
      M_ENSURED_ASSERT(0);
   }

   MVariant::Type delimiterType = delimiter.GetType();
   MVariant::Type resultType;
   if ( delimiterType == MVariant::VAR_BYTE_STRING || delimiterType == MVariant::VAR_BYTE )
      resultType = MVariant::VAR_BYTE_STRING;
   else
      resultType = MVariant::VAR_STRING;
   MVariant result(resultType);
   int num = source.GetCount();
   for ( int i = 0; i < num; ++i )
   {
      result += source.GetItem(i);
      if ( !delimiter.IsEmpty() && i != num - 1 ) // array is not big typically, no necessity to care about performance here
         result += delimiter;
   }
   return result;
}

#endif // !M_NO_VARIANT

void MAlgorithm::AddUnique(MStdStringVector& source, const MStdString& val)
{
   if ( std::find(source.begin(), source.end(), val) == source.end() )
      source.push_back(val);
}

#if !M_NO_VARIANT

MVariant MAlgorithm::DoSort(const MVariant& coll)
{
   return Sort(coll);
}

MVariant MAlgorithm::DoSplit2(const MVariant& source, const MVariant& separator)
{
   return Split(source, separator);
}

MVariant MAlgorithm::DoSplit3(const MVariant& source, const MVariant& separator, bool trimBlanks)
{
   return Split(source, separator, trimBlanks);
}

MVariant MAlgorithm::DoJoin(const MVariant& source)
{
   return Join(source, MVariant());
}

MVariant MAlgorithm::DoTrimLeft(const MVariant& str)
{
   return TrimLeft(str, MVariant());
}

MVariant MAlgorithm::DoTrimRight(const MVariant& str)
{
   return TrimRight(str, MVariant());
}

MVariant MAlgorithm::DoTrim(const MVariant& str)
{
   return Trim(str, MVariant());
}

#endif
