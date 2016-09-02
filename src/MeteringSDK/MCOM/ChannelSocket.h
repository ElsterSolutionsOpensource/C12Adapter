#ifndef MCOM_CHANNELSOCKET_H
#define MCOM_CHANNELSOCKET_H
/// \addtogroup MCOM
///@{
/// \file MCOM/ChannelSocket.h

#include <MCOM/ChannelSocketBase.h>

#if !M_NO_MCOM_CHANNEL_SOCKET

/// MChannelSocket implements the channel interface for an TCP/IP socket.
///
/// MChannelSocket implements TCP/IP socket behavior over channel socket base. 
/// Socket connections can be made to the peer device (outbound) or from the peer
/// device (inbound). Either connection direction can be made with MChannelSocket or
/// MMChannelSocketCallback. The default property values for MChannelSocket are tuned for an
/// outbound connection, and the default property values for MChannelSocketCallback are
/// tuned for an inbound connection. The connection attempt can take a long time to complete or time out.
/// If the connection is queued with QConnect and committed asynchronously with QCommit(True), then the connection can be
/// aborted with QAbort.
///
class MCOM_CLASS MChannelSocket : public MChannelSocketBase
{
#if !M_NO_MCOM_HANDLE_PEER_DISCONNECT
   friend class MChannelSocketBackgroundHandler;
#endif

public: // Constructor, destructor:

   /// Construct the socket channel.
   ///
   MChannelSocket();

   /// Destructor.
   ///
   virtual ~MChannelSocket();

public: // Services:

   /// Establishes connection to the device using a socket.
   ///
   /// The property \refprop{GetConnectTimeout,ConnectTimeout}, when nonzero, can alter
   /// the duration defined by the operating system for a connection to be established.
   ///
   /// \pre IsConnected should be false before calling this method.
   /// Many OS and program exceptions can be thrown by this method.
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

   /// Disconnect brings down the connection and releases the socket.
   ///
   virtual void Disconnect();

public: // Property handling routines:

   ///@{
   /// Socket object that is used by this channel.
   ///
   /// Setting socket has a feature: what actually happens, this channel's socket is swapped with the given socket.
   /// MChannelSocket accepts only MStreamSocket object as input.
   ///
   MStreamSocket& GetSocket()
   {
      M_ASSERT(m_socketPtr == &m_socket);
      return m_socket;
   }
   virtual void SetSocket(MStreamSocketBase&);
   ///@}

   /// Get the constant socket object that is used by this channel.
   ///
   const MStreamSocket& GetSocketConst() const
   {
      M_ASSERT(m_socketPtr == (MStreamSocketBase*)&m_socket);
      return m_socket;
   }

   ///@{
   /// SOCKS5 proxy configuration string.
   ///
   /// This is a combination of proxy server address, network port number (optional), 
   /// and authorization string (optional), divided with colons. Authorization string is
   /// a username or username and password divided by '@'. Proxy server address is either
   /// the IP address as string, or a DNS name.
   ///
   /// Examples:
   ///   - 10.20.30.40 (uses default port 1080)
   ///   - example.com (uses default port 1080)
   ///   - example.com:4050
   ///   - username:example.com:1080 (with authorization by username)
   ///   - username@@password:example.com:1080 (with authorization by username and password)
   ///
   /// \since MeteringSDK Version 5.0.0.890.
   ///
   /// \default_value "" (empty string)
   ///
   const MStdString& GetProxyString() const
   {
      return m_proxyString;
   }
   void SetProxyString(const MStdString& proxyString)
   {
      m_proxyString = proxyString;
   }
   ///@}

#if !M_NO_MCOM_HANDLE_PEER_DISCONNECT
   ///@{
   /// Determines whether or not to immediately close the socket when the peer has closed (sent message with FIN flag).
   ///
   /// When true, this property helps work around the issue where the peer device locks up because it did not receive a response to its FIN message
   /// in a timely manner. This can happen when the application is busy with its own processing and does not read from the socket for an extended 
   /// period of time. By the time the application does send the ACK, it is too late as the wireless link has already died. 
   /// When FIN is detected, the ACK is sent back, and then the socket is immediately closed by sending FIN to the peer.
   /// This prevents the connection from remaining in FIN_WAIT_1 or FIN_WAIT_2 (half-closed) state for a long time,
   /// leading to known problems in certain devices.
   ///
   /// Note that many socket channels can have different values of HandlePeerDisconnect in the same process.
   /// There is only one thread that polls the open sockets and it only monitors those that have HandlePeerDisconnect = true.
   /// This design helps minimize the impact to performance, but it adds extra synchronization,
   /// which can lead to extended delays in socket handling when there are many monitored sockets (hundreds or thousands).
   ///
   /// \since MeteringSDK Version 5.4.0.2427.
   ///
   /// \possible_values
   ///  - True - the open socket is periodically polled in order to handle the incoming FIN message in a timely manner.
   ///  - False - the socket is not polled. This is also the behavior of builds prior to 5.4.0.2400.
   ///
   /// \default_value True
   ///
   bool GetHandlePeerDisconnect() const
   {
      return m_handlePeerDisconnect;
   }
   void SetHandlePeerDisconnect(bool yes);
   ///@}
#endif

