#ifndef MCOM_PROTOCOLC1222_H
#define MCOM_PROTOCOLC1222_H
/// \addtogroup MCOM
///@{
/// \file MCOM/ProtocolC1222.h

#include <MCOM/BufferBidirectional.h>
#include <MCOM/ProtocolC12.h>

#if !M_NO_MCOM_PROTOCOL_C1222

/// ANSI C12.22 protocol implementation.
///
/// ANSI Std C12.22-200x "PROTOCOL SPECIFICATION FOR INTERFACING TO DATA COMMUNICATION
/// NETWORKS", is intended as a single communications standard for Water, Gas, and Electricity
/// meters that will work on any manufacturer's conforming product. The ANSI C12.22 protocol
/// was developed to transport table data over a networking communications system. It does not
/// have a data link layer, so the link layer counter properties will remain zero for this
/// protocol. A copy of the document can be obtained through NEMA or ANSI.
///
/// \ifnot M_NO_MCOM_CHANNEL_SOCKET
/// \ref MProtocolC1222 is intended for communications through the socket channel, \ref MChannelSocket.
/// \endif
///
class MCOM_CLASS MProtocolC1222 : public MProtocolC12
{
   friend class MProtocolThread;

public: // Types, constants:

   /// Values for security mode property
   ///
   /// Property \refprop{GetSecurityMode, SecurityMode} is based on ASCII standard's definition of security modes.
   ///
   enum SecurityModeEnum
   {
      SecurityUndefined                              = -1,   ///< Undefined security, special option used at configuration to mark protocols with unknown security.
      SecurityClearText                              =  0,   ///< Clear text, no authentication or encryption.
      SecurityClearTextWithAuthentication            =  1,   ///< Clear text with authentication.
      SecurityCipherTextWithAuthentication           =  2    ///< Encryption and authentication is on.
   };


   /// EPSEM Response control bits
   ///
   /// Comprises all possible values of property \refprop{GetResponseControl, ResponseControl}
   ///
   enum ResponseControlEnum
   {
      ResponseControlAlways      = 0, ///< Always respond to a C12.22 request
      ResponseControlOnException = 1, ///< Respond only on exception
      ResponseControlNever       = 2  ///< Never respond, one way communication
   };

   enum
   {
      /// Maximum possible size of APDU header (everything but data), calculated correctly.
      ///
      MaximumProperApduHeaderSize = 160,

      /// Maximum possible size of APDU header (everything but data), calculated by legacy former devices.
      ///
      MaximumLegacyApduHeaderSize = 1024,

      /// Maximum size in bytes of ISO length of a PSEM within EPSEM.
      ///
      /// The length will be is 1 to 3 bytes. 3 bytes is when it is bigger than 0xFF.
      /// 
      MaximumEpsemServiceLengthSize = 3,

      /// Minimum supported total size of APDU.
      ///
      /// As applied to \refprop{GetMaximumApduSize, MaximumApduSize} 
      /// and \refprop{GetMaximumApduSizeIncoming, MaximumApduSizeIncoming}
      ///
      MinimumMaximumApduTotalSize = 512,

      /// Maximum supported total size of APDU.
      ///
      /// As applied to \refprop{GetMaximumApduSize, MaximumApduSize}
      /// and \refprop{GetMaximumApduSizeIncoming, MaximumApduSizeIncoming}
      ///
      MaximumMaximumApduTotalSize = 0x1000000
   };

protected:
/// \cond SHOW_INTERNAL

   enum
   {

      SESSIONLESS_SECURITY_SERVICE_OVERHEAD = 24    ///< PSEM length + code + password + userId
   };

/// \endcond SHOW_INTERNAL
public: // Constructor, destructor:

   /// Create C12.22 protocol object.
   ///
   /// \param channel
   ///    The channel object. Most often used object here will be MChannelSocket,
   ///    When this parameter is NULL, the channel shall be given later to \refprop{SetChannel,Channel} property
   ///    prior to any communication attempt.
   ///
   /// \param channelIsOwned
   ///    Whether the channel object is owned by the protocol.
   ///    The value can be set later using \refprop{SetIsChannelOwned,IsChannelOwned} property.
   ///      - If true, the channel will be deleted when the protocol itself is deleted,
   ///        or when the channel is reassigned to the protocol with property \refprop{SetChannel,Channel}.
   ///      - If false, the channel will not be deleted internally by the protocol, and it is expected the
   ///        channel is deleted elsewhere.
   ///
   explicit MProtocolC1222(MChannel* channel = NULL, bool channelIsOwned = true);

   /// Destroy the protocol object.
   ///
   virtual ~MProtocolC1222();

public: // Services:

   /// Perform ANSI C12 Disconnect request.
   /// This is named DisconnectService because the name Disconnect is already
   /// used in class MProtocol.
   ///
   /// \pre The channel is open, and the protocol state allows
   /// Disconnect to be called.
   ///
   void DisconnectService();

public: // Property manipulators:


   ///@{
   /// Protocol security mode.
   ///
   /// Determines whether or not the request and response messages are encrypted.
   /// For more information regarding the security mode, refer to the ANSI Standard C12.22
   /// documentation.
   ///
   /// \since MeteringSDK Version 4.0.35.
   ///
   /// \default_value
   ///  - 0 (clear text) 
   ///
   /// \possible_values
   ///  - 0  = clear text
   ///  - 1  = clear text with authentication
   ///  - 2  = cipher text with authentication
   ///  - -1 = Undefined security mode, special value used by applications. Communication attempts with this value will fail.
   ///  - Any other value results in an error
   ///
   void SetSecurityMode(SecurityModeEnum mode);
   SecurityModeEnum GetSecurityMode() const
   {
      return m_securityMode;
   }
   ///@}

   ///@{
   /// The security key used for encryption and authentication. 
   ///
   /// The security key is a hex string of exactly 32 characters (representing
   /// 16 bytes). The property is only used when \refprop{GetSecurityMode,SecurityMode} <> 0. 
   /// For more information regarding the security key, refer to the ANSI Standard C12.22-200x
   /// documentation.
   ///
   /// \since MeteringSDK Version 4.0.35.
   ///
   /// \default_value
   ///  - "00000000000000000000000000000000"
   ///
   /// \possible_values
   ///  - Any 32 character string that uses hex characters, 0 .. 9, A .. F.
   ///
   void SetSecurityKey(const MStdString&);
   MStdString GetSecurityKey() const;
   ///@}


   ///@{
   /// Security key ID identifies the key used to encrypt and decrypt the information.
   ///
   /// The SecurityKeyID is matched with an entry in the KEY_ENTRIES array located 
   /// in the Security Table (Table 45, KEY_TBL or Table 46, EXTENDED_KEY_TBL) of the 
   /// target C12.22 Node. This element is optional, and when not provided in a 
   /// sessionless message, the SecurityKeyId shall be 0, otherwise
   /// the previous SecurityKeyId of the session shall be used.
   ///
   /// The SecurityKeyId may be different in each C12.22 Message transmitted. The SecurityKeyId
   /// used in a request may be different from the SecurityKeyId used in the response message.
   /// For more information regarding the security key ID, refer to the ANSI Standard C12.22-200x
   /// documentation.
   ///
   /// \since MeteringSDK Version 4.0.35.
   ///
   /// \default_value 0
   ///
   /// \possible_values
   ///  - 0 .. 255, though it can be limited further by the device capability.
   ///
   void SetSecurityKeyId(int);
   int GetSecurityKeyId() const
   {
      return m_securityKeyId;
   }
   ///@}


