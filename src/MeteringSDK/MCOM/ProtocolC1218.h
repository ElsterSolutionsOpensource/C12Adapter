#ifndef MCOM_PROTOCOLC1218_H
#define MCOM_PROTOCOLC1218_H
/// \addtogroup MCOM
///@{
/// \file MCOM/ProtocolC1218.h

#include <MCOM/ProtocolC12.h>

#if !M_NO_MCOM_PROTOCOL_C1218

/// ANSI C12 18 protocol implementation, a further specialization of C12 set of protocols.
///
/// ANSI Std C12.18-1996 "PROTOCOL SPECIFICATION FOR ANSI Type 2 OPTICAL Port", is intended as
/// a single communications standard for Water, Gas, and Electricity meters that will work on
/// any manufacturer's conforming product. The ANSI C12.18 is a point to point protocol
/// developed to transport table data over an optical connection. A copy of the document can
/// be obtained through NEMA or ANSI. MProtocolC1218 is the protocol implementation of the
/// ANSI C12.18 standard. It inherits properties from the abstract class MProtocol, which is
/// common to all protocols. MProtocolC1218 is intended for communications through the optical probe
/// channel, MChannelOpticalProbe. MProtocolC1221 has extensions for working through modems
/// and multi-drop networks. The Password is 20 characters long, refer to the Password help
/// topic for padding rules.
///
class MCOM_CLASS MProtocolC1218 : public MProtocolC12
{
public: // Constants:

   enum
   {
      /// Smallest packet size supported by the data link layer
      /// It shall be bigger than (BIGGEST_I2C_OPTION_BOARD_COMMAND + BIGGEST_I2C_PACKET_HEADER_OVERHEAD + 1)
      SMALLEST_PACKET_SIZE            = 32,

      BIGGEST_PACKET_SIZE             = 8192, ///< Biggest packet size supported by the data link layer
      PACKET_HEADER_AND_FOOTER_LENGTH = 8,    ///< Consists of: stp8 reserved8 ctrl8 seq_nbr8 length16 data crc2

      MAXIMUM_BAD_PACKET_LENGTH_SLEEP = 1000, ///< Maximum time to wait after receiving bad packet length
      MAXIMUM_BAD_TOGGLE_BIT_SLEEP    = 1500, ///< Maximum time to wait after receiving bad toggle bit
   };

public:

   /// Create a new C12.18 protocol with the channel given.
   ///
   /// \pre The channel allowed for this type of
   /// the protocol is compatible. Otherwise an exception
   /// is thrown.
   ///
   MProtocolC1218(MChannel* channel, bool channelIsOwned = true);

   /// Destroy the C12.18 protocol object.
   ///
   virtual ~MProtocolC1218();

   /// Setup the configuration of the channel according to the C12.18
   /// protocol handshake settings.
   ///
   virtual void ApplyChannelParameters();

protected: // Synchronous abstract services:

   // Synchronously start the session.
   // This base class service is overloaded to fulfill the duties
   // relevant to this particular protocol.
   //
   // See QStartSession for the queue version of this service.
   // Q services would work faster for the queue based protocols,
   // but they are obviously less convenient for synchronous operations.
   //
   // \pre The channel is open, the protocol state allows starting the session.
   // Otherwise exceptions can be thrown.
   //
   virtual void DoStartSession();

   // Synchronously end the session.
   // In addition to parent's implementation, this call nullifies an outgoing toggle bit afterwards.
   //
   // See QEndSession and EndSession of MProtocol for public variations of this
   // service, which also do necessary steps for monitoring, counting
   // statistics and formatting error messages.
   //
   // \pre The channel is open, the protocol state allows ending the session.
   // Otherwise an exception can be thrown.
   //
   virtual void DoEndSession();

public: // Specific C12.18 commands.

   /// Perform ANSI C12 Identify request.
   ///
   /// This service is called from StartSession, but it can also be called
   /// directly by the user for testing purpose.
   ///
   /// \pre The channel is open, and the protocol state allows
   /// Identify to be called.
   ///
   virtual void Identify();

   /// Perform ANSI C12 Negotiate request.
   ///
   /// This service is called from StartSession, but it can also be called
   /// directly by the user for testing purpose.
   /// The implementation negotiates packet size, number of packets, and
   /// one baud rate (command 0x61).
   ///
   /// \pre The channel is open, and the protocol state allows
   /// Negotiate to be called.
   ///
   virtual void Negotiate();

public: // Property manipulators:

   ///@{
   /// The maximum number of milliseconds allowed between individual characters sent or received in the same packet.
   ///
   /// Should this amount of time be exceeded in between any two characters in the packet, a timeout condition will occur.
   /// Either a retry is triggered, or if the number of retries is zero already, an error is raised.
   ///
   /// Intercharacter timeout is not precisely guarded due to hardware constraints,
   /// however the implementation guarantees that the actual timeout will not be smaller than the value given
   /// to this property. Also, when MProtocolC1221::TimingSetup gets called, intercharacter timeout is
   /// negotiated with the device to the precision of seconds. See  MProtocolC1221::TimingSetup for more details.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value
   /// \ifnot M_NO_MCOM_PROTOCOL_C1218
   ///    - 500 milliseconds for MProtocolC1218, as defined by ANSI standard.
   /// \endif
   /// \ifnot M_NO_MCOM_PROTOCOL_C1221
   ///    - 1000 milliseconds for MProtocolC1221, as defined by ANSI standard.
   /// \endif
   ///
   /// \possible_values 0 .. 255000 milliseconds, while values close to zero might not be practical.
   ///   The maximum value of 255000 milliseconds is constrained by C12.21 protocol service MProtocolC1221::TimingSetup(),
   ///   where the timeout is negotiated with the client using byte representation of seconds.
   ///
   /// \seeprop{MProtocolC1221::GetIssueTimingSetupOnStartSession,ProtocolC1221::IssueTimingSetupOnStartSession} - 
   ///          control whether TimingSetup is called at StartSession
   /// \see \ref MProtocolC1221::TimingSetup() - negotiate timeouts and retries with the client, C12.21 only
   ///
   unsigned GetIntercharacterTimeout() const
   {
      return m_intercharacterTimeout;
   }
   void SetIntercharacterTimeout(unsigned timeout);
   ///@}

