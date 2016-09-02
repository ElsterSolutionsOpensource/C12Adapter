// File MCORE/MRegistry.cpp
//
// This is an operating system dependent implementation file
//
#include "MCOREExtern.h"
#include "MRegistry.h"
#include "MAlgorithm.h"
#include "MException.h"

#if !M_NO_REGISTRY

   #if !M_NO_REFLECTION

      /// Initializes the registry access object from location specifying explicitly whether the access will be read-only or read-write.
      ///
      /// \param keyEnum
      ///     Predefined key to use as parent, either of these enumeration values:
      ///       - \ref Registry.KeyClassesRoot
      ///       - \ref Registry.KeyCurrentUser
      ///       - \ref Registry.KeyLocalMachine
      ///       - \ref Registry.KeyUsers
      ///
      /// \param location
      ///     A string that defines the location within the registry, as described in class header.
      ///
      /// \param readonly
      ///     When true, the registry is opened in read-only mode.
      ///     When false, the registry is open for both reading and writing.
      ///
      /// \pre The location is valid and the requested access is allowed, otherwise the exception is thrown.
      ///
      static MRegistry* DoNew3(int keyEnum, const MStdString& location, bool readonly)
      {
         return M_NEW MRegistry(static_cast<MRegistry::PredefinedKeyEnum>(keyEnum), location, readonly);
      }

      /// Creates an empty uninitialized registry access object.
      ///
      /// \see Open - the next logical step after registry access object is created
      ///
      static MRegistry* DoNew0()
      {
         return M_NEW MRegistry();
      }

   #endif

M_START_PROPERTIES(Registry)
   M_CLASS_ENUMERATION                         (Registry, KeyClassesRoot)
   M_CLASS_ENUMERATION                         (Registry, KeyCurrentUser)
   M_CLASS_ENUMERATION                         (Registry, KeyLocalMachine)
   M_CLASS_ENUMERATION                         (Registry, KeyUsers)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT       (Registry, IsOpen)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT       (Registry, IsReadOnly)
   M_OBJECT_PROPERTY_READONLY_STRING_COLLECTION(Registry, AllSubkeys, ST_MStdStringVector_X)
   M_OBJECT_PROPERTY_READONLY_STRING_COLLECTION(Registry, AllValues,  ST_MStdStringVector_X)
M_START_METHODS(Registry)
   M_OBJECT_SERVICE                  (Registry, Open,                 ST_X_int_constMStdStringA_bool)
   M_OBJECT_SERVICE                  (Registry, Close,                ST_X)
   M_OBJECT_SERVICE                  (Registry, IsPresent,            ST_bool_X_constMStdStringA)
   M_OBJECT_SERVICE                  (Registry, IsSubkeyPresent,      ST_bool_X_constMStdStringA)
   M_OBJECT_SERVICE                  (Registry, IsValuePresent,       ST_bool_X_constMStdStringA)
   M_OBJECT_SERVICE                  (Registry, GetString,            ST_MStdString_X_constMStdStringA_constMStdStringA)
   M_OBJECT_SERVICE                  (Registry, GetExistingString,    ST_MStdString_X_constMStdStringA)
   M_OBJECT_SERVICE                  (Registry, GetBinary,            ST_MByteString_X_constMStdStringA_constMByteStringA)
   M_OBJECT_SERVICE                  (Registry, GetExistingBinary,    ST_MByteString_X_constMStdStringA)
   M_OBJECT_SERVICE                  (Registry, GetInteger,           ST_int_X_constMStdStringA_int)
   M_OBJECT_SERVICE                  (Registry, GetExistingInteger,   ST_int_X_constMStdStringA)
   M_OBJECT_SERVICE                  (Registry, SetString,            ST_X_constMStdStringA_constMStdStringA)
   M_OBJECT_SERVICE                  (Registry, SetInteger,           ST_X_constMStdStringA_int)
   M_OBJECT_SERVICE                  (Registry, SetBinary,            ST_X_constMStdStringA_constMByteStringA)
   M_OBJECT_SERVICE                  (Registry, Remove,               ST_X_constMStdStringA)
   M_OBJECT_SERVICE                  (Registry, RemoveValue,          ST_X_constMStdStringA)
   M_OBJECT_SERVICE                  (Registry, RemoveSubkey,         ST_X_constMStdStringA)
   M_OBJECT_SERVICE                  (Registry, CheckIfOpen,          ST_X)
   M_CLASS_FRIEND_SERVICE_OVERLOADED (Registry, New, DoNew0, 0,       ST_MObjectP_S)
   M_CLASS_FRIEND_SERVICE_OVERLOADED (Registry, New, DoNew3, 3,       ST_MObjectP_S_int_constMStdStringA_bool)
