// File MCORE/private/MTimeZoneAndroid.cxx

#ifndef M__TIMEZONE_USE_ANDROID_IMPLEMENTATION
   #error "Do not compile .cxx files directly, they are included!"
#endif

#include <jni.h> // repeat the include purely for the IDE to recognize types
#include "MJavaEnv.h"

   static const char s_androidClassName[] = "com/elster/MTools/android/DynamicTimeZone";

   static jmethodID s_idConstructor = NULL;
   static jmethodID s_idGetStandardName = NULL;
   static jmethodID s_idGetDaylightName = NULL;
   static jmethodID s_idGetDisplayName = NULL;
   static jmethodID s_idClone = NULL;
   static jmethodID s_idEquals = NULL;
   static jmethodID s_idGetCurrent = NULL;
   static jmethodID s_idGetAllTimeZoneNames = NULL;
   static jmethodID s_idGetAllTimeZoneDisplayNames = NULL;
   static jmethodID s_idGetAllTimeZoneLocalNames = NULL;
   static jmethodID s_idIsDST = NULL;
   static jmethodID s_idGetUtcToLocalOffset = NULL;
   static jmethodID s_idGetLocalToUtcOffset = NULL;

   static jclass DoCreateTimeZoneClass(MJavaEnv& env)
   {
      // Check the last ID, this way we know if all are initialized
      //
      jclass clazz = env.FindClass(s_androidClassName); // do not cache class object, it is thread dependent!
      if ( s_idGetLocalToUtcOffset == NULL ) // but cache methods
      {
         s_idConstructor = env.GetMethodID(clazz, "<init>", "(Ljava/lang/String;)V");
         s_idGetStandardName = env.GetMethodID(clazz, "getStandardName", "()Ljava/lang/String;");
         s_idGetDaylightName = env.GetMethodID(clazz, "getDaylightName", "()Ljava/lang/String;");
         s_idGetDisplayName  = env.GetMethodID(clazz, "getDisplayName", "()Ljava/lang/String;");
         s_idClone = env.GetMethodID(clazz, "clone", "()Lcom/elster/MTools/android/DynamicTimeZone;");
         s_idEquals = env.GetMethodID(clazz, "equals", "(Lcom/elster/MTools/android/DynamicTimeZone;)Z");
         s_idGetCurrent = env.GetStaticMethodID(clazz, "getCurrent", "()Lcom/elster/MTools/android/DynamicTimeZone;");
         s_idGetAllTimeZoneNames = env.GetStaticMethodID(clazz, "getAllTimeZoneNames", "()[Ljava/lang/String;");
         s_idGetAllTimeZoneDisplayNames = env.GetStaticMethodID(clazz, "getAllTimeZoneDisplayNames", "()[Ljava/lang/String;");
         s_idGetAllTimeZoneLocalNames = env.GetStaticMethodID(clazz, "getAllTimeZoneLocalNames", "()[Ljava/lang/String;");
         s_idIsDST = env.GetMethodID(clazz, "isDST", "(JZ)Z");
         s_idGetUtcToLocalOffset = env.GetMethodID(clazz, "getUtcToLocalOffset", "(J)I");
         s_idGetLocalToUtcOffset = env.GetMethodID(clazz, "getLocalToUtcOffset", "(J)I");
      }
      return clazz;
   }

MTimeZone::DynamicTimeZone& MTimeZone::DynamicTimeZone::operator=(const DynamicTimeZone& other)
{
   Reset();
   M_ASSERT(m_timeZone == NULL);
   M_ASSERT(!m_isInitialized);

   m_isInitialized = other.m_isInitialized;
   if ( other.m_timeZone != NULL )
   {
      MJavaEnv env;
      jobject zone = env->CallObjectMethod(other.m_timeZone, s_idClone);
      env.CheckForJavaException();
      m_timeZone = env->NewGlobalRef(zone);
   }
   return *this;
}

bool MTimeZone::DynamicTimeZone::operator==(const DynamicTimeZone& other) const
{
   if ( m_isInitialized != other.m_isInitialized || m_timeZone == NULL || other.m_timeZone == NULL ) // easy cases
      return false;

   // both timezones are Java objects

   MJavaEnv env;
   jboolean val = env->CallBooleanMethod(m_timeZone, s_idEquals, other.m_timeZone);
   env.CheckForJavaException();
   return val == JNI_FALSE ? false : true;
}

void MTimeZone::DynamicTimeZone::Reset()
{
   m_isInitialized = false;
   if ( m_timeZone != NULL )
   {
      jobject savedTimeZone = m_timeZone;
      m_timeZone = NULL;

      MJavaEnv env;
      env->DeleteGlobalRef(savedTimeZone); // free pointer, mark for garbage collector
      env.CheckForJavaException();
   }
}

void MTimeZone::DoSetFromLocalJavaObject(MJavaEnv& env, jobject zone)
{
   m_dynamic.m_timeZone = env->NewGlobalRef(zone);
   m_dynamic.m_isInitialized = true;

   jstring name = reinterpret_cast<jstring>(env->CallObjectMethod(zone, s_idGetStandardName));
   env.CheckForJavaException();
   const char* cStr = env->GetStringUTFChars(name, 0);
   m_standardName = cStr;
   env->ReleaseStringUTFChars(name, cStr);
   env->DeleteLocalRef(name);

   name = reinterpret_cast<jstring>(env->CallObjectMethod(zone, s_idGetDaylightName));
   env.CheckForJavaException();
   cStr = env->GetStringUTFChars(name, 0);
   m_daylightName = cStr;
   env->ReleaseStringUTFChars(name, cStr);
   env->DeleteLocalRef(name);

   name = reinterpret_cast<jstring>(env->CallObjectMethod(zone, s_idGetDisplayName));
   env.CheckForJavaException();
   cStr = env->GetStringUTFChars(name, 0);
   m_displayName = cStr;
   env->ReleaseStringUTFChars(name, cStr);
   env->DeleteLocalRef(name);

   const MTime& now = MTime::GetCurrentUtcTime();
   m_standardOffset = GetStandardOffsetForTime(now);
   m_daylightOffset = GetDaylightOffsetForYear(now.GetYear());
   DoComputeRecurringSwitchTimes();
}

