// File MCORE/MStr.inc
// Used internally by MCORE to implement wchar/char versions of certain calls

#ifndef MCORE_PROJECT_COMPILING
   #error "Please do not include this implementation file outside of MCORE"
#endif

#ifndef _M_UNICODE
   #error "Please define _M_UNICODE into either 0 or 1"
#endif
#if _M_UNICODE
    #define _MString          MWideString
    #define _MChar            wchar_t
    #define _MUnsignedChar    unsigned short
    #define _M_L(s)           L ## s
    #define _m_isprint        iswprint
    #define _m_strlen         wcslen
    #define _m_isalpha        iswalpha
    #define _m_isdigit        iswdigit
    #define _m_isxdigit       iswxdigit
    #define _m_strtoul        wcstoul
    #define _m_strtol         wcstol
    #define _m_strtod         wcstod
    #define _m_strncpy        wcsncpy
    #define _m_strncmp        wcsncmp
    #define _m_strchr         wcschr
    #define _m_toupper        towupper
    #define _m_tolower        towlower
    #define _m_sprintf        swprintf
    #define _m_ultoa          ultow
    #define _m_ltoa           ltow
    #define _m_memchr         wmemchr
    #define _m_strnlen        wcsnlen
    #define _m_memmove        wmemmove
    #define _m_memcpy         wmemcpy
#else
    #define _MString          MStdString
    #define _MChar            char
    #define _MUnsignedChar    unsigned char
    #define _M_L(s)           s
    #define _m_isprint        isprint
    #define _m_strlen         strlen
    #define _m_isalpha        isalpha
    #define _m_isdigit        isdigit
    #define _m_isxdigit       isxdigit
    #define _m_strtoul        strtoul
    #define _m_strtol         strtol
    #define _m_strtod         strtod
    #define _m_strncpy        strncpy
    #define _m_strncmp        strncmp
    #define _m_strchr         strchr
    #define _m_toupper        toupper
    #define _m_tolower        tolower
    #define _m_sprintf        sprintf
    #define _m_ultoa          ultoa
    #define _m_ltoa           ltoa
    #define _m_memchr         memchr
    #define _m_strnlen        strnlen
    #define _m_memmove        memmove
    #define _m_memcpy         memcpy
#endif

// Because below we use numeric_limits::min() and numeric_limits::max()
#ifdef min
   #undef min
#endif
#ifdef max
   #undef max
#endif

#if !M_NO_VERBOSE_ERROR_INFORMATION
    static M_NORETURN_FUNC void DoThrow(MErrorEnum::Type code, MConstLocalChars str, const _MChar* param)
    {
       char buff [ MException::MaximumVisibleParameterLength ];
       #if _M_UNICODE
          MException::Throw(code, str, MException::VisualizeStringParameter(buff, MToStdString(param)));
       #else
          MException::Throw(code, str, MException::VisualizeCharsParameter(buff, param));
       #endif
       M_ENSURED_ASSERT(0);
    }
#else
    static M_NORETURN_FUNC void DoThrow(MErrorEnum::Type code)
    {
       MException::Throw(code);
       M_ENSURED_ASSERT(0);
    }
#endif

    static M_NORETURN_FUNC void DoThrowCannotConvertToFloat(const _MChar* str)
    {
       DoThrow(M_CODE_STR_P1(MErrorEnum::BadConversion, M_I("Could not convert '%s' to floating point number"), str));
       M_ENSURED_ASSERT(0);
    }

_MString MStr::ToXMLString(const _MString& str)
{
   return ToString(str, (StrXML | StrNoBackslashEscape | StrInternational));
}