   /// Whether the start session includes the security service or the authenticate service.
   ///
   /// When it is set to False, neither the security nor the authenticate service is sent,
   /// allowing data that is not password protected to be accessed without the password.
   ///
   /// Refer to the ANSI protocol standards for more information on the start session, security, 
   /// and authenticate services.
   ///   - ANSI Std C12.18-1995 "PROTOCOL SPECIFICATION FOR ANSI TYPE 2 OPTICAL PORT"
   ///   - ANSI Std C12.21-1998 "PROTOCOL SPECIFICATION FOR TELEPHONE MODEM COMMUNICATION"
   ///   - ANSI Std C12.22-2008 "PROTOCOL SPECIFICATION FOR INTERFACING TO DATA COMMUNICATION NETWORKS"
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value True
   ///
   /// \possible_values
   ///  - True [1]  : The security or authenticate service is issued with the password.
   ///  - False [0] : The security or authenticate service is NOT issued. Data that does not
   ///                require a password can be read. Attempting to access data that requires
   ///                a password will generate an error.
   ///
   virtual void SetIssueSecurityOnStartSession(bool);


   ///@{
   /// Whether the protocol mode is sessionless.
   ///
   /// When Sessionless = true, communications are done in the sessionless mode, and start and end 
   /// session requires are ignored. Sessionless mode
   /// is more efficient for connections with high latency, as it puts many services into one
   /// request. There is no guarantee that the whole queue will be done in one trip in sessionless mode.
   ///
   /// For more information regarding the sessionless behavior, refer to the ANSI Standard
   /// C12.22-200x documentation.
   ///
   /// \since MeteringSDK Version 4.0.11.
   ///
   /// \default_value True [1]
   ///
   /// \possible_values
   ///  - True  [1] = Sessionless mode
   ///  - False [0] = Session mode
   ///
   void SetSessionless(bool);
   bool GetSessionless() const
   {
      return m_sessionless;
   }
   ///@}

   ///@{
   /// Whether the protocol should transmit services one at a time (only one service per APDU).
   ///
   /// When OneServicePerApdu is false the protocol will attempt to
   /// place as many C12 services as possible into the packet.
   ///
  /// When OneServicePerApdu is true the behavior differs 
   /// on whether the protocol is sessionless:
   ///   - If \refprop{GetSessionless,Sessionless}=false each C12 service will be sent 
   ///     in a separate packet resulting in a very long start and end session sequence.
   ///     Each packet will always have only one C12 service.
   ///   - If \refprop{GetSessionless,Sessionless}=true each packet will consist of 
   ///     Security service followed by the payload service such as table read or write.
   ///     The name 'one service per APDU' is somewhat misleading 
   ///     as there will be a pair of services sent in each packet.
   ///
   /// \since MeteringSDK Version 5.0.0.980.
   ///
   /// \default_value False [0]
   ///
   /// \possible_values
   ///  - True  [1] = One payload service per APDU mode is in effect.
   ///  - False [0] = Multiple services packed into a single APDU.
   bool GetOneServicePerApdu() const
   {
      return m_oneServicePerApdu;
   }
   void SetOneServicePerApdu(bool yes)
   {
      m_oneServicePerApdu = yes;
   }
   ///@}

   ///@{
   /// Whether the protocol EPSEM request is going to be one-way.
   /// One way requests cannot pass information back from devices,
   /// therefore table reads and function executions with response cannot be handled.
   ///
   /// Response control of the outgoing EPSEM packet, as defined by the ANSI Standard C12.22
   /// documentation.
   /// When ResponseControl <> 0, \refprop{GetSessionless,Sessionless} = False is not supported and an error is thrown.
   /// When ResponseControl <> 0, an error is thrown for any request that requires application-
   /// level information to be returned from the peer. Examples are: full/partial table read,
   /// and executing a function that has RESPONSE data.
   ///
   /// \since MeteringSDK Version 4.0.46.
   ///
   /// \default_value 0 = always respond
   ///
   /// \possible_values
   ///  - 0 = always respond
   ///  - 1 = only respond on error
   ///  - 2 = never respond
   ///  - An error is thrown for any other value.
   ///
   ResponseControlEnum GetResponseControl() const
   {
      return m_responseControl;
   }
   void SetResponseControl(ResponseControlEnum c);
   ///@}

   ///@{
   /// Whether the end of the session shall be completed with Terminate (or Logoff) C12 service.
   ///
   /// When true, the session is ended with C12.22 Terminate service. When false, the session is
   /// ended with C12.22 Logoff service. This property is applied only when 
   /// \refprop{GetSessionless,Sessionless} = False.
   ///
   /// For more information regarding the end session behavior, refer to the ANSI Standard
   /// C12.22-200x documentation.
   ///
   /// \since MeteringSDK Version 5.0.0.
   ///
   /// \default_value False [0]
   ///
   /// \possible_values
   ///  - False [0] = End session with C12.22 Logoff service
   ///  - True  [1] = End session with C12.22 Terminate service
   ///
   bool GetIssueTerminateOnEndSession() const
   {
      return m_issueTerminateOnEndSession;
   }
   void SetIssueTerminateOnEndSession(bool yes)
   {
      m_issueTerminateOnEndSession = yes;
   }
   ///@}

   ///@{
   /// Tells whether or not the \refprop{GetNegotiatedSessionIdleTimeout,NegotiatedSessionIdleTimeout}
   /// property is available.
   ///
   /// For more information regarding the session idle timeout properties, refer to the ANSI
   /// Standard C12.22-200x documentation.
   ///
   /// \since MeteringSDK Version 4.0.11.
   ///
   /// \possible_values
   ///  - False [0] = \refprop{GetNegotiatedSessionIdleTimeout,NegotiatedSessionIdleTimeout} is NOT available.
   ///  - True [1]  = \refprop{GetNegotiatedSessionIdleTimeout,NegotiatedSessionIdleTimeout} is available.
   ///
   bool GetNegotiatedSessionIdleTimeoutPresent() const
   {
      return m_negotiatedSessionIdleTimeoutPresent;
   }
   unsigned GetNegotiatedSessionIdleTimeout() const;
   ///@}

   ///@{
   /// The number of seconds to wait after sending a packet to get a response.
   ///
   /// Should this timeout be exceeded, a timeout error is raised.
   ///
   /// \since MeteringSDK Version 4.0.43.
   ///
   /// \default_value
   ///  - 300 seconds for MProtocolC1222 and MeteringSDK Versions 5.0.0
   ///    and later. Prior to MeteringSDK Version 5.0.0, the default 
   ///    was 60 seconds.
   ///    Note that the standard specifies a default of 300 seconds.
   ///
   /// \possible_values
   ///    0 .. MAX_INT in seconds, while having it at zero is not practical.
   ///    Very large values will represent practically infinite timeout,
   ///    while due to the implementation constraint, values bigger than 
   ///    2147483 will have the same effect as 2147483 seconds, which is about 24 days.
   ///
   unsigned GetResponseTimeout() const
   {
      return m_responseTimeout;
   }
   void SetResponseTimeout(unsigned timeout);
   ///@}

