#ifndef MCOM_CHANNELSERIALPORT_H
#define MCOM_CHANNELSERIALPORT_H
/// \addtogroup MCOM
///@{
/// \file MCOM/ChannelSerialPort.h

#include <MCOM/MCOMDefs.h>
#include <MCOM/Channel.h>

#if !M_NO_SERIAL_PORT

/// Serial port based channel, a null cable direct serial link or a link through a current loop adapter.
///
/// It forms the basic interface to the rest of the COM port
/// based channels and MChannel Type's MChannelOpticalProbe and MChannelModem inherit their
/// properties from MChannelSerialPort. Many properties of MChannelSerialPort are controlled
/// by the communication protocols or by the child channels; however, an advanced user can
/// override the values set by the protocols and children channels at any time to achieve
/// advanced behavior for test purposes. 
///
class MCOM_CLASS MChannelSerialPort : public MChannel
{
public: // Constructor, destructor:

   /// Construct serial port channel.
   ///
   MChannelSerialPort();

   /// Destructor.
   ///
   virtual ~MChannelSerialPort();

public: // Services:

   /// Ensure that the characters from the output buffer are sent.
   ///
   /// \param numberOfCharsInBuffer
   ///     If specified, should match the number of characters
   ///     written into the serial port right before FlushOutputBuffer is called.
   ///     If the parameter is missing, the biggest possible number of characters will
   ///     be flushed.
   ///
   /// \pre The channel is open, otherwise the operation fails with the exception.
   ///
   virtual void FlushOutputBuffer(unsigned numberOfCharsInBuffer = UINT_MAX);

   /// Establishes the connection to the meter using the serial port.
   ///
   /// \pre IsConnected should be false before calling this method.
   /// Many OS and program exceptions can be thrown by this method.
   ///
   virtual void Connect();

   /// Disconnect brings down the connection and releases the serial port resource.
   ///
   virtual void Disconnect();

   /// Returns the current connection state of the serial port channel.
   ///
   virtual bool IsConnected() const;

   /// When \refprop{GetAutoAnswer,AutoAnswer} true, wait for the incoming 
   /// connection without disconnecting the channel.
   ///
   /// A typical server application sequence that uses this call:
   /// \code
   ///     Connect() // wait for the first incoming connection
   ///     loop until interrupted:
   ///         ... communicate ...
   ///         WaitForNextIncomingConnection() // wait for the next incoming connection
   ///     end loop
   ///     Disconnect()
   /// \endcode
   ///
   /// \param reinitialize Tells if reinitialization of the channel has to be made at each new incoming connection.
   ///
   /// \pre Prior to this call, the channel needs to be configured with \refprop{SetAutoAnswer,AutoAnswer} true, 
   ///      and the connection established with Connect(). Not all channels support Auto Answer mode, 
   ///      and might throw an exception. A timeout exception is thrown if no call is received 
   ///      during the \refprop{GetAutoAnswerTimeout,AutoAnswerTimeout} period.
   ///
   virtual void WaitForNextIncomingConnection(bool reinitialize = true);

   /// SetParameters is a convenience function for setting the properties
   /// baud rate, number of data bits, parity, and number of stop bits in a single call.
   ///
   void SetParameters(unsigned baud, int dataBits, char parity, int stopBits)
   {
      m_port.SetParameters(baud, dataBits, parity, stopBits);
   }

public: // Properties:

