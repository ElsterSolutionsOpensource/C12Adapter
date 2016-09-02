// File MCOM/Monitor.cpp

#include "MCOMExtern.h"
#include "Monitor.h"
#include "MCOMExceptions.h"

#if !M_NO_MCOM_MONITOR

   #define M__CHECK_MONITOR_ENUM(a, b) M_COMPILED_ASSERT(static_cast<int>(MMonitor::a) == static_cast<int>(MMonitor::b))
   M__CHECK_MONITOR_ENUM(MessageChannelAttach,                   MESSAGE_CHANNEL_ATTACH);
   M__CHECK_MONITOR_ENUM(MessageChannelConnect,                  MESSAGE_CHANNEL_CONNECT);
   M__CHECK_MONITOR_ENUM(MessageChannelDisconnect,               MESSAGE_CHANNEL_DISCONNECT);
   M__CHECK_MONITOR_ENUM(MessageChannelByteRx,                   MESSAGE_CHANNEL_BYTE_RX);
   M__CHECK_MONITOR_ENUM(MessageChannelByteTx,                   MESSAGE_CHANNEL_BYTE_TX);
   M__CHECK_MONITOR_ENUM(MessageProtocolLinkLayerInformation,    MESSAGE_PROTOCOL_DATA_LINK_LAYER_INFORMATION);
   M__CHECK_MONITOR_ENUM(MessageProtocolLinkLayerRetry,          MESSAGE_PROTOCOL_DATA_LINK_LAYER_RETRY);
   M__CHECK_MONITOR_ENUM(MessageProtocolLinkLayerFail,           MESSAGE_PROTOCOL_DATA_LINK_LAYER_FAIL);
   M__CHECK_MONITOR_ENUM(MessageProtocolApplicationLayerStart,   MESSAGE_PROTOCOL_APPLICATION_LAYER_START);
   M__CHECK_MONITOR_ENUM(MessageProtocolApplicationLayerRetry,   MESSAGE_PROTOCOL_APPLICATION_LAYER_RETRY);
   M__CHECK_MONITOR_ENUM(MessageProtocolApplicationLayerSuccess, MESSAGE_PROTOCOL_APPLICATION_LAYER_SUCCESS);
   M__CHECK_MONITOR_ENUM(MessageProtocolApplicationLayerFail,    MESSAGE_PROTOCOL_APPLICATION_LAYER_FAIL);
   M__CHECK_MONITOR_ENUM(MessageProtocolSynchronize,             MESSAGE_PROTOCOL_SYNCHRONIZE);
   M__CHECK_MONITOR_ENUM(MessageUser,                            MESSAGE_PROTOCOL_USER_MESSAGE);
   #undef M__CHECK_MONITOR_ENUM

   #if !M_NO_REFLECTION
      static MMonitor* DoNew0()
      {
         return M_NEW MMonitor();
      }
   #endif

M_START_PROPERTIES(Monitor)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT (Monitor, IsListening)
   M_OBJECT_PROPERTY_OBJECT              (Monitor, Client)
   M_CLASS_ENUMERATION                   (Monitor, MessageChannelAttach)
   M_CLASS_ENUMERATION                   (Monitor, MessageChannelConnect)
   M_CLASS_ENUMERATION                   (Monitor, MessageChannelDisconnect)
   M_CLASS_ENUMERATION                   (Monitor, MessageChannelByteRx)
   M_CLASS_ENUMERATION                   (Monitor, MessageChannelByteTx)
   M_CLASS_ENUMERATION                   (Monitor, MessageProtocolLinkLayerInformation)
   M_CLASS_ENUMERATION                   (Monitor, MessageProtocolLinkLayerRetry)
   M_CLASS_ENUMERATION                   (Monitor, MessageProtocolLinkLayerFail)
   M_CLASS_ENUMERATION                   (Monitor, MessageProtocolApplicationLayerStart)
   M_CLASS_ENUMERATION                   (Monitor, MessageProtocolApplicationLayerRetry)
   M_CLASS_ENUMERATION                   (Monitor, MessageProtocolApplicationLayerSuccess)
   M_CLASS_ENUMERATION                   (Monitor, MessageProtocolApplicationLayerFail)
   M_CLASS_ENUMERATION                   (Monitor, MessageProtocolSynchronize)
   M_CLASS_ENUMERATION                   (Monitor, MessageUser)
M_START_METHODS(Monitor)
   M_OBJECT_SERVICE                      (Monitor, Attach,                     ST_X_constMStdStringA)
   M_OBJECT_SERVICE                      (Monitor, Detach,                     ST_X)
   M_OBJECT_SERVICE                      (Monitor, Write,                      ST_X_constMStdStringA)
   M_OBJECT_SERVICE                      (Monitor, OnConnect,                  ST_X)
   M_OBJECT_SERVICE                      (Monitor, OnDisconnect,               ST_X)
   M_OBJECT_SERVICE                      (Monitor, OnBytesReceived,            ST_X_constMByteStringA)
   M_OBJECT_SERVICE                      (Monitor, OnBytesSent,                ST_X_constMByteStringA)
   M_OBJECT_SERVICE                      (Monitor, OnDataLinkLayerSuccess,     ST_X)
   M_OBJECT_SERVICE                      (Monitor, OnDataLinkLayerInformation, ST_X_constMStdStringA)
   M_OBJECT_SERVICE                      (Monitor, OnDataLinkLayerRetry,       ST_X_constMStdStringA)
   M_OBJECT_SERVICE                      (Monitor, OnDataLinkLayerFail,        ST_X_constMStdStringA)
   M_OBJECT_SERVICE                      (Monitor, OnApplicationLayerStart,    ST_X_constMStdStringA)
   M_OBJECT_SERVICE                      (Monitor, OnApplicationLayerSuccess,  ST_X_constMStdStringA)
   M_OBJECT_SERVICE                      (Monitor, OnApplicationLayerRetry,    ST_X_constMStdStringA)
   M_OBJECT_SERVICE                      (Monitor, OnApplicationLayerFail,     ST_X_constMStdStringA)
   M_CLASS_FRIEND_SERVICE                (Monitor, New, DoNew0,                ST_MObjectP_S)
