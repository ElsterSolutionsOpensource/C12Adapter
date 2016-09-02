#ifndef MCOM_PROTOCOLC12_H
#define MCOM_PROTOCOLC12_H
/// \addtogroup MCOM
///@{
/// \file MCOM/ProtocolC12.h

#include <MCOM/Buffer.h>
#include <MCOM/Protocol.h>
#include <MCOM/MCOMExceptions.h>

#if !M_NO_MCOM_PROTOCOL_C1218 || !M_NO_MCOM_PROTOCOL_C1221 || !M_NO_MCOM_PROTOCOL_C1222

/// Generic ANSI C12 abstract protocol, base for C12.18, C12.21, and C12.22.
///
class MCOM_CLASS MProtocolC12 : public MProtocol
{

public: // Constants

   /// Logic of when ST8 has to be read during execution of function.
   ///
   /// \seeprop{GetAlwaysReadFunctionResponse,AlwaysReadFunctionResponse} - property where this enumeration is used.
   ///
   enum ReadFunctionResponseEnum
   {
      /// Read function response only when response is present.
      ///
      ReadFunctionResponseWhenPresent = 0,

      /// Avoid reading function response in special cases.
      ///
      /// The behavior of this legacy flag is the same as \ref ReadFunctionResponseAlways.
      ///
      ReadFunctionResponseWhenDesired = 1,

      /// Fully compliant ANSI behavior, always read function response.
      ///
      ReadFunctionResponseAlways = 2
   };

   enum
   {

      READ_SERVICE_OVERHEAD           = 4, ///< Comprises of: ok8 count16 data chksum8
      WRITE_SERVICE_OVERHEAD          = 7, ///< Comprises of: 0x40 tableid16 count16 data chksum8
      PARTIAL_WRITE_SERVICE_OVERHEAD  = 9  ///< Comprises of: 0x4F tableid16 offset24 count16 data chksum8
   };

/// \cond SHOW_INTERNAL

   /// Flags for DoApplicationLayerRequest
   ///
   enum DoApplicationLayerRequestFlags
   {
      APPLICATIONLAYERREQUEST_NO_FLAGS        = 0, // Default behavior
   };

/// \endcond SHOW_INTERNAL
protected: // Constructor:

   /// Create a new abstract ANSI C12 protocol with the channel given.
   ///
   /// The service is protected because the class is abstract.
   ///
   /// \param channel Channel object of the protocol.
   ///     It will be a rare case when a protocol can be created without a channel,
   ///     in which case this parameter can be NULL.
   /// \param channelIsOwned Whether the channel has to be owned by the protocol,
   ///     deleted in protocol destructor or at channel reassignment.
   ///     By default the protocol owns its channel.
   ///
   MProtocolC12(MChannel* channel, bool channelIsOwned = true);

public: // Destructor:

   /// Destroy the C12 protocol object.
   ///
   /// If the channel is owned, \ref MProtocol.IsChannelOwned is true, the channel is also destroyed.
   ///
   virtual ~MProtocolC12();

public: // Synchronous abstract services:

   /// Setup the configuration of the channel in a way compatible with ANSI C12 protocol handshake sequence.
   ///
   /// The action depends on the channel and particular ANSI C12 protocol. Implementations
   /// ensure that the service behaves gracefully whether
   /// the channel is connected or not.
   ///
   virtual void ApplyChannelParameters();

protected: // Services:
/// \cond SHOW_INTERNAL

   // Synchronously read the whole table with number given as parameter.
   // This base class service is overloaded to fulfill the duties
   // relevant to this particular protocol.
   //
   // See QTableRead and TableRead of MProtocol for public variations of this
   // service, which also do necessary steps for monitoring, counting
   // statistics and formatting error messages.
   //
   // \pre The channel is open, the session is started.
   // The number is convertible to the unsigned integer two bytes long.
   // Otherwise an exception can be thrown.
   //
   virtual void DoTableRead(MCOMNumberConstRef number, MByteString& data, unsigned expectedSize = 0);

   // Synchronously write the whole table with number given as parameter.
   // This base class service is overloaded to fulfill the duties
   // relevant to this particular protocol.
   //
   // See QTableWrite and TableWrite of MProtocol for public variations of this
   // service, which also do necessary steps for monitoring, counting
   // statistics and formatting error messages.
   //
   // \pre The channel is open, the session is started.
   // The number is convertible to the unsigned integer two bytes long.
   // Otherwise an exception can be thrown.
   //
   virtual void DoTableWrite(MCOMNumberConstRef number, const MByteString& data);

   // Synchronously read part of the table with number given as parameter.
   // This base class service is overloaded to fulfill the duties
   // relevant to this particular protocol.
   //
   // See QTableReadPartial and TableReadPartial of MProtocol for public variations of this
   // service, which also do necessary steps for monitoring, counting
   // statistics and formatting error messages.
   //
   // \pre The channel is open, the session is started.
   // The number is convertible to the unsigned integer two bytes long.
   // Otherwise an exception can be thrown.
   //
   virtual void DoTableReadPartial(MCOMNumberConstRef number, MByteString& data, int offset, int length);

