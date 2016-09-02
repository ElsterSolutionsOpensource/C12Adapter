#ifndef MCOM_CHANNELSOCKETBASE_H
#define MCOM_CHANNELSOCKETBASE_H
/// \addtogroup MCOM
///@{
/// \file MCOM/ChannelSocketBase.h

#include <MCOM/MCOMDefs.h>
#include <MCOM/Channel.h>

#if !M_NO_MCOM_CHANNEL_SOCKET

/// MChannelSocket implements the channel interface for an IP socket, either TCP or UDP.
///
/// MChannelSocket implements an IP socket behavior and inherits properties from its abstract
/// parent class, Channel.
///
class MCOM_CLASS MChannelSocketBase : public MChannel
{
protected: // Constructor:

   /// Construct the abstract socket channel.
   ///
   MChannelSocketBase();

public: // Destructor:

   /// Destructor.
   ///
   virtual ~MChannelSocketBase();

public: // Services:

   /// Partly implemented connect method, overridden by children.
   ///
   /// This method handles RAS connection, when RAS is enabled.
   ///
   virtual void Connect() = 0;

   /// Returns the current state of the socket channel.
   ///
   virtual bool IsConnected() const;

   /// Partly implemented disconnect method, overridden by children.
   ///
   /// This method handles RAS disconnect, when RAS is enabled.
   ///
   virtual void Disconnect();

public: // Property handling routines:

   ///@{
   /// Port to listen to when in Auto Answer mode is enabled (waiting for the incoming connection).
   ///
   /// \refprop{SetAutoAnswer,AutoAnswer} must be true for the channel to be ready for an inbound connection
   /// and for this property to have any effect.
   ///
   /// \since MeteringSDK Version 4.0.15. In MeteringSDK Version 6.2.0.4233, 
   ///        support was added for UDP (User Datagram Protocol) messages. To facilitate 
   ///        this change, a new abstract class was created, MChannelSocketBase, which 
   ///        became the parent for all other socket classes. Prior to Version 6.2.0.4233,
   ///        this property was in the MChannelSocket class.
   ///
   /// \default_value 1153
   ///
   /// \possible_values 0 .. 0xFFFF
   ///
   unsigned GetAutoAnswerPort() const
   {
      return m_autoAnswerPort;
   }
   void SetAutoAnswerPort(unsigned port);
   ///@}

   ///@{
   /// Address to bind to when Auto Answer mode is enabled (waiting for the incoming connection).
   ///
   /// When this is an empty string, all active network interfaces are bound for listening.
   /// Otherwise, this has to be a valid IP address or host name of an interface on which to listen.
   /// \refprop{SetAutoAnswer,AutoAnswer} must be true for the channel to be ready for an inbound connection
   /// and for this property to have any effect.
   ///
   /// \since MeteringSDK Version 6.2.0.4315. In MeteringSDK Version 6.2.0.4233, 
   ///        support was added for UDP (User Datagram Protocol) messages. To facilitate 
   ///        this change, a new abstract class was created, MChannelSocketBase, which 
   ///        became the parent for all other socket classes. Prior to Version 6.2.0.4233,
   ///        this property was in the MChannelSocket class.
   ///
   /// \default_value "" (empty string)
   ///
   const MStdString& GetAutoAnswerAddress() const
   {
      return m_autoAnswerAddress;
   }
   void SetAutoAnswerAddress(const MStdString& addr)
   {
      m_autoAnswerAddress = addr;
   }
   ///@}

   ///@{
   /// Socket object that is used by this channel.
   ///
   /// Setting socket object has a feature: what actually happens, this channel's socket is swapped with the given socket.
   /// The channel type accepted depends on the actual final class: ChannelSocket accepts only StreamSocket,
   /// while ChannelSocketUdp accepts only StreamSocketUdp.
   ///
   MStreamSocketBase& GetSocket()
   {
      return *m_socketPtr;
   }
   virtual void SetSocket(MStreamSocketBase&) = 0;
   ///@}

   /// Get the constant socket object that is used by this channel.
   ///
   const MStreamSocketBase& GetSocketConst() const
   {
      return *m_socketPtr;
   }

