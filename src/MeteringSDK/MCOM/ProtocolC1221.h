#ifndef MCOM_PROTOCOLC1221_H
#define MCOM_PROTOCOLC1221_H
/// \addtogroup MCOM
///@{
/// \file MCOM/ProtocolC1221.h

#include <MCOM/MCOMDefs.h>
#include <MCOM/ProtocolC1218.h>

#if !M_NO_MCOM_PROTOCOL_C1221

/// ANSI C12.21 protocol implementation.
///
/// ANSI Std C12.21-1998 "PROTOCOL SPECIFICATION FOR TELEPHONE MODEM COMMUNICATION", is
/// intended as a single communications standard for Water, Gas, and Electricity meters that
/// will work on any manufacturer's conforming product. A copy of the document can be obtained
/// through NEMA or ANSI. \ref MProtocolC1221 is an extension of \ref MProtocolC1218 and inherits it's
/// properties.
///
/// \ifnot M_NO_MCOM_CHANNEL_MODEM
/// This protocol is intended for working with modems (\ref MChannelModem) or through the serial
/// port channel (\ref MChannelSerialPort). When working through the modem, the \refprop{GetSessionBaud,SessionBaud}
/// property of the protocol has no affect.
/// \endif
///
class MCOM_CLASS MProtocolC1221 : public MProtocolC1218
{
public: // Type:

   /// Data format of the data link packet
   ///
   enum DataFormatEnum
   {
      DataFormatC1218C1221 = 0, ///< Default data format, C12.18 or C12.21
      DataFormatC1222      = 1, ///< Data format C12.22, as appeared in this new standard
      DataFormatReserved2  = 2,
      DataFormatReserved3  = 3
   };

public:

   /// Create the protocol object.
   ///
   MProtocolC1221(MChannel* channel, bool channelIsOwned = true);

   /// Destroy the protocol object.
   ///
   virtual ~MProtocolC1221();

   /// Setup the configuration of the channel according to the C12.21
   /// protocol handshake settings.
   ///
   virtual void ApplyChannelParameters();

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

   /// Perform ANSI C12 Identify request.
   /// This service is called from StartSession, but it can also be called 
   /// directly by the user for testing purpose.
   /// The implementation of C12.21 differs from C12.18 in the aspect that
   /// the authentication algorithm is supplied by the meter.
   ///
   /// \pre The channel is open, and the protocol state allows
   /// Identify to be called.
   ///
   virtual void Identify();

   /// Perform ANSI C12 TimingSetup request.
   ///
   /// This service is called from StartSession, but it can also be called 
   /// directly by the user for testing purpose.
   /// C12.21 allows negotiating the session timing values, as the
   /// modems or similar devices might have very different timing characteristics.
   ///
   /// \pre The channel is open, and the protocol state allows
   /// TimingSetup to be called.
   ///
   /// \see \ref TimingSetupWithWorkaround
   ///
   void TimingSetup();

   /// Perform ANSI C12 TimingSetup request, and assume a potentially buggy device that does tries instead of retries.
   ///
   /// IF the link layer retries are not zero, the behavior of this method is exactly like one of TimingSetup.
   ///
   /// \pre The channel is open, and the protocol state allows
   /// TimingSetup C12.21 service to be called.
   ///
   /// \see \ref TimingSetup - standard compliant implementation.
   ///
   void TimingSetupWithWorkaround();

   /// Perform ANSI C12 Negotiate request.
   /// This service is called from StartSession, but it can also be called 
   /// directly by the user for testing purpose.
   /// The implementation negotiates packet size, and number of packets.
   /// Whether the baud rate will be negotiated depends whether the protocol
   /// uses the modem channel (in this case no baud rate is negotiated, code 0x60),
   /// or the optical probe channel (in this case one baud rate is negotiated, command 0x61).
   ///
   /// \pre The channel is open, and the protocol state allows
   /// Negotiate to be called.
   ///
   virtual void Negotiate();

   /// Perform ANSI C12 Authenticate request.
   /// This service is called from StartSession, but it can also be called 
   /// directly by the user for testing purpose.
   /// C12.21 allows authentication procedure for better security.
   ///
   /// \pre The channel is open, and the protocol state allows
   /// Authenticate to be called.
   ///
   void Authenticate();