   ///@{
   /// Port name is the file name which is used by the operating system to open the port, OS dependent.
   ///
   /// Name of the communication port. The port name is determined by the operating system.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value "COM1" on Windows, "/dev/ttyS0" on all other operating systems
   ///
   /// \possible_values
   ///   There is no syntactical restriction on what can be the name of the port,
   ///   and here are few examples given for different operating systems:
   ///     - "/dev/ttyS0" : Typical device name for a serial port on UNIX-like systems such as Linux or QNX.
   ///     - "/dev/ttyS0 {pnp/drivers/serial}" : Linux serial port with extra information as returned by GetAvailablePortNames(true).
   ///     - "COM1" .. "COM99" .. "COMxxxx" .. : Standard Windows port names (where xxxx is an integer digit).
   ///     - "COM1 {USB Serial Probe}" : Windows port name as returned by GetAvailablePortNames(true).
   ///     - "\\\\.\\COMxxxx"  : Windows serial port device names in the global namespace (where xxxx is an integer digit).
   ///       Specifying such name should open the port a bit faster for names above "COM9".
   ///       Internally on Windows the port name as provided will be tried, if it fails, then the port name
   ///       with the standard network naming convention applied is tried.
   ///     - "USB Serial Probe" : Android USB probe as returned by GetAvailablePortNames(false).
   ///     - "USB Serial Probe {usb#12425}" : Android USB probe as returned by GetAvailablePortNames(true) - allows
   ///       distinguishing between devices if there is more than one of the same type connected.
   ///     - "Bluetooth Probe" : Android paired Bluetooth probe as returned by GetAvailablePortNames(false).
   ///     - "Bluetooth Probe {mac#00:01:02:03:04:05}" : Android paired Bluetooth probe as returned by GetAvailablePortNames(true) - allows
   ///       distinguishing between devices if there is more than one of the same type paired.
   ///
   /// \see \ref GetAvailablePortNames
   ///
   const MStdString& GetPortName() const
   { 
      return m_portName;
   }
   void SetPortName(const MStdString& portName);
   ///@}

   ///@{
   /// Get the baud rate of the channel, BAUD public attribute.
   ///
   /// Serial data rate as defined by the RS-232 standard.
   /// The channels, MChannelSerialPort, MChannelCurrentLoop, MChannelModem, and
   /// MChannelModemCallback have full control of the value of the Baud property and their
   /// client protocols will not make any attempt to change it. When using MChannelSerialPort,
   /// make sure that the meter configuration matches the channel settings.
   ///
   /// In contrast, MChannelOpticalProbe, child of MChannelSerialPort, this property is
   /// controlled by the protocols.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 28800
   ///
   /// \possible_values
   ///   - 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 128000, 256000
   ///
   unsigned GetBaud() const
   { 
      return m_port.GetBaud();
   }
   void SetBaud(unsigned baud)
   {
      m_port.SetBaud(baud);
   }
   ///@}

   ///@{
   /// Sets the parity bit mode for the serial data frame.
   ///
   /// The protocols impose their own restrictions on the data bits, and they handle this
   /// property during communication. For MChannelModem and MChannelModemCallback, this property
   /// has to be set in a way that is compatible with both the modem and the protocol.
   ///
   /// Only an advanced user would change this property for MChannelCurrentLoop,
   /// MChannelSerialPort, and MChannelOpticalProbe. For example, to ensure the proper display of
   /// the communication error code by the meter.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value "N" : No Parity
   ///
   /// \possible_values
   ///  - "N" : no parity
   ///  - "O" : odd parity
   ///  - "E" : even parity
   ///  - "M" : mark
   ///  - "S" : space
   ///
   char GetParity() const
   { 
      return m_port.GetParity();
   }
   void SetParity(char p)
   {
      m_port.SetParity(p);
   }
   ///@}

   ///@{
   /// The number of stop-bits in a serial data frame.
   ///
   /// The protocols impose their own restrictions on the data bits, and they handle this
   /// property during communication. For MChannelModem and MChannelModemCallback, this property
   /// has to be set in a way that is compatible with both the modem and the protocol.
   ///
   /// Only an advanced user would change this property for MChannelCurrentLoop,
   /// MChannelSerialPort and MChannelOpticalProbe. For example, to ensure the proper display of
   /// the communication error code by the meter.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 1
   ///
   /// \possible_values
   ///  - 1 or 2, there is no support for 1.5 stop bits.
   ///
   int GetStopBits() const
   { 
      return m_port.GetStopBits();
   }
   void SetStopBits(int stopBits)
   {
      m_port.SetStopBits(stopBits);
   }
   ///@}

