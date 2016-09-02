// File MCORE/MMath.cpp

#include "MCOREExtern.h"
#include "MMath.h"
#include "MException.h"

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
   M_CLASS_SERVICE_OVERLOADED       (Math, Round, Round0,    1,   ST_double_S_double)  // SWIG_HIDE
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
   M_CLASS_SERVICE                  (Math, BinaryMantissa,        ST_double_S_double)
   M_CLASS_SERVICE                  (Math, BinaryExponent,        ST_int_S_double)
M_END_CLASS(Math, Object)

#if !M_NO_VARIANT

MVariant MMath::Min(const MVariant& a, const MVariant& b)
{
   return a < b ? a : b;
}

MVariant MMath::Max(const MVariant& a, const MVariant& b)
{
   return a < b ? b : a;
}

MVariant MMath::Abs(const MVariant& num)
{
   return (num >= 0) ? num : -num;
}

#endif // !M_NO_VARIANT

double MMath::Round0(double val)
{
   if ( val >= 0.0 )
      return floor(val + 0.5);
   else
      return ceil(val - 0.5);
}

double MMath::Round(double val, int numDecimalPlaces)
{
   if ( numDecimalPlaces == 0 ) // special much faster and more precise case
      return Round0(val);

   double multiplier = Pow10(numDecimalPlaces);
   return Round0(val * multiplier) / multiplier;
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
   M_ASSERT(minimum <= maximum); // signal on debug, return a value badly outside range on release
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
