#ifndef MCORE_MTIMER_H
#define MCORE_MTIMER_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MTimer.h

#include <MCORE/MCOREDefs.h>
#include <MCORE/MObject.h>
#include <MCORE/MVersion.h>

///@{
/// Whether the platform supports 64-bit timer.
///
/// All platforms except of Windows XP and older support 64-bit timers.
/// This is a compilation option, not runtime. Therefore, if the software
/// has to support Windows XP, compiled accordingly with _WIN32_WINNT below 0x0600,
/// 32-bit timers will be used, and those will roll over in about 25 days.
///
#ifndef M_TIMER64_SUPPORT
   #if (M_OS & M_OS_WINDOWS) != 0
      #define M_TIMER64_SUPPORT (_WIN32_WINNT >= 0x0600) // 64-bit timers are supported since Vista
   #else
      #define M_TIMER64_SUPPORT 1 // all the other platforms support 64-bit timer
   #endif
#endif
///@}

/// Timer with milliseconds resolution.
///
/// The timer has its timer moment, when its \refprop{GetTimer,Timer} call
/// will return zero and when IsExpired will start to return true.
/// When the class is created, and no expire moment is given,
/// the timer expires immediately, and IsExpired starts to return true.
/// This is still useful in case many events need to be traced by the same timer,
/// and for such case, the property \refprop{GetTimer,Timer} can be used to
/// look at the time elapsed since timer creation or reset.
/// It is also acceptable and useful to set expire moment into the past,
/// which is achieved by supplying negative times, as it can simplify
/// program logic for cases when a certain event can take place already.
///
/// On Windows platform, in order to have a real 64-bit timer, one has to
/// define _WIN32_WINNT to 0x0600 or above (Vista or later OS).
/// When Windows XP has to be supported, 64-bit timers will roll over their 32-bit parts.
///
/// Preferred timer interface manipulates with 64-bit data types,
/// while the 32-bit interface is kept for compatibility.
/// The 32-bit interface should not be used for long durations, such as bigger than a week,
/// as it overflows after about 25 days.
///
/// Timer is an embedded object, which is not necessary to destroy
/// when it is created from Reflection interface.
///
class M_CLASS MTimer : public MObject
{
public: // Types:

   ///@{
   /// Timer integer type, the internal type used for comparison of timer events.
   ///
#if M_TIMER64_SUPPORT
   typedef Mint64  TimerIntegerType;
#else
   typedef int TimerIntegerType;
#endif
   ///@}

   ///@{
   /// Timer unsigned type, the internal type used for absolute values since start of some event in the past.
   ///
#if M_TIMER64_SUPPORT
   typedef Muint64  TimerUnsignedType;
#else
   typedef unsigned TimerUnsignedType;
#endif
   ///@}

public: // Services:

   /// Construct the timer and set its event to the given number of milliseconds.
   ///
   /// \param expireInMilliseconds
   ///     Distance in milliseconds between the exact moment of the creation of the object and the desired timer event.
   ///     Negative value will mean the event has happened already in the past.
   ///
   explicit MTimer(TimerIntegerType expireInMilliseconds = 0) M_NO_THROW;

   /// Construct a copy of timer with the same timer event time.
   ///
   MTimer(const MTimer& other)
   :
      m_timerMoment(other.m_timerMoment)
   {
   }

   /// Destroy the timer.
   ///
   virtual ~MTimer()
   {
   }

public: // Operators:

   /// Assignment operator that makes the timer moment of the timer match the other timer start.
   ///
   MTimer& operator=(const MTimer& other)
   {
      m_timerMoment = other.m_timerMoment;
      return *this;
   }

   ///@{
   /// Time comparison operator that accepts milliseconds.
   ///
   /// \param duration
   ///     Milliseconds elapsed since timer creation or reset.
   ///
   bool operator==(TimerIntegerType duration) const
   {
      return DoGetTimerNative() == duration;
   }
   bool operator!=(TimerIntegerType duration) const
   {
      return DoGetTimerNative() != duration;
   }
   bool operator>=(TimerIntegerType duration) const
   {
      return DoGetTimerNative() >= duration;
   }
   bool operator<=(TimerIntegerType duration) const
   {
      return DoGetTimerNative() <= duration;
   }
   bool operator>(TimerIntegerType duration) const
   {
      return DoGetTimerNative() > duration;
   }
   bool operator<(TimerIntegerType duration) const
   {
      return DoGetTimerNative() < duration;
   }
   ///@}

public: // Properties:
   
   /// Whether the timer moment has happened.
   ///
   /// True if the timer moment matches current moment, or it is in the past.
   /// When the timer object was created without parameters, or it was reset,
   /// this method returns true until the timer is set into
   /// a milliseconds moment in the future.
   ///
   bool IsExpired() const M_NO_THROW;

