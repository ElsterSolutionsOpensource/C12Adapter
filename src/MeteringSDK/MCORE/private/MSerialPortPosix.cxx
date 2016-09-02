// File MCORE/MSerialPortPosix.cxx
//
// This file is included, not part of compilation

#ifndef M__SERIAL_PORT_INTERNAL_IMPLEMENTATION
   #error "Do not compile .cxx files directly, they are included!"
#endif

#if !M_NO_LUA_COOPERATIVE_IO
   #include "LuaIO.h"
#endif
#include "MFindFile.h"
#include "MUtilities.h"
#include "MStreamFile.h"

   static const unsigned s_acceptableBauds[] =
      {
         // Note that Windows has a bigger list as Posix does not have 14400u, 28800u, 57600u, 128000u, and 256000u
         300u, 600u, 1200u, 2400u, 4800u, 9600u, 19200u, 38400u, 57600u, 115200u, 230400u, 460800u, 500000u, 576000u, 921600u, 1000000u, 1152000u, 1500000u, 2000000u, 2500000u, 3000000u, 3500000u, 4000000u, 0
      };

   #if (M_OS & M_OS_QNXNTO)
      #define B230400  230400L
      #define B460800  460800L
      #define B500000  500000L
      #define B576000  576000L
      #define B921600  921600L
      #define B1000000 1000000L
      #define B1152000 1152000L
      #define B1500000 1500000L
      #define B2000000 2000000L
      #define B2500000 2500000L
      #define B3000000 3000000L
      #define B3500000 3500000L
      #define B4000000 4000000L
   #elif (M_OS & M_OS_NUTTX)
      #include <sys/select.h>
      #include <termios.h>
      #define B3500000 3500000L
      #define B4000000 4000000L
   #endif

   static const unsigned s_POSIXBaudRates[] =
      {
         B300, B600, B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000, 0
      };

   inline unsigned DoGetPOSIXBaudRate(unsigned baud)
   {
      for ( unsigned i = 0; s_acceptableBauds[i] != 0u; ++i )
         if ( s_acceptableBauds[i] == baud )
            return s_POSIXBaudRates[i]; // found
      MSerialPort::ThrowInvalidBaudRate(baud);
      M_ENSURED_ASSERT(0);
   }

void MSerialPort::DoOpen()
{
   M_ASSERT(m_port == -1);
   int flags = O_RDWR | O_NOCTTY | O_NONBLOCK /*O_NDELAY*/; // open port in non-blocking mode
   m_port = open(m_portName.c_str(), flags);

   if ( m_port == -1 )
   {
      DoThrowSystemError(true);
      M_ENSURED_ASSERT(0); // tell the compiler which supports __ensure() that we are never here
   }
   if ( fcntl(m_port, F_SETFL, flags&(~O_NONBLOCK)) == -1 ) // restore normal (blocking) behavior after port is open
   {
      DoThrowSystemError(true);
      M_ENSURED_ASSERT(0); // tell the compiler which supports __ensure() that we are never here
   }
}

unsigned MSerialPort::Read(char* buffer, unsigned size)
{
   UpdatePortParametersOrTimeoutsIfChanged();

   M_ASSERT(size != 0);

   ssize_t actualSize = 0;

#if !M_NO_LUA_COOPERATIVE_IO
   int status = MLuaYieldAndSelect(m_port, m_readTimeout, 0 /*read*/);
#else
   fd_set readfs;
   FD_ZERO(&readfs);
   FD_SET(m_port, &readfs);

   timeval tv;
   tv.tv_sec = static_cast<time_t>(m_readTimeout / 1000u);
   tv.tv_usec = static_cast<long>((m_readTimeout % 1000u) * 1000u);
   const int status = select(m_port + 1, &readfs, 0, 0, &tv);
#endif
   if ( status == -1 )
   {
      DoThrowSystemError(false);
      M_ENSURED_ASSERT(0);
   }
   else if ( status )
   {
#if M_OS & M_OS_NUTTX
     int flags = fcntl(m_port, F_GETFL, 0);
     fcntl(m_port, F_SETFL, flags | O_NONBLOCK); 
#endif
      actualSize = read(m_port, buffer, size);
   }

   if ( actualSize == -1 )
   {
      DoThrowSystemError(false);
      M_ENSURED_ASSERT(0);
   }
   return (unsigned)actualSize;
}

