#ifndef MCOM_PROTOCOL_H
#define MCOM_PROTOCOL_H
/// \addtogroup MCOM
///@{
/// \file MCOM/Protocol.h

#include <MCOM/MCOMDefs.h>
#include <MCOM/Channel.h>
#include <MCOM/CommunicationCommand.h>
#include <MCOM/SessionKeeper.h>

/// Abstraction of a communication protocol.
///
/// The protocol is able to execute application level commands directly,
/// or through the command queue interface.
/// The concrete instances of protocol will implement the abstractions
/// defined by this class, plus some extra services and attributes specific to those protocols.
///
/// \ifnot M_NO_MCOM_FACTORY
/// Preferred way of creating a protocol is through MCOMFactory,
/// and typically, the protocol will be created together with MChannel object using
/// \ref MCOMFactory::CreateProtocol()
/// \endif
///
class MCOM_CLASS MProtocol : public MCOMObject
{
   friend class MProtocolThread;
   friend class MProtocolC1222;
   friend class MProtocolServiceWrapper;
   friend class MSessionKeeper;
   friend class MServerProtocolC12;
   friend class MServerProtocolC1222;

public: // Types:

   enum
   {
      /// Maximum string size allowed for a Number, used in representing a number by the monitoring
      /// and error reporting facilities.
      ///
      MAXIMUM_NUMBER_STRING_SIZE = 64,

      /// Maximum string size for the service name.
      ///
      MAXIMUM_SERVICE_NAME_STRING_SIZE = MAXIMUM_NUMBER_STRING_SIZE + 64,

      /// Size that a response to a function can allocate when its actual size is not known.
      ///
      DEFAULT_ESTIMATED_RESPONSE_SIZE = 0x1000,

      /// Maximum value for abstract table offset.
      ///
      /// This value is protocol dependent, however this value implies a high level check that is not smaller than any protocol.
      ///
      MAXIMUM_POSSIBLE_TABLE_OFFSET = 0xFFFFFF,

      /// Maximum value for abstract table length.
      ///
      /// This value is protocol dependent, however this value implies a high level check that is not smaller than any protocol.
      /// Also, when the length is bigger than the physical length supported by the protocol,
      /// many consecutive application layer requests are done.
      ///
      MAXIMUM_POSSIBLE_TABLE_LENGTH = 0xFFFFFF
   };

#if !M_NO_MCOM_IDENTIFY_METER
   /// Table raw data associated with the table number.
   ///
   struct MCOM_CLASS TableRawData
   {
   public: // Constructor, destructor, services:

      /// Default constructor.
      ///
      TableRawData()
      {
      }

      /// Constructor that builds a new table raw data with parameters given.
      ///
      TableRawData(MCOMNumberConstRef number, const MByteString& data)
      :
         m_number(number),
         m_data(data)
      {
      }

      /// Copy constructor.
      ///
      TableRawData(const TableRawData& other)
      :
         m_number(other.m_number),
         m_data(other.m_data)
      {
      }

      /// Destructor.
      ///
      ~TableRawData()
      {
      }

      /// Assignment operator.
      ///
      TableRawData& operator=(const TableRawData& other)
      {
         if ( &other != this )
         {
            m_number = other.m_number;
            m_data = other.m_data;
         }
         return *this;
      }

      /// Get the table number from the table raw data entry.
      ///
      MCOMNumberConstRef GetNumber() const
      {
         return m_number;
      }

      /// Get the table data from the table raw data entry.
      ///
      const MByteString& GetData() const
      {
         return m_data;
      }

   private: // Attributes:

      // Table number
      //
      MCOMNumber m_number;

      // Associated data
      //
      MByteString m_data;
   };

   /// Vector of table raw data.
   ///
   typedef std::vector<TableRawData>
      TableRawDataVector;
#endif

protected: // Constructor:

   /// Create a new abstract protocol with the channel given.
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
   MProtocol(MChannel* channel, bool channelIsOwned = true);

public: // Destructor:

#if !M_NO_MCOM_FACTORY
   /// Virtual copy constructor, creates the protocol which is a clone of current.
   ///
   /// All persistent properties get copied, all other properties have initial values.
   /// If channel is owned, channel is duplicated.
   /// If channel is not owned, it is made shared.
   ///
   virtual MProtocol* CreateClone() const;
#endif

   /// Destroy the protocol object.
   ///
   /// If the channel is owned, \ref IsChannelOwned is true, the channel is also destroyed.
   ///
   virtual ~MProtocol() = 0;

   /// Execute this method as first action in the destructor of any child protocol.
   ///
   /// It is okay to call this method many times from a hierarchy of destructors,
   /// however nothing else shall be called after.
   ///
   void Finalize() M_NO_THROW;

public: // Attribute accessors and manipulators:

   ///@{
   /// Primary data order of the device, whether it is little endian.
   ///
   /// This information is used by some protocol implementations,
   /// for example, for ANSI procedures/functions implementation. Such protocols
   /// have application-level fields that depend on the protocol byte order.
   ///
   /// It is only necessary to set this property when communicating to a device of different endianity
   /// as the protocol default. When communicating through the upper library levels this property can be set automatically.
   ///
   /// \default_value Depends on the protocol type. Most of the protocols are little endian, while some legacy ones are big endian.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \possible_values
   ///  - True [1]  : Little Endian
   ///  - False [0] : Big Endian
   ///
   bool GetMeterIsLittleEndian() const
   {
      return m_meterIsLittleEndian;
   }
   void SetMeterIsLittleEndian(bool isLittleEndian)
   {
      m_meterIsLittleEndian = isLittleEndian;
   }
   ///@}

   /// The number of Application Layer services that have been successfully processed.
   ///
   /// The count is nullified when the protocol is created, and when \ref ResetCounts is called.
   ///
   /// \since MeteringSDK Version 2.2.17.
   ///
   /// \possible_values
   ///  - 0 .. UINT32
   ///
   unsigned GetCountApplicationLayerServicesSuccessful() const
   {
      return m_countApplicationLayerServicesSuccessful;
   }

   /// Increment the number of application layer services successfully processed.
   ///
   /// This can be used by the outside application, which is willing to treat its
   /// actions as an application layer service.
   ///
   void IncrementCountApplicationLayerServicesSuccessful()
   {
      ++ m_countApplicationLayerServicesSuccessful;
   }

   /// The number of Application Layer services that have been retried.
   ///
   /// The count is nullified when the connection is created, or when \ref ResetCounts is issued.
   /// For some protocols the application layer retries never take place,
   /// in which case this property will remain at zero.
   ///
   /// \since MeteringSDK Version 2.2.17.
   ///
   /// \possible_values
   ///  - 0 .. UINT32
   ///
   unsigned GetCountApplicationLayerServicesRetried() const
   {
      return m_countApplicationLayerServicesRetried;
   }

   /// Increment the number of application layer services retried.
   ///
   /// This can be used by the outside application, which is willing to treat its
   /// actions as an application layer retry.
   ///
   void IncrementCountApplicationLayerServicesRetried()
   {
      ++ m_countApplicationLayerServicesRetried;
   }

   /// The number of Application Layer services that have failed.
   ///
   /// The count is nullified when the connection is created, or when \ref ResetCounts is issued.
   ///
   /// \since MeteringSDK Version 2.2.17.
   ///
   /// \possible_values
   ///  - 0 .. UINT32
   ///
   unsigned GetCountApplicationLayerServicesFailed() const
   {
      return m_countApplicationLayerServicesFailed;
   }

   /// Increment the number of application layer services failed.
   ///
   /// This can be used by the outside application, which is willing to treat its
   /// actions as an application layer failure.
   ///
   void IncrementCountApplicationLayerServicesFailed()
   {
      ++ m_countApplicationLayerServicesFailed;
   }

   /// The number of Link Layer packets that have been successfully processed.
   ///
   /// The count is nullified when the connection is created, or when \ref ResetCounts is issued.
   /// Some protocols, such as \ref MProtocolC1222, do not have a link layer,
   /// therefore, they will not increment this counter.
   ///
   /// \since MeteringSDK Version 2.2.17.
   ///
   /// \possible_values
   ///  - 0 .. UINT32
   ///
   unsigned GetCountLinkLayerPacketsSuccessful() const
   {
      return m_countLinkLayerPacketsSuccessful;
   }

   /// Increment the number of data link layer packets successfully processed.
   ///
   /// This can be used by the outside application, which is willing to treat its
   /// actions as a link layer success.
   ///
   void IncrementCountLinkLayerPacketsSuccessful()
   {
      ++ m_countLinkLayerPacketsSuccessful;
   }

   /// The number of Link Layer packets that have been retried.
   ///
   /// The count is nullified when the connection is created, or when \ref ResetCounts is issued.
   /// Some protocols, such as \ref MProtocolC1222, do not have a link layer,
   /// therefore, they will not increment this counter.
   ///
   /// \since MeteringSDK Version 2.2.17.
   ///
   /// \possible_values
   ///  - 0 .. UINT32
   ///
   unsigned GetCountLinkLayerPacketsRetried() const
   {
      return m_countLinkLayerPacketsRetried;
   }

   /// Increment the number of data link layer packets retried.
   ///
   /// This can be used by the outside application, which is willing to treat its
   /// actions as a link layer retry.
   ///
   void IncrementCountLinkLayerPacketsRetried()
   {
      ++ m_countLinkLayerPacketsRetried;
   }

   /// Get number of data link layer packets failed.
   ///
   /// The count is nullified when the connection is created, or when \ref ResetCounts is issued.
   /// Some protocols, such as \ref MProtocolC1222, do not have a link layer,
   /// therefore, they will not increment this counter.
   ///
   /// \since MeteringSDK Version 2.2.17.
   ///
   /// \possible_values
   ///  - 0 .. UINT32
   ///
   unsigned GetCountLinkLayerPacketsFailed() const
   {
      return m_countLinkLayerPacketsFailed;
   }

   /// Increment the number of data link layer packets failed.
   ///
   /// This can be used by the outside application, which is willing to treat its
   /// actions as a link layer failure.
   ///
   void IncrementCountLinkLayerPacketsFailed()
   {
      ++ m_countLinkLayerPacketsFailed;
   }

   /// Gets the maximum measured approximate packet round trip time over the link layer.
   ///
   /// A 'round trip' is defined as the amount of time elapsed from the last byte sent to the
   /// first byte received back. Each trip is monitored, and the following properties updated
   /// accordingly: \refprop{GetMinimumRoundTripTime,MinimumRoundTripTime},
   /// \refprop{GetMaximumRoundTripTime,MaximumRoundTripTime}, and \refprop{GetAverageRoundTripTime,AverageRoundTripTime}.
   ///
   /// The round trip times are not updated when there is a timeout, even if there were garbage
   /// characters on the line. The round trip times are nullified when the connection is created
   /// or when \ref ResetCounts is issued.
   ///
   /// \since MeteringSDK Version 4.0.43.
   ///
   /// \possible_values
   ///  - 0 .. UINT32
   ///
   unsigned GetMaximumRoundTripTime() const
   {
      return m_maximumRoundTripTime;
   }