M_END_CLASS(Registry, Object)

   // Check values in API, test for two-way conversions - from and to HKEY
   //
   M_COMPILED_ASSERT((HKEY)(ULONG_PTR)((LONG)((unsigned)MRegistry::KeyClassesRoot  | 0x80000000)) == HKEY_CLASSES_ROOT);
   M_COMPILED_ASSERT((HKEY)(ULONG_PTR)((LONG)((unsigned)MRegistry::KeyCurrentUser  | 0x80000000)) == HKEY_CURRENT_USER);
   M_COMPILED_ASSERT((HKEY)(ULONG_PTR)((LONG)((unsigned)MRegistry::KeyLocalMachine | 0x80000000)) == HKEY_LOCAL_MACHINE);
   M_COMPILED_ASSERT((HKEY)(ULONG_PTR)((LONG)((unsigned)MRegistry::KeyUsers        | 0x80000000)) == HKEY_USERS);

   static void DoVerifySystemError(LONG result)
   {
      if ( result != 0 )
      {
         MESystemError::Throw(result, "Error when dealing with Windows registry");
         M_ENSURED_ASSERT(0);
      }
   }

   static const int s_INITIAL_VALUE_BUFFER_SIZE = 256;  // Initial buffer size, value can grow

MRegistry::MRegistry()
:
   MObject(),
   m_key(0),
   m_valueBuffer(NULL),
   m_valueBufferLength(0),
   m_readonly(false)
{
}

MRegistry::MRegistry(MRegistry::PredefinedKeyEnum parentKey, MConstChars location, bool readonly)
:
   MObject(),
   m_key(0),
   m_valueBuffer(NULL),
   m_valueBufferLength(0),
   m_readonly(false)
{
   Open(parentKey, location, readonly);
}

MRegistry::MRegistry(MRegistry::PredefinedKeyEnum parentKey, const MStdString& location, bool readonly)
:
   MObject(),
   m_key(0),
   m_valueBuffer(NULL),
   m_valueBufferLength(0),
   m_readonly(false)
{
   Open(parentKey, location, readonly);
}

MRegistry::MRegistry(const MRegistry& parent, const MStdString& subLocation)
:
   MObject(),
   m_key(0),
   m_valueBuffer(NULL),
   m_valueBufferLength(0)
   // m_readonly be initialized later in Open
{
   OpenSubkey(parent, subLocation);
}

MRegistry::~MRegistry() M_NO_THROW
{
   Close();
   delete[] m_valueBuffer;
}

MStdStringVector MRegistry::GetAllValues() const
{
   MStdStringVector list;

   if ( m_key == 0 )
      return list; // empty list if not open, or if does not exist

   DWORD type;
   TCHAR value [ M_MAX_PATH ];
   for ( DWORD index = 0 ; ; ++index )
   {
      DWORD valueSize = sizeof(value) / sizeof(TCHAR); // do it inside the loop! Each time the key changes to the actual one!
      LONG res = ::RegEnumValue(m_key, index, value, &valueSize, NULL, &type, NULL, NULL);
      if ( res == ERROR_NO_MORE_ITEMS )
         break;
      DoVerifySystemError(res);
      list.push_back(MToStdString(value, valueSize));
   }
   return list;
}