unsigned MSerialPort::Write(const char* buffer, unsigned size)
{
   UpdatePortParametersOrTimeoutsIfChanged();

   ssize_t actualLen = 0;

#if !M_NO_LUA_COOPERATIVE_IO
   int status = MLuaYieldAndSelect(m_port, m_writeTimeout, 1 /*write*/);
#else
   fd_set writefs;
   FD_ZERO(&writefs);
   FD_SET(m_port, &writefs);

   timeval tv;
   tv.tv_sec = static_cast<time_t>(m_writeTimeout / 1000u);
   tv.tv_usec = static_cast<long>((m_writeTimeout % 1000u) * 1000u);
   const int status = select(m_port + 1, 0, &writefs, 0, &tv);
#endif
   if ( status == -1 )
   {
      DoThrowSystemError(false);
      M_ENSURED_ASSERT(0);
   }

#if M_OS & M_OS_NUTTX
   int flags = fcntl(m_port, F_GETFL, 0);
   fcntl(m_port, F_SETFL, flags | O_NONBLOCK); 
#endif

   // ignore timeout error
   actualLen = write(m_port, buffer, size);

   if ( actualLen == -1 )
   {
      DoThrowSystemError(false);
      M_ENSURED_ASSERT(0);
   }
   return (unsigned)actualLen;
}

void MSerialPort::ClearInputBuffer() const
{
   // No port configuring here

#if (M_OS & M_OS_NUTTX) != 0
   // not yet implemented
#else
   if ( tcflush(m_port, TCIFLUSH) != 0 )  // TCIFLUSH - Flush data received but not read.
   {
      DoThrowSystemError(false);
      M_ENSURED_ASSERT(0);
   }
#endif
}

void MSerialPort::FlushOutputBuffer(unsigned numberOfCharsInBuffer)
{
   // no port configuring here

   #if (M_OS & (M_OS_NUTTX | M_OS_ANDROID)) != 0 // POSIX Android or Nuttix
      if ( ioctl(m_port, TCSBRK, 1) != 0 )
         MESystemError::ClearGlobalSystemError();
      DoSleepToFlushBuffers(m_baud, numberOfCharsInBuffer);
   #else
      unsigned timespent = 0;
      int oldOutbytes = 0;
      for ( ;; )
      {
         int outbytes = 0;
         int ret = ioctl(m_port, TIOCOUTQ, &outbytes);
         if ( ret )
         {
            MESystemError::ClearGlobalSystemError();
            break;
         }
         if ( outbytes == 0 )
         {
            if ( tcdrain(m_port) != 0 ) // blocks until the hardware output buffer is empty, no need to sleep
               MESystemError::ClearGlobalSystemError(); // by convention, the above call fails (due to USB-based emulation)
            break;
         }
         if ( oldOutbytes != outbytes )
         {
            timespent = 0;
            oldOutbytes = outbytes;
         }
         if ( timespent > m_writeTimeout )
            break;
         timespent += DoSleepToFlushBuffers(m_baud, outbytes);
      }
   #endif
}

void MSerialPort::Close()
{
   int savedHandle = m_port;   // this is for multithreading -- prevent operations during close
   if ( savedHandle != -1 )
   {
      m_port = -1;
      #if (M_OS & (M_OS_NUTTX | M_OS_ANDROID)) != 0 // POSIX Android or Nuttix
         if ( ioctl(m_port, TCSBRK, 1) != 0 )
            MESystemError::ClearGlobalSystemError();
      #else
         if ( tcdrain(m_port) != 0 ) // tcdrain() blocks until the output buffer is empty, no need to sleep
            MESystemError::ClearGlobalSystemError(); // by convention, the above call fails (due to USB-based emulation)
      #endif

      close(savedHandle); // ignore error
   }
}

unsigned MSerialPort::GetBytesReadyToRead() const
{
   UpdatePortParametersOrTimeoutsIfChanged();

   int bytes;
   if ( ioctl(m_port, FIONREAD, (unsigned long)&bytes) != 0 )
   {
       DoThrowSystemError(false);
       M_ENSURED_ASSERT(0);
   }
   return (unsigned)bytes;
}

