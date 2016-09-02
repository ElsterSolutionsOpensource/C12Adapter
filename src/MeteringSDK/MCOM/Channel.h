#ifndef MCOM_CHANNEL_H
#define MCOM_CHANNEL_H
/// \addtogroup MCOM
///@{
/// \file MCOM/Channel.h

#include <MCOM/MCOMDefs.h>
#include <MCOM/MCOMObject.h>
#include <MCOM/Monitor.h>

/// Abstraction of all channel-level communication media.
///
/// Channels are intended to provide the mechanism for reading and writing byte streams
/// with timeouts.
///
class MCOM_CLASS MChannel : public MCOMObject
{
   friend class MProtocol;
   friend class MComgateClient;

public: // Constants:

   enum
   {
      CANCEL_COMMUNICATION_CHECK_OPTIMUM_INTERVAL = 1000 ///< How often in milliseconds to check for the communication to cancel.
   };

public:  // Types

   /// Uninterruptible communication C++ wrapper.
   ///
   class UninterruptibleCommunication
   {
   public: // Constructor, destructor:

      /// Enter uninterruptible communication by creating constructor.
      /// By default, this particular implementation does not notify monitor.
      ///
      UninterruptibleCommunication(MChannel* channel, bool notify = false) M_NO_THROW
      :
         m_channel(channel),
         m_notify(notify)
      {
         M_ASSERT(m_channel != NULL);
         m_channel->EnterUninterruptibleCommunication(m_notify);
      }

      /// Leave uninterruptible communication at destruction.
      ///
      ~UninterruptibleCommunication() M_NO_THROW
      {
         // Some applications disconnect the channel at error,
         // which sets m_cancelCommunicationGuard to zero.
         // The IF operator below handles this situation gracefully.
         if ( m_channel->m_cancelCommunicationGuard != 0 )
            m_channel->LeaveUninterruptibleCommunication(m_notify);
      }

   private: // Data:

      // Channel for which to guard uninterruptible communication
      //
      MChannel* m_channel;

      // Whether the monitor has to be notified on uninterruptible communication status change
      //
      bool m_notify;
   };
   friend class UninterruptibleCommunication; 

   /// Temporarily overrides read timeout with a new value using scope rules.
   ///
   /// Constructor saves the current read timeout value, and sets a new given timeout.
   /// Destructor restores the previously saved value.
   ///
   /// Usage example:
   /// \code
   ///      {
   ///          .... // here the usual read timeout applies
   ///          ReadPacketHeader(channel);
   ///          ReadTimeoutSavior timeoutSavior(channel, intercharacterTimeout);
   ///          ReadTheRestOfThePacketWithIntercharacterTimeout(channel);
   ///          .... // here the timeout will be restored
   ///      }
   /// \endcode
   ///
   class ReadTimeoutSavior
   {
   public: // Constructor and destructor:

      /// Constructor saves the previous value of read timeout and sets the new one given.
      ///
      ReadTimeoutSavior(MChannel* channel, unsigned newTimeout)
      :
         m_channel(channel)
      {
         m_oldTimeout = m_channel->GetReadTimeout();
         m_channel->SetReadTimeout(newTimeout);
      }

      /// Destructor restores the previously saved timeout.
      ///
      ~ReadTimeoutSavior()
      {
         m_channel->SetReadTimeout(m_oldTimeout);
      }

   private: // Data:

      MChannel* m_channel;
      unsigned m_oldTimeout;
   };

protected: // Constructor:

   /// Construct the channel object from parent class.
   /// The constructor is protected because the class is abstract.
   ///
   MChannel();

public: // Destructor and services

#if !M_NO_MCOM_FACTORY
   /// Virtual copy constructor, creates the channel, which is a clone of current.
   /// All persistent properties get copied, all other properties have initial values.
   /// Channel will not be connected, etc.
   ///
   virtual MChannel* CreateClone() const;
#endif

   /// The destructor is public, and virtual.  MChannel objects should be
   /// deleted by their owner, MCOMFactory creates Channels, but does not destroy them.
   ///
   virtual ~MChannel();