   ///@{
   /// The maximum number of milliseconds to wait for the acknowledgement of the packet.
   ///
   /// Should this amount of time be exceeded after the packet is sent,
   /// and there is no ACK seen (byte 0x06), a timeout condition will occur.
   /// Either a retry is triggered, or if the number of retries is zero already, an error is raised.
   ///
   /// When MProtocolC1221::TimingSetup gets called, acknowledgement timeout is
   /// negotiated with the device to the precision of seconds. See  MProtocolC1221::TimingSetup for more details.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value
   /// \ifnot M_NO_MCOM_PROTOCOL_C1218
   ///    - 2000 milliseconds for MProtocolC1218, as defined by ANSI standard.
   /// \endif
   /// \ifnot M_NO_MCOM_PROTOCOL_C1221
   ///    - 4000 milliseconds for MProtocolC1221, as defined by ANSI standard.
   /// \endif
   ///
   /// \possible_values 0 .. 255000 milliseconds, while values close to zero might not be practical.
   ///   The maximum value of 255000 milliseconds is constrained by C12.21 protocol service MProtocolC1221::TimingSetup(),
   ///   where the timeout is negotiated with the client using byte representation of seconds.
   ///
   /// \seeprop{MProtocolC1221::GetIssueTimingSetupOnStartSession,ProtocolC1221::IssueTimingSetupOnStartSession} - 
   ///          control whether TimingSetup is called at StartSession
   /// \see \ref MProtocolC1221::TimingSetup() - negotiate timeouts and retries with the client, C12.21 only
   ///
   unsigned GetAcknowledgementTimeout() const
   {
      return m_acknowledgementTimeout;
   }
   void SetAcknowledgementTimeout(unsigned timeout);
   ///@}

   ///@{
   /// Maximum number of milliseconds the device waits for a valid packet before terminating the communications session.
   ///
   /// Channel traffic timeout applies to the ANSI C12.18 and C12.21 protocols.
   /// When C12.21 is used, the value of this property can be agreed with the device by using MProtocolC1221::TimingSetup() service.
   /// The agreement leads to approximation of the timeout value to the precision of seconds, and the approximation is done towards the upper value.
   /// For example, channel traffic timeouts 5001, 5500 or 5900 will become 6000 after successful MProtocolC1221::TimingSetup().
   /// Because the value is passed to the device as byte value of seconds, the maximum channel traffic timeout value is limited to 255000.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value
   /// \ifnot M_NO_MCOM_PROTOCOL_C1218
   ///   - 6000 milliseconds for MProtocolC1218 as defined by ANSI standard.
   /// \endif
   /// \ifnot M_NO_MCOM_PROTOCOL_C1221
   ///   - 30000 milliseconds for MProtocolC1221 as defined by ANSI standard.
   /// \endif
   ///
   /// \possible_values
   ///  - 0 .. 255000 milliseconds, while the value 0 makes little sense as it assumes the device times out after every request
   ///
   /// \seeprop{MProtocolC1221::GetIssueTimingSetupOnStartSession,ProtocolC1221::IssueTimingSetupOnStartSession} - 
   ///          control whether TimingSetup is called at StartSession
   /// \see \ref MProtocolC1221::TimingSetup() - negotiate timeouts and retries with the client, C12.21 only
   ///
   unsigned GetChannelTrafficTimeout() const
   {
      return m_channelTrafficTimeout;
   }
   void SetChannelTrafficTimeout(unsigned timeout);
   ///@}

   ///@{
   /// Packet size used by the ANSI protocol.
   ///
   /// The packet size that will be requested in the negotiate service sent to the end device.
   /// The packet size is the maximum number of bytes allowed in the link layer packet. After a
   /// successful negotiate service, the dynamic property NegotiatedPacketSize contains the
   /// actual value that is used during communication.
   ///
   /// If \refprop{GetIssueNegotiateOnStartSession,IssueNegotiateOnStartSession} = 0 [False], 
   /// then MCOM uses the PacketSize property
   /// value as set, even though the ANSI C12.18 and C12.21 standards define the default value
   /// as 64 bytes. This is useful for testing firmware.
   ///
   /// For more details refer to the ANSI C12.18 and C12.21 standards.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 
   /// \ifnot M_NO_MCOM_PROTOCOL_C1218
   ///   - 1024 bytes for MProtocolC1218, different then the ANSI standard which defaults to 64 bytes.
   /// \endif
   /// \ifnot M_NO_MCOM_PROTOCOL_C1221
   ///   - 1024 bytes for MProtocolC1221, different then the ANSI standard which defaults to 64 bytes.
   /// \endif
   ///
   /// \possible_values
   ///  - 32 .. 8192
   ///
   unsigned GetPacketSize() const
   {
      return m_packetSize;
   }
   void SetPacketSize(unsigned packetSize);
   ///@}

