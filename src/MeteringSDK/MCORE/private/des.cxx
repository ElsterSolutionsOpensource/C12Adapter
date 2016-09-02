// File MCORE/des.cxx
//
// This file is private, do not compile it directly as it is included into another cpp

/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@gmail.com, http://libtomcrypt.com
 */

///////////////////////////////////////////////
// BEGIN Contents of former file des.h
#define DES

#ifdef __cplusplus
extern "C" {
#endif

/* version */
#define CRYPT   0x0116
#define SCRYPT  "1.16"

/* max size of either a cipher/hash block or symmetric key [largest of the two] */
#define MAXBLOCKSIZE  128

/* descriptor table size */
/* Dropbear change - this should be smaller, saves some size */
#define TAB_SIZE    4

#ifdef DES
struct des_key {
    Muint32 ek[32], dk[32];
};

#ifdef DES3
struct des3_key {
    Muint32 ek[3][32], dk[3][32];
};
#endif
#endif

typedef union Symmetric_key {
#ifdef DES
   struct des_key des;
#ifdef DES3
   struct des3_key des3;
#endif
#endif
   void   *data;
} symmetric_key;

#ifdef DES
static int des_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
static int des_ecb_encrypt(const unsigned char *pt, unsigned char *ct, symmetric_key *skey);
static int des_ecb_decrypt(const unsigned char *ct, unsigned char *pt, symmetric_key *skey);
#endif

#ifdef __cplusplus
   }
#endif

// END Contents of former file des.h
////////////////////////////////////////

#define ENDIAN_NEUTRAL
#define LTC_NO_ROLC

///////////////////////////////////////////////
// BEGIN Contents of former file des_macros.h

/* fix for MSVC ...evil! */
#ifdef _MSC_VER
   #define CONST64(n) n ## ui64
   typedef unsigned __int64 ulong64;
#else
   #define CONST64(n) n ## ULL
   typedef unsigned long long ulong64;
#endif

/* this is the "32-bit at least" data type 
 * Re-define it to suit your platform but it must be at least 32-bits 
 */
#if defined(__x86_64__) || (defined(__sparc__) && defined(__arch64__))
   typedef unsigned ulong32;
#else
   typedef unsigned long ulong32;
#endif

/* ---- HELPER MACROS ---- */
#ifdef ENDIAN_NEUTRAL

#define STORE32L(x, y)                                                                     \
     { (y)[3] = (unsigned char)(((x)>>24)&255); (y)[2] = (unsigned char)(((x)>>16)&255);   \
       (y)[1] = (unsigned char)(((x)>>8)&255); (y)[0] = (unsigned char)((x)&255); }

#define LOAD32L(x, y)                            \
     { x = ((unsigned long)((y)[3] & 255)<<24) | \
           ((unsigned long)((y)[2] & 255)<<16) | \
           ((unsigned long)((y)[1] & 255)<<8)  | \
           ((unsigned long)((y)[0] & 255)); }

#define STORE64L(x, y)                                                                     \
     { (y)[7] = (unsigned char)(((x)>>56)&255); (y)[6] = (unsigned char)(((x)>>48)&255);   \
       (y)[5] = (unsigned char)(((x)>>40)&255); (y)[4] = (unsigned char)(((x)>>32)&255);   \
       (y)[3] = (unsigned char)(((x)>>24)&255); (y)[2] = (unsigned char)(((x)>>16)&255);   \
       (y)[1] = (unsigned char)(((x)>>8)&255); (y)[0] = (unsigned char)((x)&255); }

#define LOAD64L(x, y)                                                       \
     { x = (((ulong64)((y)[7] & 255))<<56)|(((ulong64)((y)[6] & 255))<<48)| \
           (((ulong64)((y)[5] & 255))<<40)|(((ulong64)((y)[4] & 255))<<32)| \
           (((ulong64)((y)[3] & 255))<<24)|(((ulong64)((y)[2] & 255))<<16)| \
           (((ulong64)((y)[1] & 255))<<8)|(((ulong64)((y)[0] & 255))); }

#define STORE32H(x, y)                                                                     \
     { (y)[0] = (unsigned char)(((x)>>24)&255); (y)[1] = (unsigned char)(((x)>>16)&255);   \
       (y)[2] = (unsigned char)(((x)>>8)&255); (y)[3] = (unsigned char)((x)&255); }

#define LOAD32H(x, y)                            \
     { x = ((unsigned long)((y)[0] & 255)<<24) | \
           ((unsigned long)((y)[1] & 255)<<16) | \
           ((unsigned long)((y)[2] & 255)<<8)  | \
           ((unsigned long)((y)[3] & 255)); }

#define STORE64H(x, y)                                                                     \
   { (y)[0] = (unsigned char)(((x)>>56)&255); (y)[1] = (unsigned char)(((x)>>48)&255);     \
     (y)[2] = (unsigned char)(((x)>>40)&255); (y)[3] = (unsigned char)(((x)>>32)&255);     \
     (y)[4] = (unsigned char)(((x)>>24)&255); (y)[5] = (unsigned char)(((x)>>16)&255);     \
     (y)[6] = (unsigned char)(((x)>>8)&255); (y)[7] = (unsigned char)((x)&255); }

#define LOAD64H(x, y)                                                      \
   { x = (((ulong64)((y)[0] & 255))<<56)|(((ulong64)((y)[1] & 255))<<48) | \
         (((ulong64)((y)[2] & 255))<<40)|(((ulong64)((y)[3] & 255))<<32) | \
         (((ulong64)((y)[4] & 255))<<24)|(((ulong64)((y)[5] & 255))<<16) | \
         (((ulong64)((y)[6] & 255))<<8)|(((ulong64)((y)[7] & 255))); }

#endif /* ENDIAN_NEUTRAL */

#ifdef ENDIAN_LITTLE

#if !defined(LTC_NO_BSWAP) && (defined(INTEL_CC) || (defined(__GNUC__) && (defined(__DJGPP__) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__i386__) || defined(__x86_64__))))

#define STORE32H(x, y)           \
asm __volatile__ (               \
   "bswapl %0     \n\t"          \
   "movl   %0,(%1)\n\t"          \
   "bswapl %0     \n\t"          \
      ::"r"(x), "r"(y));

#define LOAD32H(x, y)          \
asm __volatile__ (             \
   "movl (%1),%0\n\t"          \
   "bswapl %0\n\t"             \
   :"=r"(x): "r"(y));

#else

#define STORE32H(x, y)                                                                     \
     { (y)[0] = (unsigned char)(((x)>>24)&255); (y)[1] = (unsigned char)(((x)>>16)&255);   \
       (y)[2] = (unsigned char)(((x)>>8)&255); (y)[3] = (unsigned char)((x)&255); }

#define LOAD32H(x, y)                            \
     { x = ((unsigned long)((y)[0] & 255)<<24) | \
           ((unsigned long)((y)[1] & 255)<<16) | \
           ((unsigned long)((y)[2] & 255)<<8)  | \
           ((unsigned long)((y)[3] & 255)); }

#endif


/* x86_64 processor */
#if !defined(LTC_NO_BSWAP) && (defined(__GNUC__) && defined(__x86_64__))

#define STORE64H(x, y)           \
asm __volatile__ (               \
   "bswapq %0     \n\t"          \
   "movq   %0,(%1)\n\t"          \
   "bswapq %0     \n\t"          \
      ::"r"(x), "r"(y));

#define LOAD64H(x, y)          \
asm __volatile__ (             \
   "movq (%1),%0\n\t"          \
   "bswapq %0\n\t"             \
   :"=r"(x): "r"(y));

#else

#define STORE64H(x, y)                                                                     \
   { (y)[0] = (unsigned char)(((x)>>56)&255); (y)[1] = (unsigned char)(((x)>>48)&255);     \
     (y)[2] = (unsigned char)(((x)>>40)&255); (y)[3] = (unsigned char)(((x)>>32)&255);     \
     (y)[4] = (unsigned char)(((x)>>24)&255); (y)[5] = (unsigned char)(((x)>>16)&255);     \
     (y)[6] = (unsigned char)(((x)>>8)&255); (y)[7] = (unsigned char)((x)&255); }

#define LOAD64H(x, y)                                                      \
   { x = (((ulong64)((y)[0] & 255))<<56)|(((ulong64)((y)[1] & 255))<<48) | \
         (((ulong64)((y)[2] & 255))<<40)|(((ulong64)((y)[3] & 255))<<32) | \
         (((ulong64)((y)[4] & 255))<<24)|(((ulong64)((y)[5] & 255))<<16) | \
         (((ulong64)((y)[6] & 255))<<8)|(((ulong64)((y)[7] & 255))); }

#endif

#ifdef ENDIAN_32BITWORD 

#define STORE32L(x, y)        \
     { ulong32  __t = (x); XMEMCPY(y, &__t, 4); }

#define LOAD32L(x, y)         \
     XMEMCPY(&(x), y, 4);

#define STORE64L(x, y)                                                                     \
     { (y)[7] = (unsigned char)(((x)>>56)&255); (y)[6] = (unsigned char)(((x)>>48)&255);   \
       (y)[5] = (unsigned char)(((x)>>40)&255); (y)[4] = (unsigned char)(((x)>>32)&255);   \
       (y)[3] = (unsigned char)(((x)>>24)&255); (y)[2] = (unsigned char)(((x)>>16)&255);   \
       (y)[1] = (unsigned char)(((x)>>8)&255); (y)[0] = (unsigned char)((x)&255); }

#define LOAD64L(x, y)                                                       \
     { x = (((ulong64)((y)[7] & 255))<<56)|(((ulong64)((y)[6] & 255))<<48)| \
           (((ulong64)((y)[5] & 255))<<40)|(((ulong64)((y)[4] & 255))<<32)| \
           (((ulong64)((y)[3] & 255))<<24)|(((ulong64)((y)[2] & 255))<<16)| \
           (((ulong64)((y)[1] & 255))<<8)|(((ulong64)((y)[0] & 255))); }

#else /* 64-bit words then  */

#define STORE32L(x, y)        \
     { ulong32 __t = (x); XMEMCPY(y, &__t, 4); }

#define LOAD32L(x, y)         \
     { XMEMCPY(&(x), y, 4); x &= 0xFFFFFFFF; }

#define STORE64L(x, y)        \
     { ulong64 __t = (x); XMEMCPY(y, &__t, 8); }

#define LOAD64L(x, y)         \
    { XMEMCPY(&(x), y, 8); }

#endif /* ENDIAN_64BITWORD */

#endif /* ENDIAN_LITTLE */

#ifdef ENDIAN_BIG
#define STORE32L(x, y)                                                                     \
     { (y)[3] = (unsigned char)(((x)>>24)&255); (y)[2] = (unsigned char)(((x)>>16)&255);   \
       (y)[1] = (unsigned char)(((x)>>8)&255); (y)[0] = (unsigned char)((x)&255); }

#define LOAD32L(x, y)                            \
     { x = ((unsigned long)((y)[3] & 255)<<24) | \
           ((unsigned long)((y)[2] & 255)<<16) | \
           ((unsigned long)((y)[1] & 255)<<8)  | \
           ((unsigned long)((y)[0] & 255)); }

#define STORE64L(x, y)                                                                     \
   { (y)[7] = (unsigned char)(((x)>>56)&255); (y)[6] = (unsigned char)(((x)>>48)&255);     \
     (y)[5] = (unsigned char)(((x)>>40)&255); (y)[4] = (unsigned char)(((x)>>32)&255);     \
     (y)[3] = (unsigned char)(((x)>>24)&255); (y)[2] = (unsigned char)(((x)>>16)&255);     \
     (y)[1] = (unsigned char)(((x)>>8)&255); (y)[0] = (unsigned char)((x)&255); }

#define LOAD64L(x, y)                                                      \
   { x = (((ulong64)((y)[7] & 255))<<56)|(((ulong64)((y)[6] & 255))<<48) | \
         (((ulong64)((y)[5] & 255))<<40)|(((ulong64)((y)[4] & 255))<<32) | \
         (((ulong64)((y)[3] & 255))<<24)|(((ulong64)((y)[2] & 255))<<16) | \
         (((ulong64)((y)[1] & 255))<<8)|(((ulong64)((y)[0] & 255))); }

#ifdef ENDIAN_32BITWORD 

#define STORE32H(x, y)        \
     { ulong32 __t = (x); XMEMCPY(y, &__t, 4); }

#define LOAD32H(x, y)         \
     XMEMCPY(&(x), y, 4);

#define STORE64H(x, y)                                                                     \
     { (y)[0] = (unsigned char)(((x)>>56)&255); (y)[1] = (unsigned char)(((x)>>48)&255);   \
       (y)[2] = (unsigned char)(((x)>>40)&255); (y)[3] = (unsigned char)(((x)>>32)&255);   \
       (y)[4] = (unsigned char)(((x)>>24)&255); (y)[5] = (unsigned char)(((x)>>16)&255);   \
       (y)[6] = (unsigned char)(((x)>>8)&255);  (y)[7] = (unsigned char)((x)&255); }

#define LOAD64H(x, y)                                                       \
     { x = (((ulong64)((y)[0] & 255))<<56)|(((ulong64)((y)[1] & 255))<<48)| \
           (((ulong64)((y)[2] & 255))<<40)|(((ulong64)((y)[3] & 255))<<32)| \
           (((ulong64)((y)[4] & 255))<<24)|(((ulong64)((y)[5] & 255))<<16)| \
           (((ulong64)((y)[6] & 255))<<8)| (((ulong64)((y)[7] & 255))); }

#else /* 64-bit words then  */

#define STORE32H(x, y)        \
     { ulong32 __t = (x); XMEMCPY(y, &__t, 4); }

#define LOAD32H(x, y)         \
     { XMEMCPY(&(x), y, 4); x &= 0xFFFFFFFF; }

#define STORE64H(x, y)        \
     { ulong64 __t = (x); XMEMCPY(y, &__t, 8); }

#define LOAD64H(x, y)         \
    { XMEMCPY(&(x), y, 8); }

#endif /* ENDIAN_64BITWORD */
#endif /* ENDIAN_BIG */

#define BSWAP(x)  ( ((x>>24)&0x000000FFUL) | ((x<<24)&0xFF000000UL)  | \
                    ((x>>8)&0x0000FF00UL)  | ((x<<8)&0x00FF0000UL) )


/* 32-bit Rotates */
#if defined(_MSC_VER)

/* instrinsic rotate */
#include <stdlib.h>
#pragma intrinsic(_lrotr,_lrotl)
#define ROR(x,n) _lrotr(x,n)
#define ROL(x,n) _lrotl(x,n)
#define RORc(x,n) _lrotr(x,n)
#define ROLc(x,n) _lrotl(x,n)

#elif !defined(__STRICT_ANSI__) && defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)) && !defined(INTEL_CC) && !defined(LTC_NO_ASM)

static inline unsigned ROL(unsigned word, int i)
{
   asm ("roll %%cl,%0"
      :"=r" (word)
      :"0" (word),"c" (i));
   return word;
}

static inline unsigned ROR(unsigned word, int i)
{
   asm ("rorl %%cl,%0"
      :"=r" (word)
      :"0" (word),"c" (i));
   return word;
}

#ifndef LTC_NO_ROLC

static inline unsigned ROLc(unsigned word, const int i)
{
   asm ("roll %2,%0"
      :"=r" (word)
      :"0" (word),"I" (i));
   return word;
}

static inline unsigned RORc(unsigned word, const int i)
{
   asm ("rorl %2,%0"
      :"=r" (word)
      :"0" (word),"I" (i));
   return word;
}

#else

#define ROLc ROL
#define RORc ROR

#endif

#elif !defined(__STRICT_ANSI__) && defined(LTC_PPC32)

static inline unsigned ROL(unsigned word, int i)
{
   asm ("rotlw %0,%0,%2"
      :"=r" (word)
      :"0" (word),"r" (i));
   return word;
}

static inline unsigned ROR(unsigned word, int i)
{
   asm ("rotlw %0,%0,%2"
      :"=r" (word)
      :"0" (word),"r" (32-i));
   return word;
}

#ifndef LTC_NO_ROLC

static inline unsigned ROLc(unsigned word, const int i)
{
   asm ("rotlwi %0,%0,%2"
      :"=r" (word)
      :"0" (word),"I" (i));
   return word;
}

static inline unsigned RORc(unsigned word, const int i)
{
   asm ("rotrwi %0,%0,%2"
      :"=r" (word)
      :"0" (word),"I" (i));
   return word;
}

#else

#define ROLc ROL
#define RORc ROR

#endif


#else

/* rotates the hard way */
#define ROL(x, y) ( (((unsigned long)(x)<<(unsigned long)((y)&31)) | (((unsigned long)(x)&0xFFFFFFFFUL)>>(unsigned long)(32-((y)&31)))) & 0xFFFFFFFFUL)
#define ROR(x, y) ( ((((unsigned long)(x)&0xFFFFFFFFUL)>>(unsigned long)((y)&31)) | ((unsigned long)(x)<<(unsigned long)(32-((y)&31)))) & 0xFFFFFFFFUL)
#define ROLc(x, y) ( (((unsigned long)(x)<<(unsigned long)((y)&31)) | (((unsigned long)(x)&0xFFFFFFFFUL)>>(unsigned long)(32-((y)&31)))) & 0xFFFFFFFFUL)
#define RORc(x, y) ( ((((unsigned long)(x)&0xFFFFFFFFUL)>>(unsigned long)((y)&31)) | ((unsigned long)(x)<<(unsigned long)(32-((y)&31)))) & 0xFFFFFFFFUL)

#endif


/* 64-bit Rotates */
#if !defined(__STRICT_ANSI__) && defined(__GNUC__) && defined(__x86_64__) && !defined(LTC_NO_ASM)

static inline unsigned long ROL64(unsigned long word, int i)
{
   asm("rolq %%cl,%0"
      :"=r" (word)
      :"0" (word),"c" (i));
   return word;
}

static inline unsigned long ROR64(unsigned long word, int i)
{
   asm("rorq %%cl,%0"
      :"=r" (word)
      :"0" (word),"c" (i));
   return word;
}

#ifndef LTC_NO_ROLC

static inline unsigned long ROL64c(unsigned long word, const int i)
{
   asm("rolq %2,%0"
      :"=r" (word)
      :"0" (word),"J" (i));
   return word;
}

static inline unsigned long ROR64c(unsigned long word, const int i)
{
   asm("rorq %2,%0"
      :"=r" (word)
      :"0" (word),"J" (i));
   return word;
}

#else /* LTC_NO_ROLC */

#define ROL64c ROL64
#define ROR64c ROR64

#endif

#else /* Not x86_64  */

#define ROL64(x, y) \
    ( (((x)<<((ulong64)(y)&63)) | \
      (((x)&CONST64(0xFFFFFFFFFFFFFFFF))>>((ulong64)64-((y)&63)))) & CONST64(0xFFFFFFFFFFFFFFFF))

#define ROR64(x, y) \
    ( ((((x)&CONST64(0xFFFFFFFFFFFFFFFF))>>((ulong64)(y)&CONST64(63))) | \
      ((x)<<((ulong64)(64-((y)&CONST64(63)))))) & CONST64(0xFFFFFFFFFFFFFFFF))

#define ROL64c(x, y) \
    ( (((x)<<((ulong64)(y)&63)) | \
      (((x)&CONST64(0xFFFFFFFFFFFFFFFF))>>((ulong64)64-((y)&63)))) & CONST64(0xFFFFFFFFFFFFFFFF))

#define ROR64c(x, y) \
    ( ((((x)&CONST64(0xFFFFFFFFFFFFFFFF))>>((ulong64)(y)&CONST64(63))) | \
      ((x)<<((ulong64)(64-((y)&CONST64(63)))))) & CONST64(0xFFFFFFFFFFFFFFFF))

#endif

#ifndef MAX
   #define MAX(x, y) ( ((x)>(y))?(x):(y) )
#endif

#ifndef MIN
   #define MIN(x, y) ( ((x)<(y))?(x):(y) )
#endif

/* extract a byte portably */
#ifdef _MSC_VER
   #define byte(x, n) ((unsigned char)((x) >> (8 * (n))))
#else
   #define byte(x, n) (((x) >> (8 * (n))) & 255)
#endif   

/* $Source: /cvs/libtom/libtomcrypt/src/headers/tomcrypt_macros.h,v $ */
/* $Revision: 1.15 $ */
/* $Date: 2006/11/29 23:43:57 $ */

// END of contents of former file des_macros.h
///////////////////////////////////////////////


#define LTC_ARGCHK(x) assert((x))

#if M_OS & M_OS_UCLINUX
#  define LTC_SMALL_CODE
#endif

/* error codes [will be expanded in future releases] */
enum {
   CRYPT_OK=0,             /* Result OK */
   CRYPT_ERROR,            /* Generic Error */
   CRYPT_NOP,              /* Not a failure but no operation was performed */

   CRYPT_INVALID_KEYSIZE,  /* Invalid key size given */
   CRYPT_INVALID_ROUNDS,   /* Invalid number of rounds */
   CRYPT_FAIL_TESTVECTOR,  /* Algorithm failed test vectors */

   CRYPT_BUFFER_OVERFLOW,  /* Not enough space for output */
   CRYPT_INVALID_PACKET,   /* Invalid input packet given */

   CRYPT_INVALID_PRNGSIZE, /* Invalid number of bits for a PRNG */
   CRYPT_ERROR_READPRNG,   /* Could not read enough from PRNG */

   CRYPT_INVALID_CIPHER,   /* Invalid cipher specified */
   CRYPT_INVALID_HASH,     /* Invalid hash specified */
   CRYPT_INVALID_PRNG,     /* Invalid PRNG specified */

   CRYPT_MEM,              /* Out of memory */

   CRYPT_PK_TYPE_MISMATCH, /* Not equivalent types of PK keys */
   CRYPT_PK_NOT_PRIVATE,   /* Requires a private PK key */

   CRYPT_INVALID_ARG,      /* Generic invalid argument */
   CRYPT_FILE_NOTFOUND,    /* File Not Found */

   CRYPT_PK_INVALID_TYPE,  /* Invalid type of PK key */
   CRYPT_PK_INVALID_SYSTEM,/* Invalid PK system specified */
   CRYPT_PK_DUP,           /* Duplicate key already in key ring */
   CRYPT_PK_NOT_FOUND,     /* Key not found in keyring */
   CRYPT_PK_INVALID_SIZE,  /* Invalid size input for PK parameters */

   CRYPT_INVALID_PRIME_SIZE,/* Invalid size of prime requested */
   CRYPT_PK_INVALID_PADDING /* Invalid padding on input */
};

/**
  @file des.c
  DES code submitted by Dobes Vandermeer
*/

#ifdef DES

#define EN0 0
#define DE1 1

static const Muint32 bytebit[8] =
{
    0200, 0100, 040, 020, 010, 04, 02, 01
};

static const Muint32 bigbyte[24] =
{
    0x800000UL,  0x400000UL,  0x200000UL,  0x100000UL,
    0x80000UL,   0x40000UL,   0x20000UL,   0x10000UL,
    0x8000UL,    0x4000UL,    0x2000UL,    0x1000UL,
    0x800UL,     0x400UL,     0x200UL,     0x100UL,
    0x80UL,      0x40UL,      0x20UL,      0x10UL,
    0x8UL,       0x4UL,       0x2UL,       0x1L
};

/* Use the key schedule specific in the standard (ANSI X3.92-1981) */

static const unsigned char pc1[56] = {
    56, 48, 40, 32, 24, 16,  8,  0, 57, 49, 41, 33, 25, 17,
     9,  1, 58, 50, 42, 34, 26, 18, 10,  2, 59, 51, 43, 35,
    62, 54, 46, 38, 30, 22, 14,  6, 61, 53, 45, 37, 29, 21,
    13,  5, 60, 52, 44, 36, 28, 20, 12,  4, 27, 19, 11,  3
};

static const unsigned char totrot[16] = {
    1,   2,  4,  6,
    8,  10, 12, 14,
    15, 17, 19, 21,
    23, 25, 27, 28
};

static const unsigned char pc2[48] = {
    13, 16, 10, 23,  0,  4,      2, 27, 14,  5, 20,  9,
    22, 18, 11,  3, 25,  7,     15,  6, 26, 19, 12,  1,
    40, 51, 30, 36, 46, 54,     29, 39, 50, 44, 32, 47,
    43, 48, 38, 55, 33, 52,     45, 41, 49, 35, 28, 31
};


static const Muint32 SP1[64] =
{
    0x01010400UL, 0x00000000UL, 0x00010000UL, 0x01010404UL,
    0x01010004UL, 0x00010404UL, 0x00000004UL, 0x00010000UL,
    0x00000400UL, 0x01010400UL, 0x01010404UL, 0x00000400UL,
    0x01000404UL, 0x01010004UL, 0x01000000UL, 0x00000004UL,
    0x00000404UL, 0x01000400UL, 0x01000400UL, 0x00010400UL,
    0x00010400UL, 0x01010000UL, 0x01010000UL, 0x01000404UL,
    0x00010004UL, 0x01000004UL, 0x01000004UL, 0x00010004UL,
    0x00000000UL, 0x00000404UL, 0x00010404UL, 0x01000000UL,
    0x00010000UL, 0x01010404UL, 0x00000004UL, 0x01010000UL,
    0x01010400UL, 0x01000000UL, 0x01000000UL, 0x00000400UL,
    0x01010004UL, 0x00010000UL, 0x00010400UL, 0x01000004UL,
    0x00000400UL, 0x00000004UL, 0x01000404UL, 0x00010404UL,
    0x01010404UL, 0x00010004UL, 0x01010000UL, 0x01000404UL,
    0x01000004UL, 0x00000404UL, 0x00010404UL, 0x01010400UL,
    0x00000404UL, 0x01000400UL, 0x01000400UL, 0x00000000UL,
    0x00010004UL, 0x00010400UL, 0x00000000UL, 0x01010004UL
};

