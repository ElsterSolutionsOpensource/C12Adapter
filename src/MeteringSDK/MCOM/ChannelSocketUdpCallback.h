#ifndef MCOM_CHANNELSOCKETUDPCALLBACK_H
#define MCOM_CHANNELSOCKETUDPCALLBACK_H
/// \addtogroup MCOM
///@{
/// \file MCOM/ChannelSocketUdpCallback.h

#include <MCOM/ChannelSocketUdp.h>

#if !M_NO_MCOM_CHANNEL_SOCKET_UDP

/// Socket callback channel is MChannelSocketUdp that has Auto Answer enabled by default.
///
/// This class is a convenient way of establishing socket servers,
/// however one has to remember this class can only handle one request at a time.
///
/// Different from MChannelSocketUdp, this class sets persistent property
/// \refprop{MChannelSocketUdp.SetAutoAnswer,MChannelSocketUdp.AutoAnswer} to true by default.
/// This property can be set to false by applications, in which case
/// the behavior of this class will be indistinguishable from its parent.
///
/// \see MChannelSocketUdp
///
class MCOM_CLASS MChannelSocketUdpCallback : public MChannelSocketUdp
{
public: // Channel specific methods:

   /// Create a channel with initial parameters.
   ///
   MChannelSocketUdpCallback();

   /// Destroy channel.
   ///
   /// If channel is connected then it will be disconnected.
   ///
   virtual ~MChannelSocketUdpCallback();

   M_DECLARE_CLASS(ChannelSocketUdpCallback)
};

#endif // !M_NO_MCOM_CHANNEL_SOCKET_UDP

///@}
#endif