_MString MStr::FromXMLString(const _MString& str)
{
   return FromString(str, (StrXML | StrNoBackslashEscape | StrInternational));
}

   static void DoEscape(_MString& res, _MChar c)
   {
      _MChar str [ 2 ];
      str[0] = _M_L('\\');
      str[1] = c;
      res.append(str, 2);
   }

   static void DoAppendGenericEscape(_MString& res, _MChar c, unsigned flags)
   {
      _MChar str [ 4 ];
      bool useHex = true;
      if ( (flags & MStr::StrShortEscapes) != 0 )
      {
         switch ( c )
         {
         case _M_L('\a'):                   c = _M_L('a');  useHex = false; break;
         case _M_L('\b'):                   c = _M_L('b');  useHex = false; break;
         case _M_L('\f'):                   c = _M_L('f');  useHex = false; break;
         case _M_L('\n'):                   c = _M_L('n');  useHex = false; break;
         case _M_L('\r'):                   c = _M_L('r');  useHex = false; break;
         case _M_L('\t'):                   c = _M_L('t');  useHex = false; break;
         case _M_L('\v'):                   c = _M_L('v');  useHex = false; break;
         case _M_L('\\'): case _M_L('\''): case _M_L('\"'): useHex = false; break;
         default:
            ;
         }
      }
      str[0] = _M_L('\\');
      if ( useHex )
      {
         str[1] = _M_L('x');
         str[2] = (_MChar)MUtilities::NumberToHexChar(unsigned((unsigned char)c) >> 4u);
         str[3] = (_MChar)MUtilities::NumberToHexChar(unsigned((unsigned char)c) & 0xFu);
         res.append(str, 4);
      }
      else
      {
         str[1] = c;
         res.append(str, 2);
      }
   }

_MString MStr::ToString(const _MString& str, unsigned masks)
{
   _MString res;

   _MString::size_type size = str.size();
   if ( size > 0 || (masks & StrQuote) != 0 )
   {
      _MString::size_type reserveSize = size + (size >> 4) + 4; // make allowance for some escapes in the string
      res.reserve(reserveSize);

      if ( (masks & StrQuote) != 0 )
         res += _M_L('\"');

      _MString::const_iterator it = str.begin();
      _MString::const_iterator itEnd = str.end();

      if ( (masks & (StrKeepSideBlanks | StrNoBackslashEscape)) == 0 )
      {
         for ( ; it < itEnd && (*it == _M_L('\x20')); ++it )            // handle leading blanks
            res.append(_M_L("\\x20"), 4);
         for ( ; it < itEnd && *(itEnd - 1) == _M_L('\x20'); --itEnd )  // handle trailing blanks
            ;
      }

      for ( ; it < itEnd; ++it )
      {
         _MChar c = *it;
         switch ( c )
         {
         case _M_L('<'):
            if ( (masks & StrXML) != 0 )
            {
               if ( (masks & StrNoBackslashEscape) != 0 )
                  res.append(_M_L("&lt;"), 4);
               else
                  DoAppendGenericEscape(res, c, masks);
            }
            else
               res += c;
             break;
         case _M_L('>'):
            if ( (masks & StrXML) != 0 )
            {
               if ( (masks & StrNoBackslashEscape) != 0 )
                  res.append(_M_L("&gt;"), 4);
               else
                  DoAppendGenericEscape(res, c, masks);
            }
            else
               res += c;
            break;
         case _M_L('&'):
            if ( (masks & StrXML) != 0 )
            {
               if ( (masks & StrNoBackslashEscape) != 0 )
                  res.append(_M_L("&amp;"), 5);
               else
                  DoAppendGenericEscape(res, c, masks);
            }
            else
               res += c;
            break;
         case _M_L('"'):
            if ( (masks & StrXML) != 0 )
            {
               if ( (masks & StrNoBackslashEscape) != 0 )
                  res.append(_M_L("&quot;"), 6);
               else
                  DoAppendGenericEscape(res, c, masks);
            }
            else if ( (masks & StrNoBackslashEscape) == 0 )
               DoEscape(res, c);
            else
               res += c;
            break;
         case _M_L('\\'):
            if ( (masks & StrNoBackslashEscape) != 0 )
               res += c;
            else
               DoEscape(res, c);
            break;
         default:
            if ( ((_MUnsignedChar)c < 0x20 // We know what we do, do not use is** function here!
                 #if !_M_UNICODE
                     || ((masks & StrInternational) == 0 && (_MUnsignedChar)c >= 0x7Fu)
                 #endif
                 )
               )
            {
               #if _M_UNICODE
                  DoCheckCharRange(c);
               #endif

               if ( (masks & StrXML) != 0 )
               {
                  if ( (masks & StrNoBackslashEscape) != 0 )
                  {
                     _MChar buff [ 16 ];
                     buff[0] = _M_L('&');
                     buff[1] = _M_L('#');
                     MToChars((unsigned long)(unsigned char)c, buff + 2);
                     size_t len = _m_strlen(buff); // we know there is at least one char
                     buff[len] = ';';
                     res.append(buff, len + 1);
                  }
                  else
                     DoAppendGenericEscape(res, c, masks);
               }
               else if ( (masks & StrNoBackslashEscape) == 0 )
                  DoAppendGenericEscape(res, c, masks);
               else
                  res += c;
            }
            else
               res += c;
         }
      }

      if ( (masks & (StrKeepSideBlanks | StrNoBackslashEscape)) == 0 ) // Take care of trailing blanks
      {
         M_ASSERT(it == itEnd); // oh sure
         for ( itEnd = str.end(); it < itEnd; ++it )
            res.append(_M_L("\\x20"), 4);
      }

      if ( (masks & StrQuote) != 0 )
         res += _M_L('\"');
   }
   return res;
}


