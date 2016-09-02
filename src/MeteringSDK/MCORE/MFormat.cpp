#include "MCOREExtern.h"
#include "MCOREDefs.h"
#ifndef M_USE_USTL
   #include <limits>
   #include <cfloat>
#endif

#ifdef max
   #undef max
   #undef min
#endif

namespace
{

#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef __int64 F_S64;
typedef unsigned __int64 F_U64;
#define F_LIKELY(x) x
#define F_UNLIKELY(x) x
#elif defined(__GNUC__)
typedef long long F_S64;
typedef unsigned long long F_U64;
#define F_LIKELY(x) __builtin_expect((x), 1)
#define F_UNLIKELY(x) __builtin_expect((x), 0)
#elif defined(ewarm)
typedef long long F_S64;
typedef unsigned long long F_U64;
#define F_LIKELY(x) x
#define F_UNLIKELY(x) x
#endif

typedef uintptr_t F_UINTPTR;

#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef F_UINTPTR F_SSIZE_T;
#else
typedef ssize_t F_SSIZE_T;
#endif

const size_t DEF_NBUF_SIZE = 1024;
const size_t FLOAT_DIGITS = 6;
const size_t EXPONENT_LENGTH = 10;
const int NDIG = 320;

const bool IS_64BIT_LONG = (sizeof(long) == 8);
const bool IS_64BIT_POINTER = (sizeof(void*) == 8);

const char LOW_DIGITS[] = "0123456789abcdef";
const char UPPER_DIGITS[] = "0123456789ABCDEF";

enum FAdjustMode { ADJUST_LEFT, ADJUST_RIGHT };

inline int Fislower(char ch) { return islower(ch); }
inline int Fisdigit(char ch) { return isdigit(ch); }
inline int Fisspace(char ch) { return isspace(ch); }

inline size_t Fstrlen(const char* s) { return strlen(s); }

inline char* Fmemcpy(char* dst, const char* src, size_t n) { return static_cast<char*>(memcpy(dst, src, n)); }
inline char* Fmemset(char* dst, char ch, size_t n) { return static_cast<char*>(memset(dst, ch, n)); }

#if !M_NO_WCHAR_T
inline int Fislower(wchar_t ch) { return iswlower(ch); }
inline int Fisdigit(wchar_t ch) { return iswdigit(ch); }
inline int Fisspace(wchar_t ch) { return iswspace(ch); }

inline size_t Fstrlen(const wchar_t* s) { return wcslen(s); }

inline wchar_t* Fmemcpy(wchar_t* dst, const wchar_t* src, size_t n) { return static_cast<wchar_t*>(wmemcpy(dst, src, n)); }
inline wchar_t* Fmemset(wchar_t* dst, wchar_t ch, size_t n) { return static_cast<wchar_t*>(wmemset(dst, ch, n)); }
#endif

using namespace std;

inline int Fisnan(double x)
{
#if defined(_MSC_VER) || defined(__BORLANDC__)
   return _isnan(x);
#else
   return isnan(x);
#endif
}

inline int Fisinf(double x)
{
#if defined(_MSC_VER) || defined(__BORLANDC__)
   return _finite(x) == 0;
#else
   return isinf(x);
#endif
}

#if !M_NO_WCHAR_T
size_t Fwcsrtombs(char* dest, const wchar_t** src, size_t len, mbstate_t* ps)
{
#if (M_OS & M_OS_WINDOWS) != 0
   M_USED_VARIABLE(ps);
   return static_cast<size_t>(WideCharToMultiByte(M_WINDOWS_CP_ACP, 0, *src, -1, dest, static_cast<int>(len), 0, 0) - 1);
#else
   return wcsrtombs(dest, src, len, ps);
#endif
}

size_t Fmbsrtowcs(wchar_t* dest, const char** src, size_t len, mbstate_t* ps)
{
#if (M_OS & M_OS_WINDOWS) != 0
   M_USED_VARIABLE(ps);
   return static_cast<size_t>(MultiByteToWideChar(M_WINDOWS_CP_ACP, 0, *src, -1, dest, static_cast<int>(len)) - 1);
#else
   return mbsrtowcs(dest, src, len, ps);
#endif
}

size_t Fwcrtomb(char* dest, const wchar_t wc, mbstate_t* ps)
{
#if (M_OS & M_OS_WINDOWS) != 0
   M_USED_VARIABLE(ps);
   return static_cast<size_t>(WideCharToMultiByte(M_WINDOWS_CP_ACP, 0, &wc, 1, dest, MB_LEN_MAX, 0, 0) - 1);
#else
   return wcrtomb(dest, wc, ps);
#endif
}
#endif

struct FFlags
{
   FAdjustMode adjust;
   bool alternateForm;
   bool alternateFormShort;
   bool printSign;
   bool printBlank;
   bool adjustPrecision;
   bool adjustWidth;
   char pad;
   char prefix;
   char dp;
   char fform;
   size_t minWidth;
   size_t precision;
};

template <typename T> inline
size_t ToDecimal(const T*& s)
{
   size_t n = (*s++ - '0');
   while ( Fisdigit(*s) )
   {
      n *= 10;
      n += (*s++ - '0');
   }
   return n;
}

template <typename N, typename T> inline
T* DoConv10(N value, T* end)
{
   T* p = end;
   do
   {
      N n = value / 10;
      *--p = static_cast<T>(value - (n * 10) + '0');
      value = n;
   }
   while ( value );

   return p;
}

template <typename> struct ToUnsigned;
template <> struct ToUnsigned<F_S64> { typedef F_U64 Type; };
template <> struct ToUnsigned<F_U64> { typedef F_U64 Type; };
template <> struct ToUnsigned<long> { typedef unsigned long Type; };
template <> struct ToUnsigned<unsigned long> { typedef unsigned long Type; };
template <> struct ToUnsigned<int> { typedef unsigned int Type; };
template <> struct ToUnsigned<unsigned int> { typedef unsigned int Type; };

template <typename N, typename T> inline
T* DoConv10Signed(N value, bool& isNeg, T* end)
{
   typedef typename ToUnsigned<N>::Type UnsignedType;
   UnsignedType magnitude;
   isNeg = (value < 0);
   if ( isNeg )
   {
      N n = value + 1;
      magnitude = static_cast<UnsignedType>(-n) + 1;
   }
   else
   {
      magnitude = static_cast<UnsignedType>(value);
   }

   return DoConv10(magnitude, end);
}

template <typename T> inline
T* Conv10(F_U64 value, bool& isNeg, T* end, size_t& stringLength)
{
   isNeg = false;

   T* p = DoConv10(value, end);
   stringLength = end - p;
   return p;
}

template <typename T> inline
T* Conv10(F_S64 value, bool& isNeg, T* end, size_t& stringLength)
{
   T* p = DoConv10Signed(value, isNeg, end);
   stringLength = end - p;
   return p;
}

template <typename T> inline
T* Conv10(unsigned long value, bool& isNeg, T* end, size_t& stringLength)
{
   isNeg = false;

   T* p = DoConv10(value, end);
   stringLength = end - p;
   return p;
}

template <typename T> inline
T* Conv10(long value, bool& isNeg, T* end, size_t& stringLength)
{
   T* p = DoConv10Signed(value, isNeg, end);
   stringLength = end - p;
   return p;
}

template <typename T> inline
T* Conv10(unsigned int value, bool& isNeg, T* end, size_t& stringLength)
{
   isNeg = false;

   T* p = DoConv10(value, end);
   stringLength = end - p;
   return p;
}

template <typename T> inline
T* Conv10(int value, bool& isNeg, T* end, size_t& stringLength)
{
   T* p = DoConv10Signed(value, isNeg, end);
   stringLength = end - p;
   return p;
}

template <typename N, typename T> inline
T* DoConvP2(N value, int nbits, char format, T* end, size_t& stringLength)
{
   int mask = (1 << nbits) - 1;
   const char* digits = (format == 'X') ? UPPER_DIGITS : LOW_DIGITS;

   T* p = end;
   do
   {
      *--p = digits[value & mask];
      value >>= nbits;
   }
   while ( value );

   stringLength = end - p;
   return p;
}

template <typename T> inline
T* ConvP2(F_U64 value, int nbits, char format, T* end, size_t& stringLength)
{
   return DoConvP2(value, nbits, format, end, stringLength);
}

template <typename T> inline
T* ConvP2(unsigned long value, int nbits, char format, T* end, size_t& stringLength)
{
   return DoConvP2(value, nbits, format, end, stringLength);
}

template <typename T> inline
T* ConvP2(unsigned int value, int nbits, char format, T* end, size_t& stringLength)
{
   return DoConvP2(value, nbits, format, end, stringLength);
}

char* DoConvFp(double arg, int ndigits, int& decpt, bool& sign, int eflag, char *buf) // cvt_r
{
   int r2 = 0;
   double fi, fj;
   char *p = &buf[0], *p1 = &buf[NDIG];

   if ( ndigits >= NDIG - 1 )
      ndigits = NDIG - 2;

   sign = (arg < 0);
   if ( sign )
      arg = -arg;

   arg = modf(arg, &fi);

   if ( fi != 0 )
   {
      while ( p1 > &buf[0] && fi != 0 )
      {
         fj = modf(fi / 10, &fi);
         *--p1 = static_cast<int>((fj + .03) * 10) + '0';
         r2++;
      }
      while ( p1 < &buf[NDIG] )
         *p++ = *p1++;
   }
   else if ( arg > 0.0 )
   {
      while ( (fj = arg * 10.0) + DBL_EPSILON < 1.0 )
      {
         arg = fj;
         r2--;
      }
   }

   p1 = &buf[ndigits];

   if ( eflag == 0 )
      p1 += r2;

   if ( p1 < &buf[0] ) {
      decpt = -ndigits;
      buf[0] = '\0';
      return buf;
   }

   decpt = r2;
   while ( p <= p1 && p < &buf[NDIG] )
   {
      arg *= 10;
      arg = modf(arg, &fj);
      *p++ = static_cast<int>(fj) + '0';
   }

   if ( p1 >= &buf[NDIG] )
   {
      buf[NDIG - 1] = '\0';
      return buf;
   }

   p = p1;
   *p1 += 5;

   while ( *p1 > '9' )
   {
      *p1 = '0';
      if ( p1 > buf )
      {
         ++ * --p1;
      }
      else
      {
         *p1 = '1';
         ++decpt;
         if ( eflag == 0 )
         {
            if (p > buf)
               *p = '0';
            p++;
         }
      }
   }

   *p = '\0';
   return buf;
}

inline char* ConvFFp(double num, int ndigits, int& decpt, bool& sign, char* buf) // fcvt_r
{
   // int sign1;
   // fcvt_r(num, ndigits, &decpt, &sign1, buf, NDIG - 1);
   // sign = sign1;
   // return buf;
   return DoConvFp(num, ndigits, decpt, sign, 0, buf);
}

inline char* ConvEFp(double num, int ndigits, int& decpt, bool& sign, char* buf) // ecvt_r
{
   // int sign1;
   // ecvt_r(num, ndigits, &decpt, &sign1, buf, NDIG - 1);
   // sign = sign1;
   // return buf;
   return DoConvFp(num, ndigits, decpt, sign, 1, buf);
}

template <typename T>
T* ConvFp(T format, double num, const FFlags& fflags, bool& isNeg, T* buf, size_t& stringLength)
{
   int decp;
   char localbuf[NDIG];
   T* s = buf;

   int precision = int(fflags.adjustPrecision ? fflags.precision : FLOAT_DIGITS);

   char* p = (format == 'f')
      ? ConvFFp(num, precision, decp, isNeg, localbuf)
      : ConvEFp(num, precision + 1, decp, isNeg, localbuf);

   if ( format == 'f' )
   {
      if ( decp <= 0 )
      {
         *s++ = '0';
         if ( precision > 0 )
         {
            *s++ = fflags.dp;
            while ( decp++ < 0 )
               *s++ = '0';
         }
         else if ( fflags.alternateForm )
         {
            *s++ = fflags.dp;
         }
      }
      else
      {
         while ( decp-- > 0 )
            *s++ = *p++;
         if ( precision > 0 || fflags.alternateForm )
            *s++ = fflags.dp;
      }
   }
   else
   {
      *s++ = *p++;
      if ( precision > 0 || fflags.alternateForm )
         *s++ = fflags.dp;
   }

   while ( *p )
      *s++ = *p++;

   if ( format != 'f' )
   {
      char tmp[EXPONENT_LENGTH];
      size_t tmpLen;
      bool expIsNeg;

      *s++ = format;

      const double absnum = fabs(num);
      if ( (absnum > DBL_EPSILON * absnum) )
         --decp;

      if ( decp != 0 )
      {
         p = Conv10(decp, expIsNeg, &tmp[EXPONENT_LENGTH], tmpLen);
         *s++ = expIsNeg ? '-' : '+';

         if ( tmpLen == 1 )
            *s++ = '0';

         while ( tmpLen-- )
            *s++ = *p++;
      }
      else
      {
         *s++ = '+';
         *s++ = '0';
         *s++ = '0';
      }
   }

   stringLength = s - buf;
   return buf;
}

template <typename T>
T* ConvGFp(double number, const FFlags& fflags, bool& isNeg, T *buf, size_t& stringLength)
{
   int i;
   int decpt;
   char buf1[NDIG];

   int ndigits, savedndigits;
   if ( !fflags.adjustPrecision ) ndigits = FLOAT_DIGITS;
   else if ( fflags.precision == 0 ) ndigits = 1;
   else ndigits = static_cast<int>(fflags.precision);
   savedndigits = ndigits;

   char* p1 = ConvEFp(number, ndigits, decpt, isNeg, buf1);
   T* p2 = buf;

   for ( i = ndigits - 1; i > 0 && p1[i] == '0'; --i )
      --ndigits;

   if ( (decpt >= 0 && decpt - savedndigits > 0) || decpt < -4 ) /* use E-style */
   {
      --decpt;
      *p2++ = *p1++; // copy one significant digit

      if ( ndigits == 1 )
      {
         *p2++ = fflags.dp;
         if ( fflags.alternateForm )
         {
            if ( fflags.alternateFormShort )
               *p2++ = '0'; // add only one zero for one-digit representation
            else
            {
               for ( ; ndigits < savedndigits; ++ndigits )
                  *p2++ = '0';
            }
         }
      }
      else
      {
         M_ASSERT(ndigits > 1); // no less, sure
         *p2++ = fflags.dp;
         for ( i = 1; i < ndigits; ++i )
            *p2++ = *p1++;
         if ( fflags.alternateForm && !fflags.alternateFormShort )
         {
            for ( ; ndigits < savedndigits; ++ndigits )
               *p2++ = '0';
         }
      }

      *p2++ = fflags.fform;
      if ( decpt < 0 )
      {
         decpt = -decpt;
         *p2++ = '-';
      }
      else
      {
         *p2++ = '+';
      }

      if ( decpt / 100 > 0 ) *p2++ = decpt / 100 + '0';
      // else *p2++ = '0'; <== if uncommented, always have three digits for exponent
      if ( decpt / 10 > 0 ) *p2++ = (decpt % 100) / 10 + '0';
      else *p2++ = '0';
      *p2++ = decpt % 10 + '0';
   }
   else
   {
      bool decpok = false;

      if ( decpt <= 0 )
      {
         if ( *p1 != '0' )
         {
            *p2++ = '0';
            *p2++ = fflags.dp;
            decpok = true;
         }

         while ( decpt < 0 )
         {
            ++decpt;
            *p2++ = '0';
         }
      }

      for ( i = 1; i <= ndigits; ++i )
      {
         *p2++ = *p1++;
         if ( i == decpt && !decpok )
         {
            *p2++ = fflags.dp;
            decpok = true;
         }
      }

      if ( ndigits < decpt )
      {
         while ( ndigits++ < decpt )
            *p2++ = '0';

         if ( !decpok )
         {
            --ndigits;
            *p2++ = fflags.dp;
            decpok = true;
         }
      }

      if ( fflags.alternateForm )
      {
         if ( !decpok )
            *p2++ = fflags.dp;

         if ( fflags.alternateFormShort )
         {
            if ( p2[-1] == fflags.dp ) // add only one extra zero in short form
               *p2++ = '0';
         }
         else
         {
            for ( ; ndigits < savedndigits; ++ndigits )
               *p2++ = '0';
         }
      }
      else if ( p2[-1] == fflags.dp )
         p2--;
   }

   *p2 = '\0';
   stringLength = p2 - buf;

   return buf;
}

template <typename T> inline
T* FixPrecision(const FFlags& fflags, T* string, size_t& stringLength)
{
   if ( fflags.adjust )
   {
      size_t p = (fflags.precision + 1 < DEF_NBUF_SIZE) ? fflags.precision : DEF_NBUF_SIZE - 1;
      while ( stringLength < p )
      {
         *--string = '0';
         ++stringLength;
      }
   }
   return string;
}

template <typename T>
struct FBuf
{
   static const T NULL_STRING[];
   static const size_t NULL_STRING_LEN;

