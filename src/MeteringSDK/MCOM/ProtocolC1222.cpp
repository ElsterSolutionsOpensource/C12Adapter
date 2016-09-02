// File MCOM/ProtocolC1222.cpp

#include "MCOMExtern.h"
#include "ProtocolC1222.h"
#include "ChannelSocketUdp.h"
#include "MCOMExceptions.h"
#include "IdentifyString.h"

#if !M_NO_MCOM_PROTOCOL_C1222

      static int s_defaultSecurityMode = MProtocolC1222::SecurityClearTextWithAuthentication;

M_START_PROPERTIES(ProtocolC1222)
   M_OBJECT_PROPERTY_PERSISTENT_INT        (ProtocolC1222, SecurityMode,               s_defaultSecurityMode)
   M_OBJECT_PROPERTY_PERSISTENT_STRING     (ProtocolC1222, SecurityKey,                "00000000000000000000000000000000", ST_MStdString_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_PERSISTENT_INT        (ProtocolC1222, SecurityKeyId,              0)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL       (ProtocolC1222, Sessionless,                true)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL       (ProtocolC1222, OneServicePerApdu,          false)
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC1222, ResponseControl,            MProtocolC1222::ResponseControlAlways)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL       (ProtocolC1222, IssueTerminateOnEndSession, false)
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC1222, SessionIdleTimeout,         60)
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC1222, ResponseTimeout,            300)
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC1222, MaximumApduSizeIncoming,    0) // means use the same as MaximumApduSize, this has to be mentioned before MaximumApduSize
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC1222, MaximumApduSize,            0x7FFF) // outgoing or common - if MaximumApduSizeIncoming = 0
   M_OBJECT_PROPERTY_READONLY_UINT         (ProtocolC1222, NegotiatedMaximumApduSizeIncoming)
   M_OBJECT_PROPERTY_READONLY_UINT         (ProtocolC1222, NegotiatedMaximumApduSize)
   M_OBJECT_PROPERTY_PERSISTENT_INT        (ProtocolC1222, CallingAeQualifier,         -1)
   M_OBJECT_PROPERTY_PERSISTENT_STRING     (ProtocolC1222, ApplicationContext,         "",    ST_constMStdStringA_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_PERSISTENT_STRING     (ProtocolC1222, CallingApTitle,             "",    ST_constMStdStringA_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_PERSISTENT_STRING     (ProtocolC1222, CalledApTitle,              "",    ST_constMStdStringA_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_PERSISTENT_STRING     (ProtocolC1222, EdClass,                    "",    ST_constMStdStringA_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_READONLY_STRING       (ProtocolC1222, IncomingEdClass,                     ST_constMStdStringA_X)
   M_OBJECT_PROPERTY_READONLY_BOOL         (ProtocolC1222, NegotiatedSessionIdleTimeoutPresent)
   M_OBJECT_PROPERTY_READONLY_UINT         (ProtocolC1222, NegotiatedSessionIdleTimeout)
   M_OBJECT_PROPERTY_UINT                  (ProtocolC1222, InitializationVector)
   M_OBJECT_PROPERTY_UINT                  (ProtocolC1222, CallingApInvocationId)
   M_OBJECT_PROPERTY_READONLY_UINT         (ProtocolC1222, IncomingSecurityKeyId)
   M_OBJECT_PROPERTY_READONLY_UINT         (ProtocolC1222, IncomingInitializationVector)
   M_OBJECT_PROPERTY_READONLY_UINT         (ProtocolC1222, IncomingCallingApInvocationId)
   M_OBJECT_PROPERTY_READONLY_STRING       (ProtocolC1222, IncomingCalledApTitle,            ST_constMStdStringA_X)
   M_OBJECT_PROPERTY_READONLY_STRING       (ProtocolC1222, IncomingCallingApTitle,           ST_constMStdStringA_X)
   M_OBJECT_PROPERTY_BYTE_STRING           (ProtocolC1222, IncomingApdu,                     ST_MByteString_X, ST_X_constMByteStringA)
   M_OBJECT_PROPERTY_READONLY_BYTE_STRING  (ProtocolC1222, OutgoingApdu,                     ST_MByteString_X)
   M_OBJECT_PROPERTY_READONLY_BYTE_STRING  (ProtocolC1222, IncomingEpsem,                    ST_MByteString_X)
   M_OBJECT_PROPERTY_READONLY_UINT         (ProtocolC1222, IncomingSecurityMode)
   M_OBJECT_PROPERTY_READONLY_UINT         (ProtocolC1222, IncomingResponseControl)
   M_OBJECT_PROPERTY_READONLY_INT          (ProtocolC1222, IncomingCallingAeQualifier)
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   M_OBJECT_PROPERTY_BYTE_STRING_COLLECTION(ProtocolC1222, SecurityKeyList,                  ST_constMByteStringVectorA_X)
   M_OBJECT_PROPERTY_READONLY_INT          (ProtocolC1222, SecurityKeyListSuccessfulEntry)
#endif
M_START_METHODS(ProtocolC1222)
   M_OBJECT_SERVICE                      (ProtocolC1222, ServerStart,                      ST_X)
   M_OBJECT_SERVICE                      (ProtocolC1222, ServerReset,                      ST_X)
   M_OBJECT_SERVICE                      (ProtocolC1222, ProcessIncomingEPSEM,             ST_X)
   M_OBJECT_SERVICE                      (ProtocolC1222, ServerEnd,                        ST_X)
   M_OBJECT_SERVICE                      (ProtocolC1222, SendStart,                        ST_X)
   M_OBJECT_SERVICE                      (ProtocolC1222, SendService,                      ST_X_byte)
   M_OBJECT_SERVICE                      (ProtocolC1222, SendServiceWithData,              ST_X_byte_constMByteStringA)
   M_OBJECT_SERVICE                      (ProtocolC1222, SendEnd,                          ST_X)
   M_OBJECT_SERVICE                      (ProtocolC1222, ReceiveStart,                     ST_X)
   M_OBJECT_SERVICE                      (ProtocolC1222, SendEndReceiveStart,              ST_bool_X)
   M_OBJECT_SERVICE                      (ProtocolC1222, ReceiveServiceLength,             ST_unsigned_X)
   M_OBJECT_SERVICE                      (ProtocolC1222, ReceiveServiceCodeIgnoreLength,   ST_byte_X)
   M_OBJECT_SERVICE                      (ProtocolC1222, ReceiveEnd,                       ST_X)
   M_OBJECT_SERVICE                      (ProtocolC1222, WriteApdu,                        ST_X_constMByteStringA)
   M_OBJECT_SERVICE                      (ProtocolC1222, ReadApdu,                         ST_X)
M_END_CLASS_TYPED(ProtocolC1222, ProtocolC12, "PROTOCOL_ANSI_C12_22")

   // Define this for debugging protocol: M__DEBUG_C1222_WITH_CONSTANT_DATA

   static const MChar s_standardApplicationContextOid [] = "2.16.124.113620.1.22";
   static const MChar s_standardNetworkContextOid     [] = "2.16.124.113620.1.22.0";
   static const MChar s_elsterOid                     [] = "1.3.6.1.4.1.33507.1";

   static void DoAddEdClass(MBuffer& result, const MStdString& edClass)
   {
      if ( !edClass.empty() )
      {
         M_ASSERT(edClass.size() <= 4);
         result.Append(edClass);
         if ( edClass.size() < 4 ) // pad with blanks up to 4 characters
            result.Append("    ", static_cast<unsigned>(4 - edClass.size()));
      }
   }

MProtocolC1222::MProtocolC1222(MChannel* channel, bool channelIsOwned)
:
   MProtocolC12(channel, channelIsOwned),
   m_oneServicePerApdu(false),
   m_negotiatedSessionIdleTimeoutPresent(false),
   m_negotiatedSessionIdleTimeout(0),
   m_initializationVector(0),
   m_callingApInvocationId(0), // will be random anyway
   m_initializationVectorSetByUser(false),
   m_callingApInvocationIdSetByUser(false),
   m_edClass(),
   m_maximumApduSizeIncoming(0x7FFF),
   m_maximumApduSizeOutgoing(0x7FFF),
   m_incomingSecurityKeyId(0),         // this is not reset in DoResetIncomingProperties()
   m_incomingInitializationVector(0),  // this is not reset in DoResetIncomingProperties()
   m_securityKeyIdAndInitializationVectorWereSent(false)
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   , m_securityKeyList()
   , m_securityKeyListSuccessfulEntry(-1)
#endif // !M_NO_MCOM_PASSWORD_AND_KEY_LIST
{
   m_wrapperProtocol = this; // default
   M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(ProtocolC1222);
   DoResetIncomingProperties();
   DoResetSessionSpecificProperties();
}

MProtocolC1222::~MProtocolC1222()
{
   Finalize(); // need to perform this before destroying class fields

   MAes::DestroySecureData(m_securityKey);
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   MAes::DestroySecureData(m_securityKeyList);
#endif
}

void MProtocolC1222::SetResponseControl(ResponseControlEnum c)
{
   MENumberOutOfRange::CheckNamedIntegerRange((int)ResponseControlAlways, (int)ResponseControlNever, (int)c, M_OPT_STR("RESPONSE_CONTROL"));
   m_responseControl = c;
}

   static void DoCheckSecurityDefined(MProtocolC1222::SecurityModeEnum mode)
   {
      // These asserts are always the case
         M_ASSERT(mode >= MProtocolC1222::SecurityUndefined && mode <= MProtocolC1222::SecurityCipherTextWithAuthentication);
      if ( mode < MProtocolC1222::SecurityClearText )
      {
         MException::Throw(MException::ErrorSoftware, M_CODE_STR(MErrorEnum::NumberOutOfRange, "Security mode is undefined, cannot communicate"));
         M_ENSURED_ASSERT(0);
      }
   }

void MProtocolC1222::SetSecurityMode(SecurityModeEnum mode)
{
   MENumberOutOfRange::CheckNamedIntegerRange((int)SecurityUndefined, (int)SecurityCipherTextWithAuthentication, (int)mode, M_OPT_STR("SECURITY_MODE"));
   m_securityMode = mode;
}


void MProtocolC1222::SetSecurityKey(const MStdString& key)
{
   m_eax.SetHexKey(key); // this is a mere key check: whether it is a proper HEX string to represent a proper key
   MAes::AssignSecureData(m_securityKey, m_eax.GetKey());
}


void MProtocolC1222::SetIssueSecurityOnStartSession(bool yes)
{
   MProtocolC12::SetIssueSecurityOnStartSession(yes);
   DoResetNegotiatedMaximumApduSizes();
}

MStdString MProtocolC1222::GetSecurityKey() const
{
   return MUtilities::BytesToHex(m_securityKey, false);
}

void MProtocolC1222::SetSecurityKeyId(int id)
{
   MENumberOutOfRange::CheckNamedIntegerRange(0, 255, id, M_OPT_STR("SECURITY_KEY_ID"));
   m_securityKeyId = id;
}


void MProtocolC1222::SetSessionless(bool yes)
{
   m_sessionless = yes;
   DoResetNegotiatedMaximumApduSizes();
}

unsigned MProtocolC1222::GetNegotiatedSessionIdleTimeout() const
{
   if ( !m_negotiatedSessionIdleTimeoutPresent )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_NEGOTIATED_SESSION_IDLE_TIMEOUT_IS_NOT_AVAILABLE, "ANSI Logon service was not called, the session idle timeout is not available"));
      M_ENSURED_ASSERT(0);
   }
   return m_negotiatedSessionIdleTimeout;
}

void MProtocolC1222::SetResponseTimeout(unsigned timeout)
{
   MENumberOutOfRange::CheckNamedUnsignedRange(0, 0xFFFF, timeout, "RESPONSE_TIMEOUT"); // have to limit this value to prevent overflow when getting milliseconds from seconds
   m_responseTimeout = timeout;
}

void MProtocolC1222::SetSessionIdleTimeout(unsigned timeout)
{
   MENumberOutOfRange::CheckInteger(0, 0xFFFF, (int)timeout, M_OPT_STR("SESSION_IDLE_TIMEOUT"));
   m_sessionIdleTimeout = timeout;
}

void MProtocolC1222::SetApplicationContext(const MStdString& applicationContext)
{
   if ( !applicationContext.empty() && MIso8825::IsUidRelative(applicationContext) ) // IsUidRelative checks uid for validity
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_APPLICATION_CONTEXT_SHALL_BE_ABSOULTE, "Application context shall be an absolute UID"));
      M_ENSURED_ASSERT(0);
   }
   m_applicationContext = applicationContext;
}

void MProtocolC1222::SetCallingApTitle(const MStdString& callingApTitle)
{
   if ( !callingApTitle.empty() )
      MIso8825::IsUidRelative(callingApTitle); // this verifies the format
   m_callingApTitle = callingApTitle;
}

void MProtocolC1222::SetCalledApTitle(const MStdString& calledApTitle)
{
   if ( calledApTitle != m_calledApTitle ) // we go to another device
   {
      if ( !calledApTitle.empty() )
         MIso8825::IsUidRelative(calledApTitle); // this verifies the format
      m_calledApTitle = calledApTitle;
   }
}

void MProtocolC1222::SetEdClass(const MStdString& edClass)
{
   MENumberOutOfRange::CheckInteger(0, 4, unsigned(edClass.size()), M_OPT_STR("ED_CLASS length"));
   m_edClass = edClass;
}

   static void DoExtractApduFromBuffer(MByteString& result, const char* buff, unsigned size)
   {
      M_ASSERT(result.empty());
      if ( size <= 8 ) // this number is made up, no problem as there is not full syntax check here anyway
         return;
      if ( *buff != '\x60' ) // presumably, there is a transport layer in the buffer, skip it
      {
         ++buff;
         --size;
         unsigned runningIndex = 0;
         unsigned transportLayerSize = MIso8825::DecodeLengthFromBuffer(buff, size, &runningIndex);
         size -= runningIndex;
         if ( static_cast<int>(size - transportLayerSize) <= 8 ) // the number 8 is made up, no complete check anyway
            return;
         buff += runningIndex;
         buff += transportLayerSize;
         size -= transportLayerSize;
      }
      if ( *buff == '\x60' ) // correct APDU
         result.assign(buff, size);
   }

MByteString MProtocolC1222::GetIncomingApdu() const
{
   MByteString result;
   DoExtractApduFromBuffer(result, m_incomingApdu.GetTotalPtr(), m_incomingApdu.GetTotalSize());
   return result;
}

void MProtocolC1222::SetIncomingApdu(const MByteString& apdu)
{
   DoResetIncomingProperties();
   m_incomingApdu.Assign(apdu);
   m_applicationLayerReader.AssignBuffer(&m_incomingApdu);
   DoParseStartHeader();
}

MByteString MProtocolC1222::GetIncomingEpsem() const
{
   MByteString result;
   if ( m_incomingEpsemSize > 0 )
   {
      MByteString::size_type offset = m_incomingApdu.GetTotalSize() - m_incomingEpsemSize;
      if ( static_cast<int>(offset) > 0 )
         result.assign(m_incomingApdu.GetTotalPtr() + offset, m_incomingEpsemSize);
   }
   return result;
}

MByteString MProtocolC1222::GetOutgoingApdu() const
{
   MByteString result;
   DoExtractApduFromBuffer(result, m_outgoingApdu.GetTotalPtr(), m_outgoingApdu.GetTotalSize());
   return result;
}

