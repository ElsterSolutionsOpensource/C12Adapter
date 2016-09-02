#ifndef MCORE_MSERIALPORT_H
#define MCORE_MSERIALPORT_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MSerialPort.h

#include <MCORE/MCOREDefs.h>

#if !M_NO_SERIAL_PORT

/// Serial port device.
///
/// The services below can throw MException or MESystemError in the event of
/// an erroneous serial port operation.
///
class M_CLASS MSerialPort
{
public: // Types:

   ///@{
   /// Operating system dependent serial port handle type
   ///
   #if (M_OS & M_OS_ANDROID) != 0
      #if !defined(M_NO_JNI) || M_NO_JNI != 0
         #error "jni.h is not included, define configuration option M_NO_JNI=0"
      #endif
      typedef jobject PortHandleType;
   #elif (M_OS & M_OS_POSIX) != 0
      typedef int PortHandleType;
   #else
      typedef HANDLE PortHandleType;
   #endif
   ///@}

public: // Constructor, destructor:

   /// Constructor that creates an uninitialized serial port.
   /// The serial is not open until Open is called.
   ///
   MSerialPort();

   /// Destructor.
   /// No exceptions are thrown by this call.
   ///
   ~MSerialPort();

public: // Services:

   /// Open a serial port.
   ///
   /// The valid name rules depend on the operating system.
   /// All available ports can be gotten using \ref GetAvailablePortNames(bool addExtraInfo = false) call.
   /// When addExtraInfo is true, the port names returned have more human readable information
   /// that can help select the port name. The port name is operating system dependent:
   ///   - On Windows this can be "COM1", "COM12", and so on.
   ///     The call also accepts and ignores any information in curly braces such as "COM11 {USB Optical Probe}".
   ///   - On UNIX-like operating systems such as Linux, QNX (but not Android) this is a device name such as "/dev/ttyS0".
   ///     Information in curly braces such as "/dev/ttyS0 {USB Optical Probe}" is ignored.
   ///   - On Android the port name rules are more diverse as the standard Android does not expose serial port device files.
   ///     Bluetooth serial port uses the name of the paired device, and FTDI USB probe accepts the device description.
   ///     Different from Windows or *NIX, Android does not ignore the information within the curly braces
   ///     and it is used to distinguish between many devices with the same name if these are hooked up.
   /// For all operating systems, it is recommended to give the result of GetAvailablePortNames(true) call as the port name.
   ///
   /// \pre IsOpen() should be false as the port should not be open already prior to this call,
   /// and there is a debug version check for this condition.
   /// Also, any operating system related exception can take place due to many reasons why the port cannot be open.
   ///
   /// \param portName Port name to open.
   ///
   void Open(const MStdString& portName);

   /// Close the port.
   ///
   /// No exceptions are thrown by this function.
   /// If the port was not open, this method does nothing.
   ///
   void Close();

   /// Return a collection of serial port names available at this computer.
   ///
   /// Names returned can be given to \ref Open. The returned names are operating system dependent.
   ///   - On Windows the names are "COM1" or "COM12".
   ///     When extra information is added they are like "COM11 {USB Optical Probe}".
   ///   - On UNIX-like operating systems such as Linux, QNX (but not Android), it returns device names such as "/dev/ttyS0".
   ///     Extra information will be "/dev/ttyS0 {USB Optical Probe}".
   ///   - On Android the port name rules are more diverse as the standard Android does not expose serial port device files.
   ///     Bluetooth serial port uses the name of the paired device, and FTDI USB probe accepts the device description.
   ///     Extra information will include MAC address for Bluetooth or device index for FTDI USB device.
   ///
   /// \param addExtraInfo whether to add any extra information about the port, useful for users to select the port.
   /// \return array of present port names
   ///
   static MStdStringVector GetAvailablePortNames(bool addExtraInfo = false);