   ///@{
   /// Maximum size of the incoming application protocol data unit (APDU).
   ///
   /// The property was introduced in MeteringSDK 6.1 with compatibility in mind.
   /// When this property is zero, \refprop{GetMaximumApduSize, MaximumApduSize}
   /// sets both the incoming and outgoing maximum APDU size, as it did before the 
   /// introduction of this property.
   /// When this property is not zero, it sets the maximum size of the incoming APDU.
   ///
   /// For more information about maximum APDU size, refer to the ANSI Standard C12.22-200x documentation.
   ///
   /// \since MeteringSDK Version 6.1.0.3833.
   ///
   /// \default_value 
   ///  - 0 for MProtocolC1222, which indicates that the value of \refprop{GetMaximumApduSize, MaximumApduSize} 
   ///    is used for the maximum size of the incoming APDU.
   ///
   /// \possible_values
   ///  - 0 - use the value of \refprop{GetMaximumApduSize, MaximumApduSize} as the maximum incoming APDU size.
   ///  - 0x200 .. 0x1000000, which is 512 .. 16,777,216 (16 megabytes),
   ///    though it can be limited further by the device capability.
   ///    MeteringSDK versions prior to 6.4.0.4830 had 0x400 as the minimum value.
   ///    MeteringSDK versions prior to 6.2.0.4200 had 0x800 as the minimum value.
   ///
   /// \seeprop{GetMaximumApduSize, MaximumApduSize} - outgoing or common APDU size (if maximum incoming APDU size is zero).
   ///
   unsigned GetMaximumApduSizeIncoming() const
   {
      return m_maximumApduSizeIncoming;
   }
   void SetMaximumApduSizeIncoming(unsigned size);
   ///@}

   ///@{
   /// Maximum size of the outgoing application protocol data unit (APDU).
   ///
   /// Prior to the introduction of \refprop{GetMaximumApduSizeIncoming, MaximumApduSizeIncoming}, this property
   /// set the maximum size of both the incoming and outgoing APDU. If \refprop{GetMaximumApduSizeIncoming, MaximumApduSizeIncoming}
   /// is zero, this property still sets the maximum limit for both the incoming and outgoing APDU size.
   ///
   /// For more information about maximum APDU size, refer to the ANSI Standard C12.22-200x documentation.
   ///
   /// \since MeteringSDK Version 4.0.43.
   ///
   /// \default_value 
   ///  - 32767 (0x7FFF) for MProtocolC1222.
   ///
   /// \possible_values
   ///  - 0x200 .. 0x1000000, which is 512 .. 16,777,216 (16 megabytes), though it can be limited further by the device capability.
   ///    MeteringSDK versions prior to 6.4.0.4830 had 0x400 as the minimum value.
   ///    MeteringSDK versions prior to 6.2.0.4200 had 0x800 as the minimum value.
   ///
   /// \seeprop{GetMaximumApduSizeIncoming, MaximumApduSizeIncoming} - incoming APDU size that can be the same as outgoing, or different.
   ///
   unsigned GetMaximumApduSize() const
   {
      return m_maximumApduSizeOutgoing;
   }
   void SetMaximumApduSize(unsigned size);
   ///@}

   /// Get the incoming negotiated maximum size of the APDU.
   /// 
   /// This value in the beginning of the session is equal to 
   /// the maximum incoming APDU size, whether it is determined by a dedicated property
   /// \refprop{GetMaximumApduSizeIncoming, MaximumApduSizeIncoming}, or, if this is zero,
   /// by \refprop{GetMaximumApduSize, MaximumApduSize}. 
   /// Later on the value of negotiated maximum APDU size can change 
   /// if RSTL C12 Nok response is received.
   ///
   /// \since MeteringSDK Version 6.1.0.3833.
   ///
   /// \default_value 0x7FFF, which is 32767
   ///
   /// \possible_values
   ///  - 0x200 .. 0x1000000, which is 512 .. 16,777,216 (16 megabytes),
   ///    though it can be limited further by the device capability.
   ///
   /// \seeprop{GetMaximumApduSizeIncoming, MaximumApduSizeIncoming}
   /// \seeprop{GetMaximumApduSize, MaximumApduSize}
   /// \seeprop{GetNegotiatedMaximumApduSize, NegotiatedMaximumApduSize}
   ///
   unsigned GetNegotiatedMaximumApduSizeIncoming() const
   {
      return m_negotiatedMaximumApduSizeIncoming;
   }

   /// Get the outgoing negotiated maximum size of the APDU.
   /// 
   /// This value in the beginning of the session is equal to 
   /// \refprop{GetMaximumApduSize, MaximumApduSize}. 
   /// Later on the value of negotiated maximum APDU size can change 
   /// if RQTL C12 NOK response is received.
   ///
   /// \since MeteringSDK Version 5.3.0.2202.
   ///
   /// \default_value 0x7FFF, which is 32767
   ///
   /// \possible_values
   ///  - 0x200 .. 0x1000000, which is 512 .. 16,777,216 (16 megabytes),
   ///    though it can be limited further by the device capability.
   ///
   /// \seeprop{GetMaximumApduSizeIncoming, MaximumApduSizeIncoming}
   /// \seeprop{GetMaximumApduSize, MaximumApduSize}
   /// \seeprop{GetNegotiatedMaximumApduSizeIncoming, NegotiatedMaximumApduSizeIncoming}
   ///
   unsigned GetNegotiatedMaximumApduSize() const
   {
      return m_negotiatedMaximumApduSizeOutgoing;
   }

   /// Change the negotiated incoming maximum APDU size parameter in a way so that 
   /// all dependents are also updated.
   ///
   virtual void ChangeNegotiatedMaximumApduSizeIncoming(unsigned size);

   /// Change the negotiated outgoing maximum APDU size parameter in a way so that 
   /// all dependents are also updated.
   ///
   virtual void ChangeNegotiatedMaximumApduSizeOutgoing(unsigned size);