static const Muint32 SP2[64] =
{
    0x80108020UL, 0x80008000UL, 0x00008000UL, 0x00108020UL,
    0x00100000UL, 0x00000020UL, 0x80100020UL, 0x80008020UL,
    0x80000020UL, 0x80108020UL, 0x80108000UL, 0x80000000UL,
    0x80008000UL, 0x00100000UL, 0x00000020UL, 0x80100020UL,
    0x00108000UL, 0x00100020UL, 0x80008020UL, 0x00000000UL,
    0x80000000UL, 0x00008000UL, 0x00108020UL, 0x80100000UL,
    0x00100020UL, 0x80000020UL, 0x00000000UL, 0x00108000UL,
    0x00008020UL, 0x80108000UL, 0x80100000UL, 0x00008020UL,
    0x00000000UL, 0x00108020UL, 0x80100020UL, 0x00100000UL,
    0x80008020UL, 0x80100000UL, 0x80108000UL, 0x00008000UL,
    0x80100000UL, 0x80008000UL, 0x00000020UL, 0x80108020UL,
    0x00108020UL, 0x00000020UL, 0x00008000UL, 0x80000000UL,
    0x00008020UL, 0x80108000UL, 0x00100000UL, 0x80000020UL,
    0x00100020UL, 0x80008020UL, 0x80000020UL, 0x00100020UL,
    0x00108000UL, 0x00000000UL, 0x80008000UL, 0x00008020UL,
    0x80000000UL, 0x80100020UL, 0x80108020UL, 0x00108000UL
};

static const Muint32 SP3[64] =
{
    0x00000208UL, 0x08020200UL, 0x00000000UL, 0x08020008UL,
    0x08000200UL, 0x00000000UL, 0x00020208UL, 0x08000200UL,
    0x00020008UL, 0x08000008UL, 0x08000008UL, 0x00020000UL,
    0x08020208UL, 0x00020008UL, 0x08020000UL, 0x00000208UL,
    0x08000000UL, 0x00000008UL, 0x08020200UL, 0x00000200UL,
    0x00020200UL, 0x08020000UL, 0x08020008UL, 0x00020208UL,
    0x08000208UL, 0x00020200UL, 0x00020000UL, 0x08000208UL,
    0x00000008UL, 0x08020208UL, 0x00000200UL, 0x08000000UL,
    0x08020200UL, 0x08000000UL, 0x00020008UL, 0x00000208UL,
    0x00020000UL, 0x08020200UL, 0x08000200UL, 0x00000000UL,
    0x00000200UL, 0x00020008UL, 0x08020208UL, 0x08000200UL,
    0x08000008UL, 0x00000200UL, 0x00000000UL, 0x08020008UL,
    0x08000208UL, 0x00020000UL, 0x08000000UL, 0x08020208UL,
    0x00000008UL, 0x00020208UL, 0x00020200UL, 0x08000008UL,
    0x08020000UL, 0x08000208UL, 0x00000208UL, 0x08020000UL,
    0x00020208UL, 0x00000008UL, 0x08020008UL, 0x00020200UL
};

static const Muint32 SP4[64] =
{
    0x00802001UL, 0x00002081UL, 0x00002081UL, 0x00000080UL,
    0x00802080UL, 0x00800081UL, 0x00800001UL, 0x00002001UL,
    0x00000000UL, 0x00802000UL, 0x00802000UL, 0x00802081UL,
    0x00000081UL, 0x00000000UL, 0x00800080UL, 0x00800001UL,
    0x00000001UL, 0x00002000UL, 0x00800000UL, 0x00802001UL,
    0x00000080UL, 0x00800000UL, 0x00002001UL, 0x00002080UL,
    0x00800081UL, 0x00000001UL, 0x00002080UL, 0x00800080UL,
    0x00002000UL, 0x00802080UL, 0x00802081UL, 0x00000081UL,
    0x00800080UL, 0x00800001UL, 0x00802000UL, 0x00802081UL,
    0x00000081UL, 0x00000000UL, 0x00000000UL, 0x00802000UL,
    0x00002080UL, 0x00800080UL, 0x00800081UL, 0x00000001UL,
    0x00802001UL, 0x00002081UL, 0x00002081UL, 0x00000080UL,
    0x00802081UL, 0x00000081UL, 0x00000001UL, 0x00002000UL,
    0x00800001UL, 0x00002001UL, 0x00802080UL, 0x00800081UL,
    0x00002001UL, 0x00002080UL, 0x00800000UL, 0x00802001UL,
    0x00000080UL, 0x00800000UL, 0x00002000UL, 0x00802080UL
};

static const Muint32 SP5[64] =
{
    0x00000100UL, 0x02080100UL, 0x02080000UL, 0x42000100UL,
    0x00080000UL, 0x00000100UL, 0x40000000UL, 0x02080000UL,
    0x40080100UL, 0x00080000UL, 0x02000100UL, 0x40080100UL,
    0x42000100UL, 0x42080000UL, 0x00080100UL, 0x40000000UL,
    0x02000000UL, 0x40080000UL, 0x40080000UL, 0x00000000UL,
    0x40000100UL, 0x42080100UL, 0x42080100UL, 0x02000100UL,
    0x42080000UL, 0x40000100UL, 0x00000000UL, 0x42000000UL,
    0x02080100UL, 0x02000000UL, 0x42000000UL, 0x00080100UL,
    0x00080000UL, 0x42000100UL, 0x00000100UL, 0x02000000UL,
    0x40000000UL, 0x02080000UL, 0x42000100UL, 0x40080100UL,
    0x02000100UL, 0x40000000UL, 0x42080000UL, 0x02080100UL,
    0x40080100UL, 0x00000100UL, 0x02000000UL, 0x42080000UL,
    0x42080100UL, 0x00080100UL, 0x42000000UL, 0x42080100UL,
    0x02080000UL, 0x00000000UL, 0x40080000UL, 0x42000000UL,
    0x00080100UL, 0x02000100UL, 0x40000100UL, 0x00080000UL,
    0x00000000UL, 0x40080000UL, 0x02080100UL, 0x40000100UL
};

static const Muint32 SP6[64] =
{
    0x20000010UL, 0x20400000UL, 0x00004000UL, 0x20404010UL,
    0x20400000UL, 0x00000010UL, 0x20404010UL, 0x00400000UL,
    0x20004000UL, 0x00404010UL, 0x00400000UL, 0x20000010UL,
    0x00400010UL, 0x20004000UL, 0x20000000UL, 0x00004010UL,
    0x00000000UL, 0x00400010UL, 0x20004010UL, 0x00004000UL,
    0x00404000UL, 0x20004010UL, 0x00000010UL, 0x20400010UL,
    0x20400010UL, 0x00000000UL, 0x00404010UL, 0x20404000UL,
    0x00004010UL, 0x00404000UL, 0x20404000UL, 0x20000000UL,
    0x20004000UL, 0x00000010UL, 0x20400010UL, 0x00404000UL,
    0x20404010UL, 0x00400000UL, 0x00004010UL, 0x20000010UL,
    0x00400000UL, 0x20004000UL, 0x20000000UL, 0x00004010UL,
    0x20000010UL, 0x20404010UL, 0x00404000UL, 0x20400000UL,
    0x00404010UL, 0x20404000UL, 0x00000000UL, 0x20400010UL,
    0x00000010UL, 0x00004000UL, 0x20400000UL, 0x00404010UL,
    0x00004000UL, 0x00400010UL, 0x20004010UL, 0x00000000UL,
    0x20404000UL, 0x20000000UL, 0x00400010UL, 0x20004010UL
};

static const Muint32 SP7[64] =
{
    0x00200000UL, 0x04200002UL, 0x04000802UL, 0x00000000UL,
    0x00000800UL, 0x04000802UL, 0x00200802UL, 0x04200800UL,
    0x04200802UL, 0x00200000UL, 0x00000000UL, 0x04000002UL,
    0x00000002UL, 0x04000000UL, 0x04200002UL, 0x00000802UL,
    0x04000800UL, 0x00200802UL, 0x00200002UL, 0x04000800UL,
    0x04000002UL, 0x04200000UL, 0x04200800UL, 0x00200002UL,
    0x04200000UL, 0x00000800UL, 0x00000802UL, 0x04200802UL,
    0x00200800UL, 0x00000002UL, 0x04000000UL, 0x00200800UL,
    0x04000000UL, 0x00200800UL, 0x00200000UL, 0x04000802UL,
    0x04000802UL, 0x04200002UL, 0x04200002UL, 0x00000002UL,
    0x00200002UL, 0x04000000UL, 0x04000800UL, 0x00200000UL,
    0x04200800UL, 0x00000802UL, 0x00200802UL, 0x04200800UL,
    0x00000802UL, 0x04000002UL, 0x04200802UL, 0x04200000UL,
    0x00200800UL, 0x00000000UL, 0x00000002UL, 0x04200802UL,
    0x00000000UL, 0x00200802UL, 0x04200000UL, 0x00000800UL,
    0x04000002UL, 0x04000800UL, 0x00000800UL, 0x00200002UL
};

static const Muint32 SP8[64] =
{
    0x10001040UL, 0x00001000UL, 0x00040000UL, 0x10041040UL,
    0x10000000UL, 0x10001040UL, 0x00000040UL, 0x10000000UL,
    0x00040040UL, 0x10040000UL, 0x10041040UL, 0x00041000UL,
    0x10041000UL, 0x00041040UL, 0x00001000UL, 0x00000040UL,
    0x10040000UL, 0x10000040UL, 0x10001000UL, 0x00001040UL,
    0x00041000UL, 0x00040040UL, 0x10040040UL, 0x10041000UL,
    0x00001040UL, 0x00000000UL, 0x00000000UL, 0x10040040UL,
    0x10000040UL, 0x10001000UL, 0x00041040UL, 0x00040000UL,
    0x00041040UL, 0x00040000UL, 0x10041000UL, 0x00001000UL,
    0x00000040UL, 0x10040040UL, 0x00001000UL, 0x00041040UL,
    0x10001000UL, 0x00000040UL, 0x10000040UL, 0x10040000UL,
    0x10040040UL, 0x10000000UL, 0x00040000UL, 0x10001040UL,
    0x00000000UL, 0x10041040UL, 0x00040040UL, 0x10000040UL,
    0x10040000UL, 0x10001000UL, 0x10001040UL, 0x00000000UL,
    0x10041040UL, 0x00041000UL, 0x00041000UL, 0x00001040UL,
    0x00001040UL, 0x00040040UL, 0x10000000UL, 0x10041000UL
};

#ifndef LTC_SMALL_CODE

