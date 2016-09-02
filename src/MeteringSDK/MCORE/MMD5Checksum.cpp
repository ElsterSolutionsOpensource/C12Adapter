// File MCORE/MMD5ChecksumImpl.cpp

/****************************************************************************************
This software is derived from the RSA Data Security, Inc. MD5 Message-Digest Algorithm.
Incorporation of this statement is a condition of use; please see the RSA
Data Security Inc copyright notice below:-

Copyright (C) 1990-2, RSA Data Security, Inc. Created 1990. All rights reserved.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.
License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.
License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.
RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
*****************************************************************************************/

/****************************************************************************************
This implementation of the RSA MD5 Algorithm was written by Langfine Ltd.

Langfine Ltd makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

In addition to the above, Langfine make no warrant or assurances regarding the
accuracy of this implementation of the MD5 checksum algorithm nor any assurances regarding
its suitability for any purposes.

This implementation may be used freely provided that Langfine is credited
in a copyright or similar notices (eg, RSA MD5 Algorithm implemented by Langfine
Ltd.) and provided that the RSA Data Security notices are complied with.

Langfine may be contacted at mail@langfine.com
*/

/****************************************************************************************
This work is derived from the above mentioned sources with permission
*/

#include "MCOREExtern.h"
#include "MMD5Checksum.h"

   // Constants to start over
   //
   const Muint32 s_init0 = 0x67452301;
   const Muint32 s_init1 = 0xEFCDAB89;
   const Muint32 s_init2 = 0x98BADCFE;
   const Muint32 s_init3 = 0x10325476;

   // Routine constants
   //
   #define MD5_S11  7
   #define MD5_S12 12
   #define MD5_S13 17
   #define MD5_S14 22
   #define MD5_S21  5
   #define MD5_S22  9
   #define MD5_S23 14
   #define MD5_S24 20
   #define MD5_S31  4
   #define MD5_S32 11
   #define MD5_S33 16
   #define MD5_S34 23
   #define MD5_S41  6
   #define MD5_S42 10
   #define MD5_S43 15
   #define MD5_S44 21

   // Transform constants for Round 1
   #define MD5_T01  0xD76AA478
   #define MD5_T02  0xE8C7B756
   #define MD5_T03  0x242070DB
   #define MD5_T04  0xC1BDCEEE
   #define MD5_T05  0xF57C0FAF
   #define MD5_T06  0x4787C62A
   #define MD5_T07  0xA8304613
   #define MD5_T08  0xFD469501
   #define MD5_T09  0x698098D8
   #define MD5_T10  0x8B44F7AF
   #define MD5_T11  0xFFFF5BB1
   #define MD5_T12  0x895CD7BE
   #define MD5_T13  0x6B901122
   #define MD5_T14  0xFD987193
   #define MD5_T15  0xA679438E
   #define MD5_T16  0x49B40821

   //Transformation Constants - Round 2
   #define MD5_T17  0xF61E2562
   #define MD5_T18  0xC040B340
   #define MD5_T19  0x265E5A51
   #define MD5_T20  0xE9B6C7AA
   #define MD5_T21  0xD62F105D
   #define MD5_T22  0x02441453
   #define MD5_T23  0xD8A1E681
   #define MD5_T24  0xE7D3FBC8
   #define MD5_T25  0x21E1CDE6
   #define MD5_T26  0xC33707D6
   #define MD5_T27  0xF4D50D87
   #define MD5_T28  0x455A14ED
   #define MD5_T29  0xA9E3E905
   #define MD5_T30  0xFCEFA3F8
   #define MD5_T31  0x676F02D9
   #define MD5_T32  0x8D2A4C8A

   //Transformation Constants - Round 3
   #define MD5_T33  0xFFFA3942
   #define MD5_T34  0x8771F681
   #define MD5_T35  0x6D9D6122
   #define MD5_T36  0xFDE5380C
   #define MD5_T37  0xA4BEEA44
   #define MD5_T38  0x4BDECFA9
   #define MD5_T39  0xF6BB4B60
   #define MD5_T40  0xBEBFBC70
   #define MD5_T41  0x289B7EC6
   #define MD5_T42  0xEAA127FA
   #define MD5_T43  0xD4EF3085
   #define MD5_T44  0x04881D05
   #define MD5_T45  0xD9D4D039
   #define MD5_T46  0xE6DB99E5
   #define MD5_T47  0x1FA27CF8
   #define MD5_T48  0xC4AC5665

   //Transformation Constants - Round 4
   #define MD5_T49  0xF4292244
   #define MD5_T50  0x432AFF97
   #define MD5_T51  0xAB9423A7
   #define MD5_T52  0xFC93A039
   #define MD5_T53  0x655B59C3
   #define MD5_T54  0x8F0CCC92
   #define MD5_T55  0xFFEFF47D
   #define MD5_T56  0x85845DD1
   #define MD5_T57  0x6FA87E4F
   #define MD5_T58  0xFE2CE6E0
   #define MD5_T59  0xA3014314
   #define MD5_T60  0x4E0811A1
   #define MD5_T61  0xF7537E82
   #define MD5_T62  0xBD3AF235
   #define MD5_T63  0x2AD7D2BB
   #define MD5_T64  0xEB86D391

   inline unsigned DoRotateLeft(unsigned x, int n)
   {
      return (x << n) | (x >> (32 - n));
   }

   inline void DoFF(unsigned& A, unsigned B, unsigned C, unsigned D, unsigned X, unsigned S, unsigned T)
   {
      unsigned F = (B & C) | (~B & D);
      A += F + X + T;
      A = DoRotateLeft(A, S);
      A += B;
   }

   inline void DoGG(unsigned& A, unsigned B, unsigned C, unsigned D, unsigned X, unsigned S, unsigned T)
   {
      unsigned G = (B & D) | (C & ~D);
      A += G + X + T;
      A = DoRotateLeft(A, S);
      A += B;
   }

   inline void DoHH(unsigned& A, unsigned B, unsigned C, unsigned D, unsigned X, unsigned S, unsigned T)
   {
      unsigned H = (B ^ C ^ D);
      A += H + X + T;
      A = DoRotateLeft(A, S);
      A += B;
   }

   inline void DoII(unsigned& A, unsigned B, unsigned C, unsigned D, unsigned X, unsigned S, unsigned T)
   {
      unsigned I = (C ^ (B | ~D));
      A += I + X + T;
      A = DoRotateLeft(A, S);
      A += B;
   }