   /// Perform services required by C12.21 protocol to clear security with the meter.
   /// This overridable can call Security, or Authenticate, depending on the current property.
   ///
   /// \pre The channel is open, and the protocol state allows
   /// Security or Authenticate to be called.
   ///
   virtual void FullLogin();

public: // Property manipulators:

   ///@{
   /// End device identity property, the number that uniquely identifies the device.
   ///
   /// The notion of a device number is referred to as "Identity" in C12.21. In a multi-drop
   /// installation, it must be specified to target communications to a specific meter. Setting
   /// this property to zero targets all meters at once. This value is encoded in every packet
   /// sent to the meter.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 0 (all meters)
   ///
   /// \possible_values
   ///  - 0 = All, 1 .. 255
   ///
   unsigned GetIdentity() const
   {
      return m_identity;
   }
   void SetIdentity(unsigned);
   ///@}

   ///@{
   /// Protocol data format.
   ///
   /// Some meters have the ability to transparently route packets from the meter's optical port
   /// to its option board using a C12.22 meter-internal routing mechanism. The DataFormat
   /// property determines whether or not this mechanism is used.
   ///
   /// The ANSI C12.21 data link packet has a control byte (3rd byte of the packet), where bits 1
   /// .. 0 are reserved and are set to zero in normal communications. In contrast, the ANSI
   /// C12.22 standard defines these same bits as the DataFormat. Devices that support the C12.22
   /// meter-internal routing mechanism, have been made to understand the control byte's
   /// DataFormat bits when talking C12.21. The value assigned to DataFormat will be the value
   /// used for the control byte's bits 1 .. 0.
   ///
   /// \since MeteringSDK Version 4.0.23.
   ///
   /// \default_value 0
   ///
   /// \possible_values
   ///  - 0 = C12.21
   ///  - 1 = packet is routed using the meter's C12.22 internal routing mechanism
   ///  - 2 = reserved
   ///  - 3 = reserved
   ///  - Any other value throws an error
   ///
   void SetDataFormat(DataFormatEnum dataFormat);
   DataFormatEnum GetDataFormat() const
   {
      return (DataFormatEnum)m_dataFormat;
   }
   ///@}


   ///@{
   /// Tells whether the authentication or security service will be performed by StartSession service.
   ///
   /// EnableAuthentication determines whether the authentication service or the security service 
   /// is sent during the start session. It is only applicable when \refprop{GetIssueSecurityOnStartSession,IssueSecurityOnStartSession} is 
   /// True. When IssueSecurityOnStartSession is False, then EnableAuthentication is ignored as 
   /// neither the authenticate service nor the security service will be sent to the device.
   /// 
   /// When EnableAuthentication is True (and IssueSecurityOnStartSession is True), the 
   /// authenticate service is sent. Use the properties \refprop{GetAuthenticationKey,AuthenticationKey}, and 
   /// \refprop{GetAuthenticationKeyId,AuthenticationKeyId} to set the key for the authenticate service.
   ///
   /// When EnableAuthentication is False (and IssueSecurityOnStartSession is True), the security 
   /// service is sent. Use \refprop{SetPassword,Password} property to set the password for the security service. 
   ///
   /// MeteringSDK has a check prior to sending the authenticate
   /// service. When an identify service is sent (always the first service sent in a start session 
   /// sequence), the device responds with what it supports. If it says that it does not support 
   /// authentication, then MeteringSDK aborts the start session sequence and throws the error 
   /// "Meter does not support authentication". 
   ///
   /// Refer to the ANSI protocol standards for more information on the start session, security, 
   /// and authenticate services.
   /// 
   /// - ANSI Std C12.18-1995 "PROTOCOL SPECIFICATION FOR ANSI TYPE 2 OPTICAL PORT"
   /// - ANSI Std C12.21-1998 "PROTOCOL SPECIFICATION FOR TELEPHONE MODEM COMMUNICATION"
   /// - ANSI Std C12.22-2008 "PROTOCOL SPECIFICATION FOR INTERFACING TO DATA COMMUNICATION NETWORKS"
   /// 
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value True
   ///
   /// \possible_values
   ///  - True [1]  : The start session includes the authentication service 
   ///                (when \refprop{GetIssueSecurityOnStartSession,IssueSecurityOnStartSession} = True)
   ///  - False [0] : The start session includes the security service 
   ///                (when \refprop{GetIssueSecurityOnStartSession,IssueSecurityOnStartSession} = True).
   ///
   bool GetEnableAuthentication() const
   {
      return m_enableAuthentication;
   }
   void SetEnableAuthentication(bool enableAuthentication)
   {
      m_enableAuthentication = enableAuthentication;
   }
   ///@}

