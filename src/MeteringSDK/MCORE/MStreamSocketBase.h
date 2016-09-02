#ifndef MCORE_MSTREAMSOCKETBASE_H
#define MCORE_MSTREAMSOCKETBASE_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MStreamSocketBase.h

#include <MCORE/MStream.h>
#include <MCORE/MSockOptEnum.h>

#if !M_NO_SOCKETS || !M_NO_SOCKETS_UDP

#if (M_OS & M_OS_WINDOWS) != 0 // in Windows we cannot expose sockets system includes as there are too many variants
   struct addrinfo;
   struct fd_set;
#elif !(M_OS & M_OS_NUTTX)
   #include <netinet/tcp.h>
#endif

/// Abstract IP socket, either TCP or UDP.
///
/// The services below can throw MESocketError in the event of an erroneous socket operation.
///
/// Implementation detail: Windows does not support SO_RCVTIMEO
/// socket option for synchronous ports, and read timeout is supported
/// through the call to select. The timeout value in milliseconds is given to services
/// that support timeouts.
///
class M_CLASS MStreamSocketBase : public MStream
{
public:

   ///@{
   /// Operating system dependent socket handle type.
   ///
#if (M_OS & M_OS_WINDOWS) != 0
   typedef UINT_PTR SocketHandleType;
#else
   typedef int SocketHandleType;
#endif
   ///@}

#if (M_OS & M_OS_WINDOWS) != 0
   /// Socket length type, exact equivalent of the system type but in the local class scope.
   ///
   /// This allows not exposing the system header file to clients on Windows
   /// where this is problematic due to many variants of socket system headers.
   ///
   typedef int socklen_t;
#endif

   /// Value of socket handle that corresponds to uninitialized or invalid socket.
   ///
   static const SocketHandleType InvalidSocket = (SocketHandleType)(~0);

   /// Return value that corresponds to uninitialized or invalid socket.
   ///
   static const int SocketErrorStatus = -1;

   /// Timeout that represents infinity.
   ///
   /// Not entirely infinite, but certainly too big for a socket.
   ///
   static const unsigned TimeoutInfinite = (unsigned)-1;

   /// Default socket read timeout in milliseconds.
   ///
   /// One minute by default.
   ///
   static const unsigned TimeoutDefault  = 60000;

protected: // Types:
/// \cond SHOW_INTERNAL

   // Address information holder, end scope remover class
   //
   class OsAddrinfoHolder
   {
   public: // Data:

      addrinfo* m_pointer;

   public: // Constructor and destructor:

      OsAddrinfoHolder()
      :
         m_pointer(NULL)
      {
      }

      ~OsAddrinfoHolder();

   private:
      OsAddrinfoHolder(const OsAddrinfoHolder&);
      const OsAddrinfoHolder& operator=(const OsAddrinfoHolder&);
   };
   friend class OsAddrinfoHolder;

   class OsSocketHandleHolder
   {
   public: // Data:

      MStreamSocketBase::SocketHandleType m_socketHandle;

   public: // Constructor and destructor:

      OsSocketHandleHolder()
      :
         m_socketHandle(MStreamSocketBase::InvalidSocket)
      {
      }

      ~OsSocketHandleHolder();

   private:

      OsSocketHandleHolder(const OsSocketHandleHolder&);
      const OsSocketHandleHolder& operator=(const OsSocketHandleHolder&);
   };
   friend class OsSocketHandleHolder;

/// \endcond SHOW_INTERNAL
protected:

   /// Constructor that creates socket based on the existing socket handle.
   ///
   /// Socket handles are only exposed to C++ interfaces.
   /// When a valid socket handle is given, the constructor applies sequence of actions
   /// that initialize stream object.
   ///
   /// \param socketHandle
   ///    Handle to use by the socket.
   ///    When handle is InvalidSocket, the stream opening sequence will not apply.
   ///
   explicit MStreamSocketBase(SocketHandleType socketHandle = InvalidSocket);

