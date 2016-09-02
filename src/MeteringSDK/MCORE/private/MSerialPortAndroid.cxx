// File MCORE/MSerialPortPosix.cxx
//
// This file is included, not part of compilation

#ifndef M__SERIAL_PORT_INTERNAL_IMPLEMENTATION
   #error "Do not compile .cxx files directly, they are included!"
#endif

#include <jni.h> // repeat the include purely for the IDE to recognize types
#include "MJavaEnv.h"

   // Copy-paste from Java interface
   //
   enum MSerialPortJavaEnum
   {
       PARITY_NONE = 0,
       PARITY_ODD = 1,
       PARITY_EVEN = 2,
       PARITY_MARK = 3,
       PARITY_SPACE = 4,

       STOP_BITS_ONE = 1,
       STOP_BITS_TWO = 2,
       STOP_BITS_ONE_AND_HALF = 3,

       DTR_CONTROL_DISABLE = 0,
       DTR_CONTROL_ENABLE = 1,
       DTR_CONTROL_HANDSHAKE = 2,

       RTS_CONTROL_DISABLE = 0,
       RTS_CONTROL_ENABLE = 1,
       RTS_CONTROL_HANDSHAKE = 2,
       RTS_CONTROL_TOGGLE = 3
   };

   static const unsigned s_acceptableBauds[] =
      {
         // Note that Windows has a bigger list as Posix does not have 14400u, 28800u, 57600u, 128000u, and 256000u
         300u, 600u, 1200u, 2400u, 4800u, 9600u, 14400u, 19200u, 28800u, 38400u, 57600u, 115200u, 230400u, 460800u, 500000u, 576000u, 921600u, 1000000u, 1152000u, 1500000u, 2000000u, 2500000u, 3000000u, 3500000u, 4000000u, 0
      };

   static const char s_androidClassName[] = "com/elster/MTools/android/GenericSerialPort";

   static jmethodID s_idConstructor = NULL;
   static jmethodID s_idRead = NULL;
   static jmethodID s_idWrite = NULL;
   static jmethodID s_idClearInputBuffer = NULL;
   static jmethodID s_idFlushOutputBuffer = NULL;
   static jmethodID s_idDisconnect = NULL;
   static jmethodID s_idGetBytesReadyToRead = NULL;
   static jmethodID s_idGetDcd = NULL;
   static jmethodID s_idConfigurePortParameters = NULL;
   static jmethodID s_idConfigurePortTimeouts = NULL;
   static jmethodID s_idGetAvailablePortNames = NULL;
   static jmethodID s_idGetPortType = NULL;

   static jclass DoCreatePortClass(MJavaEnv& env)
   {
      // Check the last ID, this way we know if all are initialized
      //
      jclass clazz = env.FindClass(s_androidClassName); // do not cache class object, it is thread dependent!
      if ( s_idGetPortType == NULL ) // but cache methods
      {
         s_idConstructor = env.GetMethodID(clazz, "<init>", "(Ljava/lang/String;)V");
         s_idRead = env.GetMethodID(clazz, "read", "([BII)I");
         s_idWrite = env.GetMethodID(clazz, "write", "([BII)I");
         s_idClearInputBuffer = env.GetMethodID(clazz, "clearInputBuffer", "()V");
         s_idFlushOutputBuffer = env.GetMethodID(clazz, "flushOutputBuffer", "(I)V");
         s_idDisconnect = env.GetMethodID(clazz, "disconnect", "()V");
         s_idGetBytesReadyToRead = env.GetMethodID(clazz, "getBytesReadyToRead", "()I");
         s_idGetDcd = env.GetMethodID(clazz, "getDcd", "()Z");
         s_idConfigurePortParameters = env.GetMethodID(clazz, "configurePortParameters", "(IIIIIIZZZ)V");
         s_idConfigurePortTimeouts = env.GetMethodID(clazz, "configurePortTimeouts", "(II)V");
         s_idGetAvailablePortNames = env.GetStaticMethodID(clazz, "getAvailablePortNames", "(Z)[Ljava/lang/String;");
         s_idGetPortType = env.GetStaticMethodID(clazz, "getPortType", "(Ljava/lang/String;)Ljava/lang/String;");
      }
      return clazz;
   }