_MString MStr::FromString(const _MString& str, unsigned masks)
{
   _MString res;
   size_t size = str.size();
   if ( size > 0 )
   {
#ifdef M_USE_USTL
      const _MChar* it = const_cast<_MString&>(str).data();  // don't use iterator due to further use of EscapeToChar
#else
      const _MChar* it = str.data();  // don't use iterator due to further use of EscapeToChar
#endif
      const _MChar* itEnd = it + size;
      if ( (masks & StrQuote) != 0 && size >= 2 )
      {
         _MChar first = *it;
         _MChar last = *(itEnd - 1);
         if ( (first == _M_L('\"') && last == _M_L('\"')) || // starts and ends with quotes -- consider this is enough evidence this is what we need
              (first == _M_L('\'') && last == _M_L('\'')) )
         {
            ++it;
            --itEnd;
            size -= 2;
         }
      }

      res.reserve(size); // at least that number of symbols...
      while( it != itEnd )
      {
         _MChar c = *it;
         if ( c == _M_L('\\') && (masks & StrNoBackslashEscape) == 0 )
            res += EscapeToChar(it, &it);
         else if ( c == _M_L('&') && (masks & StrXML) != 0 )
            res += XMLEscapeToChar(it, &it);
         else
         {
            res += c;
            ++it;
         }
      }
   }
   return res;
}

_MString MStr::ToEscapedString(const _MString& str)
{
   return ToString(str, StrInternational);
}

_MString MStr::ToQuotedEscapedString(const _MString& str)
{
   return ToString(str, StrQuote | StrInternational);
}

_MString MStr::ToEscapedXmlString(const _MString& str)
{
   return ToString(str, StrQuote | StrXML);
}

_MString MStr::CharToQuotedEscapedString(_MChar c)
{
   _MString str(1, _M_L('\''));
   str += CharToEscapedString(c);
   str += _M_L('\'');
   return str;
}

_MString MStr::CharToEscapedString(_MChar c)
{
   _MChar str [ 4 ];
   if ( c == _M_L('\'') || c == _M_L('\\') )
   {
      str[0] = _M_L('\\');
      str[1] = c;
      return _MString(str, 2);
   }
   if ( c > 0x20 && c < 0x7F ) // We know what we do, do not use is** function here! Also note \x20 is nonprintable
      return _MString(1, c);

   #if _M_UNICODE
      DoCheckCharRange(c);
   #endif
   str[0] = _M_L('\\');
   str[1] = _M_L('x');
   str[2] = (_MChar)MUtilities::NumberToHexChar(unsigned((unsigned char)c) >> 4u);
   str[3] = (_MChar)MUtilities::NumberToHexChar(unsigned((unsigned char)c) & 0xFu);
   return _MString(str, 4);
}