M_START_PROPERTIES(MD5Checksum)
M_START_METHODS(MD5Checksum)
   M_CLASS_SERVICE(MD5Checksum, Calculate, ST_MByteString_S_constMByteStringA)
M_END_CLASS(MD5Checksum, Object)

void MMD5Checksum::Reset()
{
   m_count[0] = m_count[1] = 0;
   memset(m_buffer, 0, sizeof(m_buffer));
   m_md5[0] = s_init0;
   m_md5[1] = s_init1;
   m_md5[2] = s_init2;
   m_md5[3] = s_init3;
}

void MMD5Checksum::Update(const MByteString& data)
{
   UpdateWithBytes(data.c_str(), M_64_CAST(unsigned, data.size()));
}

void MMD5Checksum::UpdateWithBytes(const char* in, unsigned size)
{
   unsigned index = (unsigned)((m_count[0] >> 3) & 0x3F);
   if ( (m_count[0] += size << 3)  <  (size << 3) )
      ++m_count[1];
   m_count[1] += (size >> 29);

   unsigned i = 0;      
   unsigned nPartLen = 64 - index;
   if ( size >= nPartLen )
   {
      memcpy(&m_buffer[index], in, nPartLen);
      DoTransform(m_buffer);
      for ( i = nPartLen; i + 63 < size; i += 64 )
         DoTransform((const Muint8*)&in[i]);
      index = 0;
   } 
   else 
      i = 0;
   memcpy(&m_buffer[index], &in[i], size - i);
}

