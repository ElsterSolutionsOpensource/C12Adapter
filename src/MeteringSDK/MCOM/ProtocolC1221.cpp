// File MCOM/ProtocolC1221.cpp

#include "MCOMExtern.h"
#include "ProtocolC1221.h"
#include "ChannelOpticalProbe.h"
#include "ChannelModem.h"
#include "MCOMExceptions.h"

#if !M_NO_MCOM_PROTOCOL_C1221

M_START_PROPERTIES(ProtocolC1221)
   M_OBJECT_PROPERTY_PERSISTENT_BYTE_STRING(ProtocolC1221, AuthenticationKey,     "00000000", 8u, ST_constMByteStringA_X, ST_X_constMByteStringA)
   M_OBJECT_PROPERTY_PERSISTENT_INT        (ProtocolC1221, AuthenticationKeyId,               0u)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL       (ProtocolC1221, EnableAuthentication,            true)
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC1221, Identity,                          0u)
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC1221, DataFormat,                        MProtocolC1221::DataFormatC1218C1221)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL       (ProtocolC1221, IssueNegotiateOnStartSession,    true)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL       (ProtocolC1221, IssueTimingSetupOnStartSession, false)
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   M_OBJECT_PROPERTY_BYTE_STRING_COLLECTION(ProtocolC1221, AuthenticationKeyList,                 ST_constMByteStringVectorA_X)
   M_OBJECT_PROPERTY_READONLY_INT          (ProtocolC1221, AuthenticationKeyListSuccessfulEntry)
#endif // !M_NO_MCOM_PASSWORD_AND_KEY_LIST
// Changes in default values:
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC1221, IntercharacterTimeout,       1000u) // DOXYGEN_HIDE SWIG_HIDE
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC1221, AcknowledgementTimeout,      4000u) // DOXYGEN_HIDE SWIG_HIDE
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC1221, ChannelTrafficTimeout,      30000u) // DOXYGEN_HIDE SWIG_HIDE
// Number of Packets is by C12.21 default 1, but this implementation uses 64
   M_OBJECT_PROPERTY_READONLY_UINT         (ProtocolC1221, IncomingIdentity)
   M_OBJECT_PROPERTY_READONLY_UINT         (ProtocolC1221, IncomingDataFormat)
   M_OBJECT_PROPERTY_READONLY_INT          (ProtocolC1221, IdentifiedAuthenticationAlgorithm)
M_START_METHODS(ProtocolC1221)
   M_OBJECT_SERVICE                        (ProtocolC1221, TimingSetup,                ST_X)
   M_OBJECT_SERVICE                        (ProtocolC1221, TimingSetupWithWorkaround,  ST_X)
   M_OBJECT_SERVICE                        (ProtocolC1221, Authenticate,               ST_X)
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   M_OBJECT_SERVICE                        (ProtocolC1221, ClearAuthenticationKeyList, ST_X)
   M_OBJECT_SERVICE                        (ProtocolC1221, AddToAuthenticationKeyList, ST_X_constMByteStringA)
#endif
M_END_CLASS_TYPED(ProtocolC1221, ProtocolC1218, "PROTOCOL_ANSI_C12_21")

   const char CHAR_START = '\xEE';
   const char CHAR_ACK = '\x06';
   const char CHAR_NAK = '\x15';

   const unsigned char ALGORITHM_DES = 0x00;
   const unsigned char ALGORITHM_AES = 0xFF;

MProtocolC1221::MProtocolC1221(MChannel *channel, bool channelIsOwned)
:
   MProtocolC1218(channel, channelIsOwned),
   m_canAuthenticate(false),
   m_authenticationAlgorithm(ALGORITHM_DES)
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   , m_authenticationKeyList()
   , m_authenticationKeyListSuccessfulEntry(-1)
#endif
{
   M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(ProtocolC1221);
}

MProtocolC1221::~MProtocolC1221()
{
   Finalize();

   MAes::DestroySecureData(m_authenticationKey);
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   MAes::DestroySecureData(m_authenticationKeyList);
#endif
}