   ///@{
   /// Return the authentication key for C12.21 Authenticate request.
   ///
   /// This property is used during authentication upon starting the C12.21 session.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value "00000000" : Eight ASCII zeros (0x30 0x30 0x30 0x30 0x30 0x30 0x30 0x30)
   ///
   /// \possible_values
   ///  - 8 binary characters that define the standard DES C12.21 key for authentication
   ///  - 16 binary characters that define the AES key for authentication, extension to C12.21
   ///
   const MByteString& GetAuthenticationKey() const
   {
      return m_authenticationKey;
   }
   void SetAuthenticationKey(const MByteString& key);
   ///@}

   ///@{
   /// Authentication key ID for C12.21 Authenticate request.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 0
   ///
   /// \possible_values
   ///  - Key ID for the authentication process, as defined by the protocol.
   ///    The allowed values are between 0 and 255.
   ///
   unsigned GetAuthenticationKeyId() const
   {
      return (unsigned)m_authenticationKeyId;
   }
   void SetAuthenticationKeyId(unsigned);
   ///@}

#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST

   ///@{
   /// Authentication key list for the protocol.
   ///
   /// The authentication key lists overwrite the authentication key property, when given.
   /// These are used to try multiple authentication keys through a single session,
   /// which is, no doubt, a too nice security invention.
   ///
   const MByteStringVector& GetAuthenticationKeyList() const
   {
      return m_authenticationKeyList;
   }
   void SetAuthenticationKeyList(const MByteStringVector&);
   ///@}

   /// Clear the authentication key list.
   ///
   void ClearAuthenticationKeyList();

   /// Add key and key id to the authentication key list.
   ///
   void AddToAuthenticationKeyList(const MByteString& key);

   /// Return the entry which was successfully tried with the meter.
   ///
   int GetAuthenticationKeyListSuccessfulEntry() const;

#endif

   ///@{
   /// Return true if the Negotiate will be applied on starting the session.
   ///
   /// \since MeteringSDK Version 2.2.26.
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
   /// Determines whether or not the start session will issue the C12.21 timing setup service.
   ///
   /// When true, the timing setup service is issued with the values of 
   /// \refprop{GetChannelTrafficTimeout,ChannelTrafficTimeout}, \refprop{GetIntercharacterTimeout,IntercharacterTimeout},
   /// \refprop{GetAcknowledgementTimeout,AcknowledgementTimeout} and \refprop{GetLinkLayerRetries,LinkLayerRetries} 
   /// given as parameters to the timing setup service.
   ///
   /// When false, the timing setup service is not issued and instead the
   /// default values as defined by the ANSI C12.21 standard are used for communications. 
   /// In this case, setting the values of \refprop{SetChannelTrafficTimeout,ChannelTrafficTimeout},
   /// \refprop{SetIntercharacterTimeout,IntercharacterTimeout}, \refprop{SetAcknowledgementTimeout,AcknowledgementTimeout}, 
   /// or \refprop{SetLinkLayerRetries,LinkLayerRetries}, has no affect on 
   /// communications, and getting their values does not necessarily reflect the values
   /// used during communications.
   ///
   /// Set IssueTimingSetupOnStartSession to False for devices that do not
   /// support the timing setup service. The ANSI C12.21 protocol specifies that the timing setup
   /// service is optional and end devices can respond with Service Not Supported (SNS). When
   /// MeteringSDK receives a SNS from the end device, it passes the error back to the caller and
   /// does not continue with communications.
   ///
   /// \since MeteringSDK Version 3.2.6.
   ///
   /// \default_value False
   ///
   /// \possible_values
   ///  - True [1]  : The timing setup service is issued during start session.
   ///  - False [0] : The timing setup service is NOT issued.
   ///
   bool GetIssueTimingSetupOnStartSession() const
   {
      return m_issueTimingSetupOnStartSession;
   }
   void SetIssueTimingSetupOnStartSession(bool yes)
   {
      m_issueTimingSetupOnStartSession = yes;
   }
   ///@}