   /// Initializes channel and establishes connection between the computer and the end device.
   ///
   /// The specific set of actions depends on the channel type:
   ///   - For an optical probe, the serial port is opened.
   /// \ifnot M_NO_MCOM_CHANNEL_MODEM
   ///   - For a modem, the port is opened, then the modem is connected by
   ///     either dialing-in, or by waiting for the call-back.
   /// \endif
   /// \ifnot M_NO_MCOM_CHANNEL_SOCKET
   ///   - For socket channel the outgoing socket is connected or
   ///     an incoming connection is accepted, depending on channel parameters.
   /// \endif
   ///
   /// Once the connection is established (Connect successfully completed), no other application
   /// can connect to the end device using the same communication line. Use the Disconnect method
   /// to terminate the connection. Be sure to include Disconnect in error handling routines, to
   /// ensure the link is always terminated. Otherwise, other applications may not be able to
   /// connect to the end device.
   ///
   /// After successful Connect, bytes can be read/written to the meter.
   ///
   /// \pre IsConnected should be false before calling this method.
   /// Many OS, environment, and program related exceptions can be thrown by this method.
   ///
   /// \see \ref Disconnect
   ///
   virtual void Connect();

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

   /// Disconnect brings down the data link, hangs up the phone, powers down the probe, etc.
   ///
   /// After Disconnect, no communication is possible
   /// with the meter. Calling Disconnect when the MChannel is not Connect()ed
   /// has no effect, and Disconnect itself will always be successful.
   ///
   ///  - For MChannelOpticalProbe, this method simply releases the communication port
   ///    back to the operating System, sets IsConnected to False, and returns.
   ///  - For MChannelModem, a hang up sequence is performed on the modem prior to releasing
   ///    the communication port back to the operating system and setting IsConnected to False.
   ///
   /// Disconnect never generates an error. This allows it to be successfully issued even if the
   /// channel is not connected. This behavior was implemented in order to simplify handling of
   /// situations where the channel holds some resources but IsConnected = FALSE. For example,
   /// the modem link is disconnected, but the communication port is open and waiting to
   /// reestablish the connection. In this case, IsConnected = FALSE, but the communication port
   /// is still being held by the channel. Issuing Disconnect in this case would result in
   /// releasing the communication port back to the Operating System. It is not possible to use
   /// IsConnected in this situation to determine whether or not to issue the Disconnect method,
   /// so the Disconnect method was designed to operate successfully regardless of whether the
   /// channel is connected or disconnected.
   ///
   /// Use the Connect method to establish the connection between the computer and the end
   /// device. As long as the connection exists, no other application can connect to the end
   /// device using the same communication line. Use Disconnect to terminate the connection.
   /// Include Disconnect in error handling routines, otherwise, other applications may not be
   /// able to connect to the end device.
   ///
   /// \see \ref Connect
   ///
   virtual void Disconnect() = 0;

   /// Writes the data to the channel, and returns when the last character
   /// has been sent by the software (but hardware might still need to do some work).
   /// To ensure that all data is sent, use FlushOutputBuffer.
   ///
   /// Unusual variations of the protocol can be simulated by sending data to the meter using
   /// WriteBytes, then reading the meter's response with ReadBytes. Invalid packets or noise
   /// data can also be sent to the meter for test purposes.
   ///
   /// Just like Connect or Disconnect, WriteBytes is synchronous and is not queued. It should be
   /// used carefully when mixed with MProtocol's Queue Methods, which are transmitted to the end
   /// device only when QCommit is called.
   ///
   /// \pre The channel is open, otherwise the operation fails with an exception.
   ///
   /// \param buffer An array of bytes to pass to the channel. The byte in the lower bound of the
   /// array is the first byte to be transmitted and the byte in the upper bound of the array is
   /// the last byte to be transmitted.
   ///
   void WriteBytes(const MByteString& buffer);

   /// Writes the character to the channel, and returns when it has been sent.
   ///
   /// \pre The channel is open, otherwise the operation fails with
   /// an exception. The buffer is initialized correctly.
   ///
   void WriteChar(char buf);
 
   /// Writes the data buffer to the channel, and returns when the last character
   /// has been sent by the software (but hardware might still need to do some work).
   /// To ensure that all data is sent, use FlushOutputBuffer.
   ///
   /// \pre The channel is open, otherwise the operation fails
   /// with an exception. The buffer is initialized correctly,
   /// otherwise the behavior is undefined.
   ///
   void WriteBuffer(const char* buf, unsigned len);