_MChar MStr::EscapeToChar(const _MChar* str, const _MChar** strEnd)
{
   M_ASSERT(str != NULL && strEnd != NULL);
   _MChar c = *str;
   int xoChar; // accumulatof for the value of octal or hexadecimal number
   if ( c == _M_L('\\') && *(str + 1) != _M_L('\0') )
   {
      ++str;
      c = *str;
      switch ( c )
      {
      case _M_L('a'): case _M_L('A'): c = _M_L('\a'); break;
      case _M_L('b'): case _M_L('B'): c = _M_L('\b'); break;
      case _M_L('f'): case _M_L('F'): c = _M_L('\f'); break;
      case _M_L('n'): case _M_L('N'): c = _M_L('\n'); break;
      case _M_L('r'): case _M_L('R'): c = _M_L('\r'); break;
      case _M_L('t'): case _M_L('T'): c = _M_L('\t'); break;
      case _M_L('v'): case _M_L('V'): c = _M_L('\v'); break;
      case _M_L('\\')               : c = _M_L('\\'); break;
      case _M_L('\'')               : c = _M_L('\''); break;
      case _M_L('\"')               : c = _M_L('\"'); break;
      case _M_L('x'): case _M_L('X'):
         if ( _m_isxdigit(*(str + 1)) )
         {
            ++str;
            xoChar = MUtilities::HexCharToNumber(static_cast<char>(*str));
            ++str;
            c = *str;
            if ( c != _M_L('\0') && _m_isxdigit(c) )
            {
               xoChar <<= 4;
               xoChar |= MUtilities::HexCharToNumber(static_cast<char>(c));
               ++str;
            }
            if ( strEnd != NULL )
               *strEnd = str; // do not increment!
            return (_MChar)xoChar;
         }
         break;
      default:
         if ( c >= _M_L('0') && c <= _M_L('7') )
         {
            xoChar = 0;
            do
            {
               xoChar <<= 3;
               xoChar |= c - _M_L('0');
               ++str;
               c = *str;
            } while ( c >= _M_L('0') && c <= _M_L('7') );
            if ( strEnd != NULL )
               *strEnd = str; // do not increment!
            return (_MChar)xoChar;
         }
         else
         {
            MException::Throw(M_CODE_STR_P1(MErrorEnum::UnknownStringEscape,
                                            (_m_isprint(c)
                                            ? M_I("Unknown string escape sequence with character '%c'")
                                            : M_I("Unknown string escape sequence with character code 0x%X")), c));
            M_ENSURED_ASSERT(0);
         }
      }
   }
   M_ENSURED_ASSERT(strEnd != NULL);
   *strEnd = str + 1;
   return c;
}

_MChar MStr::XMLEscapeToChar(const _MChar* str, const _MChar** strEnd)
{
   struct XMLEscape
   {
      int    m_size;
      _MChar m_esc[6];
      _MChar m_repr;
   };

   static const XMLEscape s_table[] = { {3, _M_L("lt;"),   _M_L('<')},
                                        {3, _M_L("gt;"),   _M_L('>')},
                                        {4, _M_L("amp;"),  _M_L('&')},
                                        {5, _M_L("quot;"), _M_L('"')},
                                        {5, _M_L("apos;"), _M_L('\'')} };

   M_ASSERT(str != NULL && strEnd != NULL);
   const _MChar* s = str;
   if ( *s == _M_L('&') ) // We know str is always zero terminated
   {
      ++s;
      if ( *s == _M_L('#') )
      {
         ++s;
         int result;
         if ( *s == _M_L('x') || *s == _M_L('X') )
            result = (int)_m_strtoul(++s, const_cast<_MChar**>(strEnd), 16);
         else
            result = (int)_m_strtoul(s, const_cast<_MChar**>(strEnd), 10);
         if ( **strEnd == _M_L(';') )
         {
            ++*strEnd; // move past ';'
            DoCheckCharRange(result);
            return _MChar(result);
         }
      }
      else
      {
         const XMLEscape* it = s_table;
         const XMLEscape* itEnd = it + M_NUMBER_OF_ARRAY_ELEMENTS(s_table);
         for ( ; it != itEnd; ++it )
         {
            if ( _m_strncmp(s, it->m_esc, it->m_size) == 0 )
            {
               *strEnd = s + it->m_size;
               return it->m_repr;
            }
         }
      }
   }

   _MChar err [ 16 ];
   _m_strncpy(err, str, M_NUMBER_OF_ARRAY_ELEMENTS(err));
   err[M_NUMBER_OF_ARRAY_ELEMENTS(err) - 1] = _M_L('\0');
   _MChar* semicolon = _m_strchr(err, _M_L(';'));
   if ( semicolon != NULL )
      *(semicolon + 1) = _M_L('\0');
   DoThrow(M_CODE_STR_P1(M_ERR_UNKNOWN_XML_ESCAPE_SEQUENCE_S1, M_I("Unknown XML sequence '%s'"), err));
   M_ENSURED_ASSERT(0);
   return _M_L('\0');
}

_MString MStr::FromEscapedString(const _MString& str)
{
   return FromString(str, StrNone);
}