   ///@{
   /// Return the distance in milliseconds between the current time and the timer event.
   ///
   /// Negative distance means the timer event is in the future, IsExpired will be false in such case.
   /// Positive distance, IsExpired is true, will mean the timer event has happened in the past already.
   ///
   /// It is worth mentioning that if the timer value is got right after the timer is assigned, 
   /// the return value will be close to negated assignment value (milliseconds elapsed since assignment will explain the difference).
   /// Setting the timer property moves the zero position of the time scale that is used to get the timer value back.
   ///
   /// 32-bit integer variant of the interface roles over every 25 days, and therefore,
   /// should be used with care.
   ///
   /// \return integer value, distance between the timer event and current moment.
   ///
   /// \see ResetTimer sets the timer event into the exact time of the call.
   /// \see IsExpired checks whether the timer event has happened already.
   ///
   int GetTimer() const M_NO_THROW
   {
      return static_cast<int>(DoGetTimerNative()); // okay if rolls over, this is by design
   }
   void SetTimer(int) M_NO_THROW;
   Mint64 GetTimer64() const M_NO_THROW
   {
      return static_cast<Mint64>(DoGetTimerNative());
   }
   void SetTimer64(Mint64) M_NO_THROW;
   ///@}

   ///@{
   /// Get the number of milliseconds elapsed since some unspecified moment.
   ///
   /// This is a static method, and the returned value is unrelated to the timer moment of any object.
   /// For Windows, the initial moment is the startup time.
   /// The 32-bit version of tick count overflows approximately every 49 days, then starts over again.
   ///
   static unsigned GetTickCount() M_NO_THROW
   {
      return static_cast<unsigned>(DoGetTickCountNative()); // okay if rolls over, this is by design
   }
   static Muint64 GetTickCount64() M_NO_THROW
   {
      return static_cast<Muint64>(DoGetTickCountNative());
   }
   ///@}

public: // Methods:

   /// Sets the timer event into the exact moment this call is made.
   ///
   /// IsExpired will start to return true after this call,
   /// and \refprop{GetTimer,Timer} will be counting milliseconds elapsed since that moment.
   ///
   void ResetTimer() M_NO_THROW
   {
      SetTimer(0);
   }

   /// Sleep for the given number of milliseconds.
   ///
   /// The precision is not guaranteed, but the delay will not be less
   /// than the number of milliseconds specified.
   ///
   /// \param milliseconds
   ///     Time to wait in 1/1000 seconds increments.
   ///
   static void Sleep(unsigned milliseconds) M_NO_THROW;

   ///@{
   /// Convert seconds into milliseconds, where both are integers of 32-bit size.
   ///
   /// A very large value of seconds might result in the number of milliseconds overflowing
   /// the int size. When the number of milliseconds is larger than what fits into int, 
   /// the returned number of milliseconds is the maximum 32-bit positive signed number 
   /// (and it will be smaller than the seconds requested multiplied by 1000). 
   /// In either case, such a large value effectively represents eternity.

   /// \param seconds Seconds to convert.
   /// \return Result number of milliseconds. To prevent overflow, if seconds multiplied 
   ///         by 1000 is larger than the maximum int value (0x7FFFFFFF), the returned value 
   ///         is 0x7FFFFFFF milliseconds.
   ///
   static int SecondsToMilliseconds(int seconds)
   {
      if ( seconds >= (0x7FFFFFFF / 1000) )
         return 0x7FFFFFFF;
      return seconds * 1000;
   }
   ///@}

   /// Convert seconds into milliseconds, respecting the internal representation of the timer.
   ///
   /// A very large value of seconds might result in the number of milliseconds overflowing
   /// the TimerIntegerType size, which could be a 32-bit or 64-bit integer, depending on 
   /// the operating system and build options. When the number of milliseconds overflows, 
   /// the returned number of milliseconds is the maximum 32-bit or 64-bit positive signed number 
   /// (and will be smaller than the seconds requested multiplied by 1000). 
   /// In either case, such a large value effectively represents eternity.
   ///
   /// \param seconds Seconds to convert.
   /// \return Result number of milliseconds. To prevent overflow, if seconds multiplied
   ///         by 1000 is larger than the maximum TimerIntegerType value, the returned value 
   ///        is the maximum TimerIntegerType value.
   ///
   /// \see SecondsToMilliseconds - handles 32-bit integers in all compilation modes
   ///
   static TimerIntegerType SecondsToTimerMilliseconds(TimerIntegerType seconds)
   {
      #if M_TIMER64_SUPPORT
         if ( seconds >= (MINT64C(0x7FFFFFFFFFFFFFFF) / 1000) )
            return MINT64C(0x7FFFFFFFFFFFFFFF);
         return seconds * 1000;
      #else
         if ( seconds >= (0x7FFFFFFF / 1000) )
            return 0x7FFFFFFF;
         return seconds * 1000;
      #endif
   }

#if !M_NO_VARIANT
   /// Timer is an embedded object type, therefore return its size in bytes.
   ///
   /// \return Size of MTimer in bytes.
   ///
   virtual unsigned GetEmbeddedSizeof() const;
#endif

private:

   TimerIntegerType DoGetTimerNative() const M_NO_THROW;
   static TimerUnsignedType DoGetTickCountNative() M_NO_THROW;

private: // Disabled operators

   // Does not make any sense to compare two timers!
   bool operator==(const MTimer&) const;
   bool operator!=(const MTimer&) const;

private: // Data:

   // Saved tick count of the timer
   //
   TimerUnsignedType m_timerMoment;

   M_DECLARE_CLASS(Timer)
};

///@}
#endif 
