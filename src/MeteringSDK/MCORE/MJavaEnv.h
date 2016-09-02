#ifndef MCORE_MJAVAENV_H
#define MCORE_MJAVAENV_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MJavaEnv.h

#include <MCORE/MCOREDefs.h>

#if !M_NO_JNI

#if (M_OS & M_OS_ANDROID) != 0
   #include <android/asset_manager.h>
   #include <android/asset_manager_jni.h>
#endif

/// Scoped Java environment handler
///
/// Use this class as a wrapper of methods that call Java code from C++ as it provides
/// java environment variable for the current thread context.
/// Typical, most generic and reliable example of full use of the class:
/// \code
///    void MyCppClass::MyCppClass() // constructor
///    {
///       MJavaEnv env;
///       jclass c = env.FindClass("org/me/MyJavaClass"); // throws an exception at failure
///       jmethodID idConstructor = env.GetMethodID(c, "<init>", "()V");
///       jobject o = env->NewObject(c, idConstructor, portNameJ);
///       env.CheckForJavaException(); // the above c and o were local objects, they need not be destroyed
///                                    // if the exception is raised
///
///       // Globalize local references and store them in class.
///       // Storing the class reference is optional as all method IDs
///       //    can be resolved here in the constructor.
///       m_javaClass = static_cast<jclass>(env->NewGlobalRef(c));
///       m_javaObject = env->NewGlobalRef(o);
///    }
///
///    void MyCppClass::~MyCppClass() // destructor
///    {
///       MJavaEnv env;
///       // If we do not have to call anything in Java at destruction, just delete references
///       env->DeleteGlobalRef(m_javaObject); // free pointer, mark for garbage collector
///       env->DeleteGlobalRef(m_javaClass);  // (only if a global reference is stored)
///    }
///
///    void MyCppClass::MyCppMethodThatCallsJavaMethods()
///    {
///       MJavaEnv env;
///
///       // If method ID was not already fetched somewhere
///       jmethodId methodId = env.GetMethodID(m_javaClass, "someJavaMethod", "()V");
///
///       env->CallVoidMethod(m_javaObject, methodId);
///       env.CheckForJavaException();
///    }
/// \endcode
///
/// As a slight optimization it is possible to reuse java environment class among several calls,
/// however one has to be careful as java environment has to be created and released within a
/// context of a single Java call, or a single detached C++ thread.
///
class M_CLASS MJavaEnv
{
public: // Constants:

   enum
   {
      JniVersion = JNI_VERSION_1_4 ///< JNI version used by the interface
   };

private: // Types

   // Vector of java objects typically used to hold local references
   //
   typedef std::vector<jobject>
      JavaObjectVector;

public:

   /// Most useful constructor that fetches the environment and stores it internally for further use.
   ///
   /// The class has to be created in C++ code that is by itself working within Java.
   /// If necessary, and the current C++ thread is not registered with Java, the registering is performed.
   ///
   MJavaEnv();

   /// Constructor that takes an explicit environment
   ///
   /// This has to be called within the registered Java thread when the environment is received already.
   /// It is assumed that the java environment is already registered with java thread.
   ///
   /// \param env Java environment to be used within the object.
   ///
   MJavaEnv(JNIEnv* env);

   /// Destructor that unregisters the java environment.
   ///
   /// If the environment was executed in a context of a C++ only thread it is unregistered.
   ///
   ~MJavaEnv();

   ///@{
   /// Access the native java environment.
   ///
   /// This call is rarely necessary.
   ///
   /// \see operator->()
   ///
   JNIEnv* GetEnv()
   {
      M_ASSERT(m_env != NULL);
      return m_env;
   }
   const JNIEnv* GetEnv() const
   {
      M_ASSERT(m_env != NULL);
      return m_env;
   }
   ///@}

   ///@{
   /// Call JNI methods using this environment class.
   ///
   /// This operator saves a call of GetEnv() so that instead of
   /// \code
   ///   jmethodID idRead = env.GetEnv()->GetMethodID(clazz, "read", "([BII)I");
   /// \endcode
   /// one has:
   /// \code
   ///   jmethodID idRead = env->GetMethodID(clazz, "read", "([BII)I");
   /// \endcode
   /// Notice, MJavaEnv provides a number of wrapper methods that in addition to natives
   /// verifies the result of the operation and throws an exception at failure.
   /// For example, notice '.' instead of '->'
   /// \code
   ///   jmethodID idRead = env.GetMethodID(clazz, "read", "([BII)I"); // throws if such method is not found
   /// \endcode
   ///
   /// \return Native java environment variable.
   ///
   JNIEnv* operator->()
   {
       return GetEnv();
   }
   const JNIEnv* operator->() const
   {
       return GetEnv();
   }
   ///@}