bool MSerialPort::GetDCD() const
{
   UpdatePortParametersOrTimeoutsIfChanged();

   #ifdef TIOCMGET
      int status;
      if ( ioctl(m_port, TIOCMGET, (unsigned long)&status) != 0)
      {
         DoThrowSystemError(false);
         M_ENSURED_ASSERT(0);
      }
      return (status & TIOCM_CAR) != 0;
   #else
      return false;
   #endif // TIOCMGET
}

   // Helper function to perform simple serial port bits manipulation using ioctl()
   //
   static int DoChangeSerialPortSettings(int fd, unsigned bitsToAdd, unsigned bitsToOr)
   {
   #ifdef TIOCMGET
      unsigned int mstat, okay;
      okay = ioctl(fd, TIOCMGET, (unsigned long)&mstat);
      if ( okay )
         return okay;
      if ( bitsToAdd )
         mstat &= bitsToAdd;
      mstat |= bitsToOr;
      return ioctl(fd, TIOCMSET, (unsigned long)&mstat);
   #else
      return 0;
   #endif
   }

void MSerialPort::ConfigurePortParameters() const
{
#if (M_OS & M_OS_NUTTX) != 0
   //????? TODO
#else
   if ( m_port == -1 )
      return; // do nothing if not connected

   struct termios options, optionsOld;
   tcgetattr(m_port, &options);
   optionsOld = options; // store the previous options

   // VMIN = 0 and VTIME > 0
   // Because MIN is 0, TIME is a read () operation timer that is activated as soon as read() is called.
   // The read() operation returns as soon as a byte is received or when the timer expires.
   // The actual timeout value is set via DoConfigurePortTimeouts() function
   options.c_cc[VMIN] = 0;
   options.c_cc[VTIME] = 1;

   // CLOCAL and CREAD should always be enabled. These will ensure that
   // program does not become the 'owner' of the port subject to sporadic job control and hangup signals,
   // and also that the serial interface driver will read incoming data bytes.
   //
   options.c_cflag |= (CLOCAL | CREAD /* | HUPCL */);

   // HUPCL - enable hangup line (drop DTR) on last close
   options.c_cflag &= ~(HUPCL);

   // enable raw (binary) mode
//   cfmakeraw(&options);
   options.c_lflag &= ~(ICANON | ECHO | ECHONL | ECHOE | ISIG | IEXTEN);
   options.c_lflag |= NOFLSH;

   options.c_oflag &= ~OPOST; // no output processing
   options.c_oflag &= ~ONLCR; // don't convert line feeds

   // disable input processing: no parity checking or marking, no
   // signals from break conditions, do not convert line feeds or
   // carriage returns, and do not map upper case to lower case.
   options.c_iflag &= ~(PARMRK | BRKINT | INLCR | ICRNL | IUCLC | IXANY | ISTRIP | IGNCR);
#if !(M_OS & M_OS_NUTTX)
   options.c_iflag &= ~IMAXBEL;
#endif

   // ignore break conditions
   options.c_iflag |= IGNBRK;

   // Most POSIX systems do not support different input and output speeds,
   // so be sure to set both to the same value for maximum portability.
   unsigned baudPOSIX = DoGetPOSIXBaudRate(m_baud);
   cfsetispeed(&options, baudPOSIX);
   cfsetospeed(&options, baudPOSIX);

   int bytesize = m_dataBits, stopbits = m_stopBits;

#ifdef CMSPAR  // mark or space (stick) parity
   options.c_cflag &= ~(PARENB | PARODD | CMSPAR);
#else
   options.c_cflag &= ~(PARENB | PARODD);
#endif

   options.c_iflag |= INPCK;
   switch ( m_parity )
   {
   case 'N': // NOPARITY
      options.c_iflag &= ~INPCK;
      break;
   case 'O': // ODDPARITY
      options.c_cflag |= (PARENB | PARODD);
      break;
   case 'E': // EVENPARITY
      options.c_cflag |= PARENB;
      break;
#ifdef CMSPAR
   // Linux defines mark/space (stick) parity
   case 'M': // MARKPARITY
      options.c_cflag |= (PARENB | CMSPAR);
      break;
   case 'S': // SPACEPARITY
      options.c_cflag |= (PARENB | PARODD | CMSPAR);
      break;
#else
    // try the POSIX way
   case 'M': // MARKPARITY
      if ( stopbits == 1 ) // ONESTOPBIT
      {
          stopbits = 2; // TWOSTOPBITS
          options.c_iflag &= ~INPCK;
      }
      else
      {
         // do we need a new message: parity + stop bit?
         DoThrowSystemError(false);
         M_ENSURED_ASSERT(0);
      }
      break;
   case 'S': // SPACEPARITY
      if ( bytesize < 8 )
      {
          bytesize += 1;
          options.c_iflag &= ~INPCK;
      }
      else
      {
         // do we need a new message: parity + data bit?
         DoThrowSystemError(false);
         M_ENSURED_ASSERT(0);
      }
      break;
#endif
   default: M_ENSURED_ASSERT(0); // impossible if the source code is right
   }

   options.c_cflag &= ~CSIZE;
   switch ( bytesize )
   {
      case 5:
         options.c_cflag |= CS5;
         break;
      case 6:
         options.c_cflag |= CS6;
         break;
      case 7:
         options.c_cflag |= CS7;
         break;
      case 8:
         options.c_cflag |= CS8;
         break;
      default: M_ENSURED_ASSERT(0); // impossible if the source code is right
   }

   switch ( stopbits )
   {
      case 1: // ONESTOPBIT
         options.c_cflag &= ~CSTOPB;
         break;
      case 2: // TWOSTOPBITS
         options.c_cflag |= CSTOPB;
         break;
      default: M_ENSURED_ASSERT(0); // impossible if the source code is right
   }

   if ( m_isCtsFlow || m_rtsControl == 'H' ) // RTS_CONTROL_HANDSHAKE
   {
#if (M_OS & M_OS_QNXNTO)
       options.c_cflag |= (IHFLOW | OHFLOW);
#else
       options.c_cflag |= CRTSCTS;
#endif
   }
   else
   {
#if (M_OS & M_OS_QNXNTO)
       options.c_cflag &= ~(IHFLOW | OHFLOW);
#else
       options.c_cflag &= ~CRTSCTS;
#endif
   }

   // these two are always set to false in MSerialPort
   // dcb.fOutX = FALSE;
   // dcb.fInX = FALSE;

   options.c_iflag &= ~IXON;
   options.c_iflag &= ~IXOFF;

   // update only in case the settings are different, or the update is forced
   //
   if ( memcmp(&optionsOld, &options, sizeof(struct termios)) != 0 )
   {
      // Set the new options for the port
      // The TCSANOW  constant specifies that all changes should occur immediately
      // without waiting for output data to finish sending or input data to finish receiving.
      // The TCSADRAIN constant specifies that all changes should occur after all output data
      // has been written to the port
      //
      if ( tcsetattr(m_port, TCSADRAIN, &options) < 0 )
//      if ( tcsetattr(m_port, TCSANOW, &options) < 0 )
      {
         DoThrowSystemError(false);
         M_ENSURED_ASSERT(0);
      }

      // note: change DTR/RTS lines after setting the comm attributes,
      // so flow control does not interfere.
#ifdef TIOCM_DTR
      switch ( m_dtrControl )
      {
      case 'E': // DTR_CONTROL_ENABLE
         DoChangeSerialPortSettings(m_port, 0, TIOCM_DTR);
         break;
      case 'D': // DTR_CONTROL_DISABLE
         DoChangeSerialPortSettings(m_port, ~TIOCM_DTR, 0);
         break;
      case 'H': // DTR_CONTROL_HANDSHAKE
         // DSR/DTR flow control not supported
         break;
      default: M_ENSURED_ASSERT(0); // impossible if the source code is right
      }
#endif

#ifdef TIOCM_RTS
      if( !m_isCtsFlow )
      {
         switch ( m_rtsControl )
         {
         case 'E': // RTS_CONTROL_ENABLE
            DoChangeSerialPortSettings(m_port, 0, TIOCM_RTS);
            break;
         case 'D': // RTS_CONTROL_DISABLE
            DoChangeSerialPortSettings(m_port, ~TIOCM_RTS, 0);
            break;
         case 'H': // RTS_CONTROL_HANDSHAKE
            break;
         case 'T': // RTS_CONTROL_TOGGLE
            break;
         default: M_ENSURED_ASSERT(0); // impossible if the source code is right
         }
      }
#endif
   }
#endif

   m_portParametersChanged = false;
}

