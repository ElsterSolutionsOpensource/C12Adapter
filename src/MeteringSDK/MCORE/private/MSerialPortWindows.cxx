// File MCORE/MSerialPortWindows.cxx
//
// This file is included, not part of compilation

#ifndef M__SERIAL_PORT_INTERNAL_IMPLEMENTATION
   #error "Do not compile .cxx files directly, they are included!"
#endif

#include "MRegistry.h"

   static const unsigned s_acceptableBauds[] =
      {
         // Note that Windows has a bigger list as Posix does not have 14400u, 28800u, 57600u, 128000u, and 256000u
         300u, 600u, 1200u, 2400u, 4800u, 9600u, 14400u, 19200u, 28800u, 38400u, 56000u, 57600u, 115200u, 128000u, 230400u, 256000u, 460800u, 500000u, 576000u, 921600u, 1000000u, 1152000u, 1500000u, 2000000u, 2500000u, 3000000u, 3500000u, 4000000u, 0
      };

   // Check if we are compatible with the Win32 constants
   // No need to check under POSIX since it's known to support all reasonable baud rates
   //
   M_COMPILED_ASSERT(CBR_300    == 300    && CBR_600    == 600   && CBR_1200  == 1200  && CBR_2400   == 2400);
   M_COMPILED_ASSERT(CBR_4800   == 4800   && CBR_9600   == 9600  && CBR_14400 == 14400 && CBR_19200  == 19200);
   M_COMPILED_ASSERT(CBR_38400  == 38400  && CBR_56000  == 56000 && CBR_57600 == 57600 && CBR_115200 == 115200);
   M_COMPILED_ASSERT(CBR_128000 == 128000 && CBR_256000 == 256000);

   #if (M_OS & M_OS_WIN32_CE) != 0

      // Helper recursive routine that searches the serial port
      // that corresponds to a IR port in CE.
      // Returns negative value if it did not find it in a tree.
      //
      static int DoFindIrNumber(LPTSTR subkey)
      {
         HKEY hKey;

         TCHAR path [ 128 ];
         lstrcpy(path, _T("\\Comm\\"));
         lstrcat(path, subkey);
         if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0, 0, &hKey) != ERROR_SUCCESS )
            return -1;

         int port;
         DWORD dwType;
         DWORD dwData;
         DWORD dwSize = sizeof(dwData);
         if ( RegQueryValueEx(hKey, _T("Port"), 0, &dwType, (PBYTE)&dwData, &dwSize) == ERROR_SUCCESS )
            port = (dwData >= 10) ? -1 : int(dwData);
         else // Unsuccessful query. Check if there is Parms subkey
         {
            lstrcpy(path, subkey);
            lstrcat(path, _T("\\Parms"));
            port = DoFindIrNumber(path);
            if ( port < 0 )  // Unsuccessful query. Then check to see if there is Linkage
            {
               HKEY hSubkey;
               if ( RegOpenKeyEx(hKey, _T("Linkage"), 0, 0, &hSubkey) == ERROR_SUCCESS )
               {
                   dwSize = sizeof(path);
                   if ( RegQueryValueEx(hSubkey, _T("Bind"), 0, &dwType, (PBYTE)&path, &dwSize) == ERROR_SUCCESS )
                      port = DoFindIrNumber(path);
                   RegCloseKey(hSubkey);
               }
            }
         }
         RegCloseKey(hKey);
         return port;
      }

   #endif

