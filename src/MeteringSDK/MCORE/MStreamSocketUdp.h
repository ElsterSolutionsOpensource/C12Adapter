#ifndef MCORE_MSTREAMSOCKETUDP_H
#define MCORE_MSTREAMSOCKETUDP_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MStreamSocketUdp.h

#include <MCORE/MStreamSocketBase.h>

#if !M_NO_SOCKETS_UDP

/// UDP datagram socket.
///
/// The services below can throw MESocketError in the event of an erroneous socket operation.
///
class M_CLASS MStreamSocketUdp : public MStreamSocketBase
{
public: // Types and constants:

   enum
   {
      /// Maximum practical size of UDP datagram.
      ///
      /// The theoretical UDP datagram size is 65,535 according to
      /// https://en.wikipedia.org/wiki/User_Datagram_Protocol.
      /// However, such size is not practical as UDP size is typically selected to be smaller than MTU
      /// (maximum transfed unit of the media). According to
      /// http://stackoverflow.com/questions/1098897/what-is-the-largest-safe-udp-packet-size-on-the-internet
      /// the value 1500 is a good practical maximum, while in reality the size is even smaller to
      /// make sure the packet is never reassembled.
      ///
      MAXIMUM_DATAGRAM_SIZE = 1500
   };

public: // Constructor, destructor:

   /// Constructor that creates socket based on existing socket handle.
   ///
   /// This is a C++ only method that is convenient for cases when the socket is open by any means
   /// other than the socket stream object.
   ///
   /// \param socketHandle
   ///    Handle to use by the socket.
   ///    When handle is an invalid socket value, or not present, the socket will not be opened.
   ///
   explicit MStreamSocketUdp(SocketHandleType socketHandle = InvalidSocket);

   /// Destructor, destroys the socket object and never throws an exception.
   ///
   /// Use Close if there is a need to report any socket related exceptions.
   ///
   virtual ~MStreamSocketUdp() M_NO_THROW;

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

public: // Methods:

   /// Create client socket that connects to the server.
   ///
   /// \param port
   ///    Port to which to connect.
   ///
   /// \param address
   ///    IP address or a DNS name of the server to which the connection has to be made.
   ///
   /// \see MStreamSocket::ConnectWithProxy - Connect to a socket through SOCKS proxy.
   ///
   void Connect(unsigned port, const MStdString& address);

   /// Clear the input buffer by reading all the available data from the socket, if any are there.
   ///
   /// \pre The socket is alive, other read-related errors possible.
   ///
   virtual void ClearInputBuffer();

   /// The number of bytes in the receive buffer that can be read immediately.
   ///
   /// Returns the number of bytes ready to be read from socket.
   ///
   /// \pre The connection is alive, otherwise the connection-specific exception is thrown.
   ///
   /// \see MStreamSocketBase::WaitToReceive - Waits the specified time for the input data to arrive.
   ///
   virtual unsigned GetBytesReadyToRead() const;

   /// Analog of the standard socket datagram function recvfrom.
   ///
   /// Receive a datagram currently available in the socket.
   ///
   /// \param buffer
   ///     Buffer into which to receive the data.
   /// \param length
   ///     Length of the buffer or how many bytes to read.
   /// \param flags
   ///     Standard recv flags.
   /// \param addr
   ///     Address information, big enough to hold any address. This is typically sockaddr_storage.
   /// \param addrLength
   ///     Returned length of the address information.
   /// \return
   ///     How many bytes are received into buffer.
   ///
   /// \pre Socket is bound or connected.
   ///
   /// \see Recv - Variant of this call that uses an internal holder for address.
   ///
   unsigned RecvFrom(char* buffer, unsigned length, int flags, sockaddr* addr, socklen_t* addrLength);
   
   /// Send the buffer as the socket datagram, standard socket function sendto.
   ///
   /// \param buffer
   ///     Buffer, which shall be written to socket.
   /// \param length
   ///     Length of the buffer to write.
   /// \param flags
   ///     Standard send flags.
   /// \param addr
   ///     Address information, big enough to hold any address. This is typically sockaddr_storage.
   /// \param addrLength
   ///     Returned length of the address information.
   /// \return
   ///     How many bytes are sent.
   ///
   /// \pre Socket is bound or connected.
   ///
   unsigned SendTo(const char* buffer, unsigned length, int flags, const sockaddr* addr, socklen_t addrLength);

   /// Analog of the standard socket function recv, uses internal address.
   ///
   /// \param buffer
   ///     Buffer into which to receive the data.
   /// \param length
   ///     Length of the buffer or how many bytes to read.
   /// \param flags
   ///     Standard recv flags.
   /// \return
   ///     How many bytes are received into buffer.
   ///
   /// \pre Socket is bound or connected.
   ///
   /// \see RecvFrom - Complete UDP version of this call.
   ///
   unsigned Recv(char* buffer, unsigned length, int flags);

   /// Send the buffer through the socket datagram, uses the internal address.
   ///
   /// \param buffer
   ///     Buffer, which shall be written to socket.
   /// \param length
   ///     Length of the buffer to write.
   /// \param flags
   ///     Standard send flags.
   /// \return
   ///     How many bytes are sent.
   ///
   /// \pre Socket is bound or connected.
   ///
   /// \see SendTo - Complete UDP version of this call.
   ///
   unsigned Send(const char* buffer, unsigned length, int flags);

   /// Swap this UDP socket and the given socket by exchanging their handles and other properties.
   ///
   /// After the successful completion, this socket and other socket will be exchanged.
   ///
   void Swap(MStreamSocketUdp& other);

protected: // Methods:
/// \cond SHOW_INTERNAL

   virtual unsigned DoReadAllAvailableBytesImpl(char* buf, unsigned len);
   virtual unsigned DoReadAvailableBytesImpl   (char* buf, unsigned len);
   virtual void     DoWriteBytesImpl           (const char* buf, unsigned len);

/// \endcond SHOW_INTERNAL
private:

   // Peer address holder
   //
   sockaddr_storage m_peerAddr;

   // Length of peer address holder
   //
   socklen_t m_peerAddrLength;

   // Size of the input buffer, as read
   //
   unsigned m_inputBufferSize;

   // Current position within the input buffer
   //
   unsigned m_inputBufferCurr;

   // Buffer for the incoming datagram
   //
   char m_inputBuffer [ MAXIMUM_DATAGRAM_SIZE ];

   M_DECLARE_CLASS(StreamSocketUdp)
};

#endif
///@}
#endif