_MChar MStr::ToUpper(_MChar c)
{
   return (_MChar)_m_toupper(c);
}

_MString MStr::ToUpper(const _MString& str)
{
   _MString result;
   result.reserve(str.size());
   for ( _MString::const_iterator it = str.begin(); it != str.end(); ++it )
      result += (_MChar)_m_toupper(*it);
   return result;
}

_MChar MStr::ToLower(_MChar c)
{
   return (_MChar)_m_tolower(c);
}

_MString MStr::ToLower(const _MString& str)
{
   _MString result;
   result.reserve(str.size());
   for ( _MString::const_iterator it = str.begin(); it != str.end(); ++it )
      result += (_MChar)_m_tolower(*it);
   return result;
}

_MString MStr::WordWrap(const _MString& str, unsigned indentCount, unsigned lineWidth)
{
   _MString result;
   _MString indent;
   
   // adjust parameters
   //
   if ( lineWidth < MinimumLineWidth )
      lineWidth = MinimumLineWidth;
   if ( lineWidth < indentCount + MinimumLineWidth )
      indentCount = lineWidth - MinimumLineWidth;

   const size_t strSize = str.size();
   const size_t aproximateResultLineSize  = lineWidth - indentCount;
   M_ASSERT(aproximateResultLineSize >= MinimumLineWidth);
   if ( strSize < aproximateResultLineSize && str.find(_M_L('\n')) == _MString::npos ) // optimized case
      result = str;
   else
   {
      indent.reserve(indentCount + 1);
      indent += _M_L('\n');
      indent.append(indentCount, _M_L(' '));
      const size_t estimatedResultSize = strSize + indent.size() * ((strSize / aproximateResultLineSize) + 1);
      result.reserve(estimatedResultSize);

      _MString::const_iterator it = str.begin(); // running iterator
      _MString::const_iterator itEnd = str.end();
      _MString::const_iterator itPrevLine = it;  // iterator that points to the previous line's end
      _MString::const_iterator itPrevBlank = it; // iterator that points to the previous blank
      for ( ; ; ++it )
      {
         if ( it < itEnd )
         {
            _MChar c = *it;
            switch ( c )
            {
            case _M_L(' '): // do not use isblank here, only handle spaces
               {
                  size_t runningLineSize = it - itPrevLine;
                  int remainingSize = int(aproximateResultLineSize) - int(runningLineSize);
                  if ( remainingSize > 0 )
                  {
                     itPrevBlank = it;
                     continue;
                  }

                  // ready to think about line break
                  if ( itPrevLine != itPrevBlank ) // if there was a blank in the line then break at that blank
                     it = itPrevBlank;
                  break; // break into line wrapper code
               }
            case _M_L('\n'): // recovery for the case there is a newline in the input
               break; // break into line wrapper code
            default: // We do not handle tabs, and they shall not (normally) appear in a line
               continue; // continue scanning
            }
         }

         // Line wrapper code is here. Need to add itPrevLine .. it
         //
         if ( itPrevLine < it )
            result.append(itPrevLine, it);
         if ( it == itEnd )
            break;
         result += indent;

         ++it;
         if ( it == itEnd ) // and nothing more to add
            break;
         itPrevLine = it;
         itPrevBlank = it;
      }
   }
   return result;
}

int MStr::Compare(const _MString& s1, const _MString& s2, bool ignoreCase)
{
   if ( ignoreCase )
      return Compare(ToUpper(s1), ToUpper(s2), false);

   int result = s1.compare(s2);
   if ( result < 0 ) // it appears, there are implementations that return any negative number
      return -1;
   if ( result > 0 ) // or any positive number
      return 1;
   return 0;
}

   static void DoCodeUtf8(_MString& result, const MStdString& encoding, const MByteString& str, const _MString& badChar)
   {
      MByteString::const_iterator it = str.begin();
      MByteString::const_iterator itEnd = str.end();
      result.reserve(str.size()); // possible reallocations only if there are errors in the string
      while ( it < itEnd )
      {
         utf8::uint32_t c;
         utf8::internal::utf_error err = utf8::internal::validate_next(it, itEnd, c);
         if ( err != utf8::internal::UTF8_OK )
         {
            if ( badChar.empty() )
            {
               DoThrowBadString(encoding);
               M_ENSURED_ASSERT(0);
            }
            result += badChar;
         }
         else
            DoAppendChar32(result, c);
      }
   }