static const ulong64 des_ip[8][256] = {

{ CONST64(0x0000000000000000), CONST64(0x0000001000000000), CONST64(0x0000000000000010), CONST64(0x0000001000000010),
  CONST64(0x0000100000000000), CONST64(0x0000101000000000), CONST64(0x0000100000000010), CONST64(0x0000101000000010),
  CONST64(0x0000000000001000), CONST64(0x0000001000001000), CONST64(0x0000000000001010), CONST64(0x0000001000001010),
  CONST64(0x0000100000001000), CONST64(0x0000101000001000), CONST64(0x0000100000001010), CONST64(0x0000101000001010),
  CONST64(0x0010000000000000), CONST64(0x0010001000000000), CONST64(0x0010000000000010), CONST64(0x0010001000000010),
  CONST64(0x0010100000000000), CONST64(0x0010101000000000), CONST64(0x0010100000000010), CONST64(0x0010101000000010),
  CONST64(0x0010000000001000), CONST64(0x0010001000001000), CONST64(0x0010000000001010), CONST64(0x0010001000001010),
  CONST64(0x0010100000001000), CONST64(0x0010101000001000), CONST64(0x0010100000001010), CONST64(0x0010101000001010),
  CONST64(0x0000000000100000), CONST64(0x0000001000100000), CONST64(0x0000000000100010), CONST64(0x0000001000100010),
  CONST64(0x0000100000100000), CONST64(0x0000101000100000), CONST64(0x0000100000100010), CONST64(0x0000101000100010),
  CONST64(0x0000000000101000), CONST64(0x0000001000101000), CONST64(0x0000000000101010), CONST64(0x0000001000101010),
  CONST64(0x0000100000101000), CONST64(0x0000101000101000), CONST64(0x0000100000101010), CONST64(0x0000101000101010),
  CONST64(0x0010000000100000), CONST64(0x0010001000100000), CONST64(0x0010000000100010), CONST64(0x0010001000100010),
  CONST64(0x0010100000100000), CONST64(0x0010101000100000), CONST64(0x0010100000100010), CONST64(0x0010101000100010),
  CONST64(0x0010000000101000), CONST64(0x0010001000101000), CONST64(0x0010000000101010), CONST64(0x0010001000101010),
  CONST64(0x0010100000101000), CONST64(0x0010101000101000), CONST64(0x0010100000101010), CONST64(0x0010101000101010),
  CONST64(0x1000000000000000), CONST64(0x1000001000000000), CONST64(0x1000000000000010), CONST64(0x1000001000000010),
  CONST64(0x1000100000000000), CONST64(0x1000101000000000), CONST64(0x1000100000000010), CONST64(0x1000101000000010),
  CONST64(0x1000000000001000), CONST64(0x1000001000001000), CONST64(0x1000000000001010), CONST64(0x1000001000001010),
  CONST64(0x1000100000001000), CONST64(0x1000101000001000), CONST64(0x1000100000001010), CONST64(0x1000101000001010),
  CONST64(0x1010000000000000), CONST64(0x1010001000000000), CONST64(0x1010000000000010), CONST64(0x1010001000000010),
  CONST64(0x1010100000000000), CONST64(0x1010101000000000), CONST64(0x1010100000000010), CONST64(0x1010101000000010),
  CONST64(0x1010000000001000), CONST64(0x1010001000001000), CONST64(0x1010000000001010), CONST64(0x1010001000001010),
  CONST64(0x1010100000001000), CONST64(0x1010101000001000), CONST64(0x1010100000001010), CONST64(0x1010101000001010),
  CONST64(0x1000000000100000), CONST64(0x1000001000100000), CONST64(0x1000000000100010), CONST64(0x1000001000100010),
  CONST64(0x1000100000100000), CONST64(0x1000101000100000), CONST64(0x1000100000100010), CONST64(0x1000101000100010),
  CONST64(0x1000000000101000), CONST64(0x1000001000101000), CONST64(0x1000000000101010), CONST64(0x1000001000101010),
  CONST64(0x1000100000101000), CONST64(0x1000101000101000), CONST64(0x1000100000101010), CONST64(0x1000101000101010),
  CONST64(0x1010000000100000), CONST64(0x1010001000100000), CONST64(0x1010000000100010), CONST64(0x1010001000100010),
  CONST64(0x1010100000100000), CONST64(0x1010101000100000), CONST64(0x1010100000100010), CONST64(0x1010101000100010),
  CONST64(0x1010000000101000), CONST64(0x1010001000101000), CONST64(0x1010000000101010), CONST64(0x1010001000101010),
  CONST64(0x1010100000101000), CONST64(0x1010101000101000), CONST64(0x1010100000101010), CONST64(0x1010101000101010),
  CONST64(0x0000000010000000), CONST64(0x0000001010000000), CONST64(0x0000000010000010), CONST64(0x0000001010000010),
  CONST64(0x0000100010000000), CONST64(0x0000101010000000), CONST64(0x0000100010000010), CONST64(0x0000101010000010),
  CONST64(0x0000000010001000), CONST64(0x0000001010001000), CONST64(0x0000000010001010), CONST64(0x0000001010001010),
  CONST64(0x0000100010001000), CONST64(0x0000101010001000), CONST64(0x0000100010001010), CONST64(0x0000101010001010),
  CONST64(0x0010000010000000), CONST64(0x0010001010000000), CONST64(0x0010000010000010), CONST64(0x0010001010000010),
  CONST64(0x0010100010000000), CONST64(0x0010101010000000), CONST64(0x0010100010000010), CONST64(0x0010101010000010),
  CONST64(0x0010000010001000), CONST64(0x0010001010001000), CONST64(0x0010000010001010), CONST64(0x0010001010001010),
  CONST64(0x0010100010001000), CONST64(0x0010101010001000), CONST64(0x0010100010001010), CONST64(0x0010101010001010),
  CONST64(0x0000000010100000), CONST64(0x0000001010100000), CONST64(0x0000000010100010), CONST64(0x0000001010100010),
  CONST64(0x0000100010100000), CONST64(0x0000101010100000), CONST64(0x0000100010100010), CONST64(0x0000101010100010),
  CONST64(0x0000000010101000), CONST64(0x0000001010101000), CONST64(0x0000000010101010), CONST64(0x0000001010101010),
  CONST64(0x0000100010101000), CONST64(0x0000101010101000), CONST64(0x0000100010101010), CONST64(0x0000101010101010),
  CONST64(0x0010000010100000), CONST64(0x0010001010100000), CONST64(0x0010000010100010), CONST64(0x0010001010100010),
  CONST64(0x0010100010100000), CONST64(0x0010101010100000), CONST64(0x0010100010100010), CONST64(0x0010101010100010),
  CONST64(0x0010000010101000), CONST64(0x0010001010101000), CONST64(0x0010000010101010), CONST64(0x0010001010101010),
  CONST64(0x0010100010101000), CONST64(0x0010101010101000), CONST64(0x0010100010101010), CONST64(0x0010101010101010),
  CONST64(0x1000000010000000), CONST64(0x1000001010000000), CONST64(0x1000000010000010), CONST64(0x1000001010000010),
  CONST64(0x1000100010000000), CONST64(0x1000101010000000), CONST64(0x1000100010000010), CONST64(0x1000101010000010),
  CONST64(0x1000000010001000), CONST64(0x1000001010001000), CONST64(0x1000000010001010), CONST64(0x1000001010001010),
  CONST64(0x1000100010001000), CONST64(0x1000101010001000), CONST64(0x1000100010001010), CONST64(0x1000101010001010),
  CONST64(0x1010000010000000), CONST64(0x1010001010000000), CONST64(0x1010000010000010), CONST64(0x1010001010000010),
  CONST64(0x1010100010000000), CONST64(0x1010101010000000), CONST64(0x1010100010000010), CONST64(0x1010101010000010),
  CONST64(0x1010000010001000), CONST64(0x1010001010001000), CONST64(0x1010000010001010), CONST64(0x1010001010001010),
  CONST64(0x1010100010001000), CONST64(0x1010101010001000), CONST64(0x1010100010001010), CONST64(0x1010101010001010),
  CONST64(0x1000000010100000), CONST64(0x1000001010100000), CONST64(0x1000000010100010), CONST64(0x1000001010100010),
  CONST64(0x1000100010100000), CONST64(0x1000101010100000), CONST64(0x1000100010100010), CONST64(0x1000101010100010),
  CONST64(0x1000000010101000), CONST64(0x1000001010101000), CONST64(0x1000000010101010), CONST64(0x1000001010101010),
  CONST64(0x1000100010101000), CONST64(0x1000101010101000), CONST64(0x1000100010101010), CONST64(0x1000101010101010),
  CONST64(0x1010000010100000), CONST64(0x1010001010100000), CONST64(0x1010000010100010), CONST64(0x1010001010100010),
  CONST64(0x1010100010100000), CONST64(0x1010101010100000), CONST64(0x1010100010100010), CONST64(0x1010101010100010),
  CONST64(0x1010000010101000), CONST64(0x1010001010101000), CONST64(0x1010000010101010), CONST64(0x1010001010101010),
  CONST64(0x1010100010101000), CONST64(0x1010101010101000), CONST64(0x1010100010101010), CONST64(0x1010101010101010)
  },
{ CONST64(0x0000000000000000), CONST64(0x0000000800000000), CONST64(0x0000000000000008), CONST64(0x0000000800000008),
  CONST64(0x0000080000000000), CONST64(0x0000080800000000), CONST64(0x0000080000000008), CONST64(0x0000080800000008),
  CONST64(0x0000000000000800), CONST64(0x0000000800000800), CONST64(0x0000000000000808), CONST64(0x0000000800000808),
  CONST64(0x0000080000000800), CONST64(0x0000080800000800), CONST64(0x0000080000000808), CONST64(0x0000080800000808),
  CONST64(0x0008000000000000), CONST64(0x0008000800000000), CONST64(0x0008000000000008), CONST64(0x0008000800000008),
  CONST64(0x0008080000000000), CONST64(0x0008080800000000), CONST64(0x0008080000000008), CONST64(0x0008080800000008),
  CONST64(0x0008000000000800), CONST64(0x0008000800000800), CONST64(0x0008000000000808), CONST64(0x0008000800000808),
  CONST64(0x0008080000000800), CONST64(0x0008080800000800), CONST64(0x0008080000000808), CONST64(0x0008080800000808),
  CONST64(0x0000000000080000), CONST64(0x0000000800080000), CONST64(0x0000000000080008), CONST64(0x0000000800080008),
  CONST64(0x0000080000080000), CONST64(0x0000080800080000), CONST64(0x0000080000080008), CONST64(0x0000080800080008),
  CONST64(0x0000000000080800), CONST64(0x0000000800080800), CONST64(0x0000000000080808), CONST64(0x0000000800080808),
  CONST64(0x0000080000080800), CONST64(0x0000080800080800), CONST64(0x0000080000080808), CONST64(0x0000080800080808),
  CONST64(0x0008000000080000), CONST64(0x0008000800080000), CONST64(0x0008000000080008), CONST64(0x0008000800080008),
  CONST64(0x0008080000080000), CONST64(0x0008080800080000), CONST64(0x0008080000080008), CONST64(0x0008080800080008),
  CONST64(0x0008000000080800), CONST64(0x0008000800080800), CONST64(0x0008000000080808), CONST64(0x0008000800080808),
  CONST64(0x0008080000080800), CONST64(0x0008080800080800), CONST64(0x0008080000080808), CONST64(0x0008080800080808),
  CONST64(0x0800000000000000), CONST64(0x0800000800000000), CONST64(0x0800000000000008), CONST64(0x0800000800000008),
  CONST64(0x0800080000000000), CONST64(0x0800080800000000), CONST64(0x0800080000000008), CONST64(0x0800080800000008),
  CONST64(0x0800000000000800), CONST64(0x0800000800000800), CONST64(0x0800000000000808), CONST64(0x0800000800000808),
  CONST64(0x0800080000000800), CONST64(0x0800080800000800), CONST64(0x0800080000000808), CONST64(0x0800080800000808),
  CONST64(0x0808000000000000), CONST64(0x0808000800000000), CONST64(0x0808000000000008), CONST64(0x0808000800000008),
  CONST64(0x0808080000000000), CONST64(0x0808080800000000), CONST64(0x0808080000000008), CONST64(0x0808080800000008),
  CONST64(0x0808000000000800), CONST64(0x0808000800000800), CONST64(0x0808000000000808), CONST64(0x0808000800000808),
  CONST64(0x0808080000000800), CONST64(0x0808080800000800), CONST64(0x0808080000000808), CONST64(0x0808080800000808),
  CONST64(0x0800000000080000), CONST64(0x0800000800080000), CONST64(0x0800000000080008), CONST64(0x0800000800080008),
  CONST64(0x0800080000080000), CONST64(0x0800080800080000), CONST64(0x0800080000080008), CONST64(0x0800080800080008),
  CONST64(0x0800000000080800), CONST64(0x0800000800080800), CONST64(0x0800000000080808), CONST64(0x0800000800080808),
  CONST64(0x0800080000080800), CONST64(0x0800080800080800), CONST64(0x0800080000080808), CONST64(0x0800080800080808),
  CONST64(0x0808000000080000), CONST64(0x0808000800080000), CONST64(0x0808000000080008), CONST64(0x0808000800080008),
  CONST64(0x0808080000080000), CONST64(0x0808080800080000), CONST64(0x0808080000080008), CONST64(0x0808080800080008),
  CONST64(0x0808000000080800), CONST64(0x0808000800080800), CONST64(0x0808000000080808), CONST64(0x0808000800080808),
  CONST64(0x0808080000080800), CONST64(0x0808080800080800), CONST64(0x0808080000080808), CONST64(0x0808080800080808),
  CONST64(0x0000000008000000), CONST64(0x0000000808000000), CONST64(0x0000000008000008), CONST64(0x0000000808000008),
  CONST64(0x0000080008000000), CONST64(0x0000080808000000), CONST64(0x0000080008000008), CONST64(0x0000080808000008),
  CONST64(0x0000000008000800), CONST64(0x0000000808000800), CONST64(0x0000000008000808), CONST64(0x0000000808000808),
  CONST64(0x0000080008000800), CONST64(0x0000080808000800), CONST64(0x0000080008000808), CONST64(0x0000080808000808),
  CONST64(0x0008000008000000), CONST64(0x0008000808000000), CONST64(0x0008000008000008), CONST64(0x0008000808000008),
  CONST64(0x0008080008000000), CONST64(0x0008080808000000), CONST64(0x0008080008000008), CONST64(0x0008080808000008),
  CONST64(0x0008000008000800), CONST64(0x0008000808000800), CONST64(0x0008000008000808), CONST64(0x0008000808000808),
  CONST64(0x0008080008000800), CONST64(0x0008080808000800), CONST64(0x0008080008000808), CONST64(0x0008080808000808),
  CONST64(0x0000000008080000), CONST64(0x0000000808080000), CONST64(0x0000000008080008), CONST64(0x0000000808080008),
  CONST64(0x0000080008080000), CONST64(0x0000080808080000), CONST64(0x0000080008080008), CONST64(0x0000080808080008),
  CONST64(0x0000000008080800), CONST64(0x0000000808080800), CONST64(0x0000000008080808), CONST64(0x0000000808080808),
  CONST64(0x0000080008080800), CONST64(0x0000080808080800), CONST64(0x0000080008080808), CONST64(0x0000080808080808),
  CONST64(0x0008000008080000), CONST64(0x0008000808080000), CONST64(0x0008000008080008), CONST64(0x0008000808080008),
  CONST64(0x0008080008080000), CONST64(0x0008080808080000), CONST64(0x0008080008080008), CONST64(0x0008080808080008),
  CONST64(0x0008000008080800), CONST64(0x0008000808080800), CONST64(0x0008000008080808), CONST64(0x0008000808080808),
  CONST64(0x0008080008080800), CONST64(0x0008080808080800), CONST64(0x0008080008080808), CONST64(0x0008080808080808),
  CONST64(0x0800000008000000), CONST64(0x0800000808000000), CONST64(0x0800000008000008), CONST64(0x0800000808000008),
  CONST64(0x0800080008000000), CONST64(0x0800080808000000), CONST64(0x0800080008000008), CONST64(0x0800080808000008),
  CONST64(0x0800000008000800), CONST64(0x0800000808000800), CONST64(0x0800000008000808), CONST64(0x0800000808000808),
  CONST64(0x0800080008000800), CONST64(0x0800080808000800), CONST64(0x0800080008000808), CONST64(0x0800080808000808),
  CONST64(0x0808000008000000), CONST64(0x0808000808000000), CONST64(0x0808000008000008), CONST64(0x0808000808000008),
  CONST64(0x0808080008000000), CONST64(0x0808080808000000), CONST64(0x0808080008000008), CONST64(0x0808080808000008),
  CONST64(0x0808000008000800), CONST64(0x0808000808000800), CONST64(0x0808000008000808), CONST64(0x0808000808000808),
  CONST64(0x0808080008000800), CONST64(0x0808080808000800), CONST64(0x0808080008000808), CONST64(0x0808080808000808),
  CONST64(0x0800000008080000), CONST64(0x0800000808080000), CONST64(0x0800000008080008), CONST64(0x0800000808080008),
  CONST64(0x0800080008080000), CONST64(0x0800080808080000), CONST64(0x0800080008080008), CONST64(0x0800080808080008),
  CONST64(0x0800000008080800), CONST64(0x0800000808080800), CONST64(0x0800000008080808), CONST64(0x0800000808080808),
  CONST64(0x0800080008080800), CONST64(0x0800080808080800), CONST64(0x0800080008080808), CONST64(0x0800080808080808),
  CONST64(0x0808000008080000), CONST64(0x0808000808080000), CONST64(0x0808000008080008), CONST64(0x0808000808080008),
  CONST64(0x0808080008080000), CONST64(0x0808080808080000), CONST64(0x0808080008080008), CONST64(0x0808080808080008),
  CONST64(0x0808000008080800), CONST64(0x0808000808080800), CONST64(0x0808000008080808), CONST64(0x0808000808080808),
  CONST64(0x0808080008080800), CONST64(0x0808080808080800), CONST64(0x0808080008080808), CONST64(0x0808080808080808)
  },
{ CONST64(0x0000000000000000), CONST64(0x0000000400000000), CONST64(0x0000000000000004), CONST64(0x0000000400000004),
  CONST64(0x0000040000000000), CONST64(0x0000040400000000), CONST64(0x0000040000000004), CONST64(0x0000040400000004),
  CONST64(0x0000000000000400), CONST64(0x0000000400000400), CONST64(0x0000000000000404), CONST64(0x0000000400000404),
  CONST64(0x0000040000000400), CONST64(0x0000040400000400), CONST64(0x0000040000000404), CONST64(0x0000040400000404),
  CONST64(0x0004000000000000), CONST64(0x0004000400000000), CONST64(0x0004000000000004), CONST64(0x0004000400000004),
  CONST64(0x0004040000000000), CONST64(0x0004040400000000), CONST64(0x0004040000000004), CONST64(0x0004040400000004),
  CONST64(0x0004000000000400), CONST64(0x0004000400000400), CONST64(0x0004000000000404), CONST64(0x0004000400000404),
  CONST64(0x0004040000000400), CONST64(0x0004040400000400), CONST64(0x0004040000000404), CONST64(0x0004040400000404),
  CONST64(0x0000000000040000), CONST64(0x0000000400040000), CONST64(0x0000000000040004), CONST64(0x0000000400040004),
  CONST64(0x0000040000040000), CONST64(0x0000040400040000), CONST64(0x0000040000040004), CONST64(0x0000040400040004),
  CONST64(0x0000000000040400), CONST64(0x0000000400040400), CONST64(0x0000000000040404), CONST64(0x0000000400040404),
  CONST64(0x0000040000040400), CONST64(0x0000040400040400), CONST64(0x0000040000040404), CONST64(0x0000040400040404),
  CONST64(0x0004000000040000), CONST64(0x0004000400040000), CONST64(0x0004000000040004), CONST64(0x0004000400040004),
  CONST64(0x0004040000040000), CONST64(0x0004040400040000), CONST64(0x0004040000040004), CONST64(0x0004040400040004),
  CONST64(0x0004000000040400), CONST64(0x0004000400040400), CONST64(0x0004000000040404), CONST64(0x0004000400040404),
  CONST64(0x0004040000040400), CONST64(0x0004040400040400), CONST64(0x0004040000040404), CONST64(0x0004040400040404),
  CONST64(0x0400000000000000), CONST64(0x0400000400000000), CONST64(0x0400000000000004), CONST64(0x0400000400000004),
  CONST64(0x0400040000000000), CONST64(0x0400040400000000), CONST64(0x0400040000000004), CONST64(0x0400040400000004),
  CONST64(0x0400000000000400), CONST64(0x0400000400000400), CONST64(0x0400000000000404), CONST64(0x0400000400000404),
  CONST64(0x0400040000000400), CONST64(0x0400040400000400), CONST64(0x0400040000000404), CONST64(0x0400040400000404),
  CONST64(0x0404000000000000), CONST64(0x0404000400000000), CONST64(0x0404000000000004), CONST64(0x0404000400000004),
  CONST64(0x0404040000000000), CONST64(0x0404040400000000), CONST64(0x0404040000000004), CONST64(0x0404040400000004),
  CONST64(0x0404000000000400), CONST64(0x0404000400000400), CONST64(0x0404000000000404), CONST64(0x0404000400000404),
  CONST64(0x0404040000000400), CONST64(0x0404040400000400), CONST64(0x0404040000000404), CONST64(0x0404040400000404),
  CONST64(0x0400000000040000), CONST64(0x0400000400040000), CONST64(0x0400000000040004), CONST64(0x0400000400040004),
  CONST64(0x0400040000040000), CONST64(0x0400040400040000), CONST64(0x0400040000040004), CONST64(0x0400040400040004),
  CONST64(0x0400000000040400), CONST64(0x0400000400040400), CONST64(0x0400000000040404), CONST64(0x0400000400040404),
  CONST64(0x0400040000040400), CONST64(0x0400040400040400), CONST64(0x0400040000040404), CONST64(0x0400040400040404),
  CONST64(0x0404000000040000), CONST64(0x0404000400040000), CONST64(0x0404000000040004), CONST64(0x0404000400040004),
  CONST64(0x0404040000040000), CONST64(0x0404040400040000), CONST64(0x0404040000040004), CONST64(0x0404040400040004),
  CONST64(0x0404000000040400), CONST64(0x0404000400040400), CONST64(0x0404000000040404), CONST64(0x0404000400040404),
  CONST64(0x0404040000040400), CONST64(0x0404040400040400), CONST64(0x0404040000040404), CONST64(0x0404040400040404),
  CONST64(0x0000000004000000), CONST64(0x0000000404000000), CONST64(0x0000000004000004), CONST64(0x0000000404000004),
  CONST64(0x0000040004000000), CONST64(0x0000040404000000), CONST64(0x0000040004000004), CONST64(0x0000040404000004),
  CONST64(0x0000000004000400), CONST64(0x0000000404000400), CONST64(0x0000000004000404), CONST64(0x0000000404000404),
  CONST64(0x0000040004000400), CONST64(0x0000040404000400), CONST64(0x0000040004000404), CONST64(0x0000040404000404),
  CONST64(0x0004000004000000), CONST64(0x0004000404000000), CONST64(0x0004000004000004), CONST64(0x0004000404000004),
  CONST64(0x0004040004000000), CONST64(0x0004040404000000), CONST64(0x0004040004000004), CONST64(0x0004040404000004),
  CONST64(0x0004000004000400), CONST64(0x0004000404000400), CONST64(0x0004000004000404), CONST64(0x0004000404000404),
  CONST64(0x0004040004000400), CONST64(0x0004040404000400), CONST64(0x0004040004000404), CONST64(0x0004040404000404),
  CONST64(0x0000000004040000), CONST64(0x0000000404040000), CONST64(0x0000000004040004), CONST64(0x0000000404040004),
  CONST64(0x0000040004040000), CONST64(0x0000040404040000), CONST64(0x0000040004040004), CONST64(0x0000040404040004),
  CONST64(0x0000000004040400), CONST64(0x0000000404040400), CONST64(0x0000000004040404), CONST64(0x0000000404040404),
  CONST64(0x0000040004040400), CONST64(0x0000040404040400), CONST64(0x0000040004040404), CONST64(0x0000040404040404),
  CONST64(0x0004000004040000), CONST64(0x0004000404040000), CONST64(0x0004000004040004), CONST64(0x0004000404040004),
  CONST64(0x0004040004040000), CONST64(0x0004040404040000), CONST64(0x0004040004040004), CONST64(0x0004040404040004),
  CONST64(0x0004000004040400), CONST64(0x0004000404040400), CONST64(0x0004000004040404), CONST64(0x0004000404040404),
  CONST64(0x0004040004040400), CONST64(0x0004040404040400), CONST64(0x0004040004040404), CONST64(0x0004040404040404),
  CONST64(0x0400000004000000), CONST64(0x0400000404000000), CONST64(0x0400000004000004), CONST64(0x0400000404000004),
  CONST64(0x0400040004000000), CONST64(0x0400040404000000), CONST64(0x0400040004000004), CONST64(0x0400040404000004),
  CONST64(0x0400000004000400), CONST64(0x0400000404000400), CONST64(0x0400000004000404), CONST64(0x0400000404000404),
  CONST64(0x0400040004000400), CONST64(0x0400040404000400), CONST64(0x0400040004000404), CONST64(0x0400040404000404),
  CONST64(0x0404000004000000), CONST64(0x0404000404000000), CONST64(0x0404000004000004), CONST64(0x0404000404000004),
  CONST64(0x0404040004000000), CONST64(0x0404040404000000), CONST64(0x0404040004000004), CONST64(0x0404040404000004),
  CONST64(0x0404000004000400), CONST64(0x0404000404000400), CONST64(0x0404000004000404), CONST64(0x0404000404000404),
  CONST64(0x0404040004000400), CONST64(0x0404040404000400), CONST64(0x0404040004000404), CONST64(0x0404040404000404),
  CONST64(0x0400000004040000), CONST64(0x0400000404040000), CONST64(0x0400000004040004), CONST64(0x0400000404040004),
  CONST64(0x0400040004040000), CONST64(0x0400040404040000), CONST64(0x0400040004040004), CONST64(0x0400040404040004),
  CONST64(0x0400000004040400), CONST64(0x0400000404040400), CONST64(0x0400000004040404), CONST64(0x0400000404040404),
  CONST64(0x0400040004040400), CONST64(0x0400040404040400), CONST64(0x0400040004040404), CONST64(0x0400040404040404),
  CONST64(0x0404000004040000), CONST64(0x0404000404040000), CONST64(0x0404000004040004), CONST64(0x0404000404040004),
  CONST64(0x0404040004040000), CONST64(0x0404040404040000), CONST64(0x0404040004040004), CONST64(0x0404040404040004),
  CONST64(0x0404000004040400), CONST64(0x0404000404040400), CONST64(0x0404000004040404), CONST64(0x0404000404040404),
  CONST64(0x0404040004040400), CONST64(0x0404040404040400), CONST64(0x0404040004040404), CONST64(0x0404040404040404)
  },
{ CONST64(0x0000000000000000), CONST64(0x0000000200000000), CONST64(0x0000000000000002), CONST64(0x0000000200000002),
  CONST64(0x0000020000000000), CONST64(0x0000020200000000), CONST64(0x0000020000000002), CONST64(0x0000020200000002),
  CONST64(0x0000000000000200), CONST64(0x0000000200000200), CONST64(0x0000000000000202), CONST64(0x0000000200000202),
  CONST64(0x0000020000000200), CONST64(0x0000020200000200), CONST64(0x0000020000000202), CONST64(0x0000020200000202),
  CONST64(0x0002000000000000), CONST64(0x0002000200000000), CONST64(0x0002000000000002), CONST64(0x0002000200000002),
  CONST64(0x0002020000000000), CONST64(0x0002020200000000), CONST64(0x0002020000000002), CONST64(0x0002020200000002),
  CONST64(0x0002000000000200), CONST64(0x0002000200000200), CONST64(0x0002000000000202), CONST64(0x0002000200000202),
  CONST64(0x0002020000000200), CONST64(0x0002020200000200), CONST64(0x0002020000000202), CONST64(0x0002020200000202),
  CONST64(0x0000000000020000), CONST64(0x0000000200020000), CONST64(0x0000000000020002), CONST64(0x0000000200020002),
  CONST64(0x0000020000020000), CONST64(0x0000020200020000), CONST64(0x0000020000020002), CONST64(0x0000020200020002),
  CONST64(0x0000000000020200), CONST64(0x0000000200020200), CONST64(0x0000000000020202), CONST64(0x0000000200020202),
  CONST64(0x0000020000020200), CONST64(0x0000020200020200), CONST64(0x0000020000020202), CONST64(0x0000020200020202),
  CONST64(0x0002000000020000), CONST64(0x0002000200020000), CONST64(0x0002000000020002), CONST64(0x0002000200020002),
  CONST64(0x0002020000020000), CONST64(0x0002020200020000), CONST64(0x0002020000020002), CONST64(0x0002020200020002),
  CONST64(0x0002000000020200), CONST64(0x0002000200020200), CONST64(0x0002000000020202), CONST64(0x0002000200020202),
  CONST64(0x0002020000020200), CONST64(0x0002020200020200), CONST64(0x0002020000020202), CONST64(0x0002020200020202),
  CONST64(0x0200000000000000), CONST64(0x0200000200000000), CONST64(0x0200000000000002), CONST64(0x0200000200000002),
  CONST64(0x0200020000000000), CONST64(0x0200020200000000), CONST64(0x0200020000000002), CONST64(0x0200020200000002),
  CONST64(0x0200000000000200), CONST64(0x0200000200000200), CONST64(0x0200000000000202), CONST64(0x0200000200000202),
  CONST64(0x0200020000000200), CONST64(0x0200020200000200), CONST64(0x0200020000000202), CONST64(0x0200020200000202),
  CONST64(0x0202000000000000), CONST64(0x0202000200000000), CONST64(0x0202000000000002), CONST64(0x0202000200000002),
  CONST64(0x0202020000000000), CONST64(0x0202020200000000), CONST64(0x0202020000000002), CONST64(0x0202020200000002),
  CONST64(0x0202000000000200), CONST64(0x0202000200000200), CONST64(0x0202000000000202), CONST64(0x0202000200000202),
  CONST64(0x0202020000000200), CONST64(0x0202020200000200), CONST64(0x0202020000000202), CONST64(0x0202020200000202),
  CONST64(0x0200000000020000), CONST64(0x0200000200020000), CONST64(0x0200000000020002), CONST64(0x0200000200020002),
  CONST64(0x0200020000020000), CONST64(0x0200020200020000), CONST64(0x0200020000020002), CONST64(0x0200020200020002),
  CONST64(0x0200000000020200), CONST64(0x0200000200020200), CONST64(0x0200000000020202), CONST64(0x0200000200020202),
  CONST64(0x0200020000020200), CONST64(0x0200020200020200), CONST64(0x0200020000020202), CONST64(0x0200020200020202),
  CONST64(0x0202000000020000), CONST64(0x0202000200020000), CONST64(0x0202000000020002), CONST64(0x0202000200020002),
  CONST64(0x0202020000020000), CONST64(0x0202020200020000), CONST64(0x0202020000020002), CONST64(0x0202020200020002),
  CONST64(0x0202000000020200), CONST64(0x0202000200020200), CONST64(0x0202000000020202), CONST64(0x0202000200020202),
  CONST64(0x0202020000020200), CONST64(0x0202020200020200), CONST64(0x0202020000020202), CONST64(0x0202020200020202),
  CONST64(0x0000000002000000), CONST64(0x0000000202000000), CONST64(0x0000000002000002), CONST64(0x0000000202000002),
  CONST64(0x0000020002000000), CONST64(0x0000020202000000), CONST64(0x0000020002000002), CONST64(0x0000020202000002),
  CONST64(0x0000000002000200), CONST64(0x0000000202000200), CONST64(0x0000000002000202), CONST64(0x0000000202000202),
  CONST64(0x0000020002000200), CONST64(0x0000020202000200), CONST64(0x0000020002000202), CONST64(0x0000020202000202),
  CONST64(0x0002000002000000), CONST64(0x0002000202000000), CONST64(0x0002000002000002), CONST64(0x0002000202000002),
  CONST64(0x0002020002000000), CONST64(0x0002020202000000), CONST64(0x0002020002000002), CONST64(0x0002020202000002),
  CONST64(0x0002000002000200), CONST64(0x0002000202000200), CONST64(0x0002000002000202), CONST64(0x0002000202000202),
  CONST64(0x0002020002000200), CONST64(0x0002020202000200), CONST64(0x0002020002000202), CONST64(0x0002020202000202),
  CONST64(0x0000000002020000), CONST64(0x0000000202020000), CONST64(0x0000000002020002), CONST64(0x0000000202020002),
  CONST64(0x0000020002020000), CONST64(0x0000020202020000), CONST64(0x0000020002020002), CONST64(0x0000020202020002),
  CONST64(0x0000000002020200), CONST64(0x0000000202020200), CONST64(0x0000000002020202), CONST64(0x0000000202020202),
  CONST64(0x0000020002020200), CONST64(0x0000020202020200), CONST64(0x0000020002020202), CONST64(0x0000020202020202),
  CONST64(0x0002000002020000), CONST64(0x0002000202020000), CONST64(0x0002000002020002), CONST64(0x0002000202020002),
  CONST64(0x0002020002020000), CONST64(0x0002020202020000), CONST64(0x0002020002020002), CONST64(0x0002020202020002),
  CONST64(0x0002000002020200), CONST64(0x0002000202020200), CONST64(0x0002000002020202), CONST64(0x0002000202020202),
  CONST64(0x0002020002020200), CONST64(0x0002020202020200), CONST64(0x0002020002020202), CONST64(0x0002020202020202),
  CONST64(0x0200000002000000), CONST64(0x0200000202000000), CONST64(0x0200000002000002), CONST64(0x0200000202000002),
  CONST64(0x0200020002000000), CONST64(0x0200020202000000), CONST64(0x0200020002000002), CONST64(0x0200020202000002),
  CONST64(0x0200000002000200), CONST64(0x0200000202000200), CONST64(0x0200000002000202), CONST64(0x0200000202000202),
  CONST64(0x0200020002000200), CONST64(0x0200020202000200), CONST64(0x0200020002000202), CONST64(0x0200020202000202),
  CONST64(0x0202000002000000), CONST64(0x0202000202000000), CONST64(0x0202000002000002), CONST64(0x0202000202000002),
  CONST64(0x0202020002000000), CONST64(0x0202020202000000), CONST64(0x0202020002000002), CONST64(0x0202020202000002),
  CONST64(0x0202000002000200), CONST64(0x0202000202000200), CONST64(0x0202000002000202), CONST64(0x0202000202000202),
  CONST64(0x0202020002000200), CONST64(0x0202020202000200), CONST64(0x0202020002000202), CONST64(0x0202020202000202),
  CONST64(0x0200000002020000), CONST64(0x0200000202020000), CONST64(0x0200000002020002), CONST64(0x0200000202020002),
  CONST64(0x0200020002020000), CONST64(0x0200020202020000), CONST64(0x0200020002020002), CONST64(0x0200020202020002),
  CONST64(0x0200000002020200), CONST64(0x0200000202020200), CONST64(0x0200000002020202), CONST64(0x0200000202020202),
  CONST64(0x0200020002020200), CONST64(0x0200020202020200), CONST64(0x0200020002020202), CONST64(0x0200020202020202),
  CONST64(0x0202000002020000), CONST64(0x0202000202020000), CONST64(0x0202000002020002), CONST64(0x0202000202020002),
  CONST64(0x0202020002020000), CONST64(0x0202020202020000), CONST64(0x0202020002020002), CONST64(0x0202020202020002),
  CONST64(0x0202000002020200), CONST64(0x0202000202020200), CONST64(0x0202000002020202), CONST64(0x0202000202020202),
  CONST64(0x0202020002020200), CONST64(0x0202020202020200), CONST64(0x0202020002020202), CONST64(0x0202020202020202)
  },
{ CONST64(0x0000000000000000), CONST64(0x0000010000000000), CONST64(0x0000000000000100), CONST64(0x0000010000000100),
  CONST64(0x0001000000000000), CONST64(0x0001010000000000), CONST64(0x0001000000000100), CONST64(0x0001010000000100),
  CONST64(0x0000000000010000), CONST64(0x0000010000010000), CONST64(0x0000000000010100), CONST64(0x0000010000010100),
  CONST64(0x0001000000010000), CONST64(0x0001010000010000), CONST64(0x0001000000010100), CONST64(0x0001010000010100),
  CONST64(0x0100000000000000), CONST64(0x0100010000000000), CONST64(0x0100000000000100), CONST64(0x0100010000000100),
  CONST64(0x0101000000000000), CONST64(0x0101010000000000), CONST64(0x0101000000000100), CONST64(0x0101010000000100),
  CONST64(0x0100000000010000), CONST64(0x0100010000010000), CONST64(0x0100000000010100), CONST64(0x0100010000010100),
  CONST64(0x0101000000010000), CONST64(0x0101010000010000), CONST64(0x0101000000010100), CONST64(0x0101010000010100),
  CONST64(0x0000000001000000), CONST64(0x0000010001000000), CONST64(0x0000000001000100), CONST64(0x0000010001000100),
  CONST64(0x0001000001000000), CONST64(0x0001010001000000), CONST64(0x0001000001000100), CONST64(0x0001010001000100),
  CONST64(0x0000000001010000), CONST64(0x0000010001010000), CONST64(0x0000000001010100), CONST64(0x0000010001010100),
  CONST64(0x0001000001010000), CONST64(0x0001010001010000), CONST64(0x0001000001010100), CONST64(0x0001010001010100),
  CONST64(0x0100000001000000), CONST64(0x0100010001000000), CONST64(0x0100000001000100), CONST64(0x0100010001000100),
  CONST64(0x0101000001000000), CONST64(0x0101010001000000), CONST64(0x0101000001000100), CONST64(0x0101010001000100),
  CONST64(0x0100000001010000), CONST64(0x0100010001010000), CONST64(0x0100000001010100), CONST64(0x0100010001010100),
  CONST64(0x0101000001010000), CONST64(0x0101010001010000), CONST64(0x0101000001010100), CONST64(0x0101010001010100),
  CONST64(0x0000000100000000), CONST64(0x0000010100000000), CONST64(0x0000000100000100), CONST64(0x0000010100000100),
  CONST64(0x0001000100000000), CONST64(0x0001010100000000), CONST64(0x0001000100000100), CONST64(0x0001010100000100),
  CONST64(0x0000000100010000), CONST64(0x0000010100010000), CONST64(0x0000000100010100), CONST64(0x0000010100010100),
  CONST64(0x0001000100010000), CONST64(0x0001010100010000), CONST64(0x0001000100010100), CONST64(0x0001010100010100),
  CONST64(0x0100000100000000), CONST64(0x0100010100000000), CONST64(0x0100000100000100), CONST64(0x0100010100000100),
  CONST64(0x0101000100000000), CONST64(0x0101010100000000), CONST64(0x0101000100000100), CONST64(0x0101010100000100),
  CONST64(0x0100000100010000), CONST64(0x0100010100010000), CONST64(0x0100000100010100), CONST64(0x0100010100010100),
  CONST64(0x0101000100010000), CONST64(0x0101010100010000), CONST64(0x0101000100010100), CONST64(0x0101010100010100),
  CONST64(0x0000000101000000), CONST64(0x0000010101000000), CONST64(0x0000000101000100), CONST64(0x0000010101000100),
  CONST64(0x0001000101000000), CONST64(0x0001010101000000), CONST64(0x0001000101000100), CONST64(0x0001010101000100),
  CONST64(0x0000000101010000), CONST64(0x0000010101010000), CONST64(0x0000000101010100), CONST64(0x0000010101010100),
  CONST64(0x0001000101010000), CONST64(0x0001010101010000), CONST64(0x0001000101010100), CONST64(0x0001010101010100),
  CONST64(0x0100000101000000), CONST64(0x0100010101000000), CONST64(0x0100000101000100), CONST64(0x0100010101000100),
  CONST64(0x0101000101000000), CONST64(0x0101010101000000), CONST64(0x0101000101000100), CONST64(0x0101010101000100),
  CONST64(0x0100000101010000), CONST64(0x0100010101010000), CONST64(0x0100000101010100), CONST64(0x0100010101010100),
  CONST64(0x0101000101010000), CONST64(0x0101010101010000), CONST64(0x0101000101010100), CONST64(0x0101010101010100),
  CONST64(0x0000000000000001), CONST64(0x0000010000000001), CONST64(0x0000000000000101), CONST64(0x0000010000000101),
  CONST64(0x0001000000000001), CONST64(0x0001010000000001), CONST64(0x0001000000000101), CONST64(0x0001010000000101),
  CONST64(0x0000000000010001), CONST64(0x0000010000010001), CONST64(0x0000000000010101), CONST64(0x0000010000010101),
  CONST64(0x0001000000010001), CONST64(0x0001010000010001), CONST64(0x0001000000010101), CONST64(0x0001010000010101),
  CONST64(0x0100000000000001), CONST64(0x0100010000000001), CONST64(0x0100000000000101), CONST64(0x0100010000000101),
  CONST64(0x0101000000000001), CONST64(0x0101010000000001), CONST64(0x0101000000000101), CONST64(0x0101010000000101),
  CONST64(0x0100000000010001), CONST64(0x0100010000010001), CONST64(0x0100000000010101), CONST64(0x0100010000010101),
  CONST64(0x0101000000010001), CONST64(0x0101010000010001), CONST64(0x0101000000010101), CONST64(0x0101010000010101),
  CONST64(0x0000000001000001), CONST64(0x0000010001000001), CONST64(0x0000000001000101), CONST64(0x0000010001000101),
  CONST64(0x0001000001000001), CONST64(0x0001010001000001), CONST64(0x0001000001000101), CONST64(0x0001010001000101),
  CONST64(0x0000000001010001), CONST64(0x0000010001010001), CONST64(0x0000000001010101), CONST64(0x0000010001010101),
  CONST64(0x0001000001010001), CONST64(0x0001010001010001), CONST64(0x0001000001010101), CONST64(0x0001010001010101),
  CONST64(0x0100000001000001), CONST64(0x0100010001000001), CONST64(0x0100000001000101), CONST64(0x0100010001000101),
  CONST64(0x0101000001000001), CONST64(0x0101010001000001), CONST64(0x0101000001000101), CONST64(0x0101010001000101),
  CONST64(0x0100000001010001), CONST64(0x0100010001010001), CONST64(0x0100000001010101), CONST64(0x0100010001010101),
  CONST64(0x0101000001010001), CONST64(0x0101010001010001), CONST64(0x0101000001010101), CONST64(0x0101010001010101),
  CONST64(0x0000000100000001), CONST64(0x0000010100000001), CONST64(0x0000000100000101), CONST64(0x0000010100000101),
  CONST64(0x0001000100000001), CONST64(0x0001010100000001), CONST64(0x0001000100000101), CONST64(0x0001010100000101),
  CONST64(0x0000000100010001), CONST64(0x0000010100010001), CONST64(0x0000000100010101), CONST64(0x0000010100010101),
  CONST64(0x0001000100010001), CONST64(0x0001010100010001), CONST64(0x0001000100010101), CONST64(0x0001010100010101),
  CONST64(0x0100000100000001), CONST64(0x0100010100000001), CONST64(0x0100000100000101), CONST64(0x0100010100000101),
  CONST64(0x0101000100000001), CONST64(0x0101010100000001), CONST64(0x0101000100000101), CONST64(0x0101010100000101),
  CONST64(0x0100000100010001), CONST64(0x0100010100010001), CONST64(0x0100000100010101), CONST64(0x0100010100010101),
  CONST64(0x0101000100010001), CONST64(0x0101010100010001), CONST64(0x0101000100010101), CONST64(0x0101010100010101),
  CONST64(0x0000000101000001), CONST64(0x0000010101000001), CONST64(0x0000000101000101), CONST64(0x0000010101000101),
  CONST64(0x0001000101000001), CONST64(0x0001010101000001), CONST64(0x0001000101000101), CONST64(0x0001010101000101),
  CONST64(0x0000000101010001), CONST64(0x0000010101010001), CONST64(0x0000000101010101), CONST64(0x0000010101010101),
  CONST64(0x0001000101010001), CONST64(0x0001010101010001), CONST64(0x0001000101010101), CONST64(0x0001010101010101),
  CONST64(0x0100000101000001), CONST64(0x0100010101000001), CONST64(0x0100000101000101), CONST64(0x0100010101000101),
  CONST64(0x0101000101000001), CONST64(0x0101010101000001), CONST64(0x0101000101000101), CONST64(0x0101010101000101),
  CONST64(0x0100000101010001), CONST64(0x0100010101010001), CONST64(0x0100000101010101), CONST64(0x0100010101010101),
  CONST64(0x0101000101010001), CONST64(0x0101010101010001), CONST64(0x0101000101010101), CONST64(0x0101010101010101)
  },
{ CONST64(0x0000000000000000), CONST64(0x0000008000000000), CONST64(0x0000000000000080), CONST64(0x0000008000000080),
  CONST64(0x0000800000000000), CONST64(0x0000808000000000), CONST64(0x0000800000000080), CONST64(0x0000808000000080),
  CONST64(0x0000000000008000), CONST64(0x0000008000008000), CONST64(0x0000000000008080), CONST64(0x0000008000008080),
  CONST64(0x0000800000008000), CONST64(0x0000808000008000), CONST64(0x0000800000008080), CONST64(0x0000808000008080),
  CONST64(0x0080000000000000), CONST64(0x0080008000000000), CONST64(0x0080000000000080), CONST64(0x0080008000000080),
  CONST64(0x0080800000000000), CONST64(0x0080808000000000), CONST64(0x0080800000000080), CONST64(0x0080808000000080),
  CONST64(0x0080000000008000), CONST64(0x0080008000008000), CONST64(0x0080000000008080), CONST64(0x0080008000008080),
  CONST64(0x0080800000008000), CONST64(0x0080808000008000), CONST64(0x0080800000008080), CONST64(0x0080808000008080),
  CONST64(0x0000000000800000), CONST64(0x0000008000800000), CONST64(0x0000000000800080), CONST64(0x0000008000800080),
  CONST64(0x0000800000800000), CONST64(0x0000808000800000), CONST64(0x0000800000800080), CONST64(0x0000808000800080),
  CONST64(0x0000000000808000), CONST64(0x0000008000808000), CONST64(0x0000000000808080), CONST64(0x0000008000808080),
  CONST64(0x0000800000808000), CONST64(0x0000808000808000), CONST64(0x0000800000808080), CONST64(0x0000808000808080),
  CONST64(0x0080000000800000), CONST64(0x0080008000800000), CONST64(0x0080000000800080), CONST64(0x0080008000800080),
  CONST64(0x0080800000800000), CONST64(0x0080808000800000), CONST64(0x0080800000800080), CONST64(0x0080808000800080),
  CONST64(0x0080000000808000), CONST64(0x0080008000808000), CONST64(0x0080000000808080), CONST64(0x0080008000808080),
  CONST64(0x0080800000808000), CONST64(0x0080808000808000), CONST64(0x0080800000808080), CONST64(0x0080808000808080),
  CONST64(0x8000000000000000), CONST64(0x8000008000000000), CONST64(0x8000000000000080), CONST64(0x8000008000000080),
  CONST64(0x8000800000000000), CONST64(0x8000808000000000), CONST64(0x8000800000000080), CONST64(0x8000808000000080),
  CONST64(0x8000000000008000), CONST64(0x8000008000008000), CONST64(0x8000000000008080), CONST64(0x8000008000008080),
  CONST64(0x8000800000008000), CONST64(0x8000808000008000), CONST64(0x8000800000008080), CONST64(0x8000808000008080),
  CONST64(0x8080000000000000), CONST64(0x8080008000000000), CONST64(0x8080000000000080), CONST64(0x8080008000000080),
  CONST64(0x8080800000000000), CONST64(0x8080808000000000), CONST64(0x8080800000000080), CONST64(0x8080808000000080),
  CONST64(0x8080000000008000), CONST64(0x8080008000008000), CONST64(0x8080000000008080), CONST64(0x8080008000008080),
  CONST64(0x8080800000008000), CONST64(0x8080808000008000), CONST64(0x8080800000008080), CONST64(0x8080808000008080),
  CONST64(0x8000000000800000), CONST64(0x8000008000800000), CONST64(0x8000000000800080), CONST64(0x8000008000800080),
  CONST64(0x8000800000800000), CONST64(0x8000808000800000), CONST64(0x8000800000800080), CONST64(0x8000808000800080),
  CONST64(0x8000000000808000), CONST64(0x8000008000808000), CONST64(0x8000000000808080), CONST64(0x8000008000808080),
  CONST64(0x8000800000808000), CONST64(0x8000808000808000), CONST64(0x8000800000808080), CONST64(0x8000808000808080),
  CONST64(0x8080000000800000), CONST64(0x8080008000800000), CONST64(0x8080000000800080), CONST64(0x8080008000800080),
  CONST64(0x8080800000800000), CONST64(0x8080808000800000), CONST64(0x8080800000800080), CONST64(0x8080808000800080),
  CONST64(0x8080000000808000), CONST64(0x8080008000808000), CONST64(0x8080000000808080), CONST64(0x8080008000808080),
  CONST64(0x8080800000808000), CONST64(0x8080808000808000), CONST64(0x8080800000808080), CONST64(0x8080808000808080),
  CONST64(0x0000000080000000), CONST64(0x0000008080000000), CONST64(0x0000000080000080), CONST64(0x0000008080000080),
  CONST64(0x0000800080000000), CONST64(0x0000808080000000), CONST64(0x0000800080000080), CONST64(0x0000808080000080),
  CONST64(0x0000000080008000), CONST64(0x0000008080008000), CONST64(0x0000000080008080), CONST64(0x0000008080008080),
  CONST64(0x0000800080008000), CONST64(0x0000808080008000), CONST64(0x0000800080008080), CONST64(0x0000808080008080),
  CONST64(0x0080000080000000), CONST64(0x0080008080000000), CONST64(0x0080000080000080), CONST64(0x0080008080000080),
  CONST64(0x0080800080000000), CONST64(0x0080808080000000), CONST64(0x0080800080000080), CONST64(0x0080808080000080),
  CONST64(0x0080000080008000), CONST64(0x0080008080008000), CONST64(0x0080000080008080), CONST64(0x0080008080008080),
  CONST64(0x0080800080008000), CONST64(0x0080808080008000), CONST64(0x0080800080008080), CONST64(0x0080808080008080),
  CONST64(0x0000000080800000), CONST64(0x0000008080800000), CONST64(0x0000000080800080), CONST64(0x0000008080800080),
  CONST64(0x0000800080800000), CONST64(0x0000808080800000), CONST64(0x0000800080800080), CONST64(0x0000808080800080),
  CONST64(0x0000000080808000), CONST64(0x0000008080808000), CONST64(0x0000000080808080), CONST64(0x0000008080808080),
  CONST64(0x0000800080808000), CONST64(0x0000808080808000), CONST64(0x0000800080808080), CONST64(0x0000808080808080),
  CONST64(0x0080000080800000), CONST64(0x0080008080800000), CONST64(0x0080000080800080), CONST64(0x0080008080800080),
  CONST64(0x0080800080800000), CONST64(0x0080808080800000), CONST64(0x0080800080800080), CONST64(0x0080808080800080),
  CONST64(0x0080000080808000), CONST64(0x0080008080808000), CONST64(0x0080000080808080), CONST64(0x0080008080808080),
  CONST64(0x0080800080808000), CONST64(0x0080808080808000), CONST64(0x0080800080808080), CONST64(0x0080808080808080),
  CONST64(0x8000000080000000), CONST64(0x8000008080000000), CONST64(0x8000000080000080), CONST64(0x8000008080000080),
  CONST64(0x8000800080000000), CONST64(0x8000808080000000), CONST64(0x8000800080000080), CONST64(0x8000808080000080),
  CONST64(0x8000000080008000), CONST64(0x8000008080008000), CONST64(0x8000000080008080), CONST64(0x8000008080008080),
  CONST64(0x8000800080008000), CONST64(0x8000808080008000), CONST64(0x8000800080008080), CONST64(0x8000808080008080),
  CONST64(0x8080000080000000), CONST64(0x8080008080000000), CONST64(0x8080000080000080), CONST64(0x8080008080000080),
  CONST64(0x8080800080000000), CONST64(0x8080808080000000), CONST64(0x8080800080000080), CONST64(0x8080808080000080),
  CONST64(0x8080000080008000), CONST64(0x8080008080008000), CONST64(0x8080000080008080), CONST64(0x8080008080008080),
  CONST64(0x8080800080008000), CONST64(0x8080808080008000), CONST64(0x8080800080008080), CONST64(0x8080808080008080),
  CONST64(0x8000000080800000), CONST64(0x8000008080800000), CONST64(0x8000000080800080), CONST64(0x8000008080800080),
  CONST64(0x8000800080800000), CONST64(0x8000808080800000), CONST64(0x8000800080800080), CONST64(0x8000808080800080),
  CONST64(0x8000000080808000), CONST64(0x8000008080808000), CONST64(0x8000000080808080), CONST64(0x8000008080808080),
  CONST64(0x8000800080808000), CONST64(0x8000808080808000), CONST64(0x8000800080808080), CONST64(0x8000808080808080),
  CONST64(0x8080000080800000), CONST64(0x8080008080800000), CONST64(0x8080000080800080), CONST64(0x8080008080800080),
  CONST64(0x8080800080800000), CONST64(0x8080808080800000), CONST64(0x8080800080800080), CONST64(0x8080808080800080),
  CONST64(0x8080000080808000), CONST64(0x8080008080808000), CONST64(0x8080000080808080), CONST64(0x8080008080808080),
  CONST64(0x8080800080808000), CONST64(0x8080808080808000), CONST64(0x8080800080808080), CONST64(0x8080808080808080)
  },
{ CONST64(0x0000000000000000), CONST64(0x0000004000000000), CONST64(0x0000000000000040), CONST64(0x0000004000000040),
  CONST64(0x0000400000000000), CONST64(0x0000404000000000), CONST64(0x0000400000000040), CONST64(0x0000404000000040),
  CONST64(0x0000000000004000), CONST64(0x0000004000004000), CONST64(0x0000000000004040), CONST64(0x0000004000004040),
  CONST64(0x0000400000004000), CONST64(0x0000404000004000), CONST64(0x0000400000004040), CONST64(0x0000404000004040),
  CONST64(0x0040000000000000), CONST64(0x0040004000000000), CONST64(0x0040000000000040), CONST64(0x0040004000000040),
  CONST64(0x0040400000000000), CONST64(0x0040404000000000), CONST64(0x0040400000000040), CONST64(0x0040404000000040),
  CONST64(0x0040000000004000), CONST64(0x0040004000004000), CONST64(0x0040000000004040), CONST64(0x0040004000004040),
  CONST64(0x0040400000004000), CONST64(0x0040404000004000), CONST64(0x0040400000004040), CONST64(0x0040404000004040),
  CONST64(0x0000000000400000), CONST64(0x0000004000400000), CONST64(0x0000000000400040), CONST64(0x0000004000400040),
  CONST64(0x0000400000400000), CONST64(0x0000404000400000), CONST64(0x0000400000400040), CONST64(0x0000404000400040),
  CONST64(0x0000000000404000), CONST64(0x0000004000404000), CONST64(0x0000000000404040), CONST64(0x0000004000404040),
  CONST64(0x0000400000404000), CONST64(0x0000404000404000), CONST64(0x0000400000404040), CONST64(0x0000404000404040),
  CONST64(0x0040000000400000), CONST64(0x0040004000400000), CONST64(0x0040000000400040), CONST64(0x0040004000400040),
  CONST64(0x0040400000400000), CONST64(0x0040404000400000), CONST64(0x0040400000400040), CONST64(0x0040404000400040),
  CONST64(0x0040000000404000), CONST64(0x0040004000404000), CONST64(0x0040000000404040), CONST64(0x0040004000404040),
  CONST64(0x0040400000404000), CONST64(0x0040404000404000), CONST64(0x0040400000404040), CONST64(0x0040404000404040),
  CONST64(0x4000000000000000), CONST64(0x4000004000000000), CONST64(0x4000000000000040), CONST64(0x4000004000000040),
  CONST64(0x4000400000000000), CONST64(0x4000404000000000), CONST64(0x4000400000000040), CONST64(0x4000404000000040),
  CONST64(0x4000000000004000), CONST64(0x4000004000004000), CONST64(0x4000000000004040), CONST64(0x4000004000004040),
  CONST64(0x4000400000004000), CONST64(0x4000404000004000), CONST64(0x4000400000004040), CONST64(0x4000404000004040),
  CONST64(0x4040000000000000), CONST64(0x4040004000000000), CONST64(0x4040000000000040), CONST64(0x4040004000000040),
  CONST64(0x4040400000000000), CONST64(0x4040404000000000), CONST64(0x4040400000000040), CONST64(0x4040404000000040),
  CONST64(0x4040000000004000), CONST64(0x4040004000004000), CONST64(0x4040000000004040), CONST64(0x4040004000004040),
  CONST64(0x4040400000004000), CONST64(0x4040404000004000), CONST64(0x4040400000004040), CONST64(0x4040404000004040),
  CONST64(0x4000000000400000), CONST64(0x4000004000400000), CONST64(0x4000000000400040), CONST64(0x4000004000400040),
  CONST64(0x4000400000400000), CONST64(0x4000404000400000), CONST64(0x4000400000400040), CONST64(0x4000404000400040),
  CONST64(0x4000000000404000), CONST64(0x4000004000404000), CONST64(0x4000000000404040), CONST64(0x4000004000404040),
  CONST64(0x4000400000404000), CONST64(0x4000404000404000), CONST64(0x4000400000404040), CONST64(0x4000404000404040),
  CONST64(0x4040000000400000), CONST64(0x4040004000400000), CONST64(0x4040000000400040), CONST64(0x4040004000400040),
  CONST64(0x4040400000400000), CONST64(0x4040404000400000), CONST64(0x4040400000400040), CONST64(0x4040404000400040),
  CONST64(0x4040000000404000), CONST64(0x4040004000404000), CONST64(0x4040000000404040), CONST64(0x4040004000404040),
  CONST64(0x4040400000404000), CONST64(0x4040404000404000), CONST64(0x4040400000404040), CONST64(0x4040404000404040),
  CONST64(0x0000000040000000), CONST64(0x0000004040000000), CONST64(0x0000000040000040), CONST64(0x0000004040000040),
  CONST64(0x0000400040000000), CONST64(0x0000404040000000), CONST64(0x0000400040000040), CONST64(0x0000404040000040),
  CONST64(0x0000000040004000), CONST64(0x0000004040004000), CONST64(0x0000000040004040), CONST64(0x0000004040004040),
  CONST64(0x0000400040004000), CONST64(0x0000404040004000), CONST64(0x0000400040004040), CONST64(0x0000404040004040),
  CONST64(0x0040000040000000), CONST64(0x0040004040000000), CONST64(0x0040000040000040), CONST64(0x0040004040000040),
  CONST64(0x0040400040000000), CONST64(0x0040404040000000), CONST64(0x0040400040000040), CONST64(0x0040404040000040),
  CONST64(0x0040000040004000), CONST64(0x0040004040004000), CONST64(0x0040000040004040), CONST64(0x0040004040004040),
  CONST64(0x0040400040004000), CONST64(0x0040404040004000), CONST64(0x0040400040004040), CONST64(0x0040404040004040),
  CONST64(0x0000000040400000), CONST64(0x0000004040400000), CONST64(0x0000000040400040), CONST64(0x0000004040400040),
  CONST64(0x0000400040400000), CONST64(0x0000404040400000), CONST64(0x0000400040400040), CONST64(0x0000404040400040),
  CONST64(0x0000000040404000), CONST64(0x0000004040404000), CONST64(0x0000000040404040), CONST64(0x0000004040404040),
  CONST64(0x0000400040404000), CONST64(0x0000404040404000), CONST64(0x0000400040404040), CONST64(0x0000404040404040),
  CONST64(0x0040000040400000), CONST64(0x0040004040400000), CONST64(0x0040000040400040), CONST64(0x0040004040400040),
  CONST64(0x0040400040400000), CONST64(0x0040404040400000), CONST64(0x0040400040400040), CONST64(0x0040404040400040),
  CONST64(0x0040000040404000), CONST64(0x0040004040404000), CONST64(0x0040000040404040), CONST64(0x0040004040404040),
  CONST64(0x0040400040404000), CONST64(0x0040404040404000), CONST64(0x0040400040404040), CONST64(0x0040404040404040),
  CONST64(0x4000000040000000), CONST64(0x4000004040000000), CONST64(0x4000000040000040), CONST64(0x4000004040000040),
  CONST64(0x4000400040000000), CONST64(0x4000404040000000), CONST64(0x4000400040000040), CONST64(0x4000404040000040),
  CONST64(0x4000000040004000), CONST64(0x4000004040004000), CONST64(0x4000000040004040), CONST64(0x4000004040004040),
  CONST64(0x4000400040004000), CONST64(0x4000404040004000), CONST64(0x4000400040004040), CONST64(0x4000404040004040),
  CONST64(0x4040000040000000), CONST64(0x4040004040000000), CONST64(0x4040000040000040), CONST64(0x4040004040000040),
  CONST64(0x4040400040000000), CONST64(0x4040404040000000), CONST64(0x4040400040000040), CONST64(0x4040404040000040),
  CONST64(0x4040000040004000), CONST64(0x4040004040004000), CONST64(0x4040000040004040), CONST64(0x4040004040004040),
  CONST64(0x4040400040004000), CONST64(0x4040404040004000), CONST64(0x4040400040004040), CONST64(0x4040404040004040),
  CONST64(0x4000000040400000), CONST64(0x4000004040400000), CONST64(0x4000000040400040), CONST64(0x4000004040400040),
  CONST64(0x4000400040400000), CONST64(0x4000404040400000), CONST64(0x4000400040400040), CONST64(0x4000404040400040),
  CONST64(0x4000000040404000), CONST64(0x4000004040404000), CONST64(0x4000000040404040), CONST64(0x4000004040404040),
  CONST64(0x4000400040404000), CONST64(0x4000404040404000), CONST64(0x4000400040404040), CONST64(0x4000404040404040),
  CONST64(0x4040000040400000), CONST64(0x4040004040400000), CONST64(0x4040000040400040), CONST64(0x4040004040400040),
  CONST64(0x4040400040400000), CONST64(0x4040404040400000), CONST64(0x4040400040400040), CONST64(0x4040404040400040),
  CONST64(0x4040000040404000), CONST64(0x4040004040404000), CONST64(0x4040000040404040), CONST64(0x4040004040404040),
  CONST64(0x4040400040404000), CONST64(0x4040404040404000), CONST64(0x4040400040404040), CONST64(0x4040404040404040)
  },
{ CONST64(0x0000000000000000), CONST64(0x0000002000000000), CONST64(0x0000000000000020), CONST64(0x0000002000000020),
  CONST64(0x0000200000000000), CONST64(0x0000202000000000), CONST64(0x0000200000000020), CONST64(0x0000202000000020),
  CONST64(0x0000000000002000), CONST64(0x0000002000002000), CONST64(0x0000000000002020), CONST64(0x0000002000002020),
  CONST64(0x0000200000002000), CONST64(0x0000202000002000), CONST64(0x0000200000002020), CONST64(0x0000202000002020),
  CONST64(0x0020000000000000), CONST64(0x0020002000000000), CONST64(0x0020000000000020), CONST64(0x0020002000000020),
  CONST64(0x0020200000000000), CONST64(0x0020202000000000), CONST64(0x0020200000000020), CONST64(0x0020202000000020),
  CONST64(0x0020000000002000), CONST64(0x0020002000002000), CONST64(0x0020000000002020), CONST64(0x0020002000002020),
  CONST64(0x0020200000002000), CONST64(0x0020202000002000), CONST64(0x0020200000002020), CONST64(0x0020202000002020),
  CONST64(0x0000000000200000), CONST64(0x0000002000200000), CONST64(0x0000000000200020), CONST64(0x0000002000200020),
  CONST64(0x0000200000200000), CONST64(0x0000202000200000), CONST64(0x0000200000200020), CONST64(0x0000202000200020),
  CONST64(0x0000000000202000), CONST64(0x0000002000202000), CONST64(0x0000000000202020), CONST64(0x0000002000202020),
  CONST64(0x0000200000202000), CONST64(0x0000202000202000), CONST64(0x0000200000202020), CONST64(0x0000202000202020),
  CONST64(0x0020000000200000), CONST64(0x0020002000200000), CONST64(0x0020000000200020), CONST64(0x0020002000200020),
  CONST64(0x0020200000200000), CONST64(0x0020202000200000), CONST64(0x0020200000200020), CONST64(0x0020202000200020),
  CONST64(0x0020000000202000), CONST64(0x0020002000202000), CONST64(0x0020000000202020), CONST64(0x0020002000202020),
  CONST64(0x0020200000202000), CONST64(0x0020202000202000), CONST64(0x0020200000202020), CONST64(0x0020202000202020),
  CONST64(0x2000000000000000), CONST64(0x2000002000000000), CONST64(0x2000000000000020), CONST64(0x2000002000000020),
  CONST64(0x2000200000000000), CONST64(0x2000202000000000), CONST64(0x2000200000000020), CONST64(0x2000202000000020),
  CONST64(0x2000000000002000), CONST64(0x2000002000002000), CONST64(0x2000000000002020), CONST64(0x2000002000002020),
  CONST64(0x2000200000002000), CONST64(0x2000202000002000), CONST64(0x2000200000002020), CONST64(0x2000202000002020),
  CONST64(0x2020000000000000), CONST64(0x2020002000000000), CONST64(0x2020000000000020), CONST64(0x2020002000000020),
  CONST64(0x2020200000000000), CONST64(0x2020202000000000), CONST64(0x2020200000000020), CONST64(0x2020202000000020),
  CONST64(0x2020000000002000), CONST64(0x2020002000002000), CONST64(0x2020000000002020), CONST64(0x2020002000002020),
  CONST64(0x2020200000002000), CONST64(0x2020202000002000), CONST64(0x2020200000002020), CONST64(0x2020202000002020),
  CONST64(0x2000000000200000), CONST64(0x2000002000200000), CONST64(0x2000000000200020), CONST64(0x2000002000200020),
  CONST64(0x2000200000200000), CONST64(0x2000202000200000), CONST64(0x2000200000200020), CONST64(0x2000202000200020),
  CONST64(0x2000000000202000), CONST64(0x2000002000202000), CONST64(0x2000000000202020), CONST64(0x2000002000202020),
  CONST64(0x2000200000202000), CONST64(0x2000202000202000), CONST64(0x2000200000202020), CONST64(0x2000202000202020),
  CONST64(0x2020000000200000), CONST64(0x2020002000200000), CONST64(0x2020000000200020), CONST64(0x2020002000200020),
  CONST64(0x2020200000200000), CONST64(0x2020202000200000), CONST64(0x2020200000200020), CONST64(0x2020202000200020),
  CONST64(0x2020000000202000), CONST64(0x2020002000202000), CONST64(0x2020000000202020), CONST64(0x2020002000202020),
  CONST64(0x2020200000202000), CONST64(0x2020202000202000), CONST64(0x2020200000202020), CONST64(0x2020202000202020),
  CONST64(0x0000000020000000), CONST64(0x0000002020000000), CONST64(0x0000000020000020), CONST64(0x0000002020000020),
  CONST64(0x0000200020000000), CONST64(0x0000202020000000), CONST64(0x0000200020000020), CONST64(0x0000202020000020),
  CONST64(0x0000000020002000), CONST64(0x0000002020002000), CONST64(0x0000000020002020), CONST64(0x0000002020002020),
  CONST64(0x0000200020002000), CONST64(0x0000202020002000), CONST64(0x0000200020002020), CONST64(0x0000202020002020),
  CONST64(0x0020000020000000), CONST64(0x0020002020000000), CONST64(0x0020000020000020), CONST64(0x0020002020000020),
  CONST64(0x0020200020000000), CONST64(0x0020202020000000), CONST64(0x0020200020000020), CONST64(0x0020202020000020),
  CONST64(0x0020000020002000), CONST64(0x0020002020002000), CONST64(0x0020000020002020), CONST64(0x0020002020002020),
  CONST64(0x0020200020002000), CONST64(0x0020202020002000), CONST64(0x0020200020002020), CONST64(0x0020202020002020),
  CONST64(0x0000000020200000), CONST64(0x0000002020200000), CONST64(0x0000000020200020), CONST64(0x0000002020200020),
  CONST64(0x0000200020200000), CONST64(0x0000202020200000), CONST64(0x0000200020200020), CONST64(0x0000202020200020),
  CONST64(0x0000000020202000), CONST64(0x0000002020202000), CONST64(0x0000000020202020), CONST64(0x0000002020202020),
  CONST64(0x0000200020202000), CONST64(0x0000202020202000), CONST64(0x0000200020202020), CONST64(0x0000202020202020),
  CONST64(0x0020000020200000), CONST64(0x0020002020200000), CONST64(0x0020000020200020), CONST64(0x0020002020200020),
  CONST64(0x0020200020200000), CONST64(0x0020202020200000), CONST64(0x0020200020200020), CONST64(0x0020202020200020),
  CONST64(0x0020000020202000), CONST64(0x0020002020202000), CONST64(0x0020000020202020), CONST64(0x0020002020202020),
  CONST64(0x0020200020202000), CONST64(0x0020202020202000), CONST64(0x0020200020202020), CONST64(0x0020202020202020),
  CONST64(0x2000000020000000), CONST64(0x2000002020000000), CONST64(0x2000000020000020), CONST64(0x2000002020000020),
  CONST64(0x2000200020000000), CONST64(0x2000202020000000), CONST64(0x2000200020000020), CONST64(0x2000202020000020),
  CONST64(0x2000000020002000), CONST64(0x2000002020002000), CONST64(0x2000000020002020), CONST64(0x2000002020002020),
  CONST64(0x2000200020002000), CONST64(0x2000202020002000), CONST64(0x2000200020002020), CONST64(0x2000202020002020),
  CONST64(0x2020000020000000), CONST64(0x2020002020000000), CONST64(0x2020000020000020), CONST64(0x2020002020000020),
  CONST64(0x2020200020000000), CONST64(0x2020202020000000), CONST64(0x2020200020000020), CONST64(0x2020202020000020),
  CONST64(0x2020000020002000), CONST64(0x2020002020002000), CONST64(0x2020000020002020), CONST64(0x2020002020002020),
  CONST64(0x2020200020002000), CONST64(0x2020202020002000), CONST64(0x2020200020002020), CONST64(0x2020202020002020),
  CONST64(0x2000000020200000), CONST64(0x2000002020200000), CONST64(0x2000000020200020), CONST64(0x2000002020200020),
  CONST64(0x2000200020200000), CONST64(0x2000202020200000), CONST64(0x2000200020200020), CONST64(0x2000202020200020),
  CONST64(0x2000000020202000), CONST64(0x2000002020202000), CONST64(0x2000000020202020), CONST64(0x2000002020202020),
  CONST64(0x2000200020202000), CONST64(0x2000202020202000), CONST64(0x2000200020202020), CONST64(0x2000202020202020),
  CONST64(0x2020000020200000), CONST64(0x2020002020200000), CONST64(0x2020000020200020), CONST64(0x2020002020200020),
  CONST64(0x2020200020200000), CONST64(0x2020202020200000), CONST64(0x2020200020200020), CONST64(0x2020202020200020),
  CONST64(0x2020000020202000), CONST64(0x2020002020202000), CONST64(0x2020000020202020), CONST64(0x2020002020202020),
  CONST64(0x2020200020202000), CONST64(0x2020202020202000), CONST64(0x2020200020202020), CONST64(0x2020202020202020)
  }};

