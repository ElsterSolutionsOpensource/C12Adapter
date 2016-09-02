// File MCORE/MCOREDefs.cpp

#include "MCOREExtern.h"
#include "MCORE.h"

using namespace std;

   static unsigned  s_runtimeOSMask = M_OS;

#if M_OS & M_OS_WINDOWS
   static HINSTANCE s_DLLInstance = NULL;                             // instance of the current DLL or EXE
#endif

   #if !M_NO_AUTOMATION
      M_LINK_THE_CLASS_IN(MAutomation)
   #endif
   #if !M_NO_FILESYSTEM
      M_LINK_THE_CLASS_IN(MIniFile)
   #endif
   M_LINK_THE_CLASS_IN(MErrorEnum)
   M_LINK_THE_CLASS_IN(MMD5Checksum)
   M_LINK_THE_CLASS_IN(MGuid)
   M_LINK_THE_CLASS_IN(MMath)


   #if !M_NO_XML
      M_LINK_THE_CLASS_IN(MXmlNode)
      M_LINK_THE_CLASS_IN(MXmlDocument)
   #endif

   #if !M_NO_SOCKETS_UDP
      M_LINK_THE_CLASS_IN(MStreamSocketUdp)
   #endif
   #if !M_NO_SOCKETS
      M_LINK_THE_CLASS_IN(MStreamSocket)
   #endif

   // Having this class static makes the constructor
   // of AInitDebugSupport executed once during initialization
   //
   static const struct MInit
   {

      // For VC++ define the standard behavior for new operator. This means that it
      // throws an exception in case when there is not enough memory to complete request.
      //
      #if defined(_MSC_VER) && ((M_OS & M_OS_WIN32_CE) == 0)

         _PNH m_oldHandler;

         static int NewHandler(size_t)
         {
            throw bad_alloc();
         }

      #endif

      MInit()
      {
         #if defined(_MSC_VER) && ((M_OS & M_OS_WIN32_CE) == 0)

            #if defined(M_DEBUG) && !defined(__BOUNDSCHECKER__)
            {
               int flg = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG); // Get flag
               flg |= _CRTDBG_LEAK_CHECK_DF; // Turn on leak checking bit
               _CrtSetDbgFlag(flg);          // Set flag to the new value
            }
            #endif

            m_oldHandler = _set_new_handler(NewHandler);

         #endif

         #if M_OS & M_OS_WINDOWS
            #if M_DYNAMIC == 0
               // Otherwise it will be set in DllMain()
               s_DLLInstance = GetModuleHandle(NULL);
            #endif
         #endif

         srand(MTimer::GetTickCount()); // seed the randomizer with the current time

      }

      #if defined(_MSC_VER) && ((M_OS & M_OS_WIN32_CE) == 0)
         ~MInit()
         {
            _set_new_handler(m_oldHandler);
         }
      #endif

   } s_init;

#if M_DYNAMIC && M_OS & M_OS_WINDOWS

   // Entry point for Windows version of MCORE library
#if (M_OS & M_OS_WIN32_CE) != 0 // prevent "cannot overload" error on CE
   BOOL WINAPI DllMain(HANDLE hinst, DWORD dwReason, LPVOID lpvReserved)
#else
   int WINAPI DllMain(HINSTANCE hinst, unsigned long, void*)
#endif
   {
      s_DLLInstance = (HINSTANCE)hinst; // cast required for CE
      M_ASSERT(s_DLLInstance != NULL);
      return TRUE;
   }

#endif

#if M_OS & M_OS_WINDOWS
M_FUNC void MSetDLLInstance(HINSTANCE inst) M_NO_THROW
{
   s_DLLInstance = inst;
}

M_FUNC HINSTANCE MGetDLLInstance() M_NO_THROW
{
   return s_DLLInstance;
}
#endif

M_FUNC MStdString MGetStdString(MConstLocalChars str, ...) M_NO_THROW
{
   #if M_NO_MESSAGE_CATALOG
      va_list va;
      va_start(va, str);
      const MStdString& result = MGetStdStringVA((const char*)str, va);
      va_end(va);
      return result;
   #else
      va_list va;
      va_start(va, str);
      const MStdString& result = MMessageCatalog::GetVaTextDefault(M_GLOBAL_MESSAGE_CATALOG_DOMAIN, str, va);
      va_end(va);
      return result;
   #endif
}