void MSerialPort::DoOpen()
{
   MJavaEnv env;

   M_ASSERT(m_port == NULL); // it is a debug level application error to call Open twice

   jclass c = DoCreatePortClass(env); // throws an exception at failure
   jstring portNameJ = env.NewLocalStringUTF(m_portName);
   jobject port = env->NewObject(c, s_idConstructor, portNameJ);
   env.CheckForJavaException();

   m_port = env->NewGlobalRef(port);
}

unsigned MSerialPort::Read(char* buffer, unsigned size)
{
   M_ASSERT(m_port != NULL); // it is a debug level application error
   M_ASSERT(size != 0);
   UpdatePortParametersOrTimeoutsIfChanged();

   MJavaEnv env;
   jbyteArray jBuffer = env.NewLocalByteArray(static_cast<int>(size));
   jint actualSize = env->CallIntMethod(m_port, s_idRead, jBuffer, 0, static_cast<jint>(size));
   env.CheckForJavaException();
   M_ASSERT(static_cast<unsigned>(actualSize) <= size);
   env->GetByteArrayRegion(jBuffer, 0, actualSize, reinterpret_cast<jbyte*>(buffer));
   return static_cast<unsigned>(actualSize);
}

unsigned MSerialPort::Write(const char* buffer, unsigned size)
{
   M_ASSERT(m_port != NULL); // it is a debug level application error
   UpdatePortParametersOrTimeoutsIfChanged();

   MJavaEnv env;
   jbyteArray jBuffer = env.NewLocalByteArray(buffer, static_cast<jsize>(size));
   jint actualSize = env->CallIntMethod(m_port, s_idWrite, jBuffer, 0, static_cast<jint>(size));
   env.CheckForJavaException();
   M_ASSERT(static_cast<unsigned>(actualSize) <= size);
   return static_cast<unsigned>(actualSize);
}

void MSerialPort::ClearInputBuffer() const
{
   M_ASSERT(m_port != NULL); // it is a debug level application error
   // No port configuring here

   MJavaEnv env;
   env->CallVoidMethod(m_port, s_idClearInputBuffer);
   env.CheckForJavaException();
}

void MSerialPort::FlushOutputBuffer(unsigned numberOfCharsInBuffer)
{
   M_ASSERT(m_port != NULL); // it is a debug level application error
   // no port configuring here

   MJavaEnv env;
   env->CallVoidMethod(m_port, s_idFlushOutputBuffer, static_cast<jint>(numberOfCharsInBuffer));
   env.CheckForJavaException();
}

void MSerialPort::Close()
{
   PortHandleType savedHandle = m_port;   // this is for multithreading -- prevent operations during close
   if ( savedHandle != NULL )
   {
       m_port = NULL;

       MJavaEnv env;
       env->CallVoidMethod(savedHandle, s_idDisconnect);
       env->DeleteGlobalRef(savedHandle); // free pointer, mark for garbage collector
       env.CheckForJavaException();
   }
}

unsigned MSerialPort::GetBytesReadyToRead() const
{
   M_ASSERT(m_port != NULL); // it is a debug level application error
   UpdatePortParametersOrTimeoutsIfChanged();

   MJavaEnv env;
   jint bytesReadyToRead = env->CallIntMethod(m_port, s_idGetBytesReadyToRead);
   env.CheckForJavaException();

   return static_cast<unsigned>(bytesReadyToRead);
}

bool MSerialPort::GetDCD() const
{
   M_ASSERT(m_port != NULL); // it is a debug level application error
   UpdatePortParametersOrTimeoutsIfChanged();

   MJavaEnv env;
   jboolean val = env->CallBooleanMethod(m_port, s_idGetDcd);
   env.CheckForJavaException();

   return val == JNI_FALSE ? false : true;
}

