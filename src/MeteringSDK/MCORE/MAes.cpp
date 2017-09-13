// File MCORE/MAes.cpp

#include "MCOREExtern.h"
#include "MAes.h"
#include "MUtilities.h"
#include "MException.h"

#if defined(M_USE_CRYPTODEV) && M_USE_CRYPTODEV
   #include "private/aes_impl_cryptodev.cxx"
   #include "private/aes_wrap.cxx"   // include generic implementation as /dev/crypto does not have one
   #include "private/aes_unwrap.cxx" // include generic implementation as /dev/crypto does not have one
#elif defined(M_USE_CRYPTOAPI) && M_USE_CRYPTOAPI
   #include "private/aes_impl_cryptoapi.cxx"
   #include "private/aes_wrap.cxx"   // include generic implementation as Microsoft CryptoAPI does not have one
   #include "private/aes_unwrap.cxx" // include generic implementation as Microsoft CryptoAPI does not have one
#elif defined(M_USE_OPENSSL) && M_USE_OPENSSL
   #include "private/aes_impl_openssl.cxx"
#else // Elster's private implementation
   #include "private/aes_impl_legacy.cxx"
#endif

#if !M_NO_REFLECTION

   static MAes* DoNew0()
   {
      return M_NEW MAes();
   }

   static MAes* DoNew1(const MVariant& keyOrCopy)
   {
      if ( keyOrCopy.IsObject() )
      {
         const MAes* aes = M_DYNAMIC_CAST_WITH_THROW(const MAes, keyOrCopy.DoInterpretAsObject());
         M_ASSERT(aes != NULL);
         return M_NEW MAes(*aes);
      }
      return M_NEW MAes(keyOrCopy.AsByteString());
   }

#endif

M_START_PROPERTIES(Aes)
   M_OBJECT_PROPERTY_BYTE_STRING    (Aes, Key,              ST_constMByteStringA_X, ST_X_constMByteStringA)
   M_OBJECT_PROPERTY_STRING         (Aes, HexKey,           ST_MStdString_X, ST_X_constMStdStringA)
M_START_METHODS(Aes)
   M_OBJECT_SERVICE                 (Aes, Encrypt,               ST_MByteString_X_constMByteStringA)
   M_OBJECT_SERVICE                 (Aes, Decrypt,               ST_MByteString_X_constMByteStringA)
   M_CLASS_SERVICE                  (Aes, StaticEncrypt,         ST_MByteString_S_constMByteStringA_constMByteStringA)
   M_CLASS_SERVICE                  (Aes, StaticDecrypt,         ST_MByteString_S_constMByteStringA_constMByteStringA)
   M_OBJECT_SERVICE                 (Aes, KeyWrap,               ST_MByteString_X_constMByteStringA)
   M_OBJECT_SERVICE                 (Aes, KeyUnwrap,             ST_MByteString_X_constMByteStringA)
   M_CLASS_SERVICE                  (Aes, StaticKeyWrap,         ST_MByteString_S_constMByteStringA_constMByteStringA)
   M_CLASS_SERVICE                  (Aes, StaticKeyUnwrap,       ST_MByteString_S_constMByteStringA_constMByteStringA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(Aes, New, DoNew0,        0, ST_MObjectP_S)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(Aes, New, DoNew1,        1, ST_MObjectP_S_constMVariantA)
   M_CLASS_SERVICE                  (Aes, CheckKeySizeValid,     ST_S_constMByteStringA)
   M_CLASS_SERVICE                  (Aes, CheckHexKeySizeValid,  ST_S_constMStdStringA)
M_END_CLASS(Aes, Object)

MAes::MAes()
:
   m_key() // empty, key not set
{
   DoConstructAesContext(m_context);
}

MAes::MAes(const MByteString& key)
:
   m_key()
{
   DoConstructAesContext(m_context);
   SetKey(key);
}

MAes::MAes(const MAes& other)
:
   m_key(other.m_key)
{
   DoConstructAesContext(m_context);
}

MAes::~MAes()
{
   if ( !m_key.empty() )
   {
      DoDestructAesContext(m_context);
      DestroySecureData(m_key);
   }
}

MAes& MAes::operator=(const MAes& other)
{
   if ( &other != this )
      SetKey(other.m_key);
   return *this;
}

void MAes::CheckKeySizeValid(const MByteString& key)
{
   if ( key.size() != MAes::KeySize )
   {
      MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_SIZE_OF_NUMBER_OUTSIDE_RANGE, "Key size is expected to be 16 bytes"));
      M_ENSURED_ASSERT(0);
   }
}

