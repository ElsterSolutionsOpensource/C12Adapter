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

   /// Returns the smaller of the two two given parameters.
   ///
   /// Internally the standard less-than comparison operator is used to compare the given values.
   ///
   /// \param v1 One value, a number or a string, anything comparable using the order operator.
   /// \param v2 Another value, a number or a string, anything comparable using the order operator.
   /// \return Either v1 or v2, whatever is smaller. If the values are equal then v2 is returned (same as v2, supposedly).
   ///
   static MVariant Min(const MVariant& v1, const MVariant& v2);

   /// Returns the bigger of the two given parameters.
   ///
   /// Internally the standard less-than comparison operator is used to compare the given values.
   ///
   /// \param v1 One value, a number or a string, anything comparable using the order operator.
   /// \param v2 Another value, a number or a string, anything comparable using the order operator.
   /// \return Either v1 or v2, whatever is bigger. If the values are equal then v1 is returned (same as v2, supposedly).
   ///
   static MVariant Max(const MVariant& v1, const MVariant& v2);

   /// Returns the absolute value of the given parameter.
   ///
   /// Internally the standard less-than comparison operator is used to compare the given number with zero.
   /// If the number is less than zero unary minus operator is applied.
   ///
   /// \param num Numeric value, positive or negative.
   /// \return If num is negative -num is returned, otherwise num itself is returned.
   ///
   static MVariant Abs(const MVariant& num);