M_FUNC MStdString MGetStdString(MConstChars str, ...) M_NO_THROW
{
   va_list va;
   va_start(va, str);
   const MStdString& result = MGetStdStringVA(str, va);
   va_end(va);
   return result;
}

M_FUNC MStdString MGetStdStringVA(MConstLocalChars str, va_list va) M_NO_THROW
{
   #if M_NO_MESSAGE_CATALOG
      return MGetStdStringVA((const char*)str, va);
   #else
      return MMessageCatalog::GetVaTextDefault(M_GLOBAL_MESSAGE_CATALOG_DOMAIN, str, va);
   #endif
}

   const size_t preallocatedSize = 2048;

M_FUNC MStdString MGetStdStringVA(MConstChars format, va_list args) M_NO_THROW
{
   MStdString response;
#ifdef __GLIBC__
   va_list args2;
# if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L || defined(__GXX_EXPERIMENTAL_CXX0X__)
   va_copy(args2, args);
# else
   __va_copy(args2, args);
# endif
#endif
   MChar buffer [ preallocatedSize ];
   size_t size = MFormatVA(buffer, preallocatedSize, format,
#ifdef __GLIBC__
                             args2
#else
                             args
#endif
      );
   if ( size < static_cast<size_t>(preallocatedSize) )
   {
      response.assign(buffer);
   }
   else
   {
      response.resize(size);
      MFormatVA(&response[0], size + 1, format, args);
   }
   return response;
}

M_FUNC MChar* MSignedToString(Mint64 value, MChar* string, size_t& length);
M_FUNC MChar* MUnsignedToString(Muint64 value, MChar* string, size_t& length);
M_FUNC MChar* MSignedToString(int value, MChar* string, size_t& length);
M_FUNC MChar* MUnsignedToString(unsigned int value, MChar* string, size_t& length);

M_FUNC MStdString MToStdString(Mint64 value)
{
   size_t length;
   MChar buffer[64];
   MChar* ptr = MSignedToString(value, &buffer[64], length);
   return MStdString(ptr, length);
}

M_FUNC MStdString MToStdString(Muint64 value)
{
   size_t length;
   MChar buffer[64];
   MChar* ptr = MUnsignedToString(value, &buffer[64], length);
   return MStdString(ptr, length);
}

M_FUNC MStdString MToStdString(int value)
{
   size_t length;
   MChar buffer[32];
   MChar* ptr = MSignedToString(value, &buffer[32], length);
   return MStdString(ptr, length);
}

M_FUNC MStdString MToStdString(unsigned value)
{
   size_t length;
   MChar buffer[32];
   MChar* ptr = MUnsignedToString(value, &buffer[32], length);
   return MStdString(ptr, length);
}

M_FUNC MStdString MToStdString(double value)
{
   MChar buff [ 128 ]; // Maximum number of characters in the number
   return MStdString(MToChars(value, buff));
}

#if !M_NO_WCHAR_T
M_FUNC MWideString MToWideString(const char* str)
{
   MWideString result;
   if ( str != NULL )
      result = MToWideString(str, strlen(str));
   return result;
}

M_FUNC MWideString MToWideString(const wchar_t* str)
{
   MWideString result;
   if ( str != NULL )
      result.assign(str, wcslen(str));
   return result;
}

