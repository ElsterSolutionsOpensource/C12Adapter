#ifndef MCORE_MSTREAMSOCKET_H
#define MCORE_MSTREAMSOCKET_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MStreamSocket.h

#include <MCORE/MStreamSocketBase.h>

#if !M_NO_SOCKETS

/// TCP/IP socket, reliable stream.
///
/// The services below can throw MESocketError in the event of an erroneous socket operation.
///
/// Implementation detail: Windows does not support SO_RCVTIMEO
/// socket option for synchronous ports, and read timeout is supported
/// through the call to select. The timeout value in milliseconds is given to services
/// that support timeouts.
///
class M_CLASS MStreamSocket : public MStreamSocketBase
{
public:

   /// Generic class that can be used for cancellation of open socket operation.
   ///
   /// Open socket methods have optional pointer to a child of this class
   /// and if it is given, they will periodically call OperationHandler::CheckIfCancelled.
   /// In this method, specific implementations shall throw an exception if 
   /// opening needs to be canceled.
   ///
   class OperationHandler
   {
   public: // Methods:

      /// Virtual destructor of the class.
      ///
      /// The abstract implementation does nothing.
      ///
      virtual ~OperationHandler()
      {
      }

      /// This method shall be overwritten to check if the socket opening shall be canceled.
      ///
      /// If application decides the operation shall be canceled, it shall throw any desired
      /// exception from this call, typically MEOperationCancelled.
      ///
      virtual void CheckIfCancelled() = 0;
   };

public:

   /// Constructor that creates socket based on existing socket handle.
   ///
   /// This is a C++ only method that is convenient for cases when the socket is open by any means
   /// other than the socket stream object.
   ///
   /// \param socketHandle
   ///    Handle to use by the socket.
   ///    When handle is an invalid socket value, or not present, the socket will not be opened.
   ///
   explicit MStreamSocket(SocketHandleType socketHandle = InvalidSocket);

   /// Destructor, destroys the socket object and never throws an exception.
   ///
   /// Use Close if there is a need to report any socket related exceptions.
   ///
   virtual ~MStreamSocket() M_NO_THROW;

public: // Properties:

   /// Return a representative name of a socket stream.
   ///
   /// This method overwrites the parent implementation and gives descriptive information about the socket.
   /// For example, it can return client host name or IP address, and its port number.
   /// The particular information is not guaranteed to have the same format.
   ///
   virtual MStdString GetName() const;

   /// Get the IP address for the peer socket.
   ///
   /// The IP address returned has text representation, such as "10.0.0.120".
   ///
   /// \pre Socket has to be initialized, and the connection is active, otherwise a system error is thrown.
   ///
   virtual MStdString GetPeerSocketName() const;

   /// Get the port for the peer socket.
   ///
   /// \pre Socket has to be initialized, and the connection is active, otherwise a system error is thrown.
   ///
   virtual unsigned GetPeerSocketPort() const;

   ///@{
   /// Socket send timeout in milliseconds, 60000 by default.
   ///
   /// If a send call times out, the connection is in an indeterminate state and should be closed.
   ///
   unsigned GetSendTimeout() const
   {
      return m_sendTimeout;
   }
   void SetSendTimeout(unsigned milliseconds)
   {
      m_sendTimeout = milliseconds;
   }
   ///@}

   ///@{
   /// No Delay socket option.
   ///
   /// Having No Delay true disables Nagle algorithm, which might cause an increase in network traffic,
   /// with smaller than needed packets wasting bandwidth.
   ///
   bool GetNoDelay() const;
   void SetNoDelay(bool);
   ///@}

   /// Set Linger socket option.
   ///
   /// This controls how the socket behaves when the socket is closed.
   /// If doLinger is set to true and the data are present in the buffer,
   /// will delay closing the socket until the data are sent.
   ///
   /// \param doLinger
   ///    Whether to enable linger
   ///
   /// \param lingerTime
   ///    For how long to linger. This has to be a positive value.
   ///    When doLinger is true, and lingerTime is zero, RST is sent immediately at the close.
   ///
   /// \pre Socket has to be created.
   ///
   void SetLinger(bool doLinger, int lingerTime);

   ///@{
   /// Send Buffer size socket option.
   ///
   /// This controls how many bytes are allocated for the socket send buffer.
   ///
   /// \pre Socket has to be created.
   ///
   int GetSendBufferSize() const;
   void SetSendBufferSize(int);
   ///@}

