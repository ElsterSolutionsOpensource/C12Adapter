// File MCORE/MAesEax.h

// The following code is based on:
/******************************************************************************/
/*  Title:            EAX' Encryption/Decryption/Authentication               */
/*  Project:          C12.22 Node Library                                     */
/*  Module Name:      Eax.c                                                   */
/*  File Created:     January 26, 2009                                        */
/*  By:               Edward J. Beroset                                       */
/******************************************************************************/

#include "MCOREExtern.h"
#include "MAesEax.h"
#include "MUtilities.h"
#include "MException.h"

inline void CopyBlock(Muint8 out[MAes::KeySize], const Muint8 in[MAes::KeySize])
{
   memcpy(out, in, MAes::KeySize);
}

inline void AddRoundKey(Muint8 state[MAes::KeySize], const Muint8* w)
{
   for (int i = MAes::KeySize; i; --i) // hope the compiler will unwind the loop and merge the words
      *state++ ^= *w++;
}

/* these are defined as macros so they'll be easy to redo in assembly if desired */
#define BLK_CPY(dst, src) CopyBlock((dst), (src))
#define BLK_XOR(dst, src) AddRoundKey((dst), (src))

//  Only define BADCODE if you want to match the erroneous code
//  printed in the last draft of C12.22.  As of 30 January 2009,
//  the committee is working on an errata document to correct the
//  code in the standard as well as the examples that were derived
//  from it.
//
#if 0
   #define BADCODE
#endif

#if !M_NO_REFLECTION

   static MAesEax* DoNew0()
   {
      return M_NEW MAesEax();
   }

   static MAesEax* DoNew1(const MVariant& keyOrCopy)
   {
      if ( keyOrCopy.IsObject() )
      {
         const MAesEax* eax = M_DYNAMIC_CAST_WITH_THROW(const MAesEax, keyOrCopy.DoInterpretAsObject());
         M_ASSERT(eax != NULL);
         return M_NEW MAesEax(*eax);
      }
      return M_NEW MAesEax(keyOrCopy.AsByteString());
   }

#endif