M_FUNC MWideString MToWideString(const char* string, size_t stringLength)
{
   MWideString wstring;
   if ( stringLength != 0 )
   {
      #if (M_OS & M_OS_WINDOWS)
         int wstringLength = MultiByteToWideChar(M_WINDOWS_CP_ACP, 0, string, M_64_CAST(int, stringLength), 0, 0);
         if ( wstringLength == 0 )
            MESystemError::ThrowLastSystemError();
         wstring.resize(wstringLength);
         (void)MultiByteToWideChar(M_WINDOWS_CP_ACP, 0, string, M_64_CAST(int, stringLength), &wstring[0], wstringLength);
      #elif (M_OS & M_OS_QNXNTO) || (M_OS & M_OS_BSD)
         std::locale currentLocale("");
         typedef std::string::traits_type::state_type State;
         typedef std::codecvt<wchar_t, char, State> CodeCVT;
         const CodeCVT& cc = std::use_facet<CodeCVT>(currentLocale);
         State state = State();

         const char* fromBeg = string;
         const char* fromEnd = string + stringLength;
         const char* fromNxt = NULL;

         wchar_t tmpBuf[256];
         wchar_t* toBeg = tmpBuf;
         wchar_t* toEnd = toBeg + 256;
         wchar_t* toNxt = NULL;

         wstring.reserve(stringLength);
         for ( ;; )
         {
            CodeCVT::result res = cc.in(state, fromBeg, fromEnd, fromNxt, toBeg, toEnd, toNxt);
            if ( res == CodeCVT::error )
            {
               wstring.append(toBeg, toNxt);
               wstring += L'?';
               fromBeg = fromNxt + 1;

               state = State();
            }
            else if ( res == CodeCVT::partial )
            {
               wstring.append(toBeg, toNxt);
               fromBeg = fromNxt;
            }
            else if ( res == CodeCVT::ok )
            {
               wstring.append(toBeg, toNxt);
               break;
            }
         }
      #else
         size_t converted = 0;
         size_t needSize = 0;

         while ( converted <= stringLength )
         {
            mbstate_t ps = {0};
            const char* source = string + converted;
            const size_t sourceLength = Mstrnlen(source, stringLength - converted);
            const size_t n = mbsnrtowcs(0, &source, sourceLength, 0, &ps);
            converted += sourceLength + 1;
            needSize += ( n == static_cast<size_t>(-1) ) ? 1 : n + 1;
         }
         wstring.resize(needSize - 1);
         const size_t fullSize = needSize;
         converted = needSize = 0;
         while ( converted <= stringLength )
         {
            mbstate_t ps = {0};
            const char* source = string + converted;
            const size_t sourceLength = Mstrnlen(source, stringLength - converted);
            const size_t n = mbsnrtowcs(&wstring[needSize], &source, sourceLength, fullSize - needSize, &ps);
            converted += sourceLength + 1;
            needSize += ( n == static_cast<size_t>(-1) ) ? 1 : n + 1;
         }
      #endif
   }
   return wstring;
}

M_FUNC MStdString MToStdString(const wchar_t* wstring, size_t wstringLength)
{
   MStdString string;
   if ( wstringLength != 0 )
   {
      #if (M_OS & M_OS_WINDOWS)
         int stringLength = WideCharToMultiByte(M_WINDOWS_CP_ACP, 0, wstring, M_64_CAST(int, wstringLength), 0, 0, NULL, NULL);
         if ( stringLength == 0 )
            MESystemError::ThrowLastSystemError();
         string.resize(stringLength);
         (void)WideCharToMultiByte(M_WINDOWS_CP_ACP, 0, wstring, M_64_CAST(int, wstringLength), &string[0], stringLength, NULL, NULL);
      #elif (M_OS & M_OS_QNXNTO) || (M_OS & M_OS_BSD)
         std::locale currentLocale("");
         typedef std::string::traits_type::state_type State;
         typedef std::codecvt<wchar_t, char, State> CodeCVT;
         const CodeCVT& cc = std::use_facet<CodeCVT>(currentLocale);
         State state = State();

         const wchar_t* fromBeg = wstring;
         const wchar_t* fromEnd = wstring + wstringLength;
         const wchar_t* fromNxt = NULL;

         string.reserve(wstringLength);

         char tmpBuf[256];
         char* toBeg = tmpBuf;
         char* toEnd = toBeg + 256;
         char* toNxt = NULL;

         for ( ;; )
         {
            CodeCVT::result res = cc.out(state, fromBeg, fromEnd, fromNxt, toBeg, toEnd, toNxt);
            if ( res == CodeCVT::error )
            {
               string.append(toBeg, toNxt);
               string += '?';
               fromBeg = fromNxt + 1;

               state = State();
            }
            else if ( res == CodeCVT::partial )
            {
               string.append(toBeg, toNxt);
               fromBeg = fromNxt;
            }
            else if ( res == CodeCVT::ok )
            {
               string.append(toBeg, toNxt);
               break;
            }
         }
      #else
         size_t converted = 0;
         size_t needSize = 0;

         while ( converted <= wstringLength )
         {
            mbstate_t ps = {0};
            const wchar_t* source = wstring + converted;
            const size_t sourceLength = Mstrnlen(source, wstringLength - converted);
            const size_t n = wcsnrtombs(0, &source, sourceLength, 0, &ps);
            converted += sourceLength + 1;
            needSize += ( n == static_cast<size_t>(-1) ) ? 1 : n + 1;
         }
         string.resize(needSize - 1);
         const size_t fullSize = needSize;
         converted = needSize = 0;
         while ( converted <= wstringLength )
         {
            mbstate_t ps = {0};
            const wchar_t* source = wstring + converted;
            const size_t sourceLength = Mstrnlen(source, wstringLength - converted);
            const size_t n = wcsnrtombs(&string[needSize], &source, sourceLength, fullSize - needSize, &ps);
            converted += sourceLength + 1;
            needSize += ( n == static_cast<size_t>(-1) ) ? 1 : n + 1;
         }
      #endif
   }
   return string;
}