   // Synchronously write part of the table with number given as parameter.
   // This base class service is overloaded to fulfill the duties
   // relevant to this particular protocol.
   //
   // See QTableWritePartial and TableWritePartial of MProtocol for public variations of this
   // service, which also do necessary steps for monitoring, counting
   // statistics and formatting error messages.
   //
   // \pre The channel is open, the session is started.
   // The number is convertible to the unsigned integer two bytes long.
   // Otherwise an exception can be thrown.
   //
   virtual void DoTableWritePartial(MCOMNumberConstRef number, const MByteString& data, int offset);

/// \endcond SHOW_INTERNAL
public: // I2C public services:


protected: // Services:
/// \cond SHOW_INTERNAL

   // Synchronously execute the function with no parameters,
   // the number of the function is given as parameter.
   // This base class service is overloaded to fulfill the duties
   // relevant to this particular protocol.
   //
   // See QFunctionExecute and FunctionExecute of MProtocol for public variations of this
   // service, which do the necessary steps for monitoring, counting statistics, 
   // and formatting error messages.
   //
   // \pre The channel is open, the session is started.
   // The function number is convertible to two-byte unsigned integer.
   // Otherwise an exception can be thrown.
   //
   virtual void DoFunctionExecute(MCOMNumberConstRef number);

   // Synchronously execute the function with request data,
   // the number of the function is given as parameter.
   // This base class service is overloaded to fulfill the duties
   // relevant to this particular protocol.
   //
   // See QFunctionExecuteRequest and FunctionExecuteRequest of MProtocol 
   // for public variations of this service, which also do necessary 
   // steps for monitoring, counting statistics, and formatting error messages.
   //
   // \pre The channel is open, the session is started.
   // The function number is convertible to two-byte unsigned integer.
   // Otherwise exceptions can be thrown.
   //
   virtual void DoFunctionExecuteRequest(MCOMNumberConstRef number, const MByteString& request);

   // Synchronously execute the function with response data,
   // the number of the function is given as parameter.
   // This base class service is overloaded to fulfill the duties
   // relevant to this particular protocol.
   //
   // See QFunctionExecuteResponse and FunctionExecuteResponse of MProtocol 
   // for public variations of this service, which do the necessary 
   // steps for monitoring, counting statistics, and formatting error messages.
   //
   // \pre The channel is open, the session is started.
   // The function number is convertible to two-byte unsigned integer.
   // Otherwise an exception can be thrown.
   //
   virtual void DoFunctionExecuteResponse(MCOMNumberConstRef number, MByteString& response);

   // Synchronously execute the function with request and response data,
   // the number of the function is given as parameter.
   // This base class service is overloaded to fulfill the duties
   // relevant to this particular protocol.
   //
   // See QFunctionExecuteRequestResponse and FunctionExecuteRequestResponse of MProtocol 
   // for public variations of this service, which do the necessary 
   // steps for monitoring, counting statistics, and formatting error messages.
   //
   // \pre The channel is open, the session is started.
   // The function number is convertible to two-byte unsigned integer.
   // Otherwise an exception can be thrown.
   //
   virtual void DoFunctionExecuteRequestResponse(MCOMNumberConstRef number, const MByteString& request, MByteString& response);


   bool DoHaveToSkipReadFunctionResponseTable8(unsigned num, const MByteString& request, bool expectResponse);
   void DoHandleFunctionResponseTable8Read(MByteString& response);

   void DoReadFunctionResponse(MByteString& response);

/// \endcond SHOW_INTERNAL
public: // Services:


public: // Specific C12 commands:

   /// Perform ANSI C12 Logon service.
   ///
   /// This standard service is part of \ref MProtocol.StartSession(),
   /// but it can also be called directly by the user for any purpose such as testing,
   /// or custom implementation of Start Session sequence.
   ///
   /// \pre The channel is open, and the protocol state allows Logon to be called.
   ///
   virtual void Logon();

   /// Perform ANSI C12 Security service.
   ///
   /// This standard service is part of \ref MProtocol.StartSession(),
   /// but it can also be called directly by the user for any purpose such as testing,
   /// or custom implementation of Start Session sequence.
   ///
   /// \pre The channel is open, and the protocol state allows Logon to be called.
   ///
   virtual void Security();


   /// Perform whatever is required by the protocol to clear security with the meter.
   ///
   /// This can call ANSI C12 \ref Security request, or in case of C12.21 Authenticate request.
   ///
   /// \pre The channel is open, and the protocol state allows
   /// Security or Authenticate to be called.
   ///
   virtual void FullLogin();

   /// Perform ANSI C12 Logoff service.
   ///
   /// This standard service is part of \ref MProtocol.StartSession(),
   /// but it can also be called directly by the user for any purpose such as testing,
   /// or custom implementation of Start Session sequence.
   ///
   /// \pre The channel is open, and the protocol state allows Logoff to be called.
   ///
   virtual void Logoff();

   /// Perform ANSI C12 Wait service.
   ///
   /// ANSI C12 Wait extends the session to the given number of seconds,
   /// so in case of inactivity of the communication the session does not time out.
   ///
   /// \pre The channel is open, and the protocol state allows
   /// Wait to be called. Seconds are no bigger than 255.
   ///
   virtual void Wait(unsigned seconds);

   /// Perform ANSI C12 Terminate service.
   ///
   /// This standard service is part of \ref MProtocol.EndSession(),
   /// but it can also be called directly by the user for any purpose such as testing,
   /// or custom implementation of End Session sequence.
   ///
   /// \pre The channel is open, and the protocol state allows Terminate to be called.
   ///
   virtual void Terminate();