   ///@{
   /// Calling Application Entity Qualifier is to supply additional information about the message. 
   ///
   /// In MeteringSDK 6.1.0.3832, this property was renamed from IncomingAeQualifier to IncomingCallingAeQualifier
   ///
   /// This is an optional ACSE element, bit mask. When it is -1, the AE Qualifier was not included
   /// in the incoming message, or there was no incoming message. The property stands for the outgoing message,
   /// while there is also a read-only property IncomingCallingAeQualifier. When the device does not support
   /// Calling AE Qualifier, the incoming property will not be present (will return -1).
   ///
   /// The ANSI C12.22 standard defines the following bits for the AE Qualifier:
   ///   - Bit 0 : When this is set, the message is a test, and it shall not affect the state of the device.
   ///             If the device supports this, the device shall include a calling-AE-qualifier-element in its
   ///             response with this bit set.
   ///   - Bit 1 : Urgent message, should be processed in priority manner through relays and by devices.
   ///   - Bit 2 : Notification bit.
   ///
   /// \since MeteringSDK Version 6.1.0.3832.
   ///
   /// \default_value -1, the AE Qualifier element is not included in the message.
   ///
   /// \possible_values
   ///   - -1 : AE Qualifier is not included in the message.
   ///   - 0 .. 7 : Possible values for Bits 0 .. 2.
   ///   - 8 .. 255 : Bits 3 .. 7 are reserved, so these values have no meaning.
   ///
   /// \refprop{GetIncomingCallingAeQualifier, IncomingCallingAeQualifier}
   ///
   int GetCallingAeQualifier() const
   {
      return m_callingAeQualifier;
   }
   void SetCallingAeQualifier(int q)
   {
      m_callingAeQualifier = q;
   }
   ///@}

   /// The Incoming Application Entity Qualifier is to supply additional information about the message.
   ///
   /// In MeteringSDK 6.1.0.3832, this property was renamed from AeQualifier to CallingAeQualifier
   ///
   /// This is an optional ACSE element, bit mask. When it is -1, the AE Qualifier was not included
   /// in the incoming message, or there was no incoming message. If the outgoing message had a calling AE qualifier
   /// but the corresponding incoming one does not, according to standard this means that incoming AE qualifier is not
   /// supported by the callee.
   ///
   /// \since MeteringSDK Version 6.1.0.3832.
   ///
   /// \possible_values
   ///  - -1 : Incoming AE Qualifier is not included in the message.
   ///  - 0 .. 255 : Refer to CallingAeQualifier for description of values.
   ///
   /// \refprop{GetCallingAeQualifier, CallingAeQualifier}
   ///
   int GetIncomingCallingAeQualifier() const
   {
      return m_incomingCallingAeQualifier;
   }

   ///@{
   /// Application context is the ISO8825 Universal Identifier of the common context for all Relative IOS8825 Universal 
   /// Identifiers used by the protocol.
   /// 
   /// When ApplicationContext is "", it is not included in the message. Both the C12.22 Client 
   /// and C12.22 Communication Module assume that the ApplicationContext is 
   /// "2.16.124.113620.1.22", which is the identifier reserved to C12.22.
   /// 
   /// The following text is taken, nearly verbatim, from the ANSI C12.22 standard: 
   /// 
   /// <blockquote>
   ///    The called application title uniquely identifies the target of an ACSE (Association 
   ///    Control Service Element, ISO8650-1-1995) message. This uniqueness is guaranteed by the 
   ///    use of an Absolute or Relative Universal Identifier. Relative Universal Identifiers are 
   ///    derived from the ANSI C12.22 ApTitle branch (<application-context-oid>.0).
   /// </blockquote>
   ///
   /// The value of <application-context-oid> is set by ApplicationContext.
   /// The value of the called application title is set by \refprop{GetCalledApTitle,CalledApTitle}. 
   /// Here is an example of how to use Relative Universal Identifiers:
   ///  
   /// \code
   ///    ApplicationContext = "1.1.112"
   ///    CalledApTitle = ".0"
   /// \endcode   
   ///  
   /// When using Relative Universal Identifiers, the CalledApTitle should start with a period. 
   /// However, no error is thrown if it doesn't which allows firmware developers 
   /// and testers to try invalid combinations.
   /// 
   /// Here is an example of how to use Absolute Universal Identifiers:
   ///  
   /// \code
   ///    ApplicationContext = ""
   ///    CalledApTitle = "1.1.112"
   /// \endcode   
   ///  
   /// When MeteringSDK assembles the message, the ApplicationContext property (if not "") is sent 
   /// in the Application Context Element (0xA1), and the \refprop{GetCalledApTitle,CalledApTitle} is sent in the Called AP 
   /// Title Element (0xA2).
   /// 
   /// For more information, refer to the ANSI Standard C12.22-200x documentation, or the 
   /// ISO8825-1-1997 documentation.
   ///
   /// \since MeteringSDK Version 4.0.35.
   ///
   /// \default_value
   ///  - "" (empty string) 
   ///
   /// \possible_values
   ///  - Any valid Universal Identifier.
   ///
   void SetApplicationContext(const MStdString&);
   const MStdString& GetApplicationContext() const
   {
      return m_applicationContext;
   }
   ///@}

   ///@{
   /// Calling Ap Title is the ISO8825 Universal Identifier of the calling application.
   ///
   /// If the string is empty, no Application Title will be associated with the calling application.
   /// The calling application title uniquely identifies the initiator of an ASCE (Association
   /// Control Service Element, ISO8650-1-1995) message. This uniqueness is guaranteed by the use
   /// of a absolute or relative Universal Identifier. Relative Universal Identifiers are derived
   /// from the ANSI C12.22 ApTitle branch (1.2.840.10066.3).
   ///
   /// The effect of setting this property will take place next time
   /// the session is started, or the logon procedure is performed.
   ///
   /// \since MeteringSDK Version 2.2.17.
   ///
   /// \default_value
   ///  - "" (empty string) 
   ///
   /// \possible_values
   ///  - Any valid Universal Identifier.
   ///
   void SetCallingApTitle(const MStdString&);
   const MStdString& GetCallingApTitle() const
   {
      return m_callingApTitle;
   }
   ///@}

   ///@{
   /// Session idle timeout, value to use in C12 Logon service.
   ///
   /// The maximum number of seconds a session may be idle on the C12.22 Server side before the
   /// C12.22 Server may terminate the session.
   /// There is also a special value, zero, that means the session will never time out.
   ///
   /// \since MeteringSDK Version 4.0.11.
   ///
   /// \default_value 60 seconds
   ///
   /// \possible_values
   ///  - 0 : Special value, session will never time out.
   ///  - 1 .. MAX_INT : Number of seconds to allow the session to be idle.
   ///    Very large values will represent practically infinite timeout,
   ///    while due to the implementation constraint, values bigger than 
   ///    2147483 will have the same effect as 2147483 seconds, which is about 24 days.
   ///
   unsigned GetSessionIdleTimeout() const
   {
      return m_sessionIdleTimeout;
   }
   void SetSessionIdleTimeout(unsigned timeout);
   ///@}