   ///@{
   /// The address of the peer socket, either an IP address as string, or a DNS name.
   ///
   /// It is the socket to which this one connects, either
   /// a direct IP address, or the host name.
   ///
   /// \since MeteringSDK Version 2.1.27. In MeteringSDK Version 6.2.0.4233, 
   ///        support was added for UDP (User Datagram Protocol) messages. To facilitate 
   ///        this change, a new abstract class was created, MChannelSocketBase, which 
   ///        became the parent for all other socket classes. Prior to Version 6.2.0.4233,
   ///        this property was in the MChannelSocket class.
   ///
   /// \default_value "" (empty string)
   ///
   const MStdString& GetPeerAddress() const
   {
      return m_peerAddress;
   }
   void SetPeerAddress(const MStdString& addr)
   {
      m_peerAddress = addr;
   }
   ///@}

   ///@{
   /// Port number of the peer socket to which socket object connects.
   ///
   /// The default value 1153 is suited for C12.22.
   ///
   /// \since MeteringSDK Version 2.1.27. In MeteringSDK Version 6.2.0.4233, 
   ///        support was added for UDP (User Datagram Protocol) messages. To facilitate 
   ///        this change, a new abstract class was created, MChannelSocketBase, which 
   ///        became the parent for all other socket classes. Prior to Version 6.2.0.4233,
   ///        this property was in the MChannelSocket class.
   ///
   /// \default_value 1153
   ///
   /// \possible_values 0x0 .. 0xFFFF
   ///
   unsigned GetPeerPort() const
   {
      return m_peerPort;
   }
   void SetPeerPort(unsigned port);
   ///@}

   /// Get the actual address of the local socket as it is known by Sockets.
   ///
   /// The address of the local socket that was used during communication. It is available only
   /// during an active connection (after the completion of a successful Connect() and
   /// before the termination of the connection due to a Disconnect() or an error). If the
   /// property is queried without an active connection, an error will be raised. Use
   /// IsConnected() to determine if this property is available.
   ///
   /// \pre The connection should be open, otherwise a socket related exception is thrown.
   ///
   /// \since MeteringSDK Version 2.1.27. In MeteringSDK Version 6.2.0.4233, 
   ///        support was added for UDP (User Datagram Protocol) messages. To facilitate 
   ///        this change, a new abstract class was created, MChannelSocketBase, which 
   ///        became the parent for all other socket classes. Prior to Version 6.2.0.4233,
   ///        this property was in the MChannelSocket class.
   ///
   /// \return string
   ///
   MStdString GetActualLocalAddress() const;

   /// Get the actual port number of the local socket as it is known by Sockets.
   ///
   /// The port number of the local socket used for the outbound communication. The
   /// Actual Local Port number is a random number assigned by Sockets. It is available only
   /// during an active connection (after the completion of a successful Connect() and
   /// before the termination of the connection due to a Disconnect() or an error). If the
   /// property is queried without an active connection, an error will be raised. Use IsConnected()
   /// to determine if this property is available.
   ///
   /// \pre The connection should be open, otherwise a socket related exception is thrown.
   ///
   /// \since MeteringSDK Version 2.1.27. In MeteringSDK Version 6.2.0.4233, 
   ///        support was added for UDP (User Datagram Protocol) messages. To facilitate 
   ///        this change, a new abstract class was created, MChannelSocketBase, which 
   ///        became the parent for all other socket classes. Prior to Version 6.2.0.4233,
   ///        this property was in the MChannelSocket class.
   ///
   /// \return int
   ///
   unsigned GetActualLocalPort() const;

   /// Get the actual address of the peer socket as it is known by Sockets.
   ///
   /// The address of the peer socket as known to the sockets library. It is available only
   /// during an active connection (after the completion of a successful Connect() and
   /// before the termination of the connection due to a Disconnect() or an error). If the
   /// property is queried without an active connection, an error will be raised. Use IsConnected()
   /// to determine if this property is available.
   ///
   /// \pre The connection should be open, otherwise a socket related exception is thrown.
   ///
   /// \since MeteringSDK Version 2.1.27. In MeteringSDK Version 6.2.0.4233, 
   ///        support was added for UDP (User Datagram Protocol) messages. To facilitate 
   ///        this change, a new abstract class was created, MChannelSocketBase, which 
   ///        became the parent for all other socket classes. Prior to Version 6.2.0.4233,
   ///        this property was in the MChannelSocket class.
   ///
   /// \return string
   ///
   MStdString GetActualPeerAddress() const;