   /// Two-byte CRC calculation procedure specific to C12 protocol.
   ///
   /// The algorithm itself uses a set of shifts and XOR operators for efficiency.
   /// When executing on Intel, or other Little Endian architectures,
   /// the bytes are swapped after polynom calculation. 
   ///
   virtual Muint16 CalculateCRC16FromBuffer(const char* buffer, unsigned length) const;

public: // Property manipulators:


   ///@{
   /// Number of link layer retries.
   ///
   /// The link layer retry takes place if an error occurs in the data transfer.
   /// For protocols that break application layer into link layer packets, such as C12.18, C12.21, and serial C12.22,
   /// this is the number of times to re-send a packet after a link layer NAK (negative-acknowledge) or
   /// timeout before giving up. The most common causes are incorrect CRC or noise in the line
   /// (data lost on the line, extra garbage characters, framing errors, bits tweaked, etc.)
   ///
   /// \ifnot M_NO_MCOM_CHANNEL_SOCKET_UDP
   /// For C12.22 protocol, this property only applies when the channel is of type \ref MChannelSocketUdp,
   /// in which case this is the number of resends of the outgoing UDP packet.
   /// \else
   /// This property has no effect for some child protocols
   /// \endif
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 3
   ///
   unsigned GetLinkLayerRetries() const
   {
      return m_linkLayerRetries;
   }
   void SetLinkLayerRetries(unsigned retries)
   {
      m_linkLayerRetries = retries;
   }
   ///@}

   ///@{
   /// Whether the start session will include the security service or the authenticate service.
   ///
   /// When it is set to False, neither the security nor the authenticate service is sent,
   /// allowing data that is not password protected to be accessed without the password.
   /// 
   /// MeteringSDK has a check prior to sending the authenticate
   /// service. When an identify service is sent (always the first service sent in a start session 
   /// sequence), the device responds with what it supports. If it says that it does not support 
   /// authentication, then MeteringSDK aborts the start session sequence and throws the error 
   /// "Meter does not support authentication". 
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
   bool GetIssueSecurityOnStartSession() const
   {
      return m_issueSecurityOnStartSession;
   }
   virtual void SetIssueSecurityOnStartSession(bool);
   ///@}

   ///@{
   /// Determines whether or not ST-8 will be read if the function does not have a response.
   ///
   /// This property should be true for most ANSI meters. However, for some option boards installed on
   /// an ANSI meter, this property needs to be set to false.
   ///
   /// \since MeteringSDK Version 2.1.27. However, the type changed from Boolean 
   ///  to INT32 in MeteringSDK Version 5.4.0. All versions can accept the value as Boolean or INT32, 
   ///  so no changes are needed to existing applications built using older versions.
   ///
   /// \default_value 
   ///  - 2 for MeteringSDK Versions 5.4.0 and later. 
   ///  - 1 {True} for MeteringSDK Versions prior to 5.4.0. 
   ///
   /// \possible_values
   ///  - False, or 0       : ST-8 is NOT read when the function does not have a response. This behavior is 
   ///                        available for all versions. Applications that set the value to 0, or False, 
   ///                        will behave exactly the same in any version.
   ///  - True, 1, 2, or -1 :  ST-8 is read for ALL functions. 
   ///
   ReadFunctionResponseEnum GetAlwaysReadFunctionResponse() const
   {
      return m_alwaysReadFunctionResponse;
   }
   void SetAlwaysReadFunctionResponse(ReadFunctionResponseEnum value);
   ///@}

   ///@{
   /// Whether to use partial reads and writes in place of full reads and writes.
   ///
   /// Standard table 7 write and standard table 8 write, the sequence that forms function execution, 
   /// and are always performed in full, regardless of the value of this property.
   /// Also, full reads and full writes have to always be made through partial operations
   /// when full table size would not fit into a single application layer request.
   ///
   /// \since MeteringSDK Version 5.2.0.1720.
   ///
   /// \default_value
   /// \ifnot M_NO_MCOM_PROTOCOL_C1218
   ///  - False [0] for MProtocolC1218.
   /// \endif
   /// \ifnot M_NO_MCOM_PROTOCOL_C1221
   ///  - False [0] for MProtocolC1221.
   /// \endif
   /// \ifnot M_NO_MCOM_PROTOCOL_C1222
   ///  - False [0] for MProtocolC1222.
   /// \endif
   ///
   /// \possible_values
   ///  - False : perform full table reads and writes if they fit in the application layer.
   ///  - True  : replace full table reads and writes with partial operations for all tables except ST7 and ST8.
   ///
   bool GetAlwaysUsePartial() const
   {
      return m_alwaysUsePartial;
   }
   void SetAlwaysUsePartial(bool yes)
   {
      m_alwaysUsePartial = yes;
   }
   ///@}

#if !M_NO_MCOM_KEEP_SESSION_ALIVE
   ///@{
   /// Whether to use table read for session keeping instead of C12 Wait.
   /// This property has no effect if KEEP_SESSION_ALIVE is False.
   ///
   /// \since MeteringSDK Version 4.0.54.
   ///
   /// \default_value False
   ///
   /// \possible_values
   ///  - True [1]  : The first byte of ST1 will be read to keep the session alive.
   ///  - False [0] : The C12 Wait service will be used to keep the session alive.
   ///
   bool GetUseReadInKeepSessionAlive() const
   {
      return m_useReadInKeepSessionAlive;
   }
   void SetUseReadInKeepSessionAlive(bool yes)
   {
      m_useReadInKeepSessionAlive = yes;
   }
   ///@}
#endif