   /// Gets the minimum measured approximate packet round trip time over the link layer.
   ///
   /// A 'round trip' is defined as the amount of time elapsed from the last byte sent to the
   /// first byte received back. Each trip is monitored, and the following properties updated
   /// accordingly: \refprop{GetMinimumRoundTripTime,MinimumRoundTripTime},
   /// \refprop{GetMaximumRoundTripTime,MaximumRoundTripTime}, and \refprop{GetAverageRoundTripTime,AverageRoundTripTime}.
   ///
   /// The round trip times are not updated when there is a timeout, even if there were garbage
   /// characters on the line. The round trip times are nullified when the connection is created
   /// or when \ref ResetCounts is issued.
   ///
   /// \since MeteringSDK Version 4.0.43.
   ///
   /// \possible_values
   ///  - 0 .. UINT32
   ///
   unsigned GetMinimumRoundTripTime() const
   {
      return m_minimumRoundTripTime;
   }

   /// Gets the average measured approximate packet round trip time over the link layer.
   ///
   /// A 'round trip' is defined as the amount of time elapsed from the last byte sent to the
   /// first byte received back. Each trip is monitored, and the following properties updated
   /// accordingly: \refprop{GetMinimumRoundTripTime,MinimumRoundTripTime},
   /// \refprop{GetMaximumRoundTripTime,MaximumRoundTripTime}, and \refprop{GetAverageRoundTripTime,AverageRoundTripTime}.
   ///
   /// The round trip times are not updated when there is a timeout, even if there were garbage
   /// characters on the line. The round trip times are nullified when the connection is created
   /// or when \ref ResetCounts is issued.
   ///
   /// \since MeteringSDK Version 4.0.43.
   ///
   /// \possible_values
   ///  - 0 .. UINT32
   ///
   unsigned GetAverageRoundTripTime() const
   {
      if ( m_roundTripCounter > 0.0 )
         return unsigned(m_sumRoundTripTime / m_roundTripCounter);
      return 0u;
   }

   ///@{
   /// Channel associated with this protocol.
   ///
   /// When the channel is owned, and a new one is reassigned to the protocol,
   /// the previous channel will be deleted.
   ///
   MChannel* GetChannel() const
   {
      return m_channel;
   }
   void SetChannel(MChannel* channel);
   ///@}

   ///@{
   /// Whether the channel is owned by this protocol.
   ///
   /// When the channel is owned, it will be deleted by protocol destructor,
   /// or at the event of channel reassignment to the protocol.
   ///
   bool IsChannelOwned() const
   {
      return m_isChannelOwned;
   }
   void SetIsChannelOwned(bool yes)
   {
      m_isChannelOwned = yes;
   }
   ///@}

   ///@{
   /// Whether the protocol should keep session alive in case of long inactivity.
   ///
   /// When KeepSessionAlive is true, the session is kept from timing out.
   /// The feature can also be used to prevent an idling connection from being closed. 
   /// What is done to keep the session alive is protocol dependent, but usually involves
   /// repeatedly sending some type of communication request to the meter.
   /// For some sessionless protocols this property has no effect.
   ///
   /// The session is being kept alive by a background thread that is watching for
   /// the current value of property \ref IsInSession. When \ref IsInSession is true,
   /// the background communication is performed. Errors raised on the background
   /// can be silenced if \ref IsConnected value was false. Otherwise, the session keeping errors
   /// will be thrown to the foreground thread at the next communication event, such as \ref QCommit. 
   /// Another place that checks and throws an error collected by the background thread
   /// is the assignment to KeepSessionAlive property itself. Therefore, the following
   /// will check for an error, and throw it if it was present:
   /// \code
   ///      protocol.SetKeepSessionAlive(protocol.GetKeepSessionAlive()); // C++
   /// \endcode
   ///
   /// When KeepSessionAlive is false, the session will time out or the connection will be closed
   /// according to the protocol rules.
   ///
   /// \since MeteringSDK Version 3.2.6.
   ///
   /// \default_value False
   ///
   /// \possible_values
   ///  - True [1]  : The session will not time out.
   ///  - False [0] : The session will time out according to the protocol rules.
   ///
   /// \see IsInSession - whether the session keeping is active.
   ///
   bool GetKeepSessionAlive() const;
   void SetKeepSessionAlive(bool alive);
   ///@}


   ///@{
   /// Application level password of the protocol.
   ///
   /// The Password property is the binary value that is used to gain access to the device.
   /// No interpretation of C escape character sequences is performed on this string.
   ///
   /// Many protocols will typically use password in the StartSession sequence,
   /// while some, such as sessionless mode of \ref MProtocolC1222, will send password at every request.
   ///
   /// The password can have less characters than the maximum number of characters allowed. In this case,
   /// during the communication, the password will be padded with a fill character (different for
   /// each protocol). For example, if an 8 character password is set to "1111", then the
   /// password "1111 " is transmitted during communications, while Password property will continue to
   /// report "1111". The padded characters are significant, and the password stored in the meter must
   /// exactly match the password used during communications (including the fill characters) or
   /// an error is generated.
   ///
   /// \default_value
   ///    - Twenty ASCII zero digits ("00000000000000000000") for the \ref MProtocolC1218, \ref MProtocolC1221, and \ref MProtocolC1222.
   ///
   /// \possible_values
   ///    - "12345678901234567890" : Passwords for the \ref MProtocolC1218, \ref MProtocolC1221, and \ref MProtocolC1222 protocols
   ///      (A3, A1800 meters) can have a maximum of 20 characters. If the specified password is less
   ///      than 20 characters, then the remaining characters are filled with blank spaces during communication.
   ///
   MByteString GetPassword() const;
   void SetPassword(const MByteString& password);
   ///@}

#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST

   ///@{
   /// Password list for the protocol.
   ///
   /// The password lists, when not empty, overwrite the \refprop{SetPassword,Password} property.
   /// In such case, passwords from the password list are applied in the ascending order
   /// until a successful entry is found. If none exists, the last error received during
   /// list processing is thrown.
   ///
   /// \seeprop{GetPassword,Password}
   /// \see ClearPasswordList - clear the password list
   /// \see AddToPasswordList - add entry to password list
   /// \see GetPasswordListSuccessfulEntry - shall be called after successful start session to know which password list worked.
   ///
   const MByteStringVector& GetPasswordList() const
   {
      return m_passwordList;
   }
   void SetPasswordList(const MByteStringVector& passwordList);
   ///@}

   /// Set the password list for the protocol to none, so the \refprop{SetPassword,Password} property is used.
   ///
   /// When the password list is not clear, property \refprop{SetPassword,Password} will not apply.
   ///
   /// \seeprop{GetPasswordList,PasswordList}
   ///
   void ClearPasswordList();

   /// Add a password to the password list.
   ///
   /// \param password Shall be valid for this protocol, or an error is thrown.
   ///
   /// \seeprop{GetPasswordList,PasswordList}
   ///
   void AddToPasswordList(const MByteString& password);

   /// Return the entry, which was successfully tried with the meter.
   ///
   /// If there was no attempt to try the password list, or when none of the entries were successful,
   /// this property will be equal to -1.
   ///
   /// \seeprop{GetPasswordList,PasswordList}
   ///
   int GetPasswordListSuccessfulEntry() const;

#endif // !M_NO_MCOM_PASSWORD_AND_KEY_LIST

   /// Clear the statistical data for the channel.
   ///
   /// Zeroes Out the COUNT_* and *_ROUND_TRIP_TIME properties.
   /// The list of properties that are reset follows:
   ///   - \refprop{GetCountApplicationLayerServicesFailed,CountApplicationLayerServicesFailed}
   ///   - \refprop{GetCountApplicationLayerServicesRetried,CountApplicationLayerServicesRetried}
   ///   - \refprop{GetCountApplicationLayerServicesSuccessful,CountApplicationLayerServicesSuccessful}
   ///   - \refprop{GetCountLinkLayerPacketsFailed,CountLinkLayerPacketsFailed}
   ///   - \refprop{GetCountLinkLayerPacketsRetried,CountLinkLayerPacketsRetried}
   ///   - \refprop{GetCountLinkLayerPacketsSuccessful,CountLinkLayerPacketsSuccessful}
   ///   - \refprop{GetMaximumRoundTripTime,MaximumRoundTripTime}
   ///   - \refprop{GetMinimumRoundTripTime,MinimumRoundTripTime}
   ///   - \refprop{GetAverageRoundTripTime,AverageRoundTripTime}
   ///
   /// If the channel is present, its own \ref MChannel.ResetCounts() is called and channel's counts are reset.
   ///
   /// \see MChannel::ResetCounts()
   ///
   void ResetCounts();

   /// Setup the configuration of the channel in a way compatible with the protocol handshake sequence.
   ///
   /// The action depends on the channel and protocol. Implementations
   /// ensure that the service behaves gracefully no matter whether
   /// the channel is connected or not.
   ///
   virtual void ApplyChannelParameters();

   /// Initializes the channel and establishes the connection with the peer.
   ///
   /// Prior to calling \ref MChannel.Connect(), this protocol method does extra checks,
   /// and calls \ref ApplyChannelParameters(). Unless \ref MChannel is used without MProtocol,
   /// it is recommended to use protocol's Connect() method.
   ///
   /// \see MChannel.Connect() - this is where per-channel specifics of Connect are exhaustively described.
   /// \see Disconnect() - operation that undoes the connection.
   ///
   void Connect();

   /// Severs the connection between the computer and the end device.
   ///
   /// Unless \ref MChannel is used without MProtocol,
   /// it is recommended to use protocol's Disconnect method.
   ///
   /// Include Disconnect in error handling routines, otherwise, other applications may not be
   /// able to connect to the end device.
   ///
   /// \see MChannel.Disconnect() - this is where per-channel specifics of Disconnect are exhaustively described.
   /// \see Connect() - operation that sets up the connection.
   ///
   void Disconnect();

   /// Tells whether the protocol is currently connected.
   ///
   /// This method is a straightforward facade to \ref MChannel.IsConnected.
   /// It reflects the assumption the communication component has about the channel
   /// state, it does not guarantee that the next action with the channel will be successful.
   /// Issue the Connect() service to establish the connection and the Disconnect() service to
   /// terminate the connection.
   ///
   /// \default_value False
   ///
   /// \possible_values
   ///  - True  : The connection has been established.
   ///  - False : The connection has not been established.
   ///
   /// \see IsInSession
   /// \see MChannel.IsConnected
   /// \see Connect
   /// \see Disconnect
   ///
   bool IsConnected() const;

