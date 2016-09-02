// File MCOM/ProtocolC1218.cpp

#include "MCOMExtern.h"
#include "ChannelOpticalProbe.h"
#include "ChannelModem.h"
#include "MCOMExceptions.h"
#include "ProtocolC1218.h"

#if !M_NO_MCOM_PROTOCOL_C1218

M_START_PROPERTIES(ProtocolC1218)
   M_OBJECT_PROPERTY_PERSISTENT_UINT(ProtocolC1218, PacketSize,                  1024u) // C12 default is actually 64
   M_OBJECT_PROPERTY_PERSISTENT_UINT(ProtocolC1218, SessionBaud,                 9600u)
   M_OBJECT_PROPERTY_PERSISTENT_UINT(ProtocolC1218, ChannelTrafficTimeout,       6000u)
   M_OBJECT_PROPERTY_PERSISTENT_UINT(ProtocolC1218, MaximumNumberOfPackets,       255u) // C12 default is actually 1
   M_OBJECT_PROPERTY_PERSISTENT_BOOL(ProtocolC1218, IssueNegotiateOnStartSession, true)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL(ProtocolC1218, IssueLogoffOnEndSession,      true)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL(ProtocolC1218, WakeUpSharedOpticalPort,     false)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL(ProtocolC1218, CheckIncomingToggleBit,       true)
   M_OBJECT_PROPERTY_PERSISTENT_UINT(ProtocolC1218, IntercharacterTimeout,        500u)
   M_OBJECT_PROPERTY_PERSISTENT_UINT(ProtocolC1218, AcknowledgementTimeout,      2000u)
   M_OBJECT_PROPERTY_PERSISTENT_UINT(ProtocolC1218, ProcedureInitiateTimeout,   20000u)
   M_OBJECT_PROPERTY_BOOL           (ProtocolC1218, NextOutgoingToggleBit)
   M_OBJECT_PROPERTY_READONLY_INT   (ProtocolC1218, IdentifiedReferenceStandard)
   M_OBJECT_PROPERTY_READONLY_INT   (ProtocolC1218, IdentifiedStandardVersion)
   M_OBJECT_PROPERTY_READONLY_INT   (ProtocolC1218, IdentifiedStandardRevision)
   M_OBJECT_PROPERTY_READONLY_BOOL  (ProtocolC1218, IdentifiedPropertiesPresent)
   M_OBJECT_PROPERTY_READONLY_UINT  (ProtocolC1218, NegotiatedPacketSize)
   M_OBJECT_PROPERTY_READONLY_UINT  (ProtocolC1218, NegotiatedMaximumNumberOfPackets)
   M_OBJECT_PROPERTY_READONLY_UINT  (ProtocolC1218, NegotiatedSessionBaud)
   M_OBJECT_PROPERTY_READONLY_BOOL  (ProtocolC1218, NegotiatedPropertiesPresent)
M_START_METHODS(ProtocolC1218)
   M_OBJECT_SERVICE(ProtocolC1218, Identify,    ST_X)
   M_OBJECT_SERVICE(ProtocolC1218, Negotiate,   ST_X)
   M_OBJECT_SERVICE(ProtocolC1218, ServerStart, ST_X)
   M_OBJECT_SERVICE(ProtocolC1218, ServerEnd,   ST_X_byte_constMByteStringA)
M_END_CLASS_TYPED(ProtocolC1218, ProtocolC12, "PROTOCOL_ANSI_C12_18")

   const char CHAR_START = '\xEE';
   const char CHAR_ACK = '\x06';
   const char CHAR_NAK = '\x15';

   using namespace std;

MProtocolC1218::MProtocolC1218(MChannel *channel, bool channelIsOwned)
:
   MProtocolC12(channel, channelIsOwned),
   m_identifiedPropertiesPresent(false),
   m_identifiedReferenceStandard('\xFF'),
   m_identifiedStandardVersion('\xFF'),
   m_identifiedStandardRevision('\xFF'),
   m_negotiatedPropertiesPresent(false),
   m_initialBaud(9600u),
   m_identity(0),
   m_incomingIdentity(0),
   m_dataFormat(0),
   m_incomingDataFormat(0),
   m_nextOutgoingToggleBit(false),
   m_receiveToggleBit(false),
   m_receiveToggleBitKnown(false),
   m_negotiatedMaximumNumberOfPackets(128), // pacify Valgrind's uninitialized variable. This value can be anything, will be overwritten.
   m_dataLinkPacketBuffer(NULL)
{
   M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(ProtocolC1218);
   M_ASSERT(m_negotiatedPacketSize == m_packetSize);                                      // need to keep those in sync before the communication
   M_ASSERT(m_packetSize >= SMALLEST_PACKET_SIZE && m_packetSize <= BIGGEST_PACKET_SIZE); // and in range
}

MProtocolC1218::~MProtocolC1218()
{
   Finalize();
   delete [] m_dataLinkPacketBuffer;
}

void MProtocolC1218::SetSessionBaud(unsigned sessionBaud)
{
   if ( sessionBaud != 0 )
      (void)DoConvertBaudToIndex(sessionBaud); // By this means check validity of nonzero value
   m_sessionBaud = sessionBaud;
   m_negotiatedSessionBaud = sessionBaud;
}

