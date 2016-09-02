// File MCOM/ChannelModemCallback.cpp

#include "MCOMExtern.h"
#include "ChannelModemCallback.h"
#include "MCOMExceptions.h"

#if !M_NO_MCOM_CHANNEL_MODEM

M_START_PROPERTIES(ChannelModemCallback)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL(ChannelModemCallback, AutoAnswer, true) // DOXYGEN_HIDE SWIG_HIDE
M_START_METHODS(ChannelModemCallback)
M_END_CLASS_TYPED(ChannelModemCallback, ChannelModem, "CHANNEL_MODEM_CALLBACK")

MChannelModemCallback::MChannelModemCallback()
:
   MChannelModem()
{
   M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(ChannelModemCallback);
}

MChannelModemCallback::~MChannelModemCallback()
{
}

#endif // !M_NO_MCOM_CHANNEL_MODEM