   ///@{
   /// Receive Buffer size socket option.
   ///
   /// This controls how many bytes are allocated for the socket receive buffer.
   ///
   /// \pre Socket has to be created.
   ///
   int GetReceiveBufferSize() const;
   void SetReceiveBufferSize(int size);
   ///@}

public: // Methods:

   /// Create client socket that connects to the server.
   ///
   /// \param port
   ///    Port to connect to.
   ///
   /// \param address
   ///    IP address or a DNS name of the server to which the connection has to be made.
   ///
   /// \see ConnectWithProxy - Connect to a socket through SOCKS proxy.
   ///
   void Connect(unsigned port, const MStdString& address)
   {
      ConnectInterruptible(port, address, NULL);
   }

   /// Create client socket that connects to the server.
   ///
   /// \param port
   ///    Port to connect to.
   ///
   /// \param address
   ///    IP address or a DNS name of the server to which the connection has to be made.
   ///
   /// \param oph
   ///    Optional pointer to an operation handler that is capable of canceling the opening.
   ///
   /// \see ConnectWithProxy - Connect to a socket through SOCKS proxy.
   ///
   void ConnectInterruptible(unsigned port, const MStdString& address, OperationHandler* oph);

#if !M_NO_SOCKETS_SOCKS

   /// Create client socket that connects to the server through a SOCKS proxy.
   ///
   /// Supported are SOCKS4, SOCKS4+ and SOCKS5 proxy servers.
   ///
   /// \param port
   ///    Port to connect to.
   ///
   /// \param address
   ///    IP address or a DNS name of the server to which the connection has to be made.
   ///
   /// \param proxyAddress
   ///    Proxy address in one of the following forms:
   ///      - "" means no proxy shall be used, connect directly to the given host.
   ///      - "IP_ADDRESS:PORT" such as "10.0.0.123:345" for SOCKS4 proxy.
   ///      - "IP_ADDRESS:PORT" such as "10.0.0.123:345" for SOCKS4 proxy.
   ///      - "host.name:PORT" such as "example.com:678" for SOCKS4+ proxy.
   ///      - "user:host.name:PORT" such as "dennis_richie:example.com:678" for SOCKS5 proxy without password.
   ///      - "user@password:host.name:PORT" such as "dennis_richie@unix:example.com:678" for SOCKS5 proxy with password.
   ///
   void ConnectWithProxy(unsigned port, const MStdString& address, const MStdString& proxyAddress)
   {
      ConnectWithProxyInterruptible(port, address, proxyAddress, NULL);
   }

   /// Create client socket that connects to the server through a SOCKS proxy.
   ///
   /// Supported are SOCKS4, SOCKS4+ and SOCKS5 proxy servers.
   ///
   /// \param port
   ///    Port to connect to.
   ///
   /// \param address
   ///    IP address or a DNS name of the server to which the connection has to be made.
   ///
   /// \param proxyAddress
   ///    Proxy address in one of the following forms:
   ///      - "" means no proxy shall be used, connect directly to the given host.
   ///      - "IP_ADDRESS:PORT" such as "10.0.0.123:345" for SOCKS4 proxy.
   ///      - "IP_ADDRESS:PORT" such as "10.0.0.123:345" for SOCKS4 proxy.
   ///      - "host.name:PORT" such as "example.com:678" for SOCKS4+ proxy.
   ///      - "user:host.name:PORT" such as "dennis_richie:example.com:678" for SOCKS5 proxy without password.
   ///      - "user@password:host.name:PORT" such as "dennis_richie@unix:example.com:678" for SOCKS5 proxy with password.
   ///
   /// \param oph
   ///    Pointer to an operation handler that is capable of canceling the opening.
   ///    If null, this call is equivalent to ConnectWithProxy.
   ///
   void ConnectWithProxyInterruptible(unsigned port, const MStdString& address, const MStdString& proxyAddress, OperationHandler* oph);

#endif // !M_NO_SOCKETS_SOCKS

   /// Listen for incoming socket connections.
   ///
   /// Usually, there is a special dedicated port to listen for incoming connections.
   /// When the connection comes, Accept has to be called with the new socket as parameter.
   ///
   /// \param backlog
   ///    Length of the pending connections queue, by default it is 5 pending connections.
   ///
   /// \pre The socket has to be created successfully, and bound to a port known by clients.
   ///      Otherwise this is a programming error, and the socket-specific exception is called.
   ///
   void Listen(unsigned backlog = 5);