   /// Whether the protocol is in session.
   ///
   /// This is a generic method, and the meaning of 'session' depends on protocol type and mode.
   /// The notion of a session exists in most protocols, 
   /// and for these protocols the property indicates whether MeteringSDK
   /// considers the device to be in session mode.
   /// The current implementation, however, does not respect session timeout,
   /// therefore, when the client protocol is in session, the device might have timed out already.
   ///
   /// Some protocols such as \ref MProtocolC1222
   /// do not have session state, however for them this property will behave as if
   /// the notion of a session is present, such as:
   ///   - Initially, IsInSession is false
   ///   - StartSession, if successful, sets IsInSession to true even though
   ///     it might not result in any communication
   ///   - Any successful table or function operation sets IsInSession to true
   ///   - EndSession, even a dummy one, sets IsInSession to false
   ///   - Disconnect sets IsInSession to false
   ///
   /// When both this property and \refprop{GetKeepSessionAlive,KeepSessionAlive} are true,
   /// and session keeping is implemented for this protocol type or mode, the value of IsInSession
   /// will indicate whether the session keeping is being done.
   /// Session keeping sequence, however, can be interrupted by an error.
   ///
   /// \possible_values
   ///  - True  : The protocol is in session state.
   ///  - False : The protocol is not in session.
   ///
   /// \see IsConnected
   ///
   bool IsInSession() const;

#if !M_NO_PROGRESS_MONITOR

   /// Create root of the progress actions hierarchy.
   ///
   /// Any existing hierarchy is destroyed.
   /// If progress monitor is not set, dummy action is returned.
   /// A dummy action implements all action's properties and services as no-ops
   ///
   MProgressAction* CreateRootProgressAction();

   /// Read-only access to the currently preset local action in progress monitor.
   ///
   /// If progress monitor is not set, or if it does not have local action, dummy action is returned.
   ///
   MProgressAction* GetLocalProgressAction();

   ///@{
   /// Access to the progress monitor, if exists
   ///
   MProgressMonitor* GetProgressMonitor() const
   {
      return m_progressMonitor;
   }
   void SetProgressMonitor(MProgressMonitor* p)
   {
      m_progressMonitor = p;
   }
   ///@}

#endif //!M_NO_PROGRESS_MONITOR

#if !M_NO_MCOM_COMMAND_QUEUE
public: // Queue services:

   ///@{
   /// Get the command queue of the protocol.
   ///
   /// If the items were added to the queue by Q services,
   /// and no QCommit is performed yet, the queue is not empty.
   ///
   MCommunicationQueue& GetCommandQueue()
   {
      return m_queue;
   }
   const MCommunicationQueue& GetCommandQueue() const
   {
      return m_queue;
   }
   ///@}

#if !M_NO_MCOM_PROTOCOL_THREAD

   /// Whether or not it is time to call QCommit(true) in order to sync with the background thread.
   ///
   /// In case M_NO_MCOM_PROTOCOL_THREAD is nonzero during compilation, this service is not present.
   /// Asynchronous communication will not be supported in this case.
   ///
   /// \see \ref QIsDone that combines QNeedToCommit with \ref QCommit when the communication finishes.
   ///
   bool QNeedToCommit() const;

   /// True if the background communication is still progressing.
   ///
   /// This is different from \ref QIsDone in the sense that it will be true only if there is a background activity.
   /// \code
   ///    //             ... QNeedToCommit() == false, QIsBackgroundCommunicationProgressing() == false
   ///    protocol.QConnect
   ///    protocol.QStartSession
   ///    protocol.QEndSession
   ///    protocol.QDisconnect
   ///    //             ... QNeedToCommit() == false, QIsBackgroundCommunicationProgressing() == false
   ///    protocol.QCommit(true);
   ///    //             ... QNeedToCommit() == false, QIsBackgroundCommunicationProgressing() == true
   ///    //             ...
   ///    while ( !protocol.QNeedToCommit() )
   ///        MUtilities::Sleep(200);
   ///    //             ... QNeedToCommit() == true, QIsBackgroundCommunicationProgressing() == true
   ///    protocol.QCommit(false);
   ///    //             ... QNeedToCommit() == false, QIsBackgroundCommunicationProgressing() == false
   /// \endcode
   ///
   bool QIsBackgroundCommunicationProgressing() const
   {
      return m_backgroundCommunicationIsProgressing;
   }

   /// Combines \ref QNeedToCommit with the following \ref QCommit in case all commands in the queue have been sent.
   ///
   /// The relationship of this property with \ref QNeedToCommit is shown below:
   /// \code
   ///    //             ... QIsDone() == true, QNeedToCommit() == false
   ///    protocol.QConnect
   ///    protocol.QStartSession
   ///    protocol.QEndSession
   ///    protocol.QDisconnect
   ///    //             ... QIsDone() == true, QNeedToCommit() == false
   ///    protocol.QCommit(true);
   ///    //             ... QIsDone() == false, QNeedToCommit() == true
   ///    //             ...
   ///    while ( !protocol.QIsDone() )
   ///        MUtilities::Sleep(200);
   ///    //             ... QIsDone() == true, QNeedToCommit() == true
   ///    //             ... No need to call meter.QCommit(false)
   /// \endcode
   ///
   /// This property is used when QCommit was issued asynchronously.
   /// During asynchronous communications, the application is free to perform other work
   /// while waiting for the communications to complete. The application can periodically
   /// check QIsDone to determine if MCOM has completed processing the commands in the queue.
   /// For asynchronous communications, QIsDone will raise any communication errors.
   /// (For synchronous communications, any errors will be raised on QCommit.)
   ///
   /// Asynchronous communications should check QIsDone until it reports True or raises an error.
   /// The asynchronous communication background thread can not terminate until QIsDone has been
   /// checked and it reports True or raises an error. If QIsDone is not checked until one of
   /// these states occur, then the next Q Command (such as QStartSession), will raise an error,
   /// even if the communications have completed. Most likely the error will indicate "Invalid
   /// operation during active background communication".
   ///
   /// Note that QIsDone and \ref QAbort are issued immediately, they do NOT require a \ref QCommit.
   ///
   bool QIsDone();

#endif // !M_NO_MCOM_PROTOCOL_THREAD

   /// Clears the commands in the queue, or cancel the ongoing background communication.
   ///
   /// If the queue is empty when the QAbort is issued, the queue remains empty and no error is
   /// generated. If the queue has commands, but they are not being processed (QCommit has not
   /// been issued), then the queue is cleared and no error is generated.
   ///
   /// \ifnot M_NO_MCOM_PROTOCOL_THREAD
   /// If the queue is being executed asynchronously (QCommit(true) then QAbort
   /// will tell MCOM to stop executing the commands in the queue and clear all remaining
   /// commands from the queue. If the abort is requested while a table is being written, the
   /// write request is completed before the abort request will be processed. No attempt is made
   /// to end (or clean up) the session. Note that QAbort returns immediately, it does not wait
   /// for the communications to stop. How long it takes for MCOM to respond to the QAbort
   /// request depends on the protocol and the tasks running on the PC. Typically, one could
   /// expect the time to be less than 500 mSec. To determine when communications have been
   /// terminated, \ref QIsDone must be checked. For communications that have been stopped by a
   /// QAbort, \ref QIsDone will raise an error. If \ref QIsDone is not checked and allowed to raise the
   /// error, then the next Q Command (such as QStartSession), will raise an error. Most likely
   /// the error will indicate "Invalid operation during active background communication". This
   /// is because the asynchronous communication background thread can not terminate while there
   /// is an error to hand back to the foreground thread that started the asynchronous
   /// communications. Asynchronous communications should always check \ref QIsDone until it is True
   /// or raises an error.
   ///
   /// Note that QAbort and \ref QIsDone are issued immediately, they do NOT require a \ref QCommit.
   /// \else
   /// This release of MeteringSDK does not have background communication feature
   /// \endif
   ///
   void QAbort();

   /// Add the message to write to the Monitor log file to MProtocol's command queue.
   ///
   /// \ifnot M_NO_MCOM_MONITOR
   /// The QWriteToMonitor method can be used to make communication transactions more readable
   /// by marking key data transactions, such as the start/end of a test.
   ///
   /// QWriteToMonitor must be issued after the connection is established with the end device
   /// (\ref Connect or \ref QConnect). QWriteToMonitor is different than \ref MMonitor.Write in that Write
   /// sends the message to the monitor immediately, whereas QWriteToMonitor is executed as part
   /// of the command queue on \ref QCommit.
   ///
   /// No error is thrown if the Monitor is not listening or the log file is not accumulating.
   ///
   /// \param message The message to write to the Monitor log file.
   ///
   /// \see WriteToMonitor - no-queue version of the method
   /// \else
   /// Monitor feature is not present in this release of MeteringSDK and this method does nothing.
   /// \endif
   ///
   void QWriteToMonitor(const MStdString& message);

   /// Places a Connect command in the queue.
   ///
   /// The QConnect can be paired with \ref QDisconnect or \ref Disconnect.
   /// Protocol supports synchronous and asynchronous communications.
   ///
   /// \ifnot M_NO_MCOM_PROTOCOL_THREAD
   /// When synchronous communications are initiated, \ref QCommit will not return until all commands in the queue have
   /// been executed or an error occurs. This prevents the calling application from performing
   /// any other work on that thread. When asynchronous communications are initiated, the \ref QCommit
   /// returns immediately while the commands in the queue are executed in the background. The
   /// calling application is free to perform other work while waiting for the communications to
   /// complete. The application will have to check \ref QIsDone to determine when asynchronous
   /// communications have completed. The application can also call \ref QAbort to stop and clear the
   /// commands from the queue. \ref QAbort can be useful for Connect sequences that take a long time
   /// (such as the 30-60 seconds it takes to connect to a modem).
   /// \endif
   ///
   void QConnect();

   /// Places a Disconnect command in the queue.
   ///
   /// The QDisconnect can be paired with \ref QConnect or \ref Connect.
   /// Note that an commands are cleared from the queue when an error occurs. If the QDisconnect
   /// is placed in the queue and an error occurs before it can be issued, it will be cleared
   /// from the queue along with the remaining unexecuted commands. Error handling routines must
   /// issue a \ref Disconnect (or QDisconnect/\ref QCommit), in order to disconnect from the channel. For
   /// this reason, it is recommended that users use \ref Disconnect instead of \ref QDisconnect for most
   /// cases.
   ///
   /// \ifnot M_NO_MCOM_PROTOCOL_THREAD
   /// Protocol supports synchronous and asynchronous communications. When synchronous
   /// communications are initiated, \ref QCommit will not return until all commands in the queue have
   /// been executed or an error occurs. This prevents the calling application from performing
   /// any other work on that thread. When asynchronous communications are initiated, the \ref QCommit
   /// returns immediately while the commands in the queue are executed in the background. The
   /// calling application is free to perform other work while waiting for the communications to
   /// complete. The application will have to check \ref QIsDone to determine when asynchronous
   /// communications have completed. The application can also call \ref QAbort to stop and clear the
   /// commands from the queue.
   /// \endif
   ///
   void QDisconnect();

#if !M_NO_MCOM_IDENTIFY_METER
   /// Places an IdentifyMeter task in the queue.
   ///
   /// The commands stored in the command queue will be sent to the meter when \ref QCommit is issued.
   /// The identify meter task identifies the meter version and other information if the protocol
   /// is known (note that this is not an ANSI Identify protocol command). Typically,
   /// QIdentifyMeter is used when creating a meter of a known type, but an unknown version.
   ///
   /// The QIdentifyMeter service does NOT wrap the start and end session around the
   /// identification task. To use QIdentifyMeter, one has to connect to the meter and start the
   /// session. This differs from \ref IdentifyMeter which wraps start and end session around the
   /// identification task.
   ///
   /// The identification information is retrieved with \ref QGetIdentifyMeterData after
   /// QIdentifyMeter has been successfully QCommit-ted (all requests in queue sent to the
   /// meter).
   ///
   void QIdentifyMeter();
#endif // !M_NO_MCOM_IDENTIFY_METER