   ///@{
   /// Maximum number of link layer packets used by the protocol.
   ///
   /// The maximum number of packets that will be requested in the negotiate service sent to the
   /// end device. The maximum number of packets is the maximum number of data link layer packets
   /// that can be assembled by the implementation into a single service request. After a
   /// successful negotiate service, the dynamic property 
   /// \refprop{GetNegotiatedMaximumNumberOfPackets,NegotiatedMaximumNumberOfPackets}
   /// contains the actual value that is used during communication.
   ///
   /// The MaximumNumberOfPackets property is only used when the negotiate service is sent to the
   /// end device. Whether or not the negotiate service is sent to the end device depends on the
   /// property \refprop{GetIssueNegotiateOnStartSession,IssueNegotiateOnStartSession}.
   ///
   /// If \refprop{GetIssueNegotiateOnStartSession,IssueNegotiateOnStartSession} = 0 [False], 
   /// then MCOM uses the MaximumNumberOfPackets property
   /// value as set, even though the ANSI C12.18 and C12.21 standards define the default value as 1.
   /// This is useful for testing firmware.
   ///
   /// For more details refer to the ANSI C12.18 and C12.21 standards.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 255 : This is the value that MeteringSDK defaults to. The default maximum
   ///    number of packets as defined by the ANSI C12.18 and C12.21 standards is 1 packet.
   ///
   /// \possible_values
   ///  - 1 .. 255
   ///
   unsigned GetMaximumNumberOfPackets() const
   {
      return m_maximumNumberOfPackets;
   }
   void SetMaximumNumberOfPackets(unsigned num);
   ///@}

   ///@{
   /// Session baud, one which is negotiated with the meter during communication.
   ///
   /// When the session baud is nonzero, the suggested baud rate is requested in the negotiate service sent to the device.
   /// After a successful negotiate service, the dynamic property \refprop{GetNegotiatedSessionBaud,NegotiatedSessionBaud} contains the
   /// actual value that is used during communication. Prior to the negotiate service, in case of optical probe the
   /// communications start at 9600 bps.
   ///
   /// Whether or not the negotiate service is sent to the device depends on the property
   /// \refprop{GetIssueNegotiateOnStartSession,IssueNegotiateOnStartSession}. 
   /// Therefore, to change optical probe baud at negotiate,
   /// one shall set \refprop{SetIssueNegotiateOnStartSession,IssueNegotiateOnStartSession} to True, 
   /// and \refprop{SetSessionBaud,SessionBaud} shall be a nonzero value
   /// that is a valid baud rate accepted by this property, and by the device.
   ///
   /// For more details refer to the ANSI C12.18 and C12.21 standards.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 9600
   ///
   /// \possible_values
   ///  - 0 means session baud is not agreed on Negotiate.
   ///  - 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 128000, 256000 - possible session baud rates.
   ///    Not all optical probes are able to support the higher baud rates.
   ///    Devices may not support all speeds. Support of 9600 is required because this is the speed that the session starts at.
   ///    Device support for speeds is not necessarily hierarchical, for example a device might support 128000 but not 115200.
   ///
   unsigned GetSessionBaud() const
   {
      return m_sessionBaud;
   }
   void SetSessionBaud(unsigned sessionBaud);
   ///@}

   ///@{
   /// Whether the Negotiate C12 service shall be applied within StartSession sequence.
   ///
   /// When the negotiate service is issued (\refprop{GetIssueNegotiateOnStartSession,IssueNegotiateOnStartSession} = 1), 
   /// the values of the protocol properties 
   /// \refprop{GetSessionBaud,SessionBaud}, \refprop{GetMaximumNumberOfPackets,MaximumNumberOfPackets}, 
   /// and \refprop{GetPacketSize,PacketSize} are the
   /// communication parameters that are requested in the negotiate service. The device does not
   /// have to accept the requested communication parameters and may reject them by specifying
   /// different values to be used. To discover what values were actually used during
   /// communications, one can query the negotiated properties.
   ///
   /// When the negotiate service is not issued (IssueNegotiateOnStartSession = 0), the default
   /// values of the protocol properties (as defined by the ANSI C12.18 and C12.21 standard) are
   /// used for communications. Setting the values of \refprop{GetSessionBaud,SessionBaud}, 
   /// \refprop{GetMaximumNumberOfPackets,MaximumNumberOfPackets}, and \refprop{GetPacketSize,PacketSize}, 
   /// does not affect communications, and reading these property values does not
   /// necessarily reflect the values used during communications.
   ///
   /// Set IssueNegotiateOnStartSession to false [0] for devices that do not support the
   /// negotiate service. The ANSI C12.18 and C12.21 protocol specifies that the negotiate
   /// service is optional and devices can respond with Service Not Supported (SNS). When
   /// MeteringSDK receives an SNS from the device, it passes the error back to the caller and
   /// does not continue with communications.
   ///
   /// \since MeteringSDK Version 2.2.26.
   ///
   /// \default_value True
   ///
   /// \possible_values
   ///  - True [1]  : The negotiate service is issued during start session.
   ///  - False [0] : The negotiate service is NOT issued, \refprop{GetNegotiatedPropertiesPresent,NegotiatedPropertiesPresent} is False
   ///    and the dynamic negotiated properties raise an error if accessed.
   ///
   bool GetIssueNegotiateOnStartSession() const
   {
      return m_issueNegotiateOnStartSession;
   }
   void SetIssueNegotiateOnStartSession(bool yes)
   {
      m_issueNegotiateOnStartSession = yes;
   }
   ///@}

