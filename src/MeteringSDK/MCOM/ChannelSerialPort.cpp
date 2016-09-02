// File MCOM/ChannelSerialPort.cpp

#include "MCOMExtern.h"
#include "ChannelSerialPort.h"
#include "MCOMExceptions.h"

#if !M_NO_SERIAL_PORT

using namespace std;

M_START_PROPERTIES(ChannelSerialPort)
#if M_OS & M_OS_POSIX
#if defined(__CYGWIN__)
   M_OBJECT_PROPERTY_PERSISTENT_STRING(ChannelSerialPort, PortName,       "COM1", ST_constMStdStringA_X, ST_X_constMStdStringA)
#else
   M_OBJECT_PROPERTY_PERSISTENT_STRING(ChannelSerialPort, PortName,       "/dev/ttyS0", ST_constMStdStringA_X, ST_X_constMStdStringA) // DOXYGEN_HIDE due to the above definition
#endif
   M_OBJECT_PROPERTY_PERSISTENT_UINT  (ChannelSerialPort, Baud,           9600u)
#else
   M_OBJECT_PROPERTY_PERSISTENT_STRING(ChannelSerialPort, PortName,       "COM1", ST_constMStdStringA_X, ST_X_constMStdStringA) // DOXYGEN_HIDE due to the above definition
   M_OBJECT_PROPERTY_PERSISTENT_UINT  (ChannelSerialPort, Baud,           28800u)                                               // DOXYGEN_HIDE due to the above definition
#endif
   M_OBJECT_PROPERTY_PERSISTENT_CHAR  (ChannelSerialPort, Parity,         'N')
   M_OBJECT_PROPERTY_PERSISTENT_INT   (ChannelSerialPort, DataBits,       8)
   M_OBJECT_PROPERTY_PERSISTENT_INT   (ChannelSerialPort, StopBits,       1)
   M_OBJECT_PROPERTY_PERSISTENT_CHAR  (ChannelSerialPort, DtrControl,     'D')
   M_OBJECT_PROPERTY_PERSISTENT_CHAR  (ChannelSerialPort, RtsControl,     'H')
   M_OBJECT_PROPERTY_PERSISTENT_BOOL  (ChannelSerialPort, CtsFlow,        false)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL  (ChannelSerialPort, DsrFlow,        false)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL  (ChannelSerialPort, DsrSensitivity, false)
M_START_METHODS(ChannelSerialPort)
   M_CLASS_SERVICE                    (ChannelSerialPort, GetAvailablePortNames, ST_MStdStringVector_S_bool)
   M_CLASS_SERVICE                    (ChannelSerialPort, GetPortType,           ST_MStdString_S_constMStdStringA)
M_END_CLASS_TYPED(ChannelSerialPort, Channel, "CHANNEL_DIRECT_SERIAL")

MChannelSerialPort::MChannelSerialPort()
{
   M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(ChannelSerialPort);
}

MChannelSerialPort::~MChannelSerialPort()
{
   Disconnect();
}

void MChannelSerialPort::SetPortName(const MStdString& portName)
{
   m_portName = portName;
   m_port.SetPortName(portName); // this one is to improve error handling
}

void MChannelSerialPort::DoClearInputBuffer()
{
   const_cast<MChannelSerialPort*>(this)->m_port.ClearInputBuffer();
}

void MChannelSerialPort::FlushOutputBuffer(unsigned numberOfCharsInBuffer)
{
   m_port.FlushOutputBuffer(numberOfCharsInBuffer);
}

void MChannelSerialPort::Connect()
{
   MChannel::Connect();
   DoConnect();
   DoNotifyConnect();
}

void MChannelSerialPort::DoConnect()
{
   M_ASSERT(!IsConnected()); // otherwise an error should have been reported already
   if ( m_port.IsOpen() ) // IsConnected is not the same as m_port.IsOpen (this way we handle Modem differences)
      m_port.Close();
   m_port.Open(m_portName);

   if ( m_isAutoAnswer )
   {
      try
      {
         WaitForNextIncomingConnection(true); // this is done for every implementation of this class
      }
      catch ( MException& )
      {
         Disconnect(); // will never throw an exception
         throw;
      }
   }
}

void MChannelSerialPort::WaitForNextIncomingConnection(bool)
{
   if ( !m_isAutoAnswer )
   {
      MChannel::WaitForNextIncomingConnection(false); // this throws exception "channel is not in answer mode"
      M_ENSURED_ASSERT(0);
   }

   char firstByte;
   unsigned readSize = DoReadCancellable(&firstByte, 1, MTimer::SecondsToMilliseconds(m_autoAnswerTimeout), false);
   if ( readSize == 0 )
      MCOMException::Throw(M_CODE_STR(M_ERR_TIMED_OUT_WHILE_WAITING_FOR_CONNECTION, M_I("Timed out while waiting for connection")));
   UnreadBuffer(&firstByte, sizeof(firstByte));
}

void MChannelSerialPort::Disconnect()
{
   if ( m_port.IsOpen() )
   {
      m_port.Close();
      DoNotifyDisconnect(); // notify only if was connected
   }
}

unsigned MChannelSerialPort::DoWrite(const char* buf, unsigned len)
{
   if ( m_port.GetWriteTimeout() != m_writeTimeout )
      m_port.SetWriteTimeout(m_writeTimeout);
   return m_port.Write(buf, len);
}

unsigned MChannelSerialPort::DoRead(char* buf, unsigned size, unsigned timeout)
{
   m_port.SetTimeouts((m_intercharacterTimeout == 0) ? timeout : m_intercharacterTimeout, timeout, m_writeTimeout);
   return m_port.Read(buf, size);
}

bool MChannelSerialPort::IsConnected() const
{
   return m_port.IsOpen();
}

MStdString MChannelSerialPort::GetMediaIdentification() const
{
   MStdString result = MUtilities::GetLocalHostName();
   result += ":SERIAL:";
   result += m_portName;
   return result;
}

#endif // !M_NO_SERIAL_PORT