   /// Get type based on the port name.
   ///
   /// Provides information about underlying port technology.
   /// When such information is not available, for example, the given port name is not present,
   /// an empty string is returned. Otherwise the string returned can be one of the following types:
   ///     - "serial" wide range of different UART based serial drivers
   ///     - "bluetooth" for bluetooth devices
   ///     - "usb" USB serial devices
   ///     - "remote" remote ports such as MOXA NPort
   ///     - "" empty string can be returned only on Android when MeteringSDK
   ///       could not determine the type of the port
   ///     - Any other name will mean the type of the port is determined,
   ///       but not recognized and classified. The name is still useful
   ///       as it identifies the port type
   ///
   /// All known port types will always appear in lowercase.
   /// Unknown port types will preserve the letter case defined by the operating system.
   /// The port will not be opened by the call.
   ///
   /// This is a static method as it allows getting information without opening the port.
   /// A string is returned rather than an enumeration as it allows returning
   /// useful information even when the port type is not recognized and classified.
   ///
   /// \param portName Name of the port such as "COM11" on Windows or "/dev/ttyS0" on Linux
   /// \return Port type string with contents described above
   ///
   static MStdString GetPortType(const MStdString& portName);

   /// Receive a number of bytes available in the serial port.
   /// return the actual number of bytes read.
   ///
   /// \pre The connection is alive, otherwise the system
   /// exception is raised. The given length is bigger than zero,
   /// otherwise a debug version hits an assert.
   ///
   unsigned Read(char* buffer, unsigned size);

   /// Write the buffer into the port.
   ///
   /// \pre The port is opened successfully, 
   /// otherwise the system-specific exception is thrown.
   ///
   unsigned Write(const char* buffer, unsigned size);

   /// Discard the contents of the input buffer of the port.
   /// All characters in the receive buffer that are waiting to be read are lost.
   ///
   /// \pre The port is open, otherwise the
   /// operation fails with the exception.
   ///
   void ClearInputBuffer() const;

   /// Ensure that the characters from the output buffer are sent.
   /// The parameter, if specified, should match the number of characters
   /// written into the serial port right before FlushOutputBuffer is called.
   /// If the parameter is missing, the biggest possible number of characters will 
   /// be ensured to go away.
   ///
   /// \pre The port is open, otherwise the
   /// operation fails with the exception.
   ///
   void FlushOutputBuffer(unsigned numberOfCharsInBuffer = UINT_MAX);

   /// Returns the number of bytes in the serial port input buffer,
   /// which are not read yet.
   ///
   unsigned GetBytesReadyToRead() const;

public: // Property accessors:

   /// Whether the port is open.
   ///
   bool IsOpen() const
   {
      #if (M_OS & M_OS_ANDROID) != 0
         return m_port != NULL;
      #elif M_OS & M_OS_POSIX
         return m_port != -1;
      #else
         return m_port != INVALID_HANDLE_VALUE;
      #endif
   }

   ///@{
   /// Port name, whatever was used during Open.
   ///
   /// Setting port name explicitly outside Open can be used to keep the port name
   /// associated with the object that will open it later.
   ///
   const MStdString& GetPortName() const
   {
      return m_portName;
   }
   void SetPortName(const MStdString& port)
   {
      m_portName = port;
   }
   ///@}

#if (M_OS & M_OS_WIN32_CE) != 0

   ///@{
   /// Infrared port flag, true if this is an infrared port.
   ///
   /// This property has to be set before calling Open on the port.
   /// Only supported by some operating systems, such as Windows CE.
   ///
   bool IsInfraredPort() const
   {
      return m_infraredPort;
   }
   void SetIsInfraredPort(bool is)
   {
      m_infraredPort = is;
   }
   ///@}

#endif

   /// Get the current state of the DCD signal of the port.
   ///
   /// \pre The port has to be open, otherwise a system error
   /// takes place.
   ///
   bool GetDCD() const;

   ///@{
   /// Baud rate of the port.
   ///
   /// If the port is connected, the baud rate switch will happen
   /// immediately within the function. If the port is not connected,
   /// the baud rate will be used to configure the port when Open is called.
   ///
   /// \pre The baud rate has to be a valid value defined by the OS. That is 300, 600, 1200, 2400, 4800, and so on.
   /// When assigning a bad baud, an exception takes place.
   ///
   unsigned GetBaud() const
   { 
      return m_baud;
   }
   void SetBaud(unsigned);
   ///@}

