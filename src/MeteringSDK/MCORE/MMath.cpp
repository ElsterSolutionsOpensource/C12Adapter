// File MCORE/MMath.cpp

#include "MCOREExtern.h"
#include "MMath.h"
#include "MException.h"

   #ifdef max
      #undef max
      #undef min
   #endif

   #if defined(_MSC_VER) || defined(__BORLANDC__)
      #define isnan(x)       _isnan(x)
      #define isinf(x)       (!_finite(x))
      #define nextafter(x,y) (_nextafter(x, y))
   #endif

   inline bool IsNanOrInf(double d)
   {
      return isnan(d) || isinf(d);
   }

   extern "C"  // double-define to make sure the prototypes are as expected in "private/dtoa.c"
   {
      void meteringsdk_freedtoa(char* s);
      char* meteringsdk_dtoa(double d, int mode, int ndigits, int* decpt, int* sign, char** rve);
   }

M_START_PROPERTIES(Math)
   M_CLASS_PROPERTY_READONLY_DOUBLE (Math, PI) // use all uppercase due to the way we search properties less than 3 chars long
   M_CLASS_PROPERTY_READONLY_DOUBLE (Math, E)  // use all uppercase due to the way we search properties less than 3 chars long
M_START_METHODS(Math)
   M_CLASS_SERVICE                  (Math, Min,                   ST_MVariant_S_constMVariantA_constMVariantA)
   M_CLASS_SERVICE                  (Math, Max,                   ST_MVariant_S_constMVariantA_constMVariantA)
   M_CLASS_SERVICE                  (Math, Abs,                   ST_MVariant_S_constMVariantA)
   M_CLASS_SERVICE                  (Math, Floor,                 ST_double_S_double)
   M_CLASS_SERVICE                  (Math, Ceil,                  ST_double_S_double)
   M_CLASS_SERVICE_OVERLOADED       (Math, Round, Round,     2,   ST_double_S_double_int)
   M_CLASS_SERVICE_OVERLOADED       (Math, Round, Round0,    1,   ST_double_S_double)  // DOXYGEN_HIDE SWIG_HIDE
   M_CLASS_SERVICE                  (Math, Sqrt,                  ST_double_S_double)
   M_CLASS_SERVICE                  (Math, Pow10,                 ST_double_S_int)
   M_CLASS_SERVICE                  (Math, Pow2,                  ST_double_S_int)
   M_CLASS_SERVICE                  (Math, Pow,                   ST_double_S_double_double)
   M_CLASS_SERVICE                  (Math, Exp,                   ST_double_S_double)
   M_CLASS_SERVICE                  (Math, Log,                   ST_double_S_double)
   M_CLASS_SERVICE                  (Math, Log10,                 ST_double_S_double)
   M_CLASS_SERVICE                  (Math, Sin,                   ST_double_S_double)
   M_CLASS_SERVICE                  (Math, Cos,                   ST_double_S_double)
   M_CLASS_SERVICE                  (Math, Tan,                   ST_double_S_double)
   M_CLASS_SERVICE                  (Math, Asin,                  ST_double_S_double)
   M_CLASS_SERVICE                  (Math, Acos,                  ST_double_S_double)
   M_CLASS_SERVICE                  (Math, Atan,                  ST_double_S_double)
   M_CLASS_SERVICE                  (Math, Rand,                  ST_int_S)
   M_CLASS_SERVICE                  (Math, RandomInRange,         ST_unsigned_S_unsigned_unsigned)
   M_CLASS_SERVICE                  (Math, RandomFloat,           ST_double_S)
   M_CLASS_SERVICE                  (Math, RandomFloatInRange,    ST_double_S_double_double)
   M_CLASS_SERVICE                  (Math, BinaryMantissa,        ST_double_S_double)
   M_CLASS_SERVICE                  (Math, BinaryExponent,        ST_int_S_double)
M_END_CLASS(Math, Object)

#if !M_NO_VARIANT

MVariant MMath::Min(const MVariant& v1, const MVariant& v2)
{
   return v1 < v2 ? v1 : v2;
}

MVariant MMath::Max(const MVariant& v1, const MVariant& v2)
{
   return v1 < v2 ? v2 : v1;
}

MVariant MMath::Abs(const MVariant& num)
{
   return num < 0 ? -num : num;
}

#endif // !M_NO_VARIANT

double MMath::Round0(double val)
{
   double result;

   // The algo is similar to boost/math/special_functions/round.hpp
   // Explanation: https://github.com/boostorg/math/pull/8
   //
   // The following prvious code could give incorrect values due to +0.5 or -0.5 operation
   //     if ( val >= 0.0 )
   //        result = floor(val + 0.5);
   //     else
   //        result = ceil(val - 0.5);
   //

   if ( val > -0.5 && val < 0.5 )
      result = 0.0;
   else if ( val > 0 )
   {
      result = ceil(val);
      if ( result - val > 0.5 )
          result -= 1.0;
   }
   else
   {
      result = floor(val);
      if ( val - result > 0.5 )
         result += 1.0;
   }
   return result;
}

