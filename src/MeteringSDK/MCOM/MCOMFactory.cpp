// File MCOM/MCOMFactory.cpp

#include "MCOMExtern.h"
#include "MCOMFactory.h"
#include "MCOM.h"
#include "MonitorSocket.h"
#include <MCORE/MDictionary.h>

#if !M_NO_MCOM_FACTORY

M_START_PROPERTIES(COMFactory)
   M_CLASS_PROPERTY_READONLY_STRING_COLLECTION(COMFactory, AllAvailableChannels,      ST_MStdStringVector_S)
   M_CLASS_PROPERTY_READONLY_STRING_COLLECTION(COMFactory, AllAvailableProtocols,     ST_MStdStringVector_S)
M_START_METHODS(COMFactory)
#if !M_NO_MCOM_IDENTIFY_METER
   M_CLASS_SERVICE           (COMFactory, GetIdentifyStrings,               ST_MStdStringVector_S_constMStdStringA)
#endif
   M_CLASS_SERVICE           (COMFactory, CreateChannel,                    ST_MObjectP_S_constMStdStringA)
   M_CLASS_SERVICE           (COMFactory, CreateProtocol,                   ST_MObjectP_S_constMVariantA_constMStdStringA)
   M_CLASS_SERVICE           (COMFactory, CreateProtocolWithoutChannel,     ST_MObjectP_S_constMStdStringA)
M_END_CLASS(COMFactory, Object)


#if !M_NO_MCOM_MONITOR
   bool MCOMFactory::s_createDefaultMonitor = false;
#endif

   using namespace std;

   #if !M_NO_MCOM_MONITOR
      static MChannel* SetMonitor(MChannel* channel)
      {
         M_ASSERT(channel != NULL);
         if ( MCOMFactory::s_createDefaultMonitor )
         {
            // TODO: implement this
            channel->SetMonitor(M_NEW MMonitorSocket);
         }
         return channel;
      }
   #else
      inline MChannel* SetMonitor(MChannel* channel)
      {
         M_ASSERT(channel != NULL);
         return channel;
      }
   #endif

MChannel* MCOMFactory::CreateChannelByName(const MStdString& channelName)
{
#if !M_NO_MCOM_CHANNEL_SOCKET
   if ( MChannelSocket::GetStaticClass()->MatchesClassOrTypeName(channelName) )
      return SetMonitor(M_NEW MChannelSocket());
   if ( MChannelSocketCallback::GetStaticClass()->MatchesClassOrTypeName(channelName) )
      return SetMonitor(M_NEW MChannelSocketCallback());
#endif

#if !M_NO_MCOM_CHANNEL_SOCKET_UDP
   if ( MChannelSocketUdp::GetStaticClass()->MatchesClassOrTypeName(channelName) )
      return SetMonitor(M_NEW MChannelSocketUdp());
   if ( MChannelSocketUdpCallback::GetStaticClass()->MatchesClassOrTypeName(channelName) )
      return SetMonitor(M_NEW MChannelSocketUdpCallback());
#endif

#if !M_NO_SERIAL_PORT
   if ( MChannelOpticalProbe::GetStaticClass()->MatchesClassOrTypeName(channelName) )
      return SetMonitor(M_NEW MChannelOpticalProbe());
   if ( MChannelSerialPort::GetStaticClass()->MatchesClassOrTypeName(channelName) )
      return SetMonitor(M_NEW MChannelSerialPort());
   if ( MChannelCurrentLoop::GetStaticClass()->MatchesClassOrTypeName(channelName) )
      return SetMonitor(M_NEW MChannelCurrentLoop());
#endif

#if !M_NO_MCOM_CHANNEL_MODEM
   if ( MChannelModem::GetStaticClass()->MatchesClassOrTypeName(channelName) )
      return SetMonitor(M_NEW MChannelModem());
   if ( MChannelModemCallback::GetStaticClass()->MatchesClassOrTypeName(channelName) )
      return SetMonitor(M_NEW MChannelModemCallback());
#endif

   MCOMException::Throw(M_ERR_UNKNOWN_CHANNEL_S1, "Channel '%s' is unknown", channelName.c_str());
   M_ENSURED_ASSERT(0); // we are never here
   return NULL;
}

MChannel* MCOMFactory::CreateChannel(const MStdString& channelSource)
{
   MDictionary properties(channelSource);
   MVariant* val = properties.GetValue(MCOMObject::s_typeString);
   if ( val == NULL )
      val = properties.GetValue(MCOMObject::s_typeCamelcaseString);
   const MStdString channelName = (val != NULL) ? val->AsString() : channelSource;
   MUniquePtr<MChannel> channel(CreateChannelByName(channelName));
   channel->SetPropertyValues(properties);


   return channel.release();
}