void MAes::CheckHexKeySizeValid(const MStdString& hexKey)
{
   CheckKeySizeValid(MUtilities::HexStringToBytes(hexKey));
}

void MAes::SetKey(const MByteString& key)
{
   CheckKeySizeValid(key);
   M_ASSERT(key.size() == KeySize);
   if ( m_key != key ) // Do it after the above checks.
   {
      DoDestructContext();
      AssignSecureData(m_key, key);
   }
}

MStdString MAes::GetHexKey() const
{
   return MUtilities::BytesToHex(m_key, false);
}

void MAes::SetHexKey(const MStdString& key)
{
   SetKey(MUtilities::HexStringToBytes(key));
}

   static void DoCheckDataIsDivisibleByBlockSize(const MByteString& data)
   {
      if ( data.size() % static_cast<unsigned>(MAes::BlockSize) != 0 ) // hopefully a bitwise operation
      {
         MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_SIZE_OF_NUMBER_OUTSIDE_RANGE, "Block size is expected to be divisible by 16 bytes"));
         M_ENSURED_ASSERT(0);
      }
   }

MByteString MAes::Encrypt(const MByteString& data)
{
   MByteString result;
   DoCheckDataIsDivisibleByBlockSize(data);
   DoCheckAndPrepareContext();
   result = data;
   char* it = &(result[0]); // yeah, we know that the C++ standard does not allow this...
   const char* itEnd = it + result.size();
   for ( ; it < itEnd; it += BlockSize )
      EncryptBuffer(it, it);
   return result;
}

MByteString MAes::Decrypt(const MByteString& data)
{
   MByteString result;
   DoCheckDataIsDivisibleByBlockSize(data);
   DoCheckAndPrepareContext();
   result = data;
   char* it = &(result[0]); // yeah, we know that the C++ standard does not allow this...
   const char* itEnd = it + result.size();
   for ( ; it < itEnd; it += BlockSize )
      DecryptBuffer(it, it);
   return result;
}

MByteString MAes::StaticEncrypt(const MByteString& key, const MByteString& data)
{
   MAes aes(key);
   return aes.Encrypt(data);
}

MByteString MAes::StaticDecrypt(const MByteString& key, const MByteString& data)
{
   MAes aes(key);
   return aes.Decrypt(data);
}

MByteString MAes::KeyWrap(const MByteString& keys)
{
   MByteString result;
   DoCheckKeyWrapArgumentSize(static_cast<unsigned>(keys.size()));
   MByteString::size_type resultSize = keys.size() + KeyWrapEncryptionExtraSize;
   result.resize(resultSize);
   unsigned size = KeyWrapBuffer(keys.data(), static_cast<unsigned>(keys.size()), &(result[0]));
   M_ASSERT(size == resultSize);
   M_USED_VARIABLE(size);
   return result;
}

MByteString MAes::KeyUnwrap(const MByteString& cipher)
{
   MByteString result;
   DoCheckKeyUnwrapArgumentSize(static_cast<unsigned>(cipher.size()));
   MByteString::size_type resultSize = cipher.size() - KeyWrapEncryptionExtraSize;
   result.resize(resultSize);
   unsigned size = KeyUnwrapBuffer(cipher.data(), static_cast<unsigned>(cipher.size()), &(result[0]));
   M_ASSERT(size == resultSize);
   M_USED_VARIABLE(size);
   return result;
}