   ///@{
   /// Port parity.
   ///
   /// If the port is connected, the parity change happens
   /// immediately at the assignment. If the port is not connected,
   /// the parity setting is used to configure the port when Open is called.
   ///
   /// \pre Parity has to be any of the following characters:
   /// 'N', 'O', 'E', 'M', or 'S'. Otherwise an exception takes place.
   ///
   char GetParity() const
   { 
      return m_parity;
   }
   void SetParity(char);
   ///@}

   ///@{
   /// Number of stop bits.
   ///
   /// If the port is connected, the number of stop bits changes
   /// immediately at the assignment. If the port is not connected,
   /// the property is used to configure the port when Open is called.
   ///
   /// \pre A valid number of stop bits is to be given, 1 or 2.
   /// Otherwise an exception is thrown. Notice that one and a half stop bits is not supported.
   ///
   int GetStopBits() const
   { 
      return m_stopBits;
   }
   void SetStopBits(int);
   ///@}

   ///@{
   /// Number of data bits.
   ///
   /// If the port is connected, the number of data bits changes
   /// immediately at the assignment. If the port is not connected,
   /// the property is used to configure the port when Open is called.
   ///
   /// \pre A valid number of data bits is to be given, 5, 6, 7 or 8.
   /// Otherwise an exception is thrown.
   ///
   int GetDataBits() const 
   { 
      return m_dataBits;
   }
   void SetDataBits(int);
   ///@}

   ///@{
   /// CTS flow used for port handling.
   ///
   /// If the port is connected, CTS flow handling changes
   /// immediately at the assignment. If the port is not connected,
   /// the property is used to configure the port when Open is called.
   ///
   bool GetCtsFlow() const
   {
      return m_isCtsFlow;
   }
   void SetCtsFlow(bool);
   ///@}

   ///@{
   /// DSR flow used for port handling.
   ///
   /// If the port is connected, CTS flow handling changes
   /// immediately at the assignment. If the port is not connected,
   /// the property is used to configure the port when Open is called.
   ///
   bool GetDsrFlow() const
   {
      return m_isDsrFlow;
   }
   void SetDsrFlow(bool);
   ///@}

   ///@{
   /// Sensitivity of the communication port to DSR signal.
   ///
   /// If the port is connected, DSR sensitivity handling changes
   /// immediately at the assignment. If the port is not connected,
   /// the property is used to configure the port when Open is called.
   ///
   bool GetDsrSensitivity() const
   {
      return m_dsrSensitivity;
   }
   void SetDsrSensitivity(bool);
   ///@}

   ///@{
   /// Data Terminal Ready (DTR) control mode of the communication port.
   ///
   /// 'E' means Enable, 'D' means Disable, and 'H' means Handshake.
   ///
   /// If the port is connected, DTR control changes
   /// immediately at the assignment. If the port is not connected,
   /// the property is used to configure port when Open is called.
   ///
   /// \pre The allowed values for the character are 'E', 'D' or 'H',
   /// otherwise an exception will be thrown.
   ///
   char GetDtrControl() const
   {
      return m_dtrControl;
   }
   void SetDtrControl(char);
   ///@}

   ///@{
   /// Request To Send (RTS) control mode of the communication port. 
   ///
   /// 'E' means Enable, 'D' means Disable, 'H' means Handshake, and 'T' means Toggle.
   ///
   /// If the port is connected, RTS control changes
   /// immediately at the assignment. If the port is not connected,
   /// the property is used to configure the port when Open is called.
   ///
   /// \pre The allowed values for the character are 'E', 'D' 'H', or 'T',
   /// otherwise an exception will be thrown.
   ///
   char GetRtsControl() const
   {
      return m_rtsControl;
   }
   void SetRtsControl(char rtsControl);
   ///@}

