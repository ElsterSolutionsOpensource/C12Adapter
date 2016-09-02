// File MCORE/MVariantParser.cpp

#include "MCOREExtern.h"
#include "MVariant.h"
#include "MUtilities.h"
#include "MVariantParser.h"
#include "MException.h"
#include "MStr.h"

#if !M_NO_VARIANT

   inline bool IsMdlSpace(char c) // do not use locale dependent isspace(), instead use MDL set for space characters
   {
      return c == ' ' ||  c == '\t' ||  c == '\r' ||  c == '\n';
   }

void MVariantParser::Parse(MConstChars beginDoc, MConstChars endDoc, MVariant& root)
{
   M_ASSERT(m_nodes.empty()); // Parse should never be called twice

   m_begin = beginDoc;
   m_end = endDoc;
   m_current = m_begin;
   m_nodes.push(&root);
   ReadValue();
   M_ASSERT(m_nodes.size() == 1); // make sure the stack only has the top item

   // The trailing characters should only be spaces
   for ( ; m_current != m_end; ++m_current )
   {
      if ( !IsMdlSpace(*m_current) )
      {
         ThrowSyntaxError();
         M_ASSERT(0);
      }
   }
}

void MVariantParser::ReadValue()
{
   Token token;
   FetchToken(token);
   switch ( token.type )
   {
   case TokenStrCollectionBegin:
      ReadStringCollection();
      break;
   case TokenCollectionBegin:
      ReadCollection();
      break;
   case TokenString:
   case TokenStringWithEscapes:
      ReadString(token);
      break;
   case TokenNumber:
      CurrentValue().DoAssignToEmpty(MToInt(MStdString(token.start, token.end)));
      break;
   case TokenUnsignedNumber:
      CurrentValue().DoAssignToEmpty(MToUnsigned(MStdString(token.start, token.end)));
      break;
   case TokenDoubleNumber:
      CurrentValue().DoAssignToEmpty(MToDouble(MStdString(token.start, token.end)));
      break;
   case TokenFalse:
      CurrentValue().DoAssignToEmpty(false);
      break;
   case TokenTrue:
      CurrentValue().DoAssignToEmpty(true);
      break;
   case TokenEmpty:
      break; // remains empty
   default:
      ThrowSyntaxError();
   }
}

MConstChars MVariantParser::NextSignificantChars()
{
   MConstChars s = m_current;
   for ( ; s != m_end; ++s )
      if ( !IsMdlSpace(*s) )
         break;
   return s;
}

void MVariantParser::FetchToken(Token& token)
{
   m_current = NextSignificantChars();
   token.start = m_current;
   char c = GetNextChar();
   switch ( c )
   {
   case '\0':
      token.type = TokenEndOfStream;
      break;
   case '{':
      token.type = TokenCollectionBegin;
      break;
   case '}':
      token.type = TokenCollectionEnd;
      break;
   case '[':
      token.type = TokenStrCollectionBegin;
      break;
   case ']':
      token.type = TokenStrCollectionEnd;
      break;
   case '\'':
   case '"':
   case '`':
      token.type = FetchString(c);
      break;
   case ',':
      token.type = TokenCollectionSeparator;
      break;
   case ':':
      token.type = TokenMapAsociate;
      break;
   case 'X': // hex string
   case 'x':
   case 'B': // byte string
   case 'b':
   case 'D': // dec string
   case 'd':
      c = GetNextChar();
      if ( c != '\'' && c != '"' && c != '`' )
      {
         ThrowSyntaxError();
         M_ENSURED_ASSERT(0);
      }
      token.type = FetchString(c);
      break;
   case '0': case '1': case '2': case '3': case '4': case '5':
   case '6': case '7': case '8': case '9': case '-':
      token.type = TokenNumber;
      while ( m_current != m_end )
      {
         char c = *m_current;
         if ( c == '.' )
            token.type = TokenDoubleNumber;
         else if ( c == 'e' || c == 'E' )
         {
            if ( token.type != TokenUnsignedNumber )
               token.type = TokenDoubleNumber;
         }
         else if ( c == 'u' || c == 'U' || c == 'x' || c == 'X' )
            token.type = TokenUnsignedNumber;
         else if ( !isxdigit(c) && c != '-' )
            break;
         GetNextChar();
      }
      break;
   case 'F': // FALSE
      token.type = TokenFalse;
      FetchRemainingKeyword("ALSE");
      break;
   case 'T': // TRUE
      token.type = TokenTrue;
      FetchRemainingKeyword("RUE");
      break;
   case 'E': // EMPTY
      token.type = TokenEmpty;
      FetchRemainingKeyword("MPTY");
      break;
   default: // proceed reading a symbolic token, number, anything
      ThrowSyntaxError();
      break;
   }
   token.end = m_current;
}