static const ulong64 des_fp[8][256] = {

{ CONST64(0x0000000000000000), CONST64(0x0000008000000000), CONST64(0x0000000002000000), CONST64(0x0000008002000000),
  CONST64(0x0000000000020000), CONST64(0x0000008000020000), CONST64(0x0000000002020000), CONST64(0x0000008002020000),
  CONST64(0x0000000000000200), CONST64(0x0000008000000200), CONST64(0x0000000002000200), CONST64(0x0000008002000200),
  CONST64(0x0000000000020200), CONST64(0x0000008000020200), CONST64(0x0000000002020200), CONST64(0x0000008002020200),
  CONST64(0x0000000000000002), CONST64(0x0000008000000002), CONST64(0x0000000002000002), CONST64(0x0000008002000002),
  CONST64(0x0000000000020002), CONST64(0x0000008000020002), CONST64(0x0000000002020002), CONST64(0x0000008002020002),
  CONST64(0x0000000000000202), CONST64(0x0000008000000202), CONST64(0x0000000002000202), CONST64(0x0000008002000202),
  CONST64(0x0000000000020202), CONST64(0x0000008000020202), CONST64(0x0000000002020202), CONST64(0x0000008002020202),
  CONST64(0x0200000000000000), CONST64(0x0200008000000000), CONST64(0x0200000002000000), CONST64(0x0200008002000000),
  CONST64(0x0200000000020000), CONST64(0x0200008000020000), CONST64(0x0200000002020000), CONST64(0x0200008002020000),
  CONST64(0x0200000000000200), CONST64(0x0200008000000200), CONST64(0x0200000002000200), CONST64(0x0200008002000200),
  CONST64(0x0200000000020200), CONST64(0x0200008000020200), CONST64(0x0200000002020200), CONST64(0x0200008002020200),
  CONST64(0x0200000000000002), CONST64(0x0200008000000002), CONST64(0x0200000002000002), CONST64(0x0200008002000002),
  CONST64(0x0200000000020002), CONST64(0x0200008000020002), CONST64(0x0200000002020002), CONST64(0x0200008002020002),
  CONST64(0x0200000000000202), CONST64(0x0200008000000202), CONST64(0x0200000002000202), CONST64(0x0200008002000202),
  CONST64(0x0200000000020202), CONST64(0x0200008000020202), CONST64(0x0200000002020202), CONST64(0x0200008002020202),
  CONST64(0x0002000000000000), CONST64(0x0002008000000000), CONST64(0x0002000002000000), CONST64(0x0002008002000000),
  CONST64(0x0002000000020000), CONST64(0x0002008000020000), CONST64(0x0002000002020000), CONST64(0x0002008002020000),
  CONST64(0x0002000000000200), CONST64(0x0002008000000200), CONST64(0x0002000002000200), CONST64(0x0002008002000200),
  CONST64(0x0002000000020200), CONST64(0x0002008000020200), CONST64(0x0002000002020200), CONST64(0x0002008002020200),
  CONST64(0x0002000000000002), CONST64(0x0002008000000002), CONST64(0x0002000002000002), CONST64(0x0002008002000002),
  CONST64(0x0002000000020002), CONST64(0x0002008000020002), CONST64(0x0002000002020002), CONST64(0x0002008002020002),
  CONST64(0x0002000000000202), CONST64(0x0002008000000202), CONST64(0x0002000002000202), CONST64(0x0002008002000202),
  CONST64(0x0002000000020202), CONST64(0x0002008000020202), CONST64(0x0002000002020202), CONST64(0x0002008002020202),
  CONST64(0x0202000000000000), CONST64(0x0202008000000000), CONST64(0x0202000002000000), CONST64(0x0202008002000000),
  CONST64(0x0202000000020000), CONST64(0x0202008000020000), CONST64(0x0202000002020000), CONST64(0x0202008002020000),
  CONST64(0x0202000000000200), CONST64(0x0202008000000200), CONST64(0x0202000002000200), CONST64(0x0202008002000200),
  CONST64(0x0202000000020200), CONST64(0x0202008000020200), CONST64(0x0202000002020200), CONST64(0x0202008002020200),
  CONST64(0x0202000000000002), CONST64(0x0202008000000002), CONST64(0x0202000002000002), CONST64(0x0202008002000002),
  CONST64(0x0202000000020002), CONST64(0x0202008000020002), CONST64(0x0202000002020002), CONST64(0x0202008002020002),
  CONST64(0x0202000000000202), CONST64(0x0202008000000202), CONST64(0x0202000002000202), CONST64(0x0202008002000202),
  CONST64(0x0202000000020202), CONST64(0x0202008000020202), CONST64(0x0202000002020202), CONST64(0x0202008002020202),
  CONST64(0x0000020000000000), CONST64(0x0000028000000000), CONST64(0x0000020002000000), CONST64(0x0000028002000000),
  CONST64(0x0000020000020000), CONST64(0x0000028000020000), CONST64(0x0000020002020000), CONST64(0x0000028002020000),
  CONST64(0x0000020000000200), CONST64(0x0000028000000200), CONST64(0x0000020002000200), CONST64(0x0000028002000200),
  CONST64(0x0000020000020200), CONST64(0x0000028000020200), CONST64(0x0000020002020200), CONST64(0x0000028002020200),
  CONST64(0x0000020000000002), CONST64(0x0000028000000002), CONST64(0x0000020002000002), CONST64(0x0000028002000002),
  CONST64(0x0000020000020002), CONST64(0x0000028000020002), CONST64(0x0000020002020002), CONST64(0x0000028002020002),
  CONST64(0x0000020000000202), CONST64(0x0000028000000202), CONST64(0x0000020002000202), CONST64(0x0000028002000202),
  CONST64(0x0000020000020202), CONST64(0x0000028000020202), CONST64(0x0000020002020202), CONST64(0x0000028002020202),
  CONST64(0x0200020000000000), CONST64(0x0200028000000000), CONST64(0x0200020002000000), CONST64(0x0200028002000000),
  CONST64(0x0200020000020000), CONST64(0x0200028000020000), CONST64(0x0200020002020000), CONST64(0x0200028002020000),
  CONST64(0x0200020000000200), CONST64(0x0200028000000200), CONST64(0x0200020002000200), CONST64(0x0200028002000200),
  CONST64(0x0200020000020200), CONST64(0x0200028000020200), CONST64(0x0200020002020200), CONST64(0x0200028002020200),
  CONST64(0x0200020000000002), CONST64(0x0200028000000002), CONST64(0x0200020002000002), CONST64(0x0200028002000002),
  CONST64(0x0200020000020002), CONST64(0x0200028000020002), CONST64(0x0200020002020002), CONST64(0x0200028002020002),
  CONST64(0x0200020000000202), CONST64(0x0200028000000202), CONST64(0x0200020002000202), CONST64(0x0200028002000202),
  CONST64(0x0200020000020202), CONST64(0x0200028000020202), CONST64(0x0200020002020202), CONST64(0x0200028002020202),
  CONST64(0x0002020000000000), CONST64(0x0002028000000000), CONST64(0x0002020002000000), CONST64(0x0002028002000000),
  CONST64(0x0002020000020000), CONST64(0x0002028000020000), CONST64(0x0002020002020000), CONST64(0x0002028002020000),
  CONST64(0x0002020000000200), CONST64(0x0002028000000200), CONST64(0x0002020002000200), CONST64(0x0002028002000200),
  CONST64(0x0002020000020200), CONST64(0x0002028000020200), CONST64(0x0002020002020200), CONST64(0x0002028002020200),
  CONST64(0x0002020000000002), CONST64(0x0002028000000002), CONST64(0x0002020002000002), CONST64(0x0002028002000002),
  CONST64(0x0002020000020002), CONST64(0x0002028000020002), CONST64(0x0002020002020002), CONST64(0x0002028002020002),
  CONST64(0x0002020000000202), CONST64(0x0002028000000202), CONST64(0x0002020002000202), CONST64(0x0002028002000202),
  CONST64(0x0002020000020202), CONST64(0x0002028000020202), CONST64(0x0002020002020202), CONST64(0x0002028002020202),
  CONST64(0x0202020000000000), CONST64(0x0202028000000000), CONST64(0x0202020002000000), CONST64(0x0202028002000000),
  CONST64(0x0202020000020000), CONST64(0x0202028000020000), CONST64(0x0202020002020000), CONST64(0x0202028002020000),
  CONST64(0x0202020000000200), CONST64(0x0202028000000200), CONST64(0x0202020002000200), CONST64(0x0202028002000200),
  CONST64(0x0202020000020200), CONST64(0x0202028000020200), CONST64(0x0202020002020200), CONST64(0x0202028002020200),
  CONST64(0x0202020000000002), CONST64(0x0202028000000002), CONST64(0x0202020002000002), CONST64(0x0202028002000002),
  CONST64(0x0202020000020002), CONST64(0x0202028000020002), CONST64(0x0202020002020002), CONST64(0x0202028002020002),
  CONST64(0x0202020000000202), CONST64(0x0202028000000202), CONST64(0x0202020002000202), CONST64(0x0202028002000202),
  CONST64(0x0202020000020202), CONST64(0x0202028000020202), CONST64(0x0202020002020202), CONST64(0x0202028002020202)
  },
{ CONST64(0x0000000000000000), CONST64(0x0000000200000000), CONST64(0x0000000008000000), CONST64(0x0000000208000000),
  CONST64(0x0000000000080000), CONST64(0x0000000200080000), CONST64(0x0000000008080000), CONST64(0x0000000208080000),
  CONST64(0x0000000000000800), CONST64(0x0000000200000800), CONST64(0x0000000008000800), CONST64(0x0000000208000800),
  CONST64(0x0000000000080800), CONST64(0x0000000200080800), CONST64(0x0000000008080800), CONST64(0x0000000208080800),
  CONST64(0x0000000000000008), CONST64(0x0000000200000008), CONST64(0x0000000008000008), CONST64(0x0000000208000008),
  CONST64(0x0000000000080008), CONST64(0x0000000200080008), CONST64(0x0000000008080008), CONST64(0x0000000208080008),
  CONST64(0x0000000000000808), CONST64(0x0000000200000808), CONST64(0x0000000008000808), CONST64(0x0000000208000808),
  CONST64(0x0000000000080808), CONST64(0x0000000200080808), CONST64(0x0000000008080808), CONST64(0x0000000208080808),
  CONST64(0x0800000000000000), CONST64(0x0800000200000000), CONST64(0x0800000008000000), CONST64(0x0800000208000000),
  CONST64(0x0800000000080000), CONST64(0x0800000200080000), CONST64(0x0800000008080000), CONST64(0x0800000208080000),
  CONST64(0x0800000000000800), CONST64(0x0800000200000800), CONST64(0x0800000008000800), CONST64(0x0800000208000800),
  CONST64(0x0800000000080800), CONST64(0x0800000200080800), CONST64(0x0800000008080800), CONST64(0x0800000208080800),
  CONST64(0x0800000000000008), CONST64(0x0800000200000008), CONST64(0x0800000008000008), CONST64(0x0800000208000008),
  CONST64(0x0800000000080008), CONST64(0x0800000200080008), CONST64(0x0800000008080008), CONST64(0x0800000208080008),
  CONST64(0x0800000000000808), CONST64(0x0800000200000808), CONST64(0x0800000008000808), CONST64(0x0800000208000808),
  CONST64(0x0800000000080808), CONST64(0x0800000200080808), CONST64(0x0800000008080808), CONST64(0x0800000208080808),
  CONST64(0x0008000000000000), CONST64(0x0008000200000000), CONST64(0x0008000008000000), CONST64(0x0008000208000000),
  CONST64(0x0008000000080000), CONST64(0x0008000200080000), CONST64(0x0008000008080000), CONST64(0x0008000208080000),
  CONST64(0x0008000000000800), CONST64(0x0008000200000800), CONST64(0x0008000008000800), CONST64(0x0008000208000800),
  CONST64(0x0008000000080800), CONST64(0x0008000200080800), CONST64(0x0008000008080800), CONST64(0x0008000208080800),
  CONST64(0x0008000000000008), CONST64(0x0008000200000008), CONST64(0x0008000008000008), CONST64(0x0008000208000008),
  CONST64(0x0008000000080008), CONST64(0x0008000200080008), CONST64(0x0008000008080008), CONST64(0x0008000208080008),
  CONST64(0x0008000000000808), CONST64(0x0008000200000808), CONST64(0x0008000008000808), CONST64(0x0008000208000808),
  CONST64(0x0008000000080808), CONST64(0x0008000200080808), CONST64(0x0008000008080808), CONST64(0x0008000208080808),
  CONST64(0x0808000000000000), CONST64(0x0808000200000000), CONST64(0x0808000008000000), CONST64(0x0808000208000000),
  CONST64(0x0808000000080000), CONST64(0x0808000200080000), CONST64(0x0808000008080000), CONST64(0x0808000208080000),
  CONST64(0x0808000000000800), CONST64(0x0808000200000800), CONST64(0x0808000008000800), CONST64(0x0808000208000800),
  CONST64(0x0808000000080800), CONST64(0x0808000200080800), CONST64(0x0808000008080800), CONST64(0x0808000208080800),
  CONST64(0x0808000000000008), CONST64(0x0808000200000008), CONST64(0x0808000008000008), CONST64(0x0808000208000008),
  CONST64(0x0808000000080008), CONST64(0x0808000200080008), CONST64(0x0808000008080008), CONST64(0x0808000208080008),
  CONST64(0x0808000000000808), CONST64(0x0808000200000808), CONST64(0x0808000008000808), CONST64(0x0808000208000808),
  CONST64(0x0808000000080808), CONST64(0x0808000200080808), CONST64(0x0808000008080808), CONST64(0x0808000208080808),
  CONST64(0x0000080000000000), CONST64(0x0000080200000000), CONST64(0x0000080008000000), CONST64(0x0000080208000000),
  CONST64(0x0000080000080000), CONST64(0x0000080200080000), CONST64(0x0000080008080000), CONST64(0x0000080208080000),
  CONST64(0x0000080000000800), CONST64(0x0000080200000800), CONST64(0x0000080008000800), CONST64(0x0000080208000800),
  CONST64(0x0000080000080800), CONST64(0x0000080200080800), CONST64(0x0000080008080800), CONST64(0x0000080208080800),
  CONST64(0x0000080000000008), CONST64(0x0000080200000008), CONST64(0x0000080008000008), CONST64(0x0000080208000008),
  CONST64(0x0000080000080008), CONST64(0x0000080200080008), CONST64(0x0000080008080008), CONST64(0x0000080208080008),
  CONST64(0x0000080000000808), CONST64(0x0000080200000808), CONST64(0x0000080008000808), CONST64(0x0000080208000808),
  CONST64(0x0000080000080808), CONST64(0x0000080200080808), CONST64(0x0000080008080808), CONST64(0x0000080208080808),
  CONST64(0x0800080000000000), CONST64(0x0800080200000000), CONST64(0x0800080008000000), CONST64(0x0800080208000000),
  CONST64(0x0800080000080000), CONST64(0x0800080200080000), CONST64(0x0800080008080000), CONST64(0x0800080208080000),
  CONST64(0x0800080000000800), CONST64(0x0800080200000800), CONST64(0x0800080008000800), CONST64(0x0800080208000800),
  CONST64(0x0800080000080800), CONST64(0x0800080200080800), CONST64(0x0800080008080800), CONST64(0x0800080208080800),
  CONST64(0x0800080000000008), CONST64(0x0800080200000008), CONST64(0x0800080008000008), CONST64(0x0800080208000008),
  CONST64(0x0800080000080008), CONST64(0x0800080200080008), CONST64(0x0800080008080008), CONST64(0x0800080208080008),
  CONST64(0x0800080000000808), CONST64(0x0800080200000808), CONST64(0x0800080008000808), CONST64(0x0800080208000808),
  CONST64(0x0800080000080808), CONST64(0x0800080200080808), CONST64(0x0800080008080808), CONST64(0x0800080208080808),
  CONST64(0x0008080000000000), CONST64(0x0008080200000000), CONST64(0x0008080008000000), CONST64(0x0008080208000000),
  CONST64(0x0008080000080000), CONST64(0x0008080200080000), CONST64(0x0008080008080000), CONST64(0x0008080208080000),
  CONST64(0x0008080000000800), CONST64(0x0008080200000800), CONST64(0x0008080008000800), CONST64(0x0008080208000800),
  CONST64(0x0008080000080800), CONST64(0x0008080200080800), CONST64(0x0008080008080800), CONST64(0x0008080208080800),
  CONST64(0x0008080000000008), CONST64(0x0008080200000008), CONST64(0x0008080008000008), CONST64(0x0008080208000008),
  CONST64(0x0008080000080008), CONST64(0x0008080200080008), CONST64(0x0008080008080008), CONST64(0x0008080208080008),
  CONST64(0x0008080000000808), CONST64(0x0008080200000808), CONST64(0x0008080008000808), CONST64(0x0008080208000808),
  CONST64(0x0008080000080808), CONST64(0x0008080200080808), CONST64(0x0008080008080808), CONST64(0x0008080208080808),
  CONST64(0x0808080000000000), CONST64(0x0808080200000000), CONST64(0x0808080008000000), CONST64(0x0808080208000000),
  CONST64(0x0808080000080000), CONST64(0x0808080200080000), CONST64(0x0808080008080000), CONST64(0x0808080208080000),
  CONST64(0x0808080000000800), CONST64(0x0808080200000800), CONST64(0x0808080008000800), CONST64(0x0808080208000800),
  CONST64(0x0808080000080800), CONST64(0x0808080200080800), CONST64(0x0808080008080800), CONST64(0x0808080208080800),
  CONST64(0x0808080000000008), CONST64(0x0808080200000008), CONST64(0x0808080008000008), CONST64(0x0808080208000008),
  CONST64(0x0808080000080008), CONST64(0x0808080200080008), CONST64(0x0808080008080008), CONST64(0x0808080208080008),
  CONST64(0x0808080000000808), CONST64(0x0808080200000808), CONST64(0x0808080008000808), CONST64(0x0808080208000808),
  CONST64(0x0808080000080808), CONST64(0x0808080200080808), CONST64(0x0808080008080808), CONST64(0x0808080208080808)
  },
{ CONST64(0x0000000000000000), CONST64(0x0000000800000000), CONST64(0x0000000020000000), CONST64(0x0000000820000000),
  CONST64(0x0000000000200000), CONST64(0x0000000800200000), CONST64(0x0000000020200000), CONST64(0x0000000820200000),
  CONST64(0x0000000000002000), CONST64(0x0000000800002000), CONST64(0x0000000020002000), CONST64(0x0000000820002000),
  CONST64(0x0000000000202000), CONST64(0x0000000800202000), CONST64(0x0000000020202000), CONST64(0x0000000820202000),
  CONST64(0x0000000000000020), CONST64(0x0000000800000020), CONST64(0x0000000020000020), CONST64(0x0000000820000020),
  CONST64(0x0000000000200020), CONST64(0x0000000800200020), CONST64(0x0000000020200020), CONST64(0x0000000820200020),
  CONST64(0x0000000000002020), CONST64(0x0000000800002020), CONST64(0x0000000020002020), CONST64(0x0000000820002020),
  CONST64(0x0000000000202020), CONST64(0x0000000800202020), CONST64(0x0000000020202020), CONST64(0x0000000820202020),
  CONST64(0x2000000000000000), CONST64(0x2000000800000000), CONST64(0x2000000020000000), CONST64(0x2000000820000000),
  CONST64(0x2000000000200000), CONST64(0x2000000800200000), CONST64(0x2000000020200000), CONST64(0x2000000820200000),
  CONST64(0x2000000000002000), CONST64(0x2000000800002000), CONST64(0x2000000020002000), CONST64(0x2000000820002000),
  CONST64(0x2000000000202000), CONST64(0x2000000800202000), CONST64(0x2000000020202000), CONST64(0x2000000820202000),
  CONST64(0x2000000000000020), CONST64(0x2000000800000020), CONST64(0x2000000020000020), CONST64(0x2000000820000020),
  CONST64(0x2000000000200020), CONST64(0x2000000800200020), CONST64(0x2000000020200020), CONST64(0x2000000820200020),
  CONST64(0x2000000000002020), CONST64(0x2000000800002020), CONST64(0x2000000020002020), CONST64(0x2000000820002020),
  CONST64(0x2000000000202020), CONST64(0x2000000800202020), CONST64(0x2000000020202020), CONST64(0x2000000820202020),
  CONST64(0x0020000000000000), CONST64(0x0020000800000000), CONST64(0x0020000020000000), CONST64(0x0020000820000000),
  CONST64(0x0020000000200000), CONST64(0x0020000800200000), CONST64(0x0020000020200000), CONST64(0x0020000820200000),
  CONST64(0x0020000000002000), CONST64(0x0020000800002000), CONST64(0x0020000020002000), CONST64(0x0020000820002000),
  CONST64(0x0020000000202000), CONST64(0x0020000800202000), CONST64(0x0020000020202000), CONST64(0x0020000820202000),
  CONST64(0x0020000000000020), CONST64(0x0020000800000020), CONST64(0x0020000020000020), CONST64(0x0020000820000020),
  CONST64(0x0020000000200020), CONST64(0x0020000800200020), CONST64(0x0020000020200020), CONST64(0x0020000820200020),
  CONST64(0x0020000000002020), CONST64(0x0020000800002020), CONST64(0x0020000020002020), CONST64(0x0020000820002020),
  CONST64(0x0020000000202020), CONST64(0x0020000800202020), CONST64(0x0020000020202020), CONST64(0x0020000820202020),
  CONST64(0x2020000000000000), CONST64(0x2020000800000000), CONST64(0x2020000020000000), CONST64(0x2020000820000000),
  CONST64(0x2020000000200000), CONST64(0x2020000800200000), CONST64(0x2020000020200000), CONST64(0x2020000820200000),
  CONST64(0x2020000000002000), CONST64(0x2020000800002000), CONST64(0x2020000020002000), CONST64(0x2020000820002000),
  CONST64(0x2020000000202000), CONST64(0x2020000800202000), CONST64(0x2020000020202000), CONST64(0x2020000820202000),
  CONST64(0x2020000000000020), CONST64(0x2020000800000020), CONST64(0x2020000020000020), CONST64(0x2020000820000020),
  CONST64(0x2020000000200020), CONST64(0x2020000800200020), CONST64(0x2020000020200020), CONST64(0x2020000820200020),
  CONST64(0x2020000000002020), CONST64(0x2020000800002020), CONST64(0x2020000020002020), CONST64(0x2020000820002020),
  CONST64(0x2020000000202020), CONST64(0x2020000800202020), CONST64(0x2020000020202020), CONST64(0x2020000820202020),
  CONST64(0x0000200000000000), CONST64(0x0000200800000000), CONST64(0x0000200020000000), CONST64(0x0000200820000000),
  CONST64(0x0000200000200000), CONST64(0x0000200800200000), CONST64(0x0000200020200000), CONST64(0x0000200820200000),
  CONST64(0x0000200000002000), CONST64(0x0000200800002000), CONST64(0x0000200020002000), CONST64(0x0000200820002000),
  CONST64(0x0000200000202000), CONST64(0x0000200800202000), CONST64(0x0000200020202000), CONST64(0x0000200820202000),
  CONST64(0x0000200000000020), CONST64(0x0000200800000020), CONST64(0x0000200020000020), CONST64(0x0000200820000020),
  CONST64(0x0000200000200020), CONST64(0x0000200800200020), CONST64(0x0000200020200020), CONST64(0x0000200820200020),
  CONST64(0x0000200000002020), CONST64(0x0000200800002020), CONST64(0x0000200020002020), CONST64(0x0000200820002020),
  CONST64(0x0000200000202020), CONST64(0x0000200800202020), CONST64(0x0000200020202020), CONST64(0x0000200820202020),
  CONST64(0x2000200000000000), CONST64(0x2000200800000000), CONST64(0x2000200020000000), CONST64(0x2000200820000000),
  CONST64(0x2000200000200000), CONST64(0x2000200800200000), CONST64(0x2000200020200000), CONST64(0x2000200820200000),
  CONST64(0x2000200000002000), CONST64(0x2000200800002000), CONST64(0x2000200020002000), CONST64(0x2000200820002000),
  CONST64(0x2000200000202000), CONST64(0x2000200800202000), CONST64(0x2000200020202000), CONST64(0x2000200820202000),
  CONST64(0x2000200000000020), CONST64(0x2000200800000020), CONST64(0x2000200020000020), CONST64(0x2000200820000020),
  CONST64(0x2000200000200020), CONST64(0x2000200800200020), CONST64(0x2000200020200020), CONST64(0x2000200820200020),
  CONST64(0x2000200000002020), CONST64(0x2000200800002020), CONST64(0x2000200020002020), CONST64(0x2000200820002020),
  CONST64(0x2000200000202020), CONST64(0x2000200800202020), CONST64(0x2000200020202020), CONST64(0x2000200820202020),
  CONST64(0x0020200000000000), CONST64(0x0020200800000000), CONST64(0x0020200020000000), CONST64(0x0020200820000000),
  CONST64(0x0020200000200000), CONST64(0x0020200800200000), CONST64(0x0020200020200000), CONST64(0x0020200820200000),
  CONST64(0x0020200000002000), CONST64(0x0020200800002000), CONST64(0x0020200020002000), CONST64(0x0020200820002000),
  CONST64(0x0020200000202000), CONST64(0x0020200800202000), CONST64(0x0020200020202000), CONST64(0x0020200820202000),
  CONST64(0x0020200000000020), CONST64(0x0020200800000020), CONST64(0x0020200020000020), CONST64(0x0020200820000020),
  CONST64(0x0020200000200020), CONST64(0x0020200800200020), CONST64(0x0020200020200020), CONST64(0x0020200820200020),
  CONST64(0x0020200000002020), CONST64(0x0020200800002020), CONST64(0x0020200020002020), CONST64(0x0020200820002020),
  CONST64(0x0020200000202020), CONST64(0x0020200800202020), CONST64(0x0020200020202020), CONST64(0x0020200820202020),
  CONST64(0x2020200000000000), CONST64(0x2020200800000000), CONST64(0x2020200020000000), CONST64(0x2020200820000000),
  CONST64(0x2020200000200000), CONST64(0x2020200800200000), CONST64(0x2020200020200000), CONST64(0x2020200820200000),
  CONST64(0x2020200000002000), CONST64(0x2020200800002000), CONST64(0x2020200020002000), CONST64(0x2020200820002000),
  CONST64(0x2020200000202000), CONST64(0x2020200800202000), CONST64(0x2020200020202000), CONST64(0x2020200820202000),
  CONST64(0x2020200000000020), CONST64(0x2020200800000020), CONST64(0x2020200020000020), CONST64(0x2020200820000020),
  CONST64(0x2020200000200020), CONST64(0x2020200800200020), CONST64(0x2020200020200020), CONST64(0x2020200820200020),
  CONST64(0x2020200000002020), CONST64(0x2020200800002020), CONST64(0x2020200020002020), CONST64(0x2020200820002020),
  CONST64(0x2020200000202020), CONST64(0x2020200800202020), CONST64(0x2020200020202020), CONST64(0x2020200820202020)
  },
{ CONST64(0x0000000000000000), CONST64(0x0000002000000000), CONST64(0x0000000080000000), CONST64(0x0000002080000000),
  CONST64(0x0000000000800000), CONST64(0x0000002000800000), CONST64(0x0000000080800000), CONST64(0x0000002080800000),
  CONST64(0x0000000000008000), CONST64(0x0000002000008000), CONST64(0x0000000080008000), CONST64(0x0000002080008000),
  CONST64(0x0000000000808000), CONST64(0x0000002000808000), CONST64(0x0000000080808000), CONST64(0x0000002080808000),
  CONST64(0x0000000000000080), CONST64(0x0000002000000080), CONST64(0x0000000080000080), CONST64(0x0000002080000080),
  CONST64(0x0000000000800080), CONST64(0x0000002000800080), CONST64(0x0000000080800080), CONST64(0x0000002080800080),
  CONST64(0x0000000000008080), CONST64(0x0000002000008080), CONST64(0x0000000080008080), CONST64(0x0000002080008080),
  CONST64(0x0000000000808080), CONST64(0x0000002000808080), CONST64(0x0000000080808080), CONST64(0x0000002080808080),
  CONST64(0x8000000000000000), CONST64(0x8000002000000000), CONST64(0x8000000080000000), CONST64(0x8000002080000000),
  CONST64(0x8000000000800000), CONST64(0x8000002000800000), CONST64(0x8000000080800000), CONST64(0x8000002080800000),
  CONST64(0x8000000000008000), CONST64(0x8000002000008000), CONST64(0x8000000080008000), CONST64(0x8000002080008000),
  CONST64(0x8000000000808000), CONST64(0x8000002000808000), CONST64(0x8000000080808000), CONST64(0x8000002080808000),
  CONST64(0x8000000000000080), CONST64(0x8000002000000080), CONST64(0x8000000080000080), CONST64(0x8000002080000080),
  CONST64(0x8000000000800080), CONST64(0x8000002000800080), CONST64(0x8000000080800080), CONST64(0x8000002080800080),
  CONST64(0x8000000000008080), CONST64(0x8000002000008080), CONST64(0x8000000080008080), CONST64(0x8000002080008080),
  CONST64(0x8000000000808080), CONST64(0x8000002000808080), CONST64(0x8000000080808080), CONST64(0x8000002080808080),
  CONST64(0x0080000000000000), CONST64(0x0080002000000000), CONST64(0x0080000080000000), CONST64(0x0080002080000000),
  CONST64(0x0080000000800000), CONST64(0x0080002000800000), CONST64(0x0080000080800000), CONST64(0x0080002080800000),
  CONST64(0x0080000000008000), CONST64(0x0080002000008000), CONST64(0x0080000080008000), CONST64(0x0080002080008000),
  CONST64(0x0080000000808000), CONST64(0x0080002000808000), CONST64(0x0080000080808000), CONST64(0x0080002080808000),
  CONST64(0x0080000000000080), CONST64(0x0080002000000080), CONST64(0x0080000080000080), CONST64(0x0080002080000080),
  CONST64(0x0080000000800080), CONST64(0x0080002000800080), CONST64(0x0080000080800080), CONST64(0x0080002080800080),
  CONST64(0x0080000000008080), CONST64(0x0080002000008080), CONST64(0x0080000080008080), CONST64(0x0080002080008080),
  CONST64(0x0080000000808080), CONST64(0x0080002000808080), CONST64(0x0080000080808080), CONST64(0x0080002080808080),
  CONST64(0x8080000000000000), CONST64(0x8080002000000000), CONST64(0x8080000080000000), CONST64(0x8080002080000000),
  CONST64(0x8080000000800000), CONST64(0x8080002000800000), CONST64(0x8080000080800000), CONST64(0x8080002080800000),
  CONST64(0x8080000000008000), CONST64(0x8080002000008000), CONST64(0x8080000080008000), CONST64(0x8080002080008000),
  CONST64(0x8080000000808000), CONST64(0x8080002000808000), CONST64(0x8080000080808000), CONST64(0x8080002080808000),
  CONST64(0x8080000000000080), CONST64(0x8080002000000080), CONST64(0x8080000080000080), CONST64(0x8080002080000080),
  CONST64(0x8080000000800080), CONST64(0x8080002000800080), CONST64(0x8080000080800080), CONST64(0x8080002080800080),
  CONST64(0x8080000000008080), CONST64(0x8080002000008080), CONST64(0x8080000080008080), CONST64(0x8080002080008080),
  CONST64(0x8080000000808080), CONST64(0x8080002000808080), CONST64(0x8080000080808080), CONST64(0x8080002080808080),
  CONST64(0x0000800000000000), CONST64(0x0000802000000000), CONST64(0x0000800080000000), CONST64(0x0000802080000000),
  CONST64(0x0000800000800000), CONST64(0x0000802000800000), CONST64(0x0000800080800000), CONST64(0x0000802080800000),
  CONST64(0x0000800000008000), CONST64(0x0000802000008000), CONST64(0x0000800080008000), CONST64(0x0000802080008000),
  CONST64(0x0000800000808000), CONST64(0x0000802000808000), CONST64(0x0000800080808000), CONST64(0x0000802080808000),
  CONST64(0x0000800000000080), CONST64(0x0000802000000080), CONST64(0x0000800080000080), CONST64(0x0000802080000080),
  CONST64(0x0000800000800080), CONST64(0x0000802000800080), CONST64(0x0000800080800080), CONST64(0x0000802080800080),
  CONST64(0x0000800000008080), CONST64(0x0000802000008080), CONST64(0x0000800080008080), CONST64(0x0000802080008080),
  CONST64(0x0000800000808080), CONST64(0x0000802000808080), CONST64(0x0000800080808080), CONST64(0x0000802080808080),
  CONST64(0x8000800000000000), CONST64(0x8000802000000000), CONST64(0x8000800080000000), CONST64(0x8000802080000000),
  CONST64(0x8000800000800000), CONST64(0x8000802000800000), CONST64(0x8000800080800000), CONST64(0x8000802080800000),
  CONST64(0x8000800000008000), CONST64(0x8000802000008000), CONST64(0x8000800080008000), CONST64(0x8000802080008000),
  CONST64(0x8000800000808000), CONST64(0x8000802000808000), CONST64(0x8000800080808000), CONST64(0x8000802080808000),
  CONST64(0x8000800000000080), CONST64(0x8000802000000080), CONST64(0x8000800080000080), CONST64(0x8000802080000080),
  CONST64(0x8000800000800080), CONST64(0x8000802000800080), CONST64(0x8000800080800080), CONST64(0x8000802080800080),
  CONST64(0x8000800000008080), CONST64(0x8000802000008080), CONST64(0x8000800080008080), CONST64(0x8000802080008080),
  CONST64(0x8000800000808080), CONST64(0x8000802000808080), CONST64(0x8000800080808080), CONST64(0x8000802080808080),
  CONST64(0x0080800000000000), CONST64(0x0080802000000000), CONST64(0x0080800080000000), CONST64(0x0080802080000000),
  CONST64(0x0080800000800000), CONST64(0x0080802000800000), CONST64(0x0080800080800000), CONST64(0x0080802080800000),
  CONST64(0x0080800000008000), CONST64(0x0080802000008000), CONST64(0x0080800080008000), CONST64(0x0080802080008000),
  CONST64(0x0080800000808000), CONST64(0x0080802000808000), CONST64(0x0080800080808000), CONST64(0x0080802080808000),
  CONST64(0x0080800000000080), CONST64(0x0080802000000080), CONST64(0x0080800080000080), CONST64(0x0080802080000080),
  CONST64(0x0080800000800080), CONST64(0x0080802000800080), CONST64(0x0080800080800080), CONST64(0x0080802080800080),
  CONST64(0x0080800000008080), CONST64(0x0080802000008080), CONST64(0x0080800080008080), CONST64(0x0080802080008080),
  CONST64(0x0080800000808080), CONST64(0x0080802000808080), CONST64(0x0080800080808080), CONST64(0x0080802080808080),
  CONST64(0x8080800000000000), CONST64(0x8080802000000000), CONST64(0x8080800080000000), CONST64(0x8080802080000000),
  CONST64(0x8080800000800000), CONST64(0x8080802000800000), CONST64(0x8080800080800000), CONST64(0x8080802080800000),
  CONST64(0x8080800000008000), CONST64(0x8080802000008000), CONST64(0x8080800080008000), CONST64(0x8080802080008000),
  CONST64(0x8080800000808000), CONST64(0x8080802000808000), CONST64(0x8080800080808000), CONST64(0x8080802080808000),
  CONST64(0x8080800000000080), CONST64(0x8080802000000080), CONST64(0x8080800080000080), CONST64(0x8080802080000080),
  CONST64(0x8080800000800080), CONST64(0x8080802000800080), CONST64(0x8080800080800080), CONST64(0x8080802080800080),
  CONST64(0x8080800000008080), CONST64(0x8080802000008080), CONST64(0x8080800080008080), CONST64(0x8080802080008080),
  CONST64(0x8080800000808080), CONST64(0x8080802000808080), CONST64(0x8080800080808080), CONST64(0x8080802080808080)
  },
{ CONST64(0x0000000000000000), CONST64(0x0000004000000000), CONST64(0x0000000001000000), CONST64(0x0000004001000000),
  CONST64(0x0000000000010000), CONST64(0x0000004000010000), CONST64(0x0000000001010000), CONST64(0x0000004001010000),
  CONST64(0x0000000000000100), CONST64(0x0000004000000100), CONST64(0x0000000001000100), CONST64(0x0000004001000100),
  CONST64(0x0000000000010100), CONST64(0x0000004000010100), CONST64(0x0000000001010100), CONST64(0x0000004001010100),
  CONST64(0x0000000000000001), CONST64(0x0000004000000001), CONST64(0x0000000001000001), CONST64(0x0000004001000001),
  CONST64(0x0000000000010001), CONST64(0x0000004000010001), CONST64(0x0000000001010001), CONST64(0x0000004001010001),
  CONST64(0x0000000000000101), CONST64(0x0000004000000101), CONST64(0x0000000001000101), CONST64(0x0000004001000101),
  CONST64(0x0000000000010101), CONST64(0x0000004000010101), CONST64(0x0000000001010101), CONST64(0x0000004001010101),
  CONST64(0x0100000000000000), CONST64(0x0100004000000000), CONST64(0x0100000001000000), CONST64(0x0100004001000000),
  CONST64(0x0100000000010000), CONST64(0x0100004000010000), CONST64(0x0100000001010000), CONST64(0x0100004001010000),
  CONST64(0x0100000000000100), CONST64(0x0100004000000100), CONST64(0x0100000001000100), CONST64(0x0100004001000100),
  CONST64(0x0100000000010100), CONST64(0x0100004000010100), CONST64(0x0100000001010100), CONST64(0x0100004001010100),
  CONST64(0x0100000000000001), CONST64(0x0100004000000001), CONST64(0x0100000001000001), CONST64(0x0100004001000001),
  CONST64(0x0100000000010001), CONST64(0x0100004000010001), CONST64(0x0100000001010001), CONST64(0x0100004001010001),
  CONST64(0x0100000000000101), CONST64(0x0100004000000101), CONST64(0x0100000001000101), CONST64(0x0100004001000101),
  CONST64(0x0100000000010101), CONST64(0x0100004000010101), CONST64(0x0100000001010101), CONST64(0x0100004001010101),
  CONST64(0x0001000000000000), CONST64(0x0001004000000000), CONST64(0x0001000001000000), CONST64(0x0001004001000000),
  CONST64(0x0001000000010000), CONST64(0x0001004000010000), CONST64(0x0001000001010000), CONST64(0x0001004001010000),
  CONST64(0x0001000000000100), CONST64(0x0001004000000100), CONST64(0x0001000001000100), CONST64(0x0001004001000100),
  CONST64(0x0001000000010100), CONST64(0x0001004000010100), CONST64(0x0001000001010100), CONST64(0x0001004001010100),
  CONST64(0x0001000000000001), CONST64(0x0001004000000001), CONST64(0x0001000001000001), CONST64(0x0001004001000001),
  CONST64(0x0001000000010001), CONST64(0x0001004000010001), CONST64(0x0001000001010001), CONST64(0x0001004001010001),
  CONST64(0x0001000000000101), CONST64(0x0001004000000101), CONST64(0x0001000001000101), CONST64(0x0001004001000101),
  CONST64(0x0001000000010101), CONST64(0x0001004000010101), CONST64(0x0001000001010101), CONST64(0x0001004001010101),
  CONST64(0x0101000000000000), CONST64(0x0101004000000000), CONST64(0x0101000001000000), CONST64(0x0101004001000000),
  CONST64(0x0101000000010000), CONST64(0x0101004000010000), CONST64(0x0101000001010000), CONST64(0x0101004001010000),
  CONST64(0x0101000000000100), CONST64(0x0101004000000100), CONST64(0x0101000001000100), CONST64(0x0101004001000100),
  CONST64(0x0101000000010100), CONST64(0x0101004000010100), CONST64(0x0101000001010100), CONST64(0x0101004001010100),
  CONST64(0x0101000000000001), CONST64(0x0101004000000001), CONST64(0x0101000001000001), CONST64(0x0101004001000001),
  CONST64(0x0101000000010001), CONST64(0x0101004000010001), CONST64(0x0101000001010001), CONST64(0x0101004001010001),
  CONST64(0x0101000000000101), CONST64(0x0101004000000101), CONST64(0x0101000001000101), CONST64(0x0101004001000101),
  CONST64(0x0101000000010101), CONST64(0x0101004000010101), CONST64(0x0101000001010101), CONST64(0x0101004001010101),
  CONST64(0x0000010000000000), CONST64(0x0000014000000000), CONST64(0x0000010001000000), CONST64(0x0000014001000000),
  CONST64(0x0000010000010000), CONST64(0x0000014000010000), CONST64(0x0000010001010000), CONST64(0x0000014001010000),
  CONST64(0x0000010000000100), CONST64(0x0000014000000100), CONST64(0x0000010001000100), CONST64(0x0000014001000100),
  CONST64(0x0000010000010100), CONST64(0x0000014000010100), CONST64(0x0000010001010100), CONST64(0x0000014001010100),
  CONST64(0x0000010000000001), CONST64(0x0000014000000001), CONST64(0x0000010001000001), CONST64(0x0000014001000001),
  CONST64(0x0000010000010001), CONST64(0x0000014000010001), CONST64(0x0000010001010001), CONST64(0x0000014001010001),
  CONST64(0x0000010000000101), CONST64(0x0000014000000101), CONST64(0x0000010001000101), CONST64(0x0000014001000101),
  CONST64(0x0000010000010101), CONST64(0x0000014000010101), CONST64(0x0000010001010101), CONST64(0x0000014001010101),
  CONST64(0x0100010000000000), CONST64(0x0100014000000000), CONST64(0x0100010001000000), CONST64(0x0100014001000000),
  CONST64(0x0100010000010000), CONST64(0x0100014000010000), CONST64(0x0100010001010000), CONST64(0x0100014001010000),
  CONST64(0x0100010000000100), CONST64(0x0100014000000100), CONST64(0x0100010001000100), CONST64(0x0100014001000100),
  CONST64(0x0100010000010100), CONST64(0x0100014000010100), CONST64(0x0100010001010100), CONST64(0x0100014001010100),
  CONST64(0x0100010000000001), CONST64(0x0100014000000001), CONST64(0x0100010001000001), CONST64(0x0100014001000001),
  CONST64(0x0100010000010001), CONST64(0x0100014000010001), CONST64(0x0100010001010001), CONST64(0x0100014001010001),
  CONST64(0x0100010000000101), CONST64(0x0100014000000101), CONST64(0x0100010001000101), CONST64(0x0100014001000101),
  CONST64(0x0100010000010101), CONST64(0x0100014000010101), CONST64(0x0100010001010101), CONST64(0x0100014001010101),
  CONST64(0x0001010000000000), CONST64(0x0001014000000000), CONST64(0x0001010001000000), CONST64(0x0001014001000000),
  CONST64(0x0001010000010000), CONST64(0x0001014000010000), CONST64(0x0001010001010000), CONST64(0x0001014001010000),
  CONST64(0x0001010000000100), CONST64(0x0001014000000100), CONST64(0x0001010001000100), CONST64(0x0001014001000100),
  CONST64(0x0001010000010100), CONST64(0x0001014000010100), CONST64(0x0001010001010100), CONST64(0x0001014001010100),
  CONST64(0x0001010000000001), CONST64(0x0001014000000001), CONST64(0x0001010001000001), CONST64(0x0001014001000001),
  CONST64(0x0001010000010001), CONST64(0x0001014000010001), CONST64(0x0001010001010001), CONST64(0x0001014001010001),
  CONST64(0x0001010000000101), CONST64(0x0001014000000101), CONST64(0x0001010001000101), CONST64(0x0001014001000101),
  CONST64(0x0001010000010101), CONST64(0x0001014000010101), CONST64(0x0001010001010101), CONST64(0x0001014001010101),
  CONST64(0x0101010000000000), CONST64(0x0101014000000000), CONST64(0x0101010001000000), CONST64(0x0101014001000000),
  CONST64(0x0101010000010000), CONST64(0x0101014000010000), CONST64(0x0101010001010000), CONST64(0x0101014001010000),
  CONST64(0x0101010000000100), CONST64(0x0101014000000100), CONST64(0x0101010001000100), CONST64(0x0101014001000100),
  CONST64(0x0101010000010100), CONST64(0x0101014000010100), CONST64(0x0101010001010100), CONST64(0x0101014001010100),
  CONST64(0x0101010000000001), CONST64(0x0101014000000001), CONST64(0x0101010001000001), CONST64(0x0101014001000001),
  CONST64(0x0101010000010001), CONST64(0x0101014000010001), CONST64(0x0101010001010001), CONST64(0x0101014001010001),
  CONST64(0x0101010000000101), CONST64(0x0101014000000101), CONST64(0x0101010001000101), CONST64(0x0101014001000101),
  CONST64(0x0101010000010101), CONST64(0x0101014000010101), CONST64(0x0101010001010101), CONST64(0x0101014001010101)
  },
{ CONST64(0x0000000000000000), CONST64(0x0000000100000000), CONST64(0x0000000004000000), CONST64(0x0000000104000000),
  CONST64(0x0000000000040000), CONST64(0x0000000100040000), CONST64(0x0000000004040000), CONST64(0x0000000104040000),
  CONST64(0x0000000000000400), CONST64(0x0000000100000400), CONST64(0x0000000004000400), CONST64(0x0000000104000400),
  CONST64(0x0000000000040400), CONST64(0x0000000100040400), CONST64(0x0000000004040400), CONST64(0x0000000104040400),
  CONST64(0x0000000000000004), CONST64(0x0000000100000004), CONST64(0x0000000004000004), CONST64(0x0000000104000004),
  CONST64(0x0000000000040004), CONST64(0x0000000100040004), CONST64(0x0000000004040004), CONST64(0x0000000104040004),
  CONST64(0x0000000000000404), CONST64(0x0000000100000404), CONST64(0x0000000004000404), CONST64(0x0000000104000404),
  CONST64(0x0000000000040404), CONST64(0x0000000100040404), CONST64(0x0000000004040404), CONST64(0x0000000104040404),
  CONST64(0x0400000000000000), CONST64(0x0400000100000000), CONST64(0x0400000004000000), CONST64(0x0400000104000000),
  CONST64(0x0400000000040000), CONST64(0x0400000100040000), CONST64(0x0400000004040000), CONST64(0x0400000104040000),
  CONST64(0x0400000000000400), CONST64(0x0400000100000400), CONST64(0x0400000004000400), CONST64(0x0400000104000400),
  CONST64(0x0400000000040400), CONST64(0x0400000100040400), CONST64(0x0400000004040400), CONST64(0x0400000104040400),
  CONST64(0x0400000000000004), CONST64(0x0400000100000004), CONST64(0x0400000004000004), CONST64(0x0400000104000004),
  CONST64(0x0400000000040004), CONST64(0x0400000100040004), CONST64(0x0400000004040004), CONST64(0x0400000104040004),
  CONST64(0x0400000000000404), CONST64(0x0400000100000404), CONST64(0x0400000004000404), CONST64(0x0400000104000404),
  CONST64(0x0400000000040404), CONST64(0x0400000100040404), CONST64(0x0400000004040404), CONST64(0x0400000104040404),
  CONST64(0x0004000000000000), CONST64(0x0004000100000000), CONST64(0x0004000004000000), CONST64(0x0004000104000000),
  CONST64(0x0004000000040000), CONST64(0x0004000100040000), CONST64(0x0004000004040000), CONST64(0x0004000104040000),
  CONST64(0x0004000000000400), CONST64(0x0004000100000400), CONST64(0x0004000004000400), CONST64(0x0004000104000400),
  CONST64(0x0004000000040400), CONST64(0x0004000100040400), CONST64(0x0004000004040400), CONST64(0x0004000104040400),
  CONST64(0x0004000000000004), CONST64(0x0004000100000004), CONST64(0x0004000004000004), CONST64(0x0004000104000004),
  CONST64(0x0004000000040004), CONST64(0x0004000100040004), CONST64(0x0004000004040004), CONST64(0x0004000104040004),
  CONST64(0x0004000000000404), CONST64(0x0004000100000404), CONST64(0x0004000004000404), CONST64(0x0004000104000404),
  CONST64(0x0004000000040404), CONST64(0x0004000100040404), CONST64(0x0004000004040404), CONST64(0x0004000104040404),
  CONST64(0x0404000000000000), CONST64(0x0404000100000000), CONST64(0x0404000004000000), CONST64(0x0404000104000000),
  CONST64(0x0404000000040000), CONST64(0x0404000100040000), CONST64(0x0404000004040000), CONST64(0x0404000104040000),
  CONST64(0x0404000000000400), CONST64(0x0404000100000400), CONST64(0x0404000004000400), CONST64(0x0404000104000400),
  CONST64(0x0404000000040400), CONST64(0x0404000100040400), CONST64(0x0404000004040400), CONST64(0x0404000104040400),
  CONST64(0x0404000000000004), CONST64(0x0404000100000004), CONST64(0x0404000004000004), CONST64(0x0404000104000004),
  CONST64(0x0404000000040004), CONST64(0x0404000100040004), CONST64(0x0404000004040004), CONST64(0x0404000104040004),
  CONST64(0x0404000000000404), CONST64(0x0404000100000404), CONST64(0x0404000004000404), CONST64(0x0404000104000404),
  CONST64(0x0404000000040404), CONST64(0x0404000100040404), CONST64(0x0404000004040404), CONST64(0x0404000104040404),
  CONST64(0x0000040000000000), CONST64(0x0000040100000000), CONST64(0x0000040004000000), CONST64(0x0000040104000000),
  CONST64(0x0000040000040000), CONST64(0x0000040100040000), CONST64(0x0000040004040000), CONST64(0x0000040104040000),
  CONST64(0x0000040000000400), CONST64(0x0000040100000400), CONST64(0x0000040004000400), CONST64(0x0000040104000400),
  CONST64(0x0000040000040400), CONST64(0x0000040100040400), CONST64(0x0000040004040400), CONST64(0x0000040104040400),
  CONST64(0x0000040000000004), CONST64(0x0000040100000004), CONST64(0x0000040004000004), CONST64(0x0000040104000004),
  CONST64(0x0000040000040004), CONST64(0x0000040100040004), CONST64(0x0000040004040004), CONST64(0x0000040104040004),
  CONST64(0x0000040000000404), CONST64(0x0000040100000404), CONST64(0x0000040004000404), CONST64(0x0000040104000404),
  CONST64(0x0000040000040404), CONST64(0x0000040100040404), CONST64(0x0000040004040404), CONST64(0x0000040104040404),
  CONST64(0x0400040000000000), CONST64(0x0400040100000000), CONST64(0x0400040004000000), CONST64(0x0400040104000000),
  CONST64(0x0400040000040000), CONST64(0x0400040100040000), CONST64(0x0400040004040000), CONST64(0x0400040104040000),
  CONST64(0x0400040000000400), CONST64(0x0400040100000400), CONST64(0x0400040004000400), CONST64(0x0400040104000400),
  CONST64(0x0400040000040400), CONST64(0x0400040100040400), CONST64(0x0400040004040400), CONST64(0x0400040104040400),
  CONST64(0x0400040000000004), CONST64(0x0400040100000004), CONST64(0x0400040004000004), CONST64(0x0400040104000004),
  CONST64(0x0400040000040004), CONST64(0x0400040100040004), CONST64(0x0400040004040004), CONST64(0x0400040104040004),
  CONST64(0x0400040000000404), CONST64(0x0400040100000404), CONST64(0x0400040004000404), CONST64(0x0400040104000404),
  CONST64(0x0400040000040404), CONST64(0x0400040100040404), CONST64(0x0400040004040404), CONST64(0x0400040104040404),
  CONST64(0x0004040000000000), CONST64(0x0004040100000000), CONST64(0x0004040004000000), CONST64(0x0004040104000000),
  CONST64(0x0004040000040000), CONST64(0x0004040100040000), CONST64(0x0004040004040000), CONST64(0x0004040104040000),
  CONST64(0x0004040000000400), CONST64(0x0004040100000400), CONST64(0x0004040004000400), CONST64(0x0004040104000400),
  CONST64(0x0004040000040400), CONST64(0x0004040100040400), CONST64(0x0004040004040400), CONST64(0x0004040104040400),
  CONST64(0x0004040000000004), CONST64(0x0004040100000004), CONST64(0x0004040004000004), CONST64(0x0004040104000004),
  CONST64(0x0004040000040004), CONST64(0x0004040100040004), CONST64(0x0004040004040004), CONST64(0x0004040104040004),
  CONST64(0x0004040000000404), CONST64(0x0004040100000404), CONST64(0x0004040004000404), CONST64(0x0004040104000404),
  CONST64(0x0004040000040404), CONST64(0x0004040100040404), CONST64(0x0004040004040404), CONST64(0x0004040104040404),
  CONST64(0x0404040000000000), CONST64(0x0404040100000000), CONST64(0x0404040004000000), CONST64(0x0404040104000000),
  CONST64(0x0404040000040000), CONST64(0x0404040100040000), CONST64(0x0404040004040000), CONST64(0x0404040104040000),
  CONST64(0x0404040000000400), CONST64(0x0404040100000400), CONST64(0x0404040004000400), CONST64(0x0404040104000400),
  CONST64(0x0404040000040400), CONST64(0x0404040100040400), CONST64(0x0404040004040400), CONST64(0x0404040104040400),
  CONST64(0x0404040000000004), CONST64(0x0404040100000004), CONST64(0x0404040004000004), CONST64(0x0404040104000004),
  CONST64(0x0404040000040004), CONST64(0x0404040100040004), CONST64(0x0404040004040004), CONST64(0x0404040104040004),
  CONST64(0x0404040000000404), CONST64(0x0404040100000404), CONST64(0x0404040004000404), CONST64(0x0404040104000404),
  CONST64(0x0404040000040404), CONST64(0x0404040100040404), CONST64(0x0404040004040404), CONST64(0x0404040104040404)
  },
{ CONST64(0x0000000000000000), CONST64(0x0000000400000000), CONST64(0x0000000010000000), CONST64(0x0000000410000000),
  CONST64(0x0000000000100000), CONST64(0x0000000400100000), CONST64(0x0000000010100000), CONST64(0x0000000410100000),
  CONST64(0x0000000000001000), CONST64(0x0000000400001000), CONST64(0x0000000010001000), CONST64(0x0000000410001000),
  CONST64(0x0000000000101000), CONST64(0x0000000400101000), CONST64(0x0000000010101000), CONST64(0x0000000410101000),
  CONST64(0x0000000000000010), CONST64(0x0000000400000010), CONST64(0x0000000010000010), CONST64(0x0000000410000010),
  CONST64(0x0000000000100010), CONST64(0x0000000400100010), CONST64(0x0000000010100010), CONST64(0x0000000410100010),
  CONST64(0x0000000000001010), CONST64(0x0000000400001010), CONST64(0x0000000010001010), CONST64(0x0000000410001010),
  CONST64(0x0000000000101010), CONST64(0x0000000400101010), CONST64(0x0000000010101010), CONST64(0x0000000410101010),
  CONST64(0x1000000000000000), CONST64(0x1000000400000000), CONST64(0x1000000010000000), CONST64(0x1000000410000000),
  CONST64(0x1000000000100000), CONST64(0x1000000400100000), CONST64(0x1000000010100000), CONST64(0x1000000410100000),
  CONST64(0x1000000000001000), CONST64(0x1000000400001000), CONST64(0x1000000010001000), CONST64(0x1000000410001000),
  CONST64(0x1000000000101000), CONST64(0x1000000400101000), CONST64(0x1000000010101000), CONST64(0x1000000410101000),
  CONST64(0x1000000000000010), CONST64(0x1000000400000010), CONST64(0x1000000010000010), CONST64(0x1000000410000010),
  CONST64(0x1000000000100010), CONST64(0x1000000400100010), CONST64(0x1000000010100010), CONST64(0x1000000410100010),
  CONST64(0x1000000000001010), CONST64(0x1000000400001010), CONST64(0x1000000010001010), CONST64(0x1000000410001010),
  CONST64(0x1000000000101010), CONST64(0x1000000400101010), CONST64(0x1000000010101010), CONST64(0x1000000410101010),
  CONST64(0x0010000000000000), CONST64(0x0010000400000000), CONST64(0x0010000010000000), CONST64(0x0010000410000000),
  CONST64(0x0010000000100000), CONST64(0x0010000400100000), CONST64(0x0010000010100000), CONST64(0x0010000410100000),
  CONST64(0x0010000000001000), CONST64(0x0010000400001000), CONST64(0x0010000010001000), CONST64(0x0010000410001000),
  CONST64(0x0010000000101000), CONST64(0x0010000400101000), CONST64(0x0010000010101000), CONST64(0x0010000410101000),
  CONST64(0x0010000000000010), CONST64(0x0010000400000010), CONST64(0x0010000010000010), CONST64(0x0010000410000010),
  CONST64(0x0010000000100010), CONST64(0x0010000400100010), CONST64(0x0010000010100010), CONST64(0x0010000410100010),
  CONST64(0x0010000000001010), CONST64(0x0010000400001010), CONST64(0x0010000010001010), CONST64(0x0010000410001010),
  CONST64(0x0010000000101010), CONST64(0x0010000400101010), CONST64(0x0010000010101010), CONST64(0x0010000410101010),
  CONST64(0x1010000000000000), CONST64(0x1010000400000000), CONST64(0x1010000010000000), CONST64(0x1010000410000000),
  CONST64(0x1010000000100000), CONST64(0x1010000400100000), CONST64(0x1010000010100000), CONST64(0x1010000410100000),
  CONST64(0x1010000000001000), CONST64(0x1010000400001000), CONST64(0x1010000010001000), CONST64(0x1010000410001000),
  CONST64(0x1010000000101000), CONST64(0x1010000400101000), CONST64(0x1010000010101000), CONST64(0x1010000410101000),
  CONST64(0x1010000000000010), CONST64(0x1010000400000010), CONST64(0x1010000010000010), CONST64(0x1010000410000010),
  CONST64(0x1010000000100010), CONST64(0x1010000400100010), CONST64(0x1010000010100010), CONST64(0x1010000410100010),
  CONST64(0x1010000000001010), CONST64(0x1010000400001010), CONST64(0x1010000010001010), CONST64(0x1010000410001010),
  CONST64(0x1010000000101010), CONST64(0x1010000400101010), CONST64(0x1010000010101010), CONST64(0x1010000410101010),
  CONST64(0x0000100000000000), CONST64(0x0000100400000000), CONST64(0x0000100010000000), CONST64(0x0000100410000000),
  CONST64(0x0000100000100000), CONST64(0x0000100400100000), CONST64(0x0000100010100000), CONST64(0x0000100410100000),
  CONST64(0x0000100000001000), CONST64(0x0000100400001000), CONST64(0x0000100010001000), CONST64(0x0000100410001000),
  CONST64(0x0000100000101000), CONST64(0x0000100400101000), CONST64(0x0000100010101000), CONST64(0x0000100410101000),
  CONST64(0x0000100000000010), CONST64(0x0000100400000010), CONST64(0x0000100010000010), CONST64(0x0000100410000010),
  CONST64(0x0000100000100010), CONST64(0x0000100400100010), CONST64(0x0000100010100010), CONST64(0x0000100410100010),
  CONST64(0x0000100000001010), CONST64(0x0000100400001010), CONST64(0x0000100010001010), CONST64(0x0000100410001010),
  CONST64(0x0000100000101010), CONST64(0x0000100400101010), CONST64(0x0000100010101010), CONST64(0x0000100410101010),
  CONST64(0x1000100000000000), CONST64(0x1000100400000000), CONST64(0x1000100010000000), CONST64(0x1000100410000000),
  CONST64(0x1000100000100000), CONST64(0x1000100400100000), CONST64(0x1000100010100000), CONST64(0x1000100410100000),
  CONST64(0x1000100000001000), CONST64(0x1000100400001000), CONST64(0x1000100010001000), CONST64(0x1000100410001000),
  CONST64(0x1000100000101000), CONST64(0x1000100400101000), CONST64(0x1000100010101000), CONST64(0x1000100410101000),
  CONST64(0x1000100000000010), CONST64(0x1000100400000010), CONST64(0x1000100010000010), CONST64(0x1000100410000010),
  CONST64(0x1000100000100010), CONST64(0x1000100400100010), CONST64(0x1000100010100010), CONST64(0x1000100410100010),
  CONST64(0x1000100000001010), CONST64(0x1000100400001010), CONST64(0x1000100010001010), CONST64(0x1000100410001010),
  CONST64(0x1000100000101010), CONST64(0x1000100400101010), CONST64(0x1000100010101010), CONST64(0x1000100410101010),
  CONST64(0x0010100000000000), CONST64(0x0010100400000000), CONST64(0x0010100010000000), CONST64(0x0010100410000000),
  CONST64(0x0010100000100000), CONST64(0x0010100400100000), CONST64(0x0010100010100000), CONST64(0x0010100410100000),
  CONST64(0x0010100000001000), CONST64(0x0010100400001000), CONST64(0x0010100010001000), CONST64(0x0010100410001000),
  CONST64(0x0010100000101000), CONST64(0x0010100400101000), CONST64(0x0010100010101000), CONST64(0x0010100410101000),
  CONST64(0x0010100000000010), CONST64(0x0010100400000010), CONST64(0x0010100010000010), CONST64(0x0010100410000010),
  CONST64(0x0010100000100010), CONST64(0x0010100400100010), CONST64(0x0010100010100010), CONST64(0x0010100410100010),
  CONST64(0x0010100000001010), CONST64(0x0010100400001010), CONST64(0x0010100010001010), CONST64(0x0010100410001010),
  CONST64(0x0010100000101010), CONST64(0x0010100400101010), CONST64(0x0010100010101010), CONST64(0x0010100410101010),
  CONST64(0x1010100000000000), CONST64(0x1010100400000000), CONST64(0x1010100010000000), CONST64(0x1010100410000000),
  CONST64(0x1010100000100000), CONST64(0x1010100400100000), CONST64(0x1010100010100000), CONST64(0x1010100410100000),
  CONST64(0x1010100000001000), CONST64(0x1010100400001000), CONST64(0x1010100010001000), CONST64(0x1010100410001000),
  CONST64(0x1010100000101000), CONST64(0x1010100400101000), CONST64(0x1010100010101000), CONST64(0x1010100410101000),
  CONST64(0x1010100000000010), CONST64(0x1010100400000010), CONST64(0x1010100010000010), CONST64(0x1010100410000010),
  CONST64(0x1010100000100010), CONST64(0x1010100400100010), CONST64(0x1010100010100010), CONST64(0x1010100410100010),
  CONST64(0x1010100000001010), CONST64(0x1010100400001010), CONST64(0x1010100010001010), CONST64(0x1010100410001010),
  CONST64(0x1010100000101010), CONST64(0x1010100400101010), CONST64(0x1010100010101010), CONST64(0x1010100410101010)
  },
{ CONST64(0x0000000000000000), CONST64(0x0000001000000000), CONST64(0x0000000040000000), CONST64(0x0000001040000000),
  CONST64(0x0000000000400000), CONST64(0x0000001000400000), CONST64(0x0000000040400000), CONST64(0x0000001040400000),
  CONST64(0x0000000000004000), CONST64(0x0000001000004000), CONST64(0x0000000040004000), CONST64(0x0000001040004000),
  CONST64(0x0000000000404000), CONST64(0x0000001000404000), CONST64(0x0000000040404000), CONST64(0x0000001040404000),
  CONST64(0x0000000000000040), CONST64(0x0000001000000040), CONST64(0x0000000040000040), CONST64(0x0000001040000040),
  CONST64(0x0000000000400040), CONST64(0x0000001000400040), CONST64(0x0000000040400040), CONST64(0x0000001040400040),
  CONST64(0x0000000000004040), CONST64(0x0000001000004040), CONST64(0x0000000040004040), CONST64(0x0000001040004040),
  CONST64(0x0000000000404040), CONST64(0x0000001000404040), CONST64(0x0000000040404040), CONST64(0x0000001040404040),
  CONST64(0x4000000000000000), CONST64(0x4000001000000000), CONST64(0x4000000040000000), CONST64(0x4000001040000000),
  CONST64(0x4000000000400000), CONST64(0x4000001000400000), CONST64(0x4000000040400000), CONST64(0x4000001040400000),
  CONST64(0x4000000000004000), CONST64(0x4000001000004000), CONST64(0x4000000040004000), CONST64(0x4000001040004000),
  CONST64(0x4000000000404000), CONST64(0x4000001000404000), CONST64(0x4000000040404000), CONST64(0x4000001040404000),
  CONST64(0x4000000000000040), CONST64(0x4000001000000040), CONST64(0x4000000040000040), CONST64(0x4000001040000040),
  CONST64(0x4000000000400040), CONST64(0x4000001000400040), CONST64(0x4000000040400040), CONST64(0x4000001040400040),
  CONST64(0x4000000000004040), CONST64(0x4000001000004040), CONST64(0x4000000040004040), CONST64(0x4000001040004040),
  CONST64(0x4000000000404040), CONST64(0x4000001000404040), CONST64(0x4000000040404040), CONST64(0x4000001040404040),
  CONST64(0x0040000000000000), CONST64(0x0040001000000000), CONST64(0x0040000040000000), CONST64(0x0040001040000000),
  CONST64(0x0040000000400000), CONST64(0x0040001000400000), CONST64(0x0040000040400000), CONST64(0x0040001040400000),
  CONST64(0x0040000000004000), CONST64(0x0040001000004000), CONST64(0x0040000040004000), CONST64(0x0040001040004000),
  CONST64(0x0040000000404000), CONST64(0x0040001000404000), CONST64(0x0040000040404000), CONST64(0x0040001040404000),
  CONST64(0x0040000000000040), CONST64(0x0040001000000040), CONST64(0x0040000040000040), CONST64(0x0040001040000040),
  CONST64(0x0040000000400040), CONST64(0x0040001000400040), CONST64(0x0040000040400040), CONST64(0x0040001040400040),
  CONST64(0x0040000000004040), CONST64(0x0040001000004040), CONST64(0x0040000040004040), CONST64(0x0040001040004040),
  CONST64(0x0040000000404040), CONST64(0x0040001000404040), CONST64(0x0040000040404040), CONST64(0x0040001040404040),
  CONST64(0x4040000000000000), CONST64(0x4040001000000000), CONST64(0x4040000040000000), CONST64(0x4040001040000000),
  CONST64(0x4040000000400000), CONST64(0x4040001000400000), CONST64(0x4040000040400000), CONST64(0x4040001040400000),
  CONST64(0x4040000000004000), CONST64(0x4040001000004000), CONST64(0x4040000040004000), CONST64(0x4040001040004000),
  CONST64(0x4040000000404000), CONST64(0x4040001000404000), CONST64(0x4040000040404000), CONST64(0x4040001040404000),
  CONST64(0x4040000000000040), CONST64(0x4040001000000040), CONST64(0x4040000040000040), CONST64(0x4040001040000040),
  CONST64(0x4040000000400040), CONST64(0x4040001000400040), CONST64(0x4040000040400040), CONST64(0x4040001040400040),
  CONST64(0x4040000000004040), CONST64(0x4040001000004040), CONST64(0x4040000040004040), CONST64(0x4040001040004040),
  CONST64(0x4040000000404040), CONST64(0x4040001000404040), CONST64(0x4040000040404040), CONST64(0x4040001040404040),
  CONST64(0x0000400000000000), CONST64(0x0000401000000000), CONST64(0x0000400040000000), CONST64(0x0000401040000000),
  CONST64(0x0000400000400000), CONST64(0x0000401000400000), CONST64(0x0000400040400000), CONST64(0x0000401040400000),
  CONST64(0x0000400000004000), CONST64(0x0000401000004000), CONST64(0x0000400040004000), CONST64(0x0000401040004000),
  CONST64(0x0000400000404000), CONST64(0x0000401000404000), CONST64(0x0000400040404000), CONST64(0x0000401040404000),
  CONST64(0x0000400000000040), CONST64(0x0000401000000040), CONST64(0x0000400040000040), CONST64(0x0000401040000040),
  CONST64(0x0000400000400040), CONST64(0x0000401000400040), CONST64(0x0000400040400040), CONST64(0x0000401040400040),
  CONST64(0x0000400000004040), CONST64(0x0000401000004040), CONST64(0x0000400040004040), CONST64(0x0000401040004040),
  CONST64(0x0000400000404040), CONST64(0x0000401000404040), CONST64(0x0000400040404040), CONST64(0x0000401040404040),
  CONST64(0x4000400000000000), CONST64(0x4000401000000000), CONST64(0x4000400040000000), CONST64(0x4000401040000000),
  CONST64(0x4000400000400000), CONST64(0x4000401000400000), CONST64(0x4000400040400000), CONST64(0x4000401040400000),
  CONST64(0x4000400000004000), CONST64(0x4000401000004000), CONST64(0x4000400040004000), CONST64(0x4000401040004000),
  CONST64(0x4000400000404000), CONST64(0x4000401000404000), CONST64(0x4000400040404000), CONST64(0x4000401040404000),
  CONST64(0x4000400000000040), CONST64(0x4000401000000040), CONST64(0x4000400040000040), CONST64(0x4000401040000040),
  CONST64(0x4000400000400040), CONST64(0x4000401000400040), CONST64(0x4000400040400040), CONST64(0x4000401040400040),
  CONST64(0x4000400000004040), CONST64(0x4000401000004040), CONST64(0x4000400040004040), CONST64(0x4000401040004040),
  CONST64(0x4000400000404040), CONST64(0x4000401000404040), CONST64(0x4000400040404040), CONST64(0x4000401040404040),
  CONST64(0x0040400000000000), CONST64(0x0040401000000000), CONST64(0x0040400040000000), CONST64(0x0040401040000000),
  CONST64(0x0040400000400000), CONST64(0x0040401000400000), CONST64(0x0040400040400000), CONST64(0x0040401040400000),
  CONST64(0x0040400000004000), CONST64(0x0040401000004000), CONST64(0x0040400040004000), CONST64(0x0040401040004000),
  CONST64(0x0040400000404000), CONST64(0x0040401000404000), CONST64(0x0040400040404000), CONST64(0x0040401040404000),
  CONST64(0x0040400000000040), CONST64(0x0040401000000040), CONST64(0x0040400040000040), CONST64(0x0040401040000040),
  CONST64(0x0040400000400040), CONST64(0x0040401000400040), CONST64(0x0040400040400040), CONST64(0x0040401040400040),
  CONST64(0x0040400000004040), CONST64(0x0040401000004040), CONST64(0x0040400040004040), CONST64(0x0040401040004040),
  CONST64(0x0040400000404040), CONST64(0x0040401000404040), CONST64(0x0040400040404040), CONST64(0x0040401040404040),
  CONST64(0x4040400000000000), CONST64(0x4040401000000000), CONST64(0x4040400040000000), CONST64(0x4040401040000000),
  CONST64(0x4040400000400000), CONST64(0x4040401000400000), CONST64(0x4040400040400000), CONST64(0x4040401040400000),
  CONST64(0x4040400000004000), CONST64(0x4040401000004000), CONST64(0x4040400040004000), CONST64(0x4040401040004000),
  CONST64(0x4040400000404000), CONST64(0x4040401000404000), CONST64(0x4040400040404000), CONST64(0x4040401040404000),
  CONST64(0x4040400000000040), CONST64(0x4040401000000040), CONST64(0x4040400040000040), CONST64(0x4040401040000040),
  CONST64(0x4040400000400040), CONST64(0x4040401000400040), CONST64(0x4040400040400040), CONST64(0x4040401040400040),
  CONST64(0x4040400000004040), CONST64(0x4040401000004040), CONST64(0x4040400040004040), CONST64(0x4040401040004040),
  CONST64(0x4040400000404040), CONST64(0x4040401000404040), CONST64(0x4040400040404040), CONST64(0x4040401040404040)
  }};