   ///@{
   /// Read intercharacter timeout in milliseconds.
   ///
   /// Maximum amount of milliseconds between any two characters before timeout
   /// exception takes place.
   ///
   unsigned GetIntercharacterTimeout() const
   {
      return m_intercharacterTimeout;
   }
   void SetIntercharacterTimeout(unsigned timeout);
   ///@}

   ///@{
   /// Read timeout in milliseconds.
   ///
   /// Maximum amount of milliseconds for completion of the whole read operation.
   ///
   unsigned GetReadTimeout() const
   {
      return m_readTimeout;
   }
   void SetReadTimeout(unsigned timeout);
   ///@}

   ///@{
   /// Write timeout in milliseconds.
   ///
   /// Timeout is the time for which the write operation should complete.
   ///
   unsigned GetWriteTimeout() const
   {
      return m_writeTimeout;
   }
   void SetWriteTimeout(unsigned timeout);
   ///@}

   ///@{
   /// Access operating system handle of the port.
   ///
   /// On Windows this is file `HANDLE` type, while
   /// on Posix operating systems this is an integer file number.
   ///
   PortHandleType GetHandle() const
   {
      return m_port;
   }
   ///@}

public: // Methods:

   /// Convenience method for setting related port parameters in a single call.
   ///
   /// \param baud Valid baud rate
   /// \param dataBits Valid data bits value
   /// \param parity Valid parity value
   /// \param stopBits Valid stop bits value
   ///
   void SetParameters(unsigned baud, int dataBits, char parity, int stopBits);

   /// Convenience method for setting port timeouts in a single call.
   ///
   /// \param intercharacterTimeout Intercharacter timeout in milliseconds
   /// \param readTimeout Read timeout in milliseconds
   /// \param writeTimeout Write timeout in milliseconds
   ///
   void SetTimeouts(unsigned intercharacterTimeout, unsigned readTimeout, unsigned writeTimeout);

   /// Throw an error that says the baud is invalid, giving the baud value as parameter.
   ///
   /// \pre The exception is thrown unconditionally
   ///
   /// \param baud Reported bad baud
   ///
   static M_NORETURN_FUNC void ThrowInvalidBaudRate(unsigned baud);

   /// Check if the given baud rate is valid.
   ///
   /// \param baud Baud value to be checked. If the baud rate is not valid, an exception is thrown.
   ///
   static void CheckIsBaudValid(unsigned baud);

   /// Check if the given parity is valid.
   ///
   /// \param parity Parity value to be checked. If the parity is not valid, an exception is thrown.
   ///
   static void CheckIsParityValid(char parity);

   /// Get a zero-terminated list of acceptable baud rates.
   ///
   static const unsigned* GetAcceptableBaudsArray();

   /// Get a zero-terminated list of acceptable parity values.
   ///
   static const char* GetAcceptableParitiesArray();

   /// Check if the given number of stop bits is a valid value.
   ///
   /// \param stopBits Stop bits to test. If the number of stop bits is not 1 or 2, an exception is thrown.
   ///
   static void CheckIsStopBitsValid(int stopBits);

   /// Check if the given number of data bits is a valid value.
   ///
   /// \param dataBits Data bits to check. If the number of data bits is not 5, 6, 7 or 8, an exception is thrown.
   ///
   static void CheckIsDataBitsValid(int dataBits);

   /// Check if the given character represents a valid value for DTR control.
   ///
   /// \param dtrControl DTR control parameter to check. It shall either be 'E', 'D' or 'H', or an exception is thrown.
   ///
   static void CheckIsDtrControlValid(char dtrControl);

   /// Check if the given character represents a valid value for RTS control.
   ///
   /// \param rtsControl RTS control parameter to check. It shall either be 'E', 'D', 'H', or 'T', or an exception is thrown.
   ///
   static void CheckIsRtsControlValid(char rtsControl);

   /// Force configuration of port parameters to properties of this class.
   ///
   /// No action is performed in case communication is not yet established.
   ///
   /// \pre There are few checks that relate
   /// to the communication port state. OS-related exceptions
   /// can be thrown in case the port parameters cannot be set.
   ///
   /// \see ConfigurePortTimeouts for timeout handling.
   ///
   void ConfigurePortParameters() const;