void MProtocolC1218::SetIntercharacterTimeout(unsigned timeout)
{
   MENumberOutOfRange::CheckNamedUnsignedRange(0, 255000, timeout, M_OPT_STR("INTERCHARACTER_TIMEOUT"));
   m_intercharacterTimeout = timeout;
}

void MProtocolC1218::SetAcknowledgementTimeout(unsigned timeout)
{
   MENumberOutOfRange::CheckNamedUnsignedRange(0, 255000, timeout, M_OPT_STR("ACKNOWLEDGEMENT_TIMEOUT"));
   m_acknowledgementTimeout = timeout;
}

void MProtocolC1218::SetChannelTrafficTimeout(unsigned timeout)
{
   MENumberOutOfRange::CheckNamedUnsignedRange(0, 255000, timeout, M_OPT_STR("CHANNEL_TRAFFIC_TIMEOUT"));
   m_channelTrafficTimeout = timeout;
}

void MProtocolC1218::SetPacketSize(unsigned packetSize)
{
   MENumberOutOfRange::CheckInteger(SMALLEST_PACKET_SIZE, BIGGEST_PACKET_SIZE, int(packetSize), M_OPT_STR("PACKET_SIZE"));
   m_packetSize = packetSize;
   DoSetNegotiatedPacketSize(packetSize);
}

void MProtocolC1218::SetMaximumNumberOfPackets(unsigned num)
{
   MENumberOutOfRange::CheckInteger(1, 255, int(num), M_OPT_STR("MAXIMUM_NUMBER_OF_PACKETS"));
   m_maximumNumberOfPackets = num;
   m_negotiatedMaximumNumberOfPackets = num;
   DoSetMaximumApplicationLayerPacketSize();
}

int MProtocolC1218::GetIdentifiedReferenceStandard() const
{
   if ( !m_identifiedPropertiesPresent )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_IDENTIFIED_INFORMATION_IS_NOT_AVAILABLE, "ANSI Identify service was not called, identified information is not available"));
      M_ENSURED_ASSERT(0);
   }
   return (int)(unsigned char)m_identifiedReferenceStandard;
}

int MProtocolC1218::GetIdentifiedStandardVersion() const
{
   if ( !m_identifiedPropertiesPresent )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_IDENTIFIED_INFORMATION_IS_NOT_AVAILABLE, "ANSI Identify service was not called, identified information is not available"));
      M_ENSURED_ASSERT(0);
   }
   return (int)(unsigned char)m_identifiedStandardVersion;
}

int MProtocolC1218::GetIdentifiedStandardRevision() const
{
   if ( !m_identifiedPropertiesPresent )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_IDENTIFIED_INFORMATION_IS_NOT_AVAILABLE, "ANSI Identify service was not called, identified information is not available"));
      M_ENSURED_ASSERT(0);
   }
   return (int)(unsigned char)m_identifiedStandardRevision;
}

unsigned MProtocolC1218::GetNegotiatedPacketSize() const
{
   if ( !m_negotiatedPropertiesPresent )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_NEGOTIATED_INFORMATION_IS_NOT_AVAILABLE, "ANSI Negotiate service was not called, negotiated information is not available"));
      M_ENSURED_ASSERT(0);
   }
   return m_negotiatedPacketSize;
}

unsigned MProtocolC1218::GetNegotiatedMaximumNumberOfPackets() const
{
   if ( !m_negotiatedPropertiesPresent )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_NEGOTIATED_INFORMATION_IS_NOT_AVAILABLE, "ANSI Negotiate service was not called, negotiated information is not available"));
      M_ENSURED_ASSERT(0);
   }
   return m_negotiatedMaximumNumberOfPackets;
}

unsigned MProtocolC1218::GetNegotiatedSessionBaud() const
{
   if ( !m_negotiatedPropertiesPresent )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_NEGOTIATED_INFORMATION_IS_NOT_AVAILABLE, "ANSI Negotiate service was not called, negotiated information is not available"));
      M_ENSURED_ASSERT(0);
   }
   return m_negotiatedSessionBaud;
}

void MProtocolC1218::DoApplicationLayerRequestForIdentify()
{
   DoApplicationLayerRequest('\x20');

   // Now cure an often cause of failure when a previous session produced a duplicate packet.
   // At the session start, we cannot use toggle bit for identifying such packet,
   // therefore try to identify it by application layer's length.
   //
   static const unsigned s_minimumIdentifyResponseSize = 3;  // Identify response is at least three bytes
   if ( m_applicationLayerIncoming.GetTotalSize() < s_minimumIdentifyResponseSize )
   {
      MProtocolLinkLayerWrapper wrapper(this);
      for ( int retries = m_linkLayerRetries; ; --retries )
      {
         MCOMException ex(M_CODE_STR(MErrorEnum::ReceivedPacketToggleBitFailure, M_I("Packet was likely produced by a previous session")));
         wrapper.NotifyOrThrowRetry(ex, retries);

         m_applicationLayerIncoming.Clear();
         m_receiveToggleBitKnown = false;
         MEC12NokResponse::ResponseCode code = DoApplicationLayerRead();
         if ( code != MEC12NokResponse::RESPONSE_OK )
            MEC12NokResponse::Throw(code);
         if ( m_applicationLayerIncoming.GetTotalSize() >= 3 )
            break; // success
      }
   }
}