MVariantParser::TokenType MVariantParser::FetchString(char endChar)
{
   TokenType tokenType = TokenString;
   for ( ; ; )
   {
      if ( m_current == m_end )
      {
         ThrowSyntaxError();
         M_ENSURED_ASSERT(0);
      }
      char c = GetNextChar();
      if ( c == endChar )
         break;
      else if ( c == '\\' && endChar != '`' )
      {
         GetNextChar();
         tokenType = TokenStringWithEscapes; // signal that there is a necessity to process escapes
      }
   }
   return tokenType;
}

void MVariantParser::FetchRemainingKeyword(const char* remainder)
{
   for ( ; *remainder != '\0'; ++remainder )
   {
      char c = GetNextChar();
      if ( c != *remainder ) // this can return \0, end of stream. Also bad syntax.
      {
         ThrowSyntaxError();
         M_ENSURED_ASSERT(0);
      }
   }
}

void MVariantParser::ReadStringCollection()
{
   CurrentValue().SetToNull(MVariant::VAR_STRING_COLLECTION);
   if ( *NextSignificantChars() == ']' ) // empty
   {
      Token tok;
      FetchToken(tok);
      M_ASSERT(tok.type == TokenStrCollectionEnd);
   }
   else
   {
      for ( ;; )
      {
         MVariant value;
         m_nodes.push(&value);
         ReadValue();
         m_nodes.pop();
         if ( value.GetType() != MVariant::VAR_STRING )
         {
            ThrowSyntaxError();
            M_ENSURED_ASSERT(0);
         }
         CurrentValue().AddToVariantCollection(value);

         Token token;
         FetchToken(token);
         if ( token.type == TokenStrCollectionEnd )
            break;
         if ( token.type != TokenCollectionSeparator )
         {
            ThrowSyntaxError();
            M_ENSURED_ASSERT(0);
         }
      }
   }
}

void MVariantParser::ReadCollection()
{
   CurrentValue().SetToNull(MVariant::VAR_VARIANT_COLLECTION);
   char nextChar = *NextSignificantChars();
   if ( nextChar == '}' ) // empty collection
   {
      Token tok;
      FetchToken(tok);
      M_ASSERT(tok.type == TokenCollectionEnd);
   }
   else if ( nextChar == ':' ) // '{:}' // empty dictionary
   {
      CurrentValue().SetToNull(MVariant::VAR_MAP); // reassign type
      Token tok;
      FetchToken(tok);
      M_ASSERT(tok.type == TokenMapAsociate);
      FetchToken(tok);
      if ( tok.type != TokenCollectionEnd )
      {
         ThrowSyntaxError();
         M_ENSURED_ASSERT(0);
      }
   }
   else
   {
      for ( ; ; )
      {
         MVariant value;
         m_nodes.push(&value);
         ReadValue();
         m_nodes.pop();

         Token token;
         FetchToken(token);
         if ( token.type == TokenCollectionEnd )
         {
            if ( CurrentValue().GetType() == MVariant::VAR_MAP )
            {
               ThrowSyntaxError();
               M_ENSURED_ASSERT(0);
            }
            CurrentValue().AddToVariantCollection(value);
            break;
         }

         if ( token.type == TokenMapAsociate )
         {
            if ( CurrentValue().GetType() == MVariant::VAR_VARIANT_COLLECTION )
            {
               if ( CurrentValue().GetCount() != 0 ) // cannot convert nonempty array into map
               {
                  ThrowSyntaxError();
                  M_ENSURED_ASSERT(0);
               }
               CurrentValue().SetToNull(MVariant::VAR_MAP); // reassign type
            }

            MVariant value2;
            m_nodes.push(&value2);
            ReadValue();
            m_nodes.pop();
            CurrentValue().SetItem(value, value2);

            FetchToken(token);
            if ( token.type == TokenCollectionEnd )
               break;
            else if ( token.type != TokenCollectionSeparator )
            {
               ThrowSyntaxError();
               M_ENSURED_ASSERT(0);
            }
         }
         else if ( token.type == TokenCollectionSeparator )
             CurrentValue().AddToVariantCollection(value);
         else
         {
            ThrowSyntaxError();
            M_ENSURED_ASSERT(0);
         }
      }
   }
}