void MSerialPort::ConfigurePortTimeouts() const
{
#if (M_OS & M_OS_NUTTX) != 0
    // ???
#else

   if ( m_port == -1 )
      return; // do nothing if not connected

   struct termios options;
   tcgetattr(m_port, &options);

   // VTIME is in 1/10 seconds
   {
      unsigned int ux_timeout;
      if( m_intercharacterTimeout == 0 ) // 0 means no timeout
      {
         ux_timeout = 0;
      }
      else
      {
         ux_timeout = (m_intercharacterTimeout + 99) / 100;
         if( ux_timeout == 0 )
         {
            ux_timeout = 1; // must be at least some timeout
         }
      }

      // The same logic to calculate multiplier as below for Win32 version
      M_ASSERT(m_baud != 0);
      unsigned multiplier = 8000u * (10u / 8u) / m_baud + 1u;

      ux_timeout *= multiplier;
      options.c_cc[VMIN] = 1;    // this implements intercharacter timeout behaviour
      options.c_cc[VTIME] = ux_timeout;
   }

//   if ( tcsetattr(m_port, TCSADRAIN, &options) < 0 )
   if ( tcsetattr(m_port, TCSANOW, &options) < 0 )
   {
      DoThrowSystemError(false);
      M_ENSURED_ASSERT(0);
   }

#endif

   m_portTimeoutsChanged = false;
}

