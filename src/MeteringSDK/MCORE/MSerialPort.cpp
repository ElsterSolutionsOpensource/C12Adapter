// File MCORE/MSerialPort.cpp

#include "MCOREExtern.h"
#include "MException.h"
#include "MUtilities.h"
#include "MAlgorithm.h"

#if !M_NO_SERIAL_PORT

#define M__SERIAL_PORT_INTERNAL_IMPLEMENTATION
#include "MSerialPort.h"

// Operating system specific method implementations follow
#if (M_OS & M_OS_ANDROID) != 0
   #include "private/MSerialPortAndroid.cxx"
#elif (M_OS & M_OS_WINDOWS) != 0
   #include "private/MSerialPortWindows.cxx"
#else
   #include "private/MSerialPortPosix.cxx"
#endif

   const char s_acceptableParities[] =
      {
         'N', 'O', 'E', 'M', 'S', '\0'
      };

MSerialPort::MSerialPort()
:
#if (M_OS & M_OS_ANDROID) != 0
   m_port(NULL),
   m_baud(19200u),
#elif (M_OS & M_OS_POSIX) != 0
   m_port(-1),
   m_baud(19200u),
#else
   m_port(INVALID_HANDLE_VALUE),
   m_baud(28800u),
#endif
   m_parity('N'),
   m_stopBits(1),
   m_dataBits(8),
   m_isCtsFlow(true),
   m_isDsrFlow(true),
   m_dsrSensitivity(true),
   m_dtrControl('H'),
   m_rtsControl('H'),
   m_intercharacterTimeout(500u),
   m_readTimeout(1000u),
   m_writeTimeout(2000u),
   m_portParametersChanged(true),
   m_portTimeoutsChanged(true),

   #if (M_OS & M_OS_WIN32_CE) != 0
      m_infraredPort(false),
   #endif

   m_portName()
{
}

MSerialPort::~MSerialPort()
{
   Close();

   #if (M_OS & M_OS_ANDROID) != 0
      M_ASSERT(m_port == NULL);
   #endif
}

void MSerialPort::Open(const MStdString& portName)
{
   M_ASSERT(!IsOpen());

#if (M_OS & M_OS_ANDROID) != 0 // In case of Android fully interpret the port name

   m_portName = portName;

#else // otherwise abbreviate it to the name only

   if ( !portName.empty() && *portName.rbegin() == '}' ) // extra info is possibly present
   {
      MStdString::size_type openingBracePos = portName.find(" {", 2);
      if ( openingBracePos != MStdString::npos && openingBracePos > 0 )
         m_portName.assign(portName.begin(), portName.begin() + openingBracePos);
      else
         m_portName = portName;
   }
   else
      m_portName = portName;

#endif

   DoOpen();

   m_portParametersChanged = true; // force parameter configuration
   m_portTimeoutsChanged = true;   // force parameter configuration

}

unsigned MSerialPort::DoSleepToFlushBuffers(unsigned baud, unsigned numberOfCharsInBuffer)
{
   // These empirically determined delays resulted from a serial port baud shift test on NT and w9x,
   // which yielded the following table for the flush buffer delays (case of large buffer to flush):
   //
   //   Baud    MinimumNT Delay(test)  NT Delay    W9x Delay
   //   300     540                    648         66
   //   600     270                    324         33
   //   1200    135                    162         16
   //   2400     61                     81          8
   //   4800     31                     41          4
   //   9600     11                     20          2
   //  14400      1                     14          1
   //   ....    ...
   //
   // The constant 10 is added for safety, because the timer resolution is about 10ms.
   //
   unsigned delayForMaximumChars = (162000u + 32400u) / baud + 10;
   unsigned delayForSpecifiedChars = ((1000u * 8u) * numberOfCharsInBuffer) / baud + 60u; // ((milliseconds * numberOfBitsInByte) * ...
   if ( delayForSpecifiedChars > delayForMaximumChars )
      delayForSpecifiedChars = delayForMaximumChars; // don't use min(a,b), it has compiler-specific flaws

   MUtilities::Sleep(delayForSpecifiedChars);

   return delayForSpecifiedChars;
}