#endif


static void cookey(const Muint32 *raw1, Muint32 *keyout);

#ifdef LTC_CLEAN_STACK
static void _deskey(const unsigned char *key, short edf, Muint32 *keyout)
#else
static void deskey(const unsigned char *key, short edf, Muint32 *keyout)
#endif
{
    Muint32 i, j, l, m, n, kn[32];
    unsigned char pc1m[56], pcr[56];

    for (j=0; j < 56; j++) {
        l = (Muint32)pc1[j];
        m = l & 7;
        pc1m[j] = (unsigned char)((key[l >> 3U] & bytebit[m]) == bytebit[m] ? 1 : 0);
    }

    for (i=0; i < 16; i++) {
        if (edf == DE1) {
           m = (15 - i) << 1;
        } else {
           m = i << 1;
        }
        n = m + 1;
        kn[m] = kn[n] = 0L;
        for (j=0; j < 28; j++) {
            l = j + (Muint32)totrot[i];
            if (l < 28) {
               pcr[j] = pc1m[l];
            } else {
               pcr[j] = pc1m[l - 28];
            }
        }
        for (/*j = 28*/; j < 56; j++) {
            l = j + (Muint32)totrot[i];
            if (l < 56) {
               pcr[j] = pc1m[l];
            } else {
               pcr[j] = pc1m[l - 28];
            }
        }
        for (j=0; j < 24; j++)  {
            if ((int)pcr[(int)pc2[j]] != 0) {
               kn[m] |= bigbyte[j];
            }
            if ((int)pcr[(int)pc2[j+24]] != 0) {
               kn[n] |= bigbyte[j];
            }
        }
    }

    cookey(kn, keyout);
}

