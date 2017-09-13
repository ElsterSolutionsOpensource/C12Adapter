// File MCOM/IdentifyString.cpp

#include "MCOMExtern.h"
#include "IdentifyString.h"

#if !M_NO_MCOM_IDENTIFY_METER

   const MConstChars MIdentifyString::s_unsupported = "J00[UNSUPPORTED_METER]";

MIdentifyString::MIdentifyString()
:
   MStdString("J00", 3),
   m_obTagOrdinalNumber(0)
{
}

void MIdentifyString::AppendTag(MConstChars tag, const MStdString& value)
{
   operator+=('[');
   operator+=(tag);
   operator+=(':');
   operator+=(value);
   operator+=(']');
}

void MIdentifyString::AppendTag(MConstChars tag, const char* value, unsigned len)
{
   if ( len > 0 )
      AppendTag(tag, MToStdString(value, len));
   else
      AppendTag(tag, MToStdString(value));
}

void MIdentifyString::AppendTag(MConstChars tag, unsigned v)
{
   char str [ 16 ];
   MStdString value = MToChars((unsigned long)v, str);
   AppendTag(tag, value);
}

void MIdentifyString::AppendTag(MConstChars tag, unsigned v1, unsigned v2)
{
   char str [ 16 ];
   MStdString value = MToChars((unsigned long)v1, str);
   value += '.';
   value += MToChars((unsigned long)v2, str);
   AppendTag(tag, value);
}

void MIdentifyString::AppendHexTag(MConstChars tag, const char* value, unsigned len)
{
   operator+=('[');
   operator+=(tag);
   operator+=(':');
   operator+=(MUtilities::BufferToHex(value, len, false));
   operator+=(']');
}

bool MIdentifyString::AppendObTags(int position, const char* sspec, unsigned group, unsigned revnum, const char* type)
{
   int typeSize;
   if ( type == NULL )
   {
      type = "NOT_AVAILABLE";
      typeSize = 13;
   }
   else
   {
      if ( type[0] == '\0' && type[1] == '\0' )
         return false;
      typeSize = 2;
   }

   if ( sspec[0] == '\0' && sspec[1] == '\0' && sspec[2] == '\0' )
      return false;

   ++m_obTagOrdinalNumber;

   char nameBuffer [ 32 ]; // add some for alignment
   nameBuffer[0] = 'O';
   nameBuffer[1] = 'B';
   nameBuffer[2] = char(m_obTagOrdinalNumber + '0'); // we know we are ASCII
   nameBuffer[3] = '_';
   char* nameBufferStart = nameBuffer + 4;

   #define M__DO_COPY(s)  memcpy(nameBufferStart, (s), sizeof(s));

   M__DO_COPY("TYPE");
   AppendTag(nameBuffer, type, typeSize);
   M__DO_COPY("SSPEC");
   AppendHexTag(nameBuffer, sspec, 3);
   M__DO_COPY("REVISION");
   AppendTag(nameBuffer, group, revnum);
   M__DO_COPY("POSITION");
   AppendTag(nameBuffer, position);

   #undef M__DO_COPY

   return true;
}

void MIdentifyString::InsertNumberOfObTags()
{
   M_ASSERT(m_obTagOrdinalNumber >= 0);

   char buff [ 64 ];
   MFormat(buff, sizeof(buff), "[NUMBER_OF_OPTION_BOARDS:%d]", m_obTagOrdinalNumber);

   MStdString::size_type pos = find("[OB1_");
   if ( pos == MStdString::npos )
      pos = size();
#ifdef M_USE_USTL
   insert(pos, buff, strlen(buff));
#else
   insert(pos, buff);
#endif
}

void MIdentifyString::AppendNew()
{
   m_obTagOrdinalNumber = 0;
   append(";J00", 4); // create a new J here
}

#endif