   ///@{
   /// Timeout in seconds for TCP/IP connection to be established.
   ///
   /// A connection timeout will only result from socket errors such as when the connected party
   /// did not respond after a certain period of time. Any other socket connect errors,
   /// such as Destination host is unreachable, are reported immediately.
   ///
   /// If this property is zero, the operating system defined connect timeout is in effect,
   /// which can vary from 10 to 180 seconds or more, depending on the internal TCP/IP settings, 
   /// and the networking devices involved.
   ///
   /// If the property is nonzero, the operating system behavior is altered, and a new timeout takes effect.
   ///
   /// When ConnectTimeout expires before the operating system timeout, the error thrown is 
   /// \ref MCOMException, code 0x8004180E, with text (in case of English locale): 
   /// "Failed to connect within X seconds" where X is the value of this property. 
   /// Small connect timeouts are useful on private networks where the connection time  
   /// to an online device is fast.
   ///
   /// When ConnectTimeout expires after the operating system timeout, the connection is attempted in a 
   /// loop until it is established, or fails. If it fails, the error thrown is \ref MESocketError with 
   /// text from the last operating system error. Very large connect timeouts can be used to wait for a 
   /// device to come online. Due to limits in the operating system timer utilized by MeteringSDK, 
   /// values larger than MAX_INT/1000 (which is about 24 days), is considered to be an eternity, and
   /// the timeout is disabled (it will wait forever). Care should be taken when using large values, 
   /// especially with synchronous communications, as one could lock up an application. Applications 
   /// that allow the user to specify the ConnectTimeout, may wish to limit the maximum value.</p>
   ///
   /// At any moment, the user can cancel the ongoing \ref Connect() call by issuing \ref CancelCommunication(),
   /// in which case \ref MEOperationCancelled is raised.
   ///
   /// \possible_values
   ///   - 0                        Operating system defined TCP/IP connection timeout is in effect.
   ///   - 1 .. MAX_INT/1000 - 1    Connect timeout in seconds takes effect for cases when the host is not responding.
   ///   - MAX_INT/1000 .. MAX_UINT Try to connect forever until succeeding or failing with a socket error, which is not a timeout.
   ///
   /// \default_value 0 means the connect time is limited only by the OS behavior.
   ///
   unsigned GetConnectTimeout() const
   {
      return m_connectTimeout;
   }
   void SetConnectTimeout(unsigned timeout)
   {
      m_connectTimeout = timeout;
   }
   ///@}

public: // Services:

   /// Throw an appropriate exception if the channel is not connected.
   /// The exception can be different depending on whether the connection was
   /// not made previously, or if the connection was unexpectedly terminated.
   ///
   /// \pre The channel is connected, or an exception is thrown.
   ///
   /// \see CheckIfConnectedConst - constant version of this method
   ///
   virtual void CheckIfConnected();

private: // Methods:

   // Translates socket codes to channel codes, if necessary.
   //
   virtual M_NORETURN_FUNC void DoHandleExceptionAndRethrow(MException&);
   M_NORETURN_FUNC void DoHandleExceptionAndRethrow(MException& ex) const
   {
      const_cast<MChannelSocket*>(this)->DoHandleExceptionAndRethrow(ex);
   }

private: // Attributes:

   // Socket object.
   //
   MStreamSocket m_socket;

   // SOCS proxy string
   //
   MStdString m_proxyString;

#if !M_NO_MCOM_HANDLE_PEER_DISCONNECT
   // When true, the socket will be closed immediately when the other end closes
   //
   bool m_handlePeerDisconnect;

   // State of background handling of HandlePeerDisconnect
   //
   bool m_closedByBackgroundHandler;
#endif

   // Timeout to wait for the connection to establish
   //
   unsigned m_connectTimeout;

   M_DECLARE_CLASS(ChannelSocket)
};

#endif // !M_NO_MCOM_CHANNEL_SOCKET

///@}
#endif