void MProtocolC1221::DoStartSession()
{
#if !M_NO_PROGRESS_MONITOR
   MProgressAction* action = GetLocalProgressAction();
#endif

   ApplyChannelParameters(); // need to do so, case if the session is started several times without reconnecting

#if !M_NO_PROGRESS_MONITOR
   action->SetProgress(5.0);
#endif

   Identify();

#if !M_NO_PROGRESS_MONITOR
   action->SetProgress(20.0);
#endif

   if ( m_issueTimingSetupOnStartSession )
      TimingSetupWithWorkaround();

#if !M_NO_PROGRESS_MONITOR
   action->SetProgress(40.0);
#endif

   if ( m_issueNegotiateOnStartSession )
      Negotiate();

#if !M_NO_PROGRESS_MONITOR
   action->SetProgress(60.0);
#endif

   Logon();

#if !M_NO_PROGRESS_MONITOR
   action->SetProgress(80.0);
#endif

   if ( m_issueSecurityOnStartSession )
      FullLogin();


#if !M_NO_PROGRESS_MONITOR
   action->Complete();
#endif
}

void MProtocolC1221::FullLogin()
{
   if ( m_enableAuthentication )
      Authenticate();
   else
      Security();
}

void MProtocolC1221::DoVerifyAuthenticationKey(const MByteString& key)
{
   const size_t keySize = key.size();
   if ( keySize != 8 && keySize != 16 )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_AUTHENTICATION_KEY_IS_EXPECTED_TO_BE_D1_BYTES_LONG, M_I("Authentication key shall be 8 bytes long for DES or 16 bytes long for AES")));
      M_ENSURED_ASSERT(0);
   }
}

void MProtocolC1221::SetAuthenticationKey(const MByteString& key)
{
   DoVerifyAuthenticationKey(key);
   MAes::AssignSecureData(m_authenticationKey, key);
}

void MProtocolC1221::SetAuthenticationKeyId(unsigned id)
{
   MENumberOutOfRange::CheckInteger(0, 255, (int)id, M_OPT_STR("AUTHENTICATION_KEY_ID"));
   m_authenticationKeyId = (unsigned char)id;
}

#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST

void MProtocolC1221::ClearAuthenticationKeyList()
{
   m_authenticationKeyListSuccessfulEntry = -1;
   m_authenticationKeyList.clear();
}

void MProtocolC1221::SetAuthenticationKeyList(const MByteStringVector& authenticationKeyList)
{
   m_authenticationKeyListSuccessfulEntry = -1;

   MByteStringVector::const_iterator it = authenticationKeyList.begin();
   MByteStringVector::const_iterator itEnd = authenticationKeyList.end();
   for ( ; it != itEnd; ++it )
      DoVerifyAuthenticationKey(*it);

   MAes::AssignSecureData(m_authenticationKeyList, authenticationKeyList);
}

void MProtocolC1221::AddToAuthenticationKeyList(const MByteString& key)
{
   m_authenticationKeyListSuccessfulEntry = -1;
   DoVerifyAuthenticationKey(key);
   m_authenticationKeyList.push_back(key);
}

int MProtocolC1221::GetAuthenticationKeyListSuccessfulEntry() const
{
   return m_authenticationKeyListSuccessfulEntry;
}

#endif // !M_NO_MCOM_PASSWORD_AND_KEY_LIST

void MProtocolC1221::SetIdentity(unsigned id)
{
   if ( m_identity != id ) // we go to another device
   {
      MENumberOutOfRange::CheckInteger(0, 255, int(id), M_OPT_STR("IDENTITY"));
      m_identity = id;
   }
}

void MProtocolC1221::SetDataFormat(DataFormatEnum dataFormat)
{
   MENumberOutOfRange::CheckNamedUnsignedRange(0, 3, (unsigned)dataFormat, M_OPT_STR("DATA_FORMAT"));
   m_dataFormat = (Muint8)dataFormat;
   DoSetMaximumApplicationLayerPacketSize(); // because C12.21 sizes depend on it
}