   ///@{
   /// The number of Data bits in a serial data frame.
   ///
   /// The protocols impose their own restrictions on the data bits, and they handle this
   /// property during communication. For MChannelModem and MChannelModemCallback, this property
   /// has to be set in a way that is compatible with both the modem and the protocol.
   ///
   /// Only an advanced user would change this property for MChannelCurrentLoop,
   /// MChannelSerialPort, and MChannelOpticalProbe. For example, to ensure the proper display of
   /// the communication error code by the meter.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 8
   ///
   /// \possible_values
   ///  - 5 .. 8
   ///
   int GetDataBits() const 
   { 
      return m_port.GetDataBits();
   }
   void SetDataBits(int dataBits)
   {
      m_port.SetDataBits(dataBits);
   }
   ///@}

   ///@{
   /// Flag to enable Clear To Send (CTS) flow control on the serial port.
   ///
   /// Use caution when setting this property, as the protocols might override this value
   /// while communicating.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value
   ///  - True  : For MChannelModem, MChannelModemCallback
   ///  - False : For MChannelOpticalProbe, MChannelSerialPort, MChannelCurrentLoop
   ///
   /// \possible_values
   ///  - True [1]  : CTS flow control enabled
   ///  - False [0] : CTS flow control disabled
   ///
   bool GetCtsFlow() const
   {
      return m_port.GetCtsFlow();
   }
   void SetCtsFlow(bool isCtsFlow)
   {
      m_port.SetCtsFlow(isCtsFlow);
   }
   ///@}

   ///@{
   /// Flag to enable Data Set Ready (DSR) flow control on the serial port.
   ///
   /// Use caution when setting this property, as the protocols might override this value
   /// while communicating.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value False
   ///
   /// \possible_values
   ///  - True [1]  : DSR flow control enabled
   ///  - False [0] : DSR flow control disabled
   ///
   bool GetDsrFlow() const
   {
      return m_port.GetDsrFlow();
   }
   void SetDsrFlow(bool isDsrFlow)
   {
      m_port.SetDsrFlow(isDsrFlow);
   }
   ///@}

   ///@{
   /// Flag to set the level sense of the Data Set Ready (DSR) signal for the serial port.
   ///
   /// Use caution when setting this property, as the protocols might override this value
   /// while communicating.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value False
   ///
   /// \possible_values
   ///  - True [1]  : Communication is sensitive to DSR level.
   ///  - False [0] : Communication is not sensitive to DSR level.
   ///
   bool GetDsrSensitivity() const
   {
      return m_port.GetDsrSensitivity();
   }
   void SetDsrSensitivity(bool isDsrSens)
   {
      m_port.SetDsrSensitivity(isDsrSens);
   }
   ///@}

   ///@{
   /// Sets the Data Terminal Ready (DTR) control mode for the serial port.
   ///
   /// Use caution when overwriting this property, as the protocols impose their own restrictions on the DTR
   /// control, and they handle this property during communication. For example, setting the
   /// value of \refprop{MChannelOpticalProbe.GetBatteryState,ChannelOpticalProbe.BatteryState}
   /// will affect the value of DtrControl.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value
   ///  - "H" : For MChannelModem, MChannelModemCallback
   ///  - "D" : For MChannelSerialPort, MChannelCurrentLoop, MChannelOpticalProbe
   ///
   /// \possible_values
   ///  - "E" : enable
   ///  - "D" : disable
   ///  - "H" : handshake
   ///
   char GetDtrControl() const
   {
      return m_port.GetDtrControl();
   }
   void SetDtrControl(char dtrControl)
   {
      m_port.SetDtrControl(dtrControl);
      m_port.UpdatePortParametersOrTimeoutsIfChanged();
   }
   ///@}

   ///@{
   /// Request To Send (RTS) control mode for the serial port.
   ///
   /// The protocols impose their own restrictions on the data bits, and they handle this
   /// property during communication. For MChannelModem and MChannelModemCallback, this property
   /// has to be set in a way that is compatible with the modem.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 
   ///  - "H" : For MChannelCurrentLoop, MChannelSerialPort, MChannelModem, MChannelModemCallback 
   ///  - "D" : For MChannelOpticalProbe
   ///
   /// \possible_values
   ///  - "E" : enable
   ///  - "D" : disable
   ///  - "H" : handshake
   ///  - "T" : toggle
   ///
   char GetRtsControl() const
   {
      return m_port.GetRtsControl();
   }
   void SetRtsControl(char rtsControl)
   {
      m_port.SetRtsControl(rtsControl);
      m_port.UpdatePortParametersOrTimeoutsIfChanged();
   }
   ///@}

