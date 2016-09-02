// File MCORE/MTimer.cpp

#include "MCOREExtern.h"
#include "MTimer.h"

   #if !M_NO_REFLECTION

      /// Construct the timer and set the timer event to the time of construction.
      ///
      static MVariant DoNew0()
      {
         MTimer timer;
         return MVariant(&timer, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

      /// Construct the timer from a given timer object, or by supplying a distance to the timer event.
      ///
      /// \param timerOrDistance
      ///     In case this is an object, a check is made to verify this is timer. If so, the timer event gets copied from.
      ///     Otherwise the parameter is interpreted as an integer, a distance in milliseconds
      ///     between the exact moment of the creation and the desired timer event.
      ///     Negative value will mean the event has passed already.
      ///     This way, the sign of the parameter given is negative to the sign of \ref Timer property.
      ///
      static MVariant DoNew1(const MVariant& timerOrDistance)
      {
         MTimer timer;
         if ( timerOrDistance.IsObject() )
         {
            const MTimer* other = M_DYNAMIC_CAST_WITH_THROW(const MTimer, timerOrDistance.DoInterpretAsObject());
            timer = *other;
         }
         else
            timer.SetTimer64(timerOrDistance.AsInt64());
         return MVariant(&timer, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

   #endif

M_START_PROPERTIES(Timer)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT(Timer, IsExpired)
   M_OBJECT_PROPERTY_INT                (Timer, Timer)
   M_CLASS_PROPERTY_READONLY_UINT       (Timer, TickCount)
M_START_METHODS(Timer)
   M_CLASS_SERVICE                      (Timer, Sleep,                  ST_S_unsigned)
   M_CLASS_SERVICE                      (Timer, SecondsToMilliseconds,  ST_int_S_int)
   M_OBJECT_SERVICE                     (Timer, ResetTimer,             ST_X)
   M_CLASS_FRIEND_SERVICE_OVERLOADED    (Timer, New, DoNew0, 0,         ST_MVariant_S)
   M_CLASS_FRIEND_SERVICE_OVERLOADED    (Timer, New, DoNew1, 1,         ST_MVariant_S_constMVariantA)
M_END_CLASS(Timer, Object)

   using namespace std;

MTimer::MTimer(TimerIntegerType milliseconds) M_NO_THROW
{
#if M_TIMER64_SUPPORT
   SetTimer64(milliseconds);
#else
   SetTimer(milliseconds);
#endif
}

bool MTimer::IsExpired() const M_NO_THROW
{
   return DoGetTimerNative() >= 0;
}

void MTimer::SetTimer(int milliseconds) M_NO_THROW
{
#if M_TIMER64_SUPPORT
   SetTimer64(static_cast<Mint64>(milliseconds));
#else
   m_timerMoment = DoGetTickCountNative();
   m_timerMoment += milliseconds; // no problem if milliseconds is negative
#endif
}

void MTimer::SetTimer64(Mint64 milliseconds) M_NO_THROW
{
#if M_TIMER64_SUPPORT
   m_timerMoment = DoGetTickCountNative();
   m_timerMoment += milliseconds; // no problem if milliseconds is negative
#else
   M_ASSERT(milliseconds >= INT_MIN && milliseconds <= INT_MAX);
   SetTimer(static_cast<int>(milliseconds));
#endif
}

MTimer::TimerIntegerType MTimer::DoGetTimerNative() const M_NO_THROW
{
   TimerIntegerType val = DoGetTickCountNative() - m_timerMoment;
   return val;
}

MTimer::TimerUnsignedType MTimer::DoGetTickCountNative() M_NO_THROW
{
   TimerUnsignedType result;
   #if (M_OS & M_OS_WINDOWS) != 0
      #if M_TIMER64_SUPPORT // compiling for Vista and above
         result = ::GetTickCount64();
      #else
         result = ::GetTickCount(); // will roll over in a month
      #endif
   #elif (M_OS & M_OS_CMX)
      extern uint64_t volatile Rtc_MillisecTick;
      result = Rtc_MillisecTick;
   // #elif ... // case of a system when librt.a with clock_gettime() is not present
   //  struct timeval tvl;
   //  gettimeofday(&tvl, NULL);
   //  Muint64 millisec = tvl.tv_usec / 1000;  // make milliseconds from microseconds
   //  M_ASSERT(millisec < 1000);
   //  result = (static_cast<Muint64>(tvl.tv_sec) * 1000) + millisec;
   #else
      struct timespec tv;
      clock_gettime(CLOCK_MONOTONIC, &tv);
      Muint64 millisec = tv.tv_nsec / 1000000;  // make milliseconds from nano
      M_ASSERT(millisec < 1000);
      result = (static_cast<Muint64>(tv.tv_sec) * 1000) + millisec;
   #endif
   return result;
}

void MTimer::Sleep(unsigned milliseconds) M_NO_THROW
{
   #if (M_OS & M_OS_WINDOWS) != 0
      // Windows has a problem - sometime Sleep sleeps for smaller time than the tick count.
      // We don't know which one is correct, so we take the maximum of both
      if ( milliseconds > 0 && milliseconds < INT_MAX ) // if it makes any sense to be so precise.
      {                                                   // Use INT_MAX because the following code handles signed values
         unsigned currentTick = ::GetTickCount(); // no need to use GetTickCount64 here
         unsigned endTick = currentTick + milliseconds;
         for ( ;; )
         {
            ::Sleep((DWORD)milliseconds);
            currentTick = ::GetTickCount();
            int diff = int(endTick - currentTick); // this works with two's complement integers
            if ( diff <= 0 )
               break;
            milliseconds = (unsigned)diff;
         }
      }
      else
         ::Sleep((DWORD)milliseconds);
   #elif (M_OS & M_OS_POSIX) != 0
      timespec tsc;
      tsc.tv_sec = static_cast<time_t>(milliseconds / 1000u);             // seconds
      tsc.tv_nsec = static_cast<long>((milliseconds % 1000u) * 1000000u); // nanoseconds
      #if (defined(_POSIX_TIMERS) && (_POSIX_TIMERS+0 >= 0)) || (defined(_XOPEN_REALTIME) && (_XOPEN_REALTIME+0 >= 0))
         int status = 0;
         do
         {
            timespec rem = {};
            status = nanosleep(&tsc, &rem);
            tsc = rem;
         } while ( status < 0 && EINTR == errno );
         M_ASSERT(status >= 0);
      #else
         const int status = pselect(0, 0, 0, 0, &tsc, 0);
         M_ASSERT((status >= 0 || errno == EINTR));
         M_USED_VARIABLE(status);
      #endif
   #elif (M_OS & M_OS_CMX)
   #else
      #error "Define sleep function, possibly through semi-standard C function delay(milliseconds), or usleep() or nanosleep()"
   #endif
}

#if !M_NO_VARIANT
unsigned MTimer::GetEmbeddedSizeof() const
{
   M_COMPILED_ASSERT(sizeof(MTimer) <= sizeof(MVariant::ObjectByValue));
   return sizeof(MTimer);
}
#endif