void MProtocolC1218::Identify()
{
   m_receiveToggleBitKnown = false; // we don't know the toggle bit at this state
   m_identifiedPropertiesPresent = false;
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("Identify"), MProtocolServiceWrapper::ServiceNotQueueable);
   try
   {
      DoApplicationLayerRequestForIdentify();
      m_identifiedReferenceStandard = ReceiveServiceByte();
      m_identifiedStandardVersion = ReceiveServiceByte();
      m_identifiedStandardRevision = ReceiveServiceByte();
      m_identifiedPropertiesPresent = true;
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

void MProtocolC1218::Negotiate()
{
   m_negotiatedPropertiesPresent = false;
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("Negotiate"), MProtocolServiceWrapper::ServiceNotQueueable);
   try
   {
      Muint16 num16 = MToBigEndianUINT16(m_packetSize);
      MByteString request((const char*)&num16, 2);
      request += (char)(unsigned char)m_maximumNumberOfPackets;
      if ( m_sessionBaud == 0 )
         DoApplicationLayerRequest('\x60', &request);
      else
      {
         request += (char)(unsigned char)DoConvertBaudToIndex(m_sessionBaud);
         DoApplicationLayerRequest('\x61', &request);
      }

      unsigned newNegotiatedPacketSize = ReceiveServiceUInt(2); // two bytes for negotiate size
      m_negotiatedMaximumNumberOfPackets = (unsigned)ReceiveServiceByte(); // third byte is maximum number of packets
      if ( m_sessionBaud == 0 || m_applicationLayerReader.GetRemainingReadSize() == 0 )
         m_negotiatedSessionBaud = 0; // signal the baud was not negotiated
      else
      {
         m_negotiatedSessionBaud = DoConvertIndexToBaud(ReceiveServiceByte()); // last byte is session baud
         if ( m_negotiatedSessionBaud != m_initialBaud )
            DoSetBaudIfOpticalProbe(m_negotiatedSessionBaud);
      }
      DoSetNegotiatedPacketSize(newNegotiatedPacketSize); // do it after m_negotiatedMaximumNumberOfPackets! It assigns to maximum application layer packet size!
      m_negotiatedPropertiesPresent = true;
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}


#if !M_NO_MCOM_IDENTIFY_METER
MStdString MProtocolC1218::DoIdentifyMeter(bool sessionIsStarted, TableRawDataVector* tablesRead)
{
   MStdString result;
   if ( !sessionIsStarted )
   {
      ApplyChannelParameters(); // need to do so, case if the session is started several times without reconnecting
      m_identifiedPropertiesPresent = false;
      Identify();
      Logon();
   }
   result = MProtocolC12::DoIdentifyMeter(sessionIsStarted, tablesRead);
   if ( !sessionIsStarted )
      EndSession();
   return result;
}
#endif // !M_NO_MCOM_IDENTIFY_METER

void MProtocolC1218::DoStartSession()
{
#if !M_NO_PROGRESS_MONITOR
   MProgressAction* action = GetLocalProgressAction();
#endif

   ApplyChannelParameters(); // need to do so, case if the session is started several times without reconnecting

   // In case of StartSession, reset the read-only properties
   m_identifiedPropertiesPresent = false;
   m_negotiatedPropertiesPresent = false;
   if ( m_wakeUpSharedOpticalPort )
   {
      m_channel->WriteChar('\x55'); 
      MUtilities::Sleep(50);
   }

#if !M_NO_PROGRESS_MONITOR
   action->SetProgress(5.0);
#endif

   Identify();

#if !M_NO_PROGRESS_MONITOR
   action->SetProgress(30.0);
#endif

   // Special case, since Negotiate is not reliable, try it twice with the attempt to logon,
   // which could also fail due to negotiate
   //
   try
   {
      if ( m_issueNegotiateOnStartSession )
         Negotiate();

#if !M_NO_PROGRESS_MONITOR
      action->SetProgress(50.0);
#endif

      Logon();
   }
   catch ( MEChannelDisconnectedUnexpectedly& )
   {
      throw; // do not retry if operation is cancelled by the channel disconnect
   }
   catch ( MEOperationCancelled& )
   {
      throw; // do not repeat if the operation is cancelled by the user
   }
   catch ( MEC12NokResponse& )
   {
      throw;
   }
   catch ( MException& )
   {
      if ( !m_issueNegotiateOnStartSession || m_sessionBaud == m_initialBaud ) // if baud was not attempted to session baud and initial baud are the same, do not do any retries
      {
         if ( m_endSessionOnApplicationLayerError )
            DoEndSessionOnApplicationLayerError(true);
         throw;
      }

      // retry one more time out of try/catch sequence, starting from Identify
      MUtilities::Sleep(7000);
      ApplyChannelParameters(); // change the baud back
      Identify();
      Negotiate();
      Logon();
   }

#if !M_NO_PROGRESS_MONITOR
   action->SetProgress(70.0);
#endif

   if ( m_issueSecurityOnStartSession )
      FullLogin();


#if !M_NO_PROGRESS_MONITOR
   action->Complete();
#endif
}

void MProtocolC1218::DoEndSession()
{
   // Do not wrap it with try/catch
   if ( m_issueLogoffOnEndSession )
      Logoff();
   Terminate();
   m_nextOutgoingToggleBit = false;
}

void MProtocolC1218::DoApplicationLayerRequest(char command, const MByteString* request, unsigned flags)
{
   m_incomingDataFormat = 0; // Client resets this at every interaction
   for ( unsigned appRetryCount = m_applicationLayerRetries; ; --appRetryCount )
   {
      MEC12NokResponse::ResponseCode responseCode;
      for ( unsigned linkRetryCount = m_linkLayerRetries; ; --linkRetryCount ) // sequence that handles toggle bit failures (this has to be inside app-layer loop)
      {
         m_applicationLayerIncoming.Clear();

         bool writeSucceeded = DoApplicationLayerWrite(command, request);


         MValueSavior<unsigned> linkLayerRetriesSavior(&m_linkLayerRetries);
         if ( m_isST007Write && ((m_procedureInitiateTimeout / m_acknowledgementTimeout) > m_linkLayerRetries) )
         {
            // recounting link layer retries so that linkLayerRetries *  acknowledgementTimeout = procedureInitiateTimeout
            // new linkLayerRetries value is not smaller than the old value (to avoid the reduction of link layer retries)
            m_linkLayerRetries = m_procedureInitiateTimeout / m_acknowledgementTimeout;
         }
         responseCode = DoApplicationLayerRead();
         if ( writeSucceeded )
         {
            if ( responseCode == MEC12NokResponse::RESPONSE_OK )
            {
               m_applicationLayerReader.AssignBuffer(&m_applicationLayerIncoming, 1, m_applicationLayerIncoming.GetTotalSize()); // skip the first byte, status
               return; // success
            }
            if ( responseCode != (MEC12NokResponse::ResponseCode) - 1 ) // toggle bit failure, dropped multipacket packet, etc. - retry the link layer
               break; // otherwise retry the app-layer
            if ( linkRetryCount == 0 )
            {
               MCOMException::Throw(M_CODE_STR(MErrorEnum::ReceivedPacketToggleBitFailure, M_I("Link layer retries expired with received packet toggle bit failure due to duplicate packet")));
               M_ENSURED_ASSERT(0);
            }
            else
            {
               m_receiveToggleBitKnown = false;
               unsigned toggleBitSleep = m_acknowledgementTimeout;
               if ( toggleBitSleep > MAXIMUM_BAD_TOGGLE_BIT_SLEEP )
                  toggleBitSleep = MAXIMUM_BAD_TOGGLE_BIT_SLEEP;
               Sleep(toggleBitSleep);
               // and retry
            }
         }
         else if ( linkRetryCount == 0 )  // write did not succeed, and we've ran out of retries at this level
         {
            MCOMException::Throw(M_CODE_STR(M_ERR_EXPECTED_X1_GOT_X2, M_I("Packet received without prior acknowledgement")));
            M_ENSURED_ASSERT(0);
         }
      }

      MByteString extraParameters;
      m_applicationLayerReader.ReadRemainingBytes(extraParameters); // this can read zero chars

      MEC12NokResponse ex((Muint8)responseCode, extraParameters);
      bool retryCondition = responseCode == MEC12NokResponse::RESPONSE_BSY
                         || responseCode == MEC12NokResponse::RESPONSE_DNR
                         ;
      DoCheckCodeTerminateAndThrowOrNotify(ex, retryCondition, appRetryCount, true, this);
   }
}

bool MProtocolC1218::DoApplicationLayerWrite(char command, const MByteString* data)
{
   unsigned packetSizeWithNoCRC = m_negotiatedPacketSize - 2; // can be made smaller
   unsigned chunkSize = packetSizeWithNoCRC - 6; // minus header
   unsigned fullApplicationDataSize = 1; // command size
   if ( data != NULL )                   // plus data size, if the data is present
      fullApplicationDataSize += M_64_CAST(unsigned, data->size());

   if ( m_dataFormat && m_incomingDataFormat && chunkSize < fullApplicationDataSize ) // in this case we cannot send the whole packet, so we send ONP
      MEC12NokResponse::Throw(MEC12NokResponse::RESPONSE_ONP);

   unsigned numPackets = fullApplicationDataSize / chunkSize; // command plus data size
   if ( fullApplicationDataSize % chunkSize )
      numPackets++;
   bool multiPacketTransmission = (numPackets > 1);

   bool eeReceived = false;
   char* packet = DoGetPacketBuffer();
   packet[0] = CHAR_START;
   packet[1] = (char)m_identity;
   for ( unsigned index = 0; index != fullApplicationDataSize; )
   {
      unsigned controlByte;
      if ( !multiPacketTransmission )
      {
         if ( m_dataFormat != 0 && m_incomingDataFormat != 0 )
            controlByte = 0x05u;  // ACK and C12.22 bits are returned in response of 12.21 request with DATA_FORMAT=1
         else
            controlByte = 0u;
      }
      else if ( index == 0 )
         controlByte = 0xC0u;
      else
         controlByte = 0x80u;
      if ( m_nextOutgoingToggleBit )
         controlByte |= 0x20u;
      packet[2] = (char)(controlByte);
      packet[3] = (char)(unsigned char)(numPackets - 1u); // sequence number

      if ( data != NULL )
      {
         unsigned remainingDataSize = unsigned(data->size()) + 1 - index; // M_64_CAST is done above, no need to repeat it here
         if ( remainingDataSize <= chunkSize ) // set the chunksize appropriately
            chunkSize = remainingDataSize;
      }
      else // otherwise we have to send only a single character
         chunkSize = 1;
      packetSizeWithNoCRC = chunkSize + 6; // plus header
      MToBigEndianUINT16(chunkSize, packet + 4);

      if ( index == 0 ) // first packet has a command character
      {
         packet[6] = command;
         if ( data != NULL )
         {
            M_ASSERT(chunkSize > 1);
            memcpy(packet + 7, data->data(), chunkSize - 1);
         }
      }
      else
      {
         M_ASSERT(data != NULL);
         memcpy(packet + 6, data->data() + index - 1, chunkSize);
      }
      Muint16 crc = StaticCalculateCRC16FromBuffer(packet, packetSizeWithNoCRC);
      memcpy(packet + packetSizeWithNoCRC, (const char*)&crc, 2); // append crc
      const unsigned packetSize = packetSizeWithNoCRC + 2;

      MProtocolLinkLayerWrapper wrapper(this);
      for ( int retries = m_linkLayerRetries; ; --retries )
      {
         try
         {
            Sleep(m_turnAroundDelay);
            m_channel->WriteBuffer(packet, packetSize);
            m_channel->FlushOutputBuffer(packetSize);

            if ( !m_incomingDataFormat ) // otherwise we shouldn't wait for ACK
            {
               char ch;
               for ( int eeRetries = m_linkLayerRetries; ; --eeRetries )
               {
                  ch = DoReadStartCharacter("\x06\x15\xEE", m_acknowledgementTimeout, 2);
                  if ( ch != '\xEE' )
                     break;
                  if ( eeRetries == 0 )
                  {
                     MCOMException::Throw(M_CODE_STR_P2(M_ERR_EXPECTED_X1_GOT_X2, M_I("Expected character 0x%02X, received 0x%02X"), (unsigned)(unsigned char)CHAR_ACK, (unsigned)(unsigned char)ch));
                     M_ENSURED_ASSERT(0);
                  }
                  eeReceived = true;
                  try
                  {
                     char tmpPacket [ 5 ];
                     m_channel->ReadBuffer(tmpPacket, 5); // rsvd, ctrl, seqn, lenh, lenl
                     unsigned dataLength = MFromBigEndianUINT16(tmpPacket + 3);
                     if ( dataLength <= m_negotiatedPacketSize - 8 ) // length in the packet packet is good...
                        m_channel->ReadBytes(dataLength + 2); // and ignore result
                  }
                  catch ( ... )
                  {
                     // ignore any errors
                  }
                  wrapper.NotifyRetry(M_OPT_STR("Received packet when the acknowledgement is expected"));
                  Sleep(m_turnAroundDelay);
                  m_channel->WriteChar(CHAR_ACK); // <ACK> anyway, even if the CRC is bad. Don't care for duplicate packet
               }
               if ( ch == CHAR_ACK )
                  break; // successfully received 06
               else
               {
                  M_ASSERT(ch == CHAR_NAK);
                  // read and remove from buffer or NAKs
                  m_channel->ClearInputBuffer();
                  MCOMException::Throw(M_CODE_STR_P2(M_ERR_EXPECTED_X1_GOT_X2, M_I("Expected character 0x%02X, received 0x%02X"), (unsigned)(unsigned char)CHAR_ACK, (unsigned)(unsigned char)ch));
                  M_ENSURED_ASSERT(0);
               }
            }
            else
               break;
         }
         catch ( MException& ex ) // excluding timeout exception...
         {
            wrapper.NotifyOrThrowRetry(ex, retries);
         }
      }
      m_nextOutgoingToggleBit = !m_nextOutgoingToggleBit;
      index += chunkSize;
      --numPackets;
   }
   // false only if eeReceived == true and multiPacketTransmission == true
   return !(eeReceived && multiPacketTransmission);
}

MEC12NokResponse::ResponseCode MProtocolC1218::DoFullApplicationLayerRead()
{
   // No need to erase response: DoApplicationLayerRead is never called before DoApplicationLayerRequest,
   // and it is there where these lines are located:
   //    m_applicationLayerResponse.Clear();

#if !M_NO_PROGRESS_MONITOR
   MProgressAction* action = NULL; // Action will be created at necessity only. This is required for keep session alive feature because it shall not use progress monitor while keeping session
#endif

   // retry application layer only in case of multipacket transmission fails
   // in this case returns (MEC12NokResponse::ResponseCode)-1
   bool retryAppLayer = false;
   bool retryAppLayerAtEeTimeout = false;
   Muint16 crc;
   MEC12NokResponse::ResponseCode responseCode = (MEC12NokResponse::ResponseCode)-1;
   char* packet = DoGetPacketBuffer();
   Muint8 ctrl;
   unsigned sequenceNumber;
   unsigned previousSequenceNumber = (unsigned)~0; // use (unsigned) instead of (unsigned char) to allow out of bound initial value
   do
   {
      unsigned dataLength;

      MProtocolLinkLayerWrapper wrapper(this);
      for ( int retries = m_linkLayerRetries; ; --retries )
      {
         try
         {
            packet[0] = DoReadStartCharacter("\xEE", m_acknowledgementTimeout, 0);
            retryAppLayerAtEeTimeout = false;

            MChannel::ReadTimeoutSavior timeoutSavior(m_channel, m_intercharacterTimeout);
            m_channel->ReadBuffer(packet + 1, 5); // rsvd, ctrl, seqn, lenh, lenl
            ctrl = packet[2]; // reading this field for checking duplicate packet

            dataLength = MFromBigEndianUINT16(packet + 4);
            if ( dataLength == 0 || dataLength > m_negotiatedPacketSize - 8 ) // length in the packet is not any good...
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
            unsigned packetSizeWithNoCRC = dataLength + 6;

            crc = MToAlignedUINT16(packet + packetSizeWithNoCRC);
            if ( crc != StaticCalculateCRC16FromBuffer(packet, packetSizeWithNoCRC) && !retryAppLayer ) // if we retry app layer, acknowledge everything
            {
               MCOMException::Throw(M_CODE_STR(MErrorEnum::CrcCheckFailed, M_I("CRC check failed")));
               M_ENSURED_ASSERT(0);
            }

            // The incoming data format can be taken from the Control byte after verifying
            // that the CRC is correct. If it is done before, then it is possible for
            // garbage data to spoil the data.
            m_incomingDataFormat = ctrl & 0x03; //data format is the first two bits of ctrl field according to 12.21

            Sleep(m_turnAroundDelay);

            if ( !retryAppLayer )
            {
               if ( m_receiveToggleBitKnown &&                    //   toggle bit is known
                     m_receiveToggleBit == ((ctrl & 0x20) != 0) )  //   and it is the same as one of the previous packets
               {
                  if ( m_savedCRC == crc ) // this is the same packet as one before
                  {
                     // don't send ack only if there is no multipacket transmission and incomingDataFormat is not null; this is 12.21 specification
                     if ( (ctrl & 0x80) != 0 || m_incomingDataFormat == 0 )
                        m_channel->WriteChar(CHAR_ACK);
                     wrapper.NotifyRetry(M_OPT_STR("Received packet toggle bit failure"));
                     if ( retries == 0 )
                        MCOMException::Throw(M_CODE_STR(M_ERR_DID_NOT_GET_A_VALID_BYTE_AMONG_D1_GARBAGE_BYTES_LAST_ONE_HAD_CODE_X2, M_I("Received packet toggle bit failure")));
                     if ( (ctrl & 0x80) == 0 ) // only for single packet transmission retry at the app layer at the event of timeout
                        retryAppLayerAtEeTimeout = true;
                     continue;
                  }
                  else
                  {
                     m_receiveToggleBitKnown = false; // starting from this moment we do not know the correct toggle bit
                     retryAppLayer = true; // for safety, always retry such request
                     wrapper.NotifyRetry(M_OPT_STR("Packet is bad or received out of sequence, whole app layer will be retried"));
                  }
               }
            }

            break; // done retrying the CRC step. Going further interpreting packet
         }
         catch ( MException& ex )
         {
            if ( retryAppLayerAtEeTimeout )
            {
               m_nextOutgoingToggleBit = !m_nextOutgoingToggleBit; // reverse toggle back
               return (MEC12NokResponse::ResponseCode)-1; // retry app layer with the same toggle
            }
            if ( retries == 0 && retryAppLayer )
               return (MEC12NokResponse::ResponseCode)-1; // do an extra pass on the app layer

            Sleep(m_turnAroundDelay);
            if ( m_channel->IsConnected() )
               m_channel->WriteChar(CHAR_NAK); // <NAK>, the packet was not received

            wrapper.NotifyOrThrowRetry(ex, retries);
         }
      }

      m_incomingIdentity = (unsigned)(Muint8)packet[1]; //identity is the first byte of incoming packet according to 12.21

      if ( (ctrl & 0x80) != 0 || m_incomingDataFormat == 0 ) // don't send ack only if there is no multipacket transmission and incomingDataFormat is not null; this is 12.21 specification
         m_channel->WriteChar(CHAR_ACK); // <ACK>, as the CRC is correct

      sequenceNumber = (unsigned)(unsigned char)packet[3];
      if ( !retryAppLayer ) // otherwise accept anything but do nothing
      {
         bool packetLost;
         if ( (ctrl & 0x80) != 0 ) // multipacket transmission
         {
            if ( (ctrl & 0x40) != 0 ) // first in multipacket
               packetLost = (previousSequenceNumber != (unsigned)~0); // ensure this is indeed first
            else // not the first packet
               packetLost = (previousSequenceNumber - 1 != sequenceNumber);
         }
         else
            packetLost = (previousSequenceNumber != (unsigned)~0); // ensure this is indeed first

         if ( packetLost )
         {
            m_receiveToggleBitKnown = false; // starting from this moment we do not know the correct toggle bit
            retryAppLayer = true; // for safety, always retry such request
            wrapper.NotifyRetry(M_OPT_STR("Packet is bad or received out of sequence, whole app layer will be retried"));
         }
         else // otherwise we are a good packet
         {
            if ( m_applicationLayerIncoming.GetTotalSize() == 0 ) // first time, treat the response code
               responseCode = (MEC12NokResponse::ResponseCode)packet[6];

            m_applicationLayerIncoming.Append(packet + 6, dataLength);

#if !M_NO_PROGRESS_MONITOR
            if ( m_expectedPartialReadTableReadResponseSize > 0 )
            {
               if ( m_applicationLayerIncoming.GetTotalSize() < m_expectedPartialReadTableReadResponseSize )
               {
                  if ( action == NULL )
                     action = GetLocalProgressAction();
                  action->SetProgress(double(m_applicationLayerIncoming.GetTotalSize()) * 100.0 / double(m_expectedPartialReadTableReadResponseSize));
               }
               else if ( action != NULL )
                  action->SetProgress(100.0);
            }
#endif

            if ( m_checkIncomingToggleBit )
            {
               M_ASSERT(!m_receiveToggleBitKnown || m_receiveToggleBit != ((ctrl & 0x20) != 0));
               m_savedCRC = crc;
               m_receiveToggleBitKnown = true; // make it true only if m_checkIncomingToggleBit is true!
               m_receiveToggleBit = ((ctrl & 0x20) != 0); // record the toggle bit

               #if defined(M__EMULATE_PERIODIC_TOGGLE_BIT_FAILURE_RATE) && (M__EMULATE_PERIODIC_TOGGLE_BIT_FAILURE_RATE) != 0
                  M_COMPILED_ASSERT((M__EMULATE_PERIODIC_TOGGLE_BIT_FAILURE_RATE) >= 5); // make sure the rate is reasonable, guarantees some success
                  if ( MMath::Rand() % (M__EMULATE_PERIODIC_TOGGLE_BIT_FAILURE_RATE) == 0 )
                     m_receiveToggleBit = !m_receiveToggleBit;
               #endif
            }
            previousSequenceNumber = sequenceNumber;
         }
      }
   }
   while ( (ctrl & 0x80) != 0 && sequenceNumber != 0 );
   m_applicationLayerReader.AssignBuffer(&m_applicationLayerIncoming);

#if !M_NO_PROGRESS_MONITOR
   if ( action != NULL )
      action->Complete();
#endif

   return retryAppLayer ? (MEC12NokResponse::ResponseCode)-1 : responseCode; // this value is used by DoApplicationLayerRead
}

MEC12NokResponse::ResponseCode MProtocolC1218::DoApplicationLayerRead()
{
   MEC12NokResponse::ResponseCode responseCode = DoFullApplicationLayerRead();
   if ( responseCode != (MEC12NokResponse::ResponseCode)-1 )
      m_applicationLayerReader.IgnoreBytes(1); // moving by one byte
   return responseCode;
}

void MProtocolC1218::ServerStart()
{
   MEC12NokResponse::ResponseCode responseCode;
   for ( ;; )
   {
      m_applicationLayerIncoming.Clear();
      responseCode = DoFullApplicationLayerRead();
      if ( responseCode != (MEC12NokResponse::ResponseCode)-1 )
         break;
      m_channel->WriteChar(CHAR_ACK);
   }
}

void MProtocolC1218::ServerEnd(char command, const MByteString& data)
{
   if ( data.empty() )
      DoApplicationLayerWrite(command);
   else
   {
      try
      {
         DoApplicationLayerWrite(command, &data);
      }
      catch ( MEC12NokResponse& ex )
      {
         if ( ex.GetResponseCode() == MEC12NokResponse::RESPONSE_ONP )
            DoApplicationLayerWrite('\x04');
         else
            ex.Rethrow();
      }
   }
}

void MProtocolC1218::ApplyChannelParameters()
{
   MProtocolC12::ApplyChannelParameters();
   m_channel->SetReadTimeout(m_acknowledgementTimeout);
   m_channel->SetIntercharacterTimeout(m_intercharacterTimeout);

   m_receiveToggleBitKnown = false;
   m_negotiatedSessionBaud = m_sessionBaud;
   m_identifiedPropertiesPresent = false;
   m_negotiatedMaximumNumberOfPackets = m_maximumNumberOfPackets; // this can be assigned directly
   DoSetNegotiatedPacketSize(m_packetSize); // this is needed to clear the internal buffer and also initialize m_negotiatedPacketSize
   DoSetParametersIfOpticalProbe(m_initialBaud, 8, 'N', 1);
}

   static const struct MBaudToIndex
      {
         unsigned m_baud;
         char     m_index;
      } s_baudToIndex [] =
      {
         {    300u, 0x01 },
         {    600u, 0x02 },
         {   1200u, 0x03 },
         {   2400u, 0x04 },
         {   4800u, 0x05 },
         {   9600u, 0x06 },
         {  14400u, 0x07 },
         {  19200u, 0x08 },
         {  28800u, 0x09 },
         {  57600u, 0x0A },
         {  38400u, 0x0B },
         { 115200u, 0x0C },
         { 128000u, 0x0D },
         { 256000u, 0x0E }
      };

char MProtocolC1218::DoConvertBaudToIndex(unsigned baud)
{
   for ( int i = M_NUMBER_OF_ARRAY_ELEMENTS(s_baudToIndex) - 1; i >= 0; --i )
      if ( s_baudToIndex[i].m_baud == baud )
         return s_baudToIndex[i].m_index;
#if !M_NO_SERIAL_PORT // ????? TODO:
   MSerialPort::ThrowInvalidBaudRate(baud);
#endif
   M_ENSURED_ASSERT(0);
   return 0; // prevent warning
}

unsigned MProtocolC1218::DoConvertIndexToBaud(char index)
{
   for ( int i = M_NUMBER_OF_ARRAY_ELEMENTS(s_baudToIndex) - 1; i >= 0; --i )
      if ( s_baudToIndex[i].m_index == index )
         return s_baudToIndex[i].m_baud;
   MCOMException::Throw(MException::ErrorMeter, M_CODE_STR_P1(MErrorEnum::InvalidBaud, M_I("Meter requested invalid or unsupported baud rate with code 0x%X"), (unsigned)(unsigned char)index));
   M_ENSURED_ASSERT(0);
   return 0; // prevent warning
}


void MProtocolC1218::DoSetMaximumApplicationLayerPacketSize() M_NO_THROW
{
   m_maximumReadTableSize = USHRT_MAX;
   m_maximumWriteTableSize = USHRT_MAX;
   m_maximumPartialWriteTableSize = USHRT_MAX;
   if ( m_negotiatedPacketSize < USHRT_MAX && m_negotiatedMaximumNumberOfPackets < USHRT_MAX )
   {
      m_maximumReadTableSize = (m_negotiatedPacketSize - PACKET_HEADER_AND_FOOTER_LENGTH) * m_negotiatedMaximumNumberOfPackets - READ_SERVICE_OVERHEAD;
      if ( m_maximumReadTableSize > USHRT_MAX )
         m_maximumReadTableSize = USHRT_MAX;             // due to protocol restriction, size is two bytes
      m_maximumWriteTableSize = (m_negotiatedPacketSize - PACKET_HEADER_AND_FOOTER_LENGTH) * m_negotiatedMaximumNumberOfPackets - WRITE_SERVICE_OVERHEAD;
      if ( m_maximumWriteTableSize > USHRT_MAX )
         m_maximumWriteTableSize = USHRT_MAX;            // due to protocol restriction, size is two bytes
      m_maximumPartialWriteTableSize = (m_negotiatedPacketSize - PACKET_HEADER_AND_FOOTER_LENGTH) * m_negotiatedMaximumNumberOfPackets - PARTIAL_WRITE_SERVICE_OVERHEAD;
      if ( m_maximumPartialWriteTableSize > USHRT_MAX )
         m_maximumPartialWriteTableSize = USHRT_MAX;     // due to protocol restriction, size is two bytes
   }
}

void MProtocolC1218::DoSetNegotiatedPacketSize(unsigned negotiatedPacketSize)
{
   if ( m_negotiatedPacketSize != negotiatedPacketSize ) // otherwise don't bother, the buffer is properly initialized
   {
      MENumberOutOfRange::CheckInteger(SMALLEST_PACKET_SIZE, BIGGEST_PACKET_SIZE, negotiatedPacketSize, M_OPT_STR("NEGOTIATED_PACKET_SIZE"));
      m_negotiatedPacketSize = negotiatedPacketSize;
      delete [] m_dataLinkPacketBuffer; // this can delete a null pointer
      m_dataLinkPacketBuffer = M_NEW char [ negotiatedPacketSize ];
   }
   DoSetMaximumApplicationLayerPacketSize();
}

unsigned MProtocolC1218::GetNumberOfDataLinkPackets(MCommunicationCommand::CommandType typeOfRequest, unsigned applicationLayerDataSize) M_NO_THROW
{
   unsigned packetBodySize = m_negotiatedPacketSize - PACKET_HEADER_AND_FOOTER_LENGTH;
   switch ( typeOfRequest )
   {

   default: // CommandConnect, CommandDisconnect, ...
      return 0;

#if !M_NO_MCOM_IDENTIFY_METER
   case MCommunicationCommand::CommandIdentifyMeter:
      return 12; // pre-calculated, most common sequence without I2C
#endif // !M_NO_MCOM_IDENTIFY_METER

   case MCommunicationCommand::CommandStartSession:
      return 8; // Most common case when there is Login and Security

   case MCommunicationCommand::CommandEndSession:
      return 4;

   case MCommunicationCommand::CommandRead:
   case MCommunicationCommand::CommandReadPartial:
      return (applicationLayerDataSize + packetBodySize - 1 + READ_SERVICE_OVERHEAD) / packetBodySize + 1;

   case MCommunicationCommand::CommandWrite:
      return (applicationLayerDataSize + packetBodySize - 1 + WRITE_SERVICE_OVERHEAD) / packetBodySize + 1;

   case MCommunicationCommand::CommandWritePartial:
      return (applicationLayerDataSize + packetBodySize - 1 + PARTIAL_WRITE_SERVICE_OVERHEAD) / packetBodySize + 1;

   case MCommunicationCommand::CommandExecute:
   case MCommunicationCommand::CommandExecuteRequest:
   case MCommunicationCommand::CommandExecuteResponse:
   case MCommunicationCommand::CommandExecuteRequestResponse:
      return (applicationLayerDataSize + packetBodySize - 1 + WRITE_SERVICE_OVERHEAD) / packetBodySize + 3; // this one has precision +-1
   }
}

#if !M_NO_MCOM_KEEP_SESSION_ALIVE

unsigned MProtocolC1218::DoGetKeepSessionAliveFirstDelay() const
{
   if ( !m_isInSession || !IsConnected() )
      return 0u;

   if ( m_channelTrafficTimeout > 10000 ) // do not do session keeping in smaller intervals to facilitate easy task interruption
      return 8000u;
   if ( m_channelTrafficTimeout < 2000 )   // Assume something is wrong about the very value. Do not keep session more often than in 1 second intervals
      return 1000u;
   if ( m_channelTrafficTimeout < 4000 )   // Assume something is wrong about the very value. Do not keep session more often than in 100 millisecond intervals
      return m_channelTrafficTimeout - 1000u;
   return m_channelTrafficTimeout - 2000;
}

#endif // !M_NO_MCOM_KEEP_SESSION_ALIVE
#endif // !M_NO_MCOM_PROTOCOL_C1218