   static const T NIL_STRING[];
   static const size_t NIL_STRING_LEN;

   static const T INF_STRING[];
   static const size_t INF_STRING_LEN;

   static const T NAN_STRING[];
   static const size_t NAN_STRING_LEN;

   FBuf(T* buf, size_t bufSize)
   {
      if ( bufSize )
      {
         curpos = buf;
         endpos = buf + bufSize - 1;
      }
      else
      {
         curpos = 0;
         endpos = 0;
      }
      size = 0;
   }

   void Finalize()
   {
      if ( curpos )
      {
         *curpos++ = '\0';
      }
   }

   void Add(T ch)
   {
      if ( curpos != endpos )
      {
         *curpos++ = ch;
      }
      ++size;
   }

   inline void Add(T ch, size_t n)
   {
      if ( n > 8 )
      {
         const ptrdiff_t available = endpos - curpos;
         if ( available > 0 )
         {
            const size_t av = static_cast<size_t>(available);
            const size_t count = n > av ? av : n;
            Fmemset(curpos, ch, count);
            curpos += count;
         }
         size += n;
      }
      else if ( n )
      {
         size += n;
         for ( ; curpos != endpos && n > 0; --n )
            *curpos++ = ch;
      }
   }

   void Add(const T* s, size_t n)
   {
      if ( n )
      {
         const ptrdiff_t available = endpos - curpos;
         if ( available > 0 )
         {
            const size_t av = static_cast<size_t>(available);
            const size_t count = n > av ? av : n;
            Fmemcpy(curpos, s, count);
            curpos += count;
         }
         size += n;
      }
   }