#ifdef LTC_CLEAN_STACK
static void deskey(const unsigned char *key, short edf, Muint32 *keyout)
{
   _deskey(key, edf, keyout);
   burn_stack(sizeof(int)*5 + sizeof(Muint32)*32 + sizeof(unsigned char)*112);
}
#endif

#ifdef LTC_CLEAN_STACK
static void _cookey(const Muint32 *raw1, Muint32 *keyout)
#else
static void cookey(const Muint32 *raw1, Muint32 *keyout)
#endif
{
    Muint32 *cook;
    const Muint32 *raw0;
    Muint32 dough[32];
    int i;

    cook = dough;
    for(i=0; i < 16; i++, raw1++)
    {
        raw0 = raw1++;
        *cook    = (*raw0 & 0x00fc0000L) << 6;
        *cook   |= (*raw0 & 0x00000fc0L) << 10;
        *cook   |= (*raw1 & 0x00fc0000L) >> 10;
        *cook++ |= (*raw1 & 0x00000fc0L) >> 6;
        *cook    = (*raw0 & 0x0003f000L) << 12;
        *cook   |= (*raw0 & 0x0000003fL) << 16;
        *cook   |= (*raw1 & 0x0003f000L) >> 4;
        *cook++ |= (*raw1 & 0x0000003fL);
    }

    memcpy(keyout, dough, sizeof dough);
}