double MMath::Round(double val, int numDecimalPlaces)
{
   double result;
   if ( IsNanOrInf(val) )
      result = val; // just copy and do nothing else
   if ( numDecimalPlaces == 0 ) // special much faster and more precise case
      result = Round0(val);
   else
   {
      // The following previous code has imprecise arithmetic operations and it could give incorrect results
      //
      // double multiplier = Pow10(numDecimalPlaces);
      // double product = val * multiplier;
      // if ( !IsNanOrInf(product) ) // Overflow/underflow in multiplier, use an elaborate algo
      // {
      //    if ( product < -1.0e+17 || product > 1.0e+17 ) // Cannot get a better number than it is already
      //       return val;                                 // Return the original value in such case
      //    if ( product > -10000.0 && product < 10000.0 ) // Use simple and relatively fast algorithm for small product
      //    {
      //       result = Round0(product) / multiplier;
      //       return result;
      //    }
      // }

      // Perform the rounding through the conversion to decimal
      // Thanks Python 3.5 implementation of round(d, p) for the idea.

      MESystemError::ClearGlobalSystemError();
      #if (M_OS & M_OS_WINDOWS) != 0
         errno = 0;
      #endif

      // This is to adjust a number so that 0.499999999, which is the closest aproximation to 0.5, still rounds up
      if ( val >= 0.0 )
         val = nextafter(val, std::numeric_limits<double>::max());
      else
         val = nextafter(val, -std::numeric_limits<double>::max());

      bool success = false;

      char stringBuff [ 1024 ]; // longest string dtoa can return is 751 characters, reserve
      int decpt;
      int sign;
      char* buffEnd;
      char* buff = meteringsdk_dtoa(val, 3, numDecimalPlaces, &decpt, &sign, &buffEnd);
      if ( buff != NULL )
      {
         int buffLen = static_cast<int>(buffEnd - buff);
         M_ENSURED_ASSERT(buffLen >= 0);
         if ( buffLen < static_cast<int>(sizeof(stringBuff) - 7) ) // make sure it fits
         {
            int exponent = decpt - buffLen;
            MFormat(stringBuff, sizeof(stringBuff), (sign ? "-0%se%d" : "0%se%d"), buff, exponent);
            result = MToDouble(stringBuff);
            if ( errno == 0 )
               success = true;
         }
         meteringsdk_freedtoa(buff);
      }

      if ( !success )
      {
         if ( errno == 0 )
            errno = ERANGE;
         MESystemError::ThrowLastSystemError();
         M_ENSURED_ASSERT(0);
      }
   }
   return result;
}

double MMath::Sqrt(double arg)
{
   MEMath::BeforeDoingMath();
   double result = sqrt(arg);
   MEMath::AfterDoingMath(result, "Sqrt");
   return result;
}

double MMath::Pow10(int power)
{
   static const double s_powers10 [] = 
      {
         0.0000001,   //  [0] = pow(10.0, -7)
         0.000001,    //  [1] = pow(10.0, -6)
         0.00001,     //  [2] = pow(10.0, -5)
         0.0001,      //  [3] = pow(10.0, -4)
         0.001,       //  [4] = pow(10.0, -3)
         0.01,        //  [5] = pow(10.0, -2)
         0.1,         //  [6] = pow(10.0, -1)
         1.0,         //  [7] = pow(10.0,  0)
         10.0,        //  [8] = pow(10.0,  1)
         100.0,       //  [9] = pow(10.0,  2)
         1000.0,      // [10] = pow(10.0,  3)
         10000.0,     // [11] = pow(10.0,  4)
         100000.0,    // [12] = pow(10.0,  5)
         1000000.0,   // [13] = pow(10.0,  6)
         10000000.0,  // [14] = pow(10.0,  7)
         100000000.0  // [15] = pow(10.0,  8)
      };

   int index = power + 7;
   if ( index >= 0 && index < (int)M_NUMBER_OF_ARRAY_ELEMENTS(s_powers10) )
      return s_powers10[index];

   return MMath::Pow(10.0, (double)power);
}

