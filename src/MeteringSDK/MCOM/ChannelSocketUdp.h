#ifndef MCOM_CHANNELSOCKETUDP_H
#define MCOM_CHANNELSOCKETUDP_H
/// \addtogroup MCOM
///@{
/// \file MCOM/ChannelSocketUdp.h

#include <MCOM/ChannelSocketBase.h>

#if !M_NO_MCOM_CHANNEL_SOCKET_UDP

/// Implements the channel interface for an UDP datagram socket.
///
class MCOM_CLASS MChannelSocketUdp : public MChannelSocketBase
{
public: // Types:

   enum
   {
      /// Maximum practical size of UDP datagram.
      ///
      /// The theoretical UDP datagram size is 65,535 according to
      /// https://en.wikipedia.org/wiki/User_Datagram_Protocol.
      /// However, such size is not practical as UDP size is typically selected to be smaller than MTU
      /// (Maximum Transmission Unit of the media). According to
      /// http://stackoverflow.com/questions/1098897/what-is-the-largest-safe-udp-packet-size-on-the-internet
      /// the value 1500 is a good practical maximum, while in reality the size is even smaller to
      /// make sure the packet is never reassembled.
      ///
      MaximumUdpDatagramSize = 1500
   };

public: // Constructor, destructor:

   /// Construct the socket channel.
   ///
   MChannelSocketUdp();

   /// Destructor.
   ///
   virtual ~MChannelSocketUdp();

public: // Services:

   /// Establishes connection to the meter using a socket.
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
   /// \param reinitialize When true, the channel is reinitialized at each new incoming connection.
   ///
   /// \pre Prior to this call, the channel needs to be configured with \refprop{SetAutoAnswer,AutoAnswer} true, 
   ///      and the connection established with Connect(). Not all channels support Auto Answer mode, 
   ///      and might throw an exception. A timeout exception is thrown if no call is received 
   ///      during the \refprop{GetAutoAnswerTimeout,AutoAnswerTimeout} period.
   ///
   virtual void WaitForNextIncomingConnection(bool reinitialize = true);

   /// Send the whole datagram given as data pointer and size into the UDP socket.
   ///
   /// While the standard Channel data writing methods work just fine with UDP,
   /// this UDP specific method guarantees calling a single sendto method of socket interface.
   ///
   /// The address and port where the data will be sent is determined by
   /// \refprop{GetPeerAddress,PeerAddress} and \refprop{GetPeerPort,PeerPort}.
   ///
   /// If the datagram was not sent for the duration of \refprop{GetWriteTimeout,WriteTimeout},
   /// an exception is thrown. Any other socket related exception is also possible.
   /// Just as any channel sending call, the operation is cancellable,
   /// in which case MEOperationCancelled is thrown.
   ///
   /// \param buff Constant buffer pointer of the datagram.
   /// \param size Size of the buffer.
   ///
   void WriteDatagramBuffer(const char* buff, unsigned size);

   /// Send the whole datagram to the UDP socket.
   ///
   /// While the standard Channel data writing methods work just fine with UDP,
   /// this UDP specific method guarantees calling a single sendto method of socket interface.
   ///
   /// The address and port where the data will be sent is determined by
   /// \refprop{GetPeerAddress,PeerAddress} and \refprop{GetPeerPort,PeerPort}.
   ///
   /// If the datagram was not sent for the duration of \refprop{GetWriteTimeout,WriteTimeout},
   /// an exception is thrown. Any other socket related exception is also possible.
   /// Just as any channel sending call, the operation is cancellable,
   /// in which case MEOperationCancelled is thrown.
   ///
   /// \param bytes Datagram buffer
   ///
   void WriteDatagram(const MByteString& bytes);

   /// Receive the datagram given as data pointer and size.
   ///
   /// While the standard Channel data reading methods work just fine with UDP,
   /// this UDP specific method guarantees calling a single recvfrom method of socket interface.
   /// Because the method bypasses certain logic of the base channel class, unread bytes
   /// are not seen when ReceiveDatagram is called.
   ///
   /// After receiving the buffer, the address and port from where the data were got are available as
   /// \refprop{GetActualPeerAddress,ActualPeerAddress} and \refprop{GetActualPeerPort,ActualPeerPort}.
   ///
   /// If the datagram was not received for the duration of \refprop{GetReadTimeout,ReadTimeout},
   /// an exception is thrown. Any other socket related exception is also possible.
   /// Just as any channel sending call, the operation is cancellable,
   /// in which case MEOperationCancelled is thrown.
   ///
   /// \param buff Write only buffer pointer of the datagram.
   /// \param size Size of the buffer.
   ///
   /// \return Number of bytes filled in the buffer.
   ///
   unsigned ReadDatagramBuffer(char* buff, unsigned size);

   /// Receive the datagram.
   ///
   /// While the standard Channel data reading methods work just fine with UDP,
   /// this UDP specific method guarantees calling a single recvfrom method of socket interface.
   /// Because the method bypasses certain logic of the base channel class, unread bytes
   /// are not seen when ReceiveDatagram is called.
   ///
   /// After receiving the buffer, the address and port from where the data were got are available as
   /// \refprop{GetActualPeerAddress,ActualPeerAddress} and \refprop{GetActualPeerPort,ActualPeerPort}.
   ///
   /// If the datagram was not received for the duration of \refprop{GetReadTimeout,ReadTimeout},
   /// an exception is thrown. Any other socket related exception is also possible.
   /// Just as any channel sending call, the operation is cancellable,
   /// in which case MEOperationCancelled is thrown.
   ///
   /// \return Datagram bytes received.
   ///
   MByteString ReadDatagram();

public: // Property handling routines:

   ///@{
   /// Socket object that is used by this channel.
   ///
   /// Setting UDP socket object has a feature: what actually happens, this channel's socket is swapped with the given UDP socket.
   /// MChannelSocketUdp accepts only MStreamSocketUdp object as input.
   ///
   MStreamSocketUdp& GetSocket()
   {
      M_ASSERT(m_socketPtr == &m_socket);
      return m_socket;
   }
   virtual void SetSocket(MStreamSocketBase& other);
   ///@}

   /// Get the constant socket object that is used by this channel.
   ///
   const MStreamSocketUdp& GetSocketConst() const
   {
      M_ASSERT(m_socketPtr == (MStreamSocketBase*)&m_socket);
      return m_socket;
   }

private: // Attributes:

   // Socket object.
   //
   MStreamSocketUdp m_socket;

   M_DECLARE_CLASS(ChannelSocketUdp)
};

#endif // !M_NO_MCOM_CHANNEL_SOCKET_UDP

///@}
#endif