   ///@{
   /// Whether the Logoff C12 service shall be applied within EndSession sequence.
   ///
   /// A standard C12.18 and C12.21 session can end with Terminate, optionally preceded by Logoff.
   /// This property allows omitting of Logoff when EndSession is called.
   /// The property has no effect on the Logoff() method that can always be called directly.
   ///
   /// \since MeteringSDK Version 5.2.0.1720.
   ///
   /// \default_value
   /// \ifnot M_NO_MCOM_PROTOCOL_C1218
   ///   - True for MProtocolC1218.
   /// \endif
   /// \ifnot M_NO_MCOM_PROTOCOL_C1221
   ///   - True for MProtocolC1221.
   /// \endif
   ///
   /// \possible_values
   ///  - True : Logoff service is issued as part of EndSession sequence
   ///  - False : Logoff service is issued as part of EndSession sequence.
   ///
   bool GetIssueLogoffOnEndSession() const
   {
      return m_issueLogoffOnEndSession;
   }
   void SetIssueLogoffOnEndSession(bool yes)
   {
      m_issueLogoffOnEndSession = yes;
   }
   ///@}

   ///@{
   /// Number of milliseconds which the application layer should wait while reading the ST_007 table.
   ///
   /// The number of milliseconds to wait for the device to respond to a procedure initiate request, 
   /// which is a table write to ST_007. Some procedures, on some devices, can take a long time 
   /// to execute a procedure. Typically devices handle this by acknowledging the ST_007 write, then responding 
   /// to the ST_008 table read (procedure response request) with the response code 'procedure accepted 
   /// but not fully completed', which allows for \refprop{GetApplicationLayerProcedureRetries,ApplicationLayerProcedureRetries} 
   /// to be utilized. However, some devices do not respond at all while executing the procedure, resulting
   /// in channel read timeout errors on the link layer where \refprop{GetLinkLayerRetries,LinkLayerRetries} are utilized. 
   /// The ProcedureInitiateTimeout value corrects this situation by temporarily increasing the number of 
   /// Link Layer Retries so that \refprop{GetLinkLayerRetries,LinkLayerRetries} * \refprop{GetAcknowledgementTimeout,AcknowledgementTimeout} 
   /// is not less than ProcedureInitiateTimeout. 
   ///
   /// \since MeteringSDK Version 5.1.2.1690.
   ///
   /// \default_value 20000 milliseconds
   ///
   /// \possible_values
   ///   - 0 .. UINT_MAX
   ///
   unsigned GetProcedureInitiateTimeout() const
   {
      return m_procedureInitiateTimeout;
   }
   void SetProcedureInitiateTimeout(unsigned procedureInitiateTimeout)
   {
      m_procedureInitiateTimeout = procedureInitiateTimeout;
   }
   ///@}

   ///@{
   /// The value of the toggle bit that will be used in the next outgoing data link packet.
   ///
   /// The ANSI C12.18 and C12.21 protocols define a toggle bit in the control field definition
   /// byte. The toggle bit is used to detect duplicate packets. The NextOutgoingToggleBit
   /// property tells what the state of the next toggle bit will be. When True, the next toggle
   /// bit will be set to 1. When False, the next toggle bit will be reset to 0.
   ///
   /// The NextOutgoingToggleBit property is useful for clients who want to start a session with
   /// the meter using QStartSession, then create their own packet(s) to send to the
   /// meter (can be sent using channel WriteBytes service). These clients should set/reset the
   /// toggle bit of their control byte based on the value of this property. When they are done
   /// sending their own packets, they can set the NextOutgoingToggleBit property and the protocol will
   /// use this property to determine how to set the toggle bit in the next package it sends to
   /// the meter.
   ///
   /// The object sets the NextOutgoingToggleBit to zero after an end session. During normal
   /// operations, where clients are using QEndSession to terminate the session, the start
   /// session will always start with the toggle bit set to zero making it easier to compare logs
   /// from different sessions. However, for clients that want to start the session with the
   /// toggle bit set to 1, they can do so by simply setting the NextOutgoingToggleBit to 1 prior
   /// to starting the session.
   ///
   /// Note that when using the communication queues, the communication is executed on QCommit
   /// service. It is important to get the NextOutgoingToggleBit after the QCommit service
   /// completes, and to set it before starting the QCommit service.
   ///
   /// \since MeteringSDK Version 3.2.17.
   ///
   /// \default_value False
   ///
   /// \possible_values
   ///  - True [1]  : The next outgoing toggle bit will be set to 1.
   ///  - False [0] : The next outgoing toggle bit will be reset to 0.
   ///
   bool GetNextOutgoingToggleBit() const
   {
      return m_nextOutgoingToggleBit;
   }
   void SetNextOutgoingToggleBit(bool yes)
   {
      m_nextOutgoingToggleBit = yes;
   }
   ///@}