double MMath::Pow2(int power)
{
   static const double s_negativePowers2 [] = 
      {
         0.00390625,  //  [0] = pow(2.0, -8)
         0.0078125,   //  [0] = pow(2.0, -7)
         0.015625,    //  [1] = pow(2.0, -6)
         0.03125,     //  [2] = pow(2.0, -5)
         0.0625,      //  [3] = pow(2.0, -4)
         0.125,       //  [4] = pow(2.0, -3)
         0.25,        //  [5] = pow(2.0, -2)
         0.5          //  [6] = pow(2.0, -1)
      };

   if ( power >= 0 )
   {
      if ( power < 32 )
         return double(unsigned(1u << power));
   }
   else 
   {
      int powerIndex = power + M_NUMBER_OF_ARRAY_ELEMENTS(s_negativePowers2);
      if ( powerIndex >= 0 )
         return s_negativePowers2[powerIndex];
   }

   return MMath::Pow(2.0, (double)power);
}

double MMath::Pow(double x, double y)
{
   MEMath::BeforeDoingMath();
   double result = pow(x, y);
   MEMath::AfterDoingMath(result, "Pow");
   return result;
}

double MMath::Exp(double num)
{
   MEMath::BeforeDoingMath();
   double result = exp(num);
   MEMath::AfterDoingMath(result, "Exp");
   return result;
}

double MMath::Log(double num)
{
   MEMath::BeforeDoingMath();
   double result = log(num);
   MEMath::AfterDoingMath(result, "Log");
   return result;
}

double MMath::Log10(double num)
{
   MEMath::BeforeDoingMath();
   double result = log10(num);
   MEMath::AfterDoingMath(result, "Log10");
   return result;
}

int MMath::Rand()
{
#if RAND_MAX < SHRT_MAX
   #error "Please make sure the value of RAND_MAX is supported"
#endif

   int result = rand();

   // Appears, at least some rand() implementations have better randomization at the lower bits
   result &= 0xFFF;
   result |= (rand() & 0xFFF) << 12;
   result |= (rand() & 0x07F) << 24;
   M_ASSERT(result >= 0); // make sure it is positive after all
   return result;
}

unsigned MMath::RandomInRange(unsigned minimum, unsigned maximum)
{
   M_ASSERT(minimum <= maximum); // signal the bad range on debug
   if ( minimum > maximum )      // but swap the values on release
      std::swap(minimum, maximum);

   unsigned bits = maximum - minimum;
   if ( bits == 0 ) // maximum and minimum are the same
      return minimum;
   unsigned result = static_cast<unsigned>(rand());

   // Appears, at least some rand() implementations have better randomization at the lower bits
   if ( bits > 0xFFF )
   {
      result &= 0xFFF;
      result |= (rand() & 0xFFFu) << 12;
      result |= (rand() & 0x0FFu) << 24;
      if ( bits == UINT_MAX )
      {
         M_ASSERT(minimum == 0 && maximum == UINT_MAX); // otherwise there would be an assert already
         return result;
      }
   }
   return minimum + result % (bits + 1);
}

double MMath::RandomFloat()
{
   // The orginal idea of the algo belongs to Isaku Wada,
   // many different derivations can be found on the web
   // The algorithms is modified as we can use RandomInRange.
   //
   // Random value within range [0.0 .. 1.0) with 53-bit resolution
   // is computed using 2 ** 53 == 9007199254740992.0 and 2 ** 26 == 67108864.0.

   double result;
   do
   {
      Muint32 x = RandomInRange(0u, 0x07FFFFFF);
      Muint32 y = RandomInRange(0u, 0x00FFFFFF);
      result = (x * 67108864.0 + y) * (1.0 / 9007199254740992.0);
      M_ASSERT(result >= 0.0 && result <= 1.0);
   } while ( result == 1.0 ); // this can happen rarely in various rounding modes, redo the randomization in this case
   return result;
}

double MMath::RandomFloatInRange(double minimum, double maximum)
{
   if ( IsNanOrInf(minimum) )
      return minimum;
   if ( IsNanOrInf(maximum) )
      return maximum;

   M_ASSERT(minimum <= maximum); // signal the bad range on debug
   if ( minimum > maximum )      // but swap the values on release
      std::swap(minimum, maximum);

   if ( minimum == maximum || minimum == nextafter(maximum, minimum) ) // if the range is empty
      return minimum;

   double diff;

   // Check if diff would exceed std::numeric_limits<double>::max()
   if ( minimum < (-std::numeric_limits<double>::max() / 2.0) || maximum > (std::numeric_limits<double>::max() / 2.0) )
   {                    // and if it exceeds, divide the range, and randomly check which subrange to return
      diff = maximum / 2.0 - minimum / 2.0;
      if ( MMath::RandomInRange(0, 1) == 0 ) // either 0 or 1, random choise of which subrange to use
         minimum += diff;
   }
   else
      diff = maximum - minimum;

   double result;
   do
   {
      result = minimum + diff * RandomFloat();
      M_ASSERT(result >= minimum && result <= maximum);
   } while ( result == maximum ); // this is to prevent returning an exact upper bound, can happen rarely in various rounding modes
   return result;
}