   /// Accept the connection by the server and assign a new connection with the client.
   ///
   /// \param socket
   ///    The uninitialized socket object to which the connection will be accepted.
   ///
   /// \see TimedAccept method that can timeout if no connections arrived.
   ///
   void Accept(MStreamSocket& socket);

   /// Accept the connection by the server and assign a new connection with the client, or timeout.
   ///
   /// This method is typically called in the loop that also checks if the application shall
   /// cease waiting for the connection. For example, if it shall exit.
   ///
   /// \param socket
   ///    The uninitialized socket object to which the connection will be accepted.
   ///
   /// \param timeout
   ///    Milliseconds to wait for the connection.
   ///
   /// \return
   ///    - True means the connection is successfully accepted.
   ///    - False means there is no connection, and timeout has expired.
   ///
   /// \see Accept method for infinite wait.
   ///
   bool TimedAccept(MStreamSocket& socket, unsigned timeout);

   /// Swap this socket and the given socket by exchanging their handles and other properties.
   ///
   /// After the successful completion, this socket and other socket will be exchanged.
   ///
   void Swap(MStreamSocket& other);

   /// Clear the input buffer by reading all the available data from the socket, if any are there.
   ///
   /// \pre The socket is alive, other read-related errors possible.
   ///
   virtual void ClearInputBuffer();

   /// Analog of the standard socket function recv.
   ///
   /// Receive a number of bytes currently available in the socket.
   /// This method is an alternative to stream Read methods, however in majority of cases Read methods are sufficient
   /// to manipulate the socket. They also have more functionality such as handling of encrypted or compressed streams.
   ///
   /// \param buffer
   ///     Buffer into which to receive the data.
   ///
   /// \param length
   ///     Length of the buffer or how many bytes to read.
   ///
   /// \param flags
   ///     Standard recv flags.
   ///
   /// \return
   ///     How many bytes are received into buffer.
   ///
   /// \pre The connection is alive, otherwise the socket exception is raised. It is okay to read zero bytes.
   ///
   unsigned Recv(char* buffer, unsigned length, int flags);
   
   /// Send the buffer through the socket, standard socket function send.
   ///
   /// Send as many bytes as the current send buffer can fit.
   /// This method is an alternative to stream Write methods, however in majority of cases Write methods are sufficient
   /// to manipulate the socket. They also have more functionality such as handling of encrypted or compressed streams.
   ///
   /// \param buffer
   ///     Buffer, which shall be written to socket.
   ///
   /// \param length
   ///     Length of the buffer to write.
   ///
   /// \param flags
   ///     Standard send flags.
   ///
   /// \return
   ///     How many bytes are sent.
   ///
   /// \pre The connection is alive, otherwise the connection-specific exception is thrown.
   ///
   unsigned Send(const char* buffer, unsigned length, int flags);

   /// The number of bytes in the receive buffer that can be read immediately.
   ///
   /// Returns the number of bytes ready to be read from socket.
   ///
   /// \pre The connection is alive, otherwise the connection-specific exception is thrown.
   ///
   /// \see MStreamSocketBase::WaitToReceive - Waits the specified time for the input data to arrive.
   ///
   virtual unsigned GetBytesReadyToRead() const;

public: // Semi-private methods:
/// \cond SHOW_INTERNAL

   /// Listen for incoming socket connection, use default backlog.
   ///
   /// Usually, there is a special dedicated port to listen for incoming connections.
   /// When the connection comes, Accept has to be called with the new socket as parameter.
   /// This method uses the default backlog of pending connections equal to five.
   ///
   /// \pre The socket has to be created successfully, and bound to a port known by clients.
   ///      Otherwise this is a programming error, and the socket-specific exception is called.
   ///
   void DoListen();

protected: // Methods:

   virtual unsigned DoReadAllAvailableBytesImpl(char* buf, unsigned len);
   virtual unsigned DoReadAvailableBytesImpl   (char* buf, unsigned len);

   virtual void DoWriteBytesImpl(const char* buf, unsigned len);

   M_NORETURN_FUNC virtual void DoThrowEndOfStream();

/// \endcond SHOW_INTERNAL
private:

   unsigned DoReadAvailableBytesPrivate(char* buf, unsigned len, bool throwException);

private: // Properties:

   // Socket send operation timeout
   //
   unsigned m_sendTimeout;

   M_DECLARE_CLASS(StreamSocket)
};

#endif
///@}
#endif
