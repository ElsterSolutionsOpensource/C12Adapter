// File MCORE/private/aes_impl_cryptoapi.cxx

#ifndef MS_ENH_RSA_AES_PROV
   #define MS_ENH_RSA_AES_PROV "Microsoft Enhanced RSA and AES Cryptographic Provider (Prototype)"
#endif

static void DoConstructAesContext(MAesPrivateContext& context)
{
   context.m_provider = NULL;
   context.m_cryptKey = NULL;
}

static void DoDestructAesContext(MAesPrivateContext& context)
{
   if ( context.m_provider != NULL )
   {
      if ( context.m_cryptKey != NULL ) // crypt key can be zero when constructing the context leads to an exception
      {
         CryptDestroyKey(context.m_cryptKey);
         context.m_cryptKey = NULL;
      }
      CryptReleaseContext(context.m_provider, 0);
      context.m_provider = NULL; // if there were errors have to live with a memory leak
   }
}

void MAes::DoCheckAndPrepareContext()
{
   if ( m_context.m_provider == NULL )
   {
      M_ASSERT(m_context.m_cryptKey == NULL); // this will never be nonzero if provider is zero

      CheckKeySizeValid(m_key);
      M_ASSERT(m_key.size() == MAes::KeySize); // other sizes are not supported currently

      if ( !CryptAcquireContext(&m_context.m_provider, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_VERIFYCONTEXT) )
      {
         MESystemError::ThrowLastSystemError();
         M_ENSURED_ASSERT(0);
      }

      struct
      {
         BLOBHEADER m_header;
         DWORD m_size;
         BYTE m_key[16];
      } keyData;
      keyData.m_header.bType = PLAINTEXTKEYBLOB;
      keyData.m_header.bVersion = CUR_BLOB_VERSION;
      keyData.m_header.reserved = 0;
      keyData.m_header.aiKeyAlg = CALG_AES_128;
      keyData.m_size = MAes::KeySize;
      memcpy(keyData.m_key, m_key.data(), MAes::KeySize);
      if ( !CryptImportKey(m_context.m_provider, (BYTE*)&keyData, sizeof(keyData), 0, 0, &m_context.m_cryptKey) )
      {
         unsigned err = MESystemError::GetLastGlobalSystemError();
         DoDestructAesContext(m_context);
         MESystemError::Throw(err);
         M_ENSURED_ASSERT(0);
      }

      DWORD mode = CRYPT_MODE_ECB;
      if ( !CryptSetKeyParam(m_context.m_cryptKey, KP_MODE, (BYTE*)&mode, 0) )
      {
         unsigned err = MESystemError::GetLastGlobalSystemError();
         DoDestructAesContext(m_context); // perform full destruction
         MESystemError::Throw(err);
         M_ENSURED_ASSERT(0);
      }
   }
}

void MAes::EncryptBuffer(const Muint8* plainText, Muint8* cipherText)
{
   DoCheckAndPrepareContext();

   memcpy(cipherText, plainText, MAes::BlockSize);
   DWORD size = MAes::BlockSize;
   if ( !CryptEncrypt(m_context.m_cryptKey, 0, FALSE, 0, cipherText, &size, MAes::BlockSize) )
   {
      MESystemError::ThrowLastSystemError();
      M_ENSURED_ASSERT(0);
   }
}

void MAes::DecryptBuffer(const Muint8* cipherText, Muint8* plainText)
{
   DoCheckAndPrepareContext();

   memcpy(plainText, cipherText, MAes::BlockSize);
   DWORD size = MAes::BlockSize;
   if ( !CryptDecrypt(m_context.m_cryptKey, 0, FALSE, 0, plainText, &size) )
   {
      MESystemError::ThrowLastSystemError();
      M_ENSURED_ASSERT(0);
   }
}