MByteString MAes::StaticKeyWrap(const MByteString& key, const MByteString& keys)
{
   MAes aes(key);
   return aes.KeyWrap(keys);
}

MByteString MAes::StaticKeyUnwrap(const MByteString& key, const MByteString& cipher)
{
   MAes aes(key);
   return aes.KeyUnwrap(cipher);
}

void MAes::DoKeyWrapUnwrapRangeCheck(int minimum, int maximum, unsigned size)
{
   if ( size < static_cast<unsigned>(minimum) || size > static_cast<unsigned>(maximum) || (size % 8) != 0 )
   {
      MException::Throw(MException::ErrorSoftware, M_CODE_STR_P2(M_ERR_SIZE_OF_NUMBER_OUTSIDE_RANGE, "Argument size of this key wrap method should be in range %d .. %d, divisible by 8", minimum, maximum));
      M_ENSURED_ASSERT(0);
   }
}

M_NORETURN_FUNC void MAes::ThrowValidationError()
{
   MException::Throw(MException::ErrorSecurity, M_CODE_STR(MErrorEnum::DataNotValidated, M_I("Data not validated, tampering possible")));
   M_ENSURED_ASSERT(0);
}

void MAes::DestroySecureData(MByteString& data)
{
   data.assign(data.size(), '\0');
}

void MAes::DestroySecureData(MByteStringVector& data)
{
   MByteStringVector::iterator it = data.begin();
   MByteStringVector::iterator itEnd = data.end();
   for ( ; it != itEnd; ++it )
      DestroySecureData(*it);
}

   template
       <class T>
   inline void DoSecureAssign(T& v1, const T& v2)
   {
      // Temporary variable provides constant time operation
      // dependless on whether v1 and v2 are the same object or not
      T tmp1 = v2;
      if ( &v1 != &v2 ) // real assignment, simply destroy the temporary
      {
         MAes::DestroySecureData(tmp1);
         v1 = v2;
      }
      else // assignment to self, use temporary variable
      {
         MAes::DestroySecureData(v1);
         v1 = tmp1;
         MAes::DestroySecureData(tmp1);
      }
   }

void MAes::AssignSecureData(MByteString& destination, const MByteString& source)
{
   DoSecureAssign(destination, source);
}

void MAes::AssignSecureData(MByteStringVector& destination, const MByteStringVector& source)
{
   DoSecureAssign(destination, source);
}

   template
       <class T>
   inline void DoSecureMove(T& v1, T& v2)
   {
      // Temporary variable provides constant time operation
      // dependless on whether v1 and v2 are the same object or not
      T tmp1 = v2;
      MAes::DestroySecureData(v1);
      MAes::DestroySecureData(v2);
      v1 = tmp1;
      MAes::DestroySecureData(tmp1);
   }

void MAes::MoveSecureData(MByteString& destination, MByteString& source)
{
   DoSecureMove(destination, source);
}

void MAes::MoveSecureData(MByteStringVector& destination, MByteStringVector& source)
{
   DoSecureMove(destination, source);
}

   template
       <class T>
    inline void DoSecureSwap(T& v1, T& v2)
    {
       // as a safety precaution do not use std::swap
       T tmp1 = v1;
       T tmp2 = v2;
       MAes::DestroySecureData(v1);
       MAes::DestroySecureData(v2);
       v2 = tmp1;
       v1 = tmp2;
       MAes::DestroySecureData(tmp1);
       MAes::DestroySecureData(tmp2);
    }

void MAes::SwapSecureData(MByteString& v1, MByteString& v2)
{
   DoSecureSwap(v1, v2);
}

void MAes::SwapSecureData(MByteStringVector& v1, MByteStringVector& v2)
{
   DoSecureSwap(v1, v2);
}

void MAes::DoDestructContext()
{
   DoDestructAesContext(m_context);
}