   /// Adds a start session command to MProtocol's command queue.
   ///
   /// QStartSession performs the protocol services required to gain access to the meter, so that
   /// tables can be read and written and functions can be executed. Before committing the start
   /// session request, a connection with the meter must have been established using Connect() or
   /// \ref QConnect.
   ///
   /// When QStartSession is executed by \ref QCommit, an active session with the meter is initiated.
   /// The start session action is not a single protocol command but rather a sequence of
   /// commands that establish an active communication session with the meter. Generally this
   /// sequence of commands handles handshaking with the meter, presenting a password, receiving
   /// permission to proceed, and negotiating various protocol settings. The exact sequence of
   /// steps performed by this method depends on the MProtocol Type.
   /// If the protocol does not support a start session, then
   /// QStartSession does nothing and does not generate an error. This is so programs that
   /// support multiple meters and protocols can be written with the QStartSession and
   /// \ref QEndSession.
   ///
   /// \code
   ///     protocol.QStartSession()
   ///     protocol.QTableWrite(tableNum, data)
   ///     protocol.QEndSession()
   ///     protocol.QCommit()
   /// \endcode
   ///
   void QStartSession();

   /// Adds an end session command to MProtocol's command queue.
   ///
   /// When QEndSession is executed by a \ref QCommit, the active session with the meter will be
   /// terminated. Typically, this is done using the protocol's logoff service. If the protocol
   /// does not support a start/end session, such as sessionless \ref MProtocolC1222, then QEndSession does
   /// nothing and does not generate an error. This is so programs that support multiple meters
   /// and protocols can be written with the \ref QStartSession and \ref QEndSession.
   ///
   /// For protocols that support start/end session, once an QEndSession command is executed, no
   /// other commands can be sent to the meter until after the next QStartSession.
   /// The communications link to the meter will not be severed by QEndSession. For instance, a
   /// modem channel will remain on-line after this operation. To sever the communications link
   /// to the meter requires using the Disconnect() method. The Disconnect method should only be
   /// used when no more communications to the meter are required.
   ///   \code
   ///     protocol.QStartSession()
   ///     protocol.QTableWrite(tableNum, data)
   ///     protocol.QEndSession()
   ///     protocol.QCommit()
   ///   \endcode
   ///
   /// \see QEndSessionNoThrow - silences any errors if they appear
   ///
   void QEndSession();

   /// EndSessionNoThrow request is queued.
   ///
   /// This service is part of queue communication interface.
   /// In all ways it is similar to \ref QEndSession except that it silently swallows any error,
   /// should it appear. Therefore, this variant is very convenient in error handlers that
   /// need to ensure that the session has ended.
   ///
   /// \see QEndSession - throws errors
   ///
   void QEndSessionNoThrow();

   /// Adds a ReadTable command to MProtocol's command queue.
   ///
   /// This service is part of queue communication interface.
   /// The result table data can be retrieved by using the \ref QGetTableData method.
   ///
   /// \param number Identifies the table to be placed into the queue. The format is protocol dependent.
   ///    In can be a numeric value, as for the case of \ref MProtocolC1218 or \ref MProtocolC1222,
   ///    or even a string if the protocol supports strings.
   ///
   /// \param expectedSize Expected table size in bytes. If this is zero, then the entire table
   ///     will be read. If the protocol does not support full table reads
   ///     then specifying a size of zero will result in an error being generated.
   ///
   /// \param id Operation identifier to associate with the data returned from the device.
   ///     When processing tables or functions with different numbers, this can be any value,
   ///     as tables and functions can be identified by their numbers,
   ///     however when using the same table or function number in the same queue,
   ///     this ID should be unique, so the identification is possible.
   ///
   void QTableRead(MCOMNumberConstRef number, unsigned expectedSize, int id);

   /// Adds a table write command to MProtocol's command queue.
   ///
   /// This service is part of queue communication interface.
   ///
   /// \param number Identifies the table to be placed into the queue. The format is protocol dependent.
   ///    In can be a numeric value, as for the case of \ref MProtocolC1218 or \ref MProtocolC1222,
   ///    or even a string if the protocol supports it.
   ///
   /// \param data This buffer contains the data that will be written to the meter. The size
   ///    of data determines the size of the buffer to be written.
   ///
   void QTableWrite(MCOMNumberConstRef number, const MByteString& data);

   /// Adds a partial table read command to MProtocol's command queue.
   ///
   /// This service is part of queue communication interface.
   ///
   /// \param number Identifies the table to be placed into the queue. The format is protocol dependent.
   ///    In can be a numeric value, as for the case of \ref MProtocolC1218 or \ref MProtocolC1222,
   ///    or even a string if the protocol supports it.
   ///
   /// \param offset The number of bytes from the beginning of the table where the operation will begin.
   ///
   /// \param size The number of bytes to be read starting from offset.
   ///
   /// \param id Operation identifier to associate with the data returned from the device.
   ///     When processing tables or functions with different numbers, this can be any value,
   ///     as tables and functions can be identified by their numbers,
   ///     however when using the same table or function number in the same queue,
   ///     this ID should be unique, so the identification is possible.
   ///
   void QTableReadPartial(MCOMNumberConstRef number, int offset, int size, int id);

   /// Adds a partial table write command to MProtocol's command queue.
   ///
   /// This service is part of queue communication interface.
   ///
   /// \param number Identifies the table to be placed into the queue. The format is protocol dependent.
   ///    In can be a numeric value, as for the case of \ref MProtocolC1218 or \ref MProtocolC1222,
   ///    or even a string if the protocol supports it.
   ///
   /// \param data This buffer contains the data that will be written to the meter. The size
   ///    of data determines the size of the buffer to be written.
   ///
   /// \param offset The number of bytes from the beginning of the table where the write operation will begin.
   ///
   void QTableWritePartial(MCOMNumberConstRef number, const MByteString& data, int offset);

   /// Place a function without data request in MProtocol command queue.
   ///
   /// This service is part of queue communication interface.
   ///
   /// \param number Identifies the function to be placed into the queue. Typically, this is
   ///     an integer, but some protocols 
   ///     do allow it to be a string.
   ///
   void QFunctionExecute(MCOMNumberConstRef number);

   /// Place a function with request data in MProtocol's command queue.
   ///
   /// This service is part of queue communication interface.
   ///
   /// \param number Identifies the function to be placed into the queue. Typically, this is
   ///     an integer, but some protocols 
   ///     do allow it to be a string.
   ///
   /// \param request The function data to send to the meter. It must be in the form of a
   ///     byte array and must have the number of bytes that corresponds to the specified function.
   ///
   void QFunctionExecuteRequest(MCOMNumberConstRef number, const MByteString& request);

   /// Place a function with response data in MProtocol's command queue.
   ///
   /// This service is part of queue communication interface.
   /// The response data will be available after the \ref QCommit method is issued. Use the
   /// \ref QGetFunctionData to retrieve the bytes.
   ///
   /// \param number Identifies the function to be placed into the queue. Typically, this is
   ///     an integer, but some protocols 
   ///     do allow it to be a string.
   ///
   /// \param estimatedResponseSize optional parameter. If provided, this is the size of the response.
   ///     If not provided, this is estimated to be large enough to fit any possible value.
   ///
   /// \param id Operation identifier to associate with the data returned from the device.
   ///     When processing tables or functions with different numbers, this can be any value,
   ///     as tables and functions can be identified by their numbers,
   ///     however when using the same table or function number in the same queue,
   ///     this ID should be unique, so the identification is possible.
   ///
   void QFunctionExecuteResponse(MCOMNumberConstRef number, int id, unsigned estimatedResponseSize = DEFAULT_ESTIMATED_RESPONSE_SIZE);

   /// Place a function with request and response data in MProtocol's command queue.
   ///
   /// This service is part of queue communication interface.
   /// The response data will be available after the \ref QCommit method is issued. Use the
   /// \ref QGetFunctionData method to retrieve the bytes.
   ///
   /// \param number Identifies the function to be placed into the queue. Typically, this is
   ///     an integer, but some protocols 
   ///     do allow it to be a string.
   ///
   /// \param request The function data to send to the meter. It must be in the form of a
   ///    byte array and must have the number of bytes that corresponds to the specified function.
   ///
   /// \param id Operation identifier to associate with the data returned from the device.
   ///     When processing tables or functions with different numbers, this can be any value,
   ///     as tables and functions can be identified by their numbers,
   ///     however when using the same table or function number in the same queue,
   ///     this ID should be unique, so the identification is possible.
   ///
   /// \param estimatedResponseSize optional parameter. If provided, this is the size of the response.
   ///    If not provided, this is estimated to be large enough to fit any possible value.
   ///
   void QFunctionExecuteRequestResponse(MCOMNumberConstRef number, const MByteString& request, int id, unsigned estimatedResponseSize = DEFAULT_ESTIMATED_RESPONSE_SIZE);