void MSerialPort::ConfigurePortParameters() const
{
   if ( m_port == NULL )
      return; // by design do nothing if not connected

   MJavaEnv env;
   jint baud = static_cast<jint>(m_baud);
   jint dataBits = static_cast<jint>(m_dataBits);
   jint parity;
   switch ( m_parity )
   {
   default:
       M_ASSERT(0); // cannot be due to property assignment checks
   case 'N': parity = PARITY_NONE;  break;
   case 'O': parity = PARITY_ODD;   break;
   case 'E': parity = PARITY_EVEN;  break;
   case 'M': parity = PARITY_MARK;  break;
   case 'S': parity = PARITY_SPACE; break;
   }
   jint stopBits = static_cast<jint>(m_stopBits); // STOP_BITS_ONE or STOP_BITS_TWO. STOP_BITS_ONE_AND_HALF is not supported by MeteringSDK
   jint dtrControl;
   switch ( m_dtrControl )
   {
   default:
       M_ASSERT(0); // cannot be due to property assignment checks
   case 'D': dtrControl = DTR_CONTROL_DISABLE;   break;
   case 'E': dtrControl = DTR_CONTROL_ENABLE;    break;
   case 'H': dtrControl = DTR_CONTROL_HANDSHAKE; break;
   }

   jint rtsControl;
   switch ( m_rtsControl )
   {
   default:
       M_ASSERT(0); // cannot be due to property assignment checks
   case 'D': rtsControl = RTS_CONTROL_DISABLE;   break;
   case 'E': rtsControl = RTS_CONTROL_ENABLE;    break;
   case 'H': rtsControl = RTS_CONTROL_HANDSHAKE; break;
   case 'T': rtsControl = RTS_CONTROL_TOGGLE;    break;
   }

   jboolean isCtcFlow      = m_isCtsFlow      ? JNI_TRUE : JNI_FALSE;
   jboolean isDsrFlow      = m_isDsrFlow      ? JNI_TRUE : JNI_FALSE;
   jboolean isDsrSensitive = m_dsrSensitivity ? JNI_TRUE : JNI_FALSE;

   env->CallVoidMethod(m_port, s_idConfigurePortParameters,
                       baud,
                       dataBits,
                       parity,
                       stopBits,
                       dtrControl,
                       rtsControl,
                       isCtcFlow,
                       isDsrFlow,
                       isDsrSensitive
                       );
   env.CheckForJavaException();

   m_portParametersChanged = false;
}

void MSerialPort::ConfigurePortTimeouts() const
{
   if ( m_port == NULL )
      return; // do nothing if not connected

   MJavaEnv env;
   jint readTimeout = static_cast<jint>(m_readTimeout);
   jint writeTimeout = static_cast<jint>(m_writeTimeout);
   env->CallVoidMethod(m_port, s_idConfigurePortTimeouts, readTimeout, writeTimeout);
   env.CheckForJavaException();

   m_portTimeoutsChanged = false;
}

MStdStringVector MSerialPort::GetAvailablePortNames(bool addExtraInfo)
{
   MStdStringVector result;

   MJavaEnv env;
   jclass c = DoCreatePortClass(env); // static method, create class
   jboolean extras = addExtraInfo ? JNI_TRUE : JNI_FALSE;
   jobjectArray names = reinterpret_cast<jobjectArray>(env->CallStaticObjectMethod(c, s_idGetAvailablePortNames, extras));
   env.CheckForJavaException(); // names will not be returned if there is an exception

   int size = env->GetArrayLength(names);
   result.reserve(size);
   for ( int i = 0; i < size; ++i )
   {
     jstring jStr = (jstring)env->GetObjectArrayElement(names, i);
     const char* cStr = env->GetStringUTFChars(jStr, 0);
     result.push_back(cStr);
     env->ReleaseStringUTFChars(jStr, cStr);
     env->DeleteLocalRef(jStr);
   }

   return result;
}

static void DoGetPortType(MStdString& result, const MStdString& portName)
{
   M_ASSERT(result.empty());

   MJavaEnv env;
   jclass c = DoCreatePortClass(env); // static method, create class
   jstring portNameJ = env.NewLocalStringUTF(portName);
   jstring resultJ = reinterpret_cast<jstring>(env->CallStaticObjectMethod(c, s_idGetPortType, portNameJ));
   env.CheckForJavaException(); // names will not be returned if there is an exception
   const char* cStr = env->GetStringUTFChars(resultJ, 0);
   result = cStr;
   env->ReleaseStringUTFChars(resultJ, cStr);
   env->DeleteLocalRef(resultJ);
}