M_FUNC MStdString MToStdString(const wchar_t* str)
{
   MStdString result;
   if ( str != NULL )
      result = MToStdString(str, wcslen(str));
   return result;
}

M_FUNC MStdString MToStdString(const MWideString& str)
{
   return MToStdString(str.data(), str.size());
}

M_FUNC MWideString MToWideString(const MStdString& string)
{
   return MToWideString(string.c_str(), string.length());
}

#endif // !M_NO_WCHAR_T

M_FUNC MStdString MToStdString(const char* str)
{
   MStdString result;
   if ( str != NULL )
      result = str;
   return result;
}

M_FUNC MStdString MToStdString(const char* str, size_t len)
{
   return MStdString(str, len);
}

// Determining the run-time platform and the OS version
M_FUNC unsigned MGetRuntimeOSMask()
{
   return s_runtimeOSMask;
}

#if M_DEBUG && defined(__BCPLUSPLUS__) // Debugging under C++Builder
/// Alert the user if bExpr is false;
/// HALTS the app if the user decides the failure should abort.
/// \return FALSE if bExpr is true or the user chooses to ignore the failure.
/// \return TRUE if debug mode should be invoked & the expression re-evaluated.
M_FUNC bool AssertMaybeRetry(bool bExpr, const char* sExpr, const char* sFile, int nLine, const char* sFunc)
{
   if ( bExpr )
      return false;  //no retry or abort is needed

   //Prepare message-box notifications of the failure
   const char  caption[] = "Assertion Failed";

   char  msg[ 1024 ];
   ::wsprintf( msg, "%s\n\nFile %s\nLine %d\nFunction %s\n"
                     "\nFor Borland C++ Builder debugging ..."
                     "\n+ click 'Retry' to bring up the CPU-View"
                     "\n+ press <F7> once"
                     "\n+ press <Ctrl>-V once", sExpr, sFile, nLine, sFunc );

   switch ( ::MessageBox( NULL, msg, caption,
               MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION | MB_TASKMODAL ) )
   {  //Handle their decision
      case IDABORT:
         //explicitly expand assert(e) using our passed-in parameters
         _assert( const_cast<char *>(sExpr), const_cast<char *>(sFile), nLine );
      case IDRETRY:
         return true;   // break then reevaluate the expression
   }
   return false;  //no retry is desired
}
#endif

M_FUNC void MAddDirectorySeparatorIfNecessary(MStdString& directory)
{
   MStdString::size_type directorySize = directory.size();
   if ( directorySize > 0 && directory[directorySize - 1] != '/' && directory[directorySize - 1] != '\\' && directory[directorySize - 1] != ':' )
      directory += M_DIRECTORY_SEPARATOR;
}

#if __cplusplus > 201100L

M_FUNC bool MPointerIsNull(const void* ptr)
{
   return ptr == nullptr; // hide implementation from compiler inliner
}

#endif

