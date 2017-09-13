// File MCORE/MDynamicLibrary.cpp

#include "MCOREExtern.h"
#include "MException.h"
#include "MDynamicLibrary.h"
#include "MCriticalSection.h"

#if !M_NO_DYNAMIC_LIBRARY

typedef std::vector<MDynamicLibrary*>
   DLList;

static MCriticalSection s_loadedModulesCriticalSection;
static DLList s_loadedModules;
static const char MCORE_LIBRARY[] = "MCORE";

   static struct LocalLibrariesUnloader
   {
      ~LocalLibrariesUnloader()
      {
         // Unload DLLs in reverse order
         MCriticalSection::Locker locker(s_loadedModulesCriticalSection);

         DLList::reverse_iterator it    = s_loadedModules.rbegin();
         DLList::reverse_iterator itEnd = s_loadedModules.rend();
         for ( ; it != itEnd; ++it )
            delete *it;
      }
   } s_localLibrariesUnloader;

void MDynamicLibrary::EnsureLibraryIsLoaded(const MStdString& name)
{
   if ( !name.empty() && name != MCORE_LIBRARY )
   {
      MCriticalSection::Locker locker(s_loadedModulesCriticalSection);

      // search for this library in the list, ignore call if it is there
      for ( DLList::iterator it = s_loadedModules.begin(); it != s_loadedModules.end(); ++it )
         if ( name == (*it)->GetName() )
            return;

      MDynamicLibrary* m = MDynamicLibrary::Load(name);
      s_loadedModules.push_back(m);
   }
}

#if M_OS & M_OS_WIN32

MDynamicLibrary::MDynamicLibrary(HMODULE handle, const MStdString& name)
:
   m_handle(handle),
   m_name(name)
{
}

MDynamicLibrary::~MDynamicLibrary()
{
   ::FreeLibrary(m_handle);
   m_handle = 0;
}

MDynamicLibrary* MDynamicLibrary::LoadExact(const MStdString& name, const MStdString& shortName)
{
   #if M_UNICODE
      HMODULE handle = ::LoadLibrary(MToWideString(name).c_str());
   #else
      HMODULE handle = ::LoadLibrary(name.c_str());
   #endif
   MESystemError::CheckLastSystemError(handle == 0);
   return M_NEW MDynamicLibrary(handle, (shortName.size() == 0) ? name : shortName);
}

   #if M_OS & M_OS_WIN32
      static const char s_sExt [] = ".dll";
   #else
      static const char s_sExt [] = ".so";
   #endif
   const int s_sExtSize = M_NUMBER_OF_ARRAY_ELEMENTS(s_sExt) - 1;

MDynamicLibrary* MDynamicLibrary::Load(const MStdString& givenName)
{
   MStdString name = givenName;
   #if M_UNICODE
      HMODULE handle = ::LoadLibrary(MToWideString(name).c_str());
   #else
      HMODULE handle = ::LoadLibrary(name.c_str());
   #endif
   if ( handle == 0 )
   {
      #if defined(_MSC_VER)
         name.append("_vc", 3);
      #elif defined(__BORLANDC__)
         name.append("_cb", 3);
      #endif

      name.append(s_sExt, s_sExtSize);
      #if M_UNICODE
         handle = ::LoadLibrary(MToWideString(name).c_str());
      #else
         handle = ::LoadLibrary(name.c_str());
      #endif
   }

   MESystemError::CheckLastSystemError(handle == 0);
   return M_NEW MDynamicLibrary(handle, givenName);
}

MStdString MDynamicLibrary::GetPath() const
{
   MStdString result;
   TCHAR buffer [ M_MAX_PATH ];
   DWORD count = GetModuleFileName(m_handle, buffer, MAX_PATH);
   M_ASSERT(count > 0);                // Do not throw an error in this case, rather warn on debug time
#if M_UNICODE
   result = MToStdString(buffer, count);   // If count is zero due to error, return an empty string
#else
   result.assign(buffer, count);   // If count is zero due to error, return an empty string
#endif
   return result;
}

MDynamicLibrary::GlobalProcedureType MDynamicLibrary::GetProcedureAddress(MConstChars procedureName)
{
   #if (M_OS & M_OS_WIN32_CE) != 0
      FARPROC proc = GetProcAddress(m_handle, MToWideString(procedureName).c_str());
   #else
      FARPROC proc = GetProcAddress(m_handle, procedureName);
   #endif
   MESystemError::CheckLastSystemError(proc == NULL);
   return (GlobalProcedureType)proc;
}

#else
   #error "MDynamicLibrary is not implemented for this operating system"
#endif

#endif // !M_NO_DYNAMIC_LIBRARY