#ifdef LTC_CLEAN_STACK
static void cookey(const Muint32 *raw1, Muint32 *keyout)
{
   _cookey(raw1, keyout);
   burn_stack(sizeof(Muint32 *) * 2 + sizeof(Muint32)*32 + sizeof(int));
}
#endif

#ifndef LTC_CLEAN_STACK
static void desfunc(Muint32 *block, const Muint32 *keys)
#else
static void _desfunc(Muint32 *block, const Muint32 *keys)
#endif
{
    Muint32 work, right, leftt;
    int cur_round;

    leftt = block[0];
    right = block[1];

#ifdef LTC_SMALL_CODE
    work = ((leftt >> 4)  ^ right) & 0x0f0f0f0fL;
    right ^= work;
    leftt ^= (work << 4);

    work = ((leftt >> 16) ^ right) & 0x0000ffffL;
    right ^= work;
    leftt ^= (work << 16);

    work = ((right >> 2)  ^ leftt) & 0x33333333L;
    leftt ^= work;
    right ^= (work << 2);

    work = ((right >> 8)  ^ leftt) & 0x00ff00ffL;
    leftt ^= work;
    right ^= (work << 8);

    right = ROLc(right, 1);
    work = (leftt ^ right) & 0xaaaaaaaaL;

    leftt ^= work;
    right ^= work;
    leftt = ROLc(leftt, 1);
#else
   {
      ulong64 tmp;
      tmp = des_ip[0][byte(leftt, 0)] ^
            des_ip[1][byte(leftt, 1)] ^
            des_ip[2][byte(leftt, 2)] ^
            des_ip[3][byte(leftt, 3)] ^
            des_ip[4][byte(right, 0)] ^
            des_ip[5][byte(right, 1)] ^
            des_ip[6][byte(right, 2)] ^
            des_ip[7][byte(right, 3)];
      leftt = (Muint32)(tmp >> 32);
      right = (Muint32)(tmp & 0xFFFFFFFFUL);
   }
#endif

    for (cur_round = 0; cur_round < 8; cur_round++) {
        work  = RORc(right, 4) ^ *keys++;
        leftt ^= SP7[work        & 0x3fL]
              ^ SP5[(work >>  8) & 0x3fL]
              ^ SP3[(work >> 16) & 0x3fL]
              ^ SP1[(work >> 24) & 0x3fL];
        work  = right ^ *keys++;
        leftt ^= SP8[ work        & 0x3fL]
              ^  SP6[(work >>  8) & 0x3fL]
              ^  SP4[(work >> 16) & 0x3fL]
              ^  SP2[(work >> 24) & 0x3fL];

        work = RORc(leftt, 4) ^ *keys++;
        right ^= SP7[ work        & 0x3fL]
              ^  SP5[(work >>  8) & 0x3fL]
              ^  SP3[(work >> 16) & 0x3fL]
              ^  SP1[(work >> 24) & 0x3fL];
        work  = leftt ^ *keys++;
        right ^= SP8[ work        & 0x3fL]
              ^  SP6[(work >>  8) & 0x3fL]
              ^  SP4[(work >> 16) & 0x3fL]
              ^  SP2[(work >> 24) & 0x3fL];
    }

#ifdef LTC_SMALL_CODE
    right = RORc(right, 1);
    work = (leftt ^ right) & 0xaaaaaaaaL;
    leftt ^= work;
    right ^= work;
    leftt = RORc(leftt, 1);
    work = ((leftt >> 8) ^ right) & 0x00ff00ffL;
    right ^= work;
    leftt ^= (work << 8);
    /* -- */
    work = ((leftt >> 2) ^ right) & 0x33333333L;
    right ^= work;
    leftt ^= (work << 2);
    work = ((right >> 16) ^ leftt) & 0x0000ffffL;
    leftt ^= work;
    right ^= (work << 16);
    work = ((right >> 4) ^ leftt) & 0x0f0f0f0fL;
    leftt ^= work;
    right ^= (work << 4);
#else
   {
      ulong64 tmp;
      tmp = des_fp[0][byte(leftt, 0)] ^
            des_fp[1][byte(leftt, 1)] ^
            des_fp[2][byte(leftt, 2)] ^
            des_fp[3][byte(leftt, 3)] ^
            des_fp[4][byte(right, 0)] ^
            des_fp[5][byte(right, 1)] ^
            des_fp[6][byte(right, 2)] ^
            des_fp[7][byte(right, 3)];
      leftt = (Muint32)(tmp >> 32);
      right = (Muint32)(tmp & 0xFFFFFFFFUL);
   }
#endif

    block[0] = right;
    block[1] = leftt;
}