   ///@{
   /// User identity number reported to device.
   ///
   /// User ID is used in logon services of all variants of ANSI C12, 
   /// and in sessionless mode Security service of ANSI C12.22.
   /// The User ID as defined by the ANSI C12 standard is a code indicating a utility supplied identity 
   /// of the operator requesting the creation of the session. 
   /// If the metering device supports Events and History logs, it has the option of
   /// storing the User ID as defined in the "Utility Industry End Device Data Tables" document.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 0
   ///
   /// \possible_values 0 .. 65535
   ///
   unsigned GetUserId() const
   {
      return m_userId;
   }
   void SetUserId(unsigned userId);
   ///@}

   ///@{
   /// Get the user name that will be used during logon service of the protocol.
   ///
   /// Value used in the "user" parameter of the ANSI C12 logon service. The logon service is
   /// part of QStartSession. The User as defined by the ANSI C12 standard is a utility supplied
   /// name of the operator requesting the access to the device.
   ///
   /// The User property can have a maximum of 10 characters. If the User property is specified
   /// with less than 10 characters, then during communications the remaining characters are
   /// filled with blank spaces. For example, if User is specified as "METER1", then "METER1 " is
   /// used in the user field of the logon service, but the property User reports "METER1".
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 10 characters with binary zero values.
   ///
   /// \possible_values
   ///  - Any string containing 10 or less bytes, i.e. "1234567890".
   ///
   const MByteString& GetUser() const
   {
      return m_user;
   }
   void SetUser(const MByteString& userName);
   ///@}

   ///@{
   /// The number of milliseconds to wait before the computer sends data to the meter on the link layer.
   ///
   /// The data may be an ACK or NAK to a packet received from the meter or it may be the
   /// next request to send to the meter. The delay is required so that the UART and firmware in
   /// the meter has time to switch from transmit to receive and to process the last transmission
   /// it received. If the data is sent to too soon, the meter may not receive it, which results
   /// in communication failures and retry attempts. The actual amount of delay will depend on
   /// the computer and the number of tasks running (for example, the Sleep method causes a task
   /// switch). Faster machines will approach the specified time more closely.
   /// \code
   ///    0.01 StartSession
   ///    0.12 Identify
   ///    0.17 Tx> EE 00 00 00 00 01 20 13 10
   ///    0.20 Rx> 06 EE 00 00 00 00 05 00 00 01 00 00 C6 B5
   ///       // Turn around delay, approximately 60 msec before
   ///       // ACK (0x06) transmitted to the meter
   ///    0.26 Tx> 06
   ///    0.26 Identify success
   ///       // Turn around delay, approximately 60 msec before
   ///       // next request (Negotiate) transmitted to the meter
   ///    0.26 Negotiate
   ///    0.32 Tx> EE 00 20 00 00 05 61 04 00 80 06 C2 29
   ///    0.35 Rx> 06 EE 00 20 00 00 05 00 04 00 80 06 35 83
   ///    0.41 Tx> 06
   ///    0.41 Negotiate success
   /// \endcode
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value
   /// \ifnot M_NO_MCOM_PROTOCOL_C1218
   ///  - 20 milliseconds for MProtocolC1218.
   /// \endif
   /// \ifnot M_NO_MCOM_PROTOCOL_C1221
   ///  - 20 milliseconds for MProtocolC1221.
   /// \endif
   /// \ifnot M_NO_MCOM_PROTOCOL_C1222
   ///  - 20 milliseconds for MProtocolC1222.
   /// \endif
   ///
   /// \possible_values
   ///  - 1 .. MAXINT in milliseconds.
   ///
   unsigned GetTurnAroundDelay() const
   { 
      return m_turnAroundDelay;
   }
   void SetTurnAroundDelay(unsigned delay)
   { 
      m_turnAroundDelay = delay;
   }
   ///@}

   ///@{
   /// Get the number of application layer retries for tables.
   /// The application layer table request is retried only in the case the meter is busy, 
   /// or the data are not available.
   ///
   /// Number of times to retry on the application layer table request when the request has been
   /// successfully transmitted to the meter, but the meter responded that it cannot service the
   /// request. The most common meter responses that trigger an application retry are the meter
   /// is busy BSY or the data is not ready DNR.
   ///
   /// The typical value for ApplicationLayerRetries is a large number (like 20) as the
   /// communications are okay and the meter is sending a meaningful response. Set the value to
   /// something small (like 0) to force the operation to fail when the meter is busy or the data
   /// is not ready.
   ///
   /// ApplicationLayerRetries is different from LinkLayerRetries which specifies how many times
   /// to attempt sending a link layer packet (the link layer handles the data transfer) and is
   /// different from ApplicationLayerProcedureRetries which specifies the application retries
   /// when the request is a procedure.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 20
   ///
   /// \seeprop{GetApplicationLayerProcedureRetries,ApplicationLayerProcedureRetries} that works for procedure retries.
   ///
   unsigned GetApplicationLayerRetries() const
   { 
      return m_applicationLayerRetries;
   }
   void SetApplicationLayerRetries(unsigned retries)
   { 
      m_applicationLayerRetries = retries;
   }
   ///@}

