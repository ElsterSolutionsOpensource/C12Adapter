// File MCORE/MDes.cpp

#include "MCOREExtern.h"
#include "MDes.h"
#include "MUtilities.h"
#include "MException.h"

// DES implementation source code is included here, all local definitions
#include "private/des.cxx"

M_START_PROPERTIES(Des)
M_START_METHODS(Des)
   M_CLASS_SERVICE  (Des, StaticEncrypt, ST_MByteString_S_constMByteStringA_constMByteStringA)
   M_CLASS_SERVICE  (Des, StaticDecrypt, ST_MByteString_S_constMByteStringA_constMByteStringA)
M_END_CLASS(Des, Object)


    static void DoCheckKeyOrDataSize(const MByteString& bytes)
    {
        if ( bytes.size() != 8 )
        {
           MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_SIZE_OF_NUMBER_OUTSIDE_RANGE, "Size is expected to be 8 bytes"));
           M_ENSURED_ASSERT(0);
        }
    }

MByteString MDes::StaticEncrypt(const MByteString& key, const MByteString& data)
{
   MByteString result;
   DoCheckKeyOrDataSize(key);
   DoCheckKeyOrDataSize(data);
   result.resize(8);
   symmetric_key des_key;
   des_setup((const unsigned char*)key.data(), 8, 0, &des_key);
   des_ecb_encrypt((const unsigned char*)data.data(), (unsigned char*)(&result[0]), &des_key);
   des_done(&des_key);
   return result;
}

MByteString MDes::StaticDecrypt(const MByteString& key, const MByteString& data)
{
   MByteString result;
   DoCheckKeyOrDataSize(key);
   DoCheckKeyOrDataSize(data);
   result.resize(8);
   symmetric_key des_key;
   des_setup((const unsigned char*)key.data(), 8, 0, &des_key);
   des_ecb_decrypt((const unsigned char*)data.data(), (unsigned char*)(&result[0]), &des_key);
   des_done(&des_key);
   return result;
}

void MDes::StaticEncryptBuffer(const char* key, const char* plainText, char* cipherText, unsigned size)
{
   M_ASSERT(key != NULL);
   M_ASSERT(plainText != NULL);
   M_ASSERT(cipherText != NULL);
   M_ASSERT((size % 8) == 0); // size can be zero, ok

   symmetric_key des_key;
   des_setup((const unsigned char*)key, 8, 0, &des_key);
   const char* lastPlainText = plainText + size;
   for ( ; plainText < lastPlainText; cipherText += 8, plainText += 8 )
      des_ecb_encrypt((const unsigned char*)plainText, (unsigned char*)cipherText, &des_key);
   des_done(&des_key);
}

void MDes::StaticDecryptBuffer(const char* key, const char* cipherText, char* plainText, unsigned size)
{
   M_ASSERT(key != NULL);
   M_ASSERT(plainText != NULL);
   M_ASSERT(cipherText != NULL);
   M_ASSERT((size % 8) == 0); // size can be zero, ok

   symmetric_key des_key;
   des_setup((const unsigned char*)key, 8, 0, &des_key);
   const char* lastCypherText = cipherText + size;
   for ( ; cipherText < lastCypherText; cipherText += 8, plainText += 8 )
      des_ecb_decrypt((const unsigned char*)cipherText, (unsigned char*)plainText, &des_key);
   des_done(&des_key);
}