   /// The current state of the DCD signal of the port.
   ///
   /// \pre The port has to be open, otherwise a system error
   /// takes place.
   ///
   bool GetDCD() const
   {
      return m_port.GetDCD();
   }

   /// Return a string that will identify the media through which this channel is talking to.
   ///
   /// For serial port based channels, it will return a host name, SERIAL string, and port name.
   ///
   virtual MStdString GetMediaIdentification() const;

   ///@{
   /// Access serial port system object.
   ///
   MSerialPort& GetPort()
   {
      return m_port;
   }
   const MSerialPort& GetPort() const
   {
      return m_port;
   }
   ///@}

public: // Methods:

   /// Return a collection of serial port names available at this computer.
   ///
   /// Names returned can be given to \refprop{GetPortName,PortName}. The returned names are operating system dependent.
   ///   - On Windows the names are "COMxxxx", where xxxx is an integer digit.
   ///     When extra information is added , they are like "COM11 {USB Optical Probe}".
   ///   - UNIX-like operating systems such as Linux, QNX (but not on Android) return device names such as "/dev/ttyS0".
   ///     Extra information will be "/dev/ttyS0 {pnp/drivers/serial}".
   ///   - On Android the port name rules are more diverse as the standard Android does not expose serial port device files.
   ///     Bluetooth serial ports use the name of the paired device, and FTDI USB probes accepts the device description.
   ///     Extra information will include MAC address for Bluetooth or device index for FTDI USB device.
   ///
   /// \param addExtraInfo whether to add any extra information about the port, useful for users to select the port.
   /// \return array of present port names
   ///
   static MStdStringVector GetAvailablePortNames(bool addExtraInfo = false)
   {
      return MSerialPort::GetAvailablePortNames(addExtraInfo);
   }

   /// Get type based on the port name.
   ///
   /// Provides information about underlying port technology.
   /// When such information is not available, for example the given port name is not present,
   /// an empty string is returned. Otherwise the string returned can be one of the following types:
   ///     - "serial" wide range of different UART based serial drivers
   ///     - "bluetooth" for bluetooth devices
   ///     - "usb" USB serial devices
   ///     - "remote" remote ports such as MOXA NPort
   ///     - "" empty string can be returned only on Android when MeteringSDK
   ///       could not determine the type of the port
   ///     - Any other name will mean the type of the port is determined,
   ///       but not recognized and classified. Such name is still useful
   ///       as it identifies the port type.
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
   /// \return Port type string with contents described above.
   ///
   static MStdString GetPortType(const MStdString& portName)
   {
      return MSerialPort::GetPortType(portName);
   }

protected: // Services:
/// \cond SHOW_INTERNAL

   // Discard the contents of the input buffer of the channel.
   // All characters in the receive buffer that are waiting to be read are lost.
   //
   // \pre The channel is open, otherwise the
   // operation fails with the exception.
   //
   virtual void DoClearInputBuffer();

   // Protected routine that does connection without notifying the monitor.
   // This is the implementation convenience routine.
   //
   // \pre IsConnected should be false before calling this method.
   // Many OS and program exceptions can be thrown by this method.
   //
   void DoConnect();

   virtual unsigned DoWrite(const char* buf, unsigned len);
   virtual unsigned DoRead(char* buf, unsigned numberToRead, unsigned timeout);

protected: // Attributes:

   // Communication serial port
   //
   MSerialPort m_port;

   // Name of the serial communication port, PORT_NAME public property
   //
   MStdString m_portName;

   M_DECLARE_CLASS(ChannelSerialPort)

/// \endcond SHOW_INTERNAL
};
#endif // !M_NO_SERIAL_PORT

///@}
#endif