MStdStringVector MRegistry::GetAllSubkeys() const
{
   MStdStringVector list;

   if ( m_key == 0 )
      return list; // empty list if not open, or if does not exist

   TCHAR key [ M_MAX_PATH ];
   for ( DWORD index = 0 ; ; ++index )
   {
      DWORD keySize = sizeof(key) / sizeof(TCHAR); // do it inside the loop! Each time the key changes to the actual one!
      LONG res = ::RegEnumKeyEx(m_key, index, key, &keySize, NULL, NULL, NULL, NULL);
      if ( res == ERROR_NO_MORE_ITEMS )
         break;
      DoVerifySystemError(res);
      list.push_back(MToStdString(key, keySize));
   }
   return list;
}

bool MRegistry::IsReadOnly() const
{
   CheckIfOpen();
   return m_readonly;
}

void MRegistry::CheckIfOpen() const
{
   if ( !IsOpen() )
   {
      MException::Throw(MException::ErrorSoftware, M_ERR_CONFIGURATION_NOT_OPEN, "Registry not open");
      M_ENSURED_ASSERT(0);
   }
}

void MRegistry::DoOpenSubkey(HKEY parentKey, const MStdString& location, bool readonly)
{
   Close();                // Never throws an exception
   m_readonly = readonly;
   HKEY parent = (HKEY)parentKey;
   m_key = NULL;
   if ( m_readonly )
   {
      #if M_UNICODE
         LONG res = ::RegOpenKeyEx(parent, MToWideString(location).c_str(), 0, KEY_READ, &m_key);
      #else
         LONG res = ::RegOpenKeyEx(parent, location.c_str(), 0, KEY_READ, &m_key);
      #endif
      if ( res != ERROR_SUCCESS )
         m_key = NULL; // supposedly no such key (which is okay for read access, no such data will be reported)
   }
   else
   {
      #if M_UNICODE
         LONG res = ::RegCreateKeyEx(parent, MToWideString(location).c_str(), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &m_key, NULL);
      #else
         LONG res = ::RegCreateKeyEx(parent, location.c_str(), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &m_key, NULL);
      #endif
      DoVerifySystemError(res);
      M_ASSERT(m_key != 0);
   }
}

void MRegistry::Open(PredefinedKeyEnum parentKey, const MStdString& location, bool readonly)
{
   DoOpenSubkey(reinterpret_cast<HKEY>(static_cast<size_t>(parentKey | 0x80000000u)), location, readonly);
}

void MRegistry::OpenSubkey(const MRegistry& parent, const MStdString& subLocation)
{
   DoOpenSubkey(parent.m_key, subLocation, parent.IsReadOnly());
}

void MRegistry::Close()
{
   if ( m_key != 0 )
   {
      ::RegCloseKey(m_key);
      m_key = 0;
   }
}

bool MRegistry::IsPresent(const MStdString& keyOrValue) const
{
   return IsValuePresent(keyOrValue) || IsSubkeyPresent(keyOrValue);
}

bool MRegistry::IsSubkeyPresent(const MStdString& key) const
{
   CheckIfOpen();
   HKEY newKey;
   #if M_UNICODE
      LONG res = ::RegOpenKeyEx(m_key, MToWideString(key).c_str(), 0, KEY_READ, &newKey);
   #else
      LONG res = ::RegOpenKeyEx(m_key, key.c_str(), 0, KEY_READ, &newKey);
   #endif
   if ( res == ERROR_SUCCESS )
   {
      ::RegCloseKey(newKey);
      return true;
   }
   return false;
}

bool MRegistry::IsValuePresent(const MStdString& value) const
{
   return DoQueryValue(value) >= 0;
}