   /// Read single character from the channel.
   ///
   /// \pre The channel is open, otherwise the
   /// operation fails with an exception.
   /// The character is received within timeout period.
   /// Otherwise the timeout exception is thrown.
   ///
   char ReadChar();

   /// Read an exact number of characters from the channel.
   ///
   /// \pre The channel is open, otherwise the operation fails with an exception.
   /// The number of characters given as parameter are received
   /// within the timeout period. Otherwise the timeout exception is thrown.
   ///
   void ReadBuffer(char* buf, unsigned numberToRead);

#if !M_NO_VARIANT
   /// Return the given byte or bytes to the stream buffer so they get read at the next read operation.
   ///
   /// \pre The channel is open, otherwise the operation fails with an exception.
   ///
   void Unread(const MVariant& byteOrBytes);
#endif

   /// Return the given bytes to the stream buffer so they get read at the next read operation.
   ///
   /// \pre The channel is open, otherwise the operation fails with an exception.
   ///
   void UnreadBuffer(const char* buff, unsigned size);

   /// Read bytes directly from the communication channel.
   ///
   /// Unusual variations of the protocol can be simulated by sending data to the meter using
   /// WriteBytes, then reading the meter's response with ReadBytes. Invalid packets or noise
   /// data can also be sent to the meter for test purposes.
   ///
   /// Just like Connect or Disconnect, ReadBytes is synchronous and is not queued. It should be
   /// used carefully when mixed with the MProtocol's Queue Methods, which are transmitted to the
   /// end device only when QCommit is called.
   ///
   /// \param numberToRead Number of bytes to read.
   ///
   /// \pre The channel is open, otherwise the operation fails with an exception.
   ///
   /// \seeprop{GetIntercharacterTimeout,IntercharacterTimeout} - the number of
   ///    milliseconds to wait for the byte to arrive before throwing a timeout exception. 
   ///
   MByteString ReadBytes(unsigned numberToRead);

   /// Read bytes from the channel until a specified sequence is read.
   ///
   /// \pre The channel is open, otherwise the operation fails with an exception.
   ///
   /// \seeprop{GetIntercharacterTimeout,IntercharacterTimeout} - the number of
   ///    milliseconds to wait for the byte to arrive before throwing a timeout exception. 
   ///
   MByteString ReadBytesUntil(const MByteString& terminatingString);

   /// Read bytes from the channel until a specified sequence is read.
   ///
   /// \pre The channel is open, otherwise the operation fails with an exception.
   ///
   /// \seeprop{GetIntercharacterTimeout,IntercharacterTimeout} - the number of
   ///    milliseconds to wait for the byte to arrive before throwing a timeout exception. 
   /// 
   MByteString ReadBytesUntilAnyByte(const char* finisher, unsigned finisherSize, unsigned headerSize, unsigned footerSize);

   /// Read an arbitrary number of characters from the channel, as much as
   /// available. Note this service assumes that the channel has flow
   /// control that tells when the read is done.
   ///
   /// \pre The channel is open, otherwise the operation fails with an exception.
   /// Timeout exception is thrown if no characters were read from the channel during read timeout.
   ///
   MByteString ReadAllBytes();

   /// Immediately discards all the pending characters from the channel.
   ///
   /// Just like Connect or Disconnect, ClearInputBuffer is synchronous and is not queued. It
   /// should be used carefully when mixed with MProtocol's Queue Methods, which are transmitted
   /// to the end device only when QCommit is called.
   ///
   /// \pre The channel is open, otherwise the operation fails with an exception.
   ///
   /// \see \ref ClearInputUntilSilence Keep reading and ignoring input until there is silence.
   ///
   void ClearInputBuffer();

   /// Keep reading and ignoring input until there is silence.
   ///
   /// Useful when it is known that the party will be sending data
   /// that has to be ignored.
   ///
   /// \param milliseconds Time in milliseconds, for how long the party keeps silent for the
   ///                     method to return. Typically, this is equal to intercharacter timeout.
   ///
   /// \see \ref ClearInputBuffer Removes all bytes that are already present in the input buffer.
   ///
   void ClearInputUntilSilence(unsigned milliseconds);

   /// Ensure that the characters from the output buffer are sent.
   /// The parameter, if specified, should match the number of characters
   /// written into the serial port right before FlushOutputBuffer is called.
   /// If the parameter is missing, the biggest possible number of characters will
   /// be ensured to go away.
   ///
   /// \pre The channel is open, otherwise the
   /// operation fails with an exception.
   ///
   virtual void FlushOutputBuffer(unsigned numberOfCharsInBuffer = UINT_MAX) = 0;