MProtocol* MCOMFactory::CreateProtocolByName(MChannel* channel, const MStdString& protocolName)
{
#if !M_NO_MCOM_PROTOCOL_C1218
   if ( MProtocolC1218::GetStaticClass()->MatchesClassOrTypeName(protocolName) )
      return M_NEW MProtocolC1218(channel);
#endif
#if !M_NO_MCOM_PROTOCOL_C1221
   if ( MProtocolC1221::GetStaticClass()->MatchesClassOrTypeName(protocolName) )
      return M_NEW MProtocolC1221(channel);
#endif
#if !M_NO_MCOM_PROTOCOL_C1222
   if ( MProtocolC1222::GetStaticClass()->MatchesClassOrTypeName(protocolName) )
      return M_NEW MProtocolC1222(channel);
#endif
   MCOMException::Throw(M_ERR_UNKNOWN_PROTOCOL_S1, "Protocol '%s' is unknown", protocolName.c_str());
   M_ENSURED_ASSERT(0); // we are never here
   return 0;
}

MProtocol* MCOMFactory::CreateProtocol(const MVariant& channelObjectOrSource, const MStdString& protocolSource)
{
   MChannel* channel = NULL;

   if ( !channelObjectOrSource.IsEmpty() )
   {
      if ( channelObjectOrSource.IsObject() )
      {
         MObject* ch = channelObjectOrSource.AsObject();
         if ( ch != NULL )
            channel = M_DYNAMIC_CAST_WITH_THROW(MChannel, ch);
      }
      else if ( !channelObjectOrSource.IsNumeric() || channelObjectOrSource.AsDWord() != 0 ) // this guards successful passing of NULL to variant
         channel = CreateChannel(channelObjectOrSource.AsString());
   }
   return DoCreateProtocol(channel, protocolSource);
}

MProtocol* MCOMFactory::CreateProtocolWithoutChannel(const MStdString& protocolSource)
{
   return DoCreateProtocol(NULL, protocolSource);
}

MProtocol* MCOMFactory::DoCreateProtocol(MChannel* channel, const MStdString& protocolSource)
{
   MDictionary properties(protocolSource);
   MVariant* val = properties.GetValue(MCOMObject::s_typeString);
   if ( val == NULL )
      val = properties.GetValue(MCOMObject::s_typeCamelcaseString);
   const MStdString protocolName = (val != NULL) ? val->AsString() : protocolSource;
   MUniquePtr<MProtocol> protocol(CreateProtocolByName(channel, protocolName));
   protocol->SetPropertyValues(properties);


   return protocol.release();
}

   static void DoPushBackClass(MStdStringVector& result, const MClass* c)
   {
      result.push_back(c->GetTypeName());
   }

MStdStringVector MCOMFactory::GetAllAvailableChannels()
{
   MStdStringVector result;
#if !M_NO_SERIAL_PORT
   DoPushBackClass(result, MChannelSerialPort::GetStaticClass());
   DoPushBackClass(result, MChannelCurrentLoop::GetStaticClass());
   DoPushBackClass(result, MChannelOpticalProbe::GetStaticClass());
#endif

#if !M_NO_MCOM_CHANNEL_MODEM
   DoPushBackClass(result, MChannelModem::GetStaticClass());
   DoPushBackClass(result, MChannelModemCallback::GetStaticClass());
#endif

#if !M_NO_MCOM_CHANNEL_SOCKET
   DoPushBackClass(result, MChannelSocket::GetStaticClass());
   DoPushBackClass(result, MChannelSocketCallback::GetStaticClass());
#endif

#if !M_NO_MCOM_CHANNEL_SOCKET_UDP
   DoPushBackClass(result, MChannelSocketUdp::GetStaticClass());
   DoPushBackClass(result, MChannelSocketUdpCallback::GetStaticClass());
#endif

   return result;
}

MStdStringVector MCOMFactory::GetAllAvailableProtocols()
{
   MStdStringVector result;
#if !M_NO_MCOM_PROTOCOL_C1218
   DoPushBackClass(result, MProtocolC1218::GetStaticClass());
#endif
#if !M_NO_MCOM_PROTOCOL_C1221
   DoPushBackClass(result, MProtocolC1221::GetStaticClass());
#endif
#if !M_NO_MCOM_PROTOCOL_C1222
   DoPushBackClass(result, MProtocolC1222::GetStaticClass());
#endif
   return result;
}


#if !M_NO_MCOM_IDENTIFY_METER
MStdStringVector MCOMFactory::GetIdentifyStrings(const MStdString& complexIdentify)
{
   MStdStringVector result;
   if ( !complexIdentify.empty() && complexIdentify[0] == 'J' ) // address the problem if the given string is not an identify string
   {
      MStdString::size_type foundPos;
      MStdString::size_type runningPos = 0;
      while ( (foundPos = complexIdentify.find("];J", runningPos)) != MStdString::npos )
      {
         MStdString::size_type endPos = foundPos + 1u;  // + 1u, because we have to add a closing square brace
         result.push_back(complexIdentify.substr(runningPos, endPos - runningPos));
         runningPos = endPos + 1u; // + 1u, because we have to skip the semicolon
      }
      M_ASSERT(int(complexIdentify.size() - runningPos) >= 0);
      MStdString::size_type charsToCopy = int(complexIdentify.size() - runningPos);
      if ( charsToCopy > 0u )
         result.push_back(complexIdentify.substr(runningPos, charsToCopy)); // remainder
   }
   return result;
}
#endif // !M_NO_MCOM_IDENTIFY_METER

#endif // !M_NO_MCOM_FACTORY
