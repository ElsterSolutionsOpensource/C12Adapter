#ifndef MCOM_MONITOR_H
#define MCOM_MONITOR_H
/// \addtogroup MCOM
///@{
/// \file MCOM/Monitor.h

#include <MCOM/MCOMDefs.h>

#if !M_NO_MCOM_MONITOR

/// Abstract monitor to watch communication.
///
/// Concrete monitors will implement their specific actions to fulfill monitoring tasks,
/// such as dumping the contents into a file, or sending it to an interactive monitor.
///
/// Sharing of monitors among simultaneously communicating channels 
/// can produce obscure output or lead to synchronization errors,
/// therefore, each channel should have its own monitor object.
/// The monitor is registered with the client channel object using the service
/// \refprop{MChannel::SetMonitor,MChannel::Monitor}.
///
/// \if CPP
/// Note that when M_NO_MCOM_MONITOR_SHARED_POINTER=0
/// the monitor object should be created on a heap with operator new,
/// and handled through the pointer type \ref MMonitor::Pointer.
/// The deletion of such object will be done through the stared pointer.
/// When M_NO_MCOM_MONITOR_SHARED_POINTER=1 the user will be responsible
/// for deletion of the object, however the monitor object should outlive
/// the channel that it monitors.
/// \endif
///
class MCOM_CLASS MMonitor : public MObject
{
   friend class MChannel;

#if !M_NO_MCOM_MONITOR_SHARED_POINTER
   M_SHARED_POINTER_CLASS(Monitor)
#endif

public: // Types:

   /// Enumeration that defines supported monitor messages, and whether the parameters are expected.
   ///
   /// Note that the values for this enumeration have to be kept for compatibility reasons.
   ///
   enum MessageType
   {
      /// Channel is attached to the monitor.
      ///
      /// The parameter is the channel identification.
      ///
      MessageChannelAttach = 0x20,

      /// Channel connected, no parameter.
      ///
      MessageChannelConnect = 0x21,

      /// Channel disconnected, no parameter.
      ///
      MessageChannelDisconnect = 0x22,

      /// Channel bytes received.
      ///
      /// Byte string is the parameter.
      ///
      MessageChannelByteRx = 0x23,

      /// Channel bytes transmitted.
      ///
      /// Byte string is the parameter.
      ///
      MessageChannelByteTx = 0x24,

      /// Link layer information message.
      ///
      /// Information message is specified as parameter.
      ///
      MessageProtocolLinkLayerInformation = 0x35,

      /// Link layer retry error message.
      ///
      /// Retry message is specified as parameter.
      ///
      MessageProtocolLinkLayerRetry = 0x37,

      /// Link layer failure error message.
      ///
      /// Fail message is specified as parameter.
      ///
      MessageProtocolLinkLayerFail = 0x39,

      /// Application layer information message.
      ///
      /// Application layer start message is specified as parameter.
      ///
      MessageProtocolApplicationLayerStart = 0x3B,

      /// Application layer retry error message.
      ///
      /// Application layer retry message is specified as parameter.
      ///
      MessageProtocolApplicationLayerRetry = 0x3D,

      /// Application layer success error message.
      ///
      /// Application layer success message is specified as parameter.
      ///
      MessageProtocolApplicationLayerSuccess = 0x3F,

      /// Application layer failure error message.
      ///
      /// Application layer failure message is specified as parameter.
      ///
      MessageProtocolApplicationLayerFail = 0x41,

      /// Special message that has information about absolute time of messages in session.
      ///
      MessageProtocolSynchronize = 0x43,

      /// User message sent to the monitor.
      ///
      /// User message is specified as parameter.
      ///
      MessageUser = 0x60
   };

/// \cond SHOW_INTERNAL

   // Enumeration that defines supported monitor messages, and whether the parameters are expected.
   // Note that the values for this enumeration have to be kept for compatibility reasons.
   //
   // UNICODE versions of services use double byte format. The length of the packet is
   // in bytes, it is not the number of UNICODE characters.
   //
   enum
   {
      // Channel is attached to the monitor.
      // The parameter is the channel identification.
      MESSAGE_CHANNEL_ATTACH = 0x20,

      // Channel connected, no parameter.
      MESSAGE_CHANNEL_CONNECT = 0x21,

      // Channel disconnected, no parameter.
      MESSAGE_CHANNEL_DISCONNECT = 0x22,

      // Channel bytes received, bytes received is specified as parameter.
      MESSAGE_CHANNEL_BYTE_RX = 0x23,

      // Channel bytes transmitted, bytes transmitted is specified as parameter.
      MESSAGE_CHANNEL_BYTE_TX = 0x24,

      // Data link layer information messages, character version.
      // Information message is specified as parameter.
      MESSAGE_PROTOCOL_DATA_LINK_LAYER_INFORMATION = 0x35,