   ///@{
   /// Called Ap Title is the ISO8825 Universal Identifier of the called application. If the string is empty, no 
   /// Application Title will be associated with the called application and it will not be sent to 
   /// the device.
   /// 
   /// The following is taken, nearly verbatim from the ANSI C12.22 standard: 
   /// 
   /// <blockquote>
   ///    The called application title uniquely identifies the target of an ACSE (Association 
   ///    Control Service Element, ISO8650-1-1995) message. This uniqueness is guaranteed by the 
   ///    use of an Absolute or Relative Universal Identifier. Relative Universal Identifiers are 
   ///    derived from the ANSI C12.22 ApTitle branch (application-context-oid.0).
   /// </blockquote>
   /// 
   /// The value of application-context-oid is set by \refprop{GetApplicationContext,ApplicationContext}.
   /// Here is an example of how to use Absolute Universal Identifiers:
   ///  
   /// \code
   ///    ApplicationContext = ""
   ///    CalledApTitle = "1.1.112"
   /// \endcode
   ///   
   /// Here is an example of how to use Relative Universal Identifiers:
   /// 
   /// \code
   ///    ApplicationContext = "1.1.112"
   ///    CalledApTitle = ".0"
   /// \endcode
   ///  
   /// When using Relative Universal Identifiers, the CalledApTitle should start with a period. 
   /// However, no error is thrown if it doesn't which allows firmware developers 
   /// and testers to try invalid combinations.
   /// 
   /// When MeteringSDK assembles the message, the \refprop{GetApplicationContext,ApplicationContext} property (if not "") is sent 
   /// in the Application Context Element (0xA1), and the CalledApTitle is sent in the Called AP 
   /// Title Element (0xA2).
   /// 
   /// The effect of setting this property will take place next time
   /// the session is started, or the logon procedure is performed.
   ///
   /// For more information, refer to the ANSI Standard C12.22-200x documentation, or the 
   /// ISO8825-1-1997 documentation.
   /// 
   /// \since MeteringSDK Version 2.2.17.
   ///
   /// \default_value
   ///  - "" (empty string) 
   ///
   /// \possible_values
   ///  - Any valid Universal Identifier.
   ///
   void SetCalledApTitle(const MStdString& calledApTitle);
   const MStdString& GetCalledApTitle() const
   {
      return m_calledApTitle;
   }
   ///@}

   ///@{
   /// The EdClass string to place in the outgoing packet.
   ///
   /// If this is an empty string, EdClass will not be included into the EPSEM outgoing request.
   /// The EdClass is encoded in the General Configuration Table (Table 0) of ANSI C12.19 Version 2.0
   /// or the MANUFACTURER code as defined in the
   /// General Configuration Table (Table 0) of ANSI C12.19-1997 (Version 1.0).
   ///
   /// \since MeteringSDK Version 4.0.46.
   ///
   /// \default_value "" - empty string, no EdClass will be sent in the outgoing packet.
   ///
   /// \possible_values
   ///  - Any string with printable characters, maximum of 4 characters.
   ///    If it is an empty string, there will be no EdClass in the outgoing EPSEM request.
   ///    If it is one to three characters, the outgoing EPSEM request will be filled with blanks up to four characters in length.
   ///
   const MStdString& GetEdClass() const
   {
      return m_edClass;
   }
   void SetEdClass(const MStdString& edClass);
   ///@}

   ///@{
   /// The whole incoming application data unit.
   ///
   /// After any successfully accepted APDU, this property will hold 
   /// the whole byte representation of the arrived APDU,
   /// convenient for custom logging, data forwarding, or any other purpose.
   /// If there was no successful APDU received, this property will return an empty byte string.
   ///
   /// When the property is assigned a byte string,
   /// the given representation of APDU will be parsed up to the ACSE that comprises the header
   /// of the APDU, everything except the EPSEM of the packet. This should be sufficient to
   /// use the calling AP title to fetch the key from the storage, and assign it to the protocol.
   /// After that, \ref ProcessIncomingEPSEM can be called in order to decrypt the packet
   /// and initialize \refprop{GetIncomingEpsem,IncomingEpsem} property.
   ///
   /// \since MeteringSDK Version 6.0.0.3136 there is a read-only version.
   /// \since MeteringSDK Version 6.6.0.5979 there is a read-write version.
   ///
   MByteString GetIncomingApdu() const;
   void SetIncomingApdu(const MByteString&);
   ///@}

   /// The whole incoming application EPSEM.
   ///
   /// This property is an empty string until \ref ProcessIncomingEPSEM is called
   /// after receiving or initializing the incoming APDU.
   /// After a successful \ref ProcessIncomingEPSEM call, this property is a byte string that
   /// represents a sequence of PSEM data.
   ///
   /// \since MeteringSDK Version 6.6.0.5978 there is a read-write version.
   ///
   MByteString GetIncomingEpsem() const;

   /// Get the whole outgoing application data unit.
   ///
   /// After any successfully sent APDU, this property will have its whole byte representation,
   /// convenient for monitoring or debugging.
   /// If there was no successful APDU sent this property will return an empty byte string.
   ///
   /// \since MeteringSDK Version 6.4.0.4699.
   ///
   MByteString GetOutgoingApdu() const;

   /// Get the EPSEM EdClass that was used in the last EPSEM response.
   /// If this is an empty string, EdClass was not included into the last response, or there was no EPSEM issued.
   ///
   /// \pre EPSEM response information shall be present, or an exception is thrown.
   ///
   /// The EdClass string retrieved from the incoming packet. If there was no incoming packet,
   /// then it is an empty string. For more information about this property, refer to the ANSI
   /// Standard C12.22-200x documentation.
   ///
   /// \since MeteringSDK Version 4.0.46.
   ///
   /// \possible_values
   ///  - Any string with printable characters, maximum of 4 characters.
   ///
   const MStdString& GetIncomingEdClass() const
   {
      return m_incomingEdClass;
   }

   /// The Called Application Title of the incoming EPSEM packet. If no incoming packet has been
   /// received, this property returns an empty string. This property is most useful to clients
   /// who need to emulate a C12.22 device. It is not particularly useful to clients
   /// communicating to a real C12.22 device.
   ///
   /// \since MeteringSDK Version 4.0.46.
   ///
   /// \default_value "" (empty string)
   ///
   /// \possible_values
   ///  - Any valid Universal Identifier.
   ///
   const MStdString& GetIncomingCalledApTitle() const
   {
      return m_incomingCalledApTitle;
   }

   /// The Calling Application Title of the incoming EPSEM packet. If no incoming packet has been
   /// received, this property returns an empty string. This property is most useful to clients
   /// who need to emulate a C12.22 device. It is not particularly useful to clients
   /// communicating to a real C12.22 device.
   ///
   /// \since MeteringSDK Version 4.0.46.
   ///
   /// \default_value "" (empty string)
   ///
   /// \possible_values
   ///  - Any valid Universal Identifier.
   ///
   const MStdString& GetIncomingCallingApTitle() const
   {
      return m_incomingCallingApTitle;
   }

   /// Security Mode of the incoming EPSEM packet. If no incoming packet has been received, this
   /// property returns a value of zero. This property is most useful to clients who need to
   /// emulate a C12.22 device. It is not particularly useful to clients communicating to a real
   /// C12.22 device.
   ///
   /// \since MeteringSDK Version 4.0.46.
   ///
   /// \default_value 0 = clear text
   ///
   /// \possible_values
   ///  - 0 = clear text
   ///  - 1 = clear text with authentication
   ///  - 2 = cipher text with authentication
   ///  - Any other value results in an error
   ///
   SecurityModeEnum GetIncomingSecurityMode() const
   {
      return m_incomingSecurityMode;
   }

   /// The Response Control of the incoming EPSEM packet, as defined by the ANSI Standard C12.22
   /// documentation.
   ///
   /// \possible_values
   ///  - 0 = always respond
   ///  - 1 = only respond on error
   ///  - 2 = never respond
   ///
   ResponseControlEnum GetIncomingResponseControl() const
   {
      return m_incomingResponseControl;
   }

