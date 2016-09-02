// File MCORE/private/aes_impl_openssl.cxx

#include <openssl/opensslv.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include "MCriticalSection.h"

namespace
{

   // Singleton string loader and unloader
   //
   class PrivateErrorStringsLoader : protected MCriticalSection
   {
   public: // Constructor and destructor

      PrivateErrorStringsLoader()
      {
         // this is to prevent a blunder when many threads
         // attempt to show the first error all at the same time
         MCriticalSection::Locker self(*this);
         if ( !m_loaded )
         {
            ERR_load_crypto_strings();
            m_loaded = true;
         }
      }

      ~PrivateErrorStringsLoader()
      {
         MCriticalSection::Locker self(*this);
         ERR_free_strings();
         m_loaded = false;
      }

   private: // data:

      static bool m_loaded;
   };

   bool PrivateErrorStringsLoader::m_loaded = false;

   static void DoThrowOpenSSLError()
   {
      static PrivateErrorStringsLoader s_stringsLoader; // singleton, initializes at first call

      char buf [ 256 ]; // according to man page this has to be no less than 120 bytes. Allocate somewhat more
      unsigned long e = ERR_get_error();
      ERR_error_string_n(e, buf, sizeof(buf));

      // Error code is part of error string, and the error returned by ERR_error_string_n() looks like this:
      //    "error:[error code]:[library name]:[function name]:[reason string]"
      // Example:
      //    "error:0607B083:digital envelope routines:EVP_CipherInit_ex:no cipher set"
      // Therefore, the result error will be:
      //    "OpenSSL error:0607B083:digital envelope routines:EVP_CipherInit_ex:no cipher set"
      //
      MException::Throw(MException::ErrorSecurity, M_ERR_OPENSSL_ERROR, "OpenSSL %s", buf);
      M_ENSURED_ASSERT(0);
   }

}

static void DoConstructAesContext(MAesPrivateContext& context)
{
   context.m_encryptCtx = NULL;
   context.m_decryptCtx = NULL;
}

static void DoDestructAesContext(MAesPrivateContext& context)
{
   if ( context.m_encryptCtx != NULL )
   {
      M_ASSERT(context.m_decryptCtx != NULL); // always

      EVP_CIPHER_CTX *c = (EVP_CIPHER_CTX *)context.m_encryptCtx;
      Muint8 buf[16];
      int len = sizeof(buf);
      bool ok = EVP_DecryptFinal_ex(c, buf, &len) == 1;
      M_ASSERT(ok && len == 0); // do not throw here, continue. Not destructing context can only lead to a minor memory leak
      M_USED_VARIABLE(ok);
      EVP_CIPHER_CTX_cleanup(c);
      delete c;

      c = (EVP_CIPHER_CTX *)context.m_decryptCtx;
      len = sizeof(buf);
      ok = EVP_EncryptFinal_ex(c, buf, &len) == 1;
      M_ASSERT(ok && len == 0); // do not throw here, continue. Not destructing context can only lead to a minor memory leak
      M_USED_VARIABLE(ok);

      EVP_CIPHER_CTX_cleanup(c);
      delete c;

      context.m_encryptCtx = NULL; // if there were errors have to live with a memory leak
      context.m_decryptCtx = NULL;

   }
}

void MAes::DoCheckAndPrepareContext()
{
   if ( m_context.m_encryptCtx == NULL )
   {
      M_ASSERT(m_context.m_decryptCtx == NULL); // all go together if there was no error

      CheckKeySizeValid(m_key);
      M_ASSERT(m_key.size() == 32 || m_key.size() == 16); // the two supported by the below code

      MUniquePtr<EVP_CIPHER_CTX> encryptCtx(M_NEW EVP_CIPHER_CTX);
      MUniquePtr<EVP_CIPHER_CTX> decryptCtx(M_NEW EVP_CIPHER_CTX);
      EVP_CIPHER_CTX_init(encryptCtx.get());
      EVP_CIPHER_CTX_init(decryptCtx.get());

      const EVP_CIPHER* type = (m_key.size() == 32) ? EVP_aes_256_ecb() : EVP_aes_128_ecb();
      M_ASSERT(type != NULL);
      if ( EVP_EncryptInit_ex(encryptCtx.get(), type, NULL, reinterpret_cast<const Muint8*>(m_key.data()), NULL) != 1 )
      {
         DoThrowOpenSSLError();
         M_ENSURED_ASSERT(0);
      }
      if ( EVP_DecryptInit_ex(decryptCtx.get(), type, NULL, reinterpret_cast<const Muint8*>(m_key.data()), NULL) != 1 )
      {
         DoThrowOpenSSLError();
         M_ENSURED_ASSERT(0);
      }
      EVP_CIPHER_CTX_set_padding(encryptCtx.get(), 0);
      EVP_CIPHER_CTX_set_padding(decryptCtx.get(), 0);
      m_context.m_encryptCtx = encryptCtx.release();
      m_context.m_decryptCtx = decryptCtx.release();
   }
}

void MAes::EncryptBuffer(const Muint8* plainText, Muint8* cipherText)
{
   DoCheckAndPrepareContext();
   EVP_CIPHER_CTX *c = (EVP_CIPHER_CTX *)m_context.m_encryptCtx;
   int clen = BlockSize;
   if ( EVP_EncryptUpdate(c, cipherText, &clen, plainText, BlockSize) != 1 )
   {
      DoThrowOpenSSLError();
      M_ASSERT(0);
   }
}

void MAes::DecryptBuffer(const Muint8* cipherText, Muint8* plainText)
{
   DoCheckAndPrepareContext();
   EVP_CIPHER_CTX *c = (EVP_CIPHER_CTX *)m_context.m_decryptCtx;
   int clen = BlockSize;
   if ( EVP_DecryptUpdate(c, plainText, &clen, cipherText, BlockSize) != 1 )
   {
      DoThrowOpenSSLError();
      M_ASSERT(0);
   }
}

unsigned MAes::KeyWrapBuffer(const Muint8* keyText, unsigned keyTextSize, Muint8* cipherText)
{
   CheckKeySizeValid(m_key);

   AES_KEY actx;
   if ( AES_set_encrypt_key(reinterpret_cast<const Muint8*>(m_key.data()), static_cast<int>(m_key.size() << 3), &actx) )
   {
      DoThrowOpenSSLError();
      M_ASSERT(0);
   }
   int res = AES_wrap_key(&actx, NULL, cipherText, keyText, keyTextSize);
   OPENSSL_cleanse(&actx, sizeof(actx));
   if ( res < 0 )
   {
      DoThrowOpenSSLError();
      M_ASSERT(0);
   }
   M_ASSERT(res == static_cast<int>(keyTextSize + KeyWrapEncryptionExtraSize));
   return res;
}

unsigned MAes::KeyUnwrapBuffer(const Muint8* cipherText, unsigned cipherTextSize, Muint8* keyText)
{
   CheckKeySizeValid(m_key);

   AES_KEY actx;
   if ( AES_set_decrypt_key(reinterpret_cast<const Muint8*>(m_key.data()), static_cast<int>(m_key.size() << 3), &actx) )
   {
      DoThrowOpenSSLError();
      M_ASSERT(0);
   }
   int res = AES_unwrap_key(&actx, NULL, keyText, cipherText, cipherTextSize);
   OPENSSL_cleanse(&actx, sizeof(actx));
   if ( res < 0 )
   {
      ThrowValidationError();
      M_ENSURED_ASSERT(0);
   }
   return cipherTextSize - KeyWrapEncryptionExtraSize;
}