   /// Returns the current connection state of the channel.
   ///
   /// IsConnected reflects the assumption the communication component has about the channel
   /// state, it does not guarantee that the next action with the channel will be successful.
   /// Issue the Connect service to establish the connection and the Disconnect service to
   /// terminate the connection.
   ///
   ///  - For MChannelOpticalProbe, IsConnected will simply indicate that the resources
   ///    for the communication port have been acquired.
   ///  - For MChannelModem, IsConnected indicates that the resources have been acquired
   ///    and that the modem has also successfully connected to another modem.
   ///
   /// \default_value False
   ///
   /// \possible_values
   ///  - True  - The connection has been established.
   ///  - False - The connection has not been established.
   ///
   virtual bool IsConnected() const = 0;

   /// Throw an appropriate exception if the channel is not connected.
   /// The exception can be different depending on whether the connection was
   /// not made previously, or if the connection was unexpectedly terminated.
   ///
   /// \pre The channel is connected, or an exception is thrown.
   ///
   virtual void CheckIfConnected();

   /// Throw an appropriate exception if the channel is not connected, constant version.
   /// The exception can be different depending on whether the connection was
   /// not made previously, or if the connection was unexpectedly terminated.
   ///
   /// \pre The channel is connected, or an exception is thrown.
   ///
   /// \see \ref CheckIfConnected - non-constant version of this method.
   ///
   void CheckIfConnectedConst() const
   {
      const_cast<MChannel*>(this)->CheckIfConnected();
   }

   ///@{
   /// Whether the channel initiates the communication or waits for the incoming connection.
   ///
   /// When \ref Connect() is issued, the state of Auto Answer determines whether the channel initiates 
   /// communication (Auto Answer = false), or waits for the incoming connection (Auto Answer = true).
   /// When Auto Answer mode is enabled (Auto Answer = true), at the attempt to issue \ref Connect()
   /// the channel waits \refprop{GetAutoAnswerTimeout,AutoAnswerTimeout} seconds for the incoming connection.
   /// The connection attempt can take a long time to complete or time out. If the connection is
   /// queued with QConnect and committed asynchronously with QCommit(True), then the connection
   /// can be aborted with QAbort.
   ///
   /// All channels support initiating or waiting for the connection, though the typical use is to 
   /// initiate the connection. 
   ///
   /// \since Available to all channels since MeteringSDK Version 5.0.0.980.
   /// \ifnot M_NO_MCOM_CHANNEL_MODEM
   ///    \since Available to MChannelModem and MChannelModemCallback since MeteringSDK Version 2.1.27. 
   /// \endif 
   /// \ifnot M_NO_MCOM_CHANNEL_SOCKET
   ///    \since Available to MChannelSocket and MChannelSocketCallback since MeteringSDK Version 4.0.15.
   /// \endif
   ///
   /// \default_value False [0], unless otherwise noted.
   /// \ifnot M_NO_MCOM_CHANNEL_MODEM
   ///    <br>MChannelModemCallback defaults to True [1]. 
   /// \endif
   /// \ifnot M_NO_MCOM_CHANNEL_SOCKET
   ///    <br>MChannelSocketCallback defaults to True [1].
   /// \endif
   /// \ifnot M_NO_MCOM_CHANNEL_SOCKET_UDP
   ///    <br>MChannelSocketUdpCallback defaults to True [1].
   /// \endif
   ///
   /// \possible_values
   ///  - True [1]  : The channel waits for incoming connection.
   ///  - False [0] : The channel initiates the connection.
   ///
   bool GetAutoAnswer() const
   {
      return m_isAutoAnswer;
   }
   void SetAutoAnswer(bool isAutoAnswer)
   {
      m_isAutoAnswer = isAutoAnswer;
   }
   ///@}

