#ifndef MCORE_MVARIANTPARSER_H
#define MCORE_MVARIANTPARSER_H
// File MCORE/MVariantParser.h

#include <MCORE/MCOREDefs.h>
#include <MCORE/MVariant.h>

/// \cond SHOW_INTERNAL

#if !M_NO_VARIANT

class MVariantParser
{
public:

   static MVariant FromMDLConstant(const MStdString& v);

private:

   typedef std::stack<MVariant*> Nodes;

   enum TokenType
   {
      TokenEndOfStream = 0,
      TokenStrCollectionBegin,
      TokenStrCollectionEnd,
      TokenCollectionBegin,
      TokenCollectionEnd,
      TokenCollectionSeparator,
      TokenMapAsociate,
      TokenString,
      TokenStringWithEscapes,
      TokenNumber,
      TokenDoubleNumber,
      TokenUnsignedNumber,
      TokenFalse,
      TokenTrue,
      TokenEmpty
   };

   struct Token
   {
      TokenType type;
      MConstChars start;
      MConstChars end;
   };

   MVariantParser()
   :
      m_nodes(),
      m_begin(NULL),
      m_end(NULL),
      m_current(NULL)
   {
   }

   void Parse(MConstChars beginDoc, MConstChars endDoc, MVariant& root);

   MConstChars NextSignificantChars();
   void FetchToken(Token& token);
   TokenType FetchString(char endChar);
   void FetchRemainingKeyword(const char* remainder);

   void ReadValue();
   void ReadStringCollection();
   void ReadCollection();
   void ReadString(const Token& token);

   void AssignString(TokenType type, char lastChar, bool isBytes, const MByteString& bytes);

   MVariant& CurrentValue()
   {
      return *(m_nodes.top());
   }

   char GetNextChar()
   {
      if ( m_current == m_end )
         return 0;
      return *m_current++;
   }

   // Throw syntax error using current position
   void M_NORETURN_FUNC ThrowSyntaxError();

private: // Data:

   Nodes m_nodes;
   MConstChars m_begin;
   MConstChars m_end;
   MConstChars m_current;
};

#endif // !M_NO_VARIANT

/// \endcond SHOW_INTERNAL

#endif
