#ifndef MCOM_CHANNELOPTICALPROBE_H
#define MCOM_CHANNELOPTICALPROBE_H
/// \addtogroup MCOM
///@{
/// \file MCOM/ChannelOpticalProbe.h

#include <MCOM/MCOMDefs.h>
#include <MCOM/ChannelSerialPort.h>

#if !M_NO_SERIAL_PORT

/// Optical probe is a channel based on serial port that is
/// able to control the batteries of the probe (if it exists).
/// 
/// The optical probe channel exports all properties of its parent class, plus
/// the following properties through the property dispatch mechanism:
/// <ul>
/// <li> BATTERY_STATE, type bool, the current state of the batteries, if those are present. </li>
/// <li> BATTERY_CONTROL_DTR_HIGH, type bool, whether the DTR has to be high 
///      to turn the battery on. </li>
/// <li> BATTERY_CONTROL_RTS_HIGH, type bool, whether the DTR has to be high 
///      to turn the battery on. </li>
/// </ul>
/// More information on property handling and error treatment is given
/// within descriptions for property accessors and modifiers.
///
/// MChannelOpticalProbe inherits its properties from MChannelSerialPort. Several serial
/// control lines are re-assigned to operate probe features such as battery control and RTS
/// sense. For this channel type, Connect acquires the communication port from the Operating
/// System, sets IsConnected to True, and returns.  
///
class MCOM_CLASS MChannelOpticalProbe : public MChannelSerialPort
{
public: // Constructor and destructor:

   /// Object constructor.
   ///
   MChannelOpticalProbe();

   /// Object destructor.
   ///
   virtual ~MChannelOpticalProbe();

public: // Properties:

   ///@{
   /// The state of the probe's batteries.
   ///
   /// This dynamic non-persistent property is used to get or
   /// set the battery power state explicitly. If the optical probe does not use the batteries,
   /// this property is still handled to enable or disable the communication.
   ///
   /// This property is controlled by Connect and Disconnect services, when the optical probe
   /// communication is established or severed. It may be necessary to set it if the connection
   /// stays for a long time, but the communication is inactive.
   ///
   /// The logic sense of the battery control lines is set by BatteryControlDtrHigh and
   /// BatteryControlRtsHigh
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value False : The batteries are off.
   ///
   /// \possible_values
   ///  - True [1]  : Turn the battery On. If the battery had been previously turned off (which
   ///                sets DtrControl = D), turning the battery back on will restore the value of DtrControl to
   ///                what it is was prior to turning off the battery.
   ///  - False [0] : Turn the battery Off (DtrControl will be set to D)
   ///
   bool GetBatteryState() const;
   void SetBatteryState(bool power);
   ///@}

   ///@{
   /// Whether the DTR signal has to be high in order to switch the battery on.
   ///
   /// The Data Terminal Ready (DTR) line is used with the Request To Send (RTS) line to turn the
   /// optical probe battery power On / Off. Refer to the technical specifications for the probe
   /// being used to determine the correct state for each line. Typically, the DTR and RTS
   /// signals are mutually exclusive (one true, one false).
   ///
   /// BatteryControlDtrHigh must be set prior to a call to Connect because the optical probe
   /// batteries have to be turned on when the connection is established (this is just a
   /// convention if there are no batteries). This property overrides the DtrControl property.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value False : As required by the standard ABB/Elster probe.
   ///
   /// \possible_values
   ///  - True [1]  : DTR has to be high to turn the battery on.
   ///  - False [0] : DTR has to be low to turn the battery on.
   ///
   bool GetBatteryControlDtrHigh() const
   {
      return m_batteryControlDtrHigh;
   }
   void SetBatteryControlDtrHigh(bool yes)
   {
      m_batteryControlDtrHigh = yes;
   }
   ///@}

   ///@{
   /// Whether the RTS signal has to be high in order to switch the battery on.
   ///
   /// The Request To Send (RTS) line is used with the Data Terminal Ready (DTR) line to turn the
   /// optical probe battery power On / Off. Refer to the technical specifications for the probe
   /// being used to determine the correct state for each line. Typically, the RTS and DTR
   /// signals are mutually exclusive (one true, one false).
   ///
   /// BatteryControlRtsHigh must be set prior to a call to Connect because the optical probe
   /// batteries have to be turned on when the connection is established (this is just a
   /// convention if there are no batteries).  This property overrides the RtsControl property.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value True : As required by the standard ABB/Elster probe.
   ///
   /// \possible_values
   ///  - True [1]  : RTS has to be high to turn the battery on.
   ///  - False [0] : RTS has to be low to turn the battery on.
   ///
   bool GetBatteryControlRtsHigh() const
   {
      return m_batteryControlRtsHigh;
   }
   void SetBatteryControlRtsHigh(bool yes)
   {
      m_batteryControlRtsHigh = yes;
   }
   ///@}

public: // Methods:

   /// Establishes the connection to the meter using the optical probe.
   /// In addition to serial port connection, this particular
   /// service turns on probe batteries (if they are present).
   ///
   /// \pre IsConnected should be false before calling this method.
   /// Many OS and program exceptions can be thrown by this method.
   ///
   virtual void Connect();

   /// Disconnect brings down the connection and releases the serial port resource.
   /// In additional to the parent implementation, the optical probe switches the
   /// batteries off.
   ///
   virtual void Disconnect();

protected: // Attributes:
/// \cond SHOW_INTERNAL

   // Current battery state, true means active
   //
   bool m_batteryState;

   // True if to switch the batteries on DTR has to be turned high
   //
   bool m_batteryControlDtrHigh;

   // True if to switch the batteries on RTS has to be turned high
   //
   bool m_batteryControlRtsHigh;

/// \endcond SHOW_INTERNAL

   M_DECLARE_CLASS(ChannelOpticalProbe)
};

#endif // !M_NO_SERIAL_PORT

///@}
#endif
