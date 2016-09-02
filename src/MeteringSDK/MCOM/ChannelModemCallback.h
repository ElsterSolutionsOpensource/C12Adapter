#ifndef MCOM_CHANNELMODEMCALLBACK_H
#define MCOM_CHANNELMODEMCALLBACK_H
/// \addtogroup MCOM
///@{
/// \file MCOM/ChannelModemCallback.h

#include <MCOM/ChannelModem.h>

#if !M_NO_MCOM_CHANNEL_MODEM

/// Modem callback channel is MChannelModem that has Auto Answer enabled by default.
///
/// This class is a convenient way of establishing modem callback stations (servers).
/// Different from MChannelModem, this class sets persistent property
/// \refprop{MChannelModem.SetAutoAnswer,MChannelModem.AutoAnswer} to true by default.
/// This property can be set to false by applications, in which case
/// the behavior of this class will be indistinguishable from its parent.
///
/// \see MChannelModem
///
class MCOM_CLASS MChannelModemCallback : public MChannelModem
{
public: // Constructor, destructor:

   /// Create a callback modem channel with initial parameters.
   ///
   MChannelModemCallback();

   /// Destroy channel.
   ///
   /// If channel is connected then it will be disconnected within this call.
   ///
   virtual ~MChannelModemCallback();

   M_DECLARE_CLASS(ChannelModemCallback)
};

#endif // !M_NO_MCOM_CHANNEL_MODEM

///@}
#endif