   ///@{
   /// Whether to wake up the shared optical probe on start session.
   ///
   /// Determines whether or not a garbage character (0x55) is sent before an identify request to
   /// 'wake up' the shared optical port. This property only needs to be set to True when the
   /// meter has a shared optical port. Sending the garbage character resolves the problem where
   /// the UART is connected to the remote port, the identify command is sent through the optical
   /// port, the meter receives the first character (0xEE) while connected to the remote port and
   /// switches the UART to the optical port and changes the bit rate, but the meter misses at
   /// least the first character of the packet. By sending a garbage character, the meter is
   /// given time to switch the UART to the optical port before it receives the first request.
   ///
   /// \since MeteringSDK Version 2.2.26.
   ///
   /// \default_value False
   ///
   /// \possible_values
   ///  - True [1]  : The garbage character, 0x55, is sent before an identify request in the
   ///                start session sequence.
   ///  - False [0] : The garbage character is NOT sent before an identify request in the start
   ///                session sequence.
   ///
   void SetWakeUpSharedOpticalPort(bool wakeUp)
   {
      m_wakeUpSharedOpticalPort = wakeUp;
   }
   bool GetWakeUpSharedOpticalPort() const
   {
      return m_wakeUpSharedOpticalPort;
   }
   ///@}

   /// Get the reference standard, as identified by the C12.18 Identify command.
   ///
   /// \pre The Identify is issued successfully, through direct call
   /// or through StartSession. Otherwise the exception is thrown.
   ///
   /// The ANSI Reference Standard implemented by the meter.
   /// This property is available only after the ANSI C12 Identify request is executed (for
   /// example, through a start session sequence). The value of this property persists
   /// unchanged until the identification of a different meter is performed. If the property is
   /// queried before a successful Identify request, an error is raised. The property
   /// \refprop{GetIdentifiedPropertiesPresent,IdentifiedPropertiesPresent} can be used to 
   /// determine if the IdentifiedReferenceStandard is available.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value None. Must be read from meter during the start session sequence.
   ///
   /// \possible_values
   ///  - 0 : ANSI C12.18
   ///  - 1 : Used by Industry Canada
   ///  - 2 .. 255 : Reserved
   ///
   int GetIdentifiedReferenceStandard() const;

   /// Get the standard version, as identified by the C12.18 Identify command.
   ///
   /// \pre The Identify is issued successfully, through direct call
   /// or through StartSession. Otherwise the exception is thrown.
   ///
   /// The version of the ANSI C12.xx protocol standard, as implemented by the meter. This
   /// property is important for scripts that are affected by the differences between versions of
   /// the ANSI C12.xx standards.
   ///
   /// This property is available only after the ANSI C12 Identify request is executed (for
   /// example, through a start session sequence). The value of this property persists
   /// unchanged until the identification of a different meter is performed. If the property is
   /// queried before a successful Identify request, an error is raised. The property
   /// \refprop{GetIdentifiedPropertiesPresent,IdentifiedPropertiesPresent} can be used to 
   /// determine if the IdentifiedStandardVersion is available.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value None. Must be read from meter during the start session sequence.
   ///
   /// \possible_values
   ///  - 0 .. 255
   ///
   int GetIdentifiedStandardVersion() const;

   /// Get the standard revision, as identified by the C12.18 Identify command.
   ///
   /// \pre The Identify is issued successfully, through direct call
   /// or through StartSession. Otherwise the exception is thrown.
   ///
   /// The revision of the ANSI C12.xx protocol standard, as implemented by the meter. This
   /// property is important for scripts that are affected by the differences between revisions
   /// of the ANSI C12.xx standards.
   ///
   /// This property is available only after the ANSI C12 Identify request is executed (for
   /// example, through a start session sequence). The value of this property persists
   /// unchanged until the identification of a different meter is performed. If the property is
   /// queried before a successful Identify request, an error is raised. The property
   /// \refprop{GetIdentifiedPropertiesPresent,IdentifiedPropertiesPresent} can be used to 
   /// determine if the IdentifiedStandardRevision is available.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value None. Must be read from meter during the start session sequence.
   ///
   /// \possible_values
   ///  - 0 .. 255
   ///
   int GetIdentifiedStandardRevision() const;

   /// Tells if Identify command was successful, and IDENTIFIED
   /// properties are available.
   ///
   /// Specifies whether or not the dynamic identified properties are available. The identified
   /// properties are only available after a successful start session sequence.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \possible_values
   ///  - True [1]  : The identified properties are available.
   ///  - False [0] : The identified properties are not available and querying them will result
   ///                in an error being raised.
   ///
   /// \seeprop{GetIdentifiedReferenceStandard,IdentifiedReferenceStandard}
   /// \seeprop{GetIdentifiedStandardVersion,IdentifiedStandardVersion}
   /// \seeprop{GetIdentifiedStandardRevision,IdentifiedStandardRevision}
   ///
   bool GetIdentifiedPropertiesPresent() const
   {
      return m_identifiedPropertiesPresent;
   }

   /// Return the packet size which was negotiated with the meter in the Negotiate command.
   ///
   /// \pre The Negotiate is issued successfully, through direct call
   /// or through StartSession. Otherwise the exception is thrown.
   ///
   /// The number of bytes in the packet that was negotiated with the meter during the start
   /// session sequence.
   /// This dynamic property is available only after the completion of a successful start session
   /// sequence. The value of this property will persist unchanged until another start session
   /// sequence. If the property is queried before a successful start session sequence, an error
   /// is raised. The property \refprop{GetNegotiatedPropertiesPresent,NegotiatedPropertiesPresent} 
   /// can be used to determine if the NegotiatedPacketSize is available.
   ///
   /// During the Negotiate service, if the meter accepts the \refprop{GetPacketSize,PacketSize} 
   /// from MeteringSDK, then NegotiatedPacketSize = PacketSize.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \possible_values
   ///  - 32 .. 65535
   ///
   unsigned GetNegotiatedPacketSize() const;