   /// Add a given Java object to the list of local objects that have to be deleted at destruction.
   ///
   /// This method is a handy way of dealing with JNI local references that have to be removed
   /// at the exit of method.
   ///
   void AddToLocalObjects(jobject obj);

   ///@{
   /// Create a new Java string from UTF string and add it to the list of local references.
   ///
   /// \param str String in UTF encoding from which a Java string has to be created.
   ///
   /// \return Java string object.
   ///
   jstring NewLocalStringUTF(const char* str);
   jstring NewLocalStringUTF(const MStdString& str)
   {
      return NewLocalStringUTF(str.c_str());
   }
   ///@}

   /// Create a new uninitialized Java byte array and add it to the list of local references.
   ///
   /// \param size Size of the array.
   ///
   /// \return Java byte array object.
   ///
   jbyteArray NewLocalByteArray(int size);

   /// Create a new initialized Java byte array and add it to the list of local references.
   ///
   /// \param buff Buffer with byte values.
   /// \param size Size of the buffer and the result byte array.
   ///
   /// \return Java byte array object.
   ///
   jbyteArray NewLocalByteArray(const char* buff, int size);

   /// Object method that initializes the java exception in the current environment from a given C++ exception.
   ///
   /// The robustness of this method assumes the presence of MTools Java facade that
   /// provides the exception hierarchy that matches the one of MeteringSDK.
   /// After the call is made, the execution should immediately reach Java code
   /// for the exception to be raised from there.
   ///
   /// \param ex C++ exception that has to be converted into java exception of the current environment.
   ///
   /// \see StaticExceptionCppToJava - static version that is more convenient
   ///                                 for cases when the native JNIEnv is already present.
   /// \see CheckForJavaException - reverse method, convert java exception, if it is present, into C++.
   ///
   void ExceptionCppToJava(MException& ex)
   {
      StaticExceptionCppToJava(m_env, ex);
   }

   /// Class method that initializes java exception in the current environment from a given C++ exception.
   ///
   /// The robustness of this method assumes the presence of MTools Java facade that
   /// provides the exception hierarchy that matches the one of MeteringSDK.
   /// After the call is made the execution should immediately reach Java code
   /// for the exception to be raised from there.
   ///
   /// \param jenv Native Java environment.
   /// \param ex   C++ exception that has to be converted into java exception for the current environment.
   ///
   /// \see ExceptionCppToJava - non-static version that is more convenient when there is MJavaEnv present.
   /// \see CheckForJavaException - reverse method, convert java exception, if it is present, into C++.
   ///
   static void StaticExceptionCppToJava(JNIEnv* jenv, MException& ex);

   /// Convert java exception, if it is present, into C++.
   ///
   /// This call should be made after any method such as JNI Call*Method that can raise Java exceptions.
   /// If a Java exception takes place, it is converted into a C++ exception of the most appropriate MeteringSDK exception class
   /// and thrown as C++ exception into C++. Any non-MeteringSDK exceptions will be converted into \ref MException.
   ///
   /// In between Call*Method and CheckForJavaException only very few JNI methods can be called:
   /// \code
   ///    ExceptionOccurred()
   ///    ExceptionDescribe()
   ///    ExceptionClear()
   ///    ExceptionCheck()
   ///    ReleaseStringChars()
   ///    ReleaseStringUTFChars()
   ///    ReleaseStringCritical()
   ///    Release<Type>ArrayElements()
   ///    ReleasePrimitiveArrayCritical()
   ///    DeleteLocalRef()
   ///    DeleteGlobalRef()
   ///    DeleteWeakGlobalRef()
   ///    MonitorExit()
   ///    PushLocalFrame()
   ///    PopLocalFrame()
   /// \endcode
   /// as specified at http://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/design.html#exception_handling
   ///
   void CheckForJavaException();

   /// Check for JNI error code.
   ///
   /// Throw a C++ exception if the given result of JNI function is erroneous, not equal to JNI_OK.
   ///
   /// \param code         Result of a JNI function such as AttachCurrentThread or GetEnv.
   /// \param errorMessage Whatever will be the error message if code is not JNI_OK.
   ///                     This string should not have C format parameters.
   ///
   static void CheckForJniError(jint code, const char* errorMessage);

   /// Find Java class, throw error if it is not found.
   ///
   /// Different from JNI FindClass, still available as
   /// \code
   ///    jclass c = env->FindClass("org/me/MyJavaClass");
   /// \endcode
   /// this object method, callable with a period like this:
   /// \code
   ///    jclass c = env.FindClass("org/me/MyJavaClass");
   /// \endcode
   /// will throw an exception if there is no such class.
   ///
   /// \param javaClassName Java class name signature as specified by Java.
   ///
   /// \return JNI class object.
   ///
   jclass FindClass(const char* javaClassName);

