// File MCOM/ChannelCurrentLoop.cpp

#include "MCOMExtern.h"
#include "ChannelCurrentLoop.h"
#include "MCOMExceptions.h"

#if !M_NO_SERIAL_PORT

using namespace std;

M_START_PROPERTIES(ChannelCurrentLoop)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL(ChannelCurrentLoop, Echo, true) // DOXYGEN_HIDE SWIG_HIDE
M_START_METHODS(ChannelCurrentLoop)
M_END_CLASS_TYPED(ChannelCurrentLoop, ChannelSerialPort, "CHANNEL_CURRENT_LOOP")

MChannelCurrentLoop::MChannelCurrentLoop()
{
   M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(ChannelCurrentLoop);
}

MChannelCurrentLoop::~MChannelCurrentLoop()
{
}

#endif // !M_NO_SERIAL_PORT