M_START_PROPERTIES(AesEax)
M_START_METHODS(AesEax)
   M_OBJECT_SERVICE                 (AesEax, EaxEncrypt,            ST_MByteString_X_constMByteStringA_constMByteStringA)
   M_OBJECT_SERVICE                 (AesEax, EaxDecrypt,            ST_MByteString_X_constMByteStringA_constMByteStringA)
   M_OBJECT_SERVICE                 (AesEax, EaxAuthenticate,       ST_unsigned_X_constMByteStringA)
   M_CLASS_SERVICE                  (AesEax, StaticEaxEncrypt,      ST_MByteString_S_constMByteStringA_constMByteStringA_constMByteStringA)
   M_CLASS_SERVICE                  (AesEax, StaticEaxDecrypt,      ST_MByteString_S_constMByteStringA_constMByteStringA_constMByteStringA)
   M_CLASS_SERVICE                  (AesEax, StaticEaxAuthenticate, ST_unsigned_S_constMByteStringA_constMByteStringA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(AesEax, New, DoNew0,        0, ST_MObjectP_S)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(AesEax, New, DoNew1,        1, ST_MObjectP_S_constMVariantA)
M_END_CLASS(AesEax, Aes)

MAesEax::MAesEax()
:
   MAes(),
   m_eaxContext(),
   m_contextUpdatedForEax(false)
{
}

MAesEax::MAesEax(const MByteString& key)
:
   MAes(),
   m_eaxContext(),
   m_contextUpdatedForEax(false)
{
   SetKey(key);
}

MAesEax::MAesEax(const MAesEax& other)
:
   MAes(other),
   m_eaxContext(other.m_eaxContext),
   m_contextUpdatedForEax(other.m_contextUpdatedForEax)
{
}

MAesEax::~MAesEax()
{
}

MAesEax& MAesEax::operator=(const MAesEax& other)
{
   if ( &other != this )
   {
      MAes::operator=(other);
      m_eaxContext = other.m_eaxContext;
      m_contextUpdatedForEax = other.m_contextUpdatedForEax;
   }
   return *this;
}

   // set up D or Q from L
   static void DoDbl(Muint8* out, const Muint8* in)
   {
      Muint8 carry = 0;

      // this might be a lot more efficient in assembly language
      for ( int i = 0; i < MAes::KeySize; ++i )
      {
         out[i] = ( in[i] << 1 ) | carry;
         carry = (in[i] & 0x80) ? 1 : 0;
      }
      if ( carry )
         out[0] ^= 0x87;
   }

void MAesEax::DoDestructContext()
{
   MAes::DoDestructContext();
   if ( m_contextUpdatedForEax )
   {
      m_eaxContext.clear();
      m_contextUpdatedForEax = false; // this is to prevent infinite recursion at DoCheckAndPrepareContext
   }
}

void MAesEax::DoCheckAndPrepareContext()
{
   MAes::DoCheckAndPrepareContext();
   if ( !m_contextUpdatedForEax )
   {
      m_contextUpdatedForEax = true; // this is to prevent infinite recursion at DoCheckAndPrepareContext

      memset(m_eaxContext.L, 0, sizeof(m_eaxContext.L));
      EncryptBuffer(m_eaxContext.L, m_eaxContext.L);
      DoDbl(m_eaxContext.D, m_eaxContext.L);
      DoDbl(m_eaxContext.Q, m_eaxContext.D);
   }
}

unsigned MAesEax::EaxEncryptBuffer(const char* clearText, unsigned clearTextSize, char* data, unsigned dataSize)
{
   DoCheckAndPrepareContext();

   Muint8 wsn[MAes::KeySize];
   Muint8 wsc[MAes::KeySize];

   // first copy the nonce into our working space
   BLK_CPY(wsn, m_eaxContext.D);
   DoCMAC(wsn, (const Muint8*)clearText, clearTextSize);

   if ( dataSize == 0 )
      return MToAlignedUINT32(wsn + (MAes::KeySize - sizeof(Muint32)));
   DoCTR(wsn, (Muint8*)data, dataSize);
   // first copy the nonce into our working space */
   BLK_CPY(wsc, m_eaxContext.Q);
   DoCMAC(wsc, (Muint8*)data, dataSize);
   BLK_XOR(wsc, wsn);

    Muint32 result = MToAlignedUINT32(wsc + (MAes::KeySize - sizeof(Muint32)));
#ifdef BADCODE
    result = MToBigEndianUINT32(result);
#endif
    return result;
}

unsigned MAesEax::EaxDecryptBuffer(const char* clearText, unsigned clearTextSize, char* data, unsigned dataSize)
{
   DoCheckAndPrepareContext();

   Muint8 wsn[MAes::KeySize];
   Muint8 wsc[MAes::KeySize];
   Muint32 mac;

   // first copy the nonce into our working space
   BLK_CPY(wsn, m_eaxContext.D);
   DoCMAC(wsn, (const Muint8*)clearText, clearTextSize);

   if ( dataSize == 0 )
      mac = MToAlignedUINT32(wsn + (MAes::KeySize - sizeof(Muint32)));
   else
   {
      /* first copy the nonce into our working space */
      BLK_CPY(wsc, m_eaxContext.Q);
      DoCMAC(wsc, (Muint8*)data, dataSize);
      BLK_XOR(wsc, wsn);
      mac = MToAlignedUINT32(wsc + (MAes::KeySize - sizeof(Muint32)));
      DoCTR(wsn, (Muint8*)data, dataSize);
   }
#ifdef BADCODE
   mac = MToBigEndianUINT32(mac);
#endif
   return mac;
}

unsigned MAesEax::EaxAuthenticateBuffer(const char* clearText, unsigned clearTextSize)
{
   Muint8 wsn[MAes::KeySize];

   DoCheckAndPrepareContext();

   /* first copy the nonce into our working space */
   BLK_CPY(wsn, m_eaxContext.D);
   DoCMAC(wsn, (const Muint8*)clearText, clearTextSize);
   Muint32 result = MToAlignedUINT32(wsn + (MAes::KeySize - sizeof(Muint32)));
#ifdef BADCODE
   result = MToBigEndianUINT32(result);
#endif
   return result;
}

MByteString MAesEax::EaxEncrypt(const MByteString& clearText, const MByteString& data)
{
   MByteString result;
   result.reserve(data.size() + sizeof(Muint32));
   result = data;
#ifdef M_USE_USTL
   Muint32 eax = EaxEncryptBuffer(const_cast<MByteString&>(clearText).data(), M_64_CAST(unsigned, clearText.size()), (result.empty() ? NULL : &result[0]), M_64_CAST(unsigned, data.size()));
#else
   Muint32 eax = EaxEncryptBuffer(clearText.data(), M_64_CAST(unsigned, clearText.size()), (result.empty() ? NULL : &result[0]), M_64_CAST(unsigned, data.size()));
#endif
   result.append((const char*)&eax, sizeof(Muint32));
   return result;
}

bool MAesEax::EaxDecryptToResult(const MByteString& clearText, const MByteString& data, MByteString& result)
{
   if ( data.size() < sizeof(Muint32) )
   {
      MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_SIZE_OF_NUMBER_OUTSIDE_RANGE, "Data block is expected to be no less than four bytes"));
      M_ENSURED_ASSERT(0);
   }
   Muint32 dataEax;