   /// Get Java method ID, throw error if it is not found.
   ///
   /// Different from JNI GetMethodID, still available as
   /// \code
   ///    jclass c = env->GetMethodID(clazz, "methodName", "()V");
   /// \endcode
   /// this object method, callable with a period like this:
   /// \code
   ///    jclass c = env.GetMethodID(clazz, "methodName", "()V");
   /// \endcode
   /// will throw an exception if there is no such method.
   ///
   /// \param clazz Java class object.
   /// \param name Method name.
   /// \param signature Method signature, as defined by Java.
   ///
   /// \return JNI method ID, not a null.
   ///
   /// \see GetStaticMethodID - for getting an ID of a class method.
   ///
   jmethodID GetMethodID(jclass clazz, const char* name, const char* signature);

   /// Get Java static method ID, throw error if it is not found.
   ///
   /// Different from JNI GetStaticMethodID, still available as
   /// \code
   ///    jclass c = env->GetStaticMethodID(clazz, "methodName", "()V");
   /// \endcode
   /// this object method, callable with a period like this:
   /// \code
   ///    jclass c = env.GetStaticMethodID(clazz, "methodName", "()V");
   /// \endcode
   /// will throw an exception if there is no such method.
   ///
   /// \param clazz Java class object.
   /// \param name Method name.
   /// \param signature Method signature, as defined by Java.
   ///
   /// \return JNI method ID, not a null.
   ///
   /// \see GetMethodID - for getting an ID of an object method.
   ///
   jmethodID GetStaticMethodID(jclass clazz, const char* name, const char* signature);

   /// Get Java field ID, throw error if it is not found.
   ///
   /// Different from JNI GetFieldID, still available as
   /// \code
   ///    jclass c = env->GetFieldID(clazz, "fieldName", "I");
   /// \endcode
   /// this object method, callable with a period like this:
   /// \code
   ///    jclass c = env.GetFieldID(clazz, "fieldName", "I");
   /// \endcode
   /// will throw an exception if there is no such field.
   ///
   /// \param clazz Java class object.
   /// \param name Field name.
   /// \param signature Field signature, as defined by Java.
   ///
   /// \return JNI field ID, not a null.
   ///
   /// \see GetStaticMethodID - for getting an ID of a class method.
   ///
   jfieldID GetFieldID(jclass clazz, const char* name, const char* signature);

   /// Get Java static field ID, throw error if it is not found.
   ///
   /// Different from JNI GetStaticFieldID, still available as
   /// \code
   ///    jclass c = env->GetStaticFieldID(clazz, "fieldName", "I");
   /// \endcode
   /// this object method, callable with a period like this:
   /// \code
   ///    jclass c = env.GetStaticFieldID(clazz, "fieldName", "I");
   /// \endcode
   /// will throw an exception if there is no such static field.
   ///
   /// \param clazz Java class object.
   /// \param name Field name.
   /// \param signature Field signature, as defined by Java.
   ///
   /// \return JNI field ID, not a null.
   ///
   /// \see GetStaticMethodID - for getting an ID of a class method.
   ///
   jfieldID GetStaticFieldID(jclass clazz, const char* name, const char* signature);

#if (M_OS & M_OS_ANDROID) != 0

   /// Access asset manager Java object of the Android application from native code.
   ///
   /// The call returns a Java asset manager local object, not a JNI one.
   /// No need to delete it after it is used, will be done automatically at destructor.
   /// The Java code should have already called
   /// \code
   ///    ContextSingleton.setContext(getApplicationContext());
   /// \endcode
   ///
   /// \see GetJniAssetManager static method to access a JNI C++ asset manager object.
   ///
   jobject GetAssetManager();

   /// Access asset manager JNI object of the Android application from native code.
   ///
   /// The call returns a native asset manager object, not a Java one.
   /// No need to delete it after it is used.
   /// No need to create MJavaEnv for this class to operate,
   /// however the Java code should have already called
   /// \code
   ///    ContextSingleton.setContext(getApplicationContext());
   /// \endcode
   ///
   /// \see GetAssetManager non-static method to access a Java asset manager object.
   ///
   static AAssetManager* GetJniAssetManager();
#endif

private: // Data:

   // Native JNI environment
   //
   JNIEnv* m_env;

   // Collection of local references that should be freed at destruction of the thread.
   //
   JavaObjectVector m_localObjects;

   // Whether the above environment was attached to Java thread locally in the constructor.
   // This is used by destructor to detach thread.
   //
   bool m_attachedToThread;
};

#endif // !M_NO_JNI

///@}
#endif
