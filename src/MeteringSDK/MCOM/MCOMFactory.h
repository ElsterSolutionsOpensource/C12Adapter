#ifndef MCOM_MCOMFACTORY_H
#define MCOM_MCOMFACTORY_H
/// \addtogroup MCOM
///@{
/// \file MCOM/MCOMFactory.h

#include <MCOM/MCOMDefs.h>
#include <MCOM/Channel.h>
#include <MCOM/Protocol.h>

#if !M_NO_MCOM_FACTORY // Note that factory is not available without reflection

/// Factory that is capable of creating MCOM objects.
/// This is a singleton class. No instances are required,
/// the services are available through static reference syntax.
///
class MCOM_CLASS MCOMFactory : public MObject
{
   friend class MCOM_CLASS MCOMObject;

public: // Services:

   /// Create a new channel with the channel source given as a parameter.
   /// 
   /// \param channelSource
   /// \parblock 
   ///   The channel name and if desired, additional property values, separated by semi-colons.
   ///   
   ///   The string can be in the form "PROPERTY1=VALUE1;PROPERTY2=VALUE2;..."
   ///   where the TYPE=channelname property is required and the channelname is 
   ///   a known channel name, such as CHANNEL_OPTICAL_PROBE.
   ///   The list of possible channel names can be gotten with \refprop{GetAllAvailableChannels,AllAvailableChannels}.
   ///   The properties that can be supplied depend on the particular channel and
   ///   their values must be correct. If a property is mentioned more than once, the one 
   ///   which is mentioned last is in effect. For properties that are not given, the channel 
   ///   is created with their default values. Here is an example:
   ///   \code
   ///         // MChannelOpticalProbe is created with all its properties at their default value
   ///         channel = MCOMFactory::CreateChannel("TYPE=CHANNEL_OPTICAL_PROBE");
   ///
   ///         // MChannelOpticalProbe is created with PORT_NAME initialized as COM4, and
   ///         // all other properties at their default value.
   ///         channel = MCOMFactory::CreateChannel("TYPE=CHANNEL_OPTICAL_PROBE;PORT_NAME=COM4");
   ///   \endcode
   ///
   ///   Property values that are of type string can be optionally surrounded by
   ///   double quotes, so both PORT_NAME="COM2" and PORT_NAME=COM2 work. 
   ///   Not requiring the double quotes is convenient as to include them usually
   ///   requires some special handling, like using escape sequences. For example, 
   ///   C++ code looks like: 
   ///   \code
   ///    PORT_NAME=\"COM2\" 
   ///   \endcode
   ///   There are rare cases where the double quotes are part of the property value, 
   ///   such as a Bluetooth probe we encountered that actually called itself 
   ///   "BluetoothProbe", instead of BluetoothProbe. This results in a string 
   ///   that has a quoted string, that has a quoted string. You can't just have 
   ///   \code 
   ///    PORT_NAME=\"BluetoothProbe\" 
   ///   \endcode
   ///   because MeteringSDK will take the name as BluetoothProbe. You can't pass it as 
   ///   \code
   ///    PORT_NAME=\"\"BluetoothProbe\"\" 
   ///   \endcode
   ///   either because MeteringSDK assumes that the 2nd escaped quote (the one before the B)
   ///   is the end of the value (it thinks that PORT_NAME = ""). What you have to do is 
   ///   escape the backslashes used to escape the quotes that are part of the probe name, like
   ///   \code
   ///    PORT_NAME=\"\\\"BluetoothProbe\\\"\"
   ///   \endcode
   /// \endparblock
   ///
   /// \return MChannel. Note that null object is never returned.
   ///
   /// \if CPP
   /// \see \ref CreateChannelByName creates channel from known name, such as "CHANNEL_OPTICAL_PROBE".
   /// \endif
   /// \see \refprop{GetAllAvailableChannels,AllAvailableChannels} returns the list of available 
   ///       channel names, such as "CHANNEL_OPTICAL_PROBE".
   ///
   static MChannel* CreateChannel(const MStdString& channelSource);