void MVariantParser::AssignString(TokenType type, char lastChar, bool isBytes, const MByteString& bytes)
{
   if ( type == TokenStringWithEscapes )
   {
      const MByteString& bytesEscaped = MStr::FromEscapedString(bytes);
      AssignString(TokenString, lastChar, isBytes, bytesEscaped); // recurse once with an unescaped string
   }
   else if ( lastChar == '\'' )
   {
      if ( bytes.size() != 1 )
      {
         ThrowSyntaxError();
         M_ENSURED_ASSERT(0);
      }
      if ( isBytes )
         CurrentValue().DoAssignToEmpty((Muint8)bytes[0]); // byte
      else
         CurrentValue().DoAssignToEmpty((char)bytes[0]); // character
   }
   else if ( isBytes )
      CurrentValue().DoAssignByteStringToEmpty(bytes);
   else
      CurrentValue().DoAssignToEmpty(bytes);
}

void MVariantParser::ReadString(const Token& token)
{
   M_ASSERT(token.start < token.end); // never empty
   unsigned size = static_cast<unsigned>(token.end - token.start);
   char lastChar = *(token.end - 1);
   M_ASSERT(lastChar == '\'' || lastChar == '`' || lastChar == '"');
   char firstChar = *token.start & ~0x20; // reveal some knowledge of ASCII for performance
   switch ( firstChar )
   {
   case 'X':
      M_ASSERT(size >= 3);
      M_ASSERT(*(token.start + 1) == lastChar);
      AssignString(token.type, lastChar, true, MUtilities::HexBufferToBytes(token.start + 2, size - 3));
      break;
   case 'B':
      M_ASSERT(size >= 3);
      M_ASSERT(*(token.start + 1) == lastChar);
      AssignString(token.type, lastChar, true, MByteString(token.start + 2, size - 3));
      break;
   case 'D':
      M_ASSERT(size >= 3);
      M_ASSERT(*(token.start + 1) == lastChar);
      AssignString(token.type, lastChar, true, MUtilities::NumericBufferToBytes(token.start + 2, size - 3));
      break;
   default:
      M_ASSERT(size >= 2);
      M_ASSERT(*token.start == lastChar);
      AssignString(token.type, lastChar, false, MStdString(token.start + 1, size - 2));
   }
}

void MVariantParser::ThrowSyntaxError()
{
   static const size_t s_maximumContextLength = 50; // has to be an odd number!
   static const size_t s_halfContextLength = s_maximumContextLength / 2;

   size_t buffLen = s_maximumContextLength;
   MConstChars buff = m_current - s_halfContextLength;
   if ( buff < m_begin )
   {
      buffLen -= m_begin - buff;
      buff = m_begin;
   }
   MConstChars buffEnd = buff + buffLen;
   if ( buffEnd > m_end )
      buffLen -= buffEnd - m_end;

   MException::ThrowSyntaxError(buff, buffLen);
   M_ENSURED_ASSERT(0);
}

MVariant MVariantParser::FromMDLConstant(const MStdString& v)
{
   MVariant result;
   MVariantParser parser;
   parser.Parse(v.data(), v.data() + v.size(), result);
   return result;
}

#endif // !M_NO_VARIANT