   ///@{
   /// Get the number of milliseconds which the application layer should wait
   /// after receiving a busy response from the table.
   ///
   /// Number of milliseconds to wait before resubmitting the application layer retry during
   /// table handling. This is most useful for handling the meter is busy (BSY) and data is not
   /// ready (DNR) response codes.
   ///
   /// ApplicationLayerRetryDelay is different from ApplicationLayerProcedureRetryDelay which
   /// specifies the application retries when the request is a procedure.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 2000 milliseconds
   ///
   /// \seeprop{GetApplicationLayerProcedureRetryDelay,ApplicationLayerProcedureRetryDelay} that works for procedure retry delays.
   ///
   unsigned GetApplicationLayerRetryDelay() const
   { 
      return m_applicationLayerRetryDelay;
   }
   void SetApplicationLayerRetryDelay(unsigned milliseconds)
   { 
      m_applicationLayerRetryDelay = milliseconds;
   }
   ///@}

   ///@{
   /// Number of application layer retries for functions (ANSI C12 procedures).
   ///
   /// Number of times to retry on the application layer procedure request when the request has
   /// been successfully transmitted to the meter, but the meter responded that it cannot service
   /// the request. The most common meter responses that trigger an application retry are the
   /// meter is busy BSY or the data is not ready DNR.
   ///
   /// The typical value for ApplicationLayerProcedureRetries is a large number (like 20) as the
   /// communications are okay and the meter is sending a meaningful response. Set the value to
   /// something small (like 0) to force the operation to fail when the meter is busy or the data
   /// is not ready.
   ///
   /// ApplicationLayerProcedureRetries is different from ApplicationLayerRetries which specifies
   /// the application retries when the request is a table.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 20
   ///
   /// \possible_values
   ///  - 1 .. MAXINT
   ///
   unsigned GetApplicationLayerProcedureRetries() const
   { 
      return m_applicationLayerProcedureRetries;
   }
   void SetApplicationLayerProcedureRetries(unsigned retries)
   { 
      m_applicationLayerProcedureRetries = retries;
   }
   ///@}

   ///@{
   /// Get the number of milliseconds which the application layer should wait
   /// after reading procedure status code 1 from table 8.
   ///
   /// Number of milliseconds to wait before resubmitting the table 8 read request during
   /// procedure handling. This is most useful for handling the meter is busy (BSY) and data is
   /// not ready (DNR) response codes.
   ///
   /// ApplicationLayerProcedureRetryDelay is different from ApplicationLayerRetryDelay which
   /// specifies the application retries when the request is a table.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 500 milliseconds
   ///
   /// \possible_values
   ///  - 1 .. MAXINT
   ///
   /// \seeprop{GetApplicationLayerRetryDelay,ApplicationLayerRetryDelay} that works for table retry delays.
   ///
   unsigned GetApplicationLayerProcedureRetryDelay() const
   { 
      return m_applicationLayerProcedureRetryDelay;
   }
   void SetApplicationLayerProcedureRetryDelay(unsigned milliseconds)
   { 
      m_applicationLayerProcedureRetryDelay = milliseconds;
   }
   ///@}

   ///@{
   /// Sequence number byte to use in the next C12 procedure.
   ///
   /// Procedure sequence number is a dynamic property, not persistent,
   /// and it will always be set to zero at the creation of the protocol object.
   /// During the lifespan of the protocol, the property can either be modified
   /// by the user (the caller), or by the incoming packet that relates to ANSI C12 procedure
   /// as defined in ANSI C12.19 standard paper.
   ///
   /// In client protocols the user (the caller) can modify this property at any moment before
   /// a desired FUNCTION is executed to change SEQ_NBR field of ST7 write.
   /// The following ST8 read will modify this property with the SEQ_NBR value.
   /// The compliant protocol implementation will always respond with the same value for SEQ_NBR.
   /// Currently MeteringSDK client does not check by itself if sequence numbers in ST7 write
   /// and in the following ST8 read match, as this can possibly break some C12 implementations.
   /// The behavior can be reconsidered later after the extensive testing.
   ///
   /// In the server implementation, ST7 write arrives first, and it changes the value of this property
   /// with the incoming SEQ_NBR. The same value is sent back in ST8 unless the user
   /// modifies the property to a different value in the function handler,
   /// in which case a noncompliant value of SEQ_NBR is sent back.
   /// This can be helpful for testing purposes.
   ///
   /// \since MeteringSDK Version 6.6.0.5882.
   ///
   /// \default_value 0
   ///
   /// \possible_values 0 .. 255
   ///
   unsigned GetProcedureSequenceNumber() const
   {
      return static_cast<unsigned>(m_procedureSequenceNumber);
   }
   void SetProcedureSequenceNumber(unsigned number);
   ///@}