   /// Return the maximum number of packets, which was negotiated with the
   /// meter in the Negotiate command.
   ///
   /// \pre The Negotiate is issued successfully, through direct call
   /// or through StartSession. Otherwise the exception is thrown.
   ///
   /// The number of packets that was negotiated with the meter during the start session
   /// sequence.
   /// This dynamic property is available only after the completion of a successful start session
   /// sequence. The value of this property will persist unchanged until another start session
   /// sequence. If the property is queried before a successful start session sequence, an error
   /// will be raised. The property \refprop{GetNegotiatedPropertiesPresent,NegotiatedPropertiesPresent} 
   /// can be used to determine if the NegotiatedMaximumNumberOfPackets is available.
   ///
   /// During the Negotiate service, if the meter accepts the \refprop{GetMaximumNumberOfPackets,MaximumNumberOfPackets} 
   /// from MeteringSDK, then NegotiatedMaximumNumberOfPackets = MaximumNumberOfPackets.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \possible_values
   ///  - 1 .. 255
   ///
   unsigned GetNegotiatedMaximumNumberOfPackets() const;

   /// Return the negotiated session baud, which was returned by the
   /// meter in the Negotiate command.
   ///
   /// \pre The Negotiate is issued successfully, through direct call
   /// or through StartSession. Otherwise the exception is thrown.
   ///
   /// The SessionBaud rate that was negotiated with the meter during a start session sequence.
   /// This dynamic property is available only after the completion of a successful start session
   /// sequence. The value of this property will persist unchanged until another start session
   /// sequence. If the property is queried before a successful start session sequence, an error
   /// is raised. The property \refprop{GetNegotiatedPropertiesPresent,NegotiatedPropertiesPresent} 
   /// can be used to determine if the NegotiatedSessionBaud is available.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \possible_values
   ///  - 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 128000, 256000
   ///
   unsigned GetNegotiatedSessionBaud() const;

   /// Tells if Negotiate command was successful, and negotiated
   /// properties are available.
   ///
   /// Specifies whether or not the dynamic negotiated properties are available. The negotiated
   /// properties are only available after a successful start session sequence.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \possible_values
   ///  - True [1]  : The negotiated properties are available.
   ///  - False [0] : The negotiated properties are not available and querying them results in an
   ///                error being raised.
   ///
   /// \seeprop{GetNegotiatedMaximumNumberOfPackets,NegotiatedMaximumNumberOfPackets}
   /// \seeprop{GetNegotiatedPacketSize,NegotiatedPacketSize}
   /// \seeprop{GetNegotiatedSessionBaud,NegotiatedSessionBaud}
   ///
   bool GetNegotiatedPropertiesPresent() const
   {
      return m_negotiatedPropertiesPresent;
   }

   ///@{
   /// Whether to check the incoming toggle bit or not.
   ///
   /// This property is true by default. If it is false, the incoming toggle bit
   /// is ignored. 
   ///
   /// The ANSI C12.18 and C12.21 protocols define a toggle bit in the control field definition
   /// byte. This bit is used to detect duplicate packets. By setting this property to false,
   /// MeteringSDK can be forced to not check the toggle bit. This is useful for meters that do
   /// not properly implement the ANSI C12 toggle bit.
   ///
   /// \since MeteringSDK Version 3.0.5.
   ///
   /// \default_value True
   ///
   /// \possible_values
   ///  - True [1]  : The incoming toggle bit is checked.
   ///  - False [0] : The incoming toggle bit is NOT checked.
   ///
   bool GetCheckIncomingToggleBit() const
   {
      return m_checkIncomingToggleBit;
   }
   void SetCheckIncomingToggleBit(bool doCheck)
   {
      m_checkIncomingToggleBit = doCheck;
      m_receiveToggleBitKnown = false; // always clear this flag at each set, whether to true or false
   }
   ///@}


public: // Semi-public services useful for testing purposes:
/// \cond SHOW_INTERNAL


   // Perform the full application layer request, abstract service.
   // The command will be the first byte in the packet, while the request parameter,
   // if present, will form the rest of the packet.
   //
   // The need to overload this service appears through the way the toggle bit is handled.
   //
   // \pre The request corresponds to the correct C12 service packet.
   // The channel is functioning, the state allows such request.
   // Otherwise the exception is thrown.
   //
   virtual void DoApplicationLayerRequest(char command, const MByteString *request = NULL, unsigned flags = APPLICATIONLAYERREQUEST_NO_FLAGS);

   // Special brand of application layer request for Identify call, one that ignores packet from the previous session
   //
   void DoApplicationLayerRequestForIdentify();

   // Perform the application layer write, which will result in transferring
   // one or more data link packets through the reliable data link layer.
   // The data part could be missing, only the command is required.
   //
   // \pre The data corresponds to the correct C12 service packet,
   // and the meter is in the process of receiving the data
   // The channel is functioning, the state allows such request.
   // Otherwise the exception is thrown.
   //
   bool DoApplicationLayerWrite(char command, const MByteString* data = NULL);

