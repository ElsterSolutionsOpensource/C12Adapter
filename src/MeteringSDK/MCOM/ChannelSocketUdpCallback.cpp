// File MCOM/ChannelSocketUdpCallback.cpp

#include "MCOMExtern.h"
#include "ChannelSocketUdpCallback.h"
#include "MCOMExceptions.h"

#if !M_NO_MCOM_CHANNEL_SOCKET_UDP

M_START_PROPERTIES(ChannelSocketUdpCallback)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL(ChannelSocketUdpCallback, AutoAnswer, true)  // DOXYGEN_HIDE SWIG_HIDE
M_START_METHODS(ChannelSocketUdpCallback)
M_END_CLASS_TYPED(ChannelSocketUdpCallback, ChannelSocketUdp, "CHANNEL_SOCKET_UDP_CALLBACK")

MChannelSocketUdpCallback::MChannelSocketUdpCallback()
:
   MChannelSocketUdp()
{
   M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(ChannelSocketUdpCallback);
}

MChannelSocketUdpCallback::~MChannelSocketUdpCallback()
{
}

#endif // !M_NO_MCOM_CHANNEL_SOCKET_UDP