      // Data link layer information messages, unicode doublebyte version.
      // Information message in UNICODE is specified as parameter.
      MESSAGE_PROTOCOL_DATA_LINK_LAYER_INFORMATION_UNICODE = 0x36,

      // Data link layer retry error messages, character version.
      // Retry message is specified as parameter.
      MESSAGE_PROTOCOL_DATA_LINK_LAYER_RETRY = 0x37,

      // Data link layer retry error messages, unicode doublebyte version.
      // Retry message in UNICODE is specified as parameter.
      MESSAGE_PROTOCOL_DATA_LINK_LAYER_RETRY_UNICODE = 0x38,

      // Data link layer failure error messages, character version.
      // Fail message is specified as parameter.
      MESSAGE_PROTOCOL_DATA_LINK_LAYER_FAIL = 0x39,

      // Data link layer failure error messages, unicode doublebyte version.
      // Fail message in UNICODE is specified as parameter.
      MESSAGE_PROTOCOL_DATA_LINK_LAYER_FAIL_UNICODE = 0x3A,

      // Application layer information messages, character version.
      // Application layer start message is specified as parameter.
      MESSAGE_PROTOCOL_APPLICATION_LAYER_START = 0x3B,

      // Application layer information messages, unicode doublebyte version.
      // Application layer start message as UNICODE is specified as parameter.
      MESSAGE_PROTOCOL_APPLICATION_LAYER_START_UNICODE = 0x3C,

      // Application layer retry error messages, character version.
      // Application layer retry message is specified as parameter.
      MESSAGE_PROTOCOL_APPLICATION_LAYER_RETRY = 0x3D,

      // Application layer retry error messages, unicode doublebyte version.
      // Application layer retry message as UNICODE is specified as parameter.
      MESSAGE_PROTOCOL_APPLICATION_LAYER_RETRY_UNICODE = 0x3E,

      // Application layer success error messages, character version.
      // Application layer success message is specified as parameter.
      MESSAGE_PROTOCOL_APPLICATION_LAYER_SUCCESS = 0x3F,

      // Application layer success error messages, unicode doublebyte version.
      // Application layer success message as UNICODE is specified as parameter.
      MESSAGE_PROTOCOL_APPLICATION_LAYER_SUCCESS_UNICODE = 0x40,

      // Application layer failure error messages, character version.
      // Application layer failure message is specified as parameter.
      MESSAGE_PROTOCOL_APPLICATION_LAYER_FAIL = 0x41,

      // Application layer failure error messages, unicode doublebyte version.
      // Application layer failure message as UNICODE is specified as parameter.
      MESSAGE_PROTOCOL_APPLICATION_LAYER_FAIL_UNICODE = 0x42,

      // Special message that has an information about absolute time of messages in session.
      MESSAGE_PROTOCOL_SYNCHRONIZE = 0x43,

      // User message sent to the monitor, character version.
      // User message is specified as parameter.
      MESSAGE_PROTOCOL_USER_MESSAGE = 0x60,

      // User message sent to the monitor, unicode doublebyte version.
      // User message in UNICODE is specified as parameter.
      MESSAGE_PROTOCOL_USER_MESSAGE_UNICODE = 0x61,
   };

/// \endcond SHOW_INTERNAL

#if !M_NO_MCOM_MONITOR_SHARED_POINTER

   /// Pointer type that clients should use to access to monitor.
   ///
   typedef SharedPointer Pointer;

#else

   /// Pointer type for this class.
   ///
   typedef MMonitor* Pointer;

#endif

public: 

   /// Object constructor.
   ///
   MMonitor();

   /// Object destructor.
   ///
   virtual ~MMonitor();

public:

   /// Whether the monitor is interested in any events.
   ///
   bool IsListening() const
   {
      return m_listening != 0;
   }

   /// Tell that the application is starting a sequence of events that it would like
   /// to monitor, attach to the monitor. The action is not immediate, 
   /// and some time might pass before the monitor is attached.
   /// Any number of Attach can be issued, and they are not necessarily matched
   /// with the number of Detach calls.
   ///
   /// Media identification is a string that somehow tells about the client.
   /// Like for the channel based on COM port, this would be the com port name,
   /// and for the socket it will be the address and the port.
   ///
   /// Other preconditions will depend on implementation in the child class.
   ///
   virtual void Attach(const MStdString& mediaIdentification);

   /// Detach from the monitor, if attached previously.
   /// The action is not immediate, and some time might pass before the monitor is detached.
   /// No matter for how much the Attach was called, Detach will schedule
   /// detachment of the monitor.
   ///
   virtual void Detach();