bool MTimeZone::DoSetByName(const MStdString& originalName)
{
   Clear();

   MJavaEnv env;

   // Exceptions here are valid cases of bad environment

   jclass c = DoCreateTimeZoneClass(env); // throws an exception at failure
   jstring originalNameJ = env.NewLocalStringUTF(originalName);

   try
   {
      // Exceptions here are due to a bad name

      jobject zone = env->NewObject(c, s_idConstructor, originalNameJ);
      env.CheckForJavaException();
      DoSetFromLocalJavaObject(env, zone);
      return true;
   }
   catch ( ... )
   {
   }
   return false;
}

void MTimeZone::SetFromCurrentSystem()
{
   Clear();

   MJavaEnv env;
   jclass c = DoCreateTimeZoneClass(env); // throws an exception at failure
   jobject zone = env->CallStaticObjectMethod(c, s_idGetCurrent);
   env.CheckForJavaException();
   DoSetFromLocalJavaObject(env, zone);
}

bool MTimeZone::IsDST(const MTime& t, bool isTimeUtc) const
{
   if ( m_dynamic.GetInitialized() )
   {
      MJavaEnv env;
      jlong seconds = static_cast<jlong>(t.GetSecondsSince1970());
      jboolean val = env->CallBooleanMethod(m_dynamic.m_timeZone, s_idIsDST, seconds, isTimeUtc ? JNI_TRUE : JNI_FALSE);
      env.CheckForJavaException();
      return val == JNI_FALSE ? false : true;
   }
   return DoStaticTestIfDST(t, m_switchToDaylightTime, m_switchToStandardTime, m_standardOffset, m_daylightOffset, isTimeUtc);
}

int MTimeZone::GetUtcToLocalOffset(const MTime& t) const
{
   if ( m_dynamic.GetInitialized() )
   {
      MJavaEnv env;
      jlong seconds = static_cast<jlong>(t.GetSecondsSince1970());
      jint val = env->CallIntMethod(m_dynamic.m_timeZone, s_idGetUtcToLocalOffset, seconds);
      env.CheckForJavaException();
      return static_cast<int>(val);
   }
   int offset = m_standardOffset;
   if ( DoStaticTestIfDST(t, m_switchToDaylightTime, m_switchToStandardTime, m_standardOffset, m_daylightOffset, true) )
      offset += m_daylightOffset;
   return offset;
}

int MTimeZone::GetLocalToUtcOffset(const MTime& t) const
{
   if ( m_dynamic.GetInitialized() )
   {
      MJavaEnv env;
      jlong seconds = static_cast<jlong>(t.GetSecondsSince1970());
      jint val = env->CallIntMethod(m_dynamic.m_timeZone, s_idGetLocalToUtcOffset, seconds);
      env.CheckForJavaException();
      return static_cast<int>(val);
   }
   int offset = -m_standardOffset;
   if ( DoStaticTestIfDST(t, m_switchToDaylightTime, m_switchToStandardTime, m_standardOffset, m_daylightOffset, false) )
      offset -= m_daylightOffset;
   return offset;
}

   static void DoFillCollectionWith(MStdStringVector& result, jmethodID* id)
   {
      MJavaEnv env;
      M_ASSERT(result.empty());
      jclass c = DoCreateTimeZoneClass(env); // static method, create class
      jobjectArray names = reinterpret_cast<jobjectArray>(env->CallStaticObjectMethod(c, *id));
      env.CheckForJavaException(); // names will not be returned if there is an exception

      int size = env->GetArrayLength(names);
      for ( int i = 0; i < size; ++i )
      {
         jstring jStr = (jstring)env->GetObjectArrayElement(names, i);
         env.CheckForJavaException();
         const char* cStr = env->GetStringUTFChars(jStr, NULL);
         env.CheckForJavaException();
         result.push_back(cStr);
         env->ReleaseStringUTFChars(jStr, cStr);
         env->DeleteLocalRef(jStr);
      }
      env->DeleteLocalRef(names);
   }

MStdStringVector MTimeZone::GetAllTimeZoneNames()
{
   MStdStringVector result;
   DoFillCollectionWith(result, &s_idGetAllTimeZoneNames); // need to pass an address as at the time of the call s_idGetAllTimeZoneNames might not be initialized
   return result;
}

MStdStringVector MTimeZone::GetAllTimeZoneDisplayNames()
{
   MStdStringVector result;
   DoFillCollectionWith(result, &s_idGetAllTimeZoneDisplayNames); // need to pass an address as at the time of the call s_idGetAllTimeZoneDisplayNames might not be initialized
   return result;
}

MStdStringVector MTimeZone::GetAllTimeZoneLocalNames()
{
   MStdStringVector result;
   DoFillCollectionWith(result, &s_idGetAllTimeZoneLocalNames); // need to pass an address as at the time of the call s_idGetAllTimeZoneLocalNames might not be initialized
   return result;
}