   /// Destructor, destroys the socket object and never throws an exception.
   ///
   /// Use Close if there is a need to report any socket related exceptions.
   ///
   virtual ~MStreamSocketBase() M_NO_THROW;

public: // Properties:

   ///@{
   /// Socket handle for full operating system dependent control of the socket.
   ///
   /// Caution shall be given when setting a handle as in this case the socket will not be open,
   /// and flags not set. Setting a handle is rather for replacing one open socket with another.
   /// No checks are done by either the getter or the setter of this property.
   ///
   SocketHandleType GetSocketHandle() const
   {
      return m_socketHandle;
   }
   void SetSocketHandle(SocketHandleType);
   ///@}

   ///
   /// Get the local name for this socket.
   ///
   /// This is useful when connecting without binding as it provides a way to determine the local association,
   /// which has been set. This provides the access to the sockets call getsockname.
   ///
   /// \pre Socket has to be initialized, and the connection is available, otherwise a system error is thrown.
   ///
   /// \see GetLocalName - Static method to get the local host name.
   ///
   MStdString GetLocalSocketName() const;

   /// Get the local port for a socket.
   ///
   /// This is useful when connecting without binding
   /// as it provides a way to determine the local association which has been set.
   /// This provides the access to the sockets call getsockname.
   ///
   /// \pre Socket has to be initialized, and the connection is available, otherwise a system error is thrown.
   ///
   unsigned GetLocalSocketPort() const;

   /// Get the IP address for the peer socket.
   ///
   /// The IP address returned has text representation, such as "10.0.0.120".
   ///
   /// \pre Socket has to be initialized, and the connection is active, otherwise a system error is thrown.
   ///
   virtual MStdString GetPeerSocketName() const = 0;

   /// Get the port for the peer socket.
   ///
   /// \pre Socket has to be initialized, and the connection is active, otherwise a system error is thrown.
   ///
   virtual unsigned GetPeerSocketPort() const = 0;

   ///@{
   /// Socket receive timeout in milliseconds, 60000 by default.
   ///
   /// If a receive call times out, the connection is in an indeterminate state and should be closed.
   ///
   unsigned GetReceiveTimeout() const
   {
      return m_receiveTimeout;
   }
   void SetReceiveTimeout(unsigned milliseconds)
   {
      m_receiveTimeout = milliseconds;
   }
   ///@}

public: // Methods:

   /// Bind a server socket.
   ///
   /// \param port
   ///    The port to which to bind.
   ///
   /// \param address
   ///     Address, such as "localhost". When not given, the bind is going to be made on all open interfaces.
   ///
   void Bind(unsigned port, const MStdString& address = MVariant::s_emptyString);

   /// Waits the time given in milliseconds for the input data to arrive.
   ///
   /// Returns true if the data are available, or false if not. 
   ///
   /// \pre The connection is alive, otherwise the connection-specific exception is thrown.
   ///
   /// \see \ref IsInputBufferEmpty - Does not wait, but returns the information immediately.
   ///
   bool WaitToReceive(unsigned timeout = (unsigned)TimeoutInfinite) const;

   /// Waits the time given in milliseconds for the output buffer to become not full.
   ///
   /// Returns true if the data can be sent, or false if not. 
   ///
   /// \pre The connection is alive, otherwise the connection-specific exception is thrown.
   ///
   bool WaitToSend(unsigned timeout = (unsigned)TimeoutInfinite) const;

   /// Clear the input buffer by reading all the available data from the socket, if any are there.
   ///
   /// \pre The socket is alive, other read-related errors possible.
   ///
   virtual void ClearInputBuffer() = 0;

   /// Whether the input buffer has any data.
   ///
   /// Returns true if there is no data in the input buffer, or false,
   /// if the data was received, and is ready to be read immediately.
   ///
   /// \pre The connection is alive, otherwise the connection-specific exception is thrown.
   ///
   /// \see WaitToReceive - Waits the specified time for the input data to arrive.
   ///
   bool IsInputBufferEmpty() const;