   /// Create protocol from channel object or source string, and the protocol source string.
   ///
   /// \param channelObjectOrSource
   /// \parblock
   ///
   ///   An empty variant (no channel), the channel object, or a string that describes the 
   ///   channel.    
   ///  
   ///   An empty variant or an object with null value, can be given, in this case 
   ///   the channel associated with the protocol is a null object.
   ///
   ///   A channel object, such as MChannelOpticalProbe, can be given, and it will be
   ///   assigned to and owned by the protocol. The channel can be given while in any 
   ///   state - it can be connected or not, etc.
   ///
   ///   A string that represents the channel type and its property values. 
   ///   \ref CreateChannel describes in detail what the string can be.
   ///   The result channel is owned by the protocol.
   /// \endparblock
   ///
   /// \param protocolSource
   /// \parblock 
   ///
   ///   The protocol name and if desired, additional property values, separated by semi-colons.
   ///   
   ///   The string can be in the form "PROPERTY1=VALUE1;PROPERTY2=VALUE2;..."
   ///   where the TYPE=protocolname property is required and the protocolname is 
   ///   a known protocol name, such as PROTOCOL_ANSI_C12_18.
   ///   The list of possible protocol names can be gotten with \refprop{GetAllAvailableProtocols,AllAvailableProtocols}.
   ///   The properties that can be supplied depend on the particular protocol and
   ///   their values must be correct. If a property is mentioned more than once, the one 
   ///   which is mentioned last is in effect. For properties that are not given, the protocol 
   ///   is created with their default values. Here is an example:
   ///   \code
   ///     // MProtocolC1218 is created with all its properties at their default value.
   ///     protocol = MCOMFactory::CreateProtocol(channel, "TYPE=PROTOCOL_ANSI_C12_18");
   ///
   ///     // MProtocolC1218 is created with SESSION_BAUD initialized to 28800, and
   ///     // all other properties at their default value.
   ///     protocol = MCOMFactory::CreateProtocol(channel,
   ///                                 "TYPE=PROTOCOL_ANSI_C12_18;SESSION_BAUD=28800");
   ///   \endcode
   ///
   /// \endparblock
   ///
   /// \return MProtocol. Note that null object is never returned.
   ///
   /// \see \ref CreateProtocolWithoutChannel creates protocol with NULL channel.
   /// \if CPP
   /// \see \ref CreateProtocolByName creates protocol from known name, such as "PROTOCOL_ANSI_C12_18".
   /// \endif
   /// \see \refprop{GetAllAvailableChannels,AllAvailableChannels} returns the list of available 
   ///       channel names, such as "CHANNEL_OPTICAL_PROBE".
   /// \see \refprop{GetAllAvailableProtocols,AllAvailableProtocols} returns the list of available 
   ///       protocol names, such as "PROTOCOL_ANSI_C12_18".
   ///
   static MProtocol* CreateProtocol(const MVariant& channelObjectOrSource, const MStdString& protocolSource);

   /// Create protocol with null channel from the protocol source string.
   ///
   /// \param protocolSource
   /// \parblock 
   ///
   ///   The protocol name and if desired, additional property values, separated by semi-colons.
   ///   
   ///   The string can be in the form "PROPERTY1=VALUE1;PROPERTY2=VALUE2;..."
   ///   where the TYPE=protocolname property is required and the protocolname is 
   ///   a known protocol name, such as PROTOCOL_ANSI_C12_18.
   ///   The list of possible protocol names can be gotten with \refprop{GetAllAvailableProtocols,AllAvailableProtocols}.
   ///   The properties that can be supplied depend on the particular protocol and
   ///   their values must be correct. If a property is mentioned more than once, the one 
   ///   which is mentioned last is in effect. For properties that are not given, the protocol 
   ///   is created with their default values. Here is an example:
   ///   \code
   ///     // MProtocolC1218 is created with all its properties at their default value.
   ///     protocol = MCOMFactory::CreateProtocolWithoutChannel("TYPE=PROTOCOL_ANSI_C12_18");
   ///
   ///     // MProtocolC1218 is created with SESSION_BAUD initialized to 28800, and
   ///     // all other properties at their default value.
   ///     protocol = MCOMFactory::CreateProtocolWithoutChannel("TYPE=PROTOCOL_ANSI_C12_18;SESSION_BAUD=28800");
   ///   \endcode
   ///
   /// \endparblock
   ///
   /// \return MProtocol. Note that null object is never returned.
   ///
   /// \see \ref CreateProtocol creates protocol object with a channel.
   /// \if CPP
   /// \see \ref CreateProtocolByName creates protocol from known name, such as "PROTOCOL_ANSI_C12_18".
   /// \endif
   /// \see \refprop{GetAllAvailableProtocols,AllAvailableProtocols} returns the list of available 
   ///       protocol names, such as "PROTOCOL_ANSI_C12_18".
   ///
   static MProtocol* CreateProtocolWithoutChannel(const MStdString& protocolSource);

