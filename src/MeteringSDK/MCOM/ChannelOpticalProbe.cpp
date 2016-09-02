// File MCOM/ChannelOpticalProbe.cpp
#include "MCOMExtern.h"
#include "ChannelOpticalProbe.h"
#include "MCOMExceptions.h"

#if !M_NO_SERIAL_PORT

M_START_PROPERTIES(ChannelOpticalProbe)
   M_OBJECT_PROPERTY_BOOL           (ChannelOpticalProbe, BatteryState)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL(ChannelOpticalProbe, BatteryControlDtrHigh, false)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL(ChannelOpticalProbe, BatteryControlRtsHigh, true)
// Default values overwritten for the following properties:
   M_OBJECT_PROPERTY_PERSISTENT_CHAR(ChannelOpticalProbe, RtsControl,            'D') // DOXYGEN_HIDE SWIG_HIDE
M_START_METHODS(ChannelOpticalProbe)
M_END_CLASS_TYPED(ChannelOpticalProbe, ChannelSerialPort, "CHANNEL_OPTICAL_PROBE")

MChannelOpticalProbe::MChannelOpticalProbe()
:
   MChannelSerialPort(),
   m_batteryState(false)
{
   M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(MChannelOpticalProbe);
}

MChannelOpticalProbe::~MChannelOpticalProbe()
{
   Disconnect(); // Ensure the proper Disconnect service is called...
}

bool MChannelOpticalProbe::GetBatteryState() const
{
   return m_batteryState;
}

void MChannelOpticalProbe::SetBatteryState(bool power)
{
   if ( !power ) // if we switch the battery off, make sure everything went out
      FlushOutputBuffer();
   SetRtsControl((m_batteryControlRtsHigh ^ power) ? 'D' : 'E');
   SetDtrControl((m_batteryControlDtrHigh ^ power) ? 'D' : 'E');
   m_batteryState = power; // do assignment AFTER the successful previous calls
   if ( power ) // if we switch the battery on, let the batteries warm up
      MUtilities::Sleep(50); 
}

void MChannelOpticalProbe::Connect()
{
   SetBatteryState(true); // notify on RTS and DTR, will be actually set during Connect
   MChannelSerialPort::Connect();
}

void MChannelOpticalProbe::Disconnect()
{
   m_batteryState = false; // No need to physically set the battery state here
   MChannelSerialPort::Disconnect();
}

#endif // !M_NO_SERIAL_PORT