int MRegistry::DoQueryValue(const MStdString& key, DWORD* type) const
{
   if ( m_key == 0 )
      return -1;

   if ( m_valueBufferLength == 0 )
   {
      M_ASSERT(m_valueBuffer == NULL);
      m_valueBuffer = M_NEW char [ s_INITIAL_VALUE_BUFFER_SIZE ];
      m_valueBufferLength = s_INITIAL_VALUE_BUFFER_SIZE;
   }

   // CAREFUL: don't give m_valueBufferLength the pointer to RegQueryValueEx!
   //
   DWORD count = m_valueBufferLength;

   #if M_UNICODE
      LONG res = ::RegQueryValueEx(m_key, MToWideString(key).c_str(), NULL, type, (LPBYTE)m_valueBuffer, &count);
   #else
      LONG res = ::RegQueryValueEx(m_key, key.c_str(), NULL, type, (LPBYTE)m_valueBuffer, &count);
   #endif
   if ( res == ERROR_MORE_DATA ) // allow us call this function second time in this case
   {
      M_ASSERT(m_valueBufferLength < count);
      delete[] m_valueBuffer;
      m_valueBuffer = M_NEW char [ count ];
      m_valueBufferLength = count;

      // We could perform a recursion here, but prefer a safe two-times algorithm.
      // Count continues to have a desired value.
      //
      #if M_UNICODE
         res = ::RegQueryValueEx(m_key, MToWideString(key).c_str(), NULL, type, (LPBYTE)m_valueBuffer, &count);
      #else
         res = ::RegQueryValueEx(m_key, key.c_str(), NULL, type, (LPBYTE)m_valueBuffer, &count);
      #endif
   }
   return (res != ERROR_SUCCESS) ? -1 : int(count);
}

   static MStdString DoGetStringValue(const char* valueBuffer, unsigned size)
   {

      #if M_UNICODE
         const wchar_t* buff = reinterpret_cast<const wchar_t*>(valueBuffer);
         size /= sizeof(wchar_t);
         if ( size > 0 && buff[size - 1] == L'\0' )
            --size; // if the trailing zero is present, cut it out
         return MToStdString(buff, size);
      #else
         if ( size > 0 && valueBuffer[size - 1] == '\0' )
            --size; // if the trailing zero is present, cut it out
         return MToStdString(valueBuffer, size);
      #endif
   }

   static int DoGetIntValue(const char* valueBuffer, unsigned size, DWORD type)
   {
      M_ASSERT(type == REG_DWORD || type == REG_DWORD_LITTLE_ENDIAN || type == REG_DWORD_BIG_ENDIAN);
      if ( size != sizeof(DWORD) )
      {
         MException::Throw(MException::ErrorConfiguration, MErrorEnum::BadConversion, M_I("Integer is expected to be four bytes"));
         M_ENSURED_ASSERT(0);
      }
      if ( type == REG_DWORD_BIG_ENDIAN )
         return MFromBigEndianUINT32(valueBuffer);
      else
         return MFromLittleEndianUINT32(valueBuffer);
   }

MStdString MRegistry::GetString(const MStdString& key, const MStdString& defaultValue) const
{
   return DoGetString(key, &defaultValue);
}

MStdString MRegistry::GetExistingString(const MStdString& key) const
{
   return DoGetString(key);
}

MStdString MRegistry::DoGetString(const MStdString& key, const MStdString* defaultValue) const
{
   MStdString result;
   DWORD type;
   int size = DoQueryValue(key, &type);
   if ( size > 0 )
   {
      if ( type == REG_DWORD || type == REG_DWORD_LITTLE_ENDIAN || type == REG_DWORD_BIG_ENDIAN )
         result = MToStdString(DoGetIntValue(m_valueBuffer, size, type));
      else
         result = DoGetStringValue(m_valueBuffer, size);
   }
   else if ( size < 0 )
   {
      if ( defaultValue == NULL )
      {
         MException::ThrowUnknownItem(MException::ErrorConfiguration, key);
         M_ENSURED_ASSERT(0);
      }
      result = *defaultValue;
   }
   return result;
}

int MRegistry::GetInteger(const MStdString& key, int defaultValue) const
{
   return DoGetInteger(key, &defaultValue);
}

int MRegistry::GetExistingInteger(const MStdString& key) const
{
   return DoGetInteger(key);
}

int MRegistry::DoGetInteger(const MStdString& key, int* defaultValue) const
{
   DWORD type;
   int size = DoQueryValue(key, &type);
   if ( size > 0 ) // zero size would be a bad integer
   {
      try
      {
         if ( type == REG_DWORD || type == REG_DWORD_LITTLE_ENDIAN || type == REG_DWORD_BIG_ENDIAN )
            return DoGetIntValue(m_valueBuffer, size, type);
         else
         {
            MStdString str = DoGetStringValue(m_valueBuffer, size);
            MAlgorithm::InplaceTrim(str);
            return MToInt(str);
         }
      }
      catch ( ... ) // Return default on bad conversion too
      {
      }
   }
   if ( defaultValue == NULL )
   {
      MException::ThrowUnknownItem(MException::ErrorConfiguration, key);
      M_ENSURED_ASSERT(0);
   }
   return *defaultValue;
}