   ///@{
   /// Time in seconds to wait for the incoming connection.
   ///
   /// The number of seconds that the channel listens in \ref Connect() for an inbound call or connection.
   /// \refprop{GetAutoAnswer,AutoAnswer} must be True for this property to have any effect.
   /// This can be a very large value for applications that want to continuously
   /// listen, however the maximum possible value is determined by the number of milliseconds in 
   /// a 32-bit value, therefore, when an attempt is made to set the timeout to a value bigger than 
   /// 2,147,483 seconds, the result will be about 24 days.
   ///
   /// A good idea for the application is to commit the connection asynchronously using \ref MProtocol.QConnect()
   /// and \ref MProtocol.QCommit(bool asynchronous = true). This allows the
   /// application to continue to do work while waiting for the incoming connection, and also
   /// allows the connection attempt to be aborted with \ref MProtocol.QAbort().
   ///
   /// \since Available to all channels since MeteringSDK Version 5.0.0.980.
   /// \ifnot M_NO_MCOM_CHANNEL_MODEM
   ///    \since Available to MChannelModem and MChannelModemCallback since MeteringSDK Version 2.1.27. 
   /// \endif 
   /// \ifnot M_NO_MCOM_CHANNEL_SOCKET
   ///    \since Available to MChannelSocket and MChannelSocketCallback since MeteringSDK Version 4.0.15.
   /// \endif
   ///
   /// \default_value 60 seconds
   ///
   /// \possible_values 0 .. MAXINT seconds. 
   ///                  However, values larger than 2,147,483 seconds may result in 
   ///                  unpredictable timeouts as the maximum time is determined by the number 
   ///                  of milliseconds in a 32-bit value. Prior to MeteringSDK Version 6.4.0.4908, 
   ///                  values larger than 2,147,483 seconds could result in a silent overflow and 
   ///                  an unpredictable timeout period. For MeteringSDK Versions 6.4.0.4908 and later, 
   ///                  values larger than 2,147,483 seconds will result in a timeout of 
   ///                  2,147,483 seconds (slightly more than 24 days).
   ///
   unsigned GetAutoAnswerTimeout() const
   {
      return m_autoAnswerTimeout;
   }
   void SetAutoAnswerTimeout(unsigned timeout);
   ///@}

#if !M_NO_MCOM_MONITOR

   /// @{
   /// Monitor object bound to the channel object.
   ///
   /// The MMonitor object allows messages to be sent to a
   /// listening monitor application and to save the messages in a
   /// binary file that can be viewed later by loading it into the monitor application.
   /// The monitor application does not have to be running in order
   /// for a client to send messages or capture the communications into a binary file.
   ///
   /// \pre The monitor pointer must be valid, or NULL.
   ///
   /// \possible_values
   ///  - Any valid MMonitor.
   ///  - Null pointer to discard the previously set monitor.
   ///
   void SetMonitor(MMonitor::Pointer monitor);
   MMonitor::Pointer GetMonitor() const
   {
         return m_monitor;
   }
   /// @}

/// \cond SHOW_INTERNAL
   void DoSetMonitor(MMonitor* monitor)
   {
      SetMonitor(MMonitor::Pointer(monitor));
   }

   MMonitor* DoGetMonitor() const
   {
      return GetMonitor();
   }
/// \endcond SHOW_INTERNAL
#endif // !M_NO_MCOM_MONITOR

   ///@{
   /// Echo mode, whether the bytes are echoed back, so they have to be read by the channel.
   ///
   /// Indicates whether or not the end device echoes every character transmitted through the
   /// channel. For example, Echo needs to be enabled for current loop devices and two-wire
   /// RS-485 devices. 
   ///
   /// \since MeteringSDK Version 3.0.5.
   ///
   /// \default_value
   ///   - True  [1] - For MChannelCurrentLoop
   ///   - False [0] - For all other channels
   ///
   /// \possible_values
   ///  - True [1]  - The end device echoes bytes back through the channel.
   ///  - False [0] - The end device does not echo bytes.
   ///
   /// \ifnot M_NO_MCOM_MONITOR
   /// \seeprop{GetSendEchoBytesToMonitor,SendEchoBytesToMonitor} - whether or not to send the echoed bytes to monitor.
   /// \endif
   ///
   bool GetEcho() const
   {
      return m_echo;
   }
   void SetEcho(bool echo)
   {
      m_echo = echo;
   }
   ///@}

#if !M_NO_MCOM_MONITOR
   ///@{
   /// Whether to send echo bytes to the MMonitor object, 
   /// applicable to channel types that support the Echo property.
   ///
   /// Enable or disable sending the Echo characters to the MMonitor object. 
   /// Typically, users do not want to see the Echo'ed characters in the
   /// communications log. However, for developers that need to debug communication problems,
   /// having the Echo'ed characters in the communications log can be helpful.
   ///
   /// \since MeteringSDK Version 3.2.23.
   ///
   /// \default_value False [0]
   ///
   /// \possible_values
   ///  - True [1]  : Send the echo'ed characters to the MMonitor object. If Echo = false, there are no Echo'ed
   ///    characters to send, and no error is thrown.
   ///  - False [0] : Do not send the echo'ed characters.
   ///
   /// \seeprop{GetEcho,Echo} - property to control the channel echo.
   ///
   bool GetSendEchoBytesToMonitor() const
   {
      return m_sendEchoBytesToMonitor;
   }
   void SetSendEchoBytesToMonitor(bool doSend)
   {
      m_sendEchoBytesToMonitor = doSend;
   }
   ///@}
#endif
   
