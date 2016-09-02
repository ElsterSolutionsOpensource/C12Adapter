#ifndef MCOM_CHANNELCURRENTLOOP_H
#define MCOM_CHANNELCURRENTLOOP_H
/// \addtogroup MCOM
///@{
/// \file MCOM/ChannelCurrentLoop.h

#include <MCOM/MCOMDefs.h>
#include <MCOM/ChannelSerialPort.h>

#if !M_NO_SERIAL_PORT

/// MChannelCurrentLoop implements methods necessary for handling RS-232
/// serial port. This is still an abstract implementation of MChannel,
/// that facilitates:
///    - concrete child: MChannelOpticalProbe   
/// \ifnot M_NO_MCOM_CHANNEL_MODEM 
///    - concrete child: MChannelModem 
/// \endif
///
/// The serial port channel exports the following properties through
/// the property dispatch mechanism:
/// <ul>
/// <li> PORT_NAME, type MStdString, name of the port, default is COM1.
///      The port name is the one determined by the operating system. 
///      Under Windows, the typical port name is COMn, where n is some
///      number. Windows does not impose any restrictions on what the 
///      port name is. </li>
/// <li> BAUD, type unsigned, baud rate, default is 28800. The other values
///      are defined by the RS232 standard: 300, 1200, 4800, 9600, 14400, 
///      28000, etc. </li>
/// <li> PARITY, type char, port parity, default is 'N', no parity.
///      The values are N (no parity), O (odd parity), E (even parity), 
///      M (mark), or S (space). </li>
/// <li> DATA_BITS, type int, default is 8, data bit values are 5 to 8. </li>
/// <li> STOP_BITS, type int, default is 1, stop bit values are 1 or 2, there is no support for 1.5. </li>
/// <li> CTS_FLOW, type bool, whether the CTS flow is enabled. 
///      Default value for MChannelCurrentLoop is false.</li>
/// <li> DSR_FLOW, type bool, whether the DSR flow is enabled, default is false. </li>
/// <li> DSR_SENSITIVITY, type bool, whether the port is sensitive to DSR, default is false. </li>
/// <li> DTR_CONTROL, type char, DTR control values are E (enable), D (disable), 
///      and H (handshake). Default value for MChannelCurrentLoop is D (disable).</li>
/// <li> RTS_CONTROL, type char, RTS control values are E (enable), D (disable), 
///      H (handshake), and T (toggle). Default value for MChannelCurrentLoop is H (handshake).</li>
/// <li> ECHO, type bool, whether the echo is enabled by the hardware,
///      so the characters sent are received back. 
///      Default value for MChannelCurrentLoop is true. </li>
/// </ul>
/// More information on property handling and error treatment is given
/// within descriptions for property accessors and modifiers.
///
/// MChannelCurrentLoop inherits all of its properties from MChannelSerialPort and differs
/// only by the default value for the Echo property. This type is provided to allow users to
/// quickly create a direct serial port link through a current loop adapter without having to
/// set the Echo property.  Many properties of MChannelCurrentLoop are controlled by the
/// communication protocols; however, an advanced user can override the values set by the
/// protocols at any time to achieve advanced behavior for test purposes. 
///
class MCOM_CLASS MChannelCurrentLoop : public MChannelSerialPort
{
public: // Constructor, destructor:

   /// Construct serial port channel.
   ///
   MChannelCurrentLoop();

   /// Destructor.
   ///
   virtual ~MChannelCurrentLoop();

   M_DECLARE_CLASS(ChannelCurrentLoop)
};

#endif // !M_NO_SERIAL_PORT

///@}
#endif