M_END_CLASS(Monitor, Object)

MMonitor::MMonitor()
:
   #if !M_NO_MCOM_MONITOR_SHARED_POINTER
      M_SHARED_POINTER_CLASS_INIT,
   #endif
   m_listening(0), // we start by not listening, wait until attach is called
   m_client(NULL)
{
}

MMonitor::~MMonitor()
{
}

void MMonitor::OnDataLinkLayerSuccess()
{
   // nothing to do
}

void MMonitor::OnMessageWithText(MessageType code, const MStdString& text)
{
   OnMessage(code, text.data(), static_cast<int>(text.size()));
}

void MMonitor::OnDataLinkLayerInformation(const MStdString& msg)
{
   OnMessageWithText(MessageProtocolLinkLayerInformation, msg);
}

void MMonitor::OnDataLinkLayerRetry(const MStdString& reason)
{
   OnMessageWithText(MessageProtocolLinkLayerRetry, reason);
}

void MMonitor::OnDataLinkLayerFail(const MStdString& msg)
{
   OnMessageWithText(MessageProtocolLinkLayerFail, msg);
}

void MMonitor::OnApplicationLayerRetry(const MStdString& reason)
{
   OnMessageWithText(MessageProtocolApplicationLayerRetry, reason);
}

void MMonitor::OnApplicationLayerFail(const MStdString& msg)
{
   OnMessageWithText(MessageProtocolApplicationLayerFail, msg);
}

void MMonitor::OnApplicationLayerStart(const MStdString& service)
{
   OnMessageWithText(MessageProtocolApplicationLayerStart, service);
}

void MMonitor::OnApplicationLayerSuccess(const MStdString& service)
{
   OnMessageWithText(MessageProtocolApplicationLayerSuccess, service);
}

void MMonitor::OnEnterUninterruptibleCommunication()
{
   OnMessageWithText(MessageUser, "Entering communication sequence that shall not be interrupted"); // this text need not be internationalized
}

void MMonitor::OnLeaveUninterruptibleCommunication()
{
   OnMessageWithText(MessageUser, "Leaving communication sequence that shall not be interrupted"); // this text need not be internationalized
}

void MMonitor::OnBytesReceived(const MByteString& data)
{
   OnByteRX(data.data(), static_cast<int>(data.size()));
}

void MMonitor::OnBytesSent(const MByteString& data)
{
   OnByteTX(data.data(), static_cast<int>(data.size()));
}

void MMonitor::Attach(const MStdString& mediaIdentification)
{
   OnMessage(MessageChannelAttach, mediaIdentification.data(), M_64_CAST(int, mediaIdentification.size()));
   #if !M_NO_REFLECTION
      if ( m_client != NULL && m_client->IsServicePresent("Attach") )
         m_client->Call1("Attach", mediaIdentification);
   #endif
}

void MMonitor::Detach()
{
   #if !M_NO_REFLECTION
      if ( m_client != NULL && m_client->IsServicePresent("Detach") )
         m_client->Call0("Detach");
   #endif
}

void MMonitor::Write(const MStdString& message)
{
   OnMessage(MessageUser, message.data(), M_64_CAST(int, message.size()));
#if !M_NO_REFLECTION
   if ( m_client != NULL && m_client->IsServicePresent("Write") )
      m_client->Call1("Write", message);
#endif
}

void MMonitor::OnMessage(MessageType type, const char* text, int size)
{
#if !M_NO_REFLECTION
   if ( m_client != NULL && m_client->IsServicePresent("OnMessage") )
      m_client->Call2("OnMessage", static_cast<int>(type), MVariant(text, size));
#endif
}

void MMonitor::OnConnect()
{
   OnMessage(MessageChannelConnect, "", 0);
   #if !M_NO_REFLECTION
      if ( m_client != NULL && m_client->IsServicePresent("OnConnect") )
         m_client->Call0("OnConnect");
   #endif
}

void MMonitor::OnDisconnect()
{
   OnMessage(MessageChannelDisconnect, "", 0);
   #if !M_NO_REFLECTION
      if ( m_client != NULL && m_client->IsServicePresent("OnDisconnect") )
         m_client->Call0("OnDisconnect");
   #endif
}

void MMonitor::OnByteRX(const char* data, int length)
{
   OnMessage(MessageChannelByteRx, data, length);
   #if !M_NO_REFLECTION
      if ( m_client != NULL && m_client->IsServicePresent("OnByteRX") )
         m_client->Call1("OnByteRX", MVariant(MByteString(data, length), MVariant::ACCEPT_BYTE_STRING));
   #endif
}

void MMonitor::OnByteTX(const char* data, int length)
{
   OnMessage(MessageChannelByteTx, data, length);
   #if !M_NO_REFLECTION
      if ( m_client != NULL && m_client->IsServicePresent("OnByteTX") )
         m_client->Call1("OnByteTX", MVariant(MByteString(data, length), MVariant::ACCEPT_BYTE_STRING));
   #endif
}

#endif