   ///@{
   /// Timeout value in between receiving any two characters.
   ///
   /// All channel types have intercharacter timeout property.
   /// Intercharacter timeout is only applicable to read operations that request more than one byte.
   /// The protocols impose their own restrictions on IntercharacterTimeout,
   /// and they control this value while communicating.
   /// Advanced users may want to control this property when working directly with the channel
   /// using the available read services.
   ///
   /// When the value of this property is nonzero, it controls the maximum time to wait
   /// between receiving any two consecutive bytes before initiating a timeout condition.
   /// Also, when intercharacter timeout is nonzero, ReadTimeout property guards time
   /// to receive the first character in the sequence. This mode is often the case for
   /// serial communications, and the way all current protocols, except ANSI C12.22, behave.
   ///
   /// When this property is zero, the way timeouts are handled by the channel is altered.
   /// In this mode, intercharacter timeout is not guarded, however the ReadTimeout property
   /// becomes responsible for receiving the whole sequence. This mode is often the case for
   /// TCP/IP communications, and is the way ANSI C12.22 behaves.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 500 milliseconds
   ///
   /// \possible_values
   ///    0 - Special value that alters the way timeouts are handled, see description text.
   ///    1 .. MAXINT value of intercharacter timeout in milliseconds.
   ///
   unsigned GetIntercharacterTimeout() const
   {
      return m_intercharacterTimeout;
   }
   void SetIntercharacterTimeout(unsigned timeout);
   ///@}

   ///@{
   /// Read timeout, responsible for receiving either the first byte or the whole packet.
   ///
   /// The protocols always control this value themselves,
   /// while the user will typically handle it only
   /// when using the channel services to read from the channel directly.
   ///
   /// The meaning of read timeout depends on the value of IntercharacterTimeout property as follows:
   ///   - When IntercharacterTimeout is nonzero, the read timeout is the maximum time to wait
   ///     for receiving the first character in a sequence before initiating a timeout condition.
   ///     In this case, the timeout applicable to all subsequent characters in the same packet is
   ///     the value of IntercharacterTimeout.
   ///   - When IntercharacterTimeout is zero, the read timeout is guarding time
   ///     to receive the whole sequence of bytes requested.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 1000 milliseconds
   ///
   /// \possible_values 0 .. MAXINT, timeout in milliseconds.
   ///
   unsigned GetReadTimeout() const
   {
      return m_readTimeout;
   }
   void SetReadTimeout(unsigned timeout);
   ///@}

   ///@{
   /// The maximum time to wait for the successful transmission of a packet before initiating a
   /// timeout error.
   ///
   /// The protocols always control this value themselves. The user will typically handle it only
   /// when using the channel services to write to the port directly.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 2000 milliseconds
   ///
   /// \possible_values 0 .. MAXINT, timeout in milliseconds.
   ///
   unsigned GetWriteTimeout() const
   {
      return m_writeTimeout;
   }
   void SetWriteTimeout(unsigned timeout);
   ///@}

   /// Number of bytes sent through the channel since its creation or since the last \ref ResetCounts().
   ///
   /// This count starts from zero at channel creation, or at a call to \ref ResetCounts(),
   /// and it gets incremented for each byte successfully sent.
   ///
   /// \since MeteringSDK Version 2.2.17.
   ///
   /// \possible_values 0 .. MAXUINT
   ///
   unsigned GetCountBytesSent() const
   {
      return m_countBytesSent;
   }