   /// Create the protocol by the known name, all known names start with "PROTOCOL_". 
   /// The list of possible protocol names can be gotten with \refprop{GetAllAvailableProtocols,AllAvailableProtocols}.
   ///
   /// \param channel The channel to be assigned to the resulting protocol object, can be null.
   ///
   /// \param protocolName The known protocol name, such as "PROTOCOL_ANSI_C12_18".
   ///     The protocol with such name should exist in the Factory,
   ///     otherwise an exception is thrown.
   ///
   /// \return MProtocol. Note that null object is never returned.
   ///
   /// \see \ref CreateProtocol creates protocol object with a channel.
   /// \see \ref CreateProtocolWithoutChannel creates protocol with NULL channel.
   /// \see \refprop{GetAllAvailableProtocols,AllAvailableProtocols} returns the list of available 
   ///       protocol names, such as "PROTOCOL_ANSI_C12_18".
   ///
   static MProtocol* CreateProtocolByName(MChannel* channel, const MStdString& protocolName);

   /// Create the channel by the known name, all known names start with "CHANNEL_".
   /// The list of possible channel names can be gotten with \refprop{GetAllAvailableChannels,AllAvailableChannels}.
   ///
   /// \param channelName The known channel name, such as "CHANNEL_OPTICAL_PROBE".
   ///     The channel with such name should exist in the Factory,
   ///     otherwise an exception is thrown.
   ///
   /// \return MChannel. Note that null object is never returned.
   ///
   /// \see \ref CreateChannel creates channel from source string.
   /// \see \refprop{GetAllAvailableChannels,AllAvailableChannels} returns the list of available 
   ///       channel names, such as "CHANNEL_OPTICAL_PROBE".
   ///
   static MChannel* CreateChannelByName(const MStdString& channelName);

   /// Get the channel names available for creation.
   /// All channel names start with "CHANNEL_".
   ///
   static MStdStringVector GetAllAvailableChannels();

   /// Get the protocol names available for creation.
   /// All protocol names start with "PROTOCOL_".
   ///
   static MStdStringVector GetAllAvailableProtocols();


#if !M_NO_MCOM_IDENTIFY_METER
   /// Get identify strings, that are in the given, possibly complex identify string.
   ///
   /// \pre The given identify string is a valid one, possibly consisting of multiple
   /// individual identify strings. Otherwise the behavior is undefined.
   ///
   static MStdStringVector GetIdentifyStrings(const MStdString& complexIdentify);
#endif // !M_NO_MCOM_IDENTIFY_METER

private: // Constructor and destructor:

   // Private class constructors and destructor.
   // No instances are possible, the class is static.
   //
   MCOMFactory();
   MCOMFactory(const MCOMFactory&);
   virtual ~MCOMFactory();

   static MProtocol* DoCreateProtocol(MChannel* channel, const MStdString& protocolSource);

#if !M_NO_MCOM_MONITOR
public:
   /// \cond SHOW_INTERNAL
   static bool s_createDefaultMonitor; // very hidden functionality to a single user
   /// \endcond SHOW_INTERNAL
#endif

private:

   M_DECLARE_CLASS(COMFactory)
};

#endif // !M_NO_MCOM_FACTORY

///@}
#endif