   ///@{
   /// Initialization vector, as used in the outgoing APDU.
   /// Initialization vector will return zero if MCOM has not sent the ACSE initialization vector parameter to the peer.
   /// At the event of secure message exchange, MCOM will initialize the value of the property 
   /// according to C12.22 standard, unless it was changed programmatically by the client application 
   /// through assigning the value to InitializationVector.
   /// After the value is assigned by the application, even if it is zero, MProtocolC1222
   /// will use it exactly once in the following APDU - if security settings require initialization vector ACSE to be sent. 
   /// After that, if InitializationVector is not assigned by client again, MCOM will be free to fall into its 
   /// way of changing initialization vector.
   ///
   /// \since MeteringSDK Version 4.0.70.
   ///
   unsigned GetInitializationVector() const
   {
      return m_initializationVector;
   }
   void SetInitializationVector(unsigned id)
   {
      m_initializationVector = id;
      m_initializationVectorSetByUser = true;
   }
   ///@}

   ///@{
   /// The value of calling invocation ID as used in the outgoing APDU.
   ///
   /// Accessing this property has debugging and security purpose only.
   /// MCOM will always send the invocation ID in the outgoing packet, 
   /// behaving as the standard requires, but if this property is assigned by the application, 
   /// the newly assigned value will be used exactly once in the next APDU, 
   /// after which MCOM will be using its default behavior again.
   ///
   /// \since MeteringSDK Version 4.0.70.
   ///
   unsigned GetCallingApInvocationId() const
   {
      return m_callingApInvocationId;
   }
   void SetCallingApInvocationId(unsigned id)
   {
      m_callingApInvocationId = id;
      m_callingApInvocationIdSetByUser = true;
   }
   ///@}

   /// The value of security key ID, as received from the peer in the most recent APDU. 
   /// The property behaves like  \refprop{GetIncomingInitializationVector, IncomingInitializationVector}, except that zero 
   /// is a valid incoming value. To know whether or not \refprop{GetIncomingSecurityKeyId, IncomingSecurityKeyId}
   /// was obtained from peer, one will need to check \refprop{GetIncomingInitializationVector, IncomingInitializationVector} for being nonzero.
   ///
   /// \since MeteringSDK Version 4.0.70.
   ///
   unsigned GetIncomingSecurityKeyId() const
   {
      return m_incomingSecurityKeyId;
   }

   /// The value of initialization vector, as received from the peer in the most recent APDU.
   /// If IncomingSecurityMode is nonzero, and initialization vector ACSE was not sent by the peer, 
   /// or if IncomingInitializationVector is zero, MCOM is going to unconditionally throw an error. 
   /// Therefore, zero value of this property will mean the incoming packet's security is clear text, 
   /// and the initialization vector was not obtained.
   ///
   /// \since MeteringSDK Version 4.0.70.
   ///
   unsigned GetIncomingInitializationVector() const
   {
      return m_incomingInitializationVector;
   }

   /// The value of invocation ID received from the peer in the most recent APDU.
   /// If there was no APDU exchanged, the property will return zero. 
   /// MCOM will unconditionally throw an exception if the incoming APDU has no invocation ID, 
   /// and zero is a valid invocation id. Therefore, the presence of the actual value 
   /// can be detected by the fact that the communication succeeded at the ACSE level.
   ///
   /// \since MeteringSDK Version 4.0.70.
   ///
   unsigned GetIncomingCallingApInvocationId() const
   {
      return m_incomingCallingApInvocationId;
   }

#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST

   ///@{
   /// Security key list for the protocol.
   ///
   /// The security key lists overwrite the security key property, when given.
   /// These are used to try multiple keys through a single connection,
   /// which is, no doubt, a too nice security invention.
   ///
   const MByteStringVector& GetSecurityKeyList() const
   {
      return m_securityKeyList;
   }
   void SetSecurityKeyList(const MByteStringVector&);
   ///@}

   /// Return the entry which was successfully tried with the meter.
   ///
   int GetSecurityKeyListSuccessfulEntry() const
   {
      return m_securityKeyListSuccessfulEntry;
   }

#endif // !M_NO_MCOM_PASSWORD_AND_KEY_LIST


public:

   /// Start processing of the incoming APDU.
   ///
   /// Accept the whole incoming APDU and parse all its ACSE headers.
   /// This call can be used by software for implementing C12.22 server.
   ///
   /// Initialize \refprop{GetIncomingApdu,IncomingApdu} with the whole contents of what is read.
   ///
   /// All other Incoming properties are initialized, 
   /// and EPSEM part is loaded into the protocol, available for processing.
   ///
   void ServerStart();
   
   /// Reset server so that it is ready to send back the outgoing APDU.
   ///
   /// Outgoing EPSEM is cleared, ready to be filled.
   /// AP titles are initialized:
   /// calling AP Title will be set to be equal to the incoming called AP title,
   /// and called AP title will be set to the incoming calling AP title.
   ///
   void ServerReset();
   
   /// Initialize protocol machine for processing the incoming EPSEM.
   ///
   /// Do all necessary checking such as security mode check,
   /// and reset EPSEM buffer for parsing.
   ///
   void ProcessIncomingEPSEM();
   
   /// End the processing of incoming request by making and sending back the response.
   ///
   /// This call does SendEnd inside.
   ///
   void ServerEnd();

   /// Start making the outgoing EPSEM packet.
   ///
   void SendStart();
   
   /// Finish making the outgoing EPSEM packet and send it out to client.
   ///
   void SendEnd();
   
   /// Combines SendEnd and ReceiveStart.
   ///
   /// \see \ref SendEnd
   /// \see \ref ReceiveStart
   ///
   bool SendEndReceiveStart();

   /// Read raw incoming APDU.
   ///
   /// Serial variant of the protocol processes the necessary 
   /// link layer wrapping.
   ///
   virtual void ReadApdu();

   /// Write raw incoming APDU.
   ///
   /// Serial variant of the protocol processes the necessary 
   /// link layer wrapping.
   ///
   void WriteApdu(const MByteString& buffer);

   /// Start parsing the incoming EPSEM.
   ///
   void ReceiveStart();

   /// End parsing the incoming EPSEM.
   ///
   void ReceiveEnd();

   /// Send command-only service.
   ///
   /// This call should be made within SendStart/SendEnd pair of calls.
   ///
   /// \param command
   ///     C12 command or status code that has to be sent.
   ///
   void SendService(Muint8 command);

   /// Send service with command and data.
   ///
   /// This call should be made within SendStart/SendEnd pair of calls.
   ///
   /// \param command
   ///     C12 command or status code that has to be sent.
   /// \param data
   ///     Data associated with the command.
   ///
   void SendServiceWithData(Muint8 command, const MByteString& data);

   /// Receive service length.
   ///
   /// This call should be made within ReceiveStart/ReceiveEnd pair of calls.
   ///
   /// \return length of the incoming PSEM
   ///
   unsigned ReceiveServiceLength();
   