void MMD5Checksum::DoTransform(const Muint8* block)
{
   unsigned a = m_md5[0];
   unsigned b = m_md5[1];
   unsigned c = m_md5[2];
   unsigned d = m_md5[3];
   unsigned X [ 16 ];

   memcpy(X, block, 64);

   // Round 1
   DoFF(a, b, c, d, X[ 0], MD5_S11, MD5_T01); 
   DoFF(d, a, b, c, X[ 1], MD5_S12, MD5_T02); 
   DoFF(c, d, a, b, X[ 2], MD5_S13, MD5_T03); 
   DoFF(b, c, d, a, X[ 3], MD5_S14, MD5_T04); 
   DoFF(a, b, c, d, X[ 4], MD5_S11, MD5_T05); 
   DoFF(d, a, b, c, X[ 5], MD5_S12, MD5_T06); 
   DoFF(c, d, a, b, X[ 6], MD5_S13, MD5_T07); 
   DoFF(b, c, d, a, X[ 7], MD5_S14, MD5_T08); 
   DoFF(a, b, c, d, X[ 8], MD5_S11, MD5_T09); 
   DoFF(d, a, b, c, X[ 9], MD5_S12, MD5_T10); 
   DoFF(c, d, a, b, X[10], MD5_S13, MD5_T11); 
   DoFF(b, c, d, a, X[11], MD5_S14, MD5_T12); 
   DoFF(a, b, c, d, X[12], MD5_S11, MD5_T13); 
   DoFF(d, a, b, c, X[13], MD5_S12, MD5_T14); 
   DoFF(c, d, a, b, X[14], MD5_S13, MD5_T15); 
   DoFF(b, c, d, a, X[15], MD5_S14, MD5_T16); 

   // Round 2
   DoGG(a, b, c, d, X[ 1], MD5_S21, MD5_T17); 
   DoGG(d, a, b, c, X[ 6], MD5_S22, MD5_T18); 
   DoGG(c, d, a, b, X[11], MD5_S23, MD5_T19); 
   DoGG(b, c, d, a, X[ 0], MD5_S24, MD5_T20); 
   DoGG(a, b, c, d, X[ 5], MD5_S21, MD5_T21); 
   DoGG(d, a, b, c, X[10], MD5_S22, MD5_T22); 
   DoGG(c, d, a, b, X[15], MD5_S23, MD5_T23); 
   DoGG(b, c, d, a, X[ 4], MD5_S24, MD5_T24); 
   DoGG(a, b, c, d, X[ 9], MD5_S21, MD5_T25); 
   DoGG(d, a, b, c, X[14], MD5_S22, MD5_T26); 
   DoGG(c, d, a, b, X[ 3], MD5_S23, MD5_T27); 
   DoGG(b, c, d, a, X[ 8], MD5_S24, MD5_T28); 
   DoGG(a, b, c, d, X[13], MD5_S21, MD5_T29); 
   DoGG(d, a, b, c, X[ 2], MD5_S22, MD5_T30); 
   DoGG(c, d, a, b, X[ 7], MD5_S23, MD5_T31); 
   DoGG(b, c, d, a, X[12], MD5_S24, MD5_T32); 

   // Round 3
   DoHH(a, b, c, d, X[ 5], MD5_S31, MD5_T33); 
   DoHH(d, a, b, c, X[ 8], MD5_S32, MD5_T34); 
   DoHH(c, d, a, b, X[11], MD5_S33, MD5_T35); 
   DoHH(b, c, d, a, X[14], MD5_S34, MD5_T36); 
   DoHH(a, b, c, d, X[ 1], MD5_S31, MD5_T37); 
   DoHH(d, a, b, c, X[ 4], MD5_S32, MD5_T38); 
   DoHH(c, d, a, b, X[ 7], MD5_S33, MD5_T39); 
   DoHH(b, c, d, a, X[10], MD5_S34, MD5_T40); 
   DoHH(a, b, c, d, X[13], MD5_S31, MD5_T41); 
   DoHH(d, a, b, c, X[ 0], MD5_S32, MD5_T42); 
   DoHH(c, d, a, b, X[ 3], MD5_S33, MD5_T43); 
   DoHH(b, c, d, a, X[ 6], MD5_S34, MD5_T44); 
   DoHH(a, b, c, d, X[ 9], MD5_S31, MD5_T45); 
   DoHH(d, a, b, c, X[12], MD5_S32, MD5_T46); 
   DoHH(c, d, a, b, X[15], MD5_S33, MD5_T47); 
   DoHH(b, c, d, a, X[ 2], MD5_S34, MD5_T48); 

   // Round 4
   DoII(a, b, c, d, X[ 0], MD5_S41, MD5_T49); 
   DoII(d, a, b, c, X[ 7], MD5_S42, MD5_T50); 
   DoII(c, d, a, b, X[14], MD5_S43, MD5_T51); 
   DoII(b, c, d, a, X[ 5], MD5_S44, MD5_T52); 
   DoII(a, b, c, d, X[12], MD5_S41, MD5_T53); 
   DoII(d, a, b, c, X[ 3], MD5_S42, MD5_T54); 
   DoII(c, d, a, b, X[10], MD5_S43, MD5_T55); 
   DoII(b, c, d, a, X[ 1], MD5_S44, MD5_T56); 
   DoII(a, b, c, d, X[ 8], MD5_S41, MD5_T57); 
   DoII(d, a, b, c, X[15], MD5_S42, MD5_T58); 
   DoII(c, d, a, b, X[ 6], MD5_S43, MD5_T59); 
   DoII(b, c, d, a, X[13], MD5_S44, MD5_T60); 
   DoII(a, b, c, d, X[ 4], MD5_S41, MD5_T61); 
   DoII(d, a, b, c, X[11], MD5_S42, MD5_T62); 
   DoII(c, d, a, b, X[ 2], MD5_S43, MD5_T63); 
   DoII(b, c, d, a, X[ 9], MD5_S44, MD5_T64); 

   // Add the transformed values to the current checksum
   m_md5[0] += a;
   m_md5[1] += b;
   m_md5[2] += c;
   m_md5[3] += d;
}

MByteString MMD5Checksum::GetResult() 
{
   static const char s_pad[64] = 
   {
      '\x80', 
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
   };

   char bits [ 8 ];
   memcpy(bits, m_count, 8);

   unsigned index = (unsigned)((m_count[0] >> 3) & 0x3f);
   unsigned padSize = (index < 56) ? (56 - index) : (120 - index);
   UpdateWithBytes(s_pad, padSize);
   UpdateWithBytes(bits, 8);

   char result [ 16 ];
   memcpy(result, m_md5, sizeof(result));
   return MByteString(result, sizeof(result));
}

MByteString MMD5Checksum::Calculate(const MByteString& data)
{
   MMD5Checksum c;
   c.Update(data);
   return c.GetResult();
}