   /// Executes all operations in MProtocol's command queue.
   ///
   /// If no operations were queued then QCommit does nothing.
   ///
   /// Protocol supports synchronous and asynchronous communications. When synchronous
   /// communications are initiated, QCommit will not return until all commands in the queue have
   /// been executed or an error occurs. This prevents the calling application from performing
   /// any other work on that thread. When asynchronous communications are initiated, the QCommit
   /// will return immediately while the commands in the queue are executed in the background.
   /// The calling application is free to perform other work while waiting for the communications
   /// to complete. The application will have to check QIsDone to determine when asynchronous
   /// communications have completed. The application can also call QAbort to stop and clear the
   /// commands from the queue.
   ///
   /// Should an error occur during synchronous communications, it will be reported immediately
   /// on the QCommit. Should an error occur during asynchronous communications, the error is
   /// raised on QIsDone. In either case, no attempt is made to execute the remaining commands in
   /// the queue.
   ///
   /// The command queue will always be cleared after a QCommit, regardless of success or failure
   /// of the operations that were queued.
   ///
   /// \param asynchronously Whether to perform the communication on the background, asynchronously.
   ///
   /// The following sequences are completely equivalent:
   ///    \code
   ///        protocol.Connect()             // synchronously connect
   ///        protocol.StartSession()        // synchronously start the session
   ///        table = protocol.TableRead(0)  // read table zero
   ///        protocol.EndSession()          // here end the session
   ///        protocol.Disconnect()          // synchronously disconnect
   ///    \endcode
   /// or:
   ///    \code
   ///        protocol.QConnect();          // declare we will be connecting
   ///        protocol.QStartSession();     // declare we will be starting session
   ///        protocol.QTableRead(0, 0, 0)  // queue table read, have no expectation about its size
   ///        protocol.QEndSession()        // after that we will be ending the session
   ///        protocol.QDisconnect();       // declare we will be disconnecting
   ///        protocol.QCommit()            // commit the will, the sequence is executed
   ///        table = protocol.QGetTableData(0, 0); // get the result data or table 0, id 0
   ///    \endcode
   /// Case of background communication in a separate thread:
   ///    \code
   ///        protocol.QConnect();          // declare we will be connecting
   ///        protocol.QStartSession();     // declare we will be starting session
   ///        protocol.QTableRead(0, 0, 0)  // queue table read, have no expectation about its size
   ///        protocol.QEndSession()        // after that we will be ending the session
   ///        protocol.QDisconnect();       // declare we will be disconnecting
   ///        protocol.QCommit(true);       // commit on the background
   ///        while ( !protocol->QIsDone() )
   ///        {
   ///             .... do anything
   ///        }
   ///        table = protocol.QGetTableData(0, 0); // get the result data or table 0, id 0
   ///    \endcode
   ///
   virtual void QCommit(bool asynchronously = false);

   /// Fetch the table data after the table read has been successfully performed by QCommit.
   ///
   /// The data remains available after commit has performed, but until the next queue starts to be built.
   ///
   /// \param number Number of the table that was read, whatever is given
   ///     to \ref QTableRead or \ref QTableReadPartial
   ///
   /// \param id If the same table was read several times in the same session, use this
   ///     id to distinguish between individual calls of \ref QTableRead or \ref QTableReadPartial.
   ///
   /// \return Table data bytes
   ///
   MByteString QGetTableData(MCOMNumberConstRef number, int id = -1);

   /// Fetch the function response data after the function has been successfully executed in QCommit.
   ///
   /// The data remains available after commit has performed, but until the next queue starts to be built.
   ///
   /// \param number Number of the function that was executed, whatever is given
   ///     to \ref QFunctionExecuteResponse or \ref QFunctionExecuteRequestResponse
   ///
   /// \param id If the same table was read several times in the same session, use this
   ///     id to distinguish between individual calls of \ref QTableRead or \ref QTableReadPartial.
   ///
   /// \return Function data bytes.
   ///
   MByteString QGetFunctionData(MCOMNumberConstRef number, int id = -1);


#if !M_NO_MCOM_IDENTIFY_METER

   /// Fetch the identify meter string after the QIdentifyMeter has been successfully performed in QCommit.
   ///
   /// Typically, QGetIdentifyMeterData returns only one identify string that identifies the
   /// meter. However, identify strings can be created for the option boards installed on a
   /// meter. In this case, the returned string can contain multiple identify strings, where the
   /// the meter's identify string is listed first and identify strings are separated by a semi-
   /// colon. Use GetIdentifyStrings to separate multiple identify strings.
   ///
   /// The identify string is formatted as a J command and may not be suitable for showing to the
   /// user as it can contain non-printable characters. Instead, it is formatted for easy
   /// processing by a script or a program. A typical identify string returned for an A3 meter
   /// with LANOB option board follows (wrapped for clarity):
   /// \code
   ///     J00[MANUFACTURER:EE ] // A3's identify string //
   ///         [ED_MODEL:A3R ]
   ///         [HW_VERSION_REVISION:1.2]
   ///         [SW_VERSION_REVISION:2.3]
   ///         [MFG_SERIAL_NUMBER: ]
   ///         [NUMBER_OF_OPTION_BOARDS:2]
   ///         [OB1_Type:0M]
   ///         [OB1_SSPEC:000237]
   ///         [OB1_REVISION:1.0]
   ///         [OB1_POSITION:1]
   ///         [OB2_Type:0K]
   ///         [OB2_SSPEC:000239]
   ///         [OB2_REVISION:1.3]
   ///         [OB2_POSITION:2];
   ///     J00[MANUFACTURER:EE ] // LANOB's identify string //
   ///         [ED_MODEL:REXILC ]
   ///         [HW_VERSION_REVISION:1.0]
   ///         [SW_VERSION_REVISION:1.3]
   ///         [MFG_SERIAL_NUMBER:LANOB ]
   /// \endcode
   ///
   /// \return Result identify string
   ///
   MStdString QGetIdentifyMeterData();

#endif // !M_NO_MCOM_IDENTIFY_METER

private:    // internal queue implementation

   // Add command to queue.
   //
   // \pre If command has response, command with such response and identification shall not be present
   // in the queue already.
   //
   void DoAddCommandToQueue(MCommunicationCommand* command);

public: // Semi-private methods:
/// \cond SHOW_INTERNAL

   /// Executes all operations in MProtocol's command queue in synchronous mode.
   ///
   /// \see QCommit(bool asynchronously = false)
   ///
   void DoQCommit0()
   {
      QCommit(); // reflection's necessity
   }

   // Implementation of actual Commit synchronous sequence, minus all possible checks and balances.
   //
   // \pre Shall be called from QCommit.
   //
   virtual void DoQCommit();

#endif // !M_NO_MCOM_COMMAND_QUEUE

   // Helper method that connects without checking whether it was called from a background thread
   //
   // \pre The preconditions specific to the channel are
   // applied. The channel should not be connected. This set of
   // conditions can lead to an exception to be thrown.
   //
   void DoConnect();

/// \endcond SHOW_INTERNAL
public: // Services:

   /// Synchronously write a message to the monitor, if it is connected
   ///
   /// \ifnot M_NO_MCOM_MONITOR
   /// No error is thrown if the Monitor is not listening or the log file is not accumulating.
   ///
   /// \param message The message to write to the Monitor log file.
   ///
   /// \see QWriteToMonitor - queue based version of this service.
   /// \else
   /// Monitor feature is not present in this release of MeteringSDK and this method does nothing.
   /// \endif
   ///
   virtual void WriteToMonitor(const MStdString& message);

   /// Write running values of communication quality counters to monitor.
   ///
   /// This is a convenience method for all types of troubleshooting.
   /// No errors are thrown. If there is no monitor connected, nothing is done.
   ///
   void WriteCountsToMonitor();

   /// Synchronously start the session.
   ///
   /// Depending on the particular protocol this leads to a sequence of
   /// handshake and logon commands.
   ///
   /// See \ref QStartSession for the queue version of this service.
   /// Q services would work faster for the queue based protocols,
   /// but slower for synchronous protocols like C12.
   /// Also, they are obviously less convenient for synchronous operations.
   ///
   /// \pre The channel is open, the protocol state allows starting the session.
   /// Otherwise exceptions can be thrown.
   ///
   void StartSession();

   /// Synchronously end the session.
   ///
   /// Depending on the particular protocol this leads to sequence of
   /// logoff and terminate commands.
   ///
   /// See \ref QEndSession for the queue version of this service.
   /// Q services would work faster for the queue based protocols,
   /// but slower for synchronous protocols like C12.
   /// Also, they are obviously less convenient for synchronous operations.
   ///
   /// \pre The channel is open, the protocol state allows ending the session.
   /// Otherwise exceptions can be thrown.
   ///
   /// \see EndSessionNoThrow for the variant that does not throw errors.
   ///
   void EndSession();

   /// End the session, but do not throw errors.
   ///
   /// \see EndSession for the variant that throws errors.
   ///
   void EndSessionNoThrow() M_NO_THROW;

   /// Read the start byte of the packet in a proper way, taking into consideration timeouts and ignoring garbage.
   ///
   /// \param setOfValidStartBytes - sequence of bytes that are valid start characters.
   /// \param trafficTimeout - how long to wait for arrival of start byte.
   ///
   /// \return the character that was successfully received, one among setOfValidStartBytes.
   ///
   /// \pre The start character among those in the set given should be present in the input stream
   /// within trafficTimeout period. Otherwise an exception is thrown.
   ///
   char ReadStartByte(const MByteString& setOfValidStartBytes, unsigned trafficTimeout);


   /// Synchronously read the whole table with number given as parameter.
   ///
   /// See QTableRead for the queue version of this service.
   /// Q services would work faster for the queue based protocols,
   /// but slower for synchronous protocols like C12.
   /// Also, they are obviously less convenient for synchronous operations.
   ///
   /// The third argument is introduced to handle a special case where the table 
   /// size is bigger than what the protocol allows, such as configuring an ANSI meter
   /// to have more load profile data than the 64K that can be transmitted with
   /// ANSI C12.18. In this case the table is read with multiple partial reads, 
   /// where each partial read is sized to fit within the protocol constraints.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   /// \param number Table number.
   /// \param expectedSize If given, should match exactly the size of the table.
   /// 
   /// \return Table data bytes
   ///
   MByteString TableRead(MCOMNumberConstRef number, unsigned expectedSize = 0);

   /// Same as ReadTable, but instead of returning a byte string, read table into a given buffer.
   ///
   /// \pre In addition to all preconditions of ReadTable,
   /// the given buffer shall match the table size exactly, or a size mismatch exception is thrown.
   ///
   /// \param number Table number.
   /// \param buff Where to return the table bytes.
   /// \param size Size of the given buffer and size of the table.
   ///
   void TableReadBuffer(MCOMNumberConstRef number, void* buff, unsigned size);

   /// Same as ReadTable, but instead of returning a byte string, read table into a given template variable.
   ///
   /// \pre In addition to all preconditions of ReadTable,
   /// the given buffer shall match the table size exactly, or a size mismatch exception is thrown.
   ///
   template
      <class T>
   void TableReadBuffer(MCOMNumberConstRef number, T& table)
   {
      TableReadBuffer(number, (void*)&table, sizeof(table));
   }