   /// Receive and ignore service length, then service code or status. 
   ///
   /// This call should be made within ReceiveStart/ReceiveEnd pair of calls.
   /// Length parameter, not made available to the caller, is used for initialization of application layer buffer.
   ///
   /// \return Service code or status code.
   ///
   Muint8 ReceiveServiceCodeIgnoreLength();

   ///@{
   /// Process PSEM with C12 Logon service.
   ///
   void SendLogon();
   void ReceiveLogon();
   ///@}

   /// Perform ANSI C12 Logon request, both request and response.
   ///
   /// The particular implementation of this abstract service is 
   /// typically called from StartSession, but it can also be called 
   /// directly by the user for testing purpose.
   ///
   /// \pre The channel is open, and the protocol state allows
   /// Logon to be called.
   ///
   virtual void Logon();

   ///@{
   /// Process PSEM with C12 Security service.
   ///
   void SendSecurity();
   void ReceiveSecurity();
   ///@}

   ///@{
   /// Process PSEM with C12 table read service.
   ///
   void        SendTableRead   (MCOMNumberConstRef number);
   MByteString ReceiveTableRead(MCOMNumberConstRef number);
   ///@}

   ///@{
   /// Process PSEM with C12 table write service.
   ///
   void SendTableWrite   (MCOMNumberConstRef number, const MByteString& request);
   void ReceiveTableWrite(MCOMNumberConstRef number, const MByteString& request);
   ///@}

   ///@{
   /// Process PSEM with C12 partial table read service.
   ///
   void        SendTableReadPartial   (MCOMNumberConstRef number, int offset, int length);
   MByteString ReceiveTableReadPartial(MCOMNumberConstRef number, int offset, int length);
   ///@}

   ///@{
   /// Process PSEM with C12 partial table write service.
   ///
   void SendTableWritePartial   (MCOMNumberConstRef number, const MByteString& request, int offset);
   void ReceiveTableWritePartial(MCOMNumberConstRef number, const MByteString& request, int offset);
   ///@}

   ///@{
   /// Process PSEM with C12 function (procedure) execute service, no request or response.
   ///
   void FunctionExecuteSend   (MCOMNumberConstRef number);
   void FunctionExecuteReceive(MCOMNumberConstRef number);
   ///@}

   ///@{
   /// Process PSEM with C12 function (procedure) execute service, request is present.
   ///
   void FunctionExecuteRequestSend   (MCOMNumberConstRef number, const MByteString& request);
   void FunctionExecuteRequestReceive(MCOMNumberConstRef number, const MByteString& request);
   ///@}

   ///@{
   /// Process PSEM with C12 function (procedure) execute service, response is present.
   ///
   void        FunctionExecuteResponseSend   (MCOMNumberConstRef number);
   MByteString FunctionExecuteResponseReceive(MCOMNumberConstRef number);
   ///@}

   ///@{
   /// Process PSEM with C12 function (procedure) execute service, both request and response are present.
   ///
   void        FunctionExecuteRequestResponseSend   (MCOMNumberConstRef number, const MByteString& request);
   MByteString FunctionExecuteRequestResponseReceive(MCOMNumberConstRef number, const MByteString& request);
   ///@}

   /// Setup the configuration of the channel according to C12.22 protocol handshake settings.
   ///
   virtual void ApplyChannelParameters();

protected: // Services:
/// \cond SHOW_INTERNAL


   virtual void DoWriteApdu();

#if !M_NO_MCOM_IDENTIFY_METER
   virtual MStdString DoIdentifyMeter(bool sessionIsStarted, TableRawDataVector* tablesRead);
#endif
   void        DoFunctionSend(MCOMNumberConstRef number, const MByteString& request, bool expectResponse);
   MByteString DoFunctionReceive(MCOMNumberConstRef number, const MByteString& request, bool expectResponse);

   void DoReceiveStartHeader();
   void DoParseStartHeader();

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

   // Send StartSession.
   // This method is used in asynchronous communication in session mode,
   // when OneServicePerApdu parameter is set to True.
   //
   // To receive response message, use DoReceiveStartSession method.
   // 
   // \pre The channel is open, the protocol state allows starting the session.
   // Otherwise exceptions can be thrown.
   //
   virtual void DoSendStartSession();

   // Receive the response to DoSendStartSession.
   // This method is used in asynchronous communication in session mode,
   // when OneServicePerApdu parameter is set to True.
   //
   // It can be used only after DoSendStartSession is performed.
   // 
   // \pre The channel is open, the protocol state allows starting the session.
   // DoSendStartSession is performed.
   // Otherwise exceptions can be thrown.
   //
   virtual void DoReceiveStartSession();

   // Send EndSession.
   // This method is used in asynchronous communication in session mode,
   // when OneServicePerApdu parameter is set to True.
   // 
   // To receive response message, use DoReceiveEndSession method.
   //
   // \pre The channel is open, the protocol state allows ending the session.
   // Otherwise exceptions can be thrown.
   //
   virtual void DoSendEndSession();

   // Receive the response to DoSendEndSession.
   // This method is used in asynchronous communication in session mode,
   // when OneServicePerApdu parameter is set to True.
   //
   // It can be used only after DoSendEndSession is performed.
   // 
   // \pre The channel is open, the protocol state allows ending the session.
   // DoSendEndSession is performed.
   // Otherwise exceptions can be thrown.
   //
   virtual void DoReceiveEndSession();

   // Synchronously end the session.
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
   virtual void DoEndSession();

   // Perform a single C12.22 procedure, which results in writing to table 7 and possibly reading
   // from table 8. Multiple application layer retries can be performed.
   // If the response is not expected, the response table might not be read, depending on configuration and table number.
   //
   // \pre The meter allows the procedure with the specified number and
   // specified request. The protocol state allows the request. The channel is functioning.
   // Otherwise the exception is thrown.
   //
   virtual void DoMeterProcedure(unsigned number, const MByteString &request, MByteString &response, bool expectResponse);

   // Perform the full application layer request, abstract service.
   // The command will be the first byte in the packet, while the request parameter,
   // if present, will form the rest of the packet.
   //
   // \pre The request corresponds to the correct C12 service packet.
   // The channel is functioning, the state allows such request.
   // Otherwise the exception is thrown.
   //
   virtual void DoApplicationLayerRequest(char command, const MByteString *request = NULL, unsigned flags = APPLICATIONLAYERREQUEST_NO_FLAGS);
   void DoApplicationLayerRequestWithCurrentPassword(char command, const MByteString *request, unsigned flags);
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   void DoApplicationLayerRequestIteratePasswordList(char command, const MByteString *request, unsigned flags);
#endif

   // Implementation of actual Commit synchronous sequence, minus all possible checks and balances.
   // This particular flavor executes the queue in the way efficient for C12.22.
   //
   // \pre Shall be called from QCommit.
   //
   virtual void DoQCommit();