MByteString MRegistry::GetBinary(const MStdString& key, const MByteString& defaultValue) const
{
   int size = DoQueryValue(key);
   if ( size >= 0 ) // zero size would be a bad integer anyway
      return MByteString((const char*)m_valueBuffer, size);
   return defaultValue;
}

MByteString MRegistry::GetExistingBinary(const MStdString& key) const
{
   int size = DoQueryValue(key);
   if ( size < 0 )
   {
      MException::ThrowUnknownItem(MException::ErrorConfiguration, key);
      M_ENSURED_ASSERT(0);
   }
   return MByteString((const char*)m_valueBuffer, size);
}

void MRegistry::SetString(const MStdString& key, const MStdString& value)
{
   M_ASSERT(!m_readonly); // do it in assert, as the OS should throw an error below otherwise

   // This works for both UNICODE and plain char if the type is REG_SZ.
   //
   #if M_UNICODE
      MWideString wideValue = MToWideString(value);
      size_t size = (wideValue.size() + 1) * sizeof(wchar_t); // note +1, this is to include a terminating zero
      LONG res = ::RegSetValueEx(m_key, MToWideString(key).c_str(), NULL, REG_SZ, (LPBYTE)wideValue.c_str(), M_64_CAST(DWORD, size));
   #else
      size_t size = value.size() + 1; // note +1, this is to include a terminating zero
      LONG res = ::RegSetValueEx(m_key, key.c_str(), NULL, REG_SZ, (LPBYTE)value.c_str(), M_64_CAST(DWORD, size));
   #endif
   DoVerifySystemError(res);
}

void MRegistry::SetInteger(const MStdString& key, int value)
{
   MChar str [ 16 ]; // integers are always set as strings
   SetString(key, MToChars(long(value), str));
}

void MRegistry::SetBinary(const MStdString& key, const MByteString& value)
{
   M_ASSERT(!m_readonly); // do it in assert, as the OS should throw an error below otherwise

   // This works for both UNICODE and plain char if the type is REG_SZ.
   //
   #if M_UNICODE
      LONG res = ::RegSetValueEx(m_key, MToWideString(key).c_str(), NULL, REG_BINARY, (LPBYTE)value.c_str(), M_64_CAST(DWORD, value.size()));
   #else
      LONG res = ::RegSetValueEx(m_key, key.c_str(), NULL, REG_BINARY, (LPBYTE)value.c_str(), M_64_CAST(DWORD, value.size()));
   #endif
   DoVerifySystemError(res);
}

void MRegistry::Remove(const MStdString& keyOrValue)
{
   #if M_UNICODE
      MWideString wideKeyAndValue = MToWideString(keyOrValue);
      LONG res = ::RegDeleteValue(m_key, wideKeyAndValue.c_str());
      if ( !SUCCEEDED(res) )
      {
         LONG res = ::RegDeleteKey(m_key, wideKeyAndValue.c_str());
         DoVerifySystemError(res);
      }
   #else
      LONG res = ::RegDeleteValue(m_key, keyOrValue.c_str());
      if ( !SUCCEEDED(res) )
      {
         LONG res = ::RegDeleteKey(m_key, keyOrValue.c_str());
         DoVerifySystemError(res);
      }
   #endif
}

void MRegistry::RemoveValue(const MStdString& valueName)
{
   #if M_UNICODE
      LONG res = ::RegDeleteValue(m_key, MToWideString(valueName).c_str());
   #else
      LONG res = ::RegDeleteValue(m_key, valueName.c_str());
   #endif
   DoVerifySystemError(res);
}

void MRegistry::RemoveSubkey(const MStdString& key)
{
   #if M_UNICODE
      LONG res = ::RegDeleteKey(m_key, MToWideString(key).c_str());
   #else
      LONG res = ::RegDeleteKey(m_key, key.c_str());
   #endif
   DoVerifySystemError(res);
}

#endif // !M_NO_REGISTRY
