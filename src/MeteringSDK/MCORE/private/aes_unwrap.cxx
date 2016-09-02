// File MCORE/private/aes_unwrap.cxx
//
// This file is not included into any public header of MeteringSDK.
// Parts inherited, modified, used under permissive license from http://w1.fi/wpa_supplicant/

/*
 * AES key unwrap (RFC3394)
 *
 * Copyright (c) 2003-2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

unsigned MAes::KeyUnwrapBuffer(const Muint8* cipherText, unsigned cipherTextSize, Muint8* keyText)
{
   CheckKeySizeValid(m_key);

   Muint8 a[8], *r, b[BlockSize];
   int i, j;
   unsigned int t;
   const unsigned n = (cipherTextSize / static_cast<unsigned>(MAes::KeyWrapEncryptionExtraSize)) - 1; //  Length of the plaintext key in 64-bit units

   /* 1) Initialize variables. */
   memcpy(a, cipherText, MAes::KeyWrapEncryptionExtraSize);
   r = keyText;
   memcpy(r, cipherText + MAes::KeyWrapEncryptionExtraSize, cipherTextSize - MAes::KeyWrapEncryptionExtraSize);

   /* 2) Compute intermediate values.
    * For j = 5 to 0
    *     For i = n to 1
    *         B = AES-1(K, (A ^ t) | R[i]) where t = n*j+i
    *         A = MSB(64, B)
    *         R[i] = LSB(64, B)
    */
   for (j = 5; j >= 0; j--) {
      r = keyText + (n - 1) * 8;
      for ( i = static_cast<int>(n); i >= 1; i-- )
      {
         memcpy(b, a, 8);
         t = n * j + i;
         b[7] ^= t;
         b[6] ^= t >> 8;
         b[5] ^= t >> 16;
         b[4] ^= t >> 24;

         memcpy(b + 8, r, 8);
         DecryptBuffer(b, b);
         memcpy(a, b, 8);
         memcpy(r, b + 8, 8);
         r -= 8;
      }
   }

   /* 3) Output results.
    *
    * These are already in @plain due to the location of temporary
    * variables. Just verify that the IV matches with the expected value.
    */
   for (i = 0; i < 8; i++) {
      if (a[i] != 0xa6)
      {
         ThrowValidationError();
         M_ENSURED_ASSERT(0);
      }
   }

   return cipherTextSize - MAes::KeyWrapEncryptionExtraSize;
}