int MProtocolC1221::GetIdentifiedAuthenticationAlgorithm() const
{
   if ( !m_identifiedPropertiesPresent )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_IDENTIFIED_INFORMATION_IS_NOT_AVAILABLE, "ANSI Identify service was not called, identified information is not available"));
      M_ENSURED_ASSERT(0);
   }
   if ( !m_canAuthenticate )
      return -1; // convention
   return (int)(unsigned)m_authenticationAlgorithm;
}

void MProtocolC1221::ApplyChannelParameters()
{
   MProtocolC1218::ApplyChannelParameters();
   m_canAuthenticate = false;
}

void MProtocolC1221::Identify()
{
   m_canAuthenticate = false;
   m_authenticationAlgorithm = ALGORITHM_DES;
   m_authenticationTicket.clear();
   m_receiveToggleBitKnown = false;
   m_identifiedPropertiesPresent = false;
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("Identify"), MProtocolServiceWrapper::ServiceNotQueueable);
   try
   {
      DoApplicationLayerRequestForIdentify();
      m_identifiedReferenceStandard = ReceiveServiceByte();
      m_identifiedStandardVersion = ReceiveServiceByte();
      m_identifiedStandardRevision = ReceiveServiceByte();
      m_identifiedPropertiesPresent = true;

      for ( ;; )
      {
         unsigned len;
         int feature = ReceiveServiceByte();
         switch ( feature )
         {
         case 0x00:
            goto BREAK_OUT_OF_LOOP;
         case 0x01: // <auth_ser> ::= 0x01<auth_type><auth_alg_id>
            m_canAuthenticate = ((ReceiveServiceByte() & 0x01) != 0);
            m_authenticationAlgorithm = ReceiveServiceByte();
            break;
         case 0x02: // <auth_ser_ticket> ::= 0x02<auth_type><auth_alg_id><ticket_length><ticket>
            m_canAuthenticate = ((ReceiveServiceByte() & 0x01) != 0);
            m_authenticationAlgorithm = ReceiveServiceByte();
            len = (unsigned)ReceiveServiceByte();
            m_applicationLayerReader.ReadBytes(len, m_authenticationTicket);
            break;
         case 0x06: // <device-class> ::= 0x06<universal-id>
            ReceiveServiceByte(); // read either absolute or relative UID element
            len = (unsigned)ReceiveServiceByte();
            m_applicationLayerReader.IgnoreBytes(len); // ignore device class for now
            break;
         case 0x07: // <device-identity> ::= 0x07<identity-length><identity>
            len = (unsigned)ReceiveServiceByte();
            m_applicationLayerReader.IgnoreBytes(len); // ignore identity for now
            break;
         default:
            MCOMException::Throw(MException::ErrorMeter, M_CODE_STR_P1(M_ERR_IDENTIFY_FAILED_GOT_UNRECOGNIZED_FEATURE_CODE_X1, M_I("Identify protocol request failed, got unrecognized feature code 0x%02X"), feature));
            M_ENSURED_ASSERT(0);
         }
      }
   BREAK_OUT_OF_LOOP:
      ;
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

   inline unsigned char DoConvertThousandsIntoByte(unsigned val)
   {
      if ( val < 255000u )
         val = ((val + 999u) / 1000u);
      else
         val = 255u; // return maximum possible value - too big to care already
      return (unsigned char)val;
   }

void MProtocolC1221::TimingSetup()
{
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("TimingSetup"), MProtocolServiceWrapper::ServiceNotQueueable);
   try
   {
      struct TimeoutPacket
      {
         unsigned char m_channelTrafficTimeout;
         unsigned char m_intercharacterTimeout;
         unsigned char m_acknowledgementTimeout;
         unsigned char m_linkLayerRetries;
      } timeoutPacket;

      // Limit the values to be sent to the meter
      //
      timeoutPacket.m_channelTrafficTimeout  = DoConvertThousandsIntoByte(m_channelTrafficTimeout);
      timeoutPacket.m_intercharacterTimeout  = DoConvertThousandsIntoByte(m_intercharacterTimeout);
      timeoutPacket.m_acknowledgementTimeout = DoConvertThousandsIntoByte(m_acknowledgementTimeout);
      if ( m_linkLayerRetries < 255 )
         timeoutPacket.m_linkLayerRetries = (unsigned char)m_linkLayerRetries;
      else
         timeoutPacket.m_linkLayerRetries = 255;

      MByteString request((const char*)&timeoutPacket, sizeof(TimeoutPacket));
      DoApplicationLayerRequest('\x71', &request);

      m_channelTrafficTimeout = (unsigned)ReceiveServiceByte() * 1000u;
      m_intercharacterTimeout = (unsigned)ReceiveServiceByte() * 1000u;
      m_acknowledgementTimeout = (unsigned)ReceiveServiceByte() * 1000u;
      m_linkLayerRetries = (unsigned)ReceiveServiceByte();
   }
   catch ( MException& ex )
   {
      // Some devices do not set toggle bit correctly after Authentication application layer failure.
      //
      m_receiveToggleBitKnown = false;

      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

void MProtocolC1221::TimingSetupWithWorkaround()
{
   if ( m_linkLayerRetries != 0 )
      TimingSetup(); // no workaround is necessary for nonzero link layer retries
   else
   {
      // here we have m_linkLayerRetries at zero!
      TimingSetup();
      if ( m_linkLayerRetries > 1 ) // buggy device discovered
      {
         m_linkLayerRetries = 1; // work it around
         TimingSetup();
         if ( m_linkLayerRetries == 1 ) // if worked around, means the device thinks it is "no-retries"
            m_linkLayerRetries = 0;     // we have successfully negotiated zero retries with both the device and MeteringSDK
      }
   }
}

void MProtocolC1221::Negotiate()
{
   MValueSavior<unsigned> savior(&m_sessionBaud, 0); // do not have session baud in Negotiate
   MProtocolC1218::Negotiate();
}

void MProtocolC1221::Authenticate()
{
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   m_authenticationKeyListSuccessfulEntry = -1;
#endif
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("Authenticate"), MProtocolServiceWrapper::ServiceNotQueueable);
   try
   {
      if ( !m_canAuthenticate )
      {
         if ( !m_identifiedPropertiesPresent )
            MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_IDENTIFIED_INFORMATION_IS_NOT_AVAILABLE, "Identify was not issued"));
         else
            MCOMException::Throw(MException::ErrorMeter, M_CODE_STR(M_ERR_METER_DOES_NOT_SUPPORT_AUTHENTICATION, M_I("Meter does not support authentication")));
         M_ENSURED_ASSERT(0);
      }

#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST

      if ( m_authenticationKeyList.empty() )
         DoTryAuthenticationKeyEntry(m_authenticationKey); // use authenticationKey property directly
      else
      {
         unsigned num = unsigned(m_authenticationKeyList.size());
         for ( unsigned i = 0; i < num; ++i )
         {
            try
            {
               DoTryAuthenticationKeyEntry(m_authenticationKeyList[i]); // use entry in the authenticationKey list
               m_authenticationKeyListSuccessfulEntry = (int)i;
               return; // success
            }
            catch ( MEC12NokResponse& ex )
            {
               if ( ex.GetResponseCode() == MEC12NokResponse::RESPONSE_RNO ) // always throw in case of RNO
                  throw;
               MProtocolServiceWrapper::StaticNotifyOrThrowRetry(this, ex, i == num - 1 ? 0 : 1); // set retry count to nonzero, so we do not throw

               // Need to redo Identify/Logon in case of bad authentication.
               // Some devices require Logoff.
               //
               Logoff();
               Terminate();
               Identify();
               if ( m_issueTimingSetupOnStartSession )
                  TimingSetupWithWorkaround();
               if ( m_issueNegotiateOnStartSession )
                  Negotiate();
               Logon();
            }
         }
         M_ENSURED_ASSERT(0); // we are never here due to the condition under catch()
      }

#else // !M_NO_MCOM_PASSWORD_AND_KEY_LIST

      DoTryAuthenticationKeyEntry(m_authenticationKey); // use authenticationKey property directly

#endif // !M_NO_MCOM_PASSWORD_AND_KEY_LIST

   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

   static void DoCheckKeyAndTicketSize(const MByteString& key, const MByteString& ticket, unsigned size)
   {
      if ( key.size() != static_cast<size_t>(size) )
      {
         MCOMException::Throw(MException::ErrorMeter, M_CODE_STR_P1(M_ERR_PROTOCOL_IMPLEMENTATION_MISMATCH, M_I("Authentication key is not of %d-byte size"), size));
         M_ENSURED_ASSERT(0);
      }
      if ( ticket.size() != static_cast<size_t>(size) )
      {
         MCOMException::Throw(MException::ErrorMeter, M_CODE_STR_P1(M_ERR_PROTOCOL_IMPLEMENTATION_MISMATCH, M_I("Authentication ticket returned by devide is not of %d-byte size"), size));
         M_ENSURED_ASSERT(0);
      }
   }

void MProtocolC1221::DoTryAuthenticationKeyEntry(const MByteString& key)
{
   MByteString request; // 9, length of the request
   if ( m_authenticationAlgorithm == ALGORITHM_DES )
   {
      DoCheckKeyAndTicketSize(key, m_authenticationTicket, 8);
#ifdef M_USE_USTL
      request.append(1, '\x09');
#else
      request.assign(1, '\x09');
#endif

      Muint64 encryptedTicket;
      MDes::StaticEncryptBuffer(key.data(), reinterpret_cast<const char*>(m_authenticationTicket.data()), reinterpret_cast<char*>(&encryptedTicket));

      request += (char)m_authenticationKeyId;
      request.append((const char*)&encryptedTicket, 8);

      try
      {
         DoApplicationLayerRequest('\x53', &request);
      }
      catch ( MEC12NokResponse& ex )
      {
         // Some devices do not set toggle bit correctly after Authentication application layer failure.
         //
         m_receiveToggleBitKnown = false;

         if ( ex.GetResponseCode() == MEC12NokResponse::RESPONSE_ERR  )
            ex.SetKind(MException::ErrorSecurity); // override the type for C12 error
         throw;
      }

      char len = (char)ReceiveServiceByte();
      if ( len == char(9) ) // key id and token are correct
      {
         unsigned char keyIndex = ReceiveServiceByte();
         if ( keyIndex == m_authenticationKeyId )
         {
            MByteString encryptedMeterResponseByteString = ReceiveServiceBytes(8);

            Muint64 decryptedMeterResponse;
            MDes::StaticDecryptBuffer(key.data(), encryptedMeterResponseByteString.data(), reinterpret_cast<char*>(&decryptedMeterResponse));
            if ( decryptedMeterResponse == encryptedTicket )
               return; // correct authentication
         }
      }
   }
   else if ( m_authenticationAlgorithm == ALGORITHM_AES )
   {
      DoCheckKeyAndTicketSize(key, m_authenticationTicket, 16);
#ifdef M_USE_USTL
      request.append(1, '\x11');
#else
      request.assign(1, '\x11');
#endif

      MAesEax aes(key);
      request += (char)m_authenticationKeyId;
      MByteString encryptedTicket = aes.Encrypt(m_authenticationTicket);
      request += encryptedTicket;
      try
      {
         DoApplicationLayerRequest('\x53', &request);
      }
      catch ( MEC12NokResponse& ex )
      {
         if ( ex.GetResponseCode() == MEC12NokResponse::RESPONSE_ERR  )
            ex.SetKind(MException::ErrorSecurity); // override the type for C12 error
         throw;
      }
      char len = (char)ReceiveServiceByte();
      if ( len == char(17) ) // key id and token are of correct size
      {
         unsigned char keyIndex = ReceiveServiceByte();
         if ( keyIndex == m_authenticationKeyId )
         {
            MByteString encryptedMeterResponseByteString = ReceiveServiceBytes(16);
            MByteString decryptedMeterResponse = aes.Decrypt(encryptedMeterResponseByteString);
            if ( decryptedMeterResponse == encryptedTicket )
               return; // correct authentication
         }
      }
   }
   else
   {
      MCOMException::Throw(MException::ErrorMeter, M_CODE_STR(M_ERR_METER_REQUESTED_UNKNOWN_AUTHENTICATION_ALGORITHM, M_I("Meter requested unknown authentication algorithm")));
      M_ENSURED_ASSERT(0);
   }
   MCOMException::Throw(MException::ErrorSecurity, M_CODE_STR(MErrorEnum::DataNotValidated, M_I("Device failed authentication, tampering or fake device is possible")));
   M_ENSURED_ASSERT(0);
}

void MProtocolC1221::DoSetMaximumApplicationLayerPacketSize() M_NO_THROW
{
   if ( m_dataFormat == 0 )
      MProtocolC1218::DoSetMaximumApplicationLayerPacketSize();
   else
   {
      m_maximumReadTableSize         = m_negotiatedPacketSize - PACKET_HEADER_AND_FOOTER_LENGTH - READ_SERVICE_OVERHEAD;
      m_maximumWriteTableSize        = m_negotiatedPacketSize - PACKET_HEADER_AND_FOOTER_LENGTH - WRITE_SERVICE_OVERHEAD;
      m_maximumPartialWriteTableSize = m_negotiatedPacketSize - PACKET_HEADER_AND_FOOTER_LENGTH - PARTIAL_WRITE_SERVICE_OVERHEAD;
   }
}

void MProtocolC1221::DoApplicationLayerRequest(char command, const MByteString* request, unsigned flags)
{
   if ( m_dataFormat == 0 )
      MProtocolC1218::DoApplicationLayerRequest(command, request, flags);
   else
   {
      m_incomingDataFormat = 0; // Client resets this at every interaction
      if ( request != NULL && request->size() > m_negotiatedPacketSize - PACKET_HEADER_AND_FOOTER_LENGTH - 1 ) // -1 because of <command>, part of request
      {
         MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_REQUEST_LENGTH_EXCEEDS_C1222_DATA_FORMAT_PACKET_SIZE, "Request length exceeds packet size of C12.22 data format"));
         M_ENSURED_ASSERT(0);
      }

      char* packet = DoGetPacketBuffer();
      for ( unsigned appRetryCount = m_applicationLayerRetries; ; --appRetryCount )
      {
         m_applicationLayerIncoming.Clear();

         bool doNotSendOutgoingPacket = false;
         MProtocolLinkLayerWrapper wrapper(this);
         for ( unsigned linkRetryCount = m_linkLayerRetries; ; --linkRetryCount ) // sequence that handles toggle bit failures
         {
            try
            {
               if ( !doNotSendOutgoingPacket )
               {
                  packet[0] = CHAR_START;
                  packet[1] = (char)m_identity;

                  Muint8 controlByte = m_dataFormat;
                  if ( m_nextOutgoingToggleBit )
                     controlByte |= 0x20u;
                  packet[2] = (char)(controlByte);
                  packet[3] = '\0'; // sequence number
                  packet[6] = command;

                  unsigned fullApplicationDataSize = 1; // command size
                  if ( request != NULL )                   // plus request size, if the request is present
                  {
                     fullApplicationDataSize += M_64_CAST(unsigned, request->size());
                     memcpy(packet + 7, request->data(), request->size());
                  }
                  MToBigEndianUINT16(fullApplicationDataSize, packet + 4);
                  unsigned packetSizeWithNoCRC = fullApplicationDataSize + 6;
                  Muint16 crc = StaticCalculateCRC16FromBuffer(packet, packetSizeWithNoCRC);
                  memcpy(packet + packetSizeWithNoCRC, (const char*)&crc, 2); // append crc
                  const unsigned packetSize = packetSizeWithNoCRC + 2;

                  Sleep(m_turnAroundDelay);
                  m_channel->WriteBuffer(packet, packetSize);
                  m_channel->FlushOutputBuffer(packetSize);

               }
               doNotSendOutgoingPacket = false;

               packet[0] = DoReadStartCharacter("\x06\x15\xEE", m_acknowledgementTimeout, 2); // 2 means that only "\x06" and "\x15" are turn around characters
               if ( packet[0] == CHAR_NAK )
               {
                  m_channel->ClearInputBuffer();
                  MCOMException::Throw(M_CODE_STR_P2(M_ERR_EXPECTED_X1_GOT_X2, M_I("Expected character 0x%02X, received 0x%02X"), (unsigned)(unsigned char)CHAR_ACK, (unsigned)(unsigned char)packet[0]));
                  M_ENSURED_ASSERT(0);
               }
               if ( packet[0] == CHAR_ACK )
               {

                  // if waiting for ST_007 Write answer for the first time - change linkRetryCount so that timeout is procedureInitiateTimeout
                  // new linkLayerRetries value is not smaller than the old value (to avoid the reduction of link layer retries)
                  if ( m_isST007Write && linkRetryCount == m_linkLayerRetries && ((m_procedureInitiateTimeout / m_acknowledgementTimeout) > m_linkLayerRetries) )
                     linkRetryCount = m_procedureInitiateTimeout / m_acknowledgementTimeout;

                  packet[0] = DoReadStartCharacter("\xEE", m_acknowledgementTimeout, 0); // none are turn around characters
               }

               M_ASSERT(packet[0] == '\xEE');

               MChannel::ReadTimeoutSavior timeoutSavior(m_channel, m_intercharacterTimeout);

               m_channel->ReadBuffer(packet + 1, 5); // rsvd, ctrl, seqn, lenh, lenl
               unsigned dataLength = MFromBigEndianUINT16(packet + 4);
               if ( dataLength > m_negotiatedPacketSize - 8 ) // length in the packet packet is good...
               {
                  unsigned badPacketSleep = m_acknowledgementTimeout;
                  if ( badPacketSleep > MAXIMUM_BAD_PACKET_LENGTH_SLEEP )
                     badPacketSleep = MAXIMUM_BAD_PACKET_LENGTH_SLEEP;
                  Sleep(badPacketSleep);
                  m_channel->ClearInputBuffer();
                  MCOMException::Throw(M_CODE_STR(M_ERR_INBOUND_PACKET_DATA_LENGTH_IS_BAD, M_I("Inbound packet data length is bad")));
                  M_ENSURED_ASSERT(0);
               }

               m_channel->ReadBuffer(packet + 6, dataLength + 2);
               Sleep(m_turnAroundDelay);

               char incomingAckNak = (packet[2] & '\x0C');

               unsigned packetSizeWithNoCRC = dataLength + 6;
               Muint16 crc = MToAlignedUINT16(packet + packetSizeWithNoCRC);
               if ( crc != StaticCalculateCRC16FromBuffer(packet, packetSizeWithNoCRC) ) // successful packet
               {
                  if ( incomingAckNak == '\x00' )
                  {
                     doNotSendOutgoingPacket = true;
                     m_channel->WriteChar(CHAR_NAK); // <NAK>, the packet was not received
                  }
                  MCOMException::Throw(M_CODE_STR(MErrorEnum::CrcCheckFailed, M_I("CRC check failed")));
                  M_ENSURED_ASSERT(0);
               }

               if ( incomingAckNak == '\x00' )
                  m_channel->WriteChar(CHAR_ACK);
               else if ( incomingAckNak != '\x04' ) // NAK or invalid packet
               {
                  MCOMException::Throw(M_CODE_STR(MErrorEnum::DeviceReportedBadPacketCRC, M_I("Device reported bad packet CRC")));
                  M_ENSURED_ASSERT(0);
               }
               Muint8 ctrl = packet[2];
               if ( m_receiveToggleBitKnown &&                     //   toggle bit is known
                    m_receiveToggleBit == ((ctrl & 0x20) != 0) )   //   and it is the same as one of the previous packets
               {
                  m_receiveToggleBitKnown = false; // starting from this moment we do not know the correct toggle bit

                  unsigned toggleBitSleep = m_acknowledgementTimeout;
                  if ( toggleBitSleep > MAXIMUM_BAD_TOGGLE_BIT_SLEEP )
                     toggleBitSleep = MAXIMUM_BAD_TOGGLE_BIT_SLEEP;
                  Sleep(toggleBitSleep);
                  m_channel->ClearInputBuffer();
                  if ( m_savedCRC == crc )                            //   and in fact the CRC is correct and the same - in this case we definitely have a duplicate packet
                     MCOMException::Throw(M_CODE_STR(MErrorEnum::ReceivedPacketToggleBitFailure, M_I("Received packet toggle bit failure, duplicate packet ignored")));
                  else
                     MCOMException::Throw(M_CODE_STR(MErrorEnum::ReceivedPacketToggleBitFailure, M_I("Packet is bad or received out of sequence")));
                  M_ENSURED_ASSERT(0);
               }
               if ( m_checkIncomingToggleBit )
               {
                  M_ASSERT(!m_receiveToggleBitKnown || m_receiveToggleBit != ((ctrl & 0x20) != 0));
                  m_savedCRC = crc;
                  m_receiveToggleBitKnown = true; // make it true only if m_checkIncomingToggleBit is true!
                  m_receiveToggleBit = ((ctrl & 0x20) != 0); // record the toggle bit

                  #if defined(M__EMULATE_PERIODIC_TOGGLE_BIT_FAILURE_RATE) && (M__EMULATE_PERIODIC_TOGGLE_BIT_FAILURE_RATE) != 0
                     M_COMPILED_ASSERT((M__EMULATE_PERIODIC_TOGGLE_BIT_FAILURE_RATE) >= 5); // make sure the rate is reasonable, guarantees some success
                     if ( MMath::Rand() % (M__EMULATE_PERIODIC_TOGGLE_BIT_FAILURE_RATE) == 0 ) // make every 15-th packet in average
                        m_receiveToggleBit = !m_receiveToggleBit;
                  #endif
               }
               m_applicationLayerIncoming.Assign(packet + 7, dataLength);
               break; // success
            }
            catch ( MException& ex ) // excluding timeout exception...
            {
               wrapper.NotifyOrThrowRetry(ex, linkRetryCount);
            }
         }
         m_nextOutgoingToggleBit = !m_nextOutgoingToggleBit;
         MEC12NokResponse::ResponseCode responseCode = (MEC12NokResponse::ResponseCode)packet[6];
         if ( responseCode == MEC12NokResponse::RESPONSE_OK )
         {
            m_applicationLayerReader.AssignBuffer(&m_applicationLayerIncoming);
            return; // success
         }

         MByteString extraParameters;
         m_applicationLayerReader.ReadRemainingBytes(extraParameters); // this can read zero bytes
         MEC12NokResponse ex((Muint8)responseCode, extraParameters);
         bool retryCondition = responseCode == MEC12NokResponse::RESPONSE_BSY
                           || responseCode == MEC12NokResponse::RESPONSE_DNR
                           ;
         DoCheckCodeTerminateAndThrowOrNotify(ex, retryCondition, appRetryCount, true, this);
      }
   }
}

#endif // !M_NO_MCOM_PROTOCOL_C1221