M_NORETURN_FUNC void MSerialPort::ThrowInvalidBaudRate(unsigned baud)
{
   MException::Throw(MException::ErrorConfiguration, M_CODE_STR_P1(MErrorEnum::InvalidBaud, M_I("Invalid or unsupported baud rate %u"), baud));
   M_ENSURED_ASSERT(0);
}

void MSerialPort::CheckIsBaudValid(unsigned baud)
{
   for ( const unsigned* curr = s_acceptableBauds; *curr != 0u; ++curr )
      if ( *curr == baud )
         return; // valid
   ThrowInvalidBaudRate(baud);
   M_ENSURED_ASSERT(0);
}

void MSerialPort::SetBaud(unsigned baud)
{
   if ( m_baud != baud )
   {
      CheckIsBaudValid(baud);
      m_baud = baud;
      m_portParametersChanged = true;
   }
}

void MSerialPort::CheckIsParityValid(char parity)
{
   for ( int i = M_NUMBER_OF_ARRAY_ELEMENTS(s_acceptableParities) - 1; i >= 0; --i )
      if ( s_acceptableParities[i] == parity )
         return; // valid
   MException::Throw(MException::ErrorConfiguration, M_CODE_STR_P1(M_ERR_INVALID_OR_UNSUPPORTED_PARITY_U1, M_I("Invalid or unsupported parity %u"), (unsigned)(unsigned char)parity));
   M_ENSURED_ASSERT(0);
}

void MSerialPort::SetParity(char parity)
{
   if ( m_parity != parity )
   {
      CheckIsParityValid(parity);
      m_parity = parity;
      m_portParametersChanged = true;
   }
}

void MSerialPort::CheckIsStopBitsValid(int stopBits)
{
   if ( stopBits != 1 && stopBits != 2 )
   {
      MException::Throw(MException::ErrorConfiguration, M_CODE_STR_P1(M_ERR_INVALID_OR_UNSUPPORTED_NUMBER_OF_STOP_BITS_U1, M_I("Invalid or unsupported number of stop bits %u"), (unsigned)stopBits));
      M_ENSURED_ASSERT(0);
   }
}

void MSerialPort::SetStopBits(int stopBits)
{
   if ( m_stopBits != stopBits )
   {
      CheckIsStopBitsValid(stopBits);
      m_stopBits = stopBits;
      m_portParametersChanged = true;
   }
}

void MSerialPort::CheckIsDataBitsValid(int dataBits)
{
   if ( dataBits < 5 || dataBits > 8 )
   {
      MException::Throw(MException::ErrorConfiguration, M_CODE_STR_P1(M_ERR_INVALID_OR_UNSUPPORTED_NUMBER_OF_DATA_BITS_U1, M_I("Invalid or unsupported number of data bits %u"), (unsigned)dataBits));
      M_ENSURED_ASSERT(0);
   }
}

void MSerialPort::SetDataBits(int dataBits)
{
   if ( m_dataBits != dataBits )
   {
      CheckIsDataBitsValid(dataBits);
      m_dataBits = dataBits;
      m_portParametersChanged = true;
   }
}

void MSerialPort::SetCtsFlow(bool ctsFlow)
{
   if ( m_isCtsFlow != ctsFlow )
   {
      m_isCtsFlow = ctsFlow;
      m_portParametersChanged = true;
   }
}

void MSerialPort::SetDsrFlow(bool dsrFlow)
{
   if ( m_isDsrFlow != dsrFlow )
   {
      m_isDsrFlow = dsrFlow;
      m_portParametersChanged = true;
   }
}

void MSerialPort::SetDsrSensitivity(bool dsrSens)
{
   if ( m_dsrSensitivity != dsrSens )
   {
      m_dsrSensitivity = dsrSens;
      m_portParametersChanged = true;
   }
}

void MSerialPort::CheckIsDtrControlValid(char dtrControl)
{
   if ( dtrControl != 'E' && dtrControl != 'D' && dtrControl != 'H' )
   {
      MException::Throw(MException::ErrorConfiguration, M_CODE_STR_P1(M_ERR_DTR_CONTROL_WITH_CODE_X1_IS_NOT_KNOWN, M_I("DTR control character with code 0x%X is not known, expected E, D, or H"), dtrControl));
      M_ENSURED_ASSERT(0);
   }
}