#ifdef M_USE_USTL
   memcpy(&dataEax, const_cast<MByteString&>(data).data() + data.size() - sizeof(Muint32), sizeof(Muint32));
#else
   memcpy(&dataEax, data.data() + data.size() - sizeof(Muint32), sizeof(Muint32));
#endif
   result.assign(data.begin(), data.begin() + data.size() - sizeof(Muint32));
#ifdef M_USE_USTL
   Muint32 eax = EaxDecryptBuffer(const_cast<MByteString&>(clearText).data(), M_64_CAST(unsigned, clearText.size()), result.empty() ? NULL : &result[0], M_64_CAST(unsigned, result.size()));
#else
   Muint32 eax = EaxDecryptBuffer(clearText.data(), M_64_CAST(unsigned, clearText.size()), result.empty() ? NULL : &result[0], M_64_CAST(unsigned, result.size()));
#endif
   return eax == dataEax;
}

unsigned MAesEax::EaxAuthenticate(const MByteString& clearText)
{
#ifdef M_USE_USTL
   return EaxAuthenticateBuffer(const_cast<MByteString&>(clearText).data(), M_64_CAST(unsigned, clearText.size()));
#else
   return EaxAuthenticateBuffer(clearText.data(), M_64_CAST(unsigned, clearText.size()));
#endif
}

MByteString MAesEax::EaxDecrypt(const MByteString& clearText, const MByteString& data)
{
   MByteString result;
   if ( !EaxDecryptToResult(clearText, data, result) )
   {
      ThrowValidationError();
      M_ENSURED_ASSERT(0);
   }
   return result;
}

MByteString MAesEax::StaticEaxEncrypt(const MByteString& key, const MByteString& clearText, const MByteString& data)
{
   MAesEax aesEax(key);
   return aesEax.EaxEncrypt(clearText, data);
}

MByteString MAesEax::StaticEaxDecrypt(const MByteString& key, const MByteString& clearText, const MByteString& data)
{
   MAesEax aesEax(key);
   return aesEax.EaxDecrypt(clearText, data);
}

unsigned MAesEax::StaticEaxAuthenticate(const MByteString& key, const MByteString& clearText)
{
   MAesEax aesEax(key);
   return aesEax.EaxAuthenticate(clearText);
}

void MAesEax::DoCTR(const Muint8* ws, Muint8* pn, unsigned sizeN)
{
   Muint8 ctr [ MAes::KeySize ];

   // clear two bits to avoid inter-word carries
   BLK_CPY(ctr, ws);

#ifdef BADCODE
   ctr[1] &= 0x7f;
   ctr[3] &= 0x7f;
#else
   ctr[12] &= 0x7f;
   ctr[14] &= 0x7f;
#endif

   Muint8 nn [ MAes::KeySize ];

   // handle full blocks first
   while ( sizeN >= static_cast<unsigned>(MAes::KeySize) )
   {
      EncryptBuffer(ctr, nn);
      BLK_XOR(pn, nn);
      sizeN -= MAes::KeySize;
      pn += MAes::KeySize;
      // Incrementing the counter

      for ( int i = 15; i >= 0 && !++ctr[i]; --i )
         continue;
   }
   // handle the last (partial block)
   if ( sizeN != 0 )
   {
      EncryptBuffer(ctr, nn);

      // only process the part with data
      for ( int i = 0; sizeN != 0; ++i )
      {
         *pn ^= nn[i];
         sizeN--;
         pn++;
      }
   }
}

void MAesEax::DoCMAC(Muint8* ws, const Muint8* pN, unsigned sizeN)
{
   // handle full blocks first
   while ( sizeN > static_cast<unsigned>(MAes::KeySize) )
   {
      BLK_XOR(ws, pN);
      EncryptBuffer(ws, ws);
      sizeN -= MAes::KeySize;
      pN += MAes::KeySize;
   }

   if ( sizeN == static_cast<unsigned>(MAes::KeySize) )
   {
      BLK_XOR(ws, pN);
      BLK_XOR(ws, m_eaxContext.D);
      EncryptBuffer(ws, ws);
   }
   else if ( sizeN != 0 )      // handle the last (partial block)
   {
      // do the part with data
      int i;
      for ( i = 0; sizeN != 0; ++i )
      {
          ws[i] ^= *pN;
          sizeN--;
          pN++;
      }

      // add the pad byte
      ws[i] ^= 0x80;
      BLK_XOR(ws, m_eaxContext.Q);
      EncryptBuffer(ws, ws);
   }
}