   /// Perform the full application layer read, which will result in receiving
   /// one or more data link packets through the reliable data link layer.
   /// The resulted application layer packet is written in m_applicationLayerResponse.
   /// The application layer response code is available through return value,
   /// while the rest of the information (if it was sent back), is readable
   /// through DoApplicationLayerResponseRead.
   ///
   /// \pre The data link is functioning, and the meter is in the
   /// process of sending the data. The channel is functioning, the state allows such request.
   /// Otherwise the exception is thrown.
   ///
   MEC12NokResponse::ResponseCode DoFullApplicationLayerRead();

   /// Perform the application layer read, which will result in receiving
   /// one or more data link packets through the reliable data link layer.
   /// The application layer response code is available through return value,
   /// while the rest of the information (if it was sent back), is readable
   /// through DoApplicationLayerResponseRead.
   ///
   /// \pre The data link is functioning, and the meter is in the
   /// process of sending the data. The channel is functioning, the state allows such request.
   /// Otherwise the exception is thrown.
   ///
   MEC12NokResponse::ResponseCode DoApplicationLayerRead();

   /// Read the incoming packet on the server side;
   /// first, it erases m_applicationLayerResponse and then uses
   /// DoApplicationLayerRead in order to correctly read incoming packet.
   /// If DoApplicationLayerRead returns -1, then it sends BSY response and
   /// tries to read again. Can be used by both C12.18 and C12.21 protocols.
   ///
   /// \pre The data link is functioning, and the meter is in the
   /// process of sending the data. The channel is functioning, the state allows such request.
   /// Otherwise the exception is thrown.
   ///
   void ServerStart();

   /// Write the data packet given on the server side.
   ///
   /// Uses DoApplicationLayerWrite to send packet; can be used by both C12.18 and C12.21 protocols.
   /// The data part could be missing, only the command is required.
   ///
   /// \pre The data corresponds to the correct C12 service packet,
   /// and the meter is in the process of receiving the data
   /// The channel is functioning, the state allows such request.
   /// Otherwise the exception is thrown.
   ///
   void ServerEnd(char command, const MByteString& data);

   // Calculate the big endian CRC of the data link packet given as parameter,
   // as defined by C12 protocol.
   //
   // \pre The given buffers is of the valid size and length.
   //
   static Muint16 DoCalculateBigEndianCRC16(const char *buff, unsigned len);

   // Convert ordinal baud rate to index character defined by the protocol.
   // See Negotiate service description in the protocol paper for additional detail.
   // The conversion table is as follows:
   // <ul>
   // <li>    300 = 0x01 </li>
   // <li>    600 = 0x02 </li>
   // <li>   1200 = 0x03 </li>
   // <li>   2400 = 0x04 </li>
   // <li>   4800 = 0x05 </li>
   // <li>   9600 = 0x06 </li>
   // <li>  14400 = 0x07 </li>
   // <li>  19200 = 0x08 </li>
   // <li>  28800 = 0x09 </li>
   // <li>  57600 = 0x0A </li>
   // <li>  38400 = 0x0B </li>
   // <li> 115200 = 0x0C </li>
   // <li> 128000 = 0x0D </li>
   // <li> 256000 = 0x0E </li>
   // </ul>
   //
   // \pre The baud given is to be an allowed baud value,
   // such as 9600 or 14400. See details on conversion table above.
   // Otherwise the exception is thrown.
   //
   static char DoConvertBaudToIndex(unsigned baud);

   // Convert baud rate index to ordinal baud rate defined by the protocol.
   // See Negotiate service description in the protocol paper for additional detail.
   // The conversion table is as follows:
   // <ul>
   // <li> 0x01  =    300 </li>
   // <li> 0x02  =    600 </li>
   // <li> 0x03  =   1200 </li>
   // <li> 0x04  =   2400 </li>
   // <li> 0x05  =   4800 </li>
   // <li> 0x06  =   9600 </li>
   // <li> 0x07  =  14400 </li>
   // <li> 0x08  =  19200 </li>
   // <li> 0x09  =  28800 </li>
   // <li> 0x0A  =  57600 </li>
   // <li> 0x0B  =  38400 </li>
   // <li> 0x0C  = 115200 </li>
   // <li> 0x0D  = 128000 </li>
   // <li> 0x0E  = 256000 </li>
   // </ul>
   //
   // \pre The index given is to be an allowed index value,
   // such as 0x01 or 0x08. See details on conversion table above.
   // Otherwise the exception is thrown.
   //
   static unsigned DoConvertIndexToBaud(char index);

protected: // Services:

   // Return the number of data link packets that are required for a given request,
   // provided an optional size of the application data.
   // Like for table read, this will be the expected size of the table to read,
   // and for the table write this is the size of the table to be written.
   // For function execution this is the sum of request and response sizes.
   //
   // The returned value is not necessarily precise, but it can be used to estimate
   // the required time or the progress gauge movement.
   //
   // \pre The type of request has to be within the enumeration range.
   // There is a debug check for that, but there is not one in release version.
   //
   virtual unsigned GetNumberOfDataLinkPackets(MCommunicationCommand::CommandType typeOfRequest, unsigned applicationLayerDataSize = 0) M_NO_THROW;

   // Protected service that internally sets the negotiated packet size to the
   // given value. Its duty over parent is to clear the packet buffer and do the checking.
   //
   // \pre The value has to be within the valid range SMALLEST_PACKET_SIZE
   // to BIGGEST_PACKET_SIZE, or an exception will be thrown.
   //
   virtual void DoSetNegotiatedPacketSize(unsigned negotiatedPacketSize);

