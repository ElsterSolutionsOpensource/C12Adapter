// File MCORE/private/aes_wrap.cxx
//
// This file is not included into any public header of MeteringSDK.
// Parts inherited, modified, used under permissive license from http://w1.fi/wpa_supplicant/

/*
 * AES Key Wrap Algorithm (RFC3394)
 *
 * Copyright (c) 2003-2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

unsigned MAes::KeyWrapBuffer(const Muint8* keyText, unsigned keyTextSize, Muint8* cipherText)
{
   CheckKeySizeValid(m_key);

   Muint8 *a, *r, b[BlockSize];
   int i, j;
   unsigned int t;
   const unsigned n = keyTextSize / static_cast<unsigned>(MAes::KeyWrapEncryptionExtraSize); // Length of the plaintext key in 64-bit units

   a = cipherText;
   r = cipherText + MAes::KeyWrapEncryptionExtraSize;

   /* 1) Initialize variables. */
   memset(a, 0xa6, MAes::KeyWrapEncryptionExtraSize); // default iv
   memcpy(r, keyText, keyTextSize);

   /* 2) Calculate intermediate values.
    * For j = 0 to 5
    *     For i=1 to n
    *         B = AES(K, A | R[i])
    *         A = MSB(64, B) ^ t where t = (n*j)+i
    *         R[i] = LSB(64, B)
    */
   for (j = 0; j <= 5; j++) {
      r = cipherText + 8;
      for (i = 1; i <= static_cast<int>(n); i++) {
         memcpy(b, a, 8);
         memcpy(b + 8, r, 8);
         EncryptBuffer(b, b);
         memcpy(a, b, 8);
         t = n * j + i;
         a[7] ^= t;
         a[6] ^= t >> 8;
         a[5] ^= t >> 16;
         a[4] ^= t >> 24;
         memcpy(r, b + 8, 8);
         r += 8;
      }
   }

   /* 3) Output the results.
    *
    * These are already in @cipherText due to the location of temporary
    * variables.
    */

   return keyTextSize + KeyWrapEncryptionExtraSize;
}