   /// Force configuration of port timeouts to properties of this class.
   ///
   /// No action is performed in case communication is not yet established.
   ///
   /// \pre There are few checks that relate
   /// to the communication port state. OS-related exceptions
   /// can be thrown in case the port timeouts cannot be set.
   ///
   /// \see ConfigurePortParameters for parameter handling.
   ///
   void ConfigurePortTimeouts() const;

   /// If properties for parameters or timeouts are changed, change port.
   ///
   /// No action is performed in case communication is not yet established.
   ///
   /// \pre There are few checks that relate
   /// to the communication port state. OS-related exceptions
   /// can be thrown in case the port parameters or timeouts cannot be set.
   ///
   /// \see ConfigurePortParameters for parameter handling.
   /// \see ConfigurePortTimeouts for timeout handling.
   ///
   void UpdatePortParametersOrTimeoutsIfChanged() const;

private: // Services:

   // Helper operating system dependent method
   //
   void DoOpen();

   // Throw a system error related to serial port operation.
   // The message will differ depending on whether it was thrown during opening port.
   //
   M_NORETURN_FUNC void DoThrowSystemError(bool whileOpening) const;

   // Sleep a precise number of milliseconds until the given number of characters leave the UART
   //
   static unsigned DoSleepToFlushBuffers(unsigned baud, unsigned numberOfCharsInBuffer = UINT_MAX);

private: // Constructors:

   // Private copy constructor, cannot copy ports semantically 
   // and syntactically.
   //
   // \pre Should not be called, as ensured by the compiler.
   //
   MSerialPort(const MSerialPort&);

   // Private assignment operator, cannot assign ports semantically 
   // and syntactically.
   //
   // \pre Should not be called, as ensured by the compiler.
   //
   void operator=(const MSerialPort&);

private: // Attributes:

   // Communication port handle
   //
   PortHandleType m_port;

   // Port baud rate, BAUD public property
   //
   unsigned m_baud;

   // Parity of the serial port, such as N for no parity or E for even parity
   //
   char m_parity;

   // Number of stop bits, 1 or 2.
   //
   int m_stopBits;

   // Number of data bits, 5, 6, 7 or 8
   //
   int m_dataBits;

   // Whether CTS signal is monitored for output flow control.
   //
   bool m_isCtsFlow;

   // Whether DSR signal is monitored for output flow control.
   //
   bool m_isDsrFlow;

   // If this member is 'true', serial port ignores any bytes received, 
   // unless the DSR modem input line is high. 
   //
   bool m_dsrSensitivity;

   // Specifies the DTR (data-terminal-ready) flow control. Possible values 
   // are: 'D' for disable, 'E' for enable, 'H' for handshake.
   //
   char m_dtrControl;

   // Specifies the RTS (request-to-send) flow control. Possible values are:
   // 'D' for disable, 'E' for enable, 'H' for handshake, 'T' for toggle.
   // 
   char m_rtsControl;

   // Time, in milliseconds, allowed to elapse between the arrival of two 
   // characters on the line.
   //
   unsigned m_intercharacterTimeout;

   // The constant, in milliseconds, used to calculate the total timeout 
   // period for read operations.
   //
   unsigned m_readTimeout;

   // The constant, in milliseconds, used to calculate the total timeout 
   // period for write operations.
   //
   unsigned m_writeTimeout;

   // Whether there were any changes in port parameters.
   //
   mutable bool m_portParametersChanged;

   // Whether there were any changes in port timeouts.
   //
   mutable bool m_portTimeoutsChanged;

#if (M_OS & M_OS_WIN32_CE) != 0

   // True will mean this is an infrared port. Special action is required to open an infrared port.
   // This property has to be set before calling Open on the port.
   //
   bool m_infraredPort;

#endif

   // Port name, used for error handling, not available through public accessors.
   //
   MStdString m_portName;
};

#endif

///@}
#endif // !M_NO_SERIAL_PORT