   /// Synchronously read the whole table with number given as parameter,
   /// do not throw an exception, but rather return it.
   ///
   /// See QTableRead for the queue version of this service.
   /// Q services would work faster for the queue based protocols,
   /// but slower for synchronous protocols like C12.
   /// Also, they are obviously less convenient for synchronous operations.
   ///
   /// The second argument is introduced to handle exceptions internally and return
   /// the data that was successfully read before the exception was raised.
   ///
   /// The third argument is introduced to handle a special case where the table 
   /// size is bigger than what the protocol allows, such as configuring an ANSI meter
   /// to have more load profile data than the 64K that can be transmitted with
   /// ANSI C12.18. In this case the table is read with multiple partial reads, 
   /// where each partial read is sized to fit within the protocol constraints.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown. The exception is not NULL, there is a debug check.
   ///
   /// \param number Table number.
   /// \param exception Returned new object, exception, if it was thrown during the operation.
   ///      If not NULL, it has to be deleted by the caller.
   /// \param expectedSize If given, should match exactly the size of the table.
   ///
   /// \return Table data bytes.
   ///
   MByteString TableReadNoThrow(MCOMNumberConstRef number, MException** exception, unsigned expectedSize = 0);

   /// Synchronously write the whole table with number given as parameter.
   ///
   /// See QTableWrite for the queue version of this service.
   /// Q services would work faster for the queue based protocols,
   /// but slower for synchronous protocols like C12.
   /// Also, they are obviously less convenient for synchronous operations.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   /// \param number Table number.
   /// \param data Table data to be written.
   ///
   void TableWrite(MCOMNumberConstRef number, const MByteString& data);

   /// Same as TableWrite, but uses buffer, given as data and size parameters.
   ///
   /// \param number Table number.
   /// \param data Pointer to table data to be written.
   /// \param size Size of the table data.
   ///
   /// \see \ref TableWrite
   ///
   void TableWriteBuffer(MCOMNumberConstRef number, const void* data, unsigned size);

   /// Same as WriteTable, but uses variable of some specific template class or structure.
   ///
   /// \param number Table number.
   /// \param table Table raw structure or class.
   ///
   /// \see \ref TableWrite
   ///
   template
      <class T>
   void TableWriteBuffer(MCOMNumberConstRef number, const T& table)
   {
      TableWriteBuffer(number, (const void*)&table, sizeof(table));
   }

   /// Synchronously read part of the table with number given as parameter.
   ///
   /// See QTableReadPartial for the queue version of this service.
   /// Q services would work faster for the queue based protocols,
   /// but slower for synchronous protocols like C12.
   /// Also, they are obviously less convenient for synchronous operations.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   /// \param number Table number.
   /// \param offset Offset within the table.
   /// \param size Size within the table starting from the offset.
   /// \return Table data bytes.
   ///
   MByteString TableReadPartial(MCOMNumberConstRef number, int offset, int size);

   /// Same as TableReadPartial, but instead of returning a byte string, read table into a given buffer.
   ///
   /// \param number Table number.
   /// \param offset Offset within the table.
   /// \param buff Where to return the table bytes.
   /// \param size Size of the partial chunk and size of buff.
   /// \return Table data bytes.
   ///
   void TableReadPartialBuffer(MCOMNumberConstRef number, int offset, void* buff, unsigned size);

   /// Same as TableReadPartial, but instead of returning a byte string, partially read table into a given template variable.
   ///
   /// \param number Table number.
   /// \param table Table raw structure or class.
   /// \param offset Offset within the table.
   ///
   template
      <class T>
   void TableReadPartialBuffer(MCOMNumberConstRef number, T& table, int offset)
   {
      TableReadPartialBuffer(number, offset, (void*)&table, sizeof(table));
   }

   /// Synchronously write part of the table with number given as parameter.
   ///
   /// See \ref QTableWritePartial for the queue version of this service.
   /// Q services would work faster for the queue based protocols,
   /// but slower for synchronous protocols like C12.
   /// Also, they are obviously less convenient for synchronous operations.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   /// \param number Table number.
   /// \param data Table data to be written at a given offset.
   /// \param offset Offset within the table.
   ///
   void TableWritePartial(MCOMNumberConstRef number, const MByteString& data, int offset);

   /// Same as TableWritePartial, but instead of using a byte string, write table using a given buffer.
   ///
   /// \param number Table number.
   /// \param offset Offset within the table.
   /// \param buff Pointer to table data to be written at a given offset.
   /// \param size Size of the data.
   ///
   void TableWritePartialBuffer(MCOMNumberConstRef number, int offset, const void* buff, unsigned size);

   /// Same as TableWritePartial, but instead of using a byte string, write table using a given template class contents.
   ///
   /// \param number Table number.
   /// \param table Table raw structure or class.
   /// \param offset Offset within the table.
   ///
   template
      <class T>
   void TableWritePartialBuffer(MCOMNumberConstRef number, const T& table, int offset)
   {
      TableWritePartialBuffer(number, offset, (void*)&table, sizeof(table));
   }

   /// Synchronously execute the function with no parameters,
   /// the number of the function is given as parameter.
   ///
   /// See QFunctionExecute for the queue version of this service.
   /// Q services would work faster for the queue based protocols,
   /// but slower for synchronous protocols like C12.
   /// Also, they are obviously less convenient for synchronous operations.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   /// \param number Function number
   ///
   void FunctionExecute(MCOMNumberConstRef number);

   /// Synchronously execute the function with request data,
   /// the number of the function is given as parameter.
   ///
   /// See QFunctionExecuteRequest for the queue version of this service.
   /// Q services would work faster for the queue based protocols,
   /// but slower for synchronous protocols like C12.
   /// Also, they are obviously less convenient for synchronous operations.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   /// \param number Function number.
   /// \param request Request data.
   ///
   void FunctionExecuteRequest(MCOMNumberConstRef number, const MByteString& request);

   /// Synchronously execute the function with response data,
   /// the number of the function is given as parameter.
   ///
   /// See QFunctionExecuteResponse for the queue version of this service.
   /// Q services would work faster for the queue based protocols,
   /// but slower for synchronous protocols like C12.
   /// Also, they are obviously less convenient for synchronous operations.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   /// \param number Function number.
   /// \return Response data.
   ///
   MByteString FunctionExecuteResponse(MCOMNumberConstRef number);

   /// Synchronously execute the function with request and response data,
   /// the number of the function is given as parameter.
   ///
   /// See QFunctionExecuteRequestResponse for the queue version of this service.
   /// Q services would work faster for the queue based protocols,
   /// but slower for synchronous protocols like C12.
   /// Also, they are obviously less convenient for synchronous operations.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   /// \param number Function number.
   /// \param request Request data.
   /// \return Response data.
   ///
   MByteString FunctionExecuteRequestResponse(MCOMNumberConstRef number, const MByteString& request);


   /// Return the number of data link packets that are required for a given request,
   /// provided an optional size of the application data.
   /// Like for table read, this will be the expected size of the table to read,
   /// and for the table write this is the size of the table to be written.
   /// For function execution this is the sum of request and response sizes.
   ///
   /// \param typeOfRequest The type of the request, the value within the enumeration range.
   ///                      There is a debug check for the value.
   /// \param applicationLayerDataSize Table size or the sum of function request and response size,
   ///                      whatever comprises the application layer of the given command.
   ///
   /// \return Estimated number of link layer packets to process the request.
   ///
   virtual unsigned GetNumberOfDataLinkPackets(MCommunicationCommand::CommandType typeOfRequest, unsigned applicationLayerDataSize = 0) M_NO_THROW;

#if !M_NO_MCOM_IDENTIFY_METER
   /// Identify the meter if the protocol is known (note this is not an ANSI Identify protocol command).
   ///
   /// If the first parameter is true, the service assumes the session is open, and it will not
   /// start the session, or close it. The second parameter, if it is not NULL, is filled with the
   /// data from tables, which IdentifyMeter had to read from the meter during its attempts to fetch
   /// the necessary information from the meter. This table vector can be used in order to minimize
   /// the communication with the meter, if this task has its place.
   ///
   /// \pre The channel is connected. Otherwise a channel-related
   /// exception is thrown that tells about failure. The meter is indeed the one, which
   /// is able to talk the specific protocol. Otherwise the protocol-specific exception
   /// is thrown that signals the failure of communicating through the channel.
   /// If the session is started already, the first parameter has to be true,
   /// or the behavior is undefined.
   ///
   /// \param sessionIsStarted True if the method should not start and end the session as part of identification.
   /// \param tablesRead Pointer of data vector for tables that were read as part of the call.
   ///
   /// \return Identify string
   ///
   MStdString IdentifyMeterWithContext(bool sessionIsStarted = false, TableRawDataVector* tablesRead = NULL);
#endif // !M_NO_MCOM_IDENTIFY_METER

   /// Protocol dependent one-byte checksum calculation procedure that uses buffer and its size.
   ///
   /// This particular implementation of the checksum calculation
   /// fits the majority of the protocols, override if needed.
   ///
   /// \param buff The pointer to the buffer for which to calculate the checksum.
   /// \param size Size of the buffer.
   /// \return Value of the checksum, one byte as lower part of four bytes.
   ///
   /// \see \ref StaticCalculateChecksumFromBuffer Static variant of this method that implements the most common sum of bytes algorithm.
   /// \see \ref CalculateChecksum Works on byte string and returns a checksum value as unsigned integer.
   ///
   virtual unsigned CalculateChecksumFromBuffer(const char* buff, unsigned size) const;

   /// Most popular one-byte checksum calculation procedure, a sum of all bytes.
   ///
   /// The checksum is calculated based on the C12 rules.
   /// Different protocols can have their own variants of this method.
   ///
   /// \param buff The pointer to the buffer for which to calculate the checksum.
   /// \param size Size of the buffer.
   ///
   /// \return Value of the checksum, one byte as lower part of four bytes.
   ///
   /// \see \ref CalculateChecksumFromBuffer Virtual protocol dependent variant of this method.
   ///
   static unsigned StaticCalculateChecksumFromBuffer(const char* buff, unsigned size);

   /// Compute checksum of the byte string given as parameters.
   ///
   /// The checksum is calculated based on the C12 rules.
   /// Different protocols can have their own variants of this method.
   ///
   /// \param buffer The buffer for which to calculate the checksum.
   /// \return Value of the checksum, two bytes returned within the low part of four bytes.
   ///
   static unsigned StaticCalculateChecksum(const MByteString& buffer);

   /// Protocol dependent one-byte checksum calculation procedure that uses byte string.
   ///
   /// This implementation uses a protocol-dependent virtual call.
   ///
   /// \param buffer Buffer for which to calculate one-byte checksum.
   /// \return Value of the checksum, one byte returned within the low part of four bytes.
   ///
   unsigned CalculateChecksum(const MByteString& buffer) const;

   /// Abstract protocol dependent two-byte CRC calculation procedure, takes buffer chunk with the size.
   ///
   /// \param buff The pointer to the buffer for which to calculate CRC16.
   /// \param size Size of the buffer.
   /// \return Value of CRC16, two bytes.
   ///
   virtual Muint16 CalculateCRC16FromBuffer(const char* buff, unsigned size) const;