   // Get the internal packet buffer used in communication
   //
   // \pre Should be enough memory, otherwise
   // the standard memory exception is thrown.
   //
   char* DoGetPacketBuffer()
   {
      M_ASSERT(m_dataLinkPacketBuffer != NULL);
      return m_dataLinkPacketBuffer;
   }

   // Do set the maximum possible size of the application level transmission packet
   // using number of packets and packet size
   //
   virtual void DoSetMaximumApplicationLayerPacketSize() M_NO_THROW;

#if !M_NO_MCOM_IDENTIFY_METER
   virtual MStdString DoIdentifyMeter(bool sessionIsStarted, TableRawDataVector* tablesRead);
#endif

#if !M_NO_MCOM_KEEP_SESSION_ALIVE

   // Implementation of the C12.18 service that returns the number of milliseconds to delay
   // before the first sending the first KeepSessionAlive message to the meter.
   // It never throws an exception. If the returned value is zero, no keeping of the session shall be made.
   // The delays for the next keeping are returned by SendKeepSessionAliveMessage oneself.
   //
   // This particular implementation uses CHANNEL_TRAFFIC_TIMEOUT property to determine the first delay.
   //
   virtual unsigned DoGetKeepSessionAliveFirstDelay() const;

#endif

protected: // Attributes:

   // Intercharacter timeout for the protocol
   //
   unsigned m_intercharacterTimeout;

   // Number of milliseconds to wait for the acknowledgement of the packet
   //
   unsigned m_acknowledgementTimeout;

   // Channel traffic timeout, as defined by ANSI C12.18 and C12.21 protocols
   //
   unsigned m_channelTrafficTimeout;

   // Whether the IDENTIFY was called successfully, and its properties are present
   //
   bool m_identifiedPropertiesPresent;

   // Identified reference standard as received by Identify command
   //
   char m_identifiedReferenceStandard;

   // Identified standard version as received by Identify command
   //
   char m_identifiedStandardVersion;

   // Identified standard revision as received by Identify command
   //
   char m_identifiedStandardRevision;

   // Whether the NEGOTIATE was called successfully, and its properties are present
   //
   bool m_negotiatedPropertiesPresent;

   // Maximum number of data link packets that the protocol implementation can assemble.
   //
   unsigned m_maximumNumberOfPackets;

   // Initial baud of the protocol.
   // For C12.18 it is always 9600, but C12.21 is able to change it.
   // This is an implementation convenience to hold this property here.
   //
   unsigned m_initialBaud;

   // Session baud that is to be used during communication.
   //
   unsigned m_sessionBaud;

   // The device identity number.
   // This one is primarily for C12.21 protocol, but it is an implementation
   // convenience to define it here. It is always zero for C12.18.
   // There are no public properties defined within C12.18 that handle it.
   //
   unsigned m_identity;

   // The incoming device identity number.
   // This one is primarily for the server part of C12.21 protocol, but it is an implementation
   // convenience to define it here. It is always zero for C12.18.
   // There are no public properties defined within C12.18 that handle it.
   //
   unsigned m_incomingIdentity;

   // Data format, static part of the data link layer control byte.
   //
   Muint8 m_dataFormat;

   // The incoming device data format.
   // This one is primarily for the server part of C12.21 protocol, but it is an implementation
   // convenience to define it here. It is always zero for C12.18.
   // There are no public properties defined within C12.18 that handle it.
   //
   Muint8 m_incomingDataFormat;

   // CRC of the previous packet, used to tell a toggle bit error from the new session case.
   // It is valid only if m_receiveToggleBitKnown is true.
   //
   Muint16 m_savedCRC;

   // Number of milliseconds to wait while initiating procedure.
   // This parameter identifies how many milliseconds the client may wait
   // for a response to the ST07 table write request. Retries are sent until 
   // timeout expires, or the meter answers.
   //
   unsigned m_procedureInitiateTimeout;

   // This gets toggled each time a packet is sent.
   //
   bool m_nextOutgoingToggleBit;

   // This is a current state of the receive toggle bit.
   //
   bool m_receiveToggleBit;

   // This is the test whether the value of the receive toggle bit is known.
   //
   bool m_receiveToggleBitKnown;

   // Whether or not to issue Negotiate during starting session.
   //
   bool m_issueNegotiateOnStartSession;

   // Whether or not to issue Logoff at the EndSession sequence.
   //
   bool m_issueLogoffOnEndSession;

   // Whether to wake up the shared optical probe on start session.
   //
   bool m_wakeUpSharedOpticalPort;

   // Whether to check the incoming toggle bit.
   // This property allows communicating to devices that do not implement the toggle bit feature well.
   //
   bool m_checkIncomingToggleBit;

   // Packet size used during communication.
   //
   unsigned m_packetSize;

   // Negotiated session baud, the one returned by the meter.
   //
   unsigned m_negotiatedSessionBaud;

   // Negotiated maximum number of data link packets in the application layer
   // transmission, the one returned by the meter.
   //
   unsigned m_negotiatedMaximumNumberOfPackets;

   // Response of the application layer, excluding the status byte.
   // If the response has only the status byte, this is empty.
   //
   MBuffer m_applicationLayerIncoming;

   // Temporary buffer for holding the data link packets, m_negotiatedPacketSize size.
   //
   char* m_dataLinkPacketBuffer;

/// \endcond SHOW_INTERNAL

   M_DECLARE_CLASS(ProtocolC1218)
};

#endif // !M_NO_MCOM_PROTOCOL_C1218

///@}
#endif