#endif

   /// Returns the closest from the left integer number to a given double precision floating point number.
   ///
   /// In case the number is positive, the returned number will be no bigger than the one given.
   /// In case the number is negative, the returned number will be no smaller than the one given.
   /// If the number is NaN or any form of infinity, the same value is returned.
   ///
   /// \param num A number.
   /// \return The result value is a double precision number, floor.
   ///
   /// Usage:
   /// \code
   ///       double x = MMath::Floor(555.5);   // x == 555.0
   ///       double y = MMath::Floor(444.1);   // y == 444.0
   ///       double z = MMath::Floor(-2.6);    // z == -3.0
   /// \endcode
   ///
   /// \see \ref Ceil - return the whole number closest from the right
   ///
   static double Floor(double num)
   {
      return floor(num);
   }

   /// Returns the closest from the right integer number to a given double precision floating point number.
   ///
   /// In case the number is positive, the returned number will be no smaller than the one given.
   /// In case the number is negative, the returned number will be no bigger than the one given.
   /// If the number is NaN or any form of infinity, the same value is returned.
   ///
   /// \param num A number.
   /// \return The result value is a double precision number, ceiling.
   ///
   /// \code
   ///       double x = MMath::Ceil(555.5);   // x == 556.0
   ///       double y = MMath::Ceil(444.4);   // y == 445.0
   ///       double z = MMath::Ceil(-2.6);    // z == -2.0
   /// \endcode
   ///
   /// \see \ref Floor - return the whole number closest from the left
   ///
   static double Ceil(double num)
   {
      return ceil(num);
   }

   /// Round the floating point value to the nearest whole number.
   ///
   /// If the number is NaN or any form of infinity, the same value is returned.
   ///
   /// Rounding of halves is done upwards for positive numbers and downwards for negatives. In detail:
   ///   - In case the number is positive, and it ends with x.5, the returned number
   ///     will be the seiling of the one given so that 1.5 will be rounded up to 2.0, and so on.
   ///   - In case the number is negative, and it ends with x.5, the returned number
   ///     will be the floor of the one given so that -1.5 will be rounded down to -2.0, and so on.
   ///
   /// The method attempts to perform the rounding precisely using the rules described above,
   /// however since IEEE-754 floating point numbers are binary,
   /// their approsimation to decimal cannot always be precise.
   ///
   /// \param val Double precision floating point number to round.
   /// \return Result double precision floating point value rounded to the closest whole number.
   ///
   /// Usage:
   /// \if CPP
   /// \code
   ///       double x = MMath::Round0(555.5);    // 556.0
   ///       double y = MMath::Round0(444.4);    // 444.0
   ///       double z = MMath::Round0(-77.5);    // -78.0
   /// \endcode
   /// \else
   /// \code
   ///       x := @Math.Round(555.5);    // 556.0
   ///       y := @Math.Round(444.4);    // 444.0
   ///       z := @Math.Round(-77.5);    // -78.0
   /// \endcode
   /// \endif
   ///
   static double Round0(double val);

   /// Round the floating point number to the given decimal digits after comma.
   ///
   /// If the number is NaN or any form of infinity, the same value is returned.
   ///
   /// Depending on the second parameter:
   ///   - Rounding to zero digits after comma is equivalent to rounding to the nearest whole number.
   ///   - Rounding to the positive number of digits is done to the nearest
   ///     number of 1/10 for 1, 1/100 for 2, 1/1000 for 3, and so on.
   ///   - Rounding to the negative number of digits is done to the nearest
   ///     number of tens for 1, hundredths for 2, thousands for 3, and so on.
   ///
   /// Rounding of halves is done upwards for positive numbers and downwards for negatives. In detail:
   ///   - In case the number is positive, and it ends with x.5, the returned number
   ///     will be the seiling of the one given so that 1.5 will be rounded up to 2.0, and so on.
   ///   - In case the number is negative, and it ends with x.5, the returned number
   ///     will be the floor of the one given so that -1.5 will be rounded down to -2.0, and so on.
   ///
   /// The method attempts to perform the rounding precisely using the rules described above,
   /// however since IEEE-754 floating point numbers are binary,
   /// their approsimation to decimal cannot always be precise.
   ///
   /// \param val Double precision floating point number to round.
   /// \param numDecimalPlaces Number of decimal places after comma, zero by default.
   /// \return Result double precision floating point value approximated to the closest value
   ///          with the given number of decimal points.
   ///
   /// Usage:
   /// \if CPP
   /// \code
   ///       double a = MMath::Round(0.555, 2);    // 0.56
   ///       double b = MMath::Round(1.234567, 3); // 1.235
   ///       double c = MMath::Round(3456.78, -2); // 3500.0
   ///       double d = MMath::Round(-1.5, 0);     // 2.0
   /// \endcode
   /// \else
   /// \code
   ///       a = @Math.Round(0.555, 2);    // 0.56
   ///       b = @Math.Round(1.234567, 3); // 1.235
   ///       c = @Math.Round(3456.78, -2); // 3500.0
   ///       d = @Math.Round(-1.5, 0);     // 2.0
   ///       d = @Math.Round(-0.5);        // 1.0 (last parameter is defaulted to 0)
   /// \endcode
   /// \endif
   ///
   static double Round(double val, int numDecimalPlaces = 0);

   /// Round a given integer number to the nearest power of two, which is bigger or equal to the initial number.
   ///
   /// For negative numbers or zero, the value 1 is returned.
   /// Examples of other values: 2 -> 2, 3 -> 4, 4 -> 4, 9 -> 16, and so on.
   ///
   /// \param x The number for which an upper power of two has to be found.
   /// \return The result power of two number, bigger or equal to x.
   ///
   /// \see \ref RoundUpToPowerOfTwo(unsigned x) - unsigned integer variant of this method.
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

   /// Round a given unsigned integer number to the nearest power of two, which is bigger or equal to the initial number.
   ///
   /// Examples of transformations made by this method: 2 -> 2, 3 -> 4, 4 -> 4, 9 -> 16, and so on.
   ///
   /// \param x The number for which an upper power of two has to be found.
   /// \return The result power of two number, bigger or equal to x.
   ///
   /// \see \ref RoundUpToPowerOfTwo(int x) - signed integer variant of this method.
   ///
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

   /// Returns the square root of the given parameter.
   ///
   /// If the argument is negative an error of type \ref MEMath is raised.
   ///
   /// \param arg Value to find square root from.
   /// \return result value.
   ///
   static double Sqrt(double arg);

   /// Facility function that efficiently returns the integer power of ten.
   ///
   /// An error of type \ref MEMath can be raised in case of overflow.
   ///
   /// \param power Into which power 10 has to be raised.
   /// \return result value.
   ///
   static double Pow10(int power);

   /// Facility function that efficiently returns the integer power of two.
   ///
   /// Negative numbers are respected.
   /// An error of type \ref MEMath can be raised in case of overflow.
   ///
   /// \param power Into which power 2 has to be raised.
   /// \return result value.
   ///
   static double Pow2(int power);

   /// Value of x raised into power y.
   ///
   /// An error of type \ref MEMath can be raised.
   ///
   /// \param x Which number to raise.
   /// \param y Power.
   /// \return result value.
   ///
   static double Pow(double x, double y);

   /// Returns the base-e exponential function of num, which is e raised to the power num: e^num.
   ///
   /// An error of type \ref MEMath can be raised.
   ///
   /// \param num Which number is the power of e.
   /// \return result exponent value.
   ///
   static double Exp(double num);

   /// Returns the natural logarithm of the number.
   ///
   /// The natural logarithm is the base-e logarithm: the inverse of the
   /// natural exponential function (Exp).
   ///
   /// An error of type \ref MEMath wil be raised if the number is negative.
   ///
   /// \param num Number.
   /// \return result natural logarithm of the number.
   ///
   static double Log(double num);

   /// Returns the common (base-10) logarithm of num.
   ///
   /// An error of type \ref MEMath wil be raised if the number is negative.
   ///
   /// \param num Number.
   /// \return result decimal logarithm of the number.
   ///
   static double Log10(double num);

   /// Returns the sine of an angle of num radians.
   ///
   /// \param num Floating point value representing an angle expressed in radians.
   /// \return Sine of num.
   ///
   static double Sin(double num)
   {
      return sin(num);
   }

   /// Returns the cosine of an angle of num radians.
   ///
   /// \param num Floating point value representing an angle expressed in radians.
   /// \return Cosine of num.
   ///
   static double Cos(double num)
   {
      return cos(num);
   }

   /// Returns the tangent of an angle of num radians.
   ///
   /// \param num Floating point value representing an angle expressed in radians.
   /// \return Tangent of num.
   ///
   static double Tan(double num)
   {
      return tan(num);
   }

   /// Returns the principal value of the arc sine of num, expressed in radians.
   ///
   /// In trigonometry, arc sine is the inverse operation of sine.
   ///
   /// \param num Floating point value in the interval [-1, +1].
   /// \return Arc sine of num, in the interval [-pi/2, +pi/2] radians.
   ///
   static double Asin(double num)
   {
      return asin(num);
   }

   /// Returns the principal value of the arc cosine of num, expressed in radians.
   ///
   /// In trigonometry, arc cosine is the inverse operation of cosine.
   ///
   /// \param num Floating point value in the interval [-1, +1].
   /// \return Arc cosine of num, in the interval  [0, pi] radians.
   ///
   static double Acos(double num)
   {
      return acos(num);
   }

   /// Returns the principal value of the arc tangent of x, expressed in radians.
   ///
   /// In trigonometry, arc tangent is the inverse operation of tangent.
   ///
   /// \param num Floating point value.
   /// \return Principal arc tangent of num, in the interval [-pi/2, +pi/2] radians.
   ///
   static double Atan(double num)
   {
      return atan(num);
   }

   /// Pseudo-random integer number in range 0 .. INT_MAX inclusively.
   ///
   /// The name of the method (rand) has historic meaning
   /// as it is present in ANSI C standard library.
   /// Different from standard, this poarticular call
   /// always returns a value in the inclusive range [0 .. 0x7FFFFFFF].
   ///
   /// The randomizer seed is guaranteed to be initialized before this call
   /// only if MeteringSDK threading is used. The returned value
   /// is not cryptograpically secure, but the algorithm is reasonably fast.
   ///
   /// \return random number within inclusive range 0 .. INT_MAX.
   ///
   /// \see \ref RandomInRange returns an unsigned value within a given range
   /// \see \ref RandomFloat returns a floating point value in range [0 .. 1.0)
   /// \see \ref RandomFloatInRange returns a floating point value within a given range
   ///
   static int Rand();

   /// Pseudo-random unsigned number within a given inclusive range.
   ///
   /// The return value depends on function parameters in the following way:
   ///   - There is a debug check to verify that the maximum value
   ///     is not smaller than minimum. However, on release build,
   ///     when the maximum is smaller than minimum, these two values
   ///     will be swapped so they form a valid range.
   ///   - If the maximum and the minimum are equal, the returned value
   ///     will be the same as the minimum (and the maximum).
   ///   - Otherwise, the return value will be a pseudo-random number within the given
   ///     inclusive range. Linear distribution of probability is applied.
   ///
   /// The randomizer seed is guaranteed to be initialized before this call
   /// only if MeteringSDK threading is used. The returned value
   /// is not cryptograpically secure, but the algorithm is reasonably fast.
   ///
   /// \param minimum Minimum value, inclusive
   /// \param maximum Maximum value, inclusive, up to UINT_MAX.
   /// \return random unsigned number within the given inclusive range
   ///
   /// \see \ref Rand function with no parameters that returns a value in range 0 .. INT_MAX.
   /// \see \ref RandomFloat returns a floating point value in range [0 .. 1.0)
   /// \see \ref RandomFloatInRange returns a floating point value within a given range
   ///
   static unsigned RandomInRange(unsigned minimum, unsigned maximum);

   /// Returns a pseudo-random double precision floating point number in range [0.0 to 1.0).
   ///
   /// Different from integer counterparts, RandomFloat is guaranteed to never return
   /// the value of its upper range, 1.0.
   ///
   /// The randomizer seed is guaranteed to be initialized before this call
   /// only if MeteringSDK threading is used. The returned value
   /// is not cryptograpically secure, but the algorithm is reasonably fast.
   ///
   /// \return random number within range [0.0 .. 1.0).
   ///
   /// \see \ref Rand returns an integer value in range 0 .. INT_MAX.
   /// \see \ref RandomInRange returns an unsigned value within a given range.
   /// \see \ref RandomFloatInRange returns a floating point value within a given range.
   ///
   static double RandomFloat();

   /// Convenience function that returns a doubvle precision floating point number within a given range.
   ///
   /// The return value depends on function parameters in the following way:
   ///   - If minimum is Infinity or NaN, minimum (NaN or Infinity) is returned.
   ///   - If maximum is Infinity or NaN, maximum (NaN or Infinity) is returned.
   ///   - There is a debug check to verify that the maximum value
   ///     is not smaller than minimum. However, on release build,
   ///     when the maximum is smaller than minimum, these two values
   ///     will be swapped so they form a valid range.
   ///   - If the maximum and the minimum are equal, the returned value
   ///     will be the same as the minimum (and the maximum).
   ///   - Otherwise, the return value will be a pseudo-random number within the given
   ///     inclusive range. Linear distribution of probability is applied.
   ///
   /// The randomizer seed is guaranteed to be initialized before this call
   /// only if MeteringSDK threading is used. The returned value
   /// is not cryptograpically secure, but the algorithm is reasonably fast.
   ///
   /// \param minimum Minimum value, inclusive.
   /// \param maximum Maximum value, exclusive unless equal to minimum.
   /// \return random Double precision floating point number within the given range.
   ///
   /// \see \ref Rand returns an integer value within range 0 .. INT_MAX.
   /// \see \ref RandomInRange returns an unsigned value within a given range.
   /// \see \ref RandomFloat returns a floating point value in range [0 .. 1.0).
   ///
   static double RandomFloatInRange(double minimum, double maximum);

   /// Returns the binary mantissa of a given number, range 0.5 to 1.0.
   ///
   /// This is done by calling a C function frexp.
   ///
   /// \param value Source number.
   /// \return Binary mantissa of the value, range 0.5 to 1.0.
   ///
   static double BinaryMantissa(double value)
   {
      int exponent;
      return frexp(value, &exponent);
   }

   /// Returns the binary exponent of a given number.
   ///
   /// This is done by calling a C function frexp.
   ///
   /// \param value Source number.
   /// \return Binary exponent of the value.
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
///
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
   return static_cast<T>(MMath::Round0(d));
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