   /// Protocol dependent two-byte CRC calculation procedure that accepts the byte string.
   ///
   /// This implementation uses a protocol-dependent virtual call.
   ///
   /// \param buffer The buffer for which to calculate CRC16.
   ///
   /// \return Value of CRC16, two bytes returned within the low part of four bytes.
   ///
   unsigned CalculateCRC16(const MByteString& buffer) const
   {
      return (unsigned)CalculateCRC16FromBuffer(buffer.data(), M_64_CAST(unsigned, buffer.size()));
   }

   /// Calls channel's Sleep method if the channel is present.
   ///
   /// Channel's sleep is interruptible, and this is the difference of this method
   /// from plain \ref MUtilities::Sleep.
   ///
   /// \param milliseconds How many milliseconds to sleep.
   ///
   void Sleep(unsigned milliseconds);

#if !M_NO_REFLECTION
/// \cond SHOW_INTERNAL

   /// Synchronously read the whole table with number given as parameter.
   ///
   /// See QTableRead for the queue version of this service.
   /// Q services would work faster for the queue based protocols,
   /// but slower for synchronous protocols like C12.
   /// Also, they are obviously less convenient for synchronous operations.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   MByteString DoTableReadImpl(MCOMNumberConstRef number);

   // This is the same as ActionStart, but used by Reflection facility.
   // See ActionStart for details.
   //
   void DoActionStart1(const MStdString& name);

   // This is the same as ActionStep, but used by Reflection facility.
   // See ActionStep for details.
   //
   void DoActionStep1();

   /// FunctionExecuteResponse is queued.
   /// This service is part of queue communication interface.
   ///
   /// This method will place a function with response data in MProtocol's command queue. The
   /// response data will be available after the QCommit method is issued. Use the
   /// QGetFunctionData to retrieve the bytes.
   ///
   /// \param number Identifies the function to be placed into the queue. Typically, this is
   ///     an integer, but some protocols 
   ///     do allow it to be a string.
   ///
   /// \param id Operation identifier to associate with the data returned from the device.
   ///     When processing tables or functions with different numbers, this can be any value,
   ///     as tables and functions can be identified by their numbers,
   ///     however when using the same table or function number in the same queue,
   ///     this ID should be unique, so the identification is possible.
   ///
   void DoQFunctionExecuteResponse(MCOMNumberConstRef number, int id);

   /// FunctionExecuteRequestResponse is queued.
   ///
   /// This service is part of queue communication interface.
   /// It will place a function with request and response data in MProtocol's command
   /// queue. The response data will be available after the QCommit method is issued. Use the
   /// QGetFunctionData method to retrieve the bytes.
   ///
   /// \param number Identifies the function to be placed into the queue. Typically, this is
   ///     an integer, but some protocols 
   ///     do allow it to be a string.
   ///
   /// \param request The function data to send to the meter. It must be in the form of a
   ///     byte array and must have the number of bytes that corresponds to the specified function.
   ///
   /// \param id Operation identifier to associate with the data returned from the device.
   ///     When processing tables or functions with different numbers, this can be any value,
   ///     as tables and functions can be identified by their numbers,
   ///     however when using the same table or function number in the same queue,
   ///     this ID should be unique, so the identification is possible.
   ///
   void DoQFunctionExecuteRequestResponse(MCOMNumberConstRef number, const MByteString& request, int id);

/// \endcond SHOW_INTERNAL
#endif // !M_NO_REFLECTION

public:  // reflection helpers

#if !M_NO_MCOM_IDENTIFY_METER
   ///@{
   /// Identify the meter version and other information if the protocol is known.
   ///
   /// Note that this is not an ANSI Identify protocol command.
   /// Typically, IdentifyMeter would be used when
   /// creating a meter of a known type, but an unknown version.
   ///
   /// The IdentifyMeter service wraps the start and end session around the identification task.
   /// To call IdentifyMeter, one has to connect to the meter, but the session should not be
   /// started. This differs from QIdentifyMeter which does NOT wrap start and end session around
   /// the identification task.
   ///
   /// Typically, IdentifyMeter returns only one identify string that identifies the meter.
   /// However, identify strings can be created for the option boards installed on a meter. In
   /// this case, the returned string can contain multiple identify strings, where the the
   /// meter's identify string is listed first and identify strings are separated by a semi-
   /// colon. Use GetIdentifyStrings to separate multiple identify strings.
   ///
   /// The identify string is formatted as a J command and may not be suitable for showing to the
   /// user as it can contain non-printable characters. Instead, it is formatted for easy
   /// processing by a script or a program. A typical identify string returned for an A3 meter
   /// with LANOB option board follows (wrapped for clarity):
   ///
   /// Example of A3 Identify string followed by LANOB's identify string,
   /// spaces and new lines used for clarity:
   /// \code
   /// J00[MANUFACTURER:EE ]
   ///    [ED_MODEL:A3R ]
   ///    [HW_VERSION_REVISION:1.2]
   ///    [SW_VERSION_REVISION:2.3]
   ///    [MFG_SERIAL_NUMBER: ]
   ///    [NUMBER_OF_OPTION_BOARDS:2]
   ///    [OB1_Type:0M]
   ///    [OB1_SSPEC:000237]
   ///    [OB1_REVISION:1.0]
   ///    [OB1_POSITION:1]
   ///    [OB2_Type:0K]
   ///    [OB2_SSPEC:000239]
   ///    [OB2_REVISION:1.3]
   ///    [OB2_POSITION:2];
   /// J00[MANUFACTURER:EE ]
   ///    [ED_MODEL:REXILC ]
   ///    [HW_VERSION_REVISION:1.0]
   ///    [SW_VERSION_REVISION:1.3]
   ///    [MFG_SERIAL_NUMBER:LANOB ]
   /// \endcode
   ///
   MStdString IdentifyMeter(bool sessionIsStarted = false);
   MStdString DoIdentifyMeter0();
   ///@}
#endif // !M_NO_MCOM_IDENTIFY_METER

protected: // Implementation services:

   /// Synchronously start the session, don't do service count.
   /// This protected service is indeed the one,
   /// which needs overwriting by a particular protocol.
   /// Depending on the particular protocol this leads to sequence of
   /// handshake and logon commands.
   ///
   /// See StartSession, which is public. That one does necessary statistics,
   /// monitor handling and error message formatting.
   ///
   /// \pre The channel is open, the protocol state allows starting the session.
   /// Otherwise exceptions can be thrown.
   ///
   virtual void DoStartSession();

   /// Synchronously end the session, don't do service count.
   /// This protected service is indeed the one,
   /// which needs overwriting by a particular protocol.
   /// Depending on the particular protocol this leads to sequence of
   /// logoff and terminate commands.
   ///
   /// See EndSession, which is public. That one does necessary statistics,
   /// monitor handling and error message formatting.
   ///
   /// \pre The channel is open, the protocol state allows ending the session.
   /// Otherwise exceptions can be thrown.
   ///
   virtual void DoEndSession();

   /// Synchronously read the whole table with number given as parameter, don't do service count.
   /// This protected service is indeed the one,
   /// which needs overwriting by a particular protocol.
   ///
   /// See TableRead, which is public. That one does necessary statistics,
   /// monitor handling and error message formatting.
   ///
   /// The third argument is introduced to handle a special case where the table 
   /// size is bigger than what the protocol allows, such as configuring an ANSI meter
   /// to have more load profile data than the 64K that can be transmitted with
   /// ANSI C12.18. In this case the table is read with multiple partial reads, 
   /// where each partial read is sized to fit within the protocol constraints.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   virtual void DoTableRead(MCOMNumberConstRef number, MByteString& data, unsigned expectedSize = 0);

   /// Synchronously write the whole table with number given as parameter, don't do service count.
   /// This protected service is indeed the one,
   /// which needs overwriting by a particular protocol.
   ///
   /// See TableWrite, which is public. That one does necessary statistics,
   /// monitor handling and error message formatting.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   virtual void DoTableWrite(MCOMNumberConstRef number, const MByteString& data);

   /// Synchronously read part of the table with number given as parameter, don't do service count.
   /// This protected service is indeed the one,
   /// which needs overwriting by a particular protocol.
   ///
   /// See TableReadPartial, which is public. That one does necessary statistics,
   /// monitor handling and error message formatting.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   virtual void DoTableReadPartial(MCOMNumberConstRef number, MByteString& data, int offset, int size);

   /// Synchronously write part of the table with number given as parameter, don't do service count.
   /// This protected service is indeed the one,
   /// which needs overwriting by a particular protocol.
   ///
   /// See TableWritePartial, which is public. That one does necessary statistics,
   /// monitor handling and error message formatting.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   virtual void DoTableWritePartial(MCOMNumberConstRef number, const MByteString& data, int offset);

   /// Synchronously execute the function with no parameters,
   /// the number of the function is given as parameter, don't do service count.
   /// This protected service is indeed the one,
   /// which needs overwriting by a particular protocol.
   ///
   /// See FunctionExecute, which is public. That one does necessary statistics,
   /// monitor handling and error message formatting.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   virtual void DoFunctionExecute(MCOMNumberConstRef number);

   /// Synchronously execute the function with request data,
   /// the number of the function is given as parameter, don't do service count.
   /// This protected service is indeed the one,
   /// which needs overwriting by a particular protocol.
   ///
   /// See FunctionExecuteRequest, which is public. That one does necessary statistics,
   /// monitor handling and error message formatting.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   virtual void DoFunctionExecuteRequest(MCOMNumberConstRef number, const MByteString& request);

   /// Synchronously execute the function with response data,
   /// the number of the function is given as parameter, don't do service count.
   /// This protected service is indeed the one,
   /// which needs overwriting by a particular protocol.
   ///
   /// See FunctionExecuteResponse, which is public. That one does necessary statistics,
   /// monitor handling and error message formatting.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   virtual void DoFunctionExecuteResponse(MCOMNumberConstRef number, MByteString& response);

   /// Synchronously execute the function with request and response data,
   /// the number of the function is given as parameter, don't do service count.
   /// This protected service is indeed the one,
   /// which needs overwriting by a particular protocol.
   ///
   /// See FunctionExecuteRequestResponse, which is public. That one does necessary statistics,
   /// monitor handling and error message formatting.
   ///
   /// \pre The channel is open, the session is started.
   /// Otherwise exceptions can be thrown.
   ///
   virtual void DoFunctionExecuteRequestResponse(MCOMNumberConstRef number, const MByteString& request, MByteString& response);

#if !M_NO_MCOM_IDENTIFY_METER
   /// Identify the meter if the protocol is known (note this is not an ANSI Identify protocol command).
   /// This protected service is indeed the one,
   /// which needs overwriting by a particular protocol.
   ///
   /// If the first parameter is true, the service assumes the session is open, and it will not
   /// start the session, or close it. The second parameter, if it is not NULL, is filled with the
   /// data from tables, which DoIdentifyMeter had to read from the meter during its attempts to fetch
   /// the necessary information from the meter. This table vector can be used in order to minimize
   /// the communication with the meter, if this task has its place.
   ///
   /// See IdentifyMeter, which is public. That one does necessary statistics,
   /// monitor handling and error message formatting.
   ///
   /// \pre The channel is connected. Otherwise a channel-related
   /// exception is thrown that tells about the failure. The meter is indeed the one which
   /// is able to talk the specific protocol. Otherwise the protocol-specific exception
   /// is thrown that signals the failure of communicating through the channel.
   /// If the session is started already, the first parameter has to be true,
   /// or the behavior is undefined.
   ///
   virtual MStdString DoIdentifyMeter(bool sessionIsStarted, TableRawDataVector* tablesRead);
#endif