void MSerialPort::DoOpen()
{
   M_ASSERT(m_port == INVALID_HANDLE_VALUE);

   #if (M_OS & M_OS_WIN32_CE) != 0

      m_infraredPort = false;
      MWideString deviceName = MToWideString(m_portName);
      if ( !deviceName.empty() && deviceName[deviceName.size() - 1] != L':' ) // On Windows CE ports always end with ':'
         deviceName += L':';
      if ( deviceName == L"IR:" )
      {
         int irNum = DoFindIrNumber(L"IrDA");
         if ( irNum >= 0 ) // successful translation. Otherwise do not change deviceName and deviceName
         {
            deviceName = MToWideString(MToStdString("COM%d:", irNum));
            m_infraredPort = true;
         }
      }

      m_port = ::CreateFile(deviceName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); // constructed device name is a temporary, do not use it for m_portName
      if ( m_port == INVALID_HANDLE_VALUE )
      {
         DoThrowSystemError(true);
         M_ENSURED_ASSERT(0); // tell the compiler which supports __ensure() that we are never here
      }

      if ( m_infraredPort )
         ::EscapeCommFunction(m_port, SETIR);

   #else

      #if M_UNICODE
         m_port = ::CreateFile(MToWideString(m_portName).c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      #else
         m_port = ::CreateFile(m_portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      #endif
      if ( m_port == INVALID_HANDLE_VALUE )
      {
         if ( !m_portName.empty() && m_portName[0] != '\\' )  // now silently try a corresponding device name (without changing m_portName)
         {
            MStdString deviceName("\\\\.\\", 4);
            deviceName += m_portName;
            #if M_UNICODE
               m_port = ::CreateFile(MToWideString(deviceName).c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); // constructed device name is a temporary, do not use it for m_portName
            #else
               m_port = ::CreateFile(deviceName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); // constructed device name is a temporary, do not use it for m_portName
            #endif
         }

         if ( m_port == INVALID_HANDLE_VALUE ) // port value could be changed under the above IF
         {
            DoThrowSystemError(true);
            M_ENSURED_ASSERT(0); // tell the compiler which supports __ensure() that we are never here
         }
      }

   #endif
}

unsigned MSerialPort::Read(char* buffer, unsigned size)
{
   UpdatePortParametersOrTimeoutsIfChanged();

   M_ASSERT(size != 0);

   DWORD actualSize;
   BOOL result = ::ReadFile(m_port, buffer, size, &actualSize, NULL);
   if ( !result )
   {
      DoThrowSystemError(false);
      M_ENSURED_ASSERT(0);
   }
   return (unsigned)actualSize;
}

unsigned MSerialPort::Write(const char* buffer, unsigned size)
{
   UpdatePortParametersOrTimeoutsIfChanged();

   // M_ASSERT(size != 0); It is okay to write zero bytes
   DWORD actualLen;
   BOOL result = ::WriteFile(m_port, buffer, size, &actualLen, NULL);
   if ( !result )
   {
      DoThrowSystemError(false);
      M_ENSURED_ASSERT(0);
   }
   return (unsigned)actualLen;
}

void MSerialPort::ClearInputBuffer() const
{
   // No port configuring here
   if ( ! ::PurgeComm(m_port, PURGE_RXABORT | PURGE_RXCLEAR) )
   {
      DoThrowSystemError(false);
      M_ENSURED_ASSERT(0);
   }
}

void MSerialPort::FlushOutputBuffer(unsigned numberOfCharsInBuffer)
{
   // no port configuring here
   if ( ! ::FlushFileBuffers(m_port) )
      MESystemError::ClearGlobalSystemError(); // by convention, the above call ca fail (due to USB-based emulation)
   DoSleepToFlushBuffers(m_baud, numberOfCharsInBuffer);
}

void MSerialPort::Close()
{
   HANDLE savedHandle = m_port;   // this is for multithreading -- prevent operations during close
   if ( savedHandle != INVALID_HANDLE_VALUE )
   {
      m_port = INVALID_HANDLE_VALUE;
      if ( ! ::FlushFileBuffers(savedHandle) )
         MESystemError::ClearGlobalSystemError(); // by convention, the above call ca fail (due to USB-based emulation)
      DoSleepToFlushBuffers(m_baud);
      MUtilities::Sleep(200); // additional sleep, testing practice outcome

      #if (M_OS & M_OS_WIN32_CE) != 0
         if ( m_infraredPort )
            ::EscapeCommFunction(savedHandle, CLRIR);
      #endif

      ::CloseHandle(savedHandle); // ignore error
   }
}

unsigned MSerialPort::GetBytesReadyToRead() const
{
   UpdatePortParametersOrTimeoutsIfChanged();

   DWORD errors;
   COMSTAT stat;
   if ( ! ::ClearCommError(m_port, &errors, &stat) )
   {
       DoThrowSystemError(false);
       M_ENSURED_ASSERT(0);
   }
   return stat.cbInQue;
}

bool MSerialPort::GetDCD() const
{
   UpdatePortParametersOrTimeoutsIfChanged();

   DWORD status;
   if ( ! ::GetCommModemStatus(m_port, (LPDWORD)&status) )
   {
      DoThrowSystemError(false);
      M_ENSURED_ASSERT(0);
   }
   return (status & MS_RLSD_ON) != 0;
}

void MSerialPort::ConfigurePortParameters() const
{
   if ( m_port == INVALID_HANDLE_VALUE )
      return; // do nothing if not connected

   DCB dcb;
   DCB dcbOld;
   memset(&dcb, 0, sizeof(DCB)); // as in the Microsoft example...
   ::GetCommState(m_port, &dcb);

   dcbOld = dcb; // store the previous DCB structure
   dcb.fBinary = 1;
   dcb.BaudRate = m_baud;
   dcb.fOutxCtsFlow = m_isCtsFlow ? TRUE : FALSE;
   dcb.fOutxDsrFlow = m_isDsrFlow ? TRUE : FALSE;
   dcb.fDsrSensitivity = m_dsrSensitivity ? TRUE : FALSE;

   switch ( (char)m_dtrControl )
   {
   case 'E': dcb.fDtrControl = DTR_CONTROL_ENABLE; break;
   case 'D': dcb.fDtrControl = DTR_CONTROL_DISABLE; break;
   case 'H': dcb.fDtrControl = DTR_CONTROL_HANDSHAKE; break;
   default: M_ENSURED_ASSERT(0); // impossible if the source code is right
   }
   switch ( (char)m_rtsControl )
   {
   case 'E': dcb.fRtsControl = RTS_CONTROL_ENABLE; break;
   case 'D': dcb.fRtsControl = RTS_CONTROL_DISABLE; break;
   case 'H': dcb.fRtsControl = RTS_CONTROL_HANDSHAKE; break;
   case 'T': dcb.fRtsControl = RTS_CONTROL_TOGGLE; break;
   default: M_ENSURED_ASSERT(0); // impossible if the source code is right
   }

   dcb.ByteSize = (BYTE)m_dataBits;
   switch ( (char)m_parity )
   {
   case 'N': dcb.Parity = NOPARITY; break;
   case 'O': dcb.Parity = ODDPARITY; break;
   case 'E': dcb.Parity = EVENPARITY; break;
   case 'M': dcb.Parity = MARKPARITY; break;
   case 'S': dcb.Parity = SPACEPARITY; break;
   default: M_ENSURED_ASSERT(0); // impossible if the source code is right
   }
   switch ( m_stopBits )
   {
   case 1: dcb.StopBits = ONESTOPBIT; break;
   case 2: dcb.StopBits = TWOSTOPBITS; break;
   default: M_ENSURED_ASSERT(0); // impossible if the source code is right
   }

   dcb.fTXContinueOnXoff = FALSE;
   dcb.fOutX = FALSE;
   dcb.fInX = FALSE;
   dcb.fErrorChar = FALSE;
   dcb.fNull = FALSE;
   dcb.fAbortOnError = FALSE;

   // update only in case the settings are different
   //
   if ( memcmp(&dcbOld, &dcb, sizeof(DCB)) != 0 )
   {
      if ( ! ::SetCommState(m_port, &dcb) )
      {
         DoThrowSystemError(false);
         M_ENSURED_ASSERT(0);
      }
      ClearInputBuffer();
      MUtilities::Sleep(35); // required in order for the port to adjust
   }

   m_portParametersChanged = false;
}

void MSerialPort::ConfigurePortTimeouts() const
{
   if ( m_port == INVALID_HANDLE_VALUE )
      return; // do nothing if not connected

   COMMTIMEOUTS cto;
   cto.ReadIntervalTimeout = m_intercharacterTimeout;
   cto.WriteTotalTimeoutConstant = m_writeTimeout;
   cto.ReadTotalTimeoutConstant = m_readTimeout;

   // Multiplier is the empiric number based on experience: 8000 is milliseconds
   // per byte, plus one millisecond for safety, as the value is rounded down.
   // Also we should get in attention that real transfer rate is smaller than
   // supplied bauds, because of stop bits and other stuff. To achieve more
   // reliable communications we use the most frequently used combination
   // 8 data bits and 1 stop bit.
   M_ASSERT(m_baud != 0);
   DWORD multiplier = 8000u * (10u / 8u) / m_baud + 1u;
   cto.ReadTotalTimeoutMultiplier = multiplier;
   cto.WriteTotalTimeoutMultiplier = multiplier;
   if ( ! ::SetCommTimeouts(m_port, &cto) )
   {
      DoThrowSystemError(false);
      M_ENSURED_ASSERT(0);
   }

   m_portTimeoutsChanged = false;
}

#if !M_NO_REGISTRY

   typedef std::map<MStdString, MStdString>
      StringStringMap;

   static const char s_hardwareDevicemapSerialcomm [] = "HARDWARE\\DEVICEMAP\\SERIALCOMM";
   static const char s_systemCurrentControlSet     [] = "SYSTEM\\CurrentControlSet\\Enum";

   // GUID values of interfaces are taken from here:
   //   https://msdn.microsoft.com/en-us/library/windows/hardware/ff553426%28v=vs.85%29.aspx
   static const char s_guidClassComport   [] = "{4d36e978-e325-11ce-bfc1-08002be10318}"; 
   static const char s_guidClassModem     [] = "{4d36e96d-e325-11ce-bfc1-08002be10318}";
   static const char s_guidClassMultiPort [] = "{50906cb8-ba12-11d1-bf5d-0000f805f530}";

   static void DoRecurseRegistryForSerialDevices(const MStdString& path, StringStringMap& portInfo)
   {
      MRegistry reg(MRegistry::KeyLocalMachine, path, true);
      const MStdString& classGuid = reg.GetString("ClassGUID", MVariant::s_emptyString);
      if ( classGuid.empty() ) // not a device, recurse in
      {
         MStdStringVector values = reg.GetAllSubkeys();
         MStdStringVector::const_iterator it = values.begin();
         MStdStringVector::const_iterator itEnd = values.end();
         for ( ; it != itEnd; ++it )
         {
            MStdString newPath = path;
            newPath += '\\';
            newPath += *it;
            DoRecurseRegistryForSerialDevices(newPath, portInfo);
         }
      }
      else if ( (stricmp(classGuid.c_str(), s_guidClassComport) == 0 || // serial port
                 stricmp(classGuid.c_str(), s_guidClassModem) == 0   || // or modem
                 stricmp(classGuid.c_str(), s_guidClassMultiPort) == 0) // or multi-serial port
                 && reg.IsSubkeyPresent("Device Parameters") ) // and there are parameters
      {
         MStdString friendlyName = reg.GetString("FriendlyName", MVariant::s_emptyString);
         if ( !friendlyName.empty() ) // information is available
         {
            reg.Close();
            MStdString newPath = path;
            newPath.append("\\Device Parameters");
            reg.Open(MRegistry::KeyLocalMachine, newPath, true);
            const MStdString& portName = reg.GetString("PortName", MVariant::s_emptyString);
            if ( !portName.empty() ) // all necessary information is available
            {
               StringStringMap::iterator it = portInfo.find(portName);
               if ( it != portInfo.end() )
               {
                  M_ASSERT(portName.size() == it->first.size() && portName == it->first); // was suspicious on map's string equality routine

                  MStdString::size_type lastBrace = friendlyName.rfind(" (");
                  if ( lastBrace != MStdString::npos )
                     friendlyName.resize(lastBrace);
                  it->second = friendlyName;
               }
            }
         }
      }
   }

#endif

MStdStringVector MSerialPort::GetAvailablePortNames(bool addExtraInfo)
{
   MStdStringVector result;

#if !M_NO_REGISTRY

   MRegistry reg(MRegistry::KeyLocalMachine, s_hardwareDevicemapSerialcomm, true);
   MStdStringVector values = reg.GetAllValues();
   MStdStringVector::const_iterator it = values.begin();
   MStdStringVector::const_iterator itEnd = values.end();
   if ( !addExtraInfo )
   {
      for ( ; it != itEnd; ++it )
      {
         const MStdString& portName = reg.GetExistingString(*it);
         result.push_back(portName);
      }
   }
   else
   {
      typedef std::map<MStdString, MStdString>
         StringStringMap;
      StringStringMap portInfo;

      for ( ; it != itEnd; ++it )
      {
         const MStdString& val = *it;
         const MStdString& portName = reg.GetExistingString(val);
         portInfo[portName] = val;
      }

      DoRecurseRegistryForSerialDevices(s_systemCurrentControlSet, portInfo);

      StringStringMap::const_iterator mi = portInfo.begin();
      StringStringMap::const_iterator miEnd = portInfo.end();
      for ( ; mi != miEnd; ++mi )
      {
         MStdString port = mi->first;
         port.append(" {", 2);
         port += mi->second;
         port += '}';
         result.push_back(port);
      }
   }

   MAlgorithm::InplaceSort(result, true, true);
#endif

   return result;
}

static void DoGetPortType(MStdString& result, const MStdString& portName)
{
   M_ASSERT(result.empty());
#if !M_NO_REGISTRY

   MRegistry reg(MRegistry::KeyLocalMachine, s_hardwareDevicemapSerialcomm, true);
   MStdStringVector values = reg.GetAllValues();
   MStdStringVector::const_iterator it = values.begin();
   MStdStringVector::const_iterator itEnd = values.end();
   for ( ; it != itEnd; ++it )
   {
      const MStdString& deviceName = *it;
      const MStdString& port = reg.GetExistingString(deviceName);
      if ( port == portName )
      {
         const char* s = deviceName.c_str();
         if ( strnicmp(s, "\\Device\\Serial", 14) == 0 ||              // Standard
              strnicmp(s, "SISerial", 8) == 0 )                        
            result.assign("serial", 6);
         else if ( strnicmp(s, "\\Device\\VCP", 11) == 0 ||            
                   strnicmp(s, "\\Device\\ProlificSerial", 22) == 0 )  
            result.assign("usb", 3);
         else if ( strnicmp(s, "Npdrv", 5) == 0 ||                     
                   strnicmp(s, "\\Device\\Nptdrv", 14) == 0 )          
            result.assign("remote", 6);
         else if ( strnicmp(s, "\\Device\\Bth", 11) == 0 )             
            result.assign("bluetooth", 9);
         else
            result = deviceName;
         return; // found
      }
   }
#endif
}