   /// Return the maximum possible size of a table to read partially or fully in a single 
   /// application layer packet.
   ///
   /// When the requested table size is bigger than the value of this property,
   /// the request is transparently broken into multiple application layer packets,
   /// so that each table data transfer fits within the property value.
   ///
   /// The property value is calculated using the current protocol 
   /// properties, capabilities, and overhead. The value can change
   /// after properties are negotiated with the device.
   ///
   /// For example, when the protocol is C12.18, the maximum read table size will depend
   /// on the packet size and the maximum number of packets.
   /// After Negotiate is issued, the value can change depending on the
   /// negotiated packet size and the negotiated maximum number of packets.
   ///
   unsigned GetMaximumReadTableSize() const
   {
      return m_maximumReadTableSize;
   }

   ///@{
   /// Defines whether to process Terminate service on Application Layer error.
   ///
   /// This property enables automatic handling of protocols with sessions.
   /// When such protocol encounters an error within the session, and 
   /// this property is true, it is possible to enable the attempt of closing the session
   /// prior to throwing the error to the user.
   ///
   /// \since MeteringSDK Version 5.2.0.1720.
   ///
   /// \default_value false
   ///
   /// \possible_values
   ///  - True  [1] : The end session services are sent to the device after an 
   ///                Application Layer error.
   ///  - False [0] : The end session services are NOT sent to the device after 
   ///                an Application Layer error. The session timeouts according 
   ///                to the protocol rules.
   ///
   void SetEndSessionOnApplicationLayerError(bool endSessionOnApplicationLayerError)
   {
      m_endSessionOnApplicationLayerError = endSessionOnApplicationLayerError;
   }
   bool GetEndSessionOnApplicationLayerError() const
   {
      return m_endSessionOnApplicationLayerError;
   }
   ///@}

   /// Protocol dependent one-byte checksum calculation procedure that uses buffer and its size.
   ///
   /// This particular implementation of the checksum calculation
   /// fits the majority of the protocols, override if needed.
   /// There is also CalculateChecksum, that works on byte string
   /// and returns a checksum value as unsigned integer.
   ///
   /// \pre The buffer designated with the pointer and the length is valid.
   ///
   /// \param buffer 
   ///     Pointer to the beginning of buffer.
   /// \param length
   ///     Length of the buffer in bytes.
   /// \return
   ///     Value of checksum.
   ///
   virtual unsigned CalculateChecksumFromBuffer(const char* buffer, unsigned length) const;

   /// Perform the full application layer request and return a response.
   ///
   /// The command will be the first byte in the packet, while the request parameter,
   /// if present, will form the rest of the packet.
   /// The returned response will not have the status byte,
   /// which will be thrown as MEC12NokResponse if it was not zero.
   ///
   /// \pre The request corresponds to the correct C12 service packet of the given command.
   /// The channel is functioning, the state allows such request.
   /// Otherwise the exception is thrown.
   /// Any nonzero status code will be thrown as C12 exception.
   ///
   /// \param command
   ///     C12 command of the application layer.
   /// \param request
   ///     Request buffer of the command.
   /// \return
   ///     Response buffer, excluding the status code.
   ///
   MByteString ApplicationLayerRequestResponse(char command, const MByteString& request);

   /// Read a code of the incoming application layer data packet.
   ///
   /// \pre The packet was received successfully, and it has the code data available.
   ///
   Muint8 ReceiveServiceCode();

   /// Read one byte from the incoming application layer data packet.
   /// The service works like a stream, one can issue ReceiveByte many times in small chunks, and it will
   /// be returning subsequent bytes.
   ///
   /// \pre The packet was received successfully, and it has data available.
   ///
   Muint8 ReceiveServiceByte();

   /// Read several bytes from the incoming application layer data packet. No more than 4 bytes are allowed to be read.
   /// To read more than 4 bytes use ReceiveServiceBytes.
   /// The service works like a stream, one can issue ReceiveServiceUInt many times in small chunks, and it will
   /// be returning subsequent bytes.
   ///
   /// \pre The packet was received successfully, and it has data available, parameter size is no more than 4.
   ///
   unsigned ReceiveServiceUInt(unsigned size);

   /// Read several bytes of the incoming application layer data packet. The length of the returned buffer is expected.
   /// The service works like a stream, one can issue ReceiveServiceUInt many times in small chunks, and it will
   /// be returning subsequent bytes.
   ///
   /// \pre The packet was received successfully, and it has data available.
   ///
   MByteString ReceiveServiceBytes(unsigned length);

   /// Read bytes of the incoming application layer data packet. Returned buffer has a remaining size.
   ///
   /// \pre The packet was received successfully, and it has data available.
   ///
   MByteString ReceiveServiceRemainingBytes();

   /// Compute checksum of the byte string given as parameters.
   ///
   /// The checksum is calculated based on the C12 rules.
   ///
   static unsigned StaticCalculateChecksum(const MByteString& buff);

   /// Compute checksum of the buffer and length given as parameters.
   ///
   /// The checksum is calculated based on the C12 rules.
   ///
   /// \see StaticCalculateChecksum for the variation with the byte string given as byte string.
   ///
   static unsigned StaticCalculateChecksumFromBuffer(const char* data, unsigned size);

protected:
/// \cond SHOW_INTERNAL

   // Perform the full application layer request, abstract service.
   // The command will be the first byte in the packet, while the request parameter,
   // if present, will form the rest of the packet.
   //
   // \pre The request corresponds to the correct C12 service packet.
   // The channel is functioning, the state allows such request.
   // Otherwise the exception is thrown.
   //
   virtual void DoApplicationLayerRequest(char command, const MByteString *request = NULL, unsigned flags = APPLICATIONLAYERREQUEST_NO_FLAGS) = 0;