   /// Try password or passwords for the protocol according to the PASSWORD and PasswordList
   /// settings. In case the password list is specified, only the password list is tried,
   /// if there is no password list, PASSWORD property is used.
   ///
   /// \pre The security should be cleared with the meter using one of the passwords
   /// in the password list, or the password property. Otherwise a security error is thrown.
   ///
   void DoTryPasswordOrPasswordList();

   /// Try one password, throw if error. This service has to be overloaded by every final
   /// protocol to attempt applying a given password.
   ///
   /// \pre The security should be cleared with the meter using the password
   /// entry given, otherwise an exception should be thrown by the concrete implementation.
   ///
   virtual void DoTryPasswordEntry(const MByteString& entry);

#if !M_NO_VERBOSE_ERROR_INFORMATION
   /// Build service name with a number and given parameters.
   /// If parameters are not present they are not in the name.
   /// The particular protocols can fill in their own implementations for full service name.
   ///
   /// \pre The given chars pointer shall point to a buffer at least
   /// MAXIMUM_SERVICE_NAME_STRING_SIZE long. Otherwise the behavior is undefined.
   /// Under no circumstance shall this method shall throw an error.
   ///
   virtual void DoBuildComplexServiceName(MChars fullServiceName, MConstChars serviceName, MCOMNumberConstRef number, int par1 = -1, int par2 = -1) M_NO_THROW;
#endif

public: // Services:


public: // Semi-public services (for those who know):
/// \cond SHOW_INTERNAL

   // Add count from the given protocol to this protocol.
   //
   void DoAddCounts(MProtocol& from);

   // Set the channel baud ratio, if it is applicable to the channel type.
   // The baud adjustment is applicable only for protocols only with the optical probe channel.
   //
   // The numberOfCharsInBuffer parameter, if specified, should match the number of characters
   // written right before FlushOutputBuffer is called. If the parameter is missing,
   // the biggest possible number of characters will be ensured to go away.
   //
   // \pre The channel is in a good state, or the
   // channel exception will result.
   //
   void DoSetBaudIfOpticalProbe(unsigned baud, unsigned numberOfCharsInBuffer = UINT_MAX);

   // Set the channel parameters, if they are applicable to the channel type.
   // The baud adjustment is applicable only for protocols only with the optical probe channel.
   //
   // The numberOfCharsInBuffer parameter, if specified, should match the number of characters
   // written right before FlushOutputBuffer is called. If the parameter is missing,
   // the biggest possible number of characters will be ensured to go away.
   //
   // \pre The channel is in a good state, or the
   // channel exception will result.
   //
   void DoSetParametersIfOpticalProbe(unsigned baud, int databits, char parity, int stopBits, unsigned numberOfCharsInBuffer = UINT_MAX);

   // Set the channel baud ratio, if it is applicable to the channel type.
   // The baud adjustment is applicable for protocols only with the optical probe,
   // direct connect or current loop channel.
   //
   // The numberOfCharsInBuffer parameter, if specified, should match the number of characters
   // written right before FlushOutputBuffer is called. If the parameter is missing,
   // the biggest possible number of characters will be ensured to go away.
   //
   // \pre The channel is in the good state, or the
   // channel exception will result.
   //
   void DoSetBaudIfOpticalProbeOrDirect(unsigned baud, unsigned numberOfCharsInBuffer = UINT_MAX);

   // Set the channel parameters, if they are applicable to the channel type.
   // The parameters will be set for protocols only with the optical probe,
   // direct connect or current loop channel.
   //
   // The numberOfCharsInBuffer parameter, if specified, should match the number of characters
   // written right before FlushOutputBuffer is called. If the parameter is missing,
   // the biggest possible number of characters will be ensured to go away.
   //
   // \pre The channel is in the good state, or the
   // channel exception will result.
   //
   void DoSetParametersIfOpticalProbeOrDirect(unsigned baud, int databits, char parity, int stopBits, unsigned numberOfCharsInBuffer = UINT_MAX);

   // Read the start character of the packet in a proper way,
   // taking into consideration timeouts and ignoring garbage.
   // Only characters in the validStartCharacters set are allowed.
   // For obvious reasons the set cannot contain a zero character.
   //
   // Turn around characters size determines for which incoming characters turn around statistics will be updated.
   // If none of the start characters participate in turn around, set it to 0.
   // Otherwise, place turn around characters first, and next put the characters that are valid to receive, but that shall not participate in turn around calculation.
   // For example, those characters can be the start characters of the previous packet.
   // The value of turnAroundCharactersSize shall be set to the number of turn around characters in the whole sequence.
   // It is okay if this value is bigger than the string size. The default value tells exactly that - all characters are turn arounds.
   //
   // \pre The start character among those in the set given should be present in the input stream
   // within trafficTimeout period. Otherwise an exception is thrown.
   //
   char DoReadStartCharacter(const char* validStartCharacters, unsigned trafficTimeout, unsigned turnAroundCharactersSize = UINT_MAX);

   // Check if the channel is present, and there is no background communication in progress.
   //
   // \pre The channel shall be given to protocol and there should be no background communication in progress.
   // Otherwise an exception takes place.
   //
   void DoCheckChannel(bool allowBackgroundCommunication = false) const;

#if !M_NO_MCOM_KEEP_SESSION_ALIVE

public: // Methods:

   // Abstract protocol-dependent service that returns the number of milliseconds to delay
   // before the first sending of the KeepSessionAlive message to the meter. It never throws an exception.
   // If the returned value is zero, no keeping of the session shall be made.
   // Otherwise the delay for the next keeping of the session shall be used.
   // The delays for the subsequential (not first) KeepAlive messages are returned by SendKeepSessionAliveMessage oneselves.
   //
   // Default implementation will always return zero, means the session does not need to be kept alive.
   // This is a wanted behavior for protocols that do not need this function.
   //
   virtual unsigned DoGetKeepSessionAliveFirstDelay() const;

   // Abstract service that actually keeps the session alive.
   // It never throws an exception, returns the delay for the next keeping of the session.
   // If the returned value is zero, no keeping of the session shall be made.
   //
   // Default implementation will always return zero, means the session does not need to be kept alive.
   // This is a wanted behavior for protocols that do not need this function.
   //
   // \pre Shall be called only at a proper time of keeping the session alive.
   //
   virtual unsigned DoSendKeepSessionAliveMessage();

#endif

   // Convert the variant into a protocol specific unsigned table or function number.
   // The number has to be smaller or equal to the upper value.
   //
   // \pre The number should be convertible to a number, and fit into range.
   //
   static unsigned DoConvertNumberToUnsigned(MCOMNumberConstRef number, unsigned upperValue = 0xFFFFu);

   // Update round trip time statistics from the next milliseconds value of round trip time
   //
   void DoUpdateRoundTripTimes(unsigned roundTripTime);

protected: // Attributes:

   // Protocol application level password. Effective if the password list is empty.
   //
   MByteString m_password;

#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST

   // Protocol application level password list. Takes rule over m_password, if not empty.
   //
   MByteStringVector m_passwordList;

   // The successful entry of the password list,
   // valid only after the password from the password list is successfully checked.
   //
   int m_passwordListSuccessfulEntry;

#endif // !M_NO_MCOM_PASSWORD_AND_KEY_LIST

#if !M_NO_MCOM_COMMAND_QUEUE

   // Communication command queue
   //
   MCommunicationQueue m_queue;

   bool m_commitDone;

#endif // !M_NO_MCOM_COMMAND_QUEUE

   // Whether for this protocol HEX representation of password is preferable.
   // This property can be used by automatic GUI builders.
   //
   bool m_preferredPasswordIsHex;

   // Maximum allowed length of the password
   // This property can be used by automatic GUI builders.
   //
   Muint8 m_maximumPasswordLength;

   // Channel of this protocol.
   //
   MChannel* m_channel;

#if !M_NO_MCOM_PROTOCOL_THREAD

   // Protocol client thread, used to manage the background communication thread
   //
   MProtocolThread* m_protocolThread;

   // True if the background communication is progressing
   //
   bool m_backgroundCommunicationIsProgressing;

#endif // !M_NO_MCOM_PROTOCOL_THREAD

   // True if the meter is little endian, false otherwise.
   // This flag has to be used for function implementations, as there is
   // a field there which depends on the architecture.
   //
   bool m_meterIsLittleEndian;

   // Whether the channel is owned by the protocol
   //
   bool m_isChannelOwned;

   // Whether or not the session is currently active.
   //
   bool m_isInSession;

   // Whether the destructor is finalized
   //
   bool m_isFinalized;

   // Whether to update round trip times automatically, or it will be done externally with calls of UpdateRoundTripTimes
   //
   bool m_autoUpdateRoundTripTimes;

   // Stack of service wrappers, handled by protocol.
   //
   MProtocolServiceWrapper::Stack m_serviceWrappers;

#if !M_NO_MCOM_KEEP_SESSION_ALIVE

   // Thread that keeps session alive
   //
   mutable MSessionKeeper m_sessionKeeper;

#endif

   // Internal counter of application layer successes and failures that is used to flush statistics data into monitor.
   //
   unsigned m_savedTotalAppLayerServices;

   // Number of application layer services successfully processed.
   //
   unsigned m_countApplicationLayerServicesSuccessful;

   // Number of application layer services retried.
   //
   unsigned m_countApplicationLayerServicesRetried;

   // Number of application layer services failed.
   //
   unsigned m_countApplicationLayerServicesFailed;

   // Number of data link layer packets successfully processed.
   //
   unsigned m_countLinkLayerPacketsSuccessful;

   // Number of data link layer packets retried.
   //
   unsigned m_countLinkLayerPacketsRetried;

   // Number of data link layer packets failed.
   //
   unsigned m_countLinkLayerPacketsFailed;

   // The maximum approximate round trip time over the link layer.
   //
   unsigned m_maximumRoundTripTime;

   // The minimum approximate round trip time over the link layer
   //
   unsigned m_minimumRoundTripTime;

   // The average approximate round trip time over the link layer
   //
   double m_sumRoundTripTime;

   // The number of successful round trips made, up to four billion
   //
   double m_roundTripCounter;

#if !M_NO_PROGRESS_MONITOR
   MProgressMonitor* m_progressMonitor;
#endif

/// \endcond SHOW_INTERNAL

   M_DECLARE_CLASS(Protocol)
};

///@}
#endif
