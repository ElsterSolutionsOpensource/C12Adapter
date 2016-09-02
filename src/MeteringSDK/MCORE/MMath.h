#ifndef MCORE_MMATH_H
#define MCORE_MMATH_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MMath.h

#include <MCORE/MCOREDefs.h>
#include <MCORE/MObject.h>

/// MMath is derived from MObject, and it only has static properties and methods.
/// No instances of the MMath class are possible.
///
class M_CLASS MMath : public MObject
{
public: // Properties:

   /// Get the PI constant.
   ///
   static double GetPI()
   {
      return 3.14159265358979323846;
   }

   /// Get the E constant.
   ///
   static double GetE()
   {
      return 2.7182818284590452354;
   }

public: // Services:

#if !M_NO_VARIANT

   /// Returns the smaller of the two two given parameters
   ///
   /// Standard comparison operator is used.
   ///
   static MVariant Min(const MVariant& a, const MVariant& b);

   /// Returns the bigger of the two given parameters
   ///
   /// Standard comparison operator is used.
   ///
   static MVariant Max(const MVariant& a, const MVariant& b);

   /// Returns the absolute value of the given parameter
   ///
   static MVariant Abs(const MVariant& num);

#endif

   /// Returns the closest from the left integer number to a given double.
   /// The result value is a double precision number.
   /// Here is the usage:
   /// \code
   ///       double x = MMath::Floor(555.5);    // 555
   ///       double y = MMath::Floor(444.4);    // 444
   /// \endcode
   ///
   static double Floor(double num)
   {
      return floor(num);
   }

   /// Returns the closest from the right integer number to a given double.
   /// The result value is a double precision number.
   /// Here is the usage:
   /// \code
   ///       double x = MMath::Ceil(555.5);     // 556
   ///       double y = MMath::Ceil(444.4);     // 445
   /// \endcode
   ///
   static double Ceil(double num)
   {
      return ceil(num);
   }

   /// Round the floating number with the zero digits after the comma.
   /// This is to address the issue that the standard C library does not have the round service.
   /// Here is the usage:
   /// \code
   ///       double x = MMath::Round0(555.5);    // 556
   ///       double y = MMath::Round0(444.4);    // 444
   /// \endcode
   ///
   static double Round0(double val);

   /// Round the floating number, using the number of the floating point digits
   /// after comma. Note that the function can yield an imprecise result.
   /// If numDecimalPlaces is zero, the behavior is equivalent to Round0.
   /// Here is the usage:
   /// \code
   ///       double x = MMath::Round(0.555, 1);    // 0.6
   ///       double y = MMath::Round(1.234567, 2); // 1.23
   /// \endcode
   ///
   static double Round(double val, int numDecimalPlaces = 0);

   ///@{
   /// Round a given integer number to a nearest power of two, which is bigger than or equal to the initial number.
   ///
   /// For negative numbers or zero, value 1 is returned, other values: 2 -> 2, 3 -> 4, 4 -> 4, 9 -> 16 and so on.
   ///
   /// \param x
   ///    The number for which an upper power of two has to be found
   ///
   /// \return The result number
   ///
   static int RoundUpToPowerOfTwo(int x)
   {
      int num;
      if ( x <= 0 )
         num = 1;
      else
      {
         num = x - 1;
         num |= num >> 1;
         num |= num >> 2;
         num |= num >> 4;
         num |= num >> 8;
         num |= num >> 16;
         ++num;
      }
      M_ASSERT(num >= x && num != 0 && (num & (num - 1)) == 0); // check if the number is indeed a power of two
      return num;
   }
   static unsigned RoundUpToPowerOfTwo(unsigned x)
   {
      unsigned num;
      if ( x == 0 )
         num = 1;
      else
      {
         num = x - 1;
         num |= num >> 1;
         num |= num >> 2;
         num |= num >> 4;
         num |= num >> 8;
         num |= num >> 16;
         ++num;
      }
      M_ASSERT(num >= x && num != 0 && (num & (num - 1)) == 0); // check if the number is indeed a power of two
      return num;
   }
   ///@}

   /// Returns the square root of the given parameter.
   ///
   static double Sqrt(double arg);

   /// Facility function that efficiently returns the integer power of ten.
   ///
   /// \param power Into which power 10 has to be raised
   /// \return result value
   ///
   static double Pow10(int power);

   /// Facility function that efficiently returns the integer power of two.
   ///
   /// Negative numbers are respected.
   ///
   /// \param power Into which power 2 has to be raised
   /// \return result value
   ///
   static double Pow2(int power);

   /// Value of x raised into power y.
   ///
   /// \param x Which number to raise
   /// \param y Power
   /// \return result value
   ///
   static double Pow(double x, double y);

   /// Returns the base-e exponential function of num, which is e raised to the power num: e^num.
   ///
   static double Exp(double num);

   /// Returns the natural logarithm of num.
   /// The natural logarithm is the base-e logarithm: the inverse of the 
   /// natural exponential function (Exp). 
   ///
   static double Log(double num);