void MProtocolC1222::SetMaximumApduSize(unsigned size)
{
   MENumberOutOfRange::CheckNamedUnsignedRange(unsigned(MinimumMaximumApduTotalSize), unsigned(MaximumMaximumApduTotalSize), size, M_OPT_STR("MAXIMUM_APDU_SIZE"));
   m_maximumApduSizeOutgoing = size;
   ChangeNegotiatedMaximumApduSizeOutgoing(size);
   if ( m_maximumApduSizeIncoming == 0 ) // applying the default
      ChangeNegotiatedMaximumApduSizeIncoming(size);
}

#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST

void MProtocolC1222::SetSecurityKeyList(const MByteStringVector& securityKeyList)
{
   MAesEax localEax;

   m_securityKeyListSuccessfulEntry = -1;

   MByteStringVector::const_iterator it = securityKeyList.begin();
   MByteStringVector::const_iterator itEnd = securityKeyList.end();
   for ( ; it != itEnd; ++it )
      localEax.SetHexKey(*it); // check if the key is valid

   MAes::AssignSecureData(m_securityKeyList, securityKeyList);
}

#endif // !M_NO_MCOM_PASSWORD_AND_KEY_LIST


void MProtocolC1222::SetMaximumApduSizeIncoming(unsigned size)
{
   if ( size == 0 )
   {
      m_maximumApduSizeIncoming = 0;
      ChangeNegotiatedMaximumApduSizeIncoming(m_maximumApduSizeOutgoing); // use outgoing
   }
   else
   {
      MENumberOutOfRange::CheckNamedUnsignedRange(unsigned(MinimumMaximumApduTotalSize), unsigned(MaximumMaximumApduTotalSize), size, M_OPT_STR("MAXIMUM_APDU_SIZE_INCOMING"));
      m_maximumApduSizeIncoming = size;
      ChangeNegotiatedMaximumApduSizeIncoming(size);
   }
}

void MProtocolC1222::ChangeNegotiatedMaximumApduSizeOutgoing(unsigned size)
{
   MENumberOutOfRange::CheckNamedUnsignedRange(unsigned(MinimumMaximumApduTotalSize), unsigned(MaximumMaximumApduTotalSize), size, M_OPT_STR("MAXIMUM_APDU_SIZE"));
   m_effectiveMaximumApduSizeOutgoing  = size;
   m_negotiatedMaximumApduSizeOutgoing = size;
   const int maximumHeaderSize = DoGetMaximumApduHeaderSize();

   M_ASSERT(size > SESSIONLESS_SECURITY_SERVICE_OVERHEAD + unsigned(maximumHeaderSize + PARTIAL_WRITE_SERVICE_OVERHEAD + MaximumEpsemServiceLengthSize) + 16); // allow for at least 16 bytes of the data, debug check as the properties are range checked
   if ( m_sessionless )
   {
      if ( m_issueSecurityOnStartSession )
         size -= SESSIONLESS_SECURITY_SERVICE_OVERHEAD;

   }

   m_maximumWriteTableSize = size - unsigned(maximumHeaderSize + (WRITE_SERVICE_OVERHEAD + MaximumEpsemServiceLengthSize));
   m_maximumPartialWriteTableSize = size - unsigned(maximumHeaderSize + PARTIAL_WRITE_SERVICE_OVERHEAD + MaximumEpsemServiceLengthSize);
}

void MProtocolC1222::ChangeNegotiatedMaximumApduSizeIncoming(unsigned size)
{
   if ( size != 0 )
      MENumberOutOfRange::CheckNamedUnsignedRange(unsigned(MinimumMaximumApduTotalSize), unsigned(MaximumMaximumApduTotalSize), size, M_OPT_STR("MAXIMUM_APDU_SIZE"));
   else
      size = m_maximumApduSizeOutgoing;

   m_effectiveMaximumApduSizeIncoming  = size;
   m_negotiatedMaximumApduSizeIncoming = size;
   if ( m_sessionless )
   {
      if ( m_issueSecurityOnStartSession )
         size -= 2; // response to sessionless Security

   }
   m_maximumReadTableSize = size - unsigned(DoGetMaximumApduHeaderSize() + READ_SERVICE_OVERHEAD + MaximumEpsemServiceLengthSize);
}

void MProtocolC1222::ServerStart()
{
   DoReceiveStartHeader();
   ServerReset();
}

void MProtocolC1222::ServerReset()
{
   m_outgoingApdu.ClearWithReserve(MaximumProperApduHeaderSize, m_negotiatedMaximumApduSizeOutgoing);
   DoAddEdClass(m_outgoingApdu, m_edClass);

   // m_securityMode = m_incomingSecurityMode;    <- this has to be set explicitly by emulating code
   m_calledApTitle = m_incomingCallingApTitle;
   m_callingApTitle = m_incomingCalledApTitle;
   if ( m_securityKeyIdAndInitializationVectorWereReceived || m_sessionless )
      m_securityKeyIdAndInitializationVectorWereSent = false; // exchange key id and iv
}

void MProtocolC1222::ServerEnd()
{
   DoUpdateCallingApInvocationId(false);
   if ( m_outgoingApdu.GetBodySize() > m_edClass.size() ) // added something
      SendEnd();
}

void MProtocolC1222::SendStart()
{
   DoCheckSecurityDefined(m_securityMode);
   m_incomingCallingApInvocationIdPresent = false; // start a completely new request
   m_outgoingApdu.ClearWithReserve(MaximumProperApduHeaderSize, m_negotiatedMaximumApduSizeOutgoing);
   DoAddEdClass(m_outgoingApdu, m_edClass);
   if ( m_sessionless )
   {
      m_securityKeyIdAndInitializationVectorWereSent = false;
      if ( m_responseControl != ResponseControlAlways )
         DoUpdateCallingApInvocationId(false);
      if ( m_issueSecurityOnStartSession )
         SendSecurity();
   }
}

#if !M_NO_MCOM_MONITOR
void MProtocolC1222::DoSendACSEToMonitor(MConstChars elementName, char elementCode, const MStdString& value)
{
   if ( elementName != NULL && m_channel != NULL )
   {
      MMonitor* monitor = m_channel->GetMonitor();
      if ( monitor != NULL )
         monitor->Write(MGetStdString("%c ACSE 0x%2X <%s> = \"%s\"", m_logHeaderChar, (unsigned)(Muint8)elementCode, elementName, value.c_str()));
   }
}

void MProtocolC1222::DoSendACSEToMonitor(MConstChars elementName, char elementCode, unsigned value)
{
   if ( elementName != NULL && m_channel != NULL )
   {
      MMonitor* monitor = m_channel->GetMonitor();
      if ( monitor != NULL )
         monitor->Write(MGetStdString("%c ACSE 0x%2X <%s> = %u", m_logHeaderChar, (unsigned)(Muint8)elementCode, elementName, value));
   }
}

void MProtocolC1222::DoSendACSECallingAuthenticationToMonitor(unsigned key, unsigned vect)
{
   if ( m_channel != NULL )
   {
      MMonitor* monitor = m_channel->GetMonitor();
      if ( monitor != NULL )
         monitor->Write(MGetStdString("%c ACSE 0xAC <calling-authentication-value> = %d, 0x%08X", m_logHeaderChar, key, vect));
   }
}

void MProtocolC1222::DoSendEpsemToMonitor(const char* epsem, unsigned epsemSize)
{
   if ( m_channel != NULL )
   {
      MMonitor* monitor = m_channel->GetMonitor();
      if ( monitor != NULL )
      {
         MStdString str;
         str.reserve(epsemSize * 3 + 8);
         char tmpMessage [ 12 ] = "? EPSEM "; // not static!
         tmpMessage[0] = m_logHeaderChar;
         str.append(tmpMessage, 8);
         str += MUtilities::BufferToHexString(epsem, epsemSize);
         monitor->Write(str);
      }
   }
}
#endif

void MProtocolC1222::DoAppendAbsoluteUidIfPresent(MBuffer& acse, char elementCode, const MStdString& base, const MStdString& id)
{
   if ( !id.empty() )
   {
      if ( MIso8825::IsUidRelative(id) )
      {
         if ( !base.empty() )
            acse.AppendUidIfPresent(elementCode, base + id);
         else
            acse.AppendUidIfPresent(elementCode, s_standardNetworkContextOid + id);
      }
      else
         acse.AppendUidIfPresent(elementCode, id);
   }
}

void MProtocolC1222::DoAppendCallingInvocation(MBuffer& acse, unsigned keyId, Muint32 initializationVector)
{
   char callingAuthenticationValueElement[17] = "\xAC\x0F\xA2\x0D\xA0\x0B\xA1\x09\x80\x01\x00\x81\x04"; // plus four extra bytes

   M_ASSERT(m_securityKeyId < 256);
   M_ASSERT(initializationVector != 0); // in release-mode reality it can (in year 2038)
   callingAuthenticationValueElement[10] = (char)(Muint8)keyId; // assign <key-id>
   memcpy(&callingAuthenticationValueElement[13], &initializationVector, sizeof(Muint32));
   acse.Append(callingAuthenticationValueElement, sizeof(callingAuthenticationValueElement));
}

void MProtocolC1222::DoGetUid(MConstChars elementName, char elementCode, MStdString& id)
{
   Muint8 typeByte = m_applicationLayerReader.ReadByte();
   unsigned length = m_applicationLayerReader.ReadIsoLength();
   if ( typeByte == Muint8('\x80') )
      MIso8825::DecodeUidFromBuffer(id, m_applicationLayerReader.GetReadPtr(), length, true); // relative
   else if ( typeByte ==  Muint8('\x06') )
      MIso8825::DecodeUidFromBuffer(id, m_applicationLayerReader.GetReadPtr(), length, false); // absolute
   else
   {
      DoThrowBadACSEResponse(elementCode);
      M_ENSURED_ASSERT(0);
   }
   #if !M_NO_MCOM_MONITOR
      DoSendACSEToMonitor(elementName, elementCode, id);
   #endif
   m_applicationLayerReader.IgnoreBytes(length);
}

unsigned MProtocolC1222::DoGetInteger(MConstChars elementName, char elementCode)
{
   unsigned result = 0;
   Muint8 typeByte = m_applicationLayerReader.ReadByte();
   unsigned length = m_applicationLayerReader.ReadIsoLength();
   if ( typeByte != Muint8('\x02') || length == 0 || length > 4 )
   {
      DoThrowBadACSEResponse(elementCode);
      M_ENSURED_ASSERT(0);
   }
   for ( ; length > 0; --length )
   {
      result <<= 8;
      result |= m_applicationLayerReader.ReadByte();
   }
   #if !M_NO_MCOM_MONITOR
      DoSendACSEToMonitor(elementName, elementCode, result);
   #endif
   return result;
}