   /// The number of bytes received through the channel since creation or since the last \ref ResetCounts().
   ///
   /// The count is nullified when the connection is created.
   ///
   /// \since MeteringSDK Version 2.2.17.
   ///
   /// \possible_values 0 .. MAXUINT
   ///
   unsigned GetCountBytesReceived() const
   {
      return m_countBytesReceived;
   }

   /// Returns a string that uniquely identifies the media through which this channel is communicating.
   ///
   /// It will typically consist of computer host name, channel type and unique channel
   /// parameters pertinent to that channel type.
   /// Examples of media identifications:
   ///   - "myhost:SOCKET:10.65.67.169:1153" - Socket channel initiated from host myhost
   ///     towards IP address 10.65.67.169 and port 1153.
   ///   - "wincomp10x64:SERIAL:COM1" - Optical probe channel that uses serial port COM1
   ///     of host wincomp10x64.
   ///   - "wincomp10x64:SERIAL:COM25" - Direct serial channel that uses serial port COM25.
   ///
   /// Channel types in the media identification string are more generic than MeteringSDK channel type names.
   /// For instance, whether the channel is optical probe or a current loop, it will return SERIAL
   /// as its identification type.
   ///
   virtual MStdString GetMediaIdentification() const = 0;

   /// Reset channel statistical data, so the counters become zeros.
   ///
   /// The list of channel properties that are reset follows:
   ///  - \refprop{GetCountBytesReceived,CountBytesReceived}
   ///  - \refprop{GetCountBytesSent,CountBytesSent}
   ///
   /// \see \ref MProtocol::ResetCounts() - reset extra counts available in the protocol, and in the associated channel.
   ///
   void ResetCounts()
   {
      m_countBytesSent = 0u;
      m_countBytesReceived = 0u;
   }

   /// Synchronously write a message to the monitor, if it is connected.
   ///
   /// No errors are ever thrown by this method, if there is no monitor connected, nothing is done.
   ///
   virtual void WriteToMonitor(const MStdString& message);

   /// Request canceling of the communication.
   ///
   /// This service is typically called from a separate thread.
   /// The child channels might try to do some additional processing
   /// to cancel the communication. Disconnect will be called only
   /// if the parameter of the function is true.
   ///
   virtual void CancelCommunication(bool callDisconnect = false);

   /// Enter a communication sequence that shall not be be interrupted
   /// with CancelCommunication call. There is also LeaveUninterruptibleCommunication
   /// that ends such sequence. The matching pairs of these calls
   /// can be stacked. In which case the interruption can happen only
   /// after leaving the very top LeaveUninterruptibleCommunication.
   ///
   void EnterUninterruptibleCommunication(bool notify = true) M_NO_THROW;

   /// Leave a communication sequence that shall not be be interrupted
   /// with CancelCommunication call. There is also EnterUninterruptibleCommunication
   /// that ends such sequence. The matching pairs of these calls
   /// can be stacked. In which case the interruption can happen only
   /// after leaving the very top LeaveUninterruptibleCommunication.
   /// The communication is uninterrupted at next interaction with the channel,
   /// not at the call of this function.
   ///
   void LeaveUninterruptibleCommunication(bool notify = true) M_NO_THROW;

   /// Check if the user has requested the termination of the communication,
   /// and whether the cancel operation lock is zero.
   /// If both conditions are yes, throw an exception MEOperationCancelled.
   ///
   void CheckIfOperationIsCancelled();

   /// Channel version of Sleep, a delay function that is aware of cancel communication event.
   /// Different from MUtilities.Sleep, this function might throw cancel communication exception.
   ///
   void Sleep(unsigned milliseconds);

   /// Read up to size bytes into buffer using the given timeout.
   ///
   /// This method does not use ReadTimeout property, but it will not throw a timeout exception.
   ///
   unsigned ReadWithTimeout(char* buf, unsigned size, unsigned timeout);

public: // Semi-public reflection helpers:
/// \cond SHOW_INTERNAL

   /// Same as EnterUninterruptibleCommunication, but with default parameter applied.
   ///
   /// \see \ref EnterUninterruptibleCommunication for details.
   ///
   void DoEnterUninterruptibleCommunication0() M_NO_THROW;