   // Perform a single C12 procedure, which results in writing to table 7 and possibly reading
   // from table 8. Multiple application layer retries can be performed.
   // If the response is not expected, the response table might not be read, depending on configuration and table number.
   //
   // \pre The meter allows the procedure with the specified number and
   // specified request. The protocol state allows the request. The channel is functioning.
   // Otherwise the exception is thrown.
   //
   virtual void DoMeterProcedure(unsigned number, const MByteString& request, MByteString& response, bool expectResponse);

   // Helper function, common part of all function code of C12.
   // It also supports executing protocol services as meter procedures.
   //
   // \pre The meter allows the procedure or protocol service with the specified number and
   // specified request. The protocol state allows the request. The channel is functioning.
   // Otherwise the exception is thrown.
   //
   void DoFunction(MCOMNumberConstRef number, const MByteString& request, MByteString& response, bool expectResponse);

   // Try one password, throw if error.
   //
   // \pre The security is cleared with the meter using the password
   // entry given, otherwise an exception is thrown.
   //
   virtual void DoTryPasswordEntry(const MByteString& entry);

#if !M_NO_VERBOSE_ERROR_INFORMATION
   // Build service name with a number and given parameters.
   // The given number is translated to its ANSI protocol representation,
   // which adds prefixes like ST, MT or PST or PMT to a number part of the protocol.
   // If parameters are not present they are not in the name.
   // The particular protocols can fill in their own implementations for full service name.
   //
   // \pre The given chars pointer shall point to a buffer at least 
   // MAXIMUM_SERVICE_NAME_STRING_SIZE long. Otherwise the behavior is undefined.
   // Under no circumstance shall this method throw an error.
   //
   virtual void DoBuildComplexServiceName(MChars fullServiceName, MConstChars serviceName, MCOMNumberConstRef number, int par1 = -1, int par2 = -1) M_NO_THROW;

   // Helper shared implementation for the C12 translation of a number.
   // Will translate the given number to its ANSI protocol representation, if possible.
   // If the translation was unsuccessful, False is returned.
   // This implementation is shared between all C12 derived protocols.
   //
   // \pre The given chars pointer shall point to a buffer at least 
   // MAXIMUM_SERVICE_NAME_STRING_SIZE long. Otherwise the behavior is undefined.
   // Under no circumstance shall this method throw an error.
   // If there was not a number given as parameter, the procedure will return false.
   //
   static bool DoBuildComplexC12ServiceName(MChars fullServiceName, MConstChars serviceName, MCOMNumberConstRef number, int par1 = -1, int par2 = -1) M_NO_THROW;

#endif // !M_NO_VERBOSE_ERROR_INFORMATION
/// \endcond SHOW_INTERNAL

public: // Methods:

   /// Two-byte CRC calculation static procedure specific to C12 protocol.
   ///
   /// The algorithm itself uses a set of shifts and xor operators for efficiency.
   /// When executing on Intel, or other Little Endian architectures,
   /// the bytes are swapped after polynom calculation. 
   ///
   /// Note that there is also a virtual variation of this service, CalculateCRC16FromBuffer,
   /// which is used externally by interfaces which do not care about particular protocols.
   /// 
   /// \pre The buffer designated with the pointer and the length is valid.
   ///
   static Muint16 StaticCalculateCRC16FromBuffer(const char* buffer, unsigned length);

   /// Same as above, but taking byte string, reflected.
   /// 
   /// \pre Same as StaticCalculateCRC16FromBuffer.
   ///
   static unsigned CRC16(const MByteString& buffer);

protected: // Services:
/// \cond SHOW_INTERNAL

#if !M_NO_MCOM_IDENTIFY_METER
   // Identify the meter if the protocol is indeed ANSI C12 (note this is not
   // an ANSI C12 Identify request).
   //
   // If the first parameter is true, the service assumes the session is open, and it will not
   // start the session, or close it. The second parameter, if it is not NULL, is filled with the
   // data from tables, which DoIdentifyMeter had to read from the meter during its attempts to fetch
   // the necessary information from the meter. This table vector can be used in order to minimize
   // the communication with the meter, if this task has its place.
   //
   // \pre The channel is connected. Otherwise a channel-related
   // exception is thrown that tells about failure. The meter is indeed the one which
   // is able to talk ANSI C12. Otherwise the protocol-specific exception
   // is thrown that signals the failure of communicating through the channel.
   // If the session is started already, the first parameter has to be true,
   // or the behavior is undefined.
   //
   virtual MStdString DoIdentifyMeter(bool sessionIsStarted, TableRawDataVector* tablesRead) = 0;
#endif // !M_NO_MCOM_IDENTIFY_METER

   // Send Terminate in case the appropriate error occurred.
   // This method may be called only if EndSessionOnApplicationLayerError property is on.
   // This method should be called on the C12 Application Layer errors, such as ONP, ISC and others.
   // It shouldn't be called on ISSS error. It also may be called when some Exception occurred in the
   // non-base state of communication. See protocol specification for details.
   //
   // \pre The error is occurred in non-base state of communication. The current state of the
   // meter and client are the same (for example, ISSS defines that the are not the same) and
   // the termination of the session is required (for example, BSY defines that the service may be processed again).
   //
   virtual void DoEndSessionOnApplicationLayerError( bool issueOnlyTerminate );

