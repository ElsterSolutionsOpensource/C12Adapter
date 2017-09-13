// File MCORE/MDictionary.cpp

#include "MCOREExtern.h"
#include "MCOREDefs.h"
#include "MException.h"
#include "MDictionary.h"
#include "MUtilities.h"
#include "MAlgorithm.h"
#include "MStr.h"

#if !M_NO_VARIANT

#if !M_NO_REFLECTION
   static MDictionary* DoNew0()
   {
      return M_NEW MDictionary();
   }
   static MDictionary* DoNew1(const MVariant& initializeStringOrCopy)
   {
      if ( initializeStringOrCopy.IsObject() )
      {
         const MDictionary* dict = M_DYNAMIC_CAST_WITH_THROW(const MDictionary, initializeStringOrCopy.DoInterpretAsObject());
         M_ASSERT(dict != NULL);
         return dict->NewClone();
      }
      return M_NEW MDictionary(initializeStringOrCopy.AsString());
   }
#endif

M_START_PROPERTIES(Dictionary)
   M_OBJECT_PROPERTY_READONLY_UINT              (Dictionary, Count)
   M_OBJECT_PROPERTY_STRING_EXACT               (Dictionary, AsString,                    ST_MStdString_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_READONLY_STRING_EXACT      (Dictionary, AsStringUnsorted,            ST_MStdString_X)
   M_OBJECT_PROPERTY_READONLY_VARIANT_COLLECTION(Dictionary, AllKeys,                     ST_MVariantVector_X)
   M_OBJECT_PROPERTY_READONLY_VARIANT_COLLECTION(Dictionary, AllValues,                   ST_MVariantVector_X)
M_START_METHODS(Dictionary)
   M_OBJECT_SERVICE                            (Dictionary, Clear,                        ST_X)
   M_OBJECT_SERVICE                            (Dictionary, Item,                         ST_constMVariantA_X_constMVariantA)
   M_OBJECT_SERVICE                            (Dictionary, SetItem,                      ST_X_constMVariantA_constMVariantA)
   M_OBJECT_SERVICE                            (Dictionary, IsKeyPresent,                 ST_bool_X_constMVariantA)
   M_OBJECT_SERVICE                            (Dictionary, IsValuePresent,               ST_bool_X_constMVariantA)
   M_OBJECT_SERVICE                            (Dictionary, Remove,                       ST_X_constMVariantA)
   M_OBJECT_SERVICE                            (Dictionary, RemoveIfPresent,              ST_bool_X_constMVariantA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED           (Dictionary, New, DoNew0,               0, ST_MObjectP_S)
   M_CLASS_FRIEND_SERVICE_OVERLOADED           (Dictionary, New, DoNew1,               1, ST_MObjectP_S_constMVariantA)
   M_OBJECT_SERVICE                            (Dictionary, NewClone,                     ST_MObjectP_X)
M_END_CLASS(Dictionary, Object)

   static void M_NORETURN_FUNC DoThrowNoValueFor(const MStdString& name)
   {
      MException::Throw(MErrorEnum::NoValue, "No value given for '%s'", name.c_str());
      M_ENSURED_ASSERT(0);
   }

MDictionary::MDictionary()
:
   MObject(),
   m_map(MVariant::VAR_MAP)
{
}

MDictionary::MDictionary(const MStdString& properties)
:
   MObject(),
   m_map(MVariant::VAR_MAP)
{
   SetAsString(properties);
}


MDictionary::MDictionary(const MDictionary& other)
:
   MObject(),
   m_map(other.m_map)
{
}

MDictionary::~MDictionary()
{
}

MDictionary* MDictionary::NewClone() const
{
   return M_NEW MDictionary(*this);
}

void MDictionary::SetAsString(const MStdString& properties)
{
   Clear();
   DoAddKeysValues(properties);
}

void MDictionary::Clear()
{
   m_map.SetCount(0);
}

MVariant* MDictionary::GetValue(const MVariant& key)
{
   if ( m_map.IsPresent(key) )
      return &m_map.AccessItem(key);
   return NULL;
}

MVariant::VariantVector MDictionary::GetAllKeys() const
{
   return m_map.GetAllMapKeys().AsVariantCollection();
}

MVariant::VariantVector MDictionary::GetAllValues() const
{
   return m_map.GetAllMapValues().AsVariantCollection();
}

   static MStdString DoAsString(const MVariant& map)
   {
      MStdString result;
      int count = map.GetCount();
      for ( int i = 0; i < count; ++i )
      {
         result += map.GetMapKeyByIndex(i).AsString();
         result += '=';
         result += MUtilities::ToRelaxedMDLConstant(map.GetMapValueByIndex(i));
         result += ';';
      }
      return result;
   }

MStdString MDictionary::AsString() const
{
   return DoAsString(MAlgorithm::Sort(m_map));
}

MStdString MDictionary::AsStringUnsorted() const
{
   return DoAsString(m_map);
}

bool MDictionary::IsKeyPresent(const MVariant& key) const
{
   return m_map.IsPresent(key);
}

bool MDictionary::IsValuePresent(const MVariant& val) const
{
   int count = m_map.GetCount();
   for ( int i = 0; i < count; ++i )
      if ( m_map.GetMapValueByIndex(i) == val )
         return true;
   return false;
}

   static void DoThrowDictionaryHasNoSuchKey(const MVariant& key)
   {
      MException::Throw(M_ERR_DICTIONARY_DOES_NOT_HAVE_KEY_S1, "The dictionary does not have key '%s'", key.AsEscapedString().c_str());
      M_ENSURED_ASSERT(0);
   }

void MDictionary::Remove(const MVariant& key)
{
   if ( !RemoveIfPresent(key) )
   {
      DoThrowDictionaryHasNoSuchKey(key);
      M_ENSURED_ASSERT(0);
   }
}

bool MDictionary::RemoveIfPresent(const MVariant& key)
{
   int count = m_map.GetCount();
   m_map -= key;
   return count != m_map.GetCount();
}

const MVariant& MDictionary::Item(const MVariant& key) const
{
   return m_map.AccessItem(key);
}

void MDictionary::SetItem(const MVariant& key, const MVariant& val)
{
   m_map.SetItem(key, val);
}

void MDictionary::Merge(const MDictionary& dict)
{
   m_map += dict.m_map;
}

   inline bool NolocaleIsSpace(char c)
   {
      return c == ' ' || (c >= '\t' && c <= '\xD');
   }

void MDictionary::DoAddKeysValues(const MStdString& values)
{
   MStdString name;
   MStdString value;

   if ( m_strncmp(values.c_str(), "J00[", 4) == 0 ) // J strings always start with this.
   {
      // configuration is not used by J

      enum ScanState
         {
            STATE_NAME,         // Scanning the name
            STATE_VALUE         // Scanning ordinary value
         };
      ScanState state = STATE_NAME;
      for ( MStdString::const_iterator it = values.begin() + 4; it != values.end(); ++it )
      {
         char ch = *it;
         switch ( state )
         {
         case STATE_NAME:
            if ( ch == ':' )
               state = STATE_VALUE; // go on with the value
            else
               name += ch;
            break;
         case STATE_VALUE:
            if ( ch == ']' )
            {
               if ( it + 1 < values.end() && *(it + 1) == '[' )
                  ++it; // skip '[' if it is there
               if ( name.size() > 0 ) // by convention, skip unnamed objects
               {
                  m_map.SetItem(name, value); // what we need...
                  name.clear();
               }
               value.clear();
               state = STATE_NAME;
            }
            else
               value += ch;
            break;
         }
      }

      // Handle non-terminating end state
      //
      if ( state == STATE_VALUE )
      {
         DoThrowNoValueFor(name);
         M_ENSURED_ASSERT(0);
      }
   }
   else // non-J list...
   {
      enum ScanState
         {
            STATE_EXPECT_NAME,   // Next is name, may be after some blanks and ';'
            STATE_NAME,          // Scanning the name
            STATE_EXPECT_VALUE,  // Next is value or string value
            STATE_VALUE,         // Scanning ordinary value
            STATE_STRING_VALUE,  // Parsing string value enclosed in ""
            STATE_STRING_VALUE_BACKSLASH, // Parsing backslash, possibly '\"'
            STATE_CHAR_VALUE,    // Parsing char value enclosed in ''
            STATE_CHAR_VALUE_BACKSLASH, // Parsing backslash, possibly '\''
            STATE_EXPECT_SEMICOLON // property separator or end string expected
         };
      ScanState state = STATE_EXPECT_NAME;
      for ( MStdString::const_iterator it = values.begin(); it != values.end(); ++it )
      {
         char ch = *it;
         switch ( state )
         {
         case STATE_EXPECT_NAME:
            if ( NolocaleIsSpace(ch) || ch == ';' )
            {
               /* skip spaces and extra semicolons */
            }
            else if ( ch == '=' )
            {
               MException::ThrowUnexpectedChar(ch);
               M_ENSURED_ASSERT(0);
            }
            else
            {
               state = STATE_NAME;
               name = ch; // start filling property name
            }
            break;
         case STATE_NAME:
            if ( ch == '=' )
            {
               MAlgorithm::InplaceTrim(name);
               state = STATE_EXPECT_VALUE;
            }
            else if ( ch == ';' )
            {
                  DoThrowNoValueFor(name);
                  M_ENSURED_ASSERT(0);
               state = STATE_EXPECT_NAME;
            }
            else
               name += ch;
            break;
         case STATE_EXPECT_VALUE:
            if ( ch == '"' )
            {
               value.clear();
               state = STATE_STRING_VALUE;
            }
            else if ( ch == '\'' )
            {
               value.clear();
               state = STATE_CHAR_VALUE;
            }
            else if ( ch == ';' )
            {
               DoThrowNoValueFor(name);
               M_ENSURED_ASSERT(0);
            }
            else if ( NolocaleIsSpace(ch) )
            {
               /* skip spaces */
            }
            else
            {
               value = ch; // start filling property value
               state = STATE_VALUE;
            }
            break;
         case STATE_VALUE:
            if ( ch == ';' )
            {
               state = STATE_EXPECT_NAME;
               MAlgorithm::InplaceTrim(value);
               M_ASSERT(name.size() > 0 && value.size() > 0);
               m_map.SetItem(name, value); // what we need...
            }
            else
               value += ch;
            break;
         case STATE_STRING_VALUE:
            if ( ch == '"' )
            {
               M_ASSERT(name.size() > 0);
               m_map.SetItem(name, MStr::FromEscapedString(value)); // what we need...
               state = STATE_EXPECT_SEMICOLON;
            }
            else if ( ch == '\\' )
               state = STATE_STRING_VALUE_BACKSLASH;
            else
               value += ch;
            break;
         case STATE_STRING_VALUE_BACKSLASH:
            if ( ch != '"' )
               value += '\\';
            value += ch;
            state = STATE_STRING_VALUE;
            break;
         case STATE_CHAR_VALUE:
            if ( ch == '\'' )
            {
               M_ASSERT(name.size() > 0);
               MVariant tmp = MStr::FromEscapedString(value);
               m_map.SetItem(name, tmp.AsChar()); // what we need is char. Try if the conversion is successful
               state = STATE_EXPECT_SEMICOLON;
            }
            else if ( ch == '\\' )
               state = STATE_CHAR_VALUE_BACKSLASH;
            else
               value += ch;
            break;
         case STATE_CHAR_VALUE_BACKSLASH:
            if ( ch != '\'' )
               value += '\\';
            value += ch;
            state = STATE_CHAR_VALUE;
            break;
         case STATE_EXPECT_SEMICOLON:
            if ( ch == ';' )
               state = STATE_EXPECT_NAME;
            else if ( !NolocaleIsSpace(ch) )
            {
               MException::ThrowUnexpectedChar(ch);
               M_ENSURED_ASSERT(0);
            }
         }// switch
      }// for

      // Handle end states
      //
      switch ( state )
      {
      case STATE_EXPECT_NAME:
      case STATE_EXPECT_SEMICOLON:
         break;  // successful end-states
      case STATE_VALUE:
         M_ASSERT(!name.empty() && !value.empty());
         m_map.SetItem(name, value); // what we need...
         break; // successful end-state
      case STATE_NAME:
            DoThrowNoValueFor(name);
            M_ENSURED_ASSERT(0);
         break;
      case STATE_EXPECT_VALUE:
         DoThrowNoValueFor(name);
         M_ENSURED_ASSERT(0);
      case STATE_STRING_VALUE:
      case STATE_STRING_VALUE_BACKSLASH:
      case STATE_CHAR_VALUE:
      case STATE_CHAR_VALUE_BACKSLASH:
         MException::Throw(M_ERR_UNTERMINATED_STRING, "Unterminated string");
         M_ENSURED_ASSERT(0);
      default:
         M_ENSURED_ASSERT(0);
      }
   }
}


#endif // !M_NO_VARIANT