   /// Get the actual port number of the peer socket as it is known by Sockets.
   ///
   /// The port number of the peer socket as known to the sockets library. It is available only
   /// during an active connection (after the completion of a successful Connect() and
   /// before the termination of the connection due to a Disconnect() or an error). If the
   /// property is queried without an active connection, an error will be raised. Use IsConnected()
   /// to determine if this property is available.
   ///
   /// The Actual Peer Port number will be the same as \refprop{GetPeerPort,PeerPort} unless the server has multiple clients,
   /// in which case it will wait on one port, and accept connections on another.
   ///
   /// \pre The connection should be open, otherwise a socket related exception is thrown.
   ///
   /// \since MeteringSDK Version 2.1.27. In MeteringSDK Version 6.2.0.4233, 
   ///        support was added for UDP (User Datagram Protocol) messages. To facilitate 
   ///        this change, a new abstract class was created, MChannelSocketBase, which 
   ///        became the parent for all other socket classes. Prior to Version 6.2.0.4233,
   ///        this property was in the MChannelSocket class.
   ///
   /// \return int
   ///
   unsigned GetActualPeerPort() const;

#if !M_NO_MCOM_RAS_DIAL

   ///@{
   /// Dial a RAS connection with a given name prior to connecting, and hangup at disconnect.
   ///
   /// RAS connection has to be configured and its name given to this property,
   /// in which case the connection will be dialed prior to connecting and hung up after Disconnect() call.
   ///
   /// \since MeteringSDK Version 5.2.0.1914. In MeteringSDK Version 6.2.0.4233, 
   ///        support was added for UDP (User Datagram Protocol) messages. To facilitate 
   ///        this change, a new abstract class was created, MChannelSocketBase, which 
   ///        became the parent for all other socket classes. Prior to Version 6.2.0.4233,
   ///        this property was in the MChannelSocket class.
   ///
   /// \default_value "", no RAS connection to be made
   ///
   const MStdString& GetRasDialName() const
   {
      return m_rasDialName;
   }
   void SetRasDialName(const MStdString& rasDialName)
   {
      m_rasDialName = rasDialName;
   }
   ///@}

   ///@{
   /// The RAS Dial connection delay is the amount of time to wait after opening the RasDial connection,
   /// before opening the socket connection.
   ///
   /// \since MeteringSDK Version 5.2.1.2122. In MeteringSDK Version 6.2.0.4233, 
   ///        support was added for UDP (User Datagram Protocol) messages. To facilitate 
   ///        this change, a new abstract class was created, MChannelSocketBase, which 
   ///        became the parent for all other socket classes. Prior to Version 6.2.0.4233,
   ///        this property was in the MChannelSocket class.
   ///
   /// \default_value 50
   ///
   unsigned GetRasDialConnectDelay() const
   {
      return m_rasDialConnectDelay;
   }
   void SetRasDialConnectDelay(unsigned rasDialConnectDelay)
   {
      m_rasDialConnectDelay = rasDialConnectDelay;
   }
   ///@}

   ///@{
   /// The RAS Dial disconnect delay is the amount of time to wait after closing the socket connection,
   /// before closing the RasDial connection.
   ///
   /// \since MeteringSDK Version 5.2.1.2122. In MeteringSDK Version 6.2.0.4233, 
   ///        support was added for UDP (User Datagram Protocol) messages. To facilitate 
   ///        this change, a new abstract class was created, MChannelSocketBase, which 
   ///        became the parent for all other socket classes. Prior to Version 6.2.0.4233,
   ///        this property was in the MChannelSocket class.
   ///
   /// \default_value 100
   ///
   unsigned GetRasDialDisconnectDelay() const
   {
      return m_rasDialDisconnectDelay;
   }
   void SetRasDialDisconnectDelay(unsigned rasDialDisonnectDelay)
   {
      m_rasDialDisconnectDelay = rasDialDisonnectDelay;
   }
   ///@}

#endif

public: // Methods:

   /// Ensure that the characters from the output buffer are sent.
   /// The parameter, if specified, should match the number of characters
   /// written into the serial port right before FlushOutputBuffer is called.
   /// If the parameter is missing, the biggest possible number of characters will
   /// be ensured to go away. For the case of socket it is known that
   /// this number has no relevance.
   ///
   /// \pre The channel is open, otherwise the operation fails with the exception.
   ///
   virtual void FlushOutputBuffer(unsigned numberOfCharsInBuffer = UINT_MAX);

#if !M_NO_MCOM_RAS_DIAL

   /// Dial a RAS connection given as RAS dial name property
   ///
   /// When \refprop{GetRasDialName, RasDialName} is specified,
   /// RAS connection will be made transparently within Connect()
   /// unless this call is made prior to connecting, in which case
   /// there can be a separate explicit call RasHangUp to close connection.
   ///
   /// \pre
   ///   - Property \refprop{GetRasDialName, RasDialName} should not be an empty string,
   ///     and it should be a previously configured RAS connection name.
   ///   - This call will succeed only from a Windows GUI application, not a service.
   ///
   void RasConnect();

   /// Hangup RAS connection if it was made previously.
   ///
   /// When \refprop{GetRasDialName, RasDialName} is specified,
   /// RAS connection will be attempted to be hung up.
   /// The method never throws any errors, all of them are ignored.
   ///
   void RasDisconnect() M_NO_THROW;

#endif

   /// Return a string that identifies the media through
   /// which this channel is talking. For socket based channels,
   /// it will return a host name, SOCKET string, and peer name and port.
   ///
   virtual MStdString GetMediaIdentification() const;

protected: // Methods:
/// \cond SHOW_INTERNAL

   // Discard the contents of the input buffer of the channel.
   // All characters in the receive buffer that are waiting to be read are lost.
   //
   // \pre The channel is open, otherwise the operation fails with the exception.
   //
   virtual void DoClearInputBuffer();

   virtual unsigned DoWrite(const char* buf, unsigned len);
   virtual unsigned DoRead(char* buf, unsigned numberToRead, unsigned timeout);

   // Translates socket codes to channel codes, if necessary.
   //
   virtual M_NORETURN_FUNC void DoHandleExceptionAndRethrow(MException&);
   M_NORETURN_FUNC void DoHandleExceptionAndRethrow(MException& ex) const
   {
      const_cast<MChannelSocketBase*>(this)->DoHandleExceptionAndRethrow(ex);
   }

protected: // Attributes:

   // Socket object.
   //
   MStreamSocketBase* m_socketPtr;

   // Address of the peer socket, one to which this socket will be connecting.
   //
   MStdString m_peerAddress;

   // Port name of the peer socket, one to which this socket will be connecting.
   //
   unsigned m_peerPort;

   // Port to wait for the incoming connection.
   //
   unsigned m_autoAnswerPort;

   // Auto answer address
   //
   MStdString m_autoAnswerAddress;

#if !M_NO_MCOM_RAS_DIAL

   // Whether to dial RAS connection, and if yes which one
   //
   MStdString m_rasDialName;

   // RAS connection if it was made
   //
   DWORD_PTR m_rasConnection;

   // Whether RAS connection was made inside Connect call, should be hung up in Disconnect
   //
   bool m_rasConnectionMadeInConnect;

   // The RAS dial connection delay.
   // This is the amount of time to wait after opening the RAS Dial connection, before opening the socket connection.
   //
   unsigned m_rasDialConnectDelay;

   // The RAS dial disconnect delay.
   // This is the amount of time to wait after closing the socket connection, before closing the RAS Dial connection.
   //
   unsigned m_rasDialDisconnectDelay;

#endif

   // Used by foreground and background thread
   //
   MCriticalSection m_channelOperationCriticalSection;

/// \endcond SHOW_INTERNAL

   M_DECLARE_CLASS(ChannelSocketBase)
};

#endif // !M_NO_MCOM_CHANNEL_SOCKET_BASE

///@}
#endif
