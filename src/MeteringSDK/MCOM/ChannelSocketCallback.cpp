// File MCOM/ChannelSocketCallback.cpp

#include "MCOMExtern.h"
#include "ChannelSocketCallback.h"
#include "MCOMExceptions.h"

#if !M_NO_MCOM_CHANNEL_SOCKET

M_START_PROPERTIES(ChannelSocketCallback)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL(ChannelSocketCallback, AutoAnswer, true)  // DOXYGEN_HIDE SWIG_HIDE
M_START_METHODS(ChannelSocketCallback)
M_END_CLASS_TYPED(ChannelSocketCallback, ChannelSocket, "CHANNEL_SOCKET_CALLBACK")

MChannelSocketCallback::MChannelSocketCallback()
:
   MChannelSocket()
{
   M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(ChannelSocketCallback);
}

MChannelSocketCallback::~MChannelSocketCallback()
{
}

#endif // !M_NO_MCOM_CHANNEL_SOCKET