   /// The number of bytes in the receive buffer that can be read immediately.
   ///
   /// Returns the number of bytes ready to be read from socket.
   ///
   /// \pre The connection is alive, otherwise the connection-specific exception is thrown.
   ///
   /// \see WaitToReceive - Waits the specified time for the input data to arrive.
   ///
   virtual unsigned GetBytesReadyToRead() const = 0;

   /// Get socket option as integer, access to the standard socket call
   ///
   /// This call is low level, OS dependent, and should be used with care.
   ///
   /// \param level Defines the protocol level at which the option resides.
   ///    Few often used values are defined in \ref MSockOptEnum,
   ///    while the complete set is available in OS dependent documentation for getsockopt
   /// \param option Socket option.
   ///    Few often used values are defined in \ref MSockOptEnum,
   ///    while the complete set is available in OS dependent documentation for getsockopt
   ///
   /// \see MSockOptEnum - subset of possible values for level and option parameters
   /// \see GetSockOptBytes - get those socket options that are not representable as integer
   /// \see SetSockOpt - set socket option
   ///
   int GetSockOpt(int level, int option);

   /// Get socket option as byte string, access to the standard socket call
   ///
   /// This call is low level, OS dependent, and should be used with care.
   ///
   /// \param level Defines the protocol level at which the option resides.
   ///    Few often used values are defined in \ref MSockOptEnum,
   ///    while the complete set is available in OS dependent documentation for getsockopt
   /// \param option Socket option.
   ///    Few often used values are defined in \ref MSockOptEnum,
   ///    while the complete set is available in OS dependent documentation for getsockopt
   /// \param bufferSize Returned buffer size
   ///
   /// \see MSockOptEnum - subset of possible values for level and option parameters
   /// \see GetSockOpt - get integer socket options
   /// \see SetSockOpt - set socket option
   ///
   MByteString GetSockOptBytes(int level, int option, unsigned bufferSize);

   /// Set socket option, access to the standard socket call
   ///
   /// This call is low level, OS dependent, and should be used with care.
   ///
   /// \param level Defines the protocol level at which the option resides.
   ///    Few often used values are defined in \ref MSockOptEnum,
   ///    while the complete set is available in OS dependent documentation for getsockopt
   /// \param option Socket option.
   ///    Few often used values are defined in \ref MSockOptEnum,
   ///    while the complete set is available in OS dependent documentation for getsockopt
   /// \param value Integer, byte, or byte string, value of the socket option
   ///
   /// \see MSockOptEnum - subset of possible values for level and option parameters
   /// \see GetSockOpt - get integer socket options
   /// \see GetSockOptBytes - get those socket options that are not representable as integer
   ///
   void SetSockOpt(int level, int option, const MVariant& value);

public: // Static properties:

   /// Returns the standard host name for the local machine.
   ///
   /// \pre The socket library is properly initialized and the
   /// local name exists with sockets. Otherwise an exception is thrown.
   ///
   static MStdString GetLocalName();

   /// Returns the host address for the local machine.
   ///
   /// \pre The socket library is properly initialized and the
   /// local address exists. Otherwise an exception is thrown.
   ///
   static MStdString GetLocalAddress();

   /// Convert a string with IPv4 or IPv6 IP address into a binary form.
   ///
   /// \param addr
   ///     Either IPv4 or IPv6 address given in the string form.
   ///     The type of the address is inferred by presence of a semicolon, which is the case of IPv6.
   ///     The representation should be correct, otherwise an error is thrown.
   ///
   /// \return The result binary representation, either four or sixteen bytes,
   ///     depending on the address type given.
   ///
   static MByteString AddressToBinary(const MStdString& addr);

   /// Convert a binary form of IPv4 or IPv6 address into a string.
   ///
   /// \param addr
   ///     Either four or sixteen bytes for IPv4 or IPv6 address given as binary.
   ///     The type of the address is inferred from the size, so any other sizes are rejected as incorrect.
   ///
   /// \return The result string representation, either an IPv4 or IPv6.
   ///
   static MStdString BinaryToAddress(const MByteString& addr);