void MProtocolC1222::ReadApdu()
{
   MProtocolLinkLayerWrapper wrapper(this);
   try
   {
      m_incomingApdu.Clear();
      m_incomingApdu.Reserve(m_negotiatedMaximumApduSizeIncoming);

      char buff [ 8 ];
      buff[0] = '\x60'; // expected start char

      m_channel->SetIntercharacterTimeout(0); // say to channel that read timeout is responsible for the whole packet
      unsigned millisecondsTimeout = MTimer::SecondsToMilliseconds(m_responseTimeout);  // convert seconds to milliseconds
      unsigned endTime = MUtilities::GetTickCount() + millisecondsTimeout;
      DoReadStartCharacter("\x60", millisecondsTimeout);

      int timeDiff = int(endTime - MUtilities::GetTickCount());
      if ( timeDiff <= 0 )
         timeDiff = 1000; // this way handle the case when the characters are already in the buffer
      m_channel->SetReadTimeout(unsigned(timeDiff));
      m_channel->ReadBuffer(buff + 1, 1);
      unsigned buffSize = 2; // this includes length
      unsigned elementsLength = (unsigned)(Muint8)buff[1];
      if ( (elementsLength & 0x80u) != 0u ) // read extra length from buffer
      {
         unsigned lengthSize = elementsLength & 0x7Fu;      // clear the bit and get the number of the following octets
         if ( lengthSize > 3u ) // we do not support length bigger than three bytes (24 megs)
         {
            MIso8825::ThrowBadISOLength();
            M_ENSURED_ASSERT(0);
         }
         m_channel->ReadBuffer(buff + 2, lengthSize);
         elementsLength = 0u;
         buffSize = lengthSize + 2; // adjust to buffSize
         for ( unsigned i = 2; i < buffSize; ++i )
         {
            elementsLength <<= 8;
            elementsLength += (unsigned)(Muint8)buff[i];
         }
      }
      m_incomingApdu.AccessAllBytes().resize(buffSize + elementsLength);
      memcpy(&m_incomingApdu.AccessAllBytes()[0], buff, buffSize);

      if ( elementsLength > 0 )
      {
         timeDiff = int(endTime - MUtilities::GetTickCount());
         if ( timeDiff <= 0 )
            timeDiff = 1000; // this way handle the case when the characters are already in the buffer
         m_channel->SetReadTimeout(unsigned(timeDiff));
         m_channel->ReadBuffer(&m_incomingApdu.AccessAllBytes()[buffSize], elementsLength);
      }
      m_applicationLayerReader.AssignBuffer(&m_incomingApdu);
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

void MProtocolC1222::WriteApdu(const MByteString& buffer)
{
   m_outgoingApdu.Assign(buffer);
   DoWriteApdu();
}

void MProtocolC1222::DoWriteApdu()
{
   MProtocolLinkLayerWrapper wrapper(this);
   try
   {
      if ( m_responseControl != ResponseControlNever )
         Sleep(m_turnAroundDelay);
      m_channel->WriteBuffer(m_outgoingApdu.GetTotalPtr(), m_outgoingApdu.GetTotalSize());
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

void MProtocolC1222::SendEnd()
{
   M_ASSERT(m_applicationContext.empty() || !MIso8825::IsUidRelative(m_applicationContext));

   if ( m_securityMode != SecurityClearText && !m_securityKeyIdAndInitializationVectorWereSent )
   {
      if ( !m_initializationVectorSetByUser )
      {
         #if defined(M__DEBUG_C1222_WITH_CONSTANT_DATA) && M__DEBUG_C1222_WITH_CONSTANT_DATA != 0
            m_initializationVector = 0x916A784B;
         #else
            // Per standard, IV should represent UTC time within range of -15 .. 15 minutes
            time_t t = time(NULL);
            unsigned secondsSince1970 = (unsigned)t;
            unsigned ivCandidaie = MToBigEndianUINT32(secondsSince1970);
            if ( ivCandidaie == m_initializationVector )           // never send the same IV twice, per standard
            {                                                      // for that simply increment the candidate
               ++secondsSince1970;                                 // this does not prevent iv from repeating,
               ivCandidaie = MToBigEndianUINT32(secondsSince1970); // but the two will never be sent twice in subsequent packets
            }
            m_initializationVector = ivCandidaie;
         #endif
      }
      else
         m_initializationVectorSetByUser = false; // user does the initialization vector only once
   }

#if !M_NO_MCOM_MONITOR
   m_logHeaderChar = '>';

   if ( !m_applicationContext.empty() )
      DoSendACSEToMonitor("application-context", '\xA1', m_applicationContext);
   if ( !m_calledApTitle.empty() )
      DoSendACSEToMonitor("called-AP-title", '\xA2', m_calledApTitle);
   if ( m_incomingCallingApInvocationIdPresent )
      DoSendACSEToMonitor("called-AP-invocation-id", '\xA4', m_incomingCallingApInvocationId);
   if ( !m_callingApTitle.empty() )
      DoSendACSEToMonitor("calling-AP-title", '\xA6', m_callingApTitle);
   if ( m_callingAeQualifier != -1 )
      DoSendACSEToMonitor("calling-AE-qualifier", '\xA7', m_callingAeQualifier);
   DoSendACSEToMonitor("calling-AP-invocation-id", '\xA8', m_callingApInvocationId);
   if ( m_securityMode != SecurityClearText && !m_securityKeyIdAndInitializationVectorWereSent )
      DoSendACSECallingAuthenticationToMonitor(m_securityKeyId, m_initializationVector);
   if ( m_securityMode >= SecurityCipherTextWithAuthentication ) // also covers compression
      DoSendEpsemToMonitor(m_outgoingApdu.GetBodyPtr(), m_outgoingApdu.GetBodySize());
#endif

   m_canonifiedCleartext.Clear();

   SecurityModeEnum securityMode = m_securityMode;


   unsigned tailSize = (securityMode == SecurityClearText) ? 0 : 4; // conditionally add 4 bytes of <mac>

   // EPSEM control:
   //   Bit 7:    Reserved, Shall be equal to 1
   //   Bit 6:    RECOVERY_SESSION: Flag used to initiate a special session
   //   Bit 5:    PROXY_SERVICE_USED: 0 = This message has not been sent though a proxy C12.22 Relay, 1 = This message was sent though a proxy.
   //   Bit 4:    ED_CLASS_INCLUDED: 1 = <ed-class> is included in this <acse-pdu>. <ed-class> shall be included in all unsolicited messages and one-way messages.
   //   Bit 2..3: SECURITY_MODE: 0 = Cleartext, 1 = Cleartext with authentication, 2 = Ciphertext with authentication
   //   Bit 0..1: RESPONSE_CONTROL: 0 = Always respond, 1 = Respond on exception, 2 = Never respond
   //
   char epsemControl = '\x80';  // 0x90 = EPSEM control: APPLICATION_SPECIFIC_TAG = 2, ED_CLASS_INCLUDED = 0
   epsemControl |= (char)m_responseControl; // RESPONSE_CONTROL = 0..2
   epsemControl |= Muint8(securityMode << 2);
   if ( !m_edClass.empty() )
      epsemControl |= '\x10';

   m_outgoingApdu.Prepend(epsemControl);
   m_outgoingApdu.PrependIsoLength(m_outgoingApdu.GetTotalSize() + tailSize);
   m_outgoingApdu.Prepend('\x81'); // <user-information-octet-string>
   m_outgoingApdu.PrependIsoLength(m_outgoingApdu.GetTotalSize() + tailSize);
   m_outgoingApdu.Prepend('\x28'); // <user-information-external>
   m_outgoingApdu.PrependIsoLength(m_outgoingApdu.GetTotalSize() + tailSize);
   m_outgoingApdu.Prepend('\xBE'); // <user-information-external>

   if ( securityMode != SecurityClearText )
   {
      DoInitializeEax(m_calledApTitle);

      m_canonifiedCleartext.AppendUidIfPresent('\xA1', m_applicationContext);
      DoAppendAbsoluteUidIfPresent(m_canonifiedCleartext, '\xA2', m_applicationContext, m_calledApTitle);
      if ( m_incomingCallingApInvocationIdPresent )
         m_canonifiedCleartext.AppendUnsigned('\xA4', m_incomingCallingApInvocationId);
      if ( m_callingAeQualifier != -1 )
         m_canonifiedCleartext.AppendUnsigned('\xA7', m_callingAeQualifier);
      m_canonifiedCleartext.AppendUnsigned('\xA8', m_callingApInvocationId);
      unsigned sizeUpToEpsemControl = m_outgoingApdu.GetHeaderSize();
      if ( !m_securityKeyIdAndInitializationVectorWereSent )
      {
         m_securityKeyIdAndInitializationVectorWereSent = true;
         char callingAuthenticationValueElement[17] = "\xAC\x0F\xA2\x0D\xA0\x0B\xA1\x09\x80\x01\x00\x81\x04"; // plus four extra bytes
         callingAuthenticationValueElement[10] = (char)(Muint8)m_securityKeyId; // assign <key-id>
         memcpy(&callingAuthenticationValueElement[13], &m_initializationVector, sizeof(Muint32));
         m_canonifiedCleartext.Append(callingAuthenticationValueElement, sizeof(callingAuthenticationValueElement));
         m_canonifiedCleartext.Append(m_outgoingApdu.GetTotalPtr(), sizeUpToEpsemControl);
         m_outgoingApdu.Prepend(callingAuthenticationValueElement, sizeof(callingAuthenticationValueElement));
      }
      else
         m_canonifiedCleartext.Append(m_outgoingApdu.GetTotalPtr(), sizeUpToEpsemControl);

      DoAppendAbsoluteUidIfPresent(m_canonifiedCleartext, '\xA6', m_applicationContext, m_callingApTitle);
      m_canonifiedCleartext.Append((char)m_securityKeyId);
      m_canonifiedCleartext.Append((const char*)&m_initializationVector, sizeof(m_initializationVector));

      Muint32 mac;
      if ( securityMode == SecurityClearTextWithAuthentication )
      {
         M_ASSERT(securityMode == SecurityClearTextWithAuthentication);
         m_canonifiedCleartext.Append(m_outgoingApdu.GetBodyPtr(), m_outgoingApdu.GetBodySize());
         mac = m_eax.EaxAuthenticate(m_canonifiedCleartext.AccessAllBytes()); // this also adds MAC at the end
      }
      else
      {
         M_ASSERT(securityMode >= SecurityCipherTextWithAuthentication); // or possibly SecurityCompressedCipherTextWithAuthentication
         mac = m_eax.EaxEncryptBuffer(m_canonifiedCleartext.GetTotalPtr(), m_canonifiedCleartext.GetTotalSize(), m_outgoingApdu.GetBodyPtr(), m_outgoingApdu.GetBodySize());
      }
      m_outgoingApdu.Append((const char*)&mac, sizeof(mac));
   }

   m_outgoingApdu.PrependUnsigned('\xA8', m_callingApInvocationId);
   if ( m_callingAeQualifier != -1 )
      m_outgoingApdu.PrependUnsigned('\xA7', m_callingAeQualifier);
   m_outgoingApdu.PrependUidIfPresent('\xA6', m_callingApTitle);
   if ( m_incomingCallingApInvocationIdPresent )
      m_outgoingApdu.PrependUnsigned('\xA4', m_incomingCallingApInvocationId);
   m_outgoingApdu.PrependUidIfPresent('\xA2', m_calledApTitle);
   m_outgoingApdu.PrependUidIfPresent('\xA1', m_applicationContext);

   m_outgoingApdu.PrependIsoLength(m_outgoingApdu.GetTotalSize());
   m_outgoingApdu.Prepend('\x60');

   unsigned totalSize = m_outgoingApdu.GetTotalSize();
   if ( totalSize > m_effectiveMaximumApduSizeOutgoing )
   {
      char buff[4];
      MToBigEndianUINT32(m_effectiveMaximumApduSizeOutgoing, buff);
      MEC12NokResponse::ThrowWithParameters(MEC12NokResponse::RESPONSE_RQTL, MByteString(buff, sizeof(buff)));
      M_ENSURED_ASSERT(0);
   }

   DoWriteApdu();
}

void MProtocolC1222::ReceiveStart()
{
#if !M_NO_SOCKETS_UDP
   for ( int udpRetries = m_linkLayerRetries; ; --udpRetries )
   {
      try
      {
#endif
         DoReceiveStartHeader();

#if !M_NO_VERBOSE_ERROR_INFORMATION

         // Now verify everything about what we have received as a response to our request
         //
         MConstLocalChars tamperingMessage = NULL;
         if ( !m_incomingCalledApInvocationIdPresent )
            tamperingMessage = M_I("Incoming called invocation ID is not present, tampering is suspected");
         else if ( !m_incomingCallingApInvocationIdPresent )
            tamperingMessage = M_I("Incoming calling invocation ID is not present, tampering is suspected");
         else if ( m_incomingCalledApInvocationId != m_callingApInvocationId )
            tamperingMessage = M_I("Invocation ID mismatch, tampering is suspected");
         else if ( m_callingApTitle != m_incomingCalledApTitle || m_calledApTitle != m_incomingCallingApTitle )
            tamperingMessage = M_I("Ap title mismatch, tampering is suspected");

         if ( tamperingMessage != NULL )
         {
            MCOMException::Throw(MException::ErrorSecurity, MErrorEnum::PossibleTamperingDetected, tamperingMessage);
            M_ENSURED_ASSERT(0);
         }
#else
         if ( !m_incomingCalledApInvocationIdPresent ||
              !m_incomingCallingApInvocationIdPresent ||
              m_incomingCalledApInvocationId != m_callingApInvocationId ||
              m_callingApTitle != m_incomingCalledApTitle ||
              m_calledApTitle != m_incomingCallingApTitle )
         {
            MCOMException::Throw(MException::ErrorSecurity, MErrorEnum::PossibleTamperingDetected);
            M_ENSURED_ASSERT(0);
         }
#endif

#if !M_NO_SOCKETS_UDP
         break;
      }
      catch ( MException& ex )
      {
         MChannelSocketUdp* udpChannel = M_DYNAMIC_CAST(MChannelSocketUdp, m_channel);
         if ( udpChannel == NULL || udpRetries <= 0 )
            throw; // if this is not UDP channel or retries expired

         MProtocolLinkLayerWrapper::ThrowIfNotRetryable(ex);
         #if !M_NO_MCOM_MONITOR
            WriteToMonitor("Retrying APDU after ignoring error " + ex.AsString());
         #endif
         M_ASSERT(m_outgoingApdu.GetBodySize() > 0);
         DoWriteApdu();
         // and retry
      }
   }
#endif

   DoUpdateCallingApInvocationId(false);

   ProcessIncomingEPSEM();
   if ( m_sessionless && m_issueSecurityOnStartSession )
      ReceiveSecurity();
}

void MProtocolC1222::DoReceiveStartHeader()
{
   DoResetIncomingProperties();
   ReadApdu();
   DoParseStartHeader();
}

void MProtocolC1222::DoParseStartHeader()
{
#if !M_NO_MCOM_MONITOR
   m_logHeaderChar = '<';
#endif
   char start = m_applicationLayerReader.ReadByte();
   if ( start != '\x60' )
   {
      MCOMException::Throw(M_CODE_STR(M_ERR_DID_NOT_GET_A_VALID_BYTE_AMONG_D1_GARBAGE_BYTES_LAST_ONE_HAD_CODE_X2, M_I("Did not see C12.22 APDU start")));
      M_ENSURED_ASSERT(0);
   }
   unsigned isoLength = m_applicationLayerReader.ReadIsoLength();
   unsigned remainingLength = m_applicationLayerReader.GetRemainingReadSize(); // this call should follow the above one
   if ( isoLength != remainingLength )
   {
      MCOMException::Throw(M_CODE_STR(M_ERR_DID_NOT_GET_A_VALID_BYTE_AMONG_D1_GARBAGE_BYTES_LAST_ONE_HAD_CODE_X2, M_I("The APDU length is inconsistent with APDU size")));
      M_ENSURED_ASSERT(0);
   }
   while ( m_applicationLayerReader.GetRemainingReadSize() > 0 )
   {
      const char* tagStart = m_applicationLayerReader.GetReadPtr();
      Muint8 elementType = m_applicationLayerReader.ReadByte();
      unsigned elementLength = m_applicationLayerReader.ReadIsoLength();
      switch ( elementType )
      {
      case (Muint8)'\xA1':
         DoGetUid(M_OPT_STR("application-context"), '\xA1', m_incomingApplicationContext);
         break;
      case (Muint8)'\xA2':
         DoGetUid(M_OPT_STR("called-AP-title"), '\xA2', m_incomingCalledApTitle);
         break;
      case (Muint8)'\xA4':
         m_incomingCalledApInvocationId = DoGetInteger(M_OPT_STR("called-AP-invocation-id"), '\xA4');
         m_incomingCalledApInvocationIdPresent = true;
         break;
      case (Muint8)'\xA6':
         DoGetUid(M_OPT_STR("calling-AP-title"), '\xA6', m_incomingCallingApTitle);
         break;
      case (Muint8)'\xA7':
         m_incomingCallingAeQualifier = DoGetInteger(M_OPT_STR("calling-ae-qualifier"), '\xA7');
         break;
      case (Muint8)'\xA8':
         m_incomingCallingApInvocationId = DoGetInteger(M_OPT_STR("calling-AP-invocation-id"), '\xA8');
         m_incomingCallingApInvocationIdPresent = true;
         break;
      case (Muint8)'\xAC':  // key id and init vector
         {
            m_securityKeyIdAndInitializationVectorWereReceived = true;
            char buff [ 15 ];
            m_applicationLayerReader.ReadBuffer(buff, sizeof(buff));
            if ( memcmp(buff, "\xA2\x0D\xA0\x0B\xA1\x09\x80\x01", 8) != 0 || buff[9] != '\x81' || buff[10] != '\x04' )
            {
               DoThrowBadACSEResponse('\xAC');
               M_ENSURED_ASSERT(0);
            }
            m_incomingSecurityKeyId = buff[8];
            memcpy(&m_incomingInitializationVector, &buff[11], sizeof(Muint32));
            #if !M_NO_MCOM_MONITOR
               DoSendACSECallingAuthenticationToMonitor(m_incomingSecurityKeyId, m_incomingInitializationVector);
            #endif
            break;
         }
      case (Muint8)'\xBE': // Application data element
         {
            // Have to delay size check to the time header is read
            if ( isoLength > m_effectiveMaximumApduSizeIncoming )
            {
               char buff[4];
               MToBigEndianUINT32(m_effectiveMaximumApduSizeIncoming, buff);
               MEC12NokResponse::ThrowWithParameters(MEC12NokResponse::RESPONSE_RSTL, MByteString(buff, sizeof(buff)));
               M_ENSURED_ASSERT(0);
            }

            Muint8 uiExternalByte = m_applicationLayerReader.ReadByte();
            if ( uiExternalByte != Muint8('\x28') ) // only 0x28 shall be read
            {
               DoThrowBadACSEResponse('\xBE');
               M_ENSURED_ASSERT(0);
            }
            unsigned externalUserInformationLen = m_applicationLayerReader.ReadIsoLength();
            M_USED_VARIABLE(externalUserInformationLen); // ignore for now

            Muint8 nextByte = m_applicationLayerReader.ReadByte();
            if ( nextByte == Muint8('\x02') ) // user information indirect reference
            {
               unsigned indirectReferenceLen = m_applicationLayerReader.ReadIsoLength();
               m_applicationLayerReader.IgnoreBytes(indirectReferenceLen); // ignore these bytes
               nextByte = m_applicationLayerReader.ReadByte();
            }
            if ( nextByte != Muint8('\x81') ) // user information octet string
            {
               DoThrowBadACSEResponse('\xBE');
               M_ENSURED_ASSERT(0);
            }
            unsigned userInformationLen = m_applicationLayerReader.ReadIsoLength();
            if ( userInformationLen == 0 || m_applicationLayerReader.GetRemainingReadSize() != userInformationLen )
            {
               DoThrowBadACSEResponse('\xBE');
               M_ENSURED_ASSERT(0);
            }
            m_incomingEpsemControl = m_applicationLayerReader.ReadByte();
            const char* epsemStart = m_applicationLayerReader.GetReadPtr();
            m_incomingSecurityMode = (SecurityModeEnum)((m_incomingEpsemControl >> 2) & 0x3);
            m_incomingResponseControl = ResponseControlEnum(m_incomingEpsemControl & 0x3);

            m_canonifiedCleartext.Clear();
            if ( m_incomingSecurityMode != SecurityClearText )
            {
               if ( m_applicationLayerReader.GetRemainingReadSize() < 4 ) // otherwise identification will not be successful
               {
                  MAesEax::ThrowValidationError();
                  M_ENSURED_ASSERT(0);
               }
               m_canonifiedCleartext.AppendUidIfPresent('\xA1', m_incomingApplicationContext);
               DoAppendAbsoluteUidIfPresent(m_canonifiedCleartext, '\xA2', m_incomingApplicationContext, m_incomingCalledApTitle);
               if ( m_incomingCalledApInvocationIdPresent )
                  m_canonifiedCleartext.AppendUnsigned('\xA4', m_incomingCalledApInvocationId);
               if ( m_incomingCallingAeQualifier != -1 )
                  m_canonifiedCleartext.AppendUnsigned('\xA7', m_incomingCallingAeQualifier);
               if ( m_incomingCallingApInvocationIdPresent )
                  m_canonifiedCleartext.AppendUnsigned('\xA8', m_incomingCallingApInvocationId);
               if ( m_securityKeyIdAndInitializationVectorWereReceived )
                  DoAppendCallingInvocation(m_canonifiedCleartext, m_incomingSecurityKeyId, m_incomingInitializationVector);
               m_canonifiedCleartext.AccessAllBytes().append(tagStart, epsemStart); // store the whole user information here
               DoAppendAbsoluteUidIfPresent(m_canonifiedCleartext, '\xA6', m_incomingApplicationContext, m_incomingCallingApTitle);
               m_canonifiedCleartext.Append((char)m_incomingSecurityKeyId);
               m_canonifiedCleartext.Append((const char*)&m_incomingInitializationVector, sizeof(m_incomingInitializationVector));
            }
            return; // ended reading the packet
         }
      default:
         m_applicationLayerReader.IgnoreBytes(elementLength); // skip bytes
         break;
      }
   }
}

void MProtocolC1222::ProcessIncomingEPSEM()
{
   if ( m_securityKeyIdAndInitializationVectorWereReceived && m_securityKeyId != m_incomingSecurityKeyId )
   {
      MCOMException::Throw(MException::ErrorMeter, M_CODE_STR(M_ERR_PROTOCOL_IMPLEMENTATION_MISMATCH, "Key ID returned by device is different from current key ID"));
      M_ENSURED_ASSERT(0);
   }

   if ( m_incomingSecurityMode != SecurityClearText )
   {
      DoInitializeEax(m_callingApTitle);
      unsigned epsemSize = m_applicationLayerReader.GetRemainingReadSize() - 4; // the check that this is bigger than 4 is done previously
      if ( m_incomingSecurityMode == SecurityCipherTextWithAuthentication
         )
      {
         char* epsemPtr = m_applicationLayerReader.GetReadPtr();
         Muint32 mac = m_eax.EaxDecryptBuffer(m_canonifiedCleartext.GetTotalPtr(), m_canonifiedCleartext.GetTotalSize(), epsemPtr, epsemSize);
         if ( memcmp(&mac, epsemPtr + epsemSize, sizeof(mac)) != 0 )
         {
            MAesEax::ThrowValidationError();
            M_ENSURED_ASSERT(0);
         }

         {
            m_incomingApdu.Resize(m_incomingApdu.GetTotalSize() - 4);
         }
         m_applicationLayerReader.SetEndPosition(m_applicationLayerReader.GetEndPosition() - 4);

         #if !M_NO_MCOM_MONITOR
            DoSendEpsemToMonitor(m_applicationLayerReader.GetReadPtr(), m_applicationLayerReader.GetRemainingReadSize());
         #endif
      }
      else
      {
         M_ASSERT(m_incomingSecurityMode == SecurityClearTextWithAuthentication);
         unsigned appLayerSize = m_applicationLayerReader.GetRemainingReadSize() - 4;
         m_canonifiedCleartext.Append(m_applicationLayerReader.GetReadPtr(), appLayerSize);
         m_applicationLayerReader.IgnoreBytes(appLayerSize);
         Muint32 mac = m_eax.EaxAuthenticateBuffer(m_canonifiedCleartext.GetTotalPtr(), m_canonifiedCleartext.GetTotalSize());
         Muint32 macInPacket;
         m_applicationLayerReader.ReadBuffer((char*)&macInPacket, sizeof(macInPacket));
         if ( mac != macInPacket )
         {
            MAesEax::ThrowValidationError();
            M_ENSURED_ASSERT(0);
         }
         m_incomingApdu.Resize(m_incomingApdu.GetTotalSize() - 4);
         m_applicationLayerReader.SetReadPosition(m_incomingApdu.GetTotalSize() - epsemSize);
         m_applicationLayerReader.SetEndPosition(m_incomingApdu.GetTotalSize());
      }
   }

   if ( (m_incomingEpsemControl & 0x10) != 0 ) // ED_CLASS_INCLUDED = 1
   {
      m_applicationLayerReader.ReadBytes(4, m_incomingEdClass);
      MAlgorithm::InplaceTrimRight(m_incomingEdClass);
   }

   m_incomingEpsemSize = m_applicationLayerReader.GetEndPosition() - m_applicationLayerReader.GetReadPosition();
   // Now we are ready to parse EPSEM, the pointer is at the start
}

bool MProtocolC1222::SendEndReceiveStart()
{
   SendEnd();
   if ( m_responseControl == ResponseControlNever )
      return false;
   try
   {
      ReceiveStart();
   }
   catch ( MEChannelReadTimeout& ex )
   {
      if ( m_responseControl == ResponseControlOnException && ex.GetBytesRead() == 0 )
         return false;
      throw; // otherwise throw
   }
   return true;
}

void MProtocolC1222::ReceiveEnd()
{
   if ( m_securityMode > SecurityClearText && m_incomingSecurityMode == SecurityClearText ) // if the incoming security mode is less secure, and there were no errors reported while processing it...
   {
      MCOMException::Throw(MException::ErrorSecurity, M_CODE_STR(MErrorEnum::PossibleTamperingDetected, M_I("Insecure response on a secure request, tampering is suspected")));
      M_ENSURED_ASSERT(0);
   }
}

void MProtocolC1222::SendService(Muint8 command)
{
   char buff [ 2 ];
   buff[0] = '\1'; // length
   buff[1] = (char)command;
   m_outgoingApdu.Append(buff, sizeof(buff));
}

void MProtocolC1222::SendServiceWithData(Muint8 command, const MByteString& data)
{
   char buff [ 8 ];
   unsigned len = static_cast<unsigned>(data.size()) + 1;// +1 to add command byte size
   unsigned dataLen = MIso8825::EncodeLengthIntoBuffer(len, buff);
   M_ASSERT(dataLen <= 5);
   buff[dataLen++] = command;
   m_outgoingApdu.Append(buff, dataLen);
   m_outgoingApdu.Append(data);
}

unsigned MProtocolC1222::ReceiveServiceLength()
{
   m_applicationLayerReader.SetEndPosition(m_incomingApdu.GetTotalSize()); // and continue reading
   if ( m_applicationLayerReader.GetRemainingReadSize() == 0 )
      return 0u;
   unsigned length = m_applicationLayerReader.ReadIsoLength();
   if ( length > 0 )
      m_applicationLayerReader.SetEndPosition(m_applicationLayerReader.GetReadPosition() + length);
   return length;
}

Muint8 MProtocolC1222::ReceiveServiceCodeIgnoreLength()
{
   unsigned length = ReceiveServiceLength();
   if ( length == 0 ) // otherwise, "" will be returned, means no more responses.
   {
      MCOMException::CheckIfExpectedDataSizeDifferent(length, 1); // surely throws
      M_ENSURED_ASSERT(0);
   }
   return ReceiveServiceCode();
}

void MProtocolC1222::SendLogon()
{
   DoResetSessionSpecificProperties();

   M_ASSERT(m_user.size() <= 10); // ensured by the property setter
   M_ASSERT(m_sessionIdleTimeout <= 0xFFFF);

   // No special alignment necessary - the structure is well aligned already
   struct LogonRequestStruct
   {
      Muint16 m_userId;
      char    m_user [ 10 ];
      Muint16 m_sessionTimeout;
   } logon;
   logon.m_userId = MToBigEndianUINT16(m_userId);
   logon.m_sessionTimeout = MToBigEndianUINT16(m_sessionIdleTimeout);
   memset(logon.m_user, ' ', sizeof(logon.m_user));
   memcpy(logon.m_user, m_user.data(), m_user.size());

   SendServiceWithData('\x50', MByteString((const char*)&logon, sizeof(logon)));
}

void MProtocolC1222::ReceiveLogon()
{
   ReceiveServiceCodeIgnoreLength(); // signal error, if any
   m_negotiatedSessionIdleTimeout = (unsigned)ReceiveServiceByte();
   m_negotiatedSessionIdleTimeout <<= 8;
   m_negotiatedSessionIdleTimeout |= (unsigned)ReceiveServiceByte();
   m_negotiatedSessionIdleTimeoutPresent = true;
   m_isInSession = true;
}

void MProtocolC1222::Logon()
{
   MProtocolServiceWrapper wrapper(m_wrapperProtocol, M_OPT_STR("Logon"), MProtocolServiceWrapper::ServiceNotQueueable | MProtocolServiceWrapper::ServiceStartsSessionKeeping);
   for ( unsigned appRetryCount = m_applicationLayerRetries; ; --appRetryCount )
   {
      try
      {
         SendStart();
         SendLogon();
         if ( SendEndReceiveStart() )
         {
            ReceiveLogon();
            ReceiveEnd();
            if ( !m_sessionless )
               m_callingApInvocationId = 0; // according to specification, when Logon is called directly
         }
         break;
      }
      catch ( MEC12NokResponse& ex )
      {
         MEC12NokResponse::ResponseCode code = (MEC12NokResponse::ResponseCode)ex.GetResponseCode();
         if ( code != MEC12NokResponse::RESPONSE_BSY && code != MEC12NokResponse::RESPONSE_DNR )
            throw;

         MProtocolServiceWrapper::StaticNotifyOrThrowRetry(m_wrapperProtocol, ex, appRetryCount);
         Sleep(m_applicationLayerRetryDelay); // sleep and retry
      }
      catch ( MException& ex )
      {
         wrapper.HandleFailureAndRethrow(ex);
         M_ENSURED_ASSERT(0);
      }
   }
}

   static MByteString DoGetEffectivePassword(MProtocolC1222* self)
   {
      MByteString password;
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
      int entry = self->GetPasswordListSuccessfulEntry();
      if ( entry >= 0 && self->GetPasswordList().size() >= (size_t)entry + 1 )
         password = self->GetPasswordList()[entry];
      else
#endif // !M_NO_MCOM_PASSWORD_AND_KEY_LIST
      {
         password = self->GetPassword();
      }

      unsigned diff = 20 - unsigned(password.size());
      if ( diff > 0 )
         password.append(diff, ' '); // fill the rest of the password with blanks
      M_ASSERT(password.size() == 20);
      return password;
   }

void MProtocolC1222::SendSecurity()
{
   MByteString password = DoGetEffectivePassword(this);
   M_ASSERT(password.size() == 20);

   if ( m_sessionless ) // when sessionless, password shall be followed by USER_ID, otherwise USER_ID is provided by Logon service
   {
      MByteString body = password;
      Muint16 userId = MToBigEndianUINT16(m_userId);
      body.append((const char*)&userId, sizeof(userId));
      SendServiceWithData('\x51', body);
   }
   else
      SendServiceWithData('\x51', password);
}

void MProtocolC1222::ReceiveSecurity()
{
   try
   {
      ReceiveServiceCodeIgnoreLength(); // signal error, if any
   }
   catch ( MEC12NokResponse& ex )
   {
      ex.SetKind(MException::ErrorSecurity); // override the type for C12 error
      throw;
   }
}

void MProtocolC1222::DoResetNegotiatedMaximumApduSizes()
{
   ChangeNegotiatedMaximumApduSizeIncoming(m_maximumApduSizeIncoming != 0 ? m_maximumApduSizeIncoming : m_maximumApduSizeOutgoing);
   ChangeNegotiatedMaximumApduSizeOutgoing(m_maximumApduSizeOutgoing);
}

void MProtocolC1222::DoResetSessionSpecificProperties()
{
   m_isInSession = false;
   m_incomingCallingApInvocationIdPresent = false;
   m_incomingCallingApInvocationId = 0;
   DoUpdateCallingApInvocationId(true);
   m_incomingInitializationVector = 0;
   m_incomingSecurityKeyId = 0;
   m_securityKeyIdAndInitializationVectorWereSent = false;
   DoResetNegotiatedMaximumApduSizes();
}

void MProtocolC1222::DoResetIncomingProperties()
{
   // These two are per-session, not reset in DoResetIncomingProperties:
   //     m_incomingSecurityKeyId(0)
   //     m_incomingInitializationVector(0)

   m_incomingEdClass.clear();
   m_incomingApplicationContext.clear();
   m_incomingResponseControl = ResponseControlAlways;
   m_incomingCalledApTitle.clear();
   m_incomingCallingApTitle.clear();
   m_incomingCalledApInvocationId = 0;
   m_incomingCalledApInvocationIdPresent = false;
   m_incomingCallingApInvocationId = 0;
   m_incomingCallingApInvocationIdPresent = false;
   m_incomingSecurityMode = SecurityClearText;
   m_incomingCallingAeQualifier = -1;
   m_incomingEpsemSize = 0;
   m_securityKeyIdAndInitializationVectorWereReceived = false;
}

void MProtocolC1222::SendTableRead(MCOMNumberConstRef number)
{
   Muint16 num = MToBigEndianUINT16(DoConvertNumberToUnsigned(number));
   MByteString request((const char*)&num, 2);
   SendServiceWithData('\x30', request);
}

MByteString MProtocolC1222::ReceiveTableRead(MCOMNumberConstRef)
{
   MByteString result;
   ReceiveServiceCodeIgnoreLength(); // signal error, if any
   DoAppendTableReadResponse(result);
   return result;
}

void MProtocolC1222::SendTableReadPartial(MCOMNumberConstRef number, int offset, int length)
{
   char buff [ 7 ]; // hard-code the packet size
   MToBigEndianUINT16(DoConvertNumberToUnsigned(number), buff);
   MToBigEndianUINT24(offset,                           buff + 2);
   MToBigEndianUINT16(length,                           buff + 5);
   MByteString request(buff, 7);
   SendServiceWithData('\x3F', request);
}

MByteString MProtocolC1222::ReceiveTableReadPartial(MCOMNumberConstRef, int, int)
{
   MByteString result;
   ReceiveServiceCodeIgnoreLength(); // signal error, if any
   DoAppendTableReadResponse(result);
   return result;
}

#if !M_NO_MCOM_IDENTIFY_METER
MStdString MProtocolC1222::DoIdentifyMeter(bool sessionIsStarted, TableRawDataVector* tablesRead)
{
   MStdString result;
   if ( sessionIsStarted )
      result = MProtocolC12::DoIdentifyMeter(true, tablesRead);
   else
   {
      MValueSavior<bool> sessionlessSavior(&m_sessionless, true); // The whole identify is always done sessionless
      MValueSavior<bool> securitySavior(&m_issueSecurityOnStartSession, true); // Security is always issued
      ApplyChannelParameters(); // need to do so, case if the session is started several times without reconnecting
      StartSession();
      result = MProtocolC12::DoIdentifyMeter(true, tablesRead);
      EndSession();
   }
   return result;
}
#endif

   static void DoAddTableDataChunk(MByteString& request, const MByteString& dataChunk)
   {
      unsigned size = M_64_CAST(unsigned, dataChunk.size());
      M_ASSERT(size <= 0xFFFFu);
      request += (char)Muint8(size >> 8);
      request += (char)Muint8(size);
      request += dataChunk;
      request += (char)(unsigned char)MProtocolC1222::StaticCalculateChecksum(dataChunk);
   }

   static void DoAddTableData(MByteString& request, const MByteString& data)
   {
      unsigned length = M_64_CAST(unsigned, data.size());
      for ( unsigned pos = 0; pos != length; )
      {
         unsigned bytesToWrite = length - pos;
         if ( bytesToWrite > 0xFFFF )
            bytesToWrite = 0xFFFF;
         MByteString::const_iterator posIterator = data.begin() + pos;
         DoAddTableDataChunk(request, MByteString(posIterator, posIterator + bytesToWrite));
         pos += bytesToWrite;
      }
      if ( (length % 0xFFFF) == 0 ) // case when we have to add one extra empty chunk
         DoAddTableDataChunk(request, MByteString()); // last data has to be present in such case
   }

void MProtocolC1222::SendTableWrite(MCOMNumberConstRef number, const MByteString& data)
{
   Muint16 num = MToBigEndianUINT16(DoConvertNumberToUnsigned(number));
   MByteString request((const char*)&num, 2);
   DoAddTableData(request, data);
   SendServiceWithData('\x40', request);
}

void MProtocolC1222::ReceiveTableWrite(MCOMNumberConstRef, const MByteString&)
{
   ReceiveServiceCodeIgnoreLength(); // signal error, if any
}

void MProtocolC1222::SendTableWritePartial(MCOMNumberConstRef number, const MByteString& data, int offset)
{
   char buff [ 5 ]; // hard-code the packet size
   MToBigEndianUINT16(DoConvertNumberToUnsigned(number), buff);
   MToBigEndianUINT24(offset,                           buff + 2);
   MByteString request((const char*)buff, 5);
   DoAddTableData(request, data);
   SendServiceWithData('\x4F', request);
}

void MProtocolC1222::ReceiveTableWritePartial(MCOMNumberConstRef, const MByteString&, int)
{
   ReceiveServiceCodeIgnoreLength(); // signal error, if any
}

void MProtocolC1222::FunctionExecuteSend(MCOMNumberConstRef number)
{
   DoFunctionSend(number, MByteString(), false);
}

void MProtocolC1222::FunctionExecuteReceive(MCOMNumberConstRef number)
{
   DoFunctionReceive(number, MByteString(), false);
}

void MProtocolC1222::FunctionExecuteRequestSend(MCOMNumberConstRef number, const MByteString& request)
{
   DoFunctionSend(number, request, false);
}

void MProtocolC1222::FunctionExecuteRequestReceive(MCOMNumberConstRef number, const MByteString& request)
{
   DoFunctionReceive(number, request, false);
}

void MProtocolC1222::FunctionExecuteResponseSend(MCOMNumberConstRef number)
{
   DoFunctionSend(number, MByteString(), true);
}

MByteString MProtocolC1222::FunctionExecuteResponseReceive(MCOMNumberConstRef number)
{
   return DoFunctionReceive(number, MByteString(), true);
}

void MProtocolC1222::FunctionExecuteRequestResponseSend(MCOMNumberConstRef number, const MByteString& request)
{
   DoFunctionSend(number, request, true);
}

MByteString MProtocolC1222::FunctionExecuteRequestResponseReceive(MCOMNumberConstRef number, const MByteString& request)
{
   return DoFunctionReceive(number, request, true);
}

void MProtocolC1222::DoFunctionSend(MCOMNumberConstRef number, const MByteString& request, bool expectResponse)
{
   unsigned num = DoConvertNumberToUnsigned(number, 0x100FFu); // allow for protocol services tagged with 0x10000
   if ( num & 0x10000 ) // Protocol Services called
   {
      num &= ~0x10000;
      MENumberOutOfRange::CheckInteger(0, 0xFF, num);
      if ( request.empty() )
         SendService(num);
      else
         SendServiceWithData(num, request);
   }
   else // Meter Procedure
   {
      Muint16 codeAndFlags = m_meterIsLittleEndian ? MToLittleEndianUINT16(num) : MToBigEndianUINT16(num);
      MByteString table7((const char*)&codeAndFlags, 2);
      table7 += static_cast<char>(m_procedureSequenceNumber);
      table7 += request;
      SendTableWrite(7, table7);
#if !M_NO_VARIANT
      bool doSkip = DoHaveToSkipReadFunctionResponseTable8(number.AsUInt(), request, expectResponse);
#else
      bool doSkip = DoHaveToSkipReadFunctionResponseTable8(number, request, expectResponse);
#endif
      if ( !doSkip )
         SendTableRead(8);
   }
}

MByteString MProtocolC1222::DoFunctionReceive(MCOMNumberConstRef number, const MByteString& request, bool expectResponse)
{
   MByteString response;
   unsigned num = DoConvertNumberToUnsigned(number, 0x100FFu); // allow for protocol services tagged with 0x10000
   if ( num & 0x10000 ) // Protocol Services called
   {
      num &= ~0x10000;
      MENumberOutOfRange::CheckInteger(0, 0xFF, num);
      ReceiveServiceCodeIgnoreLength(); // signal error, if any
      m_applicationLayerReader.ReadRemainingBytes(response);
   }
   else // Meter Procedure
   {
      ReceiveTableWrite(7, request);
#if !M_NO_VARIANT
      bool doSkip = DoHaveToSkipReadFunctionResponseTable8(number.AsUInt(), request, expectResponse);
#else
      bool doSkip = DoHaveToSkipReadFunctionResponseTable8(number, request, expectResponse);
#endif
      if ( !doSkip )
      {
         response = ReceiveTableRead(8);
         DoHandleFunctionResponseTable8Read(response);
      }
   }
   return response;
}

void MProtocolC1222::DoStartSession()
{
   DoResetSessionSpecificProperties();

#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   if ( !m_passwordList.empty() )
      DoCheckNotOneWay(M_OPT_STR("PasswordList"));
   if ( !m_securityKeyList.empty() )
      DoCheckNotOneWay(M_OPT_STR("SecurityKeyList"));
#endif

   if ( !m_sessionless ) // session mode
   {

#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST

      if ( m_securityMode != SecurityClearText )
      {
         if ( !m_securityKeyList.empty() && m_securityKeyListSuccessfulEntry < 0 )
         {
            MValueSavior<MByteString> keySavior(&m_securityKey);
            M_ASSERT(m_securityKeyListSuccessfulEntry == -1);
            int num = static_cast<int>(m_securityKeyList.size());
            for ( int i = 0; i < num; ++i )
            {
               SetSecurityKey(m_securityKeyList[i]);
               try
               {
                  Logon();
                  m_securityKeyListSuccessfulEntry = i;
                  break; // success
               }
               catch ( MEC12NokResponse& ex )
               {
                  if ( ex.GetResponseCode() != MEC12NokResponse::RESPONSE_SME || i == num - 1 ) // rethrow the exception, if this is the last entry in the password list
                     throw;  // failure will be notified later
               }
            }
         }
         else
            Logon();
      }
      else
         Logon();
      
      m_callingApInvocationId = 0; // according to specification
      if ( m_issueSecurityOnStartSession )
      {
         try
         {
            if ( m_passwordListSuccessfulEntry < 0 )
               FullLogin();
            else
            {
               MProtocolServiceWrapper wrapper(this, M_OPT_STR("Security"), MProtocolServiceWrapper::ServiceNotQueueable);
               try
               {
                  DoTryPasswordEntry(m_passwordList[m_passwordListSuccessfulEntry]);
               }
               catch ( MException& ex )
               {
                  wrapper.HandleFailureAndRethrow(ex);
                  M_ENSURED_ASSERT(0);
               }
            }
         }
         catch ( MEC12NokResponse& ex )
         {
            if ( m_endSessionOnApplicationLayerError )
            {
               MEC12NokResponse::ResponseCode code = ex.GetResponseCode();
               if ( code != MEC12NokResponse::RESPONSE_SME && code != MEC12NokResponse::RESPONSE_ISSS && code != MEC12NokResponse::RESPONSE_RNO )
                  DoEndSessionOnApplicationLayerError(false);
            }
            throw;
         }
      }

#else // !M_NO_MCOM_PASSWORD_AND_KEY_LIST

      Logon();

      m_callingApInvocationId = 0; // according to specification
      if ( m_issueSecurityOnStartSession )
      {
         try
         {
            FullLogin();
         }
         catch ( MEC12NokResponse& ex )
         {
            if ( m_endSessionOnApplicationLayerError )
            {
               MEC12NokResponse::ResponseCode code = ex.GetResponseCode();
               if ( code != MEC12NokResponse::RESPONSE_SME && code != MEC12NokResponse::RESPONSE_ISSS && code != MEC12NokResponse::RESPONSE_RNO )
                  DoEndSessionOnApplicationLayerError(false);
            }
            throw;
         }
      }

#endif // !M_NO_MCOM_PASSWORD_AND_KEY_LIST

   }

}

void MProtocolC1222::DoSendStartSession()
{
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   if ( !m_passwordList.empty() )
      DoCheckNotOneWay(M_OPT_STR("PasswordList"));
   if ( !m_securityKeyList.empty() )
      DoCheckNotOneWay(M_OPT_STR("SecurityKeyList"));
#endif

   if ( !m_sessionless )
   {
      SendLogon();
      if ( m_issueSecurityOnStartSession )
         SendSecurity(); // in m_sessionless case security is sent on StartSession
   }

   m_callingApInvocationId = 0; // according to specification
}

void MProtocolC1222::DoReceiveStartSession()
{
   if ( !m_sessionless )
   {
      ReceiveLogon();

      // m_callingApInvocationId = 0; // according to specification, when Logon is called directly

      if ( m_issueSecurityOnStartSession )
         ReceiveSecurity();
   }
}

void MProtocolC1222::DoSendEndSession()
{
   if ( !m_sessionless )
   {
      if ( m_issueTerminateOnEndSession )
      {
         // send terminate request
         SendService('\x21');
      }
      else
      {
         // send logoff request
         SendService('\x52');
      }
   }
}

void MProtocolC1222::DoReceiveEndSession()
{
   if ( !m_sessionless )
   {
      // receive terminate or logoff response
      ReceiveServiceCodeIgnoreLength(); // signal error, if any
      m_isInSession = false;
   }
}

void MProtocolC1222::DoEndSession()
{
   if ( !m_sessionless )
   {
      if ( m_issueTerminateOnEndSession )
         Terminate();
      else
         Logoff();
   }
   DoResetSessionSpecificProperties();
}

void MProtocolC1222::DoMeterProcedure(unsigned number, const MByteString& request, MByteString& response, bool expectResponse)
{
   if ( m_sessionless && !m_oneServicePerApdu )
   {
      QFunctionExecuteRequestResponse(number, request, 0); // spawn a little new queue
      QCommit(false);
      response = QGetFunctionData(number, 0);
   }
   else
      MProtocolC12::DoMeterProcedure(number, request, response, expectResponse);
}

void MProtocolC1222::DoApplicationLayerRequestWithCurrentPassword(char command, const MByteString* request, unsigned flags)
{
   for ( unsigned appRetryCount = m_applicationLayerRetries; ; --appRetryCount )
   {
      try
      {

         SendStart();


         if ( request == NULL )
            SendService(command);
         else
            SendServiceWithData(command, *request);
         SendEnd();

         if ( m_responseControl != ResponseControlAlways )
            break; // success, no app-layer read after one way communication

         ReceiveStart();


         ReceiveServiceCodeIgnoreLength(); // signal error, if any
         ReceiveEnd();
         break; // success
      }
      catch ( MEChannelReadTimeout& ex )
      {
         if ( (m_responseControl == ResponseControlOnException) && ex.GetBytesRead() == 0 )
            break; // success, no exception
         throw; // otherwise throw
      }
      catch ( MEC12NokResponse& ex )
      {
         MEC12NokResponse::ResponseCode code = (MEC12NokResponse::ResponseCode)ex.GetResponseCode();
         if ( code == MEC12NokResponse::RESPONSE_RQTL || code == MEC12NokResponse::RESPONSE_RSTL )
            ex.Rethrow(); // this type can only be handled on an upper level

         bool retryCondition = code == MEC12NokResponse::RESPONSE_BSY
                           || code == MEC12NokResponse::RESPONSE_DNR
                           ;
         if ( !m_sessionless )
            DoCheckCodeTerminateAndThrowOrNotify(ex, retryCondition, appRetryCount, false, m_wrapperProtocol);
         else
         {
            if ( !retryCondition ) // no need to retry, severe error
            {
               ex.Rethrow(); // to restart service
               M_ENSURED_ASSERT(0);
            }
            MProtocolServiceWrapper::StaticNotifyOrThrowRetry(m_wrapperProtocol, ex, appRetryCount);
            Sleep(m_applicationLayerRetryDelay); // sleep and retry
         }
      }
   }
}

#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST

void MProtocolC1222::DoApplicationLayerRequestIteratePasswordList(char command, const MByteString* request, unsigned flags)
{
   MValueSavior<MByteString> passwordSavior(&m_password);
   M_ASSERT(m_passwordListSuccessfulEntry == -1);
   int num = static_cast<int>(m_passwordList.size());
   for ( int i = 0; i < num; ++i )
   {
      MAes::AssignSecureData(m_password, m_passwordList[i]);
      try
      {
         DoApplicationLayerRequestWithCurrentPassword(command, request, flags);
         m_passwordListSuccessfulEntry = i;
         break; // success
      }
      catch ( MEC12NokResponse& ex )
      {
         if ( ex.GetKind() != MException::ErrorSecurity || (ex.GetResponseCode() != MEC12NokResponse::RESPONSE_ERR && ex.GetResponseCode() != MEC12NokResponse::RESPONSE_SME) || i == num - 1 )
            throw;  // failure will be notified later
      }
   }
}

#endif // !M_NO_MCOM_PASSWORD_AND_KEY_LIST

void MProtocolC1222::DoApplicationLayerRequest(char command, const MByteString* request, unsigned flags)
{
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   if ( m_sessionless )
   {
      if ( m_securityMode != SecurityClearText && !m_securityKeyList.empty() && m_securityKeyListSuccessfulEntry < 0 )
      {
         MValueSavior<MByteString> securityKeySavior(&m_securityKey);
         M_ASSERT(m_securityKeyListSuccessfulEntry == -1);
         int num = static_cast<int>(m_securityKeyList.size());
         for ( int i = 0; i < num; ++i )
         {
            SetSecurityKey(m_securityKeyList[i]);
            try
            {
               if ( m_issueSecurityOnStartSession && !m_passwordList.empty() && m_passwordListSuccessfulEntry < 0 ) // have to also iterate through passwords here
                  DoApplicationLayerRequestIteratePasswordList(command, request, flags);
               else
                  DoApplicationLayerRequestWithCurrentPassword(command, request, flags);
               m_securityKeyListSuccessfulEntry = i;
               break; // success
            }
            catch ( MEC12NokResponse& ex )
            {
               if ( ex.GetResponseCode() != MEC12NokResponse::RESPONSE_SME || i == num - 1 ) // rethrow the exception, if this is the last entry in the password list
                  throw;  // failure will be notified later
            }
         }
      }
      else // security key is known, have to try passwords only
      {
         if ( m_issueSecurityOnStartSession && !m_passwordList.empty() && m_passwordListSuccessfulEntry < 0 )
            DoApplicationLayerRequestIteratePasswordList(command, request, flags);
         else // security key and password are all known
            DoApplicationLayerRequestWithCurrentPassword(command, request, flags);
      }
   }
   else
#endif
   {
      DoApplicationLayerRequestWithCurrentPassword(command, request, flags);
   }
}

void MProtocolC1222::ApplyChannelParameters()
{
   MProtocolC12::ApplyChannelParameters();
   m_channel->SetIntercharacterTimeout(0); // intercharacter timeout has no effect
}

#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST

void MProtocolC1222::DoQCommit()
{
   if ( m_oneServicePerApdu )
      MProtocol::DoQCommit(); // easy way
   else if ( m_sessionless )
   {
      if ( m_securityMode != SecurityClearText && !m_securityKeyList.empty() && m_securityKeyListSuccessfulEntry < 0 )
      {
         MValueSavior<MByteString> keySavior(&m_securityKey);
         M_ASSERT(m_securityKeyListSuccessfulEntry == -1);
         int num = static_cast<int>(m_securityKeyList.size());
         for ( int i = 0; i < num; ++i )
         {
            SetSecurityKey(m_securityKeyList[i]);
            try
            {
               if ( m_issueSecurityOnStartSession && !m_passwordList.empty() && m_passwordListSuccessfulEntry < 0 )
                  DoQCommitIteratePasswordList();
               else
                  DoQCommitWithCurrentPassword(); // use PASSWORD property directly
               m_securityKeyListSuccessfulEntry = i;
               break; // success
            }
            catch ( MEC12NokResponse& ex )
            {
               if ( ex.GetResponseCode() != MEC12NokResponse::RESPONSE_SME || i == num - 1 ) // rethrow the exception, if this is the last entry in the password list
                  throw;  // failure will be notified later
            }
         }
      }
      else if ( m_issueSecurityOnStartSession && !m_passwordList.empty() && m_passwordListSuccessfulEntry < 0 )
         DoQCommitIteratePasswordList();
      else
         DoQCommitWithCurrentPassword(); // use PASSWORD property directly
   }
   else
      DoQCommitWithCurrentPassword();
}

void MProtocolC1222::DoQCommitIteratePasswordList()
{
   MValueSavior<MByteString> passwordSavior(&m_password);
   m_passwordListSuccessfulEntry = -1;
   int num = static_cast<int>(m_passwordList.size());
   for ( int i = 0; i < num; ++i )
   {
      MAes::AssignSecureData(m_password, m_passwordList[i]);
      try
      {
         DoQCommitWithCurrentPassword(); // use PASSWORD property directly
         M_ASSERT(m_passwordListSuccessfulEntry < 0);
         m_passwordListSuccessfulEntry = i;
         return; // success
      }
      catch ( MEC12NokResponse& ex )
      {
         if ( ex.GetKind() != MException::ErrorSecurity // rethrow if this is not security related, or
              || i == num - 1 )                         // if this is the last entry in the password list
         {
            throw;  // failure will be notified later
         }
      }
   }
}

#else // !M_NO_MCOM_PASSWORD_AND_KEY_LIST

void MProtocolC1222::DoQCommit()
{
   if ( m_oneServicePerApdu )
      MProtocol::DoQCommit();
   else
      DoQCommitWithCurrentPassword();
}

#endif

void MProtocolC1222::DoQCommitWithCurrentPassword()
{
#if !M_NO_PROGRESS_MONITOR
   double totalProgress = 0.0;
   for ( MCommunicationQueue::iterator i = m_queue.begin(); i != m_queue.end(); ++i )
      totalProgress += (*i)->GetProgressWeight();
   double progressDivisor = totalProgress / 100.0;

   MProgressAction* action = GetLocalProgressAction();
   action->SetProgress(0.0);
   double localActionWeight = 0.0;
#else
   #define DoQCommitSubrange(a,b,c,d) DoQCommitSubrange((a), (b))
#endif

   MCommunicationQueue::iterator subqueueStart = m_queue.begin();
   for ( MCommunicationQueue::iterator i = subqueueStart; i != m_queue.end(); ++i )
   {
      MCommunicationCommand* cmd = *i;
      switch ( cmd->m_type ) // in a second case, do actual work
      {
      case MCommunicationCommand::CommandWriteToMonitor:
         // do nothing here
         break;
      case MCommunicationCommand::CommandConnect:
         DoQCommitSubrange(subqueueStart, i, action, localActionWeight);
         ++subqueueStart;
         DoConnect();
         break;
      case MCommunicationCommand::CommandDisconnect:
         DoQCommitSubrange(subqueueStart, i, action, localActionWeight);
         ++subqueueStart;
         Disconnect();
         break;
      case MCommunicationCommand::CommandStartSession:
         DoCheckNotOneWay(M_OPT_STR("StartSession"));
         if ( m_sessionless )
         {  // skip StartSession entirely in sessionless mode, but start the new subrange
            DoQCommitSubrange(subqueueStart, i, action, localActionWeight);
            ++subqueueStart;
            m_isInSession = true; // by convention
            #if !M_NO_MCOM_KEEP_SESSION_ALIVE
               m_sessionKeeper.CheckAndThrowErrors();
            #endif
         }
         else // not sessionless
         {

#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
            if ( m_securityMode != SecurityClearText && !m_securityKeyList.empty() && m_securityKeyListSuccessfulEntry < 0 ) // have to iterate through security and/or password list separately
            {
               DoQCommitSubrange(subqueueStart, i, action, localActionWeight);
               ++subqueueStart;
               StartSession(); // Do it easy in a dedicated step
            }
            else if ( m_issueSecurityOnStartSession && !m_passwordList.empty() && m_passwordListSuccessfulEntry < 0 ) // have to iterate through password list separately (if security list was 
            {
               DoQCommitSubrange(subqueueStart, i, action, localActionWeight);
               ++subqueueStart;
               StartSession(); // Do it easy in a dedicated step
            }
#endif

         }
         break;
      case MCommunicationCommand::CommandEndSession:
      case MCommunicationCommand::CommandEndSessionNoThrow:
         DoCheckNotOneWay(M_OPT_STR("EndSession"));
         if ( m_sessionless )
         {
            DoQCommitSubrange(subqueueStart, i, action, localActionWeight);
            ++subqueueStart;
            m_isInSession = false; // by convention
            #if !M_NO_MCOM_KEEP_SESSION_ALIVE
               m_sessionKeeper.CheckAndThrowErrors();
            #endif
         }
         else
            DoQCommitSubrange(subqueueStart, i + 1, action, localActionWeight); // i + 1 as we include end session
         break;
#if !M_NO_MCOM_IDENTIFY_METER
      case MCommunicationCommand::CommandIdentifyMeter:
         DoQCommitSubrange(subqueueStart, i, action, localActionWeight);
         ++subqueueStart;
         cmd->SetResponse(IdentifyMeter(true));
         break;
#endif
      case MCommunicationCommand::CommandRead:
      case MCommunicationCommand::CommandReadPartial:
         DoCheckNotOneWay(M_OPT_STR("TableRead")); // not a big deal, report TableRead for TableReadPartial
         break;
      case MCommunicationCommand::CommandExecuteResponse:
      case MCommunicationCommand::CommandExecuteRequestResponse:
         DoCheckNotOneWay(M_OPT_STR("FunctionExecuteResponse")); // not a big deal, report FunctionExecuteResponse for FunctionExecuteRequestResponse
         break;
      default:
         ; // otherwise just increment iterator
      }

#if !M_NO_PROGRESS_MONITOR
      localActionWeight += cmd->GetProgressWeight() / progressDivisor;
      if ( localActionWeight > 100.0 ) // this can only be the cause of rounding error
      {
         M_ASSERT(localActionWeight < 101.0);
         localActionWeight = 100.0;
      }
#endif
   }
   DoQCommitSubrange(subqueueStart, m_queue.end(), action, localActionWeight);

#if !M_NO_PROGRESS_MONITOR
   action->SetProgress(100.0);
   action->Complete();
#else
   #undef DoQCommitSubrange
#endif
}

void MProtocolC1222::DoQCommitSubrange(MCommunicationQueue::iterator& start, MCommunicationQueue::iterator end
   #if !M_NO_PROGRESS_MONITOR
                                       , MProgressAction* parentAction, double& actionWeight
   #endif
                                       )
{
   M_ASSERT(!m_oneServicePerApdu);

   if ( start == end )
   {
#if !M_NO_PROGRESS_MONITOR
      actionWeight = 0.0; // nullify here
#endif
      return; // nothing to do
   }

   MCommunicationQueue::iterator localStart = start;
   MCommunicationQueue::iterator i = localStart;
   MCommunicationQueue localQueue;
   unsigned maximumOutgoingHeaderSize = DoGetMaximumApduHeaderSize();
   unsigned maximumIncomingHeaderSize = maximumOutgoingHeaderSize;
   if ( m_sessionless )
   {
      if ( m_issueSecurityOnStartSession )
      {
         maximumOutgoingHeaderSize -= SESSIONLESS_SECURITY_SERVICE_OVERHEAD;
         maximumIncomingHeaderSize -= 2; // response to sessionless security
      }
   }

   for ( unsigned appRetryCount = m_applicationLayerRetries; ; --appRetryCount ) // the most outer loop for checking C12 errors like RSTL/RQTL
   {
#if !M_NO_PROGRESS_MONITOR
      MProgressAction* action = parentAction->CreateChild(actionWeight);
#endif

      try
      {

#if !M_NO_PROGRESS_MONITOR
         double totalProgress = 0.0;
         for ( i = localStart; i != end; ++i )
            totalProgress += (*i)->GetProgressWeight();
         double progressDivisor = totalProgress / 100.0;
         double localActionWeight = 0.0;
         double previousLocalActionWeight = 0.0;
#endif

         unsigned estimatedEpsemRequestSize = maximumOutgoingHeaderSize;
         unsigned estimatedEpsemResponseSize = maximumIncomingHeaderSize;

         unsigned maximumEpsemSizeOutgoing = m_effectiveMaximumApduSizeOutgoing - estimatedEpsemRequestSize;
         unsigned maximumEpsemSizeIncoming = m_effectiveMaximumApduSizeIncoming - estimatedEpsemResponseSize;

         for ( i = localStart; i != end; ++i )
         {
            MCommunicationCommand* cmd = *i;

#if !M_NO_PROGRESS_MONITOR
            localActionWeight += cmd->GetProgressWeight() / progressDivisor;
            if ( localActionWeight > 100.0 ) // this can only be the cause of rounding error
            {
               M_ASSERT(localActionWeight < 101.0);
               localActionWeight = 100.0;
            }
#else
   #define DoQCommitAtomicQueue(a,b,c) DoQCommitAtomicQueue((a))
#endif

            unsigned requestSize; // 1 stands for command byte
            unsigned responseSize; // 1 stands for execution status
            switch ( cmd->m_type )
            {
            case MCommunicationCommand::CommandStartSession:
               // 0F 50 00 00 00 00 00 00 00 00 00 00 00 00 00 3C                      Logon 16 bytes
               // 15 51 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30    Security 21 bytes
               requestSize = 37;
               // 03 00 00 3C // logon response, session timeout
               // 01 00       // security response
               responseSize = 6;
               break;
            case MCommunicationCommand::CommandEndSession:
            case MCommunicationCommand::CommandEndSessionNoThrow:
               requestSize = 2;  // length + command
               responseSize = 2; // length + status
               break;
            case MCommunicationCommand::CommandRead:
               {
                  requestSize = 4;  // length + command + table number like "03 30 00 03"
                  responseSize = MaximumEpsemServiceLengthSize + 1 + 2 + 1;  // length3 + status + dataLength2 + data + checksum
                  unsigned len = cmd->GetLength();
                  if ( len == 0 ) // length unknown
                     responseSize += 1000;
                  else
                     responseSize += len;
               }
               break;
            case MCommunicationCommand::CommandReadPartial:
               requestSize = 9;  // length + command + table number + offset + len 
               responseSize = MaximumEpsemServiceLengthSize + 1 + 2 + 1 + cmd->GetLength();  // length3 + status + dataLength + data + checksum
               break;
            case MCommunicationCommand::CommandWrite:
               requestSize = MaximumEpsemServiceLengthSize + 7 + static_cast<unsigned>(cmd->m_request.size());  // length3 + command + number2 + len2 + data + checksum
               responseSize = 2;  // length + status
               break;
            case MCommunicationCommand::CommandWritePartial:
               requestSize = MaximumEpsemServiceLengthSize + 8 + static_cast<unsigned>(cmd->m_request.size());  // length3 + command + number2 + offset3 + len2 + data + checksum
               responseSize = 2;  // length + status
               break;
            case MCommunicationCommand::CommandExecute:
               requestSize = 14;    // like "09 40 00 07 00 03 03 00 00 FD   03 30 00 08"
               responseSize = 11;   // like "01 00            08 00 00 04 00 03 00 00 FD"
               break;
            case MCommunicationCommand::CommandExecuteRequest:
               requestSize = MaximumEpsemServiceLengthSize + 13 + static_cast<unsigned>(cmd->m_request.size());
               responseSize = 11;
               break;
            case MCommunicationCommand::CommandExecuteResponse:
               requestSize = 14;
               responseSize = MaximumEpsemServiceLengthSize + 10 + cmd->GetLength();
               break;
            case MCommunicationCommand::CommandExecuteRequestResponse:
               requestSize = MaximumEpsemServiceLengthSize + 13 + static_cast<unsigned>(cmd->m_request.size());
               responseSize = MaximumEpsemServiceLengthSize + 10 + cmd->GetLength();
               break;
            case MCommunicationCommand::CommandWriteToMonitor:
               requestSize = 0;
               responseSize = 0;
               // and do nothing
               break;
            default:
               requestSize = 256; /// pacify analysis tools 
               responseSize = 256;
               M_ASSERT(0); // warn on debug, ignore on release -- possibility of a new command
            }

            if ( estimatedEpsemRequestSize + requestSize >= maximumEpsemSizeOutgoing ||
                 estimatedEpsemResponseSize + responseSize >= maximumEpsemSizeIncoming ) // if the sizes exceed limit
            {
               // we always try to execute long requests in a single EPSEM. Therefore flush immediately when we know we will not fit the request
               DoQCommitAtomicQueue(localQueue, action, previousLocalActionWeight);

#if !M_NO_PROGRESS_MONITOR
               previousLocalActionWeight = localActionWeight;
#endif

               M_ASSERT(localQueue.empty());
               estimatedEpsemRequestSize = maximumOutgoingHeaderSize + requestSize;     // reset estimated size, most common use case, can be overridden later
               estimatedEpsemResponseSize = maximumIncomingHeaderSize + responseSize;   // reset estimated size, most common use case, can be overridden later

               switch ( cmd->m_type )
               {
               case MCommunicationCommand::CommandStartSession:
               case MCommunicationCommand::CommandEndSession:
               case MCommunicationCommand::CommandEndSessionNoThrow:
                  M_ASSERT(!m_sessionless); // otherwise we are never here
                  localQueue.push_back(cmd->NewClone());
                  break;
               case MCommunicationCommand::CommandRead:
               case MCommunicationCommand::CommandReadPartial:
                  if ( estimatedEpsemResponseSize + responseSize < maximumEpsemSizeIncoming ) // possible case when after the flashing the queue the size starts to fit in
                     localQueue.push_back(cmd->NewClone()); // start entering a new function
                  else
                  {
                     unsigned offset = ((cmd->m_type & MCommunicationCommand::FeatureOffsetPresent) == 0) ? 0 : cmd->GetOffset();

#if !M_NO_PROGRESS_MONITOR
                     action->CreateLocalAction(localActionWeight);
#endif

                     cmd->SetResponse(TableReadPartial(cmd->GetNumber(), offset, cmd->GetLength())); // this does everything what's necessary inside
                     estimatedEpsemRequestSize  = maximumOutgoingHeaderSize; // override, empty queue
                     estimatedEpsemResponseSize = maximumIncomingHeaderSize; // override, empty queue
                  }
                  break;
               case MCommunicationCommand::CommandWrite:
               case MCommunicationCommand::CommandWritePartial:
                  if ( estimatedEpsemRequestSize + requestSize < maximumEpsemSizeOutgoing ) // possible case when after the flashing the queue the size starts to fit in
                     localQueue.push_back(cmd->NewClone()); // start entering a new function
                  else
                  {
                     unsigned offset = ((cmd->m_type & MCommunicationCommand::FeatureOffsetPresent) == 0) ? 0 : cmd->GetOffset();

#if !M_NO_PROGRESS_MONITOR
                     action->CreateLocalAction(localActionWeight);
#endif

                     TableWritePartial(cmd->GetNumber(), cmd->GetRequest(), offset);
                     estimatedEpsemRequestSize  = maximumOutgoingHeaderSize; // override, empty queue
                     estimatedEpsemResponseSize = maximumIncomingHeaderSize; // override, empty queue
                  }
                  break;
               case MCommunicationCommand::CommandExecute:
               case MCommunicationCommand::CommandExecuteRequest:
               case MCommunicationCommand::CommandExecuteResponse:
               case MCommunicationCommand::CommandExecuteRequestResponse:
                  localQueue.push_back(cmd->NewClone()); // start entering a new function
                  break;
               default:
                  M_ASSERT(0); // warn on debug, ignore on release -- possibility of a new command
               }
            }
            else
            {
               estimatedEpsemRequestSize += requestSize;
               estimatedEpsemResponseSize += responseSize;
               localQueue.push_back(cmd->NewClone());
            }
         }
         DoQCommitAtomicQueue(localQueue, action, (localActionWeight + previousLocalActionWeight) / 2.0);

#if !M_NO_PROGRESS_MONITOR
         action->Complete();
#else
   #undef DoQCommitAtomicQueue
#endif

         start = end;
         break;
      }
      catch ( MEC12NokResponse& ex )
      {
         DoRethrowIfNotProperRqtlRstl(ex, appRetryCount);

#if !M_NO_PROGRESS_MONITOR
         if ( action != NULL )
            action->Complete();
#endif

         // incrementing the iterator on the number of already processed operations
         localStart += (i - localStart) - (localQueue.end() - localQueue.begin());
         localQueue.clear();
      }
   }
}

void MProtocolC1222::DoQCommitAtomicQueue(MCommunicationQueue& q
                                      #if !M_NO_PROGRESS_MONITOR
                                          , MProgressAction* action, double progress
                                      #endif
                                          )
{
   if ( q.empty() ) // don't bother to do anything. Usual case, by the way.
      return;

   // Special case, detect when only monitor messages are in the queue, as only for them we do not do any start/end
   bool onlyMonitorMessages = true;
   for ( MCommunicationQueue::iterator i = q.begin(); i != q.end(); ++i )
   {
      if ( (*i)->m_type != MCommunicationCommand::CommandWriteToMonitor )
      {
         onlyMonitorMessages = false;
         break;
      }
   }
   if ( onlyMonitorMessages )
   {
#if !M_NO_MCOM_MONITOR
      for ( MCommunicationQueue::iterator i = q.begin(); i != q.end(); ++i )
      {
         M_ASSERT((*i)->m_type == MCommunicationCommand::CommandWriteToMonitor);
         WriteToMonitor((*i)->GetRequest());
      }
#endif
      return;
   }

   MCommunicationCommand* functionRetryCommand = NULL;
   MCommunicationCommand* cmd = NULL;
   unsigned appRetryCount = m_applicationLayerRetries;
   unsigned procRetryCount = m_applicationLayerProcedureRetries;
   for ( ; ; )
   {
      unsigned firstWrapper = (unsigned)m_wrapperProtocol->m_serviceWrappers.size(); // this will never change
      unsigned successCount = 0;
      MCommunicationQueue::iterator j = q.begin();
      try
      {

         SendStart();


         for ( MCommunicationQueue::iterator i = q.begin(); i != q.end(); ++i )
         {
            cmd = *i;
            if ( functionRetryCommand == cmd )
            {
               M_ASSERT(i == q.begin()); // a retried function is always the first command
               SendTableRead(8);
               M_NEW MProtocolServiceWrapper(m_wrapperProtocol, M_OPT_STR("FunctionExecuteRetried"), cmd->GetNumber(), -1, -1);
            }
            else
            {
               switch ( cmd->m_type )
               {
               case MCommunicationCommand::CommandStartSession:
                  M_ASSERT(!m_sessionless);
                  DoSendStartSession();
                  M_NEW MProtocolServiceWrapper(m_wrapperProtocol, M_OPT_STR("StartSession"), MProtocolServiceWrapper::ServiceStartsSessionKeeping);
                  break;
               case MCommunicationCommand::CommandEndSession:
               case MCommunicationCommand::CommandEndSessionNoThrow:
                  M_ASSERT(!m_sessionless);
                  DoSendEndSession();
                  M_NEW MProtocolServiceWrapper(m_wrapperProtocol, M_OPT_STR("EndSession"), MProtocolServiceWrapper::ServiceEndsSessionKeeping);
                  break;
               case MCommunicationCommand::CommandRead:
                  if ( m_alwaysUsePartial && cmd->GetLength() != 0 )
                     SendTableReadPartial(cmd->GetNumber(), 0, cmd->GetLength());
                  else
                     SendTableRead(cmd->GetNumber());
                  M_NEW MProtocolServiceWrapper(m_wrapperProtocol, M_OPT_STR("TableRead"), cmd->GetNumber(), -1, -1);
                  break;
               case MCommunicationCommand::CommandWrite:
                  if ( m_alwaysUsePartial )
                     SendTableWritePartial(cmd->GetNumber(), cmd->GetRequest(), 0);
                  else
                     SendTableWrite(cmd->GetNumber(), cmd->GetRequest());
                  M_NEW MProtocolServiceWrapper(m_wrapperProtocol, M_OPT_STR("TableWrite"), cmd->GetNumber(), -1, -1);
                  break;
               case MCommunicationCommand::CommandReadPartial:
                  SendTableReadPartial(cmd->GetNumber(), cmd->GetOffset(), cmd->GetLength());
                  M_NEW MProtocolServiceWrapper(m_wrapperProtocol, M_OPT_STR("TableReadPartial"), cmd->GetNumber(), cmd->GetOffset(), cmd->GetLength());
                  break;
               case MCommunicationCommand::CommandWritePartial:
                  SendTableWritePartial(cmd->GetNumber(), cmd->GetRequest(), cmd->GetOffset());
                  M_NEW MProtocolServiceWrapper(m_wrapperProtocol, M_OPT_STR("TableWritePartial"), cmd->GetNumber(), cmd->GetOffset(), (int)cmd->GetRequest().size());
                  break;
               case MCommunicationCommand::CommandExecute:
                  m_meterIsLittleEndian = cmd->GetLittleEndian(); // do it only at function data send
                  FunctionExecuteSend(cmd->GetNumber());
                  M_NEW MProtocolServiceWrapper(m_wrapperProtocol, M_OPT_STR("FunctionExecute"), cmd->GetNumber(), -1, -1);
                  break;
               case MCommunicationCommand::CommandExecuteRequest:
                  m_meterIsLittleEndian = cmd->GetLittleEndian(); // do it only at function data send
                  FunctionExecuteRequestSend(cmd->GetNumber(), cmd->GetRequest());
                  M_NEW MProtocolServiceWrapper(m_wrapperProtocol, M_OPT_STR("FunctionExecuteRequest"), cmd->GetNumber(), -1, -1);
                  break;
               case MCommunicationCommand::CommandExecuteResponse:
                  m_meterIsLittleEndian = cmd->GetLittleEndian(); // do it only at function data send
                  FunctionExecuteResponseSend(cmd->GetNumber());
                  M_NEW MProtocolServiceWrapper(m_wrapperProtocol, M_OPT_STR("FunctionExecuteResponse"), cmd->GetNumber(), -1, -1);
                  break;
               case MCommunicationCommand::CommandExecuteRequestResponse:
                  m_meterIsLittleEndian = cmd->GetLittleEndian(); // do it only at function data send
                  FunctionExecuteRequestResponseSend(cmd->GetNumber(), cmd->GetRequest());
                  M_NEW MProtocolServiceWrapper(m_wrapperProtocol, M_OPT_STR("FunctionExecuteRequestResponse"), cmd->GetNumber(), -1, -1);
                  break;
               case MCommunicationCommand::CommandWriteToMonitor:
                  #if !M_NO_MCOM_MONITOR
                     WriteToMonitor(cmd->GetRequest());
                  #endif
                  break;
               default:
                  M_ASSERT(0); // warn on debug, ignore on release -- possibility of a new command
               }
            }
         }
         SendEnd();

#if !M_NO_PROGRESS_MONITOR
         action->SetProgress(progress);
#endif

         if ( m_responseControl <= ResponseControlOnException )
         {
            ReceiveStart();


            for ( j = q.begin(); j != q.end(); ++j )
            {
               bool doNotDeleteWrapper = false;
               cmd = *j;
               MCOMNumber num;
               if ( cmd->m_type != MCommunicationCommand::CommandStartSession && 
                    cmd->m_type != MCommunicationCommand::CommandEndSession && 
                    cmd->m_type != MCommunicationCommand::CommandEndSessionNoThrow && 
                    cmd->m_type != MCommunicationCommand::CommandWriteToMonitor )
               {
                  num = cmd->GetNumber();
               }
               if ( functionRetryCommand == cmd )
               {
                  functionRetryCommand = NULL; // nullify mainly for debug purposes
                  M_ASSERT(j == q.begin()); // a retried function is always the first command
                  MByteString response = ReceiveTableRead(8);
                  DoHandleFunctionResponseTable8Read(response);
                  MCommunicationCommand::CommandType type = cmd->GetCommandType();   // Successful handling here
                  if ( type == MCommunicationCommand::CommandExecuteResponse || type == MCommunicationCommand::CommandExecuteRequestResponse )
                     m_queue.GetResponseCommand(type, cmd->m_number, cmd->m_id)->SetResponse(response);
               }
               else
               {
                  int id = cmd->GetDataId();
                  switch ( cmd->m_type )
                  {
                  case MCommunicationCommand::CommandStartSession:
                     DoReceiveStartSession();
                     break;
                  case MCommunicationCommand::CommandEndSession:
                  case MCommunicationCommand::CommandEndSessionNoThrow:
                     DoReceiveEndSession();
                     break;
                  case MCommunicationCommand::CommandRead:
                     if ( m_alwaysUsePartial && cmd->GetLength() != 0 )
                        m_queue.GetResponseCommand(cmd->m_type, num, id)->AppendResponse(ReceiveTableReadPartial(num, 0, cmd->GetLength()));
                     else
                        m_queue.GetResponseCommand(cmd->m_type, num, id)->AppendResponse(ReceiveTableRead(num));
                     break;
                  case MCommunicationCommand::CommandWrite:
                     if ( m_alwaysUsePartial )
                        ReceiveTableWritePartial(num, cmd->GetRequest(), 0);
                     else
                        ReceiveTableWrite(num, cmd->GetRequest());
                     break;
                  case MCommunicationCommand::CommandReadPartial:
                     m_queue.GetResponseCommand(cmd->m_type, num, id)->AppendResponse(ReceiveTableReadPartial(num, cmd->GetOffset(), cmd->GetLength()));
                     break;
                  case MCommunicationCommand::CommandWritePartial:
                     ReceiveTableWritePartial(num, cmd->GetRequest(), cmd->GetOffset());
                     break;
                  case MCommunicationCommand::CommandExecute:
                     FunctionExecuteReceive(num);
                     break;
                  case MCommunicationCommand::CommandExecuteRequest:
                     FunctionExecuteRequestReceive(num, cmd->GetRequest());
                     break;
                  case MCommunicationCommand::CommandExecuteResponse:
                     m_queue.GetResponseCommand(cmd->m_type, num, id)->SetResponse(FunctionExecuteResponseReceive(num));
                     break;
                  case MCommunicationCommand::CommandExecuteRequestResponse:
                     m_queue.GetResponseCommand(cmd->m_type, num, id)->SetResponse(FunctionExecuteRequestResponseReceive(num, cmd->GetRequest()));
                     break;
                  case MCommunicationCommand::CommandWriteToMonitor:
                     doNotDeleteWrapper = true;
                     break;
                  default:
                     M_ASSERT(0); // warn on debug, ignore on release -- possibility of a new command
                  }
               }
               if ( !doNotDeleteWrapper )
               {
                  M_ASSERT(firstWrapper < m_wrapperProtocol->m_serviceWrappers.size());
                  delete m_wrapperProtocol->m_serviceWrappers[firstWrapper];
               }
               ++successCount;
            }
            ReceiveEnd();
         }
         else // otherwise only
         {
            for ( MCommunicationQueue::iterator j = q.begin(); j != q.end(); ++j )
            {
               M_ASSERT(firstWrapper < m_wrapperProtocol->m_serviceWrappers.size());
               delete m_wrapperProtocol->m_serviceWrappers[firstWrapper];
               ++successCount;
            }
         }
         q.clear(); // clear the queue, so it is ready for the next use
         break; // successful exit from loop
      }
      catch ( MException& ex )
      {
         if ( ex.GetClass() == MEChannelReadTimeout::GetStaticClass() && m_responseControl == ResponseControlOnException )
         {
            MEChannelReadTimeout* readTimeoutEx = M_CHECKED_CAST(MEChannelReadTimeout*, &ex);
            if ( readTimeoutEx->GetBytesRead() == 0 )
            {
               q.clear(); // clear the queue, so it is ready for the next use
               break; // success, no exception
            }
         }
         const MClass* cls = ex.GetClass();
         if ( cls == MEC12BadProcedureResult::GetStaticClass() ) // bad procedure result
         {
            MEC12BadProcedureResult* e = M_CHECKED_CAST(MEC12BadProcedureResult*, &ex);
            MEC12BadProcedureResult::ResultCodeEnum code = e->GetProcedureResultCode();
            if ( code == MEC12BadProcedureResult::RESULT_NOT_COMPLETED ) // the only retryable code
            {
               if ( procRetryCount > 0 )
               {
                  M_ASSERT(cmd->GetCommandType() == MCommunicationCommand::CommandExecute ||
                           cmd->GetCommandType() == MCommunicationCommand::CommandExecuteRequest ||
                           cmd->GetCommandType() == MCommunicationCommand::CommandExecuteResponse ||
                           cmd->GetCommandType() == MCommunicationCommand::CommandExecuteRequestResponse); // make sure the queue entry is as expected
                  functionRetryCommand = cmd; // functionRetryCommand(q.begin() + successCount); // m_queue.GetResponseCommand(cmd->m_type, cmd->GetNumber(), cmd->GetDataId());
                  M_ASSERT(*(q.begin() + successCount) == cmd);
                  q.erase(q.begin(), q.begin() + successCount);
                  successCount = 0;
                  while ( m_wrapperProtocol->m_serviceWrappers.size() > firstWrapper )
                  {
                     MUniquePtr<MProtocolServiceWrapper> curr(m_wrapperProtocol->m_serviceWrappers[m_wrapperProtocol->m_serviceWrappers.size() - 1]); // delete at the end
                     if ( m_wrapperProtocol->m_serviceWrappers.size() == firstWrapper + 1 )
                        curr->NotifyOrThrowRetry(ex, procRetryCount); // will never throw, as the retry count is nonzero
                     curr->HandleFailureSilently();
                  }
                  --procRetryCount;
                  Sleep(m_applicationLayerProcedureRetryDelay);
                  continue; // continue a retry
               }
               MProtocolLayerWrapper::PrependRetriesExpired(ex);
            }
         }
         else if ( cls == MEC12NokResponse::GetStaticClass() )
         {
            MEC12NokResponse* e = M_CHECKED_CAST(MEC12NokResponse*, &ex);
            MEC12NokResponse::ResponseCode code = (MEC12NokResponse::ResponseCode)e->GetResponseCode();
            if ( code == MEC12NokResponse::RESPONSE_RQTL || code == MEC12NokResponse::RESPONSE_RSTL )
            {
               while ( m_wrapperProtocol->m_serviceWrappers.size() > firstWrapper )
               {
                  MUniquePtr<MProtocolServiceWrapper> curr(m_wrapperProtocol->m_serviceWrappers[m_wrapperProtocol->m_serviceWrappers.size() - 1]); // delete at the end
                  curr->HandleFailureSilently();
               }

               // Throw so that the upper DoQCommitSubrange reset packet
               if ( j != q.begin() ) // if j == first, no elements should be erased
                  q.erase(q.begin(), j);
               e->Rethrow();
            }
            else if ( (code == MEC12NokResponse::RESPONSE_BSY || code == MEC12NokResponse::RESPONSE_DNR) ) // "upper" C12 layer uses BSY and DNR as retriables
            {
               if ( appRetryCount > 0 ) // retries are not over
               {
                  q.erase(q.begin(), q.begin() + successCount);
                  successCount = 0;
                  while ( m_wrapperProtocol->m_serviceWrappers.size() > firstWrapper )
                  {
                     MUniquePtr<MProtocolServiceWrapper> curr(m_wrapperProtocol->m_serviceWrappers[m_wrapperProtocol->m_serviceWrappers.size() - 1]); // delete at the end
                     if ( m_wrapperProtocol->m_serviceWrappers.size() == firstWrapper + 1 )
                        curr->NotifyOrThrowRetry(ex, appRetryCount); // will never throw, as the retry count is nonzero
                     curr->HandleFailureSilently();
                  }
                  --appRetryCount;
                  Sleep(m_applicationLayerRetryDelay);
                  continue; // continue a retry
               }
               MProtocolLayerWrapper::PrependRetriesExpired(ex);
            }
         }

         if ( m_endSessionOnApplicationLayerError && !m_sessionless && m_isInSession )
         {
            if ( cls == MEC12BadProcedureResult::GetStaticClass() )
               DoEndSessionOnApplicationLayerError(false);
            else if ( cls == MEC12NokResponse::GetStaticClass() )
            {
               MEC12NokResponse::ResponseCode code = M_CHECKED_CAST(MEC12NokResponse*, &ex)->GetResponseCode();
               if ( code == MEC12NokResponse::RESPONSE_ERR && ex.GetKind() == MException::ErrorSecurity ) // special handling of bad password case with password list
                  DoEndSessionOnApplicationLayerError(false); // throw only for the last password in the list, or when there is only one password to check
               else if ( code != MEC12NokResponse::RESPONSE_ISSS && code != MEC12NokResponse::RESPONSE_RNO && code != MEC12NokResponse::RESPONSE_SME )
                  DoEndSessionOnApplicationLayerError(false);
            }
         }

         while ( m_wrapperProtocol->m_serviceWrappers.size() > firstWrapper )
         {
            MUniquePtr<MProtocolServiceWrapper> curr(m_wrapperProtocol->m_serviceWrappers[m_wrapperProtocol->m_serviceWrappers.size() - 1]); // delete at the end
            if ( m_wrapperProtocol->m_serviceWrappers.size() == firstWrapper + 1 )
            {
               curr->HandleFailureAndRethrow(ex);
               M_ENSURED_ASSERT(0);
            }
            curr->HandleFailureSilently();
         }

         throw;
      }
   }
}

M_NORETURN_FUNC void MProtocolC1222::DoThrowBadACSEResponse(char acse)
{
   MCOMException::Throw(M_CODE_STR_P1(M_ERR_BAD_DATA_IN_ACSE_RESPONSE, M_I("Bad ACSE element %2X received"), (unsigned)(Muint8)acse));
   M_ENSURED_ASSERT(0);
}

void MProtocolC1222::DoCheckNotOneWay(MConstChars whatOperation)
{
   if ( m_responseControl != ResponseControlAlways )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR_P1(M_ERR_NOT_SUPPORTED_IN_ONE_WAY_MODE, "%s is not supported when response control is not set to 'always'", whatOperation));
      M_ENSURED_ASSERT(0);
   }
}


void MProtocolC1222::DoInitializeEax(const MStdString& apTitle)
{
   m_eax.SetKey(m_securityKey);
}

void MProtocolC1222::DoTryPasswordEntry(const MByteString& entry)
{
   for ( unsigned appRetryCount = m_applicationLayerRetries; ; --appRetryCount )
   {
      try
      {
         MValueSavior<MByteString> passwordSavior(&m_password, entry);
         SendStart();
         SendSecurity();
         if ( SendEndReceiveStart() )
         {
            ReceiveSecurity();
            ReceiveEnd();
         }
         break;
      }
      catch ( MEC12NokResponse& ex )
      {
         MEC12NokResponse::ResponseCode code = (MEC12NokResponse::ResponseCode)ex.GetResponseCode();
         if ( code != MEC12NokResponse::RESPONSE_BSY && code != MEC12NokResponse::RESPONSE_DNR )
            throw;

         MProtocolServiceWrapper::StaticNotifyOrThrowRetry(m_wrapperProtocol, ex, appRetryCount);
         Sleep(m_applicationLayerRetryDelay); // sleep and retry
      }
   }
}

void MProtocolC1222::DoUpdateCallingApInvocationId(bool ignoreSessionless)
{
   if ( !m_callingApInvocationIdSetByUser )
   {
      if ( !ignoreSessionless && !m_sessionless )
         ++m_callingApInvocationId;
      else
      {
         #if defined(M__DEBUG_C1222_WITH_CONSTANT_DATA) && M__DEBUG_C1222_WITH_CONSTANT_DATA != 0
            m_callingApInvocationId = 0x12345678;
         #else
            for ( ; ; )
            {
               unsigned candidate = MMath::RandomInRange(0, 0xFFFFFFFF); // randomized already at startup
               if ( candidate != 0 && candidate != m_callingApInvocationId )
               {
                  m_callingApInvocationId = candidate;
                  return;
               }
            }
         #endif
      }
   }
   else
      m_callingApInvocationIdSetByUser = false; // do an invocation ID exactly once
}

void MProtocolC1222::DoTableRead(MCOMNumberConstRef number, MByteString& data, unsigned expectedSize)
{
   for ( unsigned appRetryCount = m_applicationLayerRetries; ; --appRetryCount )
   {
      try
      {
         MProtocolC12::DoTableRead(number, data, expectedSize);
         return;
      }
      catch ( MEC12NokResponse& ex )
      {
         DoRethrowIfNotProperRqtlRstl(ex, appRetryCount);
      }
   }
}

void MProtocolC1222::DoTableWrite(MCOMNumberConstRef number, const MByteString& data)
{
   for ( unsigned appRetryCount = m_applicationLayerRetries; ; --appRetryCount )
   {
      try
      {
         MProtocolC12::DoTableWrite(number, data);
         return;
      }
      catch ( MEC12NokResponse& ex )
      {
         DoRethrowIfNotProperRqtlRstl(ex, appRetryCount);
      }
   }
}

void MProtocolC1222::DoTableReadPartial(MCOMNumberConstRef number, MByteString& data, int offset, int length)
{
   for ( unsigned appRetryCount = m_applicationLayerRetries; ; --appRetryCount )
   {
      try
      {
         MProtocolC12::DoTableReadPartial(number, data, offset, length);
         return;
      }
      catch ( MEC12NokResponse& ex )
      {
         DoRethrowIfNotProperRqtlRstl(ex, appRetryCount);
      }
   }
}

void MProtocolC1222::DoTableWritePartial(MCOMNumberConstRef number, const MByteString& data, int offset)
{
   for ( unsigned appRetryCount = m_applicationLayerRetries; ; --appRetryCount )
   {
      try
      {
         MProtocolC12::DoTableWritePartial(number, data, offset);
         return;
      }
      catch ( MEC12NokResponse& ex )
      {
         DoRethrowIfNotProperRqtlRstl(ex, appRetryCount);
      }
   }
}

void MProtocolC1222::DoRethrowIfNotProperRqtlRstl(MEC12NokResponse& ex, unsigned appRetryCount)
{
   MEC12NokResponse::ResponseCode code = (MEC12NokResponse::ResponseCode)ex.GetResponseCode();
   if ( code == MEC12NokResponse::RESPONSE_RQTL || code == MEC12NokResponse::RESPONSE_RSTL )
   {
      if ( appRetryCount > 0 )
      {
         const MByteString& extraParameters = ex.GetExtraParameters();
         if ( extraParameters.size() > 0 && extraParameters.size() <= 4 )
         {
            MByteString maxApduSizeStr = ex.GetExtraParameters();
            unsigned maxApduSize = MUtilities::UnsignedFromUINT(maxApduSizeStr, false);
            if (  code == MEC12NokResponse::RESPONSE_RQTL )
            {
               if ( m_effectiveMaximumApduSizeOutgoing - 16 <= maxApduSize ) // if we are close already, assume possible firmware problem, adjust down by a large enough value and retry
               {
                  maxApduSize = m_effectiveMaximumApduSizeOutgoing - 16;
                  maxApduSize -= maxApduSize / 16;
               }
               if ( maxApduSize >= unsigned(MinimumMaximumApduTotalSize) && maxApduSize <= unsigned(MaximumMaximumApduTotalSize) ) // if we can do anything about this here...
               {
                  ChangeNegotiatedMaximumApduSizeOutgoing(maxApduSize);
                  MProtocolServiceWrapper::StaticNotifyOrThrowRetry(m_wrapperProtocol, ex, appRetryCount);
                  #if !M_NO_MCOM_MONITOR
                     WriteToMonitor(MGetStdString("Per RQTL error, adjusting maximum outgoing APDU size to %d and retrying", maxApduSize));
                  #endif
                  Sleep(m_applicationLayerRetryDelay); // sleep and retry
                  return; // go retrying
               }
               // otherwise rethrow
            }
            else
            {
               M_ASSERT(code == MEC12NokResponse::RESPONSE_RSTL);
               if ( m_effectiveMaximumApduSizeIncoming - 16 <= maxApduSize ) // if we are close already, assume possible firmware problem, adjust down by a large enough value and retry
               {
                  maxApduSize -= m_effectiveMaximumApduSizeIncoming - 16;
                  maxApduSize -= maxApduSize / 16;
               }
               if ( maxApduSize >= unsigned(MinimumMaximumApduTotalSize) && maxApduSize <= unsigned(MaximumMaximumApduTotalSize)  ) // if we can do anything about this here...
               {
                  ChangeNegotiatedMaximumApduSizeIncoming(maxApduSize);
                  MProtocolServiceWrapper::StaticNotifyOrThrowRetry(m_wrapperProtocol, ex, appRetryCount);
                  #if !M_NO_MCOM_MONITOR
                     WriteToMonitor(MGetStdString("Per RSTL error, adjusting maximum incoming APDU size to %d and retrying", maxApduSize));
                  #endif
                  Sleep(m_applicationLayerRetryDelay); // sleep and retry
                  return; // go retrying
               }
               // otherwise rethrow
            }
         }
      }
      if ( m_endSessionOnApplicationLayerError )
         DoEndSessionOnApplicationLayerError(false);
      MProtocolServiceWrapper::StaticHandleFailureAndRethrow(this, ex);
      M_ENSURED_ASSERT(0);
   }
   ex.Rethrow(); // do it without any monitor handling
   M_ENSURED_ASSERT(0);
}


#if !M_NO_MCOM_KEEP_SESSION_ALIVE

unsigned MProtocolC1222::DoGetKeepSessionAliveFirstDelay() const
{
   if ( !m_isInSession || !IsConnected() )
      return 0u;

   if ( m_sessionIdleTimeout == 0 || // special value defined by the protocol, means session never times out
        m_sessionIdleTimeout > 20 ) // do not do session keeping in smaller intervals to facilitate easy task interruption
      return 16000u;
   if ( m_sessionIdleTimeout < 2 )   // Assume something is wrong about the very value. Do not keep session more often than in 1 second intervals
      return 1000u;
   if ( m_sessionIdleTimeout < 4 )   // Assume something is wrong about the very value. Do not keep session more often than in 100 millisecond intervals
      return MTimer::SecondsToMilliseconds(m_sessionIdleTimeout) - 1000u;
   return MTimer::SecondsToMilliseconds(m_sessionIdleTimeout) - 2000u;
}

#endif // !M_NO_MCOM_KEEP_SESSION_ALIVE
#endif // !M_NO_MCOM_PROTOCOL_C1222