   // Check the response code and send MEC12NokResponse exception in case no retry is needed.
   // Retry is needed if the retryCondition is true and retryCounter > 0.
   //
   // \pre Application layer error has occurred during communication. The responseCode is the
   // error code received by the server. The retryCondition is true if the retry is required.
   //
   virtual void DoCheckCodeTerminateAndThrowOrNotify(MEC12NokResponse& ex, bool retryCondition, unsigned int retryCount, bool issueOnlyTerminate, MProtocol* wrapperProtocol);

   // Protected service that internally sets the negotiated packet size to the
   // given value. This service can be overloaded to do some buffer allocation.
   //
   // \pre The value has to be within the valid range SMALLEST_PACKET_SIZE
   // to BIGGEST_PACKET_SIZE, or an exception will be thrown.
   //
   virtual void DoSetNegotiatedPacketSize(unsigned negotiatedPacketSize);

   // Helper service that reads the response formatted for the table read
   // and table partial read requests.
   //
   // \pre The read request is successful, and the data are
   // available for fetching. Otherwise the behavior is undefined.
   // The data checksum is good, otherwise an exception is thrown.
   //
   void DoAppendTableReadResponse(MByteString& data);

protected: // Methods:

#if !M_NO_MCOM_KEEP_SESSION_ALIVE

   // Implementation of the service that actually keeps the C12 session alive.
   // It never throws an exception, returns the delay for the next keeping of the session.
   // If the returned value is zero, no keeping of the session shall be made.
   //
   // This particular implementation uses Wait protocol command.
   //
   // \pre Shall be called only at a proper time of keeping the session alive.
   // Also, if the communication is unsuccessful, zero is returned to indicate 
   // the session shall not be kept alive any more.
   //
   virtual unsigned DoSendKeepSessionAliveMessage();

#endif

protected: // Attributes:

   // Logic when to read function response when RESPONSE is absent.
   //
   ReadFunctionResponseEnum m_alwaysReadFunctionResponse;

   // Whether to issue the Security service during starting the session.
   //
   bool m_issueSecurityOnStartSession;

   // Whether to always read function response table 8, or only when the response is expected.
   //
   bool m_alwaysUsePartial;

   // Whether Wait shall be used for keeping session.
   // 
   bool m_useReadInKeepSessionAlive;

   // Procedure sequence number, a byte 0 to 255.
   //
   Muint8 m_procedureSequenceNumber;

   // The user ID field passed to the Logon service, 16 bits used.
   //
   unsigned m_userId;

   // The ten byte user name passed to the Logon service.
   //
   MByteString m_user;

   // Number of application layer retries used during table handling.
   //
   unsigned m_applicationLayerRetries;

   // Number of milliseconds delayed after the application layer got busy
   // or data unavailable response during table handling.
   //
   unsigned m_applicationLayerRetryDelay;

   // Number of application layer retries used during procedure handling.
   //
   unsigned m_applicationLayerProcedureRetries;

   // Number of milliseconds delayed after the application layer got busy
   // or data unavailable response during procedure handling.
   //
   unsigned m_applicationLayerProcedureRetryDelay;

   // This variable identifies whether the ST_007 tableWrite procedure
   // is specified. It is used by the link-layer to reset the link-layer retries
   // while receiving response from the meter during ST_007 tableWrite.
   //
   bool m_isST007Write;

   // Turn around delay
   //
   unsigned m_turnAroundDelay;

   // Negotiated size of the packet, the one returned by the meter if negotiated.
   // Note even though this is defined in the C12 abstract, it is
   // published only by C12.18. If the protocol is not packet-oriented
   // (has no data link layer like C12.22), this value will be UINT_MAX.
   //
   unsigned m_negotiatedPacketSize;

   // Maximum possible size of a table to read partially of fully.
   // By default it is USHRT_MAX, but it can be redefined on the upper level.
   //
   unsigned m_maximumReadTableSize;

   // Maximum possible size of a table to write fully.
   // By default it is USHRT_MAX, but it can be redefined on the upper level.
   //
   unsigned m_maximumWriteTableSize;

   // Maximum possible size of a table to write partially.
   // By default it is USHRT_MAX, but it can be redefined on the upper level.
   //
   unsigned m_maximumPartialWriteTableSize;

   // Expected size of the application layer response for read table request.
   // Used to estimate progress on data link layer when reading large tables.
   // Zero value means 'do not use'.
   //
   unsigned m_expectedPartialReadTableReadResponseSize;

   // Defines whether the Terminate service should be processed
   // in case the application level error is occurred.
   //
   bool m_endSessionOnApplicationLayerError;

   // Number of times the link layer retries to send the packet before giving up.
   // This is not published in C12 abstract, but in C1218 and its children.
   //
   unsigned m_linkLayerRetries;

   // Buffer read for the application layer.
   //
   MBufferReader m_applicationLayerReader;


   M_DECLARE_CLASS(ProtocolC12)

/// \endcond SHOW_INTERNAL
};

#endif // !M_NO_MCOM_PROTOCOL_C1218 || !M_NO_MCOM_PROTOCOL_C1221 || !M_NO_MCOM_PROTOCOL_C1222

///@}
#endif