   /// True if the given string represents an IPv4 address.
   ///
   /// \param address String that represents an address.
   /// \return true if the given string syntactically is an IPv4, false otherwise.
   ///
   static bool IsAddressLocalIPv4(const MStdString& address);

   /// True if the given string represents an IPv6 address.
   ///
   /// \param address String that represents an address.
   /// \return true if the given string syntactically is an IPv6, false otherwise.
   ///
   static bool IsAddressLocalIPv6(const MStdString& address);

public:
/// \cond SHOW_INTERNAL

   /// Bind a server socket into a port, activate all interfaces.
   ///
   /// \param port
   ///    The port to bind to.
   ///
   void DoBind1(unsigned port);

protected: // Methods:

   virtual void DoCloseImpl();
   virtual bool DoIsOpenImpl() const M_NO_THROW;

   M_NORETURN_FUNC virtual void DoThrowEndOfStream();

   static SocketHandleType DoOsSocket(int domain, int type, int protocol);
   static int DoOsSelect(int nfds, fd_set* rfds, fd_set* wfds, fd_set* efds, unsigned ms);
   static int DoOsClose(SocketHandleType sockfd);
   static int DoOsIoctl(SocketHandleType sockfd, unsigned long cmd, unsigned long* argp);
   static void DoOsGetsockopt(SocketHandleType sockfd, int level, int optname, void* optval, socklen_t* optlen);
   static void DoOsSetsockopt(SocketHandleType sockfd, int level, int optname, const void* optval, socklen_t optlen);
   static int DoOsListen(SocketHandleType sockfd, int backlog, bool throwException = true);
   static void DoOsGethostname(char* name, unsigned namelen);
   static int DoOsGetpeername(SocketHandleType sockfd, struct sockaddr* addr, socklen_t* addrlen, bool throwException = true);
   static int DoOsGetsockname(SocketHandleType sockfd, struct sockaddr* addr, socklen_t* addrlen, bool throwException = true);
   static int DoOsGetnameinfo(const struct sockaddr* addr, socklen_t addrlen, char* host, socklen_t hostlen, char* serv, socklen_t servlen, int flags, bool throwException = true);
   static int DoOsGetaddrinfo(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** response);

   static void DoAdjustAddress(addrinfo* ai);

   static void DoSetNonBlocking(SocketHandleType sockfd, bool nonblock);
   static bool DoNonBlockingConnectionWait(SocketHandleType sockfd, unsigned ms);
   static bool DoNonblockingReceiveWait(SocketHandleType sockfd, unsigned ms);

   static M_NORETURN_FUNC void DoThrowBadIpAddress();

protected: // Properties:

   // Implementation-specific socket handle.
   //
   SocketHandleType m_socketHandle;

   // Socket receive operation timeout.
   //
   unsigned m_receiveTimeout;

   M_DECLARE_CLASS(StreamSocketBase)

/// \endcond SHOW_INTERNAL
};

#else

/// Set of utilities to handle IP address representation
///
class M_CLASS MStreamSocketBase : public MStream
{
public:
   /// Convert a string with IPv4 or IPv6 IP address into a binary form.
   ///
   /// \param addr
   ///     Either IPv4 or IPv6 address given in the string form.
   ///     The type of the address is inferred by presence of a semicolon, which is the case of IPv6.
   ///     The representation should be correct, otherwise an error is thrown.
   ///
   /// \return The result binary representation, either four or sixteen bytes,
   ///     depending on the address type given.
   ///
   static MByteString AddressToBinary(const MStdString& addr);

   /// Convert a binary form of IPv4 or IPv6 address into a string.
   ///
   /// \param addr
   ///     Either four or sixteen bytes for IPv4 or IPv6 address given as binary.
   ///     The type of the address is inferred from the size, so any other sizes are rejected as incorrect.
   ///
   /// \return The result string representation, either an IPv4 or IPv6.
   ///
   static MStdString BinaryToAddress(const MByteString& addr);

protected:
   static M_NORETURN_FUNC void DoThrowBadIpAddress();
};

#endif
///@}
#endif