   /// Returns the common (base-10) logarithm of num.
   ///
   static double Log10(double num);

   /// Returns the sine of an angle of num radians.
   ///
   /// \param num
   ///  Floating point value representing an angle expressed in radians.
   ///
   /// \return Sine of num.
   ///
   static double Sin(double num)
   {
      return sin(num);
   }

   /// Returns the cosine of an angle of num radians.
   ///
   /// \param num
   ///  Floating point value representing an angle expressed in radians.
   ///
   /// \return Cosine of num.
   ///
   static double Cos(double num)
   {
      return cos(num);
   }

   /// Returns the tangent of an angle of num radians.
   ///
   /// \param num
   ///  Floating point value representing an angle expressed in radians.
   ///
   /// \return Tangent of num.
   ///
   static double Tan(double num)
   {
      return tan(num);
   }

   /// Returns the principal value of the arc sine of num, expressed in radians.
   /// In trigonometrics, arc sine is the inverse operation of sine.
   ///
   /// \param num
   ///  Floating point value in the interval [-1,+1].
   ///
   /// \return Arc sine of num, in the interval [-pi/2,+pi/2] radians.
   ///
   static double Asin(double num)
   {
      return asin(num);
   }

   /// Returns the principal value of the arc cosine of num, expressed in radians.
   /// In trigonometrics, arc cosine is the inverse operation of cosine.
   ///
   /// \param num Floating point value in the interval [-1, +1].
   ///
   /// \return Arc cosine of num, in the interval  [0, pi] radians.
   ///
   static double Acos(double num)
   {
      return acos(num);
   }

   /// Returns the principal value of the arc tangent of x, expressed in radians.
   /// In trigonometrics, arc tangent is the inverse operation of tangent.
   ///
   /// \param num Floating point value.
   ///
   /// \return Principal arc tangent of num, in the interval [-pi/2, +pi/2] radians.
   ///
   static double Atan(double num)
   {
      return atan(num);
   }

   /// Returns a pseudo-random integral number in the range between 0 and INT_MAX
   ///
   /// The randomizer seed is guaranteed to be initialized before this call
   /// only if MeteringSDK threading is used.
   /// The returned value is not secure, but the algorithm is reasonably fast.
   ///
   /// \return random number within 0 .. INT_MAX, independent on the host operating system.
   ///
   /// \see \ref RandomInRange for a function that returns an unsigned value within a given range
   ///
   static int Rand();

   /// Convenience function that returns an unsigned number within a given inclusive range.
   ///
   /// Linear distribution of probability is applied.
   /// Maximum value should not be smaller than minimum value, and there is a debug check,
   /// and the release build will return values outside the given range.
   ///
   /// \param minimum Minimum value, inclusive
   /// \param maximum Minimum value, inclusive, up until UINT_MAX.
   /// \return random unsigned number within the given range
   ///
   /// \see \ref Rand for a function with no parameters that returns a value in range 0 .. INT_MAX.
   ///
   static unsigned RandomInRange(unsigned minimum, unsigned maximum);

   /// Returns the binary mantissa of a given number, range 0.5 to 1.0.
   /// This is done by calling a C function frexp.
   ///
   static double BinaryMantissa(double value)
   {
      int exponent;
      return frexp(value, &exponent);
   }

   /// Returns the binary exponent of a given number.
   /// This is done by calling a C function frexp.
   ///
   static double BinaryExponent(double value)
   {
      int exponent;
      frexp(value, &exponent);
      return exponent;
   }

private: // Prevent certain operations on MMath:

   MMath(const MMath&);
   void operator=(const MMath&);
   bool operator==(const MMath&) const;
   bool operator!=(const MMath&) const;

   M_DECLARE_CLASS(Math)
};


/// Round the floating number to the type specified as template parameter.
/// This function has to be used with caution, as not all compilers support
/// the templates that do not depend on actual parameters.
///
/// Please use a macro M_ROUND_TO(type, value) if the compatibility
/// with the older C++ compilers has to be achieved.
/// Here is the usage:
/// \code
///       int i = MRoundTo<int>(0.5);
///       char j = MRoundTo<char>(1.8);
/// \endcode
///
template
   <typename T>
T MRoundTo(double d)
{
   if ( d >= 0.0 )
      return (T)(d + 0.5);
   else
      return (T)(d - 0.5);
}

/// Round the floating number to the type specified as template parameter.
///
/// The feature is implemented as a macro for compiler compatibility reasons,
/// and it is equivalent to the function MMath::RoundTo<type>(value),
/// except that the usage is different.
///
/// \pre The first parameter has to be the integer type,
/// the second parameter has to be of type double. Certain checking is provided
/// for compilers that support function templates with template parameters that
/// do not depend on the function parameters.
///
#define M_ROUND_TO(type, d) MRoundTo<type>(d)

///@}
#endif