   /// Get the end device incoming identity.
   /// The identity is the number that uniquely identifies the device.
   ///
   /// \since MeteringSDK Version 5.0.0.890.
   ///
   unsigned GetIncomingIdentity() const
   {
      return m_incomingIdentity;
   }

   /// Get the end device data format.
   /// It keeps two bits of ctrl field of link-layer packet
   ///
   /// \since MeteringSDK Version 5.0.0.890.
   ///
   DataFormatEnum GetIncomingDataFormat() const
   {
      return (DataFormatEnum)m_incomingDataFormat;
   }

   /// Get the authentication algorithm, as received by C12.21 Identify service.
   ///
   /// This property is available only after the ANSI C12.21 Identify request is executed (for
   /// example, through a start session sequence). The value of this property will persist
   /// unchanged until the identification of a different meter is performed. If the property is
   /// queried before a successful Identify request, an error will be raised. The property
   /// \refprop{GetIdentifiedPropertiesPresent,IdentifiedPropertiesPresent} can be used to 
   /// determine if the IdentifiedAuthenticationAlgorithm is available.
   ///
   /// \pre Identify() is issued successfully, either through direct call 
   /// or through StartSession(). Otherwise the exception is thrown.
   ///
   /// \since MeteringSDK Version 5.2.0.1720.
   ///
   /// \default_value None. Must be read from meter during the start session sequence.
   ///
   /// \possible_values
   ///  - -1        : Device does not support authentication.
   ///  -  0        : DES is used.
   ///  -  1 .. 254 : Algorithm unknown to MeteringSDK.
   ///  - 255       : AES is used (this is Elster's extension to C12.21).
   ///
   int GetIdentifiedAuthenticationAlgorithm() const;

protected: // Methods:
/// \cond SHOW_INTERNAL

   // Do set the maximum possible size of the application level transmission packet
   // using number of packets and packet size
   //
   virtual void DoSetMaximumApplicationLayerPacketSize() M_NO_THROW;

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

   virtual void DoTryAuthenticationKeyEntry(const MByteString& key);

   virtual void DoVerifyAuthenticationKey(const MByteString& key);

protected: // Attributes:

   // Whether the authentication can be performed
   //
   bool m_canAuthenticate;

   // Whether the authentication is enabled
   //
   bool m_enableAuthentication;

   // Authentication key (one of the keys from ST-45 table)
   //
   MByteString m_authenticationKey;

   // Ticket used for Authentication command of the ANSI protocol
   //
   MByteString m_authenticationTicket;

   // Current authentication algorithm defined by ANSI protocol.
   // Currently the only ANSI defined algorithm supported is DES, value 0.
   // Elster supports an AES authentication algorithm, value 255.  
   //
   unsigned char m_authenticationAlgorithm;

   // Authentication key ID (index of the authentication key in ST-45 table)
   //
   unsigned char m_authenticationKeyId;

#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST

   // Protocol application level key list. Takes rule over m_authenticationKey, if not empty.
   //
   MByteStringVector m_authenticationKeyList;

   // The successful entry of the authentication key list,
   // valid only after the key from the authentication key list is successfully checked.
   //
   int m_authenticationKeyListSuccessfulEntry;

#endif // !M_NO_MCOM_PASSWORD_AND_KEY_LIST

   // Whether or not to issue Negotiate when starting session
   //
   bool m_issueNegotiateOnStartSession;

   // Whether or not to issue Timing Setup when starting session
   //
   bool m_issueTimingSetupOnStartSession;

/// \endcond SHOW_INTERNAL

   M_DECLARE_CLASS(ProtocolC1221)
};

#endif // !M_NO_MCOM_PROTOCOL_C1221

///@}
#endif