void MSerialPort::SetDtrControl(char dtrControl)
{
   if ( m_dtrControl != dtrControl )
   {
      CheckIsDtrControlValid(dtrControl);
      m_dtrControl = dtrControl;
      m_portParametersChanged = true;
   }
}

void MSerialPort::CheckIsRtsControlValid(char rtsControl)
{
   if ( rtsControl != 'E' && rtsControl != 'D' && rtsControl != 'H' && rtsControl != 'T' )
   {
      MException::Throw(MException::ErrorConfiguration, M_CODE_STR_P1(M_ERR_RTS_CONTROL_WITH_CODE_X1_IS_NOT_KNOWN, M_I("RTS control character with code 0x%X is not known, expected E, D, H, or T"), rtsControl));
      M_ENSURED_ASSERT(0);
   }
}

void MSerialPort::SetRtsControl(char rtsControl)
{
   if ( m_rtsControl != rtsControl )
   {
      CheckIsRtsControlValid(rtsControl);
      m_rtsControl = rtsControl;
      m_portParametersChanged = true;
   }
}

void MSerialPort::SetIntercharacterTimeout(unsigned timeout)
{
   if ( m_intercharacterTimeout != timeout )
   {
      m_intercharacterTimeout = timeout;
      m_portTimeoutsChanged = true;
   }
}

void MSerialPort::SetReadTimeout(unsigned timeout)
{
   if ( m_readTimeout != timeout )
   {
      m_readTimeout = timeout;
      m_portTimeoutsChanged = true;
   }
}

void MSerialPort::SetWriteTimeout(unsigned timeout)
{
   if ( m_writeTimeout != timeout )
   {
      m_writeTimeout = timeout;
      m_portTimeoutsChanged = true;
   }
}

void MSerialPort::SetParameters(unsigned baud, int dataBits, char parity, int stopBits)
{
   SetBaud(baud);
   SetDataBits(dataBits);
   SetParity(parity);
   SetStopBits(stopBits);
}

void MSerialPort::SetTimeouts(unsigned intercharacterTimeout, unsigned readTimeout, unsigned writeTimeout)
{
   SetIntercharacterTimeout(intercharacterTimeout);
   SetReadTimeout(readTimeout);
   SetWriteTimeout(writeTimeout);
}

void MSerialPort::UpdatePortParametersOrTimeoutsIfChanged() const
{
   if ( IsOpen() )
   {
      if ( m_portParametersChanged )
      {
         ConfigurePortParameters();
         M_ASSERT(!m_portParametersChanged);
      }
      if ( m_portTimeoutsChanged )
      {
         ConfigurePortTimeouts();
         M_ASSERT(!m_portTimeoutsChanged);
      }
   }
}

M_NORETURN_FUNC void MSerialPort::DoThrowSystemError(bool whileOpening) const
{
#if !M_NO_VERBOSE_ERROR_INFORMATION
   MConstLocalChars str;
   if ( !whileOpening )
   {
      if ( IsOpen() )
         str = M_I("Error during operation with serial port '%s'");
      else
         str = M_I("Serial port '%s' was not previously connected");
   }
   else
      str = M_I("Could not open serial port '%s'");
   MESystemError::ThrowLastSystemError(MGetStdString(str, m_portName.c_str()));
#else
   MESystemError::ThrowLastSystemError();
#endif
   M_ENSURED_ASSERT(0);
}

const unsigned* MSerialPort::GetAcceptableBaudsArray()
{
   return s_acceptableBauds;
}

const char* MSerialPort::GetAcceptableParitiesArray()
{
   return s_acceptableParities;
}

MStdString MSerialPort::GetPortType(const MStdString& portName)
{
   MStdString result;
   MStdString realPortName = portName;
   MStdString::size_type pos = realPortName.find(" {"); // remove extra information if it is there
   if ( pos != MStdString::npos )
      realPortName.resize(pos);
   DoGetPortType(result, realPortName);
   return result;
}

#endif // !M_NO_SERIAL_PORT
