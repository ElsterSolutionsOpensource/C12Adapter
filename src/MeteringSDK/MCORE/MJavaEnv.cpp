// File MCORE/MJavaEnv.cpp

#include "MCOREExtern.h"
#include "MException.h"
#include "MUtilities.h"
#include "MJavaEnv.h"

#if !M_NO_JNI

namespace
{

   // Internal namespace for Java environment and related functionality.
   //
   class JavaGlobalEnvironment
   {
   public: // Methods:

      // Static constructor
      static void Init(JavaVM* jvm)
      {
         M_ASSERT(s_cachedJvm == NULL && s_classLoaderClass == NULL && s_classLoaderFindClassMethod == NULL); // singleton

         s_cachedJvm = jvm;
         M_ASSERT(s_cachedJvm != NULL);

         JNIEnv* env = NULL;
         jint rc = s_cachedJvm->GetEnv((void**)&env, MJavaEnv::JniVersion);
         M_USED_VARIABLE(rc);
         M_ASSERT(rc == JNI_OK); // if this fails truly nothing will work, even Java exceptions
         M_ASSERT(env != NULL);

         // Class loader has to be cached in OnLoad in order to be able to find classes from native threads, see
         //    http://stackoverflow.com/questions/13263340/findclass-from-any-thread-in-android-jni

         jclass temporaryClass = env->FindClass(s_anyClassInProject);
         if ( temporaryClass == NULL )
         {
            M_ASSERT(env->ExceptionOccurred());
            return;
         }

         jclass temporaryClassClass = env->GetObjectClass(temporaryClass);
         if ( temporaryClassClass == NULL )
         {
            M_ASSERT(env->ExceptionOccurred());
            return;
         }

         jclass classLoaderClass = env->FindClass("java/lang/ClassLoader");
         if ( classLoaderClass == NULL )
         {
            M_ASSERT(env->ExceptionOccurred());
            return;
         }

         jmethodID getClassLoaderMethod = env->GetMethodID(temporaryClassClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
         if ( getClassLoaderMethod == NULL )
         {
            M_ASSERT(env->ExceptionOccurred());
            return;
         }

         jclass classLoaderClassLocal = (jclass)env->CallObjectMethod(temporaryClass, getClassLoaderMethod);
         if ( classLoaderClassLocal == NULL )
         {
            M_ASSERT(env->ExceptionOccurred());
            return;
         }

         s_classLoaderFindClassMethod = env->GetMethodID(classLoaderClass, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
         if ( s_classLoaderFindClassMethod == NULL )
         {
            M_ASSERT(env->ExceptionOccurred());
            return;
         }

         s_classLoaderClass = static_cast<jclass>(env->NewGlobalRef(classLoaderClassLocal));
         if ( s_classLoaderClass == NULL )
         {
            M_ASSERT(env->ExceptionOccurred());
            return;
         }
      }

      // Static destructor
      static void Done(JNIEnv* env)
      {
         if ( s_classLoaderClass != NULL )
         {
            env->DeleteGlobalRef(s_classLoaderClass); // free pointer, mark for garbage collector
            s_classLoaderClass = NULL;
         }
         s_cachedJvm = NULL;
      }

      static JavaVM* GetJavaVM()
      {
         M_ASSERT(s_cachedJvm != NULL);
         return s_cachedJvm;
      }

      static jclass FindClassThroughLoader(JNIEnv* env, jstring classNameJString)
      {
         M_ASSERT(s_cachedJvm == NULL && s_classLoaderClass == NULL && s_classLoaderFindClassMethod == NULL); // singleton

         // Attempt to load a class with a loader
         // Class loader has to be cached in OnLoad in order to be able to find classes from native threads, see
         //    http://stackoverflow.com/questions/13263340/findclass-from-any-thread-in-android-jni
         //
         return (jclass)env->CallObjectMethod(s_classLoaderClass, s_classLoaderFindClassMethod, classNameJString);
      }

   private: // No instance possible

      JavaGlobalEnvironment();
      ~JavaGlobalEnvironment();

   private: // Data:

      static JavaVM*    s_cachedJvm;
      static jclass     s_classLoaderClass;
      static jmethodID  s_classLoaderFindClassMethod;
      static const char s_anyClassInProject[];
   };

   JavaVM*    JavaGlobalEnvironment::s_cachedJvm = NULL;
   jclass     JavaGlobalEnvironment::s_classLoaderClass = NULL;
   jmethodID  JavaGlobalEnvironment::s_classLoaderFindClassMethod = NULL;
   const char JavaGlobalEnvironment::s_anyClassInProject[] = "com/elster/MTools/MException";

} // anonymous namespace

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* jvm, void*)
{
   JavaGlobalEnvironment::Init(jvm);
   return MJavaEnv::JniVersion;
}

extern "C" JNIEXPORT void JNICALL Java_com_elster_MTools_MToolsHelpers_unloadMToolsLibrary(JNIEnv* env, jclass)
{
   // As it appears, the garbage collector can run after calling the shutdown hook.
   // Therefore, currently there is no reliable way of releasing library resources.
   // Destroying the context is commented out currently.
   //
   // JavaGlobalEnvironment::Done(env);
}

MJavaEnv::MJavaEnv()
:
   m_env(NULL),
   m_localObjects(),
   m_attachedToThread(false)
{
   JavaVM* vm = JavaGlobalEnvironment::GetJavaVM();
   if ( vm == NULL ) // release mode, just in case, if for any strange reason JNI_OnLoad was not called
   {
      MException::Throw(MException::ErrorSoftware, M_ERR_JAVA_NATIVE_INTERFACE_ERROR, "Java native interface not initialized");
      M_ENSURED_ASSERT(0);
   }
   jint rc = vm->GetEnv((void**)&m_env, MJavaEnv::JniVersion);
   if ( rc == JNI_EDETACHED )
   {
      M_ASSERT(m_env == NULL); // nothing is allocated yet

      JavaVMAttachArgs args;
      args.version = MJavaEnv::JniVersion;
      args.name = NULL;
      args.group = NULL;

      #if (M_OS & M_OS_ANDROID) != 0
         rc = vm->AttachCurrentThread(&m_env, &args);
      #else
         rc = vm->AttachCurrentThread((void**)&m_env, &args);
      #endif
      CheckForJniError(rc, "Failed to attach native thread");
      m_attachedToThread = true;
   }
   else
      CheckForJniError(rc, "Failed to access Java environment");
   M_ASSERT(m_env != NULL);
}

MJavaEnv::MJavaEnv(JNIEnv* env)
:
   m_env(env),
   m_localObjects(),
   m_attachedToThread(false)
{
}

MJavaEnv::~MJavaEnv()
{
   JavaObjectVector::iterator it = m_localObjects.begin();
   JavaObjectVector::iterator itEnd = m_localObjects.end();
   for ( ; it != itEnd; ++it )
      m_env->DeleteLocalRef(*it);

   if ( m_attachedToThread )
   {
       JavaVM* vm = JavaGlobalEnvironment::GetJavaVM();
       jint rc = vm->DetachCurrentThread();
       M_ASSERT(rc == JNI_OK);
       M_USED_VARIABLE(rc);
   }
}

void MJavaEnv::AddToLocalObjects(jobject obj)
{
   M_ASSERT(std::find(m_localObjects.begin(), m_localObjects.end(), obj) == m_localObjects.end()); // not there already
   m_localObjects.push_back(obj);
}

jstring MJavaEnv::NewLocalStringUTF(const char* str)
{
   jstring obj = m_env->NewStringUTF(str);
   AddToLocalObjects((jobject)obj);
   return obj;
}

jbyteArray MJavaEnv::NewLocalByteArray(int size)
{
   jbyteArray obj = m_env->NewByteArray(static_cast<jsize>(size));
   AddToLocalObjects((jobject)obj);
   return obj;
}

jbyteArray MJavaEnv::NewLocalByteArray(const char* buff, int size)
{
   jbyteArray obj = NewLocalByteArray(size);
   m_env->SetByteArrayRegion(obj, 0, static_cast<jint>(size), reinterpret_cast<const jbyte*>(buff));
   return obj;
}

void MJavaEnv::CheckForJavaException()
{
   if ( m_env->ExceptionCheck() )
   {
      MStdString msg;
      jthrowable ex = m_env->ExceptionOccurred();
      m_env->ExceptionClear();
      jclass clazz = m_env->GetObjectClass(ex);

      jfieldID getSwigCPtrField = m_env->GetFieldID(clazz, "swigCPtr", "J "); // long
      if ( getSwigCPtrField != NULL )
      {
         jlong ptr = m_env->GetLongField(ex, getSwigCPtrField);
         if ( ptr != 0 )
         {
            MException* cEx = reinterpret_cast<MException*>(ptr);
            M_ASSERT(cEx->GetKind() >= 0 && cEx->GetKind() < 0x20); // delicate way of checking this is MException, indeed
            cEx->Rethrow();
            // no need to deallocate or delete anything here!
            M_ENSURED_ASSERT(0);
         }
      }
      else
      {
         m_env->ExceptionClear(); // have to clear whatever happened
      }

      jmethodID getMessageMethod = m_env->GetMethodID(clazz, "getMessage", "()Ljava/lang/String;");
      if ( getMessageMethod == NULL )
      {
         m_env->ExceptionClear();
         MException::Throw(MException::ErrorSoftware, M_ERR_JAVA_NATIVE_INTERFACE_ERROR, "Could not find Java Exception.getMessage()");
         M_ENSURED_ASSERT(0);
      }
      jstring message = static_cast<jstring>(m_env->CallObjectMethod(ex, getMessageMethod));
      const char* cMessage = m_env->GetStringUTFChars(message, NULL);
      if ( cMessage != NULL )
      {
         msg = cMessage;
         m_env->ReleaseStringUTFChars(message, cMessage);
      }
      else
      {
         m_env->ExceptionClear();
      }
      m_env->DeleteLocalRef(clazz);
      MException::Throw(MException::ErrorSoftware, M_ERR_JAVA_NATIVE_INTERFACE_ERROR, "%s", msg.c_str());
      M_ENSURED_ASSERT(0);
   }
}

void MJavaEnv::CheckForJniError(jint code, const char* errorMessage)
{
   M_ASSERT(strchr(errorMessage, '%') == NULL); // no format specifiers allowed
   if ( code != JNI_OK )
   {
      MException::Throw(MException::ErrorSoftware, M_ERR_JAVA_NATIVE_INTERFACE_ERROR, "%s, JNI error %d", errorMessage, static_cast<int>(code));
      M_ENSURED_ASSERT(0);
   }
}

void MJavaEnv::StaticExceptionCppToJava(JNIEnv* jenv, MException& ex)
{
   // Here is an example of how to throw a custom exception
   //   http://monochrome.sutic.nu/2013/09/01/nice-jni-exceptions.html

   static const char s_classPrefix[] = "com/elster/MTools/M";

   char name [ 64 ];
   memcpy(name, s_classPrefix, sizeof(s_classPrefix) - 1);

   const MClass* cl = ex.GetClass();
   strncpy(name + sizeof(s_classPrefix) - 1, cl->GetName(), sizeof(name) - sizeof(s_classPrefix) - 1);
   name[sizeof(name) - 1] = '\0';

   jclass c = jenv->FindClass(name);
   if ( c == NULL )
   {
      M_ASSERT(0); // signal on debug, but recover in Release build
      c = jenv->FindClass("com/elster/MTools/MException");
      if ( c == NULL )
      {
         jclass ce = jenv->FindClass("java/lang/NoClassDefFoundError");
         std::string msg = "Classes not found: com/elster/MTools/MException, ";
         msg += name;
         jenv->ThrowNew(ce, msg.c_str());
         return;
      }
   }

   jmethodID constructor = jenv->GetMethodID(c, "<init>", "(JZ)V");
   jlong newEx = reinterpret_cast<jlong>(ex.NewClone());
   jobject exception = jenv->NewObject(c, constructor, newEx, JNI_TRUE);
   jenv->Throw(static_cast<jthrowable>(exception));
   jenv->DeleteLocalRef(c);
}

jclass MJavaEnv::FindClass(const char* javaClassName)
{
   jclass c = m_env->FindClass(javaClassName);
   if ( c == NULL )
   {
      m_env->ExceptionClear();

      // Attempt to load a class with a loader
      // Class loader has to be cached in OnLoad in order to be able to find classes from native threads, see
      //    http://stackoverflow.com/questions/13263340/findclass-from-any-thread-in-android-jni
      //
      jstring classNameJString = NewLocalStringUTF(javaClassName);
      c = JavaGlobalEnvironment::FindClassThroughLoader(m_env, classNameJString);
      if ( c == NULL )
      {
         m_env->ExceptionClear();

         MException::Throw(MException::ErrorSoftware, M_ERR_JAVA_NATIVE_INTERFACE_ERROR, "Java native interface could not find class '%s'", javaClassName);
         M_ENSURED_ASSERT(0);
      }
   }
   AddToLocalObjects((jobject)c);
   return c;
}

jmethodID MJavaEnv::GetMethodID(jclass clazz, const char* name, const char* signature)
{
   jmethodID method = m_env->GetMethodID(clazz, name, signature);
   if ( method == NULL )
   {
      m_env->ExceptionClear();
      MException::Throw(MException::ErrorSoftware, M_ERR_JAVA_NATIVE_INTERFACE_ERROR, "Java native interface could not find method '%s(%s)'", name, signature);
      M_ENSURED_ASSERT(0);
   }
   return method;
}

jmethodID MJavaEnv::GetStaticMethodID(jclass clazz, const char* name, const char* signature)
{
   jmethodID method = m_env->GetStaticMethodID(clazz, name, signature);
   if ( method == NULL )
   {
      m_env->ExceptionClear();
      MException::Throw(MException::ErrorSoftware, M_ERR_JAVA_NATIVE_INTERFACE_ERROR, "Java native interface could not find static method '%s(%s)'", name, signature);
      M_ENSURED_ASSERT(0);
   }
   return method;
}

jfieldID MJavaEnv::GetFieldID(jclass clazz, const char* name, const char* signature)
{
   jfieldID field = m_env->GetFieldID(clazz, name, signature);
   if ( field == NULL )
   {
      m_env->ExceptionClear();
      MException::Throw(MException::ErrorSoftware, M_ERR_JAVA_NATIVE_INTERFACE_ERROR, "Java native interface could not find field '%s %s'", signature, name);
      M_ENSURED_ASSERT(0);
   }
   return field;
}

jfieldID MJavaEnv::GetStaticFieldID(jclass clazz, const char* name, const char* signature)
{
   jfieldID field = m_env->GetStaticFieldID(clazz, name, signature);
   if ( field == NULL )
   {
      m_env->ExceptionClear();
      MException::Throw(MException::ErrorSoftware, M_ERR_JAVA_NATIVE_INTERFACE_ERROR, "Java native interface could not find field 'static %s %s'", signature, name);
      M_ENSURED_ASSERT(0);
   }
   return field;
}

#if (M_OS & M_OS_ANDROID) != 0

   static jmethodID  s_idGetAssetManager = NULL;
   static const char s_androidContextSingletonClassName[] = "com/elster/MTools/android/ContextSingleton";

jobject MJavaEnv::GetAssetManager()
{
   jclass clazz = FindClass(s_androidContextSingletonClassName);
   if ( s_idGetAssetManager == NULL ) // but cache methods (ok if called in multithreaded context as there is no need to delete method IDs)
      s_idGetAssetManager = GetStaticMethodID(clazz, "getAssetManager", "()Landroid/content/res/AssetManager;");
   jobject result = m_env->CallStaticObjectMethod(clazz, s_idGetAssetManager);
   CheckForJavaException();
   AddToLocalObjects(result);
   return result;
}

AAssetManager* MJavaEnv::GetJniAssetManager()
{
   MJavaEnv env;
   jobject jAssetManager = env.GetAssetManager();
   AAssetManager* result = AAssetManager_fromJava(env.m_env, jAssetManager);
   return result;
}

#endif // (M_OS & M_OS_ANDROID) != 0

#endif // !M_NO_JNI