   size_t GetAvailableChars() const
   {
      const ptrdiff_t available = endpos - curpos;
      if ( available > 0 )
         return static_cast<size_t>(available);
      return 0;
   }

   T* curpos;
   T* endpos;
   size_t size;
};

template <typename T>
const T FBuf<T>::NULL_STRING[] = { '(', 'n', 'u', 'l', 'l', ')', '\0' };

template <typename T>
const size_t FBuf<T>::NULL_STRING_LEN = 6;

template <typename T>
const T FBuf<T>::NIL_STRING[] = { '(', 'n', 'i', 'l', ')', '\0' };

template <typename T>
const size_t FBuf<T>::NIL_STRING_LEN = 5;

template <typename T>
const T FBuf<T>::INF_STRING[] = { 'i', 'n', 'f', '\0' };

template <typename T>
const size_t FBuf<T>::INF_STRING_LEN = 3;

template <typename T>
const T FBuf<T>::NAN_STRING[] = { 'n', 'a', 'n', '\0' };;

template <typename T>
const size_t FBuf<T>::NAN_STRING_LEN = 3;

template <typename T>
void DoPString(FBuf<T>& buf, const T* s, size_t& stringLength, const FFlags& fflags)
{
   if ( s )
   {
      stringLength = fflags.adjustPrecision ? Mstrnlen(s, fflags.precision) : Fstrlen(s);

      if ( fflags.adjustWidth && fflags.adjust == ADJUST_RIGHT && fflags.minWidth > stringLength )
      {
         const size_t diff = fflags.minWidth - stringLength;
         buf.Add(' ', diff);
      }

      buf.Add(s, stringLength);

      if ( fflags.adjustWidth && fflags.adjust == ADJUST_LEFT && fflags.minWidth > stringLength )
      {
         const size_t diff = fflags.minWidth - stringLength;
         buf.Add(' ', diff);
      }
   }
   else
   {
      stringLength = FBuf<T>::NULL_STRING_LEN;
      buf.Add(FBuf<T>::NULL_STRING, stringLength);
   }
}

template <typename T> struct SpeciateFor;
template <> struct SpeciateFor<char>
{
#if !M_NO_WCHAR_T
   typedef wchar_t Inv;

   static char* ConvWC(wchar_t ch, char* buffer, size_t& stringLength)
   {
      if ( ch < 128 )
      {
         buffer[0] = static_cast<char>(ch);
         stringLength = 1;
      }
      else
      {
         mbstate_t ps = {0};
         stringLength = Fwcrtomb(buffer, ch, &ps);
         if ( stringLength == static_cast<size_t>(-1) )
         {
            buffer[0] = '?';
            stringLength = 1;
            M_ASSERT(0); // bad format
         }
      }
      return buffer;
   }
#endif

   static char* ConvAC(char ch, char* buffer, size_t& stringLength)
   {
      stringLength = 1;
      buffer[0] = ch;
      return buffer;
   }

   static char* ConvAS(FBuf<char>& buf, const char* s, char* nBuf, size_t& stringLength, const FFlags& fflags)
   {
      DoPString(buf, s, stringLength, fflags);
      return 0;
   }

#if !M_NO_WCHAR_T
   static char* ConvWS(FBuf<char>& buf, const wchar_t* s, char* nBuf, size_t& stringLength, const FFlags& fflags)
   {
      if ( s )
      {
         mbstate_t ps = {0};
         const wchar_t* ws = s;
         char* p = NULL;
         if ( fflags.adjustPrecision )
         {
            const size_t available = buf.GetAvailableChars();
            const size_t need = (fflags.precision > available) ? available : fflags.precision;
            p = (need < DEF_NBUF_SIZE) ? nBuf : M_NEW char[need + 1];
            stringLength = Fwcsrtombs(p, &ws, need, &ps);
         }
         else
         {
            stringLength = Fwcsrtombs(0, &ws, 0, &ps);
            const size_t available = buf.GetAvailableChars();
            if ( available && stringLength && stringLength != static_cast<size_t>(-1) )
            {
               memset(&ps, 0, sizeof ps);
               const size_t need = (stringLength > available) ? available : stringLength;
               p = (need < DEF_NBUF_SIZE) ? nBuf : M_NEW char[need + 1];
               Fwcsrtombs(p, &ws, need, &ps);
            }
         }

         if ( stringLength == static_cast<size_t>(-1) )
         {
            stringLength = 0;
            M_ASSERT(0); // bad conversion
         }
         else
         {
            if ( fflags.adjustWidth && fflags.adjust == ADJUST_RIGHT && fflags.minWidth > stringLength )
            {
               const size_t diff = fflags.minWidth - stringLength;
               buf.Add(' ', diff);
            }
            buf.Add(p, stringLength);
            if ( fflags.adjustWidth && fflags.adjust == ADJUST_LEFT && fflags.minWidth > stringLength )
            {
               const size_t diff = fflags.minWidth - stringLength;
               buf.Add(' ', diff);
            }
         }

         if ( p != nBuf )
            delete[] p;
      }
      else
      {
         stringLength = FBuf<char>::NULL_STRING_LEN;
         buf.Add(FBuf<char>::NULL_STRING, stringLength);
      }

      return 0;
   }
#endif

   static char* Conv(FBuf<char>& buf, const char* s, char* nBuf, size_t& stringLength, const FFlags& fflags)
   {
      return ConvAS(buf, s, nBuf, stringLength, fflags);
   }

#if !M_NO_WCHAR_T
   static char* Conv(FBuf<char>& buf, const wchar_t* s, char* nBuf, size_t& stringLength, const FFlags& fflags)
   {
      return ConvWS(buf, s, nBuf, stringLength, fflags);
   }
#endif

   static char* ConvC(char ch, char* buffer, size_t& stringLength)
   {
      return ConvAC(ch, buffer, stringLength);
   }

#if !M_NO_WCHAR_T
   static char* ConvC(wchar_t ch, char* buffer, size_t& stringLength)
   {
      return ConvWC(ch, buffer, stringLength);
   }
#endif
};

#if !M_NO_WCHAR_T
template <> struct SpeciateFor<wchar_t>
{
   typedef char Inv;

   static wchar_t* ConvWC(wchar_t ch, wchar_t* buffer, size_t& stringLength)
   {
      stringLength = 1;
      buffer[0] = ch;
      return buffer;
   }

   static wchar_t* ConvAC(char ch, wchar_t* buffer, size_t& stringLength)
   {
      stringLength = 1;
      buffer[0] = ch;
      return buffer;
   }

   static wchar_t* ConvAS(FBuf<wchar_t>& buf, char* s, wchar_t* nBuf, size_t& stringLength, const FFlags& fflags)
   {
      if ( s )
      {
         mbstate_t ps = {0};
         const char* mbs = s;
         wchar_t* p = 0;
         if ( fflags.adjustPrecision )
         {
            const size_t available = buf.GetAvailableChars();
            const size_t need = (fflags.precision > available) ? available : fflags.precision;
            p = (need < DEF_NBUF_SIZE) ? nBuf : M_NEW wchar_t[need + 1];
            stringLength = Fmbsrtowcs(p, &mbs, need, &ps);
         }
         else
         {
            stringLength = Fmbsrtowcs(0, &mbs, 0, &ps);
            const size_t available = buf.GetAvailableChars();
            if ( available && stringLength && stringLength != static_cast<size_t>(-1) )
            {
               memset(&ps, 0, sizeof ps);
               const size_t need = (stringLength > available) ? available : stringLength;
               p = (need < DEF_NBUF_SIZE) ? nBuf : M_NEW wchar_t[need + 1];
               Fmbsrtowcs(p, &mbs, need, &ps);
            }
         }

         if ( stringLength == static_cast<size_t>(-1) )
         {
            stringLength = 0;
            M_ASSERT(0); // bad conversion, warn in debug
         }
         else
         {
            if ( fflags.adjustWidth && fflags.adjust == ADJUST_RIGHT && fflags.minWidth > stringLength )
            {
               const size_t diff = fflags.minWidth - stringLength;
               buf.Add(' ', diff);
            }
            buf.Add(p, stringLength);
            if ( fflags.adjustWidth && fflags.adjust == ADJUST_LEFT && fflags.minWidth > stringLength )
            {
               const size_t diff = fflags.minWidth - stringLength;
               buf.Add(' ', diff);
            }
         }

         if ( p != nBuf )
            delete[] p;
      }
      else
      {
         stringLength = FBuf<wchar_t>::NULL_STRING_LEN;
         buf.Add(FBuf<wchar_t>::NULL_STRING, stringLength);
      }

      return 0;
   }

   static wchar_t* ConvWS(FBuf<wchar_t>& buf, wchar_t* s, wchar_t* nBuf, size_t& stringLength, const FFlags& fflags)
   {
      DoPString(buf, s, stringLength, fflags);
      return 0;
   }

   static wchar_t* Conv(FBuf<wchar_t>& buf, wchar_t* s, wchar_t* nBuf, size_t& stringLength, const FFlags& fflags)
   {
      return ConvWS(buf, s, nBuf, stringLength, fflags);
   }

   static wchar_t* Conv(FBuf<wchar_t>& buf, char* s, wchar_t* nBuf, size_t& stringLength, const FFlags& fflags)
   {
      return ConvAS(buf, s, nBuf, stringLength, fflags);
   }

   static wchar_t* ConvC(char ch, wchar_t* buffer, size_t& stringLength)
   {
      return ConvAC(ch, buffer, stringLength);
   }

   static wchar_t* ConvC(wchar_t ch, wchar_t* buffer, size_t& stringLength)
   {
      return ConvWC(ch, buffer, stringLength);
   }
};
#endif

template <typename T>
void Format(FBuf<T>& buf, const T* fmt, va_list args, const lconv* lc) M_NO_THROW
{
   enum { TYPE_QUAD, TYPE_LONG, TYPE_INT, TYPE_SHORT, TYPE_CHAR, TYPE_SIZET, TYPE_PTRDIFFT } type = TYPE_INT;
   bool isNeg;
   bool isError = false;

   union
   {
      F_S64         s64;
      F_U64         u64;
      int           s32;
      unsigned int  u32;
      long          slong;
      unsigned long ulong;
      size_t        usize;
      F_SSIZE_T     ssize;
      ptrdiff_t     pdiff;
      F_UINTPTR     uiptr;
      double        fp;
   } val;

   FFlags fflags;
   fflags.minWidth = 0;
   fflags.precision = 0;
   fflags.dp = '.';

#if (M_OS & (M_OS_NUTTX | M_OS_CMX | M_OS_ANDROID)) != 0
   M_ASSERT(lc == NULL);
#else
   if ( lc && lc->decimal_point )
   {
      char tmp = *(lc->decimal_point);
      if ( tmp != '\0' )
         fflags.dp = tmp;
   }
#endif

   enum { MODE_DEF, MODE_CHAR } mode = MODE_DEF;
   T nBuf[DEF_NBUF_SIZE];
   T* string = 0;
   size_t stringLength = 0;
   int nbits;

   while ( *fmt )
   {
      if ( *fmt != '%' )
      {
         buf.Add(*fmt);
      }
      else
      {
         bool printSomething = true;
         fflags.adjust = ADJUST_RIGHT;
         fflags.alternateForm = false;
         fflags.alternateFormShort = false;
         fflags.printSign = false;
         fflags.printBlank = false;
         fflags.pad = ' ';
         fflags.prefix = '\0';
         mode = MODE_DEF;

         const T* savedfmt = fmt;
         ++fmt;

         if ( !Fislower(*fmt) )
         {
            for ( ;; ++fmt )
            {
               if ( *fmt == '-' ) fflags.adjust = ADJUST_LEFT;
               else if ( *fmt == '#' ) fflags.alternateForm = true;
               else if ( *fmt == '+' ) fflags.printSign = true;
               else if ( *fmt == ' ' ) fflags.printBlank = true;
               else if ( *fmt == '0' ) fflags.pad = '0';
               else break;
            }

            if ( Fisdigit(*fmt) )
            {
               fflags.minWidth = ToDecimal(fmt);
               fflags.adjustWidth = true;
            }
            else if ( *fmt == '*' )
            {
               int value = va_arg(args, int);
               ++fmt;
               fflags.adjustWidth = true;
               if ( value < 0 )
               {
                  fflags.adjust = ADJUST_LEFT;
                  fflags.minWidth = static_cast<size_t>(-value);
               }
               else
               {
                  fflags.minWidth = static_cast<size_t>(value);
               }
            }
            else
            {
               fflags.adjustWidth = false;
            }

            if ( *fmt == '.' )
            {
               fflags.adjustPrecision = true;
               ++fmt;
               if ( Fisdigit(*fmt) )
               {
                  fflags.precision = ToDecimal(fmt);
               }
               else if ( *fmt == '*' )
               {
                  int value = va_arg(args, int);
                  ++fmt;
                  fflags.precision = (value < 0) ? 0 : static_cast<size_t>(value);
               }
               else
               {
                  fflags.precision = 0;
               }
            }
            else
            {
               fflags.adjustPrecision = false;
            }
         }
         else
         {
            fflags.adjustWidth = false;
            fflags.adjustPrecision = false;
         } // if Fislower(*fmt)

         switch ( *fmt )
         {
         case 'l':
            ++fmt;
            if ( *fmt == 'l' )
            {
               type = TYPE_QUAD;
               ++fmt;
            }
            else
            {
               type = TYPE_LONG;
            }
            break;
         case 'q':
            type = TYPE_QUAD;
            ++fmt;
            break;
         case 'h':
            ++fmt;
            if ( *fmt == 'h' )
            {
               type = TYPE_CHAR;
               ++fmt;
            }
            else
            {
               type = TYPE_SHORT;
            }
            break;
         case 'z':
            type = TYPE_SIZET;
            ++fmt;
            break;
         case 't':
            type = TYPE_PTRDIFFT;
            ++fmt;
            break;
         default:
            type = TYPE_INT;
         }

         switch ( *fmt )
         {
         case 'u':
            switch ( type )
            {
            case TYPE_QUAD:
               val.u64 = va_arg(args, F_U64);
               string = Conv10(val.u64, isNeg, &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            case TYPE_LONG:
               val.ulong = va_arg(args, unsigned long);
               string = Conv10(val.ulong, isNeg, &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            case TYPE_SHORT:
               val.u32 = static_cast<unsigned int>(static_cast<unsigned short>(va_arg(args, unsigned int)));
               string = Conv10(val.u32, isNeg, &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            case TYPE_CHAR:
               val.u32 = static_cast<unsigned int>(static_cast<unsigned char>(va_arg(args, unsigned int)));
               string = Conv10(val.u32, isNeg, &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            case TYPE_SIZET:
               val.usize = va_arg(args, size_t);
               string = Conv10(val.usize, isNeg, &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            case TYPE_PTRDIFFT:
               val.uiptr = static_cast<F_UINTPTR>(va_arg(args, ptrdiff_t));
               string = Conv10(val.uiptr, isNeg, &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            default:
               val.u32 = va_arg(args, unsigned int);
               string = Conv10(val.u32, isNeg, &nBuf[DEF_NBUF_SIZE], stringLength);
            };
            string = FixPrecision(fflags, string, stringLength);
            break;
         case 'd':
         case 'i':
            switch ( type )
            {
            case TYPE_QUAD:
               val.s64 = va_arg(args, F_S64);
               string = Conv10(val.s64, isNeg, &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            case TYPE_LONG:
               val.slong = va_arg(args, long);
               string = Conv10(val.slong, isNeg, &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            case TYPE_SHORT:
               val.s32 = static_cast<int>(static_cast<short>(va_arg(args, int)));
               string = Conv10(val.s32, isNeg, &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            case TYPE_CHAR:
               val.s32 = static_cast<int>(static_cast<signed char>(va_arg(args, int)));
               string = Conv10(val.s32, isNeg, &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            case TYPE_SIZET:
               val.ssize = va_arg(args, F_SSIZE_T);
               string = Conv10(val.ssize, isNeg, &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            case TYPE_PTRDIFFT:
               val.pdiff = va_arg(args, ptrdiff_t);
               string = Conv10(val.pdiff, isNeg, &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            default:
               val.s32 = va_arg(args, int);
               string = Conv10(val.s32, isNeg, &nBuf[DEF_NBUF_SIZE], stringLength);
            };
            string = FixPrecision(fflags, string, stringLength);
            if ( isNeg ) fflags.prefix = '-';
            else if ( fflags.printSign ) fflags.prefix = '+';
            else if ( fflags.printBlank ) fflags.prefix = ' ';
            break;
         case 'o':
         case 'x':
         case 'X':
            nbits = (*fmt == 'o') ? 3 : 4;
            switch ( type )
            {
            case TYPE_QUAD:
               val.u64 = va_arg(args, F_U64);
               string = ConvP2(val.u64, nbits, static_cast<char>(*fmt), &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            case TYPE_LONG:
               val.ulong = va_arg(args, unsigned long);
               string = ConvP2(val.ulong, nbits, static_cast<char>(*fmt), &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            case TYPE_SHORT:
               val.u32 = static_cast<unsigned int>(static_cast<unsigned short>(va_arg(args, unsigned int)));
               string = ConvP2(val.u32, nbits, static_cast<char>(*fmt), &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            case TYPE_CHAR:
               val.u32 = static_cast<unsigned int>(static_cast<unsigned char>(va_arg(args, unsigned int)));
               string = ConvP2(val.u32, nbits, static_cast<char>(*fmt), &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            case TYPE_SIZET:
               val.usize = va_arg(args, size_t);
               string = ConvP2(val.usize, nbits, static_cast<char>(*fmt), &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            case TYPE_PTRDIFFT:
               val.uiptr = static_cast<F_UINTPTR>(va_arg(args, ptrdiff_t));
               string = ConvP2(val.uiptr, nbits, static_cast<char>(*fmt), &nBuf[DEF_NBUF_SIZE], stringLength);
               break;
            default:
               val.u32 = va_arg(args, unsigned int);
               string = ConvP2(val.u32, nbits, static_cast<char>(*fmt), &nBuf[DEF_NBUF_SIZE], stringLength);
            };
            string = FixPrecision(fflags, string, stringLength);
            if ( fflags.alternateForm && *string != '0' )
            {
               if ( *fmt != 'o' )
               {
                  *--string = *fmt;
                  ++stringLength;
               }
               *--string = '0';
               ++stringLength;
            }
            break;
         case 'c':
         case 'C':
#if !M_NO_WCHAR_T
            if ( type == TYPE_LONG )
            {
               wchar_t ch = static_cast<wchar_t>(va_arg(args, int));
               string = SpeciateFor<T>::ConvWC(ch, nBuf, stringLength);
            }
            else
#endif
            if ( type == TYPE_SHORT )
            {
               char ch = static_cast<char>(va_arg(args, int));
               string = SpeciateFor<T>::ConvC(ch, nBuf, stringLength);
            }
            else
#if !M_NO_WCHAR_T
            if ( *fmt == 'C' )
            {
               typedef typename SpeciateFor<T>::Inv Inv;
               Inv ch = static_cast<Inv>(va_arg(args, int));
               string = SpeciateFor<T>::ConvC(ch, nBuf, stringLength);
            }
            else
#endif
            {
               T ch = static_cast<T>(va_arg(args, int));
               string = SpeciateFor<T>::ConvC(ch, nBuf, stringLength);
            }
            mode = MODE_CHAR;
            fflags.pad = ' ';
            break;
         case 's':
         case 'S':
#if !M_NO_WCHAR_T
            if ( type == TYPE_LONG )
            {
               wchar_t* s = va_arg(args, wchar_t*);
               string = SpeciateFor<T>::ConvWS(buf, s, nBuf, stringLength, fflags);
            }
            else
#endif
            if ( type == TYPE_SHORT )
            {
               char* s = va_arg(args, char*);
               string = SpeciateFor<T>::ConvAS(buf, s, nBuf, stringLength, fflags);
            }
            else
#if !M_NO_WCHAR_T
            if ( *fmt == 'S' )
            {
               typedef typename SpeciateFor<T>::Inv Inv;
               Inv* s = va_arg(args, Inv*);
               string = SpeciateFor<T>::Conv(buf, s, nBuf, stringLength, fflags);
            }
            else
#endif
            {
               T* s = va_arg(args, T*);
               string = SpeciateFor<T>::Conv(buf, s, nBuf, stringLength, fflags);
            }
            ++fmt;
            continue;
         case 'f':
         case 'e':
         case 'E':
            fflags.fform = (*fmt == 'E') ? 'E' : 'e';
            val.fp = va_arg(args, double);
            if ( Fisnan(val.fp) )
            {
               string = const_cast<T*>(FBuf<T>::NAN_STRING);
               stringLength = FBuf<T>::NAN_STRING_LEN;
            }
            else if ( Fisinf(val.fp) )
            {
               string = const_cast<T*>(FBuf<T>::INF_STRING);
               stringLength = FBuf<T>::INF_STRING_LEN;
            }
            else
            {
               string = ConvFp(*fmt, val.fp, fflags, isNeg, &nBuf[1], stringLength);
               if ( isNeg ) fflags.prefix = '-';
               else if ( fflags.printSign ) fflags.prefix = '+';
               else if ( fflags.printBlank ) fflags.prefix = ' ';
            }
            break;
         case 'g':
         case 'G':
            fflags.fform = (*fmt == 'G') ? 'E' : 'e';
            string = ConvGFp(va_arg(args, double), fflags, isNeg, &nBuf[1], stringLength);
            if ( isNeg ) fflags.prefix = '-';
            else if ( fflags.printSign ) fflags.prefix = '+';
            else if ( fflags.printBlank ) fflags.prefix = ' ';
            break;
         case 'n':
            switch ( type )
            {
            case TYPE_QUAD:     *(va_arg(args, F_S64*)) = buf.size; break;
            case TYPE_LONG:     *(va_arg(args, long*)) = (long)buf.size; break;
            case TYPE_SHORT:    *(va_arg(args, short*)) = (short)buf.size; break;
            case TYPE_CHAR:     *(va_arg(args, char*)) = (char)buf.size; break;
            case TYPE_SIZET:    *(va_arg(args, size_t*)) = (size_t)buf.size; break;
            case TYPE_PTRDIFFT: *(va_arg(args, ptrdiff_t*)) = (ptrdiff_t)buf.size; break;
            default: *(va_arg(args, int*)) = (int)buf.size;
            };
            printSomething = false;
            break;
         case 'p':
            val.uiptr = reinterpret_cast<F_UINTPTR>(va_arg(args, void*));
            if ( val.uiptr )
            {
               string = ConvP2(val.uiptr, 4, 'x', &nBuf[DEF_NBUF_SIZE], stringLength);
               *--string = 'x';
               *--string = '0';
               stringLength += 2;
            }
            else
            {
               string = const_cast<T*>(FBuf<T>::NIL_STRING);
               stringLength = FBuf<T>::NIL_STRING_LEN;
            }
            fflags.pad = ' ';
            break;
         case '%':
            nBuf[0] = '%';
            string = nBuf;
            stringLength = 1;
            mode = MODE_CHAR;
            fflags.pad = ' ';
            break;
         case '\0':
            continue;
         default:
            isError = true;
            M_ASSERT(0); // warn on debug, okay in release
            break;
         };

         if ( isError )
         {
            isError = false;
            buf.Add('%');
            fmt = savedfmt;
         }
         else
         {
            if ( fflags.prefix != '\0' && string != FBuf<T>::NULL_STRING && mode != MODE_CHAR )
            {
               *--string = fflags.prefix;
               ++stringLength;
            }

            if ( fflags.adjustWidth && fflags.adjust == ADJUST_RIGHT && fflags.minWidth > stringLength )
            {
               if ( fflags.pad == '0' && fflags.prefix != '\0' )
               {
                  buf.Add(*string);
                  ++string;
                  --stringLength;
                  --fflags.minWidth;
               }

               const size_t diff = fflags.minWidth - stringLength;
               buf.Add(fflags.pad, diff);
            }

            if ( printSomething )
            {
               buf.Add(string, stringLength);
            }

            if ( fflags.adjustWidth && fflags.adjust == ADJUST_LEFT && fflags.minWidth > stringLength )
            {
               const size_t diff = fflags.minWidth - stringLength;
               buf.Add(fflags.pad, diff);
            }
         }

      } // if
      ++fmt;
   } // while
}

#define DECLARE_TABLE(type, name) static const type name[];

#define DEFINE_TABLE(type, cls, name, F)                                \
   const type cls<type>::name[] =                                       \
   {                                                                    \
      F(2), F(3), F(4), F(5), F(6), F(7), F(8), F(9), F(10),            \
      F(11), F(12), F(13), F(14), F(15), F(16), F(17), F(18), F(19), F(20), \
      F(21), F(22), F(23), F(24), F(25), F(26), F(27), F(28), F(29), F(30), \
      F(31), F(32), F(33), F(34), F(35), F(36)                          \
   }

#define DEFINE_SEL_MAX_TABLE(type)              \
   template <> struct SelMaxTable<type>         \
   {                                            \
      static const type MAX;                    \
      DECLARE_TABLE(type, OFFS);                \
      DECLARE_TABLE(type, REMS);                \
                                                \
      static type GetOFF(int p) { return OFFS[p]; }    \
      static type GetREM(int p) { return REMS[p]; }    \
   }

#define DEFINE_SEL_MIN_TABLE(type)              \
   template <> struct SelMinTable<type>         \
   {                                            \
      static const type MAX;                    \
      DECLARE_TABLE(type, OFFS);                \
      DECLARE_TABLE(type, REMS);                \
                                                \
      static type GetOFF(int p) { return OFFS[p]; }    \
      static type GetREM(int p) { return REMS[p]; }    \
   }

#define DEFINE_MAX_TABLE(type, prefix)                                  \
   const type SelMaxTable<type>::MAX = std::numeric_limits<type>::max(); \
   DEFINE_TABLE(type, SelMaxTable, OFFS, prefix##_MAX_DIV_X);           \
   DEFINE_TABLE(type, SelMaxTable, REMS, prefix##_MAX_REM_X)

#define DEFINE_MIN_TABLE(type, prefix)                                  \
   const type SelMinTable<type>::MAX = std::numeric_limits<type>::min(); \
   DEFINE_TABLE(type, SelMinTable, OFFS, prefix##_MIN_DIV_X);           \
   DEFINE_TABLE(type, SelMinTable, REMS, prefix##_MIN_REM_X);

#ifndef LLONG_MAX
   #ifdef _I64_MAX
      #define LLONG_MAX  _I64_MAX
      #define LLONG_MIN  _I64_MIN
      #define ULLONG_MAX _UI64_MAX
   #else
      #define LLONG_MAX  MINT64C(9223372036854775807)
      #define LLONG_MIN  (-LLONG_MAX - MINT64C(1))
      #define ULLONG_MAX MUINT64C(0xFFFFFFFFFFFFFFFF)
   #endif
#endif
M_COMPILED_ASSERT(LLONG_MAX == MINT64C(9223372036854775807));
M_COMPILED_ASSERT(LLONG_MIN == (-LLONG_MAX - MINT64C(1)));
M_COMPILED_ASSERT(ULLONG_MAX == MUINT64C(0xFFFFFFFFFFFFFFFF));

#define PP_ULLONG_MAX_DIV_X(x) ULLONG_MAX / x
#define PP_ULLONG_MAX_REM_X(x) ULLONG_MAX % x
#define PP_LLONG_MAX_DIV_X(x) LLONG_MAX / x
#define PP_LLONG_MAX_REM_X(x) LLONG_MAX % x
#define PP_LLONG_MIN_DIV_X(x) LLONG_MIN / x
#define PP_LLONG_MIN_REM_X(x) LLONG_MIN % x
#define PP_ULONG_MAX_DIV_X(x) ULONG_MAX / x
#define PP_ULONG_MAX_REM_X(x) ULONG_MAX % x
#define PP_LONG_MAX_DIV_X(x)  LONG_MAX / x
#define PP_LONG_MAX_REM_X(x)  LONG_MAX % x
#define PP_LONG_MIN_DIV_X(x)  LONG_MIN / x
#define PP_LONG_MIN_REM_X(x)  LONG_MIN % x
#define PP_UINT_MAX_DIV_X(x)  UINT_MAX / x
#define PP_UINT_MAX_REM_X(x)  UINT_MAX % x
#define PP_INT_MAX_DIV_X(x)   INT_MAX / x
#define PP_INT_MAX_REM_X(x)   INT_MAX % x
#define PP_INT_MIN_DIV_X(x)   INT_MIN / x
#define PP_INT_MIN_REM_X(x)   INT_MIN % x

template <typename> struct SelMaxTable;
template <typename> struct SelMinTable;
#if defined(LLONG_MAX) || defined(__BORLANDC__)
DEFINE_SEL_MAX_TABLE(F_U64);
DEFINE_SEL_MAX_TABLE(F_S64);
DEFINE_SEL_MIN_TABLE(F_S64);
#endif
DEFINE_SEL_MAX_TABLE(unsigned long);
DEFINE_SEL_MAX_TABLE(long);
DEFINE_SEL_MIN_TABLE(long);
DEFINE_SEL_MAX_TABLE(unsigned int);
DEFINE_SEL_MAX_TABLE(int);
DEFINE_SEL_MIN_TABLE(int);

#if defined(LLONG_MAX) || defined(__BORLANDC__)
DEFINE_MAX_TABLE(F_U64, PP_ULLONG);
DEFINE_MAX_TABLE(F_S64, PP_LLONG);
DEFINE_MIN_TABLE(F_S64, PP_LLONG);
#endif
DEFINE_MAX_TABLE(unsigned long, PP_ULONG);
DEFINE_MAX_TABLE(long, PP_LONG);
DEFINE_MIN_TABLE(long, PP_LONG);
DEFINE_MAX_TABLE(unsigned int, PP_UINT);
DEFINE_MAX_TABLE(int, PP_INT);
DEFINE_MIN_TABLE(int, PP_INT);

const char DIG2NUM_TBL[] =
{
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
   2,  3,  4,  5,  6,  7,  8,  9,  0,  0,
   0,  0,  0,  0,  0,  10, 11, 12, 13, 14,
   15, 0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  10, 11, 12,
   13, 14, 15
};

template <typename N, typename T> inline
N DoDig2Num(T ch)
{
   return static_cast<N>(*(DIG2NUM_TBL + ch));
}

template <int base> struct SpeciaceForConv;

template <> struct SpeciaceForConv<8>
{
   template <typename T>
   static bool Check(T ch) { return ('0' <= ch && ch <= '7'); }
};

template <> struct SpeciaceForConv<10>
{
   template <typename T>
   static bool Check(T ch) { return ('0' <= ch && ch <= '9'); }
};

template <> struct SpeciaceForConv<16>
{
   template <typename T>
   static bool Check(T ch) { return (ch < static_cast<T>(sizeof DIG2NUM_TBL)) && (*(DIG2NUM_TBL + ch) || ch == '0'); }
};

template <typename T, typename U, int base>
U DoBasicConvStringToUnsigned(const T* s, bool& overflow, const T** end)
{
   typedef SpeciaceForConv<base> CheckBase;

   U response = 0;
   overflow = false;
   if ( CheckBase::Check(*s) )
   {
      response = DoDig2Num<U>(*s);
      if ( CheckBase::Check(*++s) )
      {
         const U curoff = SelMaxTable<U>::GetOFF(base - 2);
         const U curlim = SelMaxTable<U>::GetREM(base - 2);
         do
         {
            const U n = DoDig2Num<U>(*s);
            if ( F_UNLIKELY(response >= curoff) && (response > curoff || (response == curoff && n > curlim)) )
            {
               response = SelMaxTable<U>::MAX;
               overflow = true;
               while ( CheckBase::Check(*++s) )
                  ;
               break;
            }
            else
            {
               response *= base;
               response += n;
            }
         }
         while ( CheckBase::Check(*++s) );
      }

      if ( end )
      {
         *end = s;
      }
   }

   return response;
}

template <typename N, typename T>
typename ToUnsigned<N>::Type DoConvStringToUnsigned(const T* s, bool& sign, bool& overflow, const T** end = 0)
{
   typedef typename ToUnsigned<N>::Type U;

   while ( Fisspace(*s) )
      ++s;

   sign = false;
   switch ( *s )
   {
   case '-':
      sign = true;
      ++s;
      break;
   case '+':
      ++s;
      break;
   }

   if ( s[0] == '0' && (s[1] == 'x' || s[1] == 'X') )
   {
      s += 2;
      return DoBasicConvStringToUnsigned<T, U, 16>(s, overflow, end);
   }
   return DoBasicConvStringToUnsigned<T, U, 10>(s, overflow, end);
}

template <typename T, typename S, int base>
S DoBasicConvStringToSigned(const T* s, bool sign, bool& overflow, const T** end)
{
   typedef SpeciaceForConv<base> CheckBase;

   S response = 0;
   overflow = false;

   if ( CheckBase::Check(*s) )
   {
      if ( sign )
      {
         response = -DoDig2Num<S>(*s);
         if ( CheckBase::Check(*++s) )
         {
            S curoff = SelMinTable<S>::GetOFF(base - 2);
            S curlim = SelMinTable<S>::GetREM(base - 2);
            if ( curlim > 0 )
            {
               curlim -= base;
               curoff += 1;
            }
            curlim = -curlim;
            do
            {
               const S n = DoDig2Num<S>(*s);
               if ( F_UNLIKELY(response <= curoff) && (response < curoff || (response == curoff && n > curlim)) )
               {
                  response = SelMinTable<S>::MAX;
                  overflow = true;
                  while ( CheckBase::Check(*++s) )
                     ;
                  break;
               }
               else
               {
                  response *= base;
                  response -= n;
               }
            }
            while ( CheckBase::Check(*++s) );
         }
      }
      else
      {
         response = DoDig2Num<S>(*s);
         if ( CheckBase::Check(*++s) )
         {
            const S curoff = SelMaxTable<S>::GetOFF(base - 2);
            const S curlim = SelMaxTable<S>::GetREM(base - 2);
            do
            {
               const S n = DoDig2Num<S>(*s);
               if ( F_UNLIKELY(response >= curoff) && (response > curoff || (response == curoff && n > curlim)) )
               {
                  response = SelMaxTable<S>::MAX;
                  overflow = true;
                  while ( CheckBase::Check(*++s) )
                     ;
                  break;
               }
               else
               {
                  response *= base;
                  response += n;
               }
            }
            while ( CheckBase::Check(*++s) );
         }
      }
      if ( end )
         *end = s;
   }
   return response;
}

template <typename S, typename T>
S DoConvStringToSigned(const T* s, bool& sign, bool& overflow, const T** end = 0)
{
   while ( Fisspace(*s) )
      ++s;

   sign = false;
   switch ( *s )
   {
   case '-':
       sign = true;
       ++s;
       break;
   case '+':
       ++s;
       break;
   }

   if ( s[0] == '0' && (s[1] == 'x' || s[1] == 'X') )
   {
      s += 2;
      return DoBasicConvStringToSigned<T, S, 16>(s, sign, overflow, end);
   }
   return DoBasicConvStringToSigned<T, S, 10>(s, sign, overflow, end);
}

// TODO: need locale class
inline lconv* Flocaleconv()
{
   #if (M_OS & (M_OS_WIN32_CE | M_OS_ANDROID | M_OS_NUTTX | M_OS_CMX)) != 0
      return 0;
   #else
      return localeconv();
   #endif
}

} // namespace

M_FUNC size_t MFormatVALc(char* buf, size_t size, const char* format, const lconv* lc, va_list args) M_NO_THROW
{
   FBuf<char> fbuf(buf, size);
   Format(fbuf, format, args, lc);
   fbuf.Finalize();
   return fbuf.size;
}

M_FUNC size_t MFormatVA(char* buf, size_t size, const char* format, va_list args) M_NO_THROW
{
   const lconv* lc = Flocaleconv();
   return MFormatVALc(buf, size, format, lc, args);
}

M_FUNC size_t MFormatLc(char* buf, size_t size, const char* format, const lconv* lc, ...) M_NO_THROW
{
   va_list args;
   va_start(args, lc);
   size_t res = MFormatVALc(buf, size, format, lc, args);
   va_end(args);
   return res;
}

M_FUNC size_t MFormat(char* buf, size_t size, const char* format, ...) M_NO_THROW
{
   va_list args;
   va_start(args, format);
   size_t res = MFormatVA(buf, size, format, args);
   va_end(args);
   return res;
}

M_FUNC unsigned MStringToUnsigned16(const char* string, bool& overflow)
{
   return DoBasicConvStringToUnsigned<char, unsigned, 16>(string, overflow, 0);
}

M_FUNC int MStringToSigned(const char* string, bool& sign, bool& overflow, const char** end)
{
   return DoConvStringToSigned<int>(string, sign, overflow, end);
}


M_FUNC unsigned MStringToUnsigned(const char* string, bool& sign, bool& overflow, const char** end)
{
   return DoConvStringToUnsigned<int>(string, sign, overflow, end);
}

M_FUNC Mint64 MStringToInt64(const char* string, bool& sign, bool& overflow, const char** end)
{
   return DoConvStringToSigned<Mint64>(string, sign, overflow, end);
}

M_FUNC Muint64 MStringToUInt64(const char* string, bool& sign, bool& overflow, const char** end)
{
   return DoConvStringToUnsigned<Muint64>(string, sign, overflow, end);
}

M_FUNC char* MUnsignedToString(Muint64 value, char* string, size_t& length)
{
   bool sign;
   char* result = Conv10(value, sign, string, length);
   return result;
}

M_FUNC char* MUnsignedToString(unsigned value, char* string, size_t& length)
{
   bool sign;
   char* result = Conv10(value, sign, string, length);
   return result;
}

M_FUNC char* MSignedToString(Mint64 value, char* string, size_t& length)
{
   bool sign;
   char* result = Conv10(value, sign, string, length);
   if ( sign )
   {
      *--result = '-';
      ++length;
   }
   return result;
}

M_FUNC char* MSignedToString(int value, char* string, size_t& length)
{
   bool sign;
   char* result = Conv10(value, sign, string, length);
   if ( sign )
   {
      *--result = '-';
      ++length;
   }
   return result;
}

template <typename T>
T* MDoubleToString(double value, T* nBuf, size_t& length, bool shortestFormat)
{
   FFlags fflags;
   fflags.adjust = ADJUST_LEFT;
   if ( shortestFormat )
   {
      fflags.alternateForm = false;
      fflags.alternateFormShort = false;
   }
   else
   {
      fflags.alternateForm = true;  // %# -> flags.alternateForm = true;
      fflags.alternateFormShort = true; // do not fill .00000 after period
   }
   fflags.printSign = false;
   fflags.printBlank = false;
   fflags.adjustPrecision = true;
   fflags.adjustWidth = false;
   fflags.pad = ' ';              // %0 -> fflags.pad = '0';
   fflags.prefix = '\0';
   fflags.dp = '.';
   fflags.fform = 'e';
   fflags.minWidth = 0;
   fflags.precision = 14;

   bool isNeg;
   length = 0;
   T* string = ConvGFp(value, fflags, isNeg, &nBuf[1], length);
   if ( isNeg )
   {
      *--string = '-';
      ++length;
   }
   return string;
}

M_FUNC char* MToChars(double value, char* buffer, bool shortestFormat) M_NO_THROW
{
   size_t length;
   char tmp [ 128 ];
   char* ptr = MDoubleToString(value, tmp, length, shortestFormat);
   memcpy(buffer, ptr, length);
   buffer[length] = '\0';
   return buffer;
}

#if !M_NO_WCHAR_T

M_FUNC size_t MFormatVALc(wchar_t* buf, size_t size, const wchar_t* format, const lconv* lc, va_list args) M_NO_THROW
{
   FBuf<wchar_t> fbuf(buf, size);
   Format(fbuf, format, args, lc);
   fbuf.Finalize();
   return fbuf.size;
}

M_FUNC size_t MFormatVA(wchar_t* buf, size_t size, const wchar_t* format, va_list args) M_NO_THROW
{
   const lconv* lc = Flocaleconv();
   return MFormatVALc(buf, size, format, lc, args);
}

M_FUNC size_t MFormatLc(wchar_t* buf, size_t size, const wchar_t* format, const lconv* lc, ...) M_NO_THROW
{
   va_list args;
   va_start(args, lc);
   size_t res = MFormatVALc(buf, size, format, lc, args);
   va_end(args);
   return res;
}

M_FUNC size_t MFormat(wchar_t* buf, size_t size, const wchar_t* format, ...) M_NO_THROW
{
   va_list args;
   va_start(args, format);
   size_t res = MFormatVA(buf, size, format, args);
   va_end(args);
   return res;
}

M_FUNC unsigned MStringToUnsigned16(const wchar_t* string, bool& overflow)
{
   return DoBasicConvStringToUnsigned<wchar_t, unsigned, 16>(string, overflow, 0);
}

M_FUNC int MStringToSigned(const wchar_t* string, bool& sign, bool& overflow, const wchar_t** end)
{
   return DoConvStringToSigned<int>(string, sign, overflow, end);
}

M_FUNC unsigned MStringToUnsigned(const wchar_t* string, bool& sign, bool& overflow, const wchar_t** end)
{
   return DoConvStringToUnsigned<int>(string, sign, overflow, end);
}

M_FUNC Mint64 MStringToInt64(const wchar_t* string, bool& sign, bool& overflow, const wchar_t** end)
{
   return DoConvStringToSigned<Mint64>(string, sign, overflow, end);
}

M_FUNC Muint64 MStringToUInt64(const wchar_t* string, bool& sign, bool& overflow, const wchar_t** end)
{
   return DoConvStringToUnsigned<Muint64>(string, sign, overflow, end);
}

M_FUNC wchar_t* MUnsignedToString(Muint64 value, wchar_t* string, size_t& length)
{
   bool sign;
   return Conv10(value, sign, string, length);
}

M_FUNC wchar_t* MUnsignedToString(unsigned value, wchar_t* string, size_t& length)
{
   bool sign;
   return Conv10(value, sign, string, length);
}

M_FUNC wchar_t* MSignedToString(Mint64 value, wchar_t* string, size_t& length)
{
   bool sign;
   wchar_t* ptr = Conv10(value, sign, string, length);
   if ( sign )
   {
      *--ptr = '-';
      ++length;
   }
   return ptr;
}

M_FUNC wchar_t* MSignedToString(int value, wchar_t* string, size_t& length)
{
   bool sign;
   wchar_t* ptr = Conv10(value, sign, string, length);
   if ( sign )
   {
      *--ptr = '-';
      ++length;
   }
   return ptr;
}

M_FUNC wchar_t* MToChars(double value, wchar_t* buffer, bool shortestFormat) M_NO_THROW
{
   size_t length;
   wchar_t tmp [ 128 ];
   wchar_t* ptr = MDoubleToString(value, tmp, length, shortestFormat);
   memcpy(buffer, ptr, length * sizeof(wchar_t));
   buffer[length] = L'\0';
   return buffer;
}

#endif

