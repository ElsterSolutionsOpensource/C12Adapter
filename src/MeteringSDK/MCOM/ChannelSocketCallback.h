#ifndef MCOM_CHANNELSOCKETCALLBACK_H
#define MCOM_CHANNELSOCKETCALLBACK_H
/// \addtogroup MCOM
///@{
/// \file MCOM/ChannelSocketCallback.h

#include <MCOM/ChannelSocket.h>

#if !M_NO_MCOM_CHANNEL_SOCKET

/// Socket callback channel is MChannelSocket that has Auto Answer enabled by default.
///
/// This class is a convenient way of establishing socket servers,
/// however one has to remember this class can only handle one request at a time.
///
/// Different from MChannelSocket, this class sets persistent property
/// \refprop{MChannelSocket.SetAutoAnswer,MChannelSocket.AutoAnswer} to true by default.
/// This property can be set to false by applications, in which case
/// the behavior of this class will be indistinguishable from its parent.
///
/// \see MChannelSocket
///
class MCOM_CLASS MChannelSocketCallback : public MChannelSocket
{
public: // Channel specific methods:

   /// Create a channel with initial parameters.
   ///
   MChannelSocketCallback();

   /// Destroy channel modem.
   ///
   /// \pre If channel is connected then it will be disconnected.
   ///
   virtual ~MChannelSocketCallback();

   M_DECLARE_CLASS(ChannelSocketCallback)
};

#endif // !M_NO_MCOM_CHANNEL_SOCKET

///@}
#endif