   /// Same as LeaveUninterruptibleCommunication, but with default parameter applied.
   ///
   /// \see \ref LeaveUninterruptibleCommunication for details.
   ///
   void DoLeaveUninterruptibleCommunication0() M_NO_THROW;

   M_NORETURN_FUNC void DoThrowCharactersNotEchoed();

protected: // Services (no virtuals are allowed in this chunk):

#if MCOM_PROJECT_COMPILING // do not expose notifications to clients!

   // Protected service that allows MChannel subclasses to notify the monitor
   // that the connection is made.
   //
   void DoNotifyConnect()
   {
      #if !M_NO_MCOM_MONITOR
         if ( m_monitor != NULL && m_monitor->IsListening() )
            m_monitor->OnConnect();
      #endif

      CheckIfOperationIsCancelled();
   }

   // Protected service that allows MChannel subclasses to notify the monitor
   // that the channel is disconnected.
   //
   void DoNotifyDisconnect()
   {
      #if !M_NO_MCOM_MONITOR
         if ( m_monitor != NULL && m_monitor->IsListening() )
            m_monitor->OnDisconnect();
      #endif

      // should not call CheckIfOperationIsCancelled
   }

   // Protected service that allows MChannel subclasses to notify the monitor
   // that some bytes are received.
   //
   void DoNotifyByteRX(const char* data, int length)
   {
      m_countBytesReceived += length;

      #if !M_NO_MCOM_MONITOR
         if ( m_monitor != NULL && m_monitor->IsListening() )
            m_monitor->OnByteRX(data, length);
      #endif // M_NO_MCOM_MONITOR

      CheckIfOperationIsCancelled();
   }

   // Protected service that allows MChannel subclasses to notify the monitor
   // that some bytes are sent.
   //
   void DoNotifyByteTX(const char* data, int length)
   {
      m_countBytesSent += length;

      #if !M_NO_MCOM_MONITOR
         if ( m_monitor != NULL && m_monitor->IsListening() )
            m_monitor->OnByteTX(data, length);
      #endif // M_NO_MCOM_MONITOR

      CheckIfOperationIsCancelled();
   }

#endif

/// \endcond SHOW_INTERNAL
public: // Services:


protected: // Methods:
/// \cond SHOW_INTERNAL

   // Discard the contents of the input buffer of the channel.
   // All characters in the receive buffer that are waiting to be read are lost.
   //
   // \pre The channel is open, otherwise the
   // operation fails with an exception.
   //
   virtual void DoClearInputBuffer();

   virtual unsigned DoWrite(const char* buf, unsigned len) = 0;
   virtual unsigned DoRead(char* buf, unsigned len, unsigned timeout) = 0;
   unsigned DoReadCancellable(char* buf, unsigned size, unsigned timeout, bool sendToMonitor);

   void DoInitChannel();

protected: // Attributes:

#if !M_NO_MCOM_MONITOR

   // Monitor associated with this channel
   //
   MMonitor::Pointer m_monitor;

#endif // !M_NO_MCOM_MONITOR

   // True if auto answer mode.
   // This mode is false by default.
   //
   bool m_isAutoAnswer;

   // Auto answer timeout. This is a timeout in which computer waits for an incoming
   // call from meter.
   //
   unsigned m_autoAnswerTimeout;

   // Interlocked thread-safe variable that tells the channel that
   // the communication should not be terminated until the value of the protector
   // becomes zero.
   //
   MInterlocked m_cancelCommunicationGuard;

   // Whether the communication shall be terminated.
   // Value zero tells no, value one tells yes, value two means yes with disconnect.
   //
   volatile int m_cancelCommunication;

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

   // The number of bytes sent through the channel since last Connect,
   // or count reset.
   //
   unsigned m_countBytesSent;

   // The number of bytes received by the channel since last Connect,
   // or count reset.
   //
   unsigned m_countBytesReceived;

   // Whether the echo is enabled on the channel, so every character
   // sent has to be read.
   //
   bool m_echo;

#if !M_NO_MCOM_MONITOR
   // If ECHO is on, send echo bytes to the monitor
   //
   bool m_sendEchoBytesToMonitor;
#endif

   // Additional buffer, which is inserted in the beginning of the read buffer
   // in case Unread operation was performed.
   //
   MByteString m_unreadBuffer;

   M_DECLARE_CLASS(Channel)

/// \endcond SHOW_INTERNAL
};

///@}
#endif