_MString MStr::Decode(const MStdString& encoding, const MByteString& str, const _MString& badChar)
{
   _MString result;
   if ( IsNameUtf8(encoding) )
      DoCodeUtf8(result, encoding, str, badChar);
#if (M_OS & M_OS_WINDOWS) != 0
   else if ( IsNameAcp(encoding) )
      DoDecodeAcp(result, encoding, str, badChar);
#endif
   else
   {
      utf8::uint32_t c;
      MByteString::const_iterator it = str.begin();
      MByteString::const_iterator itEnd = str.end();
      const MOneByteCodepage* page = DoFindCodepage(encoding);
      for ( ;it < itEnd; ++it )
      {
         c = page->DoGetChar32(*it);
         if ( c == s_impossibleChar )
         {
            if ( badChar.empty() )
            {
               DoThrowBadString(encoding);
               M_ENSURED_ASSERT(0);
            }
            result += badChar;
         }
         else
            DoAppendChar32(result, c);
      }
   }
   return result;
}

MByteString MStr::Encode(const MStdString& encoding, const _MString& str, const MByteString& badChar)
{
   MByteString result;
   utf8::uint32_t c;

   _MString::const_iterator it = str.begin();
   _MString::const_iterator itEnd = str.end();
   if ( IsNameUtf8(encoding) )
   {
      #if !_M_UNICODE
         DoCodeUtf8(result, encoding, str, badChar);
      #else
         result.reserve(str.size()); // reallocations if non-ASCII
         for ( ; it < itEnd; ++it )
         {
            c = static_cast<utf8::uint32_t>(*it);
            if ( !utf8::internal::is_code_point_valid(c)
               #if (M_OS & M_OS_WINDOWS) != 0
                 || c > 0xFFFFu  // do not support multi-wchar_t on Windows currently
               #endif
                 )
            {
               if ( badChar.empty() )
               {
                  DoThrowBadString(encoding);
                  M_ENSURED_ASSERT(0);
               }
               result += badChar;
            }
            else
               DoAppendChar32(result, c);
         }
      #endif
   }
#if (M_OS & M_OS_WINDOWS) != 0
   else if ( IsNameAcp(encoding) )
      DoEncodeAcp(result, encoding, str, badChar);
#endif
   else
   {
      const MOneByteCodepage* page = DoFindCodepage(encoding);
      while ( it < itEnd )
      {
#if !_M_UNICODE
         utf8::internal::utf_error err = utf8::internal::validate_next(it, itEnd, c);
         if ( err == utf8::internal::UTF8_OK )
         {
#else
         c = static_cast<utf8::uint32_t>(*it++);
         if ( utf8::internal::is_code_point_valid(c)
            #if (M_OS & M_OS_WINDOWS) != 0
              && c <= 0xFFFFu  // do not support multi-wchar_t on Windows currently
            #endif
              )
         {
#endif
            Muint32 cc = page->DoGetChar8(c);
            if ( cc != s_impossibleChar )
            {
               M_ASSERT(cc < 256); // sure, this is how DoGetChar8 works
               result += static_cast<char>(cc);
               continue;
            }
         }
         if ( badChar.empty() )
         {
            DoThrowBadString(encoding);
            M_ENSURED_ASSERT(0);
         }
         result += badChar;
      }
   }
   return result;
}

/// Global functions:

static M_NORETURN_FUNC void DoThrowOverflowOrUnderflow(const _MChar* str)
{
   DoThrow(M_CODE_STR_P1(MErrorEnum::OverflowOrUnderflow, M_I("Number %s is too big or too small"), str));
   M_ENSURED_ASSERT(0);
}

   // Better suited strtod will return NULL in success, or the last terminating character in case of failure
   //
   static _MChar* DoStrToD(double& value, const _MChar* str) M_NO_THROW
   {
      _MChar* last;
      value = _m_strtod(str, &last);
      if ( str != last && (*last == _M_L('\0') || ((last[0] == _M_L('U') || last[0] == _M_L('u')) && last[1] == _M_L('\0'))) )
         return NULL;
      return last;
   }

   static bool DoSimpleDoubleConversion(double& val, const _MChar* str)
   {
      _MChar* last = DoStrToD(val, str);
      if ( last != NULL )
      {
         _MChar altStr [ 128 ];
         M_ASSERT(last >= str);
         size_t diff = last - str;
         _MChar lastCh = *last;
         if ( (lastCh != _M_L('.') && lastCh != _M_L(',')) || diff >= M_NUMBER_OF_ARRAY_ELEMENTS(altStr) - 1 ) // special support for the alternative decimal separators
            return false;

         _m_strncpy(altStr, str, M_NUMBER_OF_ARRAY_ELEMENTS(altStr));
         altStr[M_NUMBER_OF_ARRAY_ELEMENTS(altStr) - 1] = '\0';
         altStr[diff] = (lastCh == _M_L(',')) ? _M_L('.') : _M_L(','); // use the decimal point in the alternative locale
         last = DoStrToD(val, altStr);
         if ( last != NULL )
            return false;
      }
      return true;
   }

   template
      <typename T>
   inline bool DoAttemptDoubleConversion(T& result, _MChar endChar, const _MChar* string)
   {
      double val;
      if ( endChar != '.' && endChar != ',' && endChar != 'E' && endChar != 'e' )
         return false;
      if ( !DoSimpleDoubleConversion(val, string) )
         return false;
      if ( val >= std::numeric_limits<T>::min() && val <= std::numeric_limits<T>::max() )
      {
         result = static_cast<unsigned>(val);
         return true;
      }
      return false;
   }

M_FUNC int MToInt(const _MChar* string)
{
   M_ASSERT(string != NULL);

   bool sign, overflow;
   const _MChar* end = string;

   int result = MStringToSigned(string, sign, overflow, &end);
   if ( !(*end == '\0' || ((*end == 'u' || *end == 'U') && *(end + 1) == '\0')) || end == string )
   {
      if ( DoAttemptDoubleConversion(result, *end, string) )
         return result;
      DoThrow(M_CODE_STR_P1(MErrorEnum::BadConversion, M_I("Could not convert '%s' to integer"), string));
      M_ENSURED_ASSERT(0);
   }
   else if ( overflow )
   {
      DoThrowOverflowOrUnderflow(string);
      M_ENSURED_ASSERT(0);
   }
   return result;
}

M_FUNC unsigned MToUnsigned(const _MChar* string)
{
   M_ASSERT(string != NULL);

   bool sign, overflow;
   const _MChar* end = string;

   unsigned result = MStringToUnsigned(string, sign, overflow, &end);
   if ( !(*end == '\0' || ((*end == 'u' || *end == 'U') && *(end + 1) == '\0')) || end == string || sign )
   {
      if ( DoAttemptDoubleConversion(result, *end, string) )
         return result;
      DoThrow(M_CODE_STR_P1(MErrorEnum::BadConversion, M_I("Could not convert '%s' to unsigned integer"), string));
      M_ENSURED_ASSERT(0);
   }
   else if ( overflow )
   {
      DoThrowOverflowOrUnderflow(string);
      M_ENSURED_ASSERT(0);
   }
   return result;
}

M_FUNC Mint64 MToInt64(const _MChar* string)
{
   M_ASSERT(string != NULL);

   bool sign, overflow;
   const _MChar* end = string;

   Mint64 result = MStringToInt64(string, sign, overflow, &end);
   if ( !(*end == '\0' || ((*end == 'u' || *end == 'U') && *(end + 1) == '\0')) || end == string )
   {
      if ( DoAttemptDoubleConversion(result, *end, string) )
         return result;
      DoThrow(M_CODE_STR_P1(MErrorEnum::BadConversion, M_I("Could not convert '%s' to 64-bit integer"), string));
      M_ENSURED_ASSERT(0);
   }
   else if ( overflow )
   {
      DoThrowOverflowOrUnderflow(string);
      M_ENSURED_ASSERT(0);
   }
   return result;
}

M_FUNC Muint64 MToUInt64(const _MChar* string)
{
   M_ASSERT(string != NULL);

   bool sign, overflow;
   const _MChar* end = string;

   Muint64 result = MStringToUInt64(string, sign, overflow, &end);
   if ( !(*end == '\0' || ((*end == 'u' || *end == 'U') && *(end + 1) == '\0')) || end == string || sign )
   {
      if ( DoAttemptDoubleConversion(result, *end, string) )
         return result;
      DoThrow(M_CODE_STR_P1(MErrorEnum::BadConversion, M_I("Could not convert '%s' to unsigned long"), string));
      M_ENSURED_ASSERT(0);
   }
   else if ( overflow )
   {
      DoThrowOverflowOrUnderflow(string);
      M_ENSURED_ASSERT(0);
   }
   return result;
}

M_FUNC double MToDouble(const _MChar* str)
{
   double val;
   const _MChar* s = str;
   if ( *s == _M_L('0') ) // try to treat 0x...
   {
      ++s;
      if ( *s == _M_L('X') || *s == _M_L('x') ) // hex number
      {
         double result = 0.0;
         for ( ++s; *s; ++s )
         {
            if ( *s == _M_L('U') || *s == _M_L('u') )
            {
               if ( s[1] != _M_L('\0') )
               {
                  DoThrowCannotConvertToFloat(str);
                  M_ENSURED_ASSERT(0);
               }
               break; // success otherwise
            }
            if ( !_m_isxdigit(*s) )
               goto TRY_SIMPLE_DOUBLE_CONVERSION; // will take care of proper exception message
            result *= 16.0;
            result += (double)MUtilities::HexCharToNumber(char(*s));
         }
         return result;
      }
   }

TRY_SIMPLE_DOUBLE_CONVERSION:
   if ( !DoSimpleDoubleConversion(val, str) )
   {
      DoThrowCannotConvertToFloat(str);
      M_ENSURED_ASSERT(0);
   }
   return val; // pacify compilers
}

M_FUNC _MChar* MSignedToString(Mint64 value, _MChar* string, size_t& length);
M_FUNC _MChar* MUnsignedToString(Muint64 value, _MChar* string, size_t& length);
M_FUNC _MChar* MSignedToString(int value, _MChar* string, size_t& length);
M_FUNC _MChar* MUnsignedToString(unsigned int value, _MChar* string, size_t& length);

M_FUNC _MChar* MToChars(Mint64 value, _MChar* buffer) M_NO_THROW
{
   size_t length;
   _MChar tmp[32];
   _MChar* ptr = MSignedToString(value, &tmp[32], length);
   _m_memcpy(buffer, ptr, length);
   buffer[length] = '\0';
   return buffer;
}

M_FUNC _MChar* MToChars(Muint64 value, _MChar* buffer) M_NO_THROW
{
   size_t length;
   _MChar tmp[32];
   _MChar* ptr = MUnsignedToString(value, &tmp[32], length);
   _m_memcpy(buffer, ptr, length);
   buffer[length] = '\0';
   return buffer;
}

M_FUNC _MChar* MToChars(int value, _MChar* buffer) M_NO_THROW
{
   size_t length;
   _MChar tmp[32];
   _MChar* ptr = MSignedToString(value, &tmp[32], length);
   _m_memcpy(buffer, ptr, length);
   buffer[length] = '\0';
   return buffer;
}

M_FUNC _MChar* MToChars(unsigned value, _MChar* buffer) M_NO_THROW
{
   size_t length;
   _MChar tmp[32];
   _MChar* ptr = MUnsignedToString(value, &tmp[32], length);
   _m_memcpy(buffer, ptr, length);
   buffer[length] = '\0';
   return buffer;
}

M_FUNC size_t Mstrnlen(const _MChar* string, size_t maxlen) M_NO_THROW
{
#if defined(__BORLANDC__) || (M_OS & M_OS_WIN32_CE) || defined(__MINGW32__) || (M_OS & M_OS_QNXNTO)
   const _MChar* end = static_cast<const _MChar*>(_m_memchr(string, '\0', maxlen));
   return end ? static_cast<size_t>(end - string) : maxlen;
#else
   return _m_strnlen(string, maxlen);
#endif
}

#undef _MString
#undef _MChar
#undef _MUnsignedChar
#undef _M_L
#undef _m_isprint
#undef _m_strlen
#undef _m_isalpha
#undef _m_isdigit
#undef _m_isxdigit
#undef _m_strtoul
#undef _m_strtol
#undef _m_strtod
#undef _m_strncpy
#undef _m_strncmp
#undef _m_strchr
#undef _m_toupper
#undef _m_tolower
#undef _m_sprintf
#undef _m_ultoa
#undef _m_ltoa
#undef _m_memchr
#undef _m_strnlen
#undef _m_memmove
#undef _m_memcpy