#ifdef LTC_CLEAN_STACK
inline void desfunc(Muint32 *block, const Muint32 *keys)
{
   _desfunc(block, keys);
   burn_stack(sizeof(Muint32) * 4 + sizeof(int));
}
#endif

 /**
    Initialize the DES block cipher
    @param key The symmetric key you wish to pass
    @param keylen The key length in bytes
    @param num_rounds The number of rounds desired (0 for default)
    @param skey The key in as scheduled by this function.
    @return CRYPT_OK if successful
 */
inline int des_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey)
{
    LTC_ARGCHK(key != NULL);
    LTC_ARGCHK(skey != NULL);

    if (num_rounds != 0 && num_rounds != 16) {
        return CRYPT_INVALID_ROUNDS;
    }

    if (keylen != 8) {
        return CRYPT_INVALID_KEYSIZE;
    }

    deskey(key, EN0, skey->des.ek);
    deskey(key, DE1, skey->des.dk);

    return CRYPT_OK;
}

#ifdef DES3
 /**
    Initialize the 3DES-EDE block cipher
    @param key The symmetric key you wish to pass
    @param keylen The key length in bytes
    @param num_rounds The number of rounds desired (0 for default)
    @param skey The key in as scheduled by this function.
    @return CRYPT_OK if successful
 */
inline int des3_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey)
{
    LTC_ARGCHK(key != NULL);
    LTC_ARGCHK(skey != NULL);

    if(num_rounds != 0 && num_rounds != 16) {
        return CRYPT_INVALID_ROUNDS;
    }

    if (keylen != 24) {
        return CRYPT_INVALID_KEYSIZE;
    }

    deskey(key,    EN0, skey->des3.ek[0]);
    deskey(key+8,  DE1, skey->des3.ek[1]);
    deskey(key+16, EN0, skey->des3.ek[2]);

    deskey(key,    DE1, skey->des3.dk[2]);
    deskey(key+8,  EN0, skey->des3.dk[1]);
    deskey(key+16, DE1, skey->des3.dk[0]);

    return CRYPT_OK;
}

#endif

/**
  Encrypts a block of text with DES
  @param pt The input plaintext (8 bytes)
  @param ct The output ciphertext (8 bytes)
  @param skey The key as scheduled
  @return CRYPT_OK if successful
*/
inline int des_ecb_encrypt(const unsigned char *pt, unsigned char *ct, symmetric_key *skey)
{
    Muint32 work[2];
    LTC_ARGCHK(pt   != NULL);
    LTC_ARGCHK(ct   != NULL);
    LTC_ARGCHK(skey != NULL);
    LOAD32H(work[0], pt+0);
    LOAD32H(work[1], pt+4);
    desfunc(work, skey->des.ek);
    STORE32H(work[0],ct+0);
    STORE32H(work[1],ct+4);
    return CRYPT_OK;
}

/**
  Decrypts a block of text with DES
  @param ct The input ciphertext (8 bytes)
  @param pt The output plaintext (8 bytes)
  @param skey The key as scheduled
  @return CRYPT_OK if successful
*/
inline int des_ecb_decrypt(const unsigned char *ct, unsigned char *pt, symmetric_key *skey)
{
    Muint32 work[2];
    LTC_ARGCHK(pt   != NULL);
    LTC_ARGCHK(ct   != NULL);
    LTC_ARGCHK(skey != NULL);
    LOAD32H(work[0], ct+0);
    LOAD32H(work[1], ct+4);
    desfunc(work, skey->des.dk);
    STORE32H(work[0],pt+0);
    STORE32H(work[1],pt+4);
    return CRYPT_OK;
}

#ifdef DES3

/**
  Encrypts a block of text with 3DES-EDE
  @param pt The input plaintext (8 bytes)
  @param ct The output ciphertext (8 bytes)
  @param skey The key as scheduled
  @return CRYPT_OK if successful
*/
inline int des3_ecb_encrypt(const unsigned char *pt, unsigned char *ct, symmetric_key *skey)
{
    Muint32 work[2];

    LTC_ARGCHK(pt   != NULL);
    LTC_ARGCHK(ct   != NULL);
    LTC_ARGCHK(skey != NULL);
    LOAD32H(work[0], pt+0);
    LOAD32H(work[1], pt+4);
    desfunc(work, skey->des3.ek[0]);
    desfunc(work, skey->des3.ek[1]);
    desfunc(work, skey->des3.ek[2]);
    STORE32H(work[0],ct+0);
    STORE32H(work[1],ct+4);
    return CRYPT_OK;
}

/**
  Decrypts a block of text with 3DES-EDE
  @param ct The input ciphertext (8 bytes)
  @param pt The output plaintext (8 bytes)
  @param skey The key as scheduled
  @return CRYPT_OK if successful
*/
inline int des3_ecb_decrypt(const unsigned char *ct, unsigned char *pt, symmetric_key *skey)
{
    Muint32 work[2];
    LTC_ARGCHK(pt   != NULL);
    LTC_ARGCHK(ct   != NULL);
    LTC_ARGCHK(skey != NULL);
    LOAD32H(work[0], ct+0);
    LOAD32H(work[1], ct+4);
    desfunc(work, skey->des3.dk[0]);
    desfunc(work, skey->des3.dk[1]);
    desfunc(work, skey->des3.dk[2]);
    STORE32H(work[0],pt+0);
    STORE32H(work[1],pt+4);
    return CRYPT_OK;
}

#endif

/**
  Performs a self-test of the DES block cipher
  @return CRYPT_OK if functional, CRYPT_NOP if self-test has been disabled
*/
inline int des_test(void)
{
 #ifndef LTC_TEST
    return CRYPT_NOP;
 #else
    int err;
    static const struct des_test_case {
        int num, mode; /* mode 1 = encrypt */
        unsigned char key[8], txt[8], out[8];
    } cases[] = {
        { 1, 1,     { 0x10, 0x31, 0x6E, 0x02, 0x8C, 0x8F, 0x3B, 0x4A },
                    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                    { 0x82, 0xDC, 0xBA, 0xFB, 0xDE, 0xAB, 0x66, 0x02 } },
        { 2, 1,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0x95, 0xF8, 0xA5, 0xE5, 0xDD, 0x31, 0xD9, 0x00 },
                    { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
        { 3, 1,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0xDD, 0x7F, 0x12, 0x1C, 0xA5, 0x01, 0x56, 0x19 },
                    { 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
        { 4, 1,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0x2E, 0x86, 0x53, 0x10, 0x4F, 0x38, 0x34, 0xEA },
                    { 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
        { 5, 1,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0x4B, 0xD3, 0x88, 0xFF, 0x6C, 0xD8, 0x1D, 0x4F },
                    { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
        { 6, 1,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0x20, 0xB9, 0xE7, 0x67, 0xB2, 0xFB, 0x14, 0x56 },
                    { 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
        { 7, 1,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0x55, 0x57, 0x93, 0x80, 0xD7, 0x71, 0x38, 0xEF },
                    { 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
        { 8, 1,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0x6C, 0xC5, 0xDE, 0xFA, 0xAF, 0x04, 0x51, 0x2F },
                    { 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
        { 9, 1,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0x0D, 0x9F, 0x27, 0x9B, 0xA5, 0xD8, 0x72, 0x60 },
                    { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
        {10, 1,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0xD9, 0x03, 0x1B, 0x02, 0x71, 0xBD, 0x5A, 0x0A },
                    { 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },

        { 1, 0,     { 0x10, 0x31, 0x6E, 0x02, 0x8C, 0x8F, 0x3B, 0x4A },
                    { 0x82, 0xDC, 0xBA, 0xFB, 0xDE, 0xAB, 0x66, 0x02 },
                    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
        { 2, 0,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                    { 0x95, 0xF8, 0xA5, 0xE5, 0xDD, 0x31, 0xD9, 0x00 } },
        { 3, 0,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                    { 0xDD, 0x7F, 0x12, 0x1C, 0xA5, 0x01, 0x56, 0x19 } },
        { 4, 0,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                    { 0x2E, 0x86, 0x53, 0x10, 0x4F, 0x38, 0x34, 0xEA } },
        { 5, 0,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                    { 0x4B, 0xD3, 0x88, 0xFF, 0x6C, 0xD8, 0x1D, 0x4F } },
        { 6, 0,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                    { 0x20, 0xB9, 0xE7, 0x67, 0xB2, 0xFB, 0x14, 0x56 } },
        { 7, 0,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                    { 0x55, 0x57, 0x93, 0x80, 0xD7, 0x71, 0x38, 0xEF } },
        { 8, 0,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                    { 0x6C, 0xC5, 0xDE, 0xFA, 0xAF, 0x04, 0x51, 0x2F } },
        { 9, 0,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                    { 0x0D, 0x9F, 0x27, 0x9B, 0xA5, 0xD8, 0x72, 0x60 } },
        {10, 0,     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
                    { 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                    { 0xD9, 0x03, 0x1B, 0x02, 0x71, 0xBD, 0x5A, 0x0A } }

        /*** more test cases you could add if you are not convinced (the above test cases aren't really too good):

                key              plaintext        ciphertext
                0000000000000000 0000000000000000 8CA64DE9C1B123A7
                FFFFFFFFFFFFFFFF FFFFFFFFFFFFFFFF 7359B2163E4EDC58
                3000000000000000 1000000000000001 958E6E627A05557B
                1111111111111111 1111111111111111 F40379AB9E0EC533
                0123456789ABCDEF 1111111111111111 17668DFC7292532D
                1111111111111111 0123456789ABCDEF 8A5AE1F81AB8F2DD
                0000000000000000 0000000000000000 8CA64DE9C1B123A7
                FEDCBA9876543210 0123456789ABCDEF ED39D950FA74BCC4
                7CA110454A1A6E57 01A1D6D039776742 690F5B0D9A26939B
                0131D9619DC1376E 5CD54CA83DEF57DA 7A389D10354BD271
                07A1133E4A0B2686 0248D43806F67172 868EBB51CAB4599A
                3849674C2602319E 51454B582DDF440A 7178876E01F19B2A
                04B915BA43FEB5B6 42FD443059577FA2 AF37FB421F8C4095
                0113B970FD34F2CE 059B5E0851CF143A 86A560F10EC6D85B
                0170F175468FB5E6 0756D8E0774761D2 0CD3DA020021DC09
                43297FAD38E373FE 762514B829BF486A EA676B2CB7DB2B7A
                07A7137045DA2A16 3BDD119049372802 DFD64A815CAF1A0F
                04689104C2FD3B2F 26955F6835AF609A 5C513C9C4886C088
                37D06BB516CB7546 164D5E404F275232 0A2AEEAE3FF4AB77
                1F08260D1AC2465E 6B056E18759F5CCA EF1BF03E5DFA575A
                584023641ABA6176 004BD6EF09176062 88BF0DB6D70DEE56
                025816164629B007 480D39006EE762F2 A1F9915541020B56
                49793EBC79B3258F 437540C8698F3CFA 6FBF1CAFCFFD0556
                4FB05E1515AB73A7 072D43A077075292 2F22E49BAB7CA1AC
                49E95D6D4CA229BF 02FE55778117F12A 5A6B612CC26CCE4A
                018310DC409B26D6 1D9D5C5018F728C2 5F4C038ED12B2E41
                1C587F1C13924FEF 305532286D6F295A 63FAC0D034D9F793
                0101010101010101 0123456789ABCDEF 617B3A0CE8F07100
                1F1F1F1F0E0E0E0E 0123456789ABCDEF DB958605F8C8C606
                E0FEE0FEF1FEF1FE 0123456789ABCDEF EDBFD1C66C29CCC7
                0000000000000000 FFFFFFFFFFFFFFFF 355550B2150E2451
                FFFFFFFFFFFFFFFF 0000000000000000 CAAAAF4DEAF1DBAE
                0123456789ABCDEF 0000000000000000 D5D44FF720683D0D
                FEDCBA9876543210 FFFFFFFFFFFFFFFF 2A2BB008DF97C2F2

            http://www.ecs.soton.ac.uk/~prw99r/ez438/vectors.txt
        ***/
    };
    int i, y;
    unsigned char tmp[8];
    symmetric_key des;

    for(i=0; i < (int)(sizeof(cases)/sizeof(cases[0])); i++)
    {
        if ((err = des_setup(cases[i].key, 8, 0, &des)) != CRYPT_OK) {
           return err;
        }
        if (cases[i].mode != 0) {
           des_ecb_encrypt(cases[i].txt, tmp, &des);
        } else {
           des_ecb_decrypt(cases[i].txt, tmp, &des);
        }

        if (XMEMCMP(cases[i].out, tmp, sizeof(tmp)) != 0) {
           return CRYPT_FAIL_TESTVECTOR;
        }

      /* now see if we can encrypt all zero bytes 1000 times, decrypt and come back where we started */
      for (y = 0; y < 8; y++) tmp[y] = 0;
      for (y = 0; y < 1000; y++) des_ecb_encrypt(tmp, tmp, &des);
      for (y = 0; y < 1000; y++) des_ecb_decrypt(tmp, tmp, &des);
      for (y = 0; y < 8; y++) if (tmp[y] != 0) return CRYPT_FAIL_TESTVECTOR;
}

    return CRYPT_OK;
  #endif
}

#ifdef DES3

inline int des3_test(void)
{
 #ifndef LTC_TEST
    return CRYPT_NOP;
 #else
   unsigned char key[24], pt[8], ct[8], tmp[8];
   symmetric_key skey;
   int x, err;

   if ((err = des_test()) != CRYPT_OK) {
      return err;
   }

   for (x = 0; x < 8; x++) {
       pt[x] = x;
   }

   for (x = 0; x < 24; x++) {
       key[x] = x;
   }

   if ((err = des3_setup(key, 24, 0, &skey)) != CRYPT_OK) {
      return err;
   }

   des3_ecb_encrypt(pt, ct, &skey);
   des3_ecb_decrypt(ct, tmp, &skey);

   if (XMEMCMP(pt, tmp, 8) != 0) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   return CRYPT_OK;
 #endif
}

#endif

/** Terminate the context
   @param skey    The scheduled key
*/
inline void des_done(symmetric_key *skey)
{
}

#ifdef DES3

/** Terminate the context
   @param skey    The scheduled key
*/
inline void des3_done(symmetric_key *skey)
{
}

#endif

/**
  Gets suitable key size
  @param keysize [in/out] The length of the recommended key (in bytes).  This function will store the suitable size back in this variable.
  @return CRYPT_OK if the input key size is acceptable.
*/
inline int des_keysize(int *keysize)
{
    LTC_ARGCHK(keysize != NULL);
    if(*keysize < 8) {
        return CRYPT_INVALID_KEYSIZE;
    }
    *keysize = 8;
    return CRYPT_OK;
}

#ifdef DES3
/**
  Gets suitable key size
  @param keysize [in/out] The length of the recommended key (in bytes).  This function will store the suitable size back in this variable.
  @return CRYPT_OK if the input key size is acceptable.
*/
inline int des3_keysize(int *keysize)
{
    LTC_ARGCHK(keysize != NULL);
    if(*keysize < 24) {
        return CRYPT_INVALID_KEYSIZE;
    }
    *keysize = 24;
    return CRYPT_OK;
}
#endif

#endif


/* $Source: /cvs/libtom/libtomcrypt/src/ciphers/des.c,v $ */
/* $Revision: 1.13 $ */
/* $Date: 2006/11/08 23:01:06 $ */