   void DoQCommitWithCurrentPassword();
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   void DoQCommitIteratePasswordList();
#endif
#if !M_NO_PROGRESS_MONITOR
   void DoQCommitSubrange(MCommunicationQueue::iterator& start, MCommunicationQueue::iterator end, MProgressAction* parentAction, double& localActionWeight);
   void DoQCommitAtomicQueue(MCommunicationQueue& q, MProgressAction* action, double progress);
#else
   void DoQCommitSubrange(MCommunicationQueue::iterator& start, MCommunicationQueue::iterator end);
   void DoQCommitAtomicQueue(MCommunicationQueue& q);
#endif
   void DoResetNegotiatedMaximumApduSizes();
   void DoResetSessionSpecificProperties();
   void DoResetIncomingProperties();

#if !M_NO_MCOM_MONITOR
   void DoSendACSEToMonitor(MConstChars elementName, char elementCode, const MStdString& value);
   void DoSendACSEToMonitor(MConstChars elementName, char elementCode, unsigned value);
   void DoSendACSECallingAuthenticationToMonitor(unsigned key, unsigned vect);
   void DoSendEpsemToMonitor(const char* epsem, unsigned epsemSize);
#endif

   void DoAppendAbsoluteUidIfPresent(MBuffer& acse, char elementCode, const MStdString& base, const MStdString& id);
   void DoAppendCallingInvocation(MBuffer& acse, unsigned keyId, Muint32 initializationVector);

   void DoGetUid(MConstChars elementName, char elementCode, MStdString& id);
   unsigned DoGetInteger(MConstChars elementName, char elementCode);

   static M_NORETURN_FUNC void DoThrowBadACSEResponse(char acse);
   void DoCheckNotOneWay(MConstChars whatOperation);

   void DoInitializeEax(const MStdString& apTitle);
   virtual void DoTryPasswordEntry(const MByteString& entry);
   void DoUpdateCallingApInvocationId(bool ignoreSessionless);

   virtual void DoTableRead(MCOMNumberConstRef number, MByteString& data, unsigned expectedSize = 0);
   virtual void DoTableWrite(MCOMNumberConstRef number, const MByteString& data);
   virtual void DoTableReadPartial(MCOMNumberConstRef number, MByteString& data, int offset, int length);
   virtual void DoTableWritePartial(MCOMNumberConstRef number, const MByteString& data, int offset);

   void DoRethrowIfNotProperRqtlRstl(MEC12NokResponse& ex, unsigned applicationRetry);

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

   // Maximum possible size of APDU header (everything but data)
   // 
   // 
   int DoGetMaximumApduHeaderSize() const
   {
      if ( m_effectiveMaximumApduSizeIncoming > 30000 && m_effectiveMaximumApduSizeOutgoing > 30000 )
         return MaximumLegacyApduHeaderSize; // work around imprecise buffer computation in some devices
      else
         return MaximumProperApduHeaderSize;
   }

protected: // Attributes:

   // Whether the protocol mode is sessionless, meaning start and end session requests are ignored
   //
   bool m_sessionless;

   // Whether the non-sessionless protocol should communicate using only one packet
   // as if it is in sessionless mode
   //
   bool m_oneServicePerApdu;

   // Whether the protocol mode is one way
   //
   ResponseControlEnum m_responseControl;

   bool m_issueTerminateOnEndSession;

   // Session idle timeout, as given to Logon service
   //
   unsigned m_sessionIdleTimeout;

   // Whether the negotiated session idle timeout is present.
   //
   bool m_negotiatedSessionIdleTimeoutPresent;

   int m_callingAeQualifier;

   // Security mode of C12.22
   //
   SecurityModeEnum m_securityMode;


   // Encryption processor for cipher-related modes
   //
   MAesEax m_eax;

   // Security key
   //
   MByteString m_securityKey;

   // Security key, case when security mode is SecurityClearTextWithAuthentication, SecurityCipherTextWithAuthentication, SecurityCompressedCipherTextWithAuthentication
   //
   int m_securityKeyId;

   // Negotiated session idle timeout, as returned by Logon service
   //
   unsigned m_negotiatedSessionIdleTimeout;

   // Timeout to wait for meter response
   //
   unsigned m_responseTimeout;

   // Application context element
   //
   MStdString m_applicationContext;

   // The calling AppTitile uses in CALLING_APTITLE_ELEMENT
   //
   MStdString m_callingApTitle;

   unsigned m_initializationVector;
   unsigned m_callingApInvocationId;

   bool m_initializationVectorSetByUser;
   bool m_callingApInvocationIdSetByUser;

   // The called AppTitile uses in CALLED_APTITLE_ELEMENT
   //
   MStdString m_calledApTitle;

   // ED_CLASS used in the EPSEM request.
   //
   MStdString m_edClass;

   MBuffer m_canonifiedCleartext;

   MBufferBidirectional m_outgoingApdu;

   // Whole incoming APDU, including EPSEM
   //
   MBuffer m_incomingApdu;

#if !M_NO_MCOM_MONITOR
   MChar m_logHeaderChar;
#endif

   unsigned m_maximumApduSizeIncoming;
   unsigned m_maximumApduSizeOutgoing;

   // The negotiated value of the incoming maximum APDU size. 
   // This value is equal to its persistent equivalent in the beginning,
   // and it can change later during communication in the event of receiving RSTL C12 Nok response. 
   //
   unsigned m_negotiatedMaximumApduSizeIncoming;

   // The negotiated value of the outgoing maximum APDU size. 
   // This value is equal to its persistent equivalent in the beginning,
   // and it can change later during communication in the event of receiving RQTL C12 Nok response. 
   //
   unsigned m_negotiatedMaximumApduSizeOutgoing;

   // The incoming maximum APDU size value that is composed of all constraints, used for packet breakage.
   // This value can be different from negotiated value.
   //
   unsigned m_effectiveMaximumApduSizeIncoming;

   // The outgoing maximum APDU size value that is composed of all constraints, used for packet breakage.
   // This value can be different from negotiated value.
   //
   unsigned m_effectiveMaximumApduSizeOutgoing;

   MStdString          m_incomingEdClass;
   ResponseControlEnum m_incomingResponseControl;
   MStdString          m_incomingApplicationContext;
   MStdString          m_incomingCalledApTitle;
   MStdString          m_incomingCallingApTitle;
   Muint32             m_incomingCalledApInvocationId;
   Muint32             m_incomingCallingApInvocationId;
   int                 m_incomingSecurityKeyId;
   Muint32             m_incomingInitializationVector;
   bool                m_incomingCalledApInvocationIdPresent;
   bool                m_incomingCallingApInvocationIdPresent;
   bool                m_securityKeyIdAndInitializationVectorWereSent;
   bool                m_securityKeyIdAndInitializationVectorWereReceived;
   SecurityModeEnum    m_incomingSecurityMode;
   int                 m_incomingCallingAeQualifier;
   unsigned            m_incomingEpsemSize;
   Muint8              m_incomingEpsemControl;
   MProtocol*          m_wrapperProtocol;

#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST

   // Protocol application level password list. Takes rule over m_password, if not empty.
   //
   MByteStringVector m_securityKeyList;

   // The successful entry of the password list,
   // valid only after the password from the password list is successfully checked.
   //
   int m_securityKeyListSuccessfulEntry;

#endif

   M_DECLARE_CLASS(ProtocolC1222)

/// \endcond SHOW_INTERNAL
};

#endif // !M_NO_MCOM_PROTOCOL_C1222

///@}
#endif
