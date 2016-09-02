#include "MCOREExtern.h"
#include "MException.h"
#include "MRandomGenerator.h"
#include "MException.h"

   #if !M_NO_REFLECTION
      static MRandomGenerator* DoNew0()
      {
         return M_NEW MRandomGenerator();
      }
   #endif

M_START_PROPERTIES(RandomGenerator)
M_START_METHODS(RandomGenerator)
   M_OBJECT_SERVICE      (RandomGenerator, Generate,       ST_MByteString_X_unsigned)
   M_CLASS_FRIEND_SERVICE(RandomGenerator, New, DoNew0,    ST_MObjectP_S)
   M_CLASS_SERVICE       (RandomGenerator, StaticGenerate, ST_MByteString_S_unsigned)
M_END_CLASS(RandomGenerator, Object)

MRandomGenerator::~MRandomGenerator() M_NO_THROW
{
   #if (M_OS & M_OS_WINDOWS)
      if ( m_crypt != NULL )
      {
         const BOOL status = CryptReleaseContext(m_crypt, 0);
         M_ASSERT(status != FALSE);
         M_USED_VARIABLE(status);
      }
   #else
      if ( m_fd >= 0 )
      {
         const int status = close(m_fd);
         M_ASSERT(status >= 0); // but do not throw an error
         M_USED_VARIABLE(status);
      }
   #endif
}

MByteString MRandomGenerator::Generate(unsigned size)
{
   MByteString response;
#ifdef M_USE_USTL
   response.append((size_t)size, '\0');
#else
   response.assign((size_t)size, '\0');
#endif
   char* data = &response[0];
   GenerateBuffer(data, size);
   return response;
}

void MRandomGenerator::GenerateBuffer(char* buff, unsigned size)
{
   MENumberOutOfRange::CheckNamedUnsignedRange(1, INT_MAX, size, "RandomBufferSize");
#if (M_OS & M_OS_WINDOWS)
   if ( m_crypt == NULL )
   {
      BOOL status = CryptAcquireContext(&m_crypt, NULL, NULL, PROV_RSA_FULL, 0);
      if ( status == FALSE )
      {
         if ( GetLastError() == (DWORD)NTE_BAD_KEYSET )
            status = CryptAcquireContext(&m_crypt, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET);
         MESystemError::CheckLastSystemError(status == FALSE);
      }
      M_ASSERT(m_crypt != NULL);
   }
   const BOOL status = CryptGenRandom(m_crypt, size, reinterpret_cast<BYTE*>(buff));
   MESystemError::CheckLastSystemError(status == FALSE);
#else
   if ( m_fd < 0 )
   {
      m_fd = open("/dev/urandom", O_RDONLY);
      MESystemError::CheckLastSystemError(m_fd < 0);
   }
   const ssize_t result = read(m_fd, buff, size);
   MESystemError::CheckLastSystemError(result < 0);
   if ( result != static_cast<ssize_t>(size) )
   {  // urandom will always return the requested number of bytes as it is nonblocking, but have a check just for the case
      MException::Throw(MException::ErrorSecurity, M_CODE_STR(MErrorEnum::NumberOutOfRange, "Random number generator failed to return the requested number of bytes"));
      M_ENSURED_ASSERT(0);
   }
#endif
}

MByteString MRandomGenerator::StaticGenerate(unsigned size)
{
   MByteString response;
#ifdef M_USE_USTL
   response.append((size_t)size, '\0');
#else
   response.assign((size_t)size, '\0');
#endif
   char* data = &response[0];
   StaticGenerateBuffer(data, size);
   return response;
}

void MRandomGenerator::StaticGenerateBuffer(char* buff, unsigned size)
{
   MRandomGenerator generator;
   generator.GenerateBuffer(buff, size);
}
