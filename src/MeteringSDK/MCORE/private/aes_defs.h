// File MCORE/private/aes_defs.h
#ifndef MCORE_PRIVATE_AES_DEFS_H
#define MCORE_PRIVATE_AES_DEFS_H
/// \cond SHOW_INTERNAL

#if defined(M_USE_CRYPTODEV) && M_USE_CRYPTODEV
   #include <crypto/cryptodev.h>
#elif defined(M_USE_CRYPTOAPI) && M_USE_CRYPTOAPI
   #include <wincrypt.h>
#endif

// Private class that represents AES encryption context
//
struct MAesPrivateContext
{
   #if defined(M_USE_CRYPTODEV) && M_USE_CRYPTODEV

      int               m_cfd;
      struct session_op m_sess;

   #elif defined(M_USE_CRYPTOAPI) && M_USE_CRYPTOAPI

      HCRYPTPROV m_provider;
      HCRYPTKEY  m_cryptKey;

   #elif defined(M_USE_OPENSSL) && M_USE_OPENSSL

      void* m_encryptCtx;
      void* m_decryptCtx;

   #else // Elster's private implementation

      enum
      {
         AES__NUM_ROUNDS = 10,
         AES__BUFFER_SIZE = 16
      };

      Muint8 m_keysched [ (AES__NUM_ROUNDS + 1) * AES__BUFFER_SIZE ];
      bool m_isInitialized;

   #endif
};

// Implementations need to expose the following methods.
//
// // Create and destroy the given context.
// // These are to be static functions, possibly inline
//
// void DoConstructAesContext(MAesPrivateContext& context);
// void DoDestructAesContext(MAesPrivateContext& context);
//
// // These are documented in class MAes:
//
// void MAes::DoCheckAndPrepareContext()
// void MAes::EncryptBuffer(const Muint8* plainText, Muint8* cipherText)
// void MAes::DecryptBuffer(const Muint8* cipherText, Muint8* plainText)
// unsigned MAes::KeyWrapBuffer(const Muint8* keyText, unsigned keyTextSize, Muint8* cipherText)
// unsigned MAes::KeyUnwrapBuffer(const Muint8* cipherText, unsigned cipherTextSize, Muint8* keyText)

/// \endcond
#endif