MStdStringVector MSerialPort::GetAvailablePortNames(bool addExtraInfo)
{
   MStdStringVector result;

   const char* fullFileName;
   MFindFile ff("/sys/class/tty", "*", true); // search for directories
   while ( (fullFileName = ff.FindNext()) != NULL )
   {
      MStdString driverPath = fullFileName;
      driverPath += "/device/driver";
      if ( MUtilities::IsPathExisting(driverPath) ) // if the driver symbolic link is there we are dealing with a present serial port
      {
         MStdString name("/dev/", 5);
         name += MUtilities::GetPathFileNameAndExtension(fullFileName);
         if ( addExtraInfo )
         {
            char buf [ M_MAX_PATH ];
            ssize_t size = readlink(driverPath.c_str(), buf, sizeof(buf));
            if ( size > 0 )
            {
               M_ASSERT(size < static_cast<ssize_t>(sizeof(buf))); // as guaranteed by the above system call

               buf[size] = '\0';
               name.append(" {", 2);
               // Since the name has to be human readable remove some extra info (relative path)
               const char* shortUserInfo = strstr(buf, "/bus/");
               if ( shortUserInfo != NULL )
                   name += shortUserInfo + 5;
               else
                   name += buf;
               name += '}';
            }
         }
         result.push_back(name);
      }
      else
      {
         driverPath = fullFileName;
         driverPath.append("/address", 8); // bluetooth device
         if ( MUtilities::IsPathExisting(driverPath) ) // if the driver symbolic link is there we are dealing with a present serial port
         {
            MStdString name("/dev/", 5);
            name += MUtilities::GetPathFileNameAndExtension(fullFileName);
            if ( addExtraInfo )
            {
               name.append(" {bluetooth ", 12);
               name += MAlgorithm::TrimString(MStreamFile::StaticReadAll(driverPath));
               name += '}';
            }
            result.push_back(name);
         }
      }
   }

   return result;
}

static void DoGetPortType(MStdString& result, const MStdString& portName)
{
   M_ASSERT(result.empty());
   M_COMPILED_ASSERT(M_MAX_PATH > 64); // the algorithm uses this fact in the below code

   const char* name = portName.c_str();
   size_t nameSize = portName.size();
   if ( portName.size() > 5 && memcmp(name, "/dev/", 5) == 0 )
   {
      name += 5;
      nameSize -= 5;
   }
   if ( nameSize < M_MAX_PATH - 64 ) // sanitize name size to avoid buffer overflow
   {
      char driverPath [ M_MAX_PATH ];
      memcpy(driverPath, "/sys/class/tty/", 15);
      memcpy(driverPath + 15, name, nameSize);
      memcpy(driverPath + nameSize + 15, "/device/driver", 15); // 15, not 14, to copy trailing \0

      char buf [ M_MAX_PATH ];
      ssize_t size = readlink(driverPath, buf, sizeof(buf));
      if ( size > 0 )
      {
         M_ASSERT(size < static_cast<ssize_t>(sizeof(buf))); // as guaranteed by the above system call
         buf[size] = '\0';
         name = basename(buf);
         M_ASSERT(name != NULL && name >= buf && name < buf + sizeof(buf)); // basename is always within buf
         if ( strncmp(name, "serial", 6) == 0 )
            result.assign("serial", 6);
         else if ( strncmp(name, "ftdi", 4) == 0 )
            result.assign("usb", 3);
         else
            result.assign(name);
      }
      else // attempt to find a bluetooth device
      {
         memcpy(driverPath + nameSize + 15, "/address", 9); // 9, not 8, to copy trailing \0

         struct stat st;
         int ret = stat(driverPath, &st);
         MESystemError::ClearGlobalSystemError();
         if ( ret == 0 && S_ISREG(st.st_mode) )
            result.assign("bluetooth", 9);
      }
   }
}