   /// Write the user message to the monitor.
   ///
   /// Writes the message to the binary log file (if one has been specified) and the
   /// Monitor (if one is attached to the connection). No error is
   /// generated if no log file has been specified or if no Monitor is
   /// attached to the connection.
   ///
   /// The Write method can be used to make communication transactions more readable by marking
   /// key data transactions, such as the start/end of a test.
   /// Note that the Write occurs when it is issued and it cannot be inserted into the Queue
   /// services. For the code example below, the Monitor would get the
   /// Write Message "Read ST-001", before the Queue services Start Session, Read Table, End
   /// Session. The QCommit causes the commands in the Queue to be issued, so the Write Message
   /// was issued before any of the Queue services had been issued.
   /// \code
   ///   protocol.QStartSession();
   ///   protocol.QTableRead(1, 0, 0); // read ST1
   ///   protocol.QEndSession();
   ///   protocol.Monitor.Write("Read ST-001");
   ///   protocol.QCommit();
   /// \endcode
   ///
   /// \param str
   ///      The message to posted to the file and/or Monitor.
   ///
   /// \see \ref MProtocol::QWriteToMonitor - inserts the message into the MProtocol queue.
   ///
   virtual void Write(const MStdString& str);

public: // Notifications:

   /// Send message with the specified code.
   /// Message is a sequence of bytes, and size is length of the sequence.
   /// No UNICODE version provided.
   ///
   virtual void OnMessage(MessageType code, const char* message, int length);

   /// Notify that the channel has just connected.
   ///
   virtual void OnConnect();

   /// Notify that the channel is disconnected.
   ///
   virtual void OnDisconnect();

   ///@{
   /// Notify that the bytes are received by the channel.
   ///
   /// \pre The errors are not reported by this service.
   ///
   virtual void OnByteRX(const char* data, int length);
   void OnBytesReceived(const MByteString& data);
   ///@}

   ///@{
   /// Notify that the bytes are sent through the channel.
   ///
   /// \pre The errors are not reported by this service.
   ///
   virtual void OnByteTX(const char* data, int length);
   void OnBytesSent(const MByteString& data);
   ///@}

   /// Notifies that the data link layer operation succeeded.
   ///
   /// \pre The errors are not reported by this service.
   ///
   virtual void OnDataLinkLayerSuccess();

   /// Notifies about any neutral or positive information during the data link layer operations.
   ///
   /// \pre The errors are not reported by this service.
   ///
   virtual void OnDataLinkLayerInformation(const MStdString& msg);

   /// Notifies that the last data link layer operation failed and will be repeated.
   ///
   /// \pre The errors are not reported by this service.
   ///
   virtual void OnDataLinkLayerRetry(const MStdString& reason);

   /// Notifies that the last data link layer operation failed and no other action will be performed.
   ///
   /// \pre The errors are not reported by this service.
   ///
   virtual void OnDataLinkLayerFail(const MStdString& msg);

   /// Notifies that the last application layer operation failed and will be repeated.
   ///
   /// \pre The errors are not reported by this service.
   ///
   virtual void OnApplicationLayerRetry(const MStdString& reason);

   /// Notifies that the last application layer operation failed and no other action will be performed.
   ///
   /// \pre The errors are not reported by this service.
   ///
   virtual void OnApplicationLayerFail(const MStdString& service);

   /// Notifies that the application layer operation started.
   ///
   /// \pre The errors are not reported by this service.
   ///
   virtual void OnApplicationLayerStart(const MStdString& service);

   /// Notifies that the last application layer operation succeeded.
   ///
   /// \pre The errors are not reported by this service.
   ///
   virtual void OnApplicationLayerSuccess(const MStdString& service);

   /// Notifies that the uninterruptible communication sequence is entered.
   ///
   /// \pre The errors are not reported by this service.
   ///
   virtual void OnEnterUninterruptibleCommunication();

   /// Notifies that the uninterruptible communication sequence is left.
   ///
   /// \pre The errors are not reported by this service.
   ///
   virtual void OnLeaveUninterruptibleCommunication();

protected: // Services:

   /// Send text message with the specified code.
   /// Default implementation will fit majority of cases,
   /// which include calling OnMessage with code and message possibly translated for UNICODE.
   ///
   /// \pre The message code shall have both UNICODE and plain variants.
   /// There is a debug check for that. The given text shall be a valid zero-terminated string.
   ///
   void OnMessageWithText(MessageType code, const MStdString& text);

public:

   ///@{
   /// Client that supports monitor messages through reflection.
   ///
   MObject* GetClient() const
   {
      return m_client;
   }
   void SetClient(MObject* client)
   {
      if ( client != NULL )
         m_listening = 1;
      m_client = client;
   }
   ///@}

protected: // Data members:
/// \cond SHOW_INTERNAL

   // Flag that is set to nonzero if the monitor is interested in any events
   //
   MInterlocked m_listening;

   // Pointer to client object
   // Used in reflection
   //
   MObject* m_client;

/// \endcond SHOW_INTERNAL

   M_DECLARE_CLASS(Monitor)
};

#endif // !M_NO_MCOM_MONITOR

///@}
#endif
