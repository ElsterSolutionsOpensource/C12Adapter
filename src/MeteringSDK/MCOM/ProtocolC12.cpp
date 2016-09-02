// File MCOM/ProtocolC12.cpp

#include "MCOMExtern.h"
#include "ChannelOpticalProbe.h"
#include "ChannelModem.h"
#include "ChannelSocket.h"
#include "MCOMExceptions.h"
#include "ProtocolC12.h"
#include "ProtocolC1222.h"
#include "IdentifyString.h"

#if !M_NO_MCOM_PROTOCOL_C1218 || !M_NO_MCOM_PROTOCOL_C1221 || !M_NO_MCOM_PROTOCOL_C1222

M_START_PROPERTIES(ProtocolC12)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL       (ProtocolC12, IssueSecurityOnStartSession,         true)
   M_OBJECT_PROPERTY_PERSISTENT_INT        (ProtocolC12, AlwaysReadFunctionResponse,          MProtocolC12::ReadFunctionResponseAlways)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL       (ProtocolC12, AlwaysUsePartial,                    false)
#if !M_NO_MCOM_KEEP_SESSION_ALIVE
   M_OBJECT_PROPERTY_PERSISTENT_BOOL       (ProtocolC12, UseReadInKeepSessionAlive,           false)
#endif
   M_OBJECT_PROPERTY_PERSISTENT_BOOL       (ProtocolC12, EndSessionOnApplicationLayerError,   false)
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC12, UserId,                              0u)
   M_OBJECT_PROPERTY_PERSISTENT_BYTE_STRING(ProtocolC12, User,        "\0\0\0\0\0\0\0\0\0\0", 10u, ST_constMByteStringA_X, ST_X_constMByteStringA)
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC12, TurnAroundDelay,                     20u)
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC12, ApplicationLayerRetries,             20u)
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC12, ApplicationLayerRetryDelay,          2000u)
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC12, ApplicationLayerProcedureRetries,    20u)
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC12, ApplicationLayerProcedureRetryDelay, 500u)
// Property default value overwritten from parent:
   M_OBJECT_PROPERTY_PERSISTENT_BYTE_STRING(ProtocolC12, Password, "00000000000000000000", 20, ST_MByteString_X, ST_X_constMByteStringA) // DOXYGEN_HIDE SWIG_HIDE
   M_OBJECT_PROPERTY_READONLY_UINT         (ProtocolC12, MaximumReadTableSize)
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ProtocolC12, LinkLayerRetries,                    3u)
   M_OBJECT_PROPERTY_UINT                  (ProtocolC12, ProcedureSequenceNumber)
M_START_METHODS(ProtocolC12)
   M_OBJECT_SERVICE(ProtocolC12, ApplicationLayerRequestResponse, ST_MByteString_X_byte_constMByteStringA)
   M_OBJECT_SERVICE(ProtocolC12, Wait,                            ST_X_unsigned)
   M_OBJECT_SERVICE(ProtocolC12, Logon,                           ST_X)
   M_OBJECT_SERVICE(ProtocolC12, Security,                        ST_X)
   M_OBJECT_SERVICE(ProtocolC12, FullLogin,                       ST_X)
   M_OBJECT_SERVICE(ProtocolC12, Logoff,                          ST_X)
   M_OBJECT_SERVICE(ProtocolC12, Terminate,                       ST_X)
   M_OBJECT_SERVICE            (ProtocolC12, ReceiveServiceCode,                                ST_byte_X)
   M_OBJECT_SERVICE            (ProtocolC12, ReceiveServiceBytes,                               ST_MByteString_X_unsigned)
   M_OBJECT_SERVICE            (ProtocolC12, ReceiveServiceRemainingBytes,                      ST_MByteString_X)
   M_OBJECT_SERVICE            (ProtocolC12, ReceiveServiceByte,                                ST_byte_X)
   M_OBJECT_SERVICE            (ProtocolC12, ReceiveServiceUInt,                                ST_unsigned_X_unsigned)
   M_CLASS_SERVICE             (ProtocolC12, CRC16,                                             ST_unsigned_S_constMByteStringA)
M_END_CLASS_TYPED(ProtocolC12, Protocol, "PROTOCOL_ANSI_C12")

   const unsigned s_MT100 = 2148;   // MT-100 = 2048 + 100 = 2148
   const unsigned s_MT101 = 2149;   // MT-101 = 2048 + 101 = 2149
   const unsigned s_MT102 = 2150;   // MT-102 = 2048 + 102 = 2150

   const unsigned NUMBER_MASK              =   0x7FFu;
   const unsigned NUMBER_MANUFACTURING_BIT =   0x800u; // =  2048, Manufacturer function or table
   const unsigned NUMBER_PENDING_BIT       =  0x1000u; // =  4096, Pending table
   const unsigned NUMBER_SERVICE_OFFSET    = 0x10000u; // = 65536, service, semi-software table or function
   const unsigned NUMBER_FUNCTION_MASK     = 0x100FFu;

   using namespace std;

MProtocolC12::MProtocolC12(MChannel* channel, bool channelIsOwned)
:
   MProtocol(channel, channelIsOwned),
   m_procedureSequenceNumber(0u),
   m_isST007Write(false),
   m_negotiatedPacketSize(UINT_MAX),  // this value can be redefined by protocols, which have data link layer
   m_maximumReadTableSize(USHRT_MAX), // this can be recalculated based on negotiated packet size and number of packets
   m_maximumWriteTableSize(USHRT_MAX), // this can be recalculated based on negotiated packet size and number of packets
   m_maximumPartialWriteTableSize(USHRT_MAX), // this can be recalculated based on negotiated packet size and number of packets
   m_expectedPartialReadTableReadResponseSize(0)
{
   m_maximumPasswordLength = 20;
   M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(ProtocolC12);
}

MProtocolC12::~MProtocolC12()
{
   Finalize();
}

void MProtocolC12::SetAlwaysReadFunctionResponse(ReadFunctionResponseEnum value)
{
   if ( static_cast<int>(value) == -1 )    // Special compatibility case, assigning vbTrue == -1 should produce ReadFunctionResponseWhenDesired
      value = ReadFunctionResponseWhenDesired;
   MENumberOutOfRange::CheckInteger(static_cast<int>(ReadFunctionResponseWhenPresent), static_cast<int>(ReadFunctionResponseAlways), static_cast<int>(value), M_OPT_STR("ALWAYS_READ_FUNCTION_RESPONSE"));
   m_alwaysReadFunctionResponse = value;
}

void MProtocolC12::SetUserId(unsigned userId)
{
   MENumberOutOfRange::CheckInteger(0, 0xFFFF, (int)userId, M_OPT_STR("USER_ID"));
   m_userId = userId;
}

void MProtocolC12::SetUser(const MByteString& user)
{
   if ( user.size() > 10 )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR_P1(M_ERR_USER_NAME_SHOULD_BE_NO_MORE_THAN_D1_BYTES_LONG, M_I("User name should be no more than %d bytes long"), 10));
      M_ENSURED_ASSERT(0);
   }
   m_user = user;
}

void MProtocolC12::SetProcedureSequenceNumber(unsigned number)
{
   MENumberOutOfRange::Check(0, 255, number, "PROCEDURE_SEQUENCE_NUMBER");
   m_procedureSequenceNumber = static_cast<Muint8>(number);
}


void MProtocolC12::SetIssueSecurityOnStartSession(bool yes)
{
   m_issueSecurityOnStartSession = yes;
}

unsigned MProtocolC12::CalculateChecksumFromBuffer(const char* buffer, unsigned length) const
{
   return StaticCalculateChecksumFromBuffer(buffer, length);
}

unsigned MProtocolC12::StaticCalculateChecksumFromBuffer(const char* data, unsigned size)
{
   unsigned char sum = 0;
   const char* dataEnd = data + size;
   for ( ; data != dataEnd; ++data )
      sum -= (unsigned char)*data;
   return (unsigned)(unsigned char)sum;
}

unsigned MProtocolC12::StaticCalculateChecksum(const MByteString& buff)
{
   return StaticCalculateChecksumFromBuffer(buff.data(), M_64_CAST(unsigned, buff.size()));
}

#if !M_NO_VERBOSE_ERROR_INFORMATION

void MProtocolC12::DoBuildComplexServiceName(MChars fullServiceName, MConstChars serviceName, MCOMNumberConstRef number, int par1, int par2) M_NO_THROW
{
   if ( !DoBuildComplexC12ServiceName(fullServiceName, serviceName, number, par1, par2) )
      MProtocol::DoBuildComplexServiceName(fullServiceName, serviceName, number, par1, par2);
}

bool MProtocolC12::DoBuildComplexC12ServiceName(MChars fullServiceName, MConstChars serviceName, MCOMNumberConstRef number, int par1, int par2) M_NO_THROW
{
#if !M_NO_VARIANT
   if ( number.IsNumeric() )
   {
      try
      {
         unsigned num = number.AsDWord(); // avoid signed/unsigned differences
         MChar  prefix [ 4 ];
         MChar* prefixPtr = prefix;
         if ( (num & ~0xFFFF) == 0 ) // not a service, etc
         {
            if ( (num & NUMBER_PENDING_BIT) != 0 )
               *prefixPtr++ = 'P';

            if ( (num & NUMBER_MANUFACTURING_BIT) != 0 )
               *prefixPtr++ = 'M';
            else
               *prefixPtr++ = 'S';

            {
               M_ASSERT(serviceName[0] == 'T' || serviceName[0] == 'F');
               *prefixPtr++ = serviceName[0]; // 'T' or 'F'
            }
         }
         *prefixPtr = '\0';

         size_t fullServiceNameSize = ( par1 == -1 && par2 == -1 )
               ? MFormat(fullServiceName, MAXIMUM_SERVICE_NAME_STRING_SIZE, "%s(%s%d[0x%04X])", serviceName, prefix, (num & NUMBER_MASK), num)
               : MFormat(fullServiceName, MAXIMUM_SERVICE_NAME_STRING_SIZE, "%s(%s%d[0x%04X], %d, %d)", serviceName, prefix, (num & NUMBER_MASK), num, par1, par2);
         M_ASSERT(fullServiceNameSize > 0 && fullServiceNameSize < MAXIMUM_SERVICE_NAME_STRING_SIZE); // Check if MAXIMUM_SERVICE_NAME_STRING_SIZE is big enough
         M_USED_VARIABLE(fullServiceNameSize);
         return true; // success
      }
      catch ( ... )
      {
         // fall into default implementation
      }
   }
#else
   fullServiceName[0] = '\0';
#endif // !M_NO_VARIANT
   return false;
}

#endif // !M_NO_VERBOSE_ERROR_INFORMATION

   inline unsigned DoGetRequestFlags(MCOMNumberConstRef)
   {
      return MProtocolC12::APPLICATIONLAYERREQUEST_NO_FLAGS; // reserved for future extensions
   }

void MProtocolC12::DoTableRead(MCOMNumberConstRef number, MByteString& data, unsigned expectedSize)
{
   M_ASSERT(m_maximumReadTableSize <= USHRT_MAX);
   data.clear();
   if ( expectedSize > 0 )
      data.reserve(expectedSize);
   unsigned unsignedNumber = DoConvertNumberToUnsigned(number);
   if ( (m_alwaysUsePartial && expectedSize > 0) || expectedSize > m_maximumReadTableSize )
      DoTableReadPartial(number, data, 0, expectedSize); // partial reads are split into several, can read any size
   else // otherwise do full table read, as defined by the protocol
   {

#if !M_NO_PROGRESS_MONITOR
      MProgressAction* action = GetLocalProgressAction(); // local action should always be completed after Write
      action->CreateLocalAction(100.0); // to send action to child procedures
#endif

      m_expectedPartialReadTableReadResponseSize = expectedSize;
      MValueEndScopeSetter<unsigned> expectedPartialReadTableReadResponseSizeSetter(&m_expectedPartialReadTableReadResponseSize, 0);

      data.clear(); // clear response data for the possible case of failure
      Muint16 num = MToBigEndianUINT16(unsignedNumber);
      MByteString request((const char*)&num, 2);

      DoApplicationLayerRequest('\x30', &request, DoGetRequestFlags(number));
      DoAppendTableReadResponse(data);

#if !M_NO_PROGRESS_MONITOR
      action->Complete();
#endif
   }
}

void MProtocolC12::DoTableWrite(MCOMNumberConstRef number, const MByteString& data)
{
   M_ASSERT(m_maximumWriteTableSize <= USHRT_MAX);
   unsigned unsignedNumber = DoConvertNumberToUnsigned(number);
   if ( (m_alwaysUsePartial && unsignedNumber != 7) || data.size() > m_maximumWriteTableSize )
      DoTableWritePartial(number, data, 0); // partial writes are split into several, can write any size
   else // otherwise do full table write, as defined by the protocol
   {
#if !M_NO_PROGRESS_MONITOR
      MProgressAction* action = GetLocalProgressAction(); // local action should always be completed after Write
      action->CreateLocalAction(100.0); // to send action to child procedures
#endif

      M_ASSERT(data.size() <= USHRT_MAX);
      Muint16 num16 = MToBigEndianUINT16(unsignedNumber);
      MByteString request((const char*)&num16, 2);
      num16 = MToBigEndianUINT16(M_64_CAST(unsigned, data.size()));
      request.append((const char*)&num16, 2);
      request += data;
      request += (char)(unsigned char)StaticCalculateChecksum(data);

      unsigned flags = DoGetRequestFlags(number); // handle ONP flag


      DoApplicationLayerRequest('\x40', &request, flags);

#if !M_NO_PROGRESS_MONITOR
      action->Complete();
#endif
   }
}

void MProtocolC12::DoTableReadPartial(MCOMNumberConstRef number, MByteString& data, int offset, int length)
{
   M_ASSERT(m_maximumReadTableSize <= USHRT_MAX);

   data.clear(); // clear response data for the case of failure

   unsigned bytesToRead = length;
   data.reserve(bytesToRead);

#if !M_NO_PROGRESS_MONITOR
   MProgressAction* action = GetLocalProgressAction();
   if ( bytesToRead > m_maximumReadTableSize )
   {
      M_ASSERT(length > 0); // sure, because m_maximumReadTableSize is positive
      bytesToRead = m_maximumReadTableSize;
      action->CreateLocalAction(double(bytesToRead) * 100.0 / double(length));
   }
   else
      action->CreateLocalAction(100.0); // to send action to child procedures
#else
   if ( bytesToRead > m_maximumReadTableSize )
      bytesToRead = m_maximumReadTableSize;
#endif

   char buff [ 7 ]; // hard-code the packet size
   MToBigEndianUINT16(DoConvertNumberToUnsigned(number), buff);
   MToBigEndianUINT24(offset,                           buff + 2);
   MToBigEndianUINT16(bytesToRead,                      buff + 5);

   m_expectedPartialReadTableReadResponseSize = bytesToRead;
   MValueEndScopeSetter<unsigned> expectedPartialReadTableReadResponseSizeSetter(&m_expectedPartialReadTableReadResponseSize, 0);

   MByteString request(buff, 7);
   unsigned appLayerFlags = DoGetRequestFlags(number);
   DoApplicationLayerRequest('\x3F', &request, appLayerFlags);
   DoAppendTableReadResponse(data);

   if ( bytesToRead < (unsigned)length ) // whether we have done everything for a single read step already
   {
      M_ASSERT((unsigned)length > m_maximumReadTableSize);

#if !M_NO_PROGRESS_MONITOR
      // If we are here, we have to continue looping
      action->SetProgress(double(data.size()) * 100.0 / double(length));
#endif

      // Now read the packets which we could not read in first chunk. Reuse the already created buffer for performance.
      //
      for ( int pos = bytesToRead; ; pos += bytesToRead )
      {
         bytesToRead = length - pos;
         if ( bytesToRead <= 0 )
            break;
         if ( bytesToRead > m_maximumReadTableSize )
            bytesToRead = m_maximumReadTableSize;

         m_expectedPartialReadTableReadResponseSize = bytesToRead;

#if !M_NO_PROGRESS_MONITOR
         double progress = double(data.size() + bytesToRead) * 100.0 / double(length);
         action->CreateLocalAction(progress);
#endif

         MToBigEndianUINT24(offset + pos, buff + 2); // this overwrites the following byte, ok
         MToBigEndianUINT16(bytesToRead,  buff + 5);
         request.replace(2, 5, buff + 2, 5);
         DoApplicationLayerRequest('\x3F', &request, appLayerFlags);
         DoAppendTableReadResponse(data);

#if !M_NO_PROGRESS_MONITOR
         action->SetProgress(progress);
#endif

      }
   }

#if !M_NO_PROGRESS_MONITOR
   action->Complete();
#endif

}

void MProtocolC12::DoTableWritePartial(MCOMNumberConstRef number, const MByteString& data, int offset)
{
   M_ASSERT(m_maximumPartialWriteTableSize <= USHRT_MAX);

   unsigned bytesToWrite = M_64_CAST(unsigned, data.size());
#if !M_NO_PROGRESS_MONITOR
   MProgressAction* action = GetLocalProgressAction();
   if ( bytesToWrite > m_maximumPartialWriteTableSize )
   {
      bytesToWrite = m_maximumPartialWriteTableSize;
      action->CreateLocalAction(double(bytesToWrite) * 100.0 / double(data.size()));
   }
   else
      action->CreateLocalAction(100.0); // to send action to child procedures
#else
   if ( bytesToWrite > m_maximumPartialWriteTableSize )
      bytesToWrite = m_maximumPartialWriteTableSize;
#endif

   char buff [ 7 ]; // hard-code the packet size
   MToBigEndianUINT16(DoConvertNumberToUnsigned(number), buff);
   MToBigEndianUINT24(offset,                           buff + 2);
   MToBigEndianUINT16(bytesToWrite,                     buff + 5);

   MByteString request(buff, 7);
   const char* chunk = data.data();
   request.append(chunk, bytesToWrite);
   request += (char)(unsigned char)StaticCalculateChecksumFromBuffer(chunk, bytesToWrite);

   unsigned appLayerFlags = DoGetRequestFlags(number);

   DoApplicationLayerRequest('\x4F', &request, appLayerFlags);
   if ( bytesToWrite < M_64_CAST(unsigned, data.size()) )
   {
      M_ASSERT((unsigned)data.size() > m_maximumPartialWriteTableSize);  // if we are here, we could not do everything per one single write
#if !M_NO_PROGRESS_MONITOR
      action->SetProgress(double(bytesToWrite) * 100.0 / double(data.size()));
#endif
      // Now write the packets which we could not write in the first chunk. Reuse the already created buffer for performance.
      //
      for ( int pos = bytesToWrite; ; pos += bytesToWrite )
      {
         bytesToWrite = unsigned(data.size()) - pos; // in case data.size() is too big the exception is thrown above
         if ( bytesToWrite <= 0 )
            break;
         if ( bytesToWrite > m_maximumPartialWriteTableSize )
            bytesToWrite = m_maximumPartialWriteTableSize;

#if !M_NO_PROGRESS_MONITOR
         double progress = double(pos + bytesToWrite) * 100.0 / double(data.size());
         action->CreateLocalAction(progress);
#endif

         MToBigEndianUINT24(offset + pos, buff + 2);
         MToBigEndianUINT16(bytesToWrite, buff + 5);
         request.assign(buff, 7);
         const char* localChunk = chunk + pos;
         request.append(localChunk, bytesToWrite);
         request += (char)(unsigned char)StaticCalculateChecksumFromBuffer(localChunk, bytesToWrite);
         DoApplicationLayerRequest('\x4F', &request, appLayerFlags);

#if !M_NO_PROGRESS_MONITOR
         action->SetProgress(progress);
#endif

      }
   }

#if !M_NO_PROGRESS_MONITOR
   action->Complete();
#endif
}


void MProtocolC12::DoFunctionExecute(MCOMNumberConstRef number)
{
   MByteString request;
   MByteString response;
   DoFunction(number, request, response, false);
}

void MProtocolC12::DoFunctionExecuteRequest(MCOMNumberConstRef number, const MByteString& request)
{
   MByteString response;
   DoFunction(number, request, response, false);
}

void MProtocolC12::DoFunctionExecuteResponse(MCOMNumberConstRef number, MByteString& response)
{
   MByteString request;
   DoFunction(number, request, response, true);
}

void MProtocolC12::DoFunctionExecuteRequestResponse(MCOMNumberConstRef number, const MByteString& request, MByteString& response)
{
   DoFunction(number, request, response, true);
}


void MProtocolC12::Logon()
{
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("Logon"), MProtocolServiceWrapper::ServiceNotQueueable | MProtocolServiceWrapper::ServiceStartsSessionKeeping);
   try
   {
      M_ASSERT(m_user.size() <= 10);
      Muint16 num16 = MToBigEndianUINT16(m_userId);
      MByteString request((const char*)&num16, 2);
      request += m_user;
      int diff = 10 - (int)m_user.size();
      if ( diff > 0 )
         request.append(diff, ' '); // fill the rest of the user request with blanks
      DoApplicationLayerRequest('\x50', &request);
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

void MProtocolC12::Security()
{
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("Security"), MProtocolServiceWrapper::ServiceNotQueueable);
   try
   {
      DoTryPasswordOrPasswordList(); // Try the password outside of retry loop
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

void MProtocolC12::FullLogin()
{
   Security(); // Do Security by default (overridable by children)
}

void MProtocolC12::Logoff()
{
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("Logoff"), MProtocolServiceWrapper::ServiceNotQueueable | MProtocolServiceWrapper::ServiceEndsSessionKeeping);
   try
   {
      DoApplicationLayerRequest('\x52');
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

void MProtocolC12::Wait(unsigned seconds)
{
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("Wait"), MProtocolServiceWrapper::ServiceNotQueueable); // Wait does not influence KeepSessionAlive
   try
   {
      if ( seconds > 255 )
      {
         MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR_P1(M_ERR_WAIT_PERIOD_U1_IS_BIGGER_THAN_MAXIMUM_255, "Requested wait period %u is bigger than supported maximum of 255 seconds", seconds));
         M_ENSURED_ASSERT(0);
      }
      char sec = (char)(unsigned char)seconds;
      MByteString request(&sec, 1);
      DoApplicationLayerRequest('\x70', &request);
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

void MProtocolC12::Terminate()
{
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("Terminate"), MProtocolServiceWrapper::ServiceNotQueueable | MProtocolServiceWrapper::ServiceEndsSessionKeeping);
   m_isInSession = false; // to avoid duplicate Terminate
   try
   {
      DoApplicationLayerRequest('\x21');
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

#if !M_NO_MCOM_IDENTIFY_METER

   // Elster-specific option board information.
   // Note the structure consists only of character types,
   // no alignment is put anywhere, and we do not need to control the alignment.
   //
   struct MElsterOptionBoardInfo
   {
      char m_type[2];    // ARRAY [2] OF CHAR;
      char m_sspec[3];   // ARRAY [3] OF BCD;
      Muint8 m_fwGroup;  // BIT FIELD OF UINT8;
      Muint8 m_revnum;   // UINT8;
   };

   // Append Identify String tag with the given name and the buffer of the MElsterOptionBoardInfo.
   // In case the option board information tells there is no option board, nothing is appended.
   //
   // PRECONDITION: The given buffer shall comprise the correct option board information.
   // No check is done however. The number shall be a positive number less than 10. There is a debug check.
   //
   // POSTCONDITION: The string is appended with the given option board information, if it is present.
   //
   static void DoAppendOptionBoard(MIdentifyString& id, const void* buff, int position, bool& isLanobPresent)
   {
      const MElsterOptionBoardInfo* ob = static_cast<const MElsterOptionBoardInfo*>(buff);
      if ( id.AppendObTags(position, ob->m_sspec, (unsigned)ob->m_fwGroup, (unsigned)ob->m_revnum, ob->m_type) &&
           ob->m_sspec[0] == '\x00' && ob->m_sspec[1] == '\x02' && (ob->m_sspec[2] == '\x39' ||
                                                                    ob->m_sspec[2] == '\x45' ||
                                                                    ob->m_sspec[2] == '\x63' ||
                                                                    ob->m_sspec[2] == '\x60') )   // Collector 5.0
      {
         isLanobPresent = true; // otherwise do not modify the value of this flag
      }
   }

   // C12 standard option board information.
   // Note the structure consists only of character types,
   // no alignment is put anywhere, and we do not need to control the alignment.
   //
   struct MA3TableST1
   {
      char   m_manufacturer      [ 4 ];  // Manufacturer code: EE, ABB, etc.
      char   m_ed_model          [ 8 ];  // Basic Meter Type. For example, "A1T-L"
      Muint8 m_hw_version_number;        // Hardware Version Number. Only written by manufacturing
      Muint8 m_hw_revision_number;       // Hardware Revision Number. Only written by manufacturing
      Muint8 m_fw_version_number;        // Firmware Version Number
      Muint8 m_fw_revision_number;       // Firmware Revision Number, used to track the firmware release. ROM field set at compile time
      char   m_mfg_serial_number [ 16 ]; // Serial Number, ASCII string
   };

   // REX-U MT1 table
   //
   struct MRexUTableMT1 // big endian, but fortunately there are no dependent data
   {
      Muint8 m_metrology_firmware_version;
      Muint8 m_metrology_firmware_revision;
      char   m_metrology_firmware_sspec [ 3 ]; // ARRAY [3] OF HEX,
      char   m_mfg_id [ 2 ];                   // ARRAY [2] OF CHAR,  VALUES ("Grid Stream" = "GS", "Undefined" = "UN");
      Muint8 m_comm_firmware_version;          // communication controller version
      Muint8 m_comm_firmware_revision;         // communication controller revision
      char   m_comm_firmware_sspec [ 3 ];      // ARRAY [3] OF HEX
      Muint8 m_zigbee_firmware_version;
      Muint8 m_zigbee_firmware_revision;
      char   m_zigbee_firmware_sspec [ 3 ];    // ARRAY [3] OF HEX
      Muint8 m_option1_firmware_version;
      Muint8 m_option1_firmware_revision;
      char   m_option1_firmware_sspec [ 3 ];   // ARRAY [3] OF HEX,
      Muint8 m_option2_firmware_version;
      Muint8 m_option2_firmware_revision;
      char   m_option2_firmware_sspec [ 3 ];   // ARRAY [3] OF HEX
   };

   static void DoCheckTableSize(const MByteString& bytes, MByteString::size_type size)
   {
      if ( bytes.size() < size )
      {
         MCOMException::Throw(MException::ErrorMeter, M_ERR_INCOMPATIBILITY_IN_TABLE_SIZE_OR_CONTENTS_DURING_IDENTIFY, M_I("Incompatibility in table size or contents, cannot identify the meter"));
         M_ENSURED_ASSERT(0);
      }
   }

   static void DoIdentifyStringAppendST1(MIdentifyString& id, const MA3TableST1* st1)
   {
      id.AppendTag("MANUFACTURER", st1->m_manufacturer, sizeof(st1->m_manufacturer));
      id.AppendTag("ED_MODEL", st1->m_ed_model, sizeof(st1->m_ed_model));
      id.AppendTag("HW_VERSION_REVISION", (unsigned)st1->m_hw_version_number, (unsigned)st1->m_hw_revision_number);
      id.AppendTag("SW_VERSION_REVISION", (unsigned)st1->m_fw_version_number, (unsigned)st1->m_fw_revision_number);
      id.AppendTag("MFG_SERIAL_NUMBER", st1->m_mfg_serial_number, sizeof(st1->m_mfg_serial_number));
   }

   static void DoIdentifyStringAppendBoard(MIdentifyString& id, const MA3TableST1* st1, unsigned version, unsigned revision, const char* sspec)
   {
      if ( sspec[0] != '\0' || sspec[1] != '\0' || sspec[2] != '\0' )
      {
         id.AppendNew();
         id.AppendTag("MANUFACTURER", st1->m_manufacturer, sizeof(st1->m_manufacturer));
         id.AppendTag("ED_MODEL", st1->m_ed_model, sizeof(st1->m_ed_model));
         id.AppendTag("SW_VERSION_REVISION", version, revision);
         id.AppendHexTag("SSPEC", sspec, 3);
      }
   }

MStdString MProtocolC12::DoIdentifyMeter(bool sessionIsStarted, TableRawDataVector* tablesRead)
{
   MVariant var1 = 1;
   MVariant varMT1 = 2048 + 1; // MT1, with the manufacturer flag set
   MByteString tableMT1; // Table MT1
   MByteString table1 = TableRead(var1, sizeof(MA3TableST1));
   DoCheckTableSize(table1, sizeof(MA3TableST1));

   const MA3TableST1* st1 = (const MA3TableST1*)table1.data();
   MIdentifyString id;
   DoIdentifyStringAppendST1(id, st1);
   if ( memcmp(st1->m_ed_model, "A3", 2) == 0 || memcmp(st1->m_ed_model, "A1800", 5) == 0 ) 
   {
      unsigned savedPacketSize = m_negotiatedPacketSize;
      DoSetNegotiatedPacketSize(1024); // Elster default is effective
      try
      {
         tableMT1 = TableRead(varMT1, 43); // Table MT1
         DoCheckTableSize(tableMT1, 43);

         const char* mt1 = tableMT1.data(); // this one is easier to manipulate as a character array
         bool isLanobPresent = false;

         // Two boards at the beginning of the table
         //
         const char* board = mt1 + 11;
         DoAppendOptionBoard(id, board, 1, isLanobPresent);
         board += sizeof(MElsterOptionBoardInfo);
         DoAppendOptionBoard(id, board, 2, isLanobPresent); // the size of the table is checked above, we fit

         // The rest of the option board structures is at the end of the table (newer revisions of A3 meter only).
         // In case of the pre-2.0 meter, the loop will execute zero cycles.
         //
         int numOb = static_cast<int>((tableMT1.size() - 43) / sizeof(MElsterOptionBoardInfo) + 2);
         if ( numOb > 2 )
         {
            if ( numOb > 8 ) // restrict the number by 8, as defined in MT1
               numOb = 8;
            board = mt1 + 43;
            for ( int boardNumber = 3; boardNumber <= numOb; ++boardNumber, board += sizeof(MElsterOptionBoardInfo) )
               DoAppendOptionBoard(id, board, boardNumber, isLanobPresent);
         }
         id.InsertNumberOfObTags();

         M_USED_VARIABLE(isLanobPresent);
      }
      catch ( ... )
      {
         DoSetNegotiatedPacketSize(savedPacketSize); // Elster default is effective
         throw;
      }
      DoSetNegotiatedPacketSize(savedPacketSize); // Elster default is effective
   }
   else if (( memcmp(st1->m_ed_model, "REXU", 4) == 0     ) && 
            ( memcmp(st1->m_ed_model, "REXU-WIC", 8) != 0 ) ) // REXU
            // Unlike the other metrology boards (EACOMMS, ZNIC2), the REXU-WIC
            // has an ST1. In the case where we are talking directly to the REXU-WIC,
            // do not read MT1, as it does not have this table.
   {
      tableMT1 = TableRead(varMT1, sizeof(MRexUTableMT1)); // Table MT1
      DoCheckTableSize(tableMT1, sizeof(MRexUTableMT1));

      const MRexUTableMT1* mt1 = (const MRexUTableMT1*)tableMT1.data();
      id.AppendHexTag("SSPEC", mt1->m_metrology_firmware_sspec, sizeof(mt1->m_metrology_firmware_sspec));

      if ( mt1->m_mfg_id[0] == 'E' && mt1->m_mfg_id[1] == 'A' ) // MFG_ID == "EA", only in this case make devices
      {
         DoIdentifyStringAppendBoard(id, st1, mt1->m_comm_firmware_version, mt1->m_comm_firmware_revision, mt1->m_comm_firmware_sspec);
         DoIdentifyStringAppendBoard(id, st1, mt1->m_zigbee_firmware_version, mt1->m_zigbee_firmware_revision, mt1->m_zigbee_firmware_sspec);
         DoIdentifyStringAppendBoard(id, st1, mt1->m_option1_firmware_version, mt1->m_option1_firmware_revision, mt1->m_option1_firmware_sspec);
         DoIdentifyStringAppendBoard(id, st1, mt1->m_option2_firmware_version, mt1->m_option2_firmware_revision, mt1->m_option2_firmware_sspec);
      }
   }
   if ( tablesRead != NULL )
   {
      tablesRead->clear();
      tablesRead->push_back(TableRawData(var1, table1));
      if ( !tableMT1.empty() )
         tablesRead->push_back(TableRawData(varMT1, tableMT1));
      // Do not add ST1 of LANOB even if it is present
   }
   return id;
}
#endif // !M_NO_MCOM_IDENTIFY_METER

void MProtocolC12::ApplyChannelParameters()
{
   MProtocol::ApplyChannelParameters();
}

MByteString MProtocolC12::ApplicationLayerRequestResponse(char command, const MByteString& request)
{
   MByteString result;
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("ApplicationLayerRequestResponse"), MProtocolServiceWrapper::ServiceNotQueueable);
   try
   {
      DoApplicationLayerRequest(command, (request.empty() ? NULL : &request));
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
   m_applicationLayerReader.ReadRemainingBytes(result);
   return result;
}

void MProtocolC12::DoFunction(MCOMNumberConstRef number, const MByteString& request, MByteString& response, bool expectResponse)
{
   unsigned num = DoConvertNumberToUnsigned(number, NUMBER_FUNCTION_MASK); // allow for protocol services tagged with 0x10000
   if ( (num & NUMBER_SERVICE_OFFSET) != 0 ) // Protocol Services called
   {
      num &= ~0x10000;
      MENumberOutOfRange::CheckInteger(0, 0xFF, num);
      DoApplicationLayerRequest(static_cast<char>(num), (request.size() != 0) ? &request : NULL);
      response.assign(m_applicationLayerReader.GetTotalPtr(), m_applicationLayerReader.GetTotalSize());
   }
   else // Meter Procedure
      DoMeterProcedure(num, request, response, expectResponse);
}

bool MProtocolC12::DoHaveToSkipReadFunctionResponseTable8(unsigned num, const MByteString& request, bool expectResponse)
{
   return !expectResponse && m_alwaysReadFunctionResponse == ReadFunctionResponseWhenPresent;
}

void MProtocolC12::DoHandleFunctionResponseTable8Read(MByteString& response)
{
   if ( response.size() < 4 )
   {
      MCOMException::Throw(MException::ErrorMeter, M_CODE_STR(M_ERR_RESPONSE_FROM_TABLE8_IS_LESS_THAN_FOUR_BYTES, M_I("Protocol violation, response from table 8 is less than four bytes")));
      M_ENSURED_ASSERT(0); // tell the optimizer we are never here, if it supports __ensure()
   }
   m_procedureSequenceNumber = static_cast<Muint8>(response[2]);
   MEC12BadProcedureResult::ResultCodeEnum resultCode = (MEC12BadProcedureResult::ResultCodeEnum)(Muint8)response[3];
   if ( resultCode != MEC12BadProcedureResult::RESULT_OK ) // Procedure completed, success
   {
      MEC12BadProcedureResult::Throw(resultCode);
      M_ENSURED_ASSERT(0);
   }
   response.erase((size_t)0, (size_t)4); // erase the four-byte header
}

void MProtocolC12::DoReadFunctionResponse(MByteString& response)
{
   for ( unsigned appRetryCount = m_applicationLayerProcedureRetries; ; --appRetryCount )
   {
      try
      {
         response = TableRead(8);
         DoHandleFunctionResponseTable8Read(response);
         break; // success
      }
      catch ( MEC12BadProcedureResult& ex )
      {
         if ( m_endSessionOnApplicationLayerError && ( appRetryCount == 0 || ex.GetProcedureResultCode() != 1 ) )
            DoEndSessionOnApplicationLayerError(true);

         if ( ex.GetProcedureResultCode() != MEC12BadProcedureResult::RESULT_NOT_COMPLETED ) // the only code that gets retried is 1
            ex.Rethrow();

         MProtocolServiceWrapper::StaticNotifyOrThrowRetry(this, ex, appRetryCount);
         Sleep(m_applicationLayerProcedureRetryDelay);
      }
   }
}

void MProtocolC12::DoMeterProcedure(unsigned number, const MByteString &request, MByteString &response, bool expectResponse)
{
   response.clear();
   Muint16 codeAndFlags = m_meterIsLittleEndian ? MToLittleEndianUINT16(number) : MToBigEndianUINT16(number);
   MByteString table7((const char*)&codeAndFlags, 2);
   table7 += static_cast<char>(m_procedureSequenceNumber);
   table7 += request;

   {
      MValueSavior<bool> ST007WriteSavior(&m_isST007Write);
      m_isST007Write = true;
      TableWrite(7, table7);
   }

   if ( !DoHaveToSkipReadFunctionResponseTable8(number, request, expectResponse) )
      DoReadFunctionResponse(response);
}

   inline Muint16 DoUpdateCRC(unsigned char b, Muint16 crc)
   {
      for ( int i = 8; i != 0; --i )
      {
         if ( crc & 0x0001 )
         {
            crc >>= 1;
            if ( b & 0x01 )
              crc |= 0x8000;
            crc ^= 0x8408;
            b >>= 1;
         }
         else
         {
            crc >>= 1;
            if ( b & 0x01 )
               crc |= 0x8000;
            b >>= 1;
         }
      }
      return crc;
   }

Muint16 MProtocolC12::StaticCalculateCRC16FromBuffer(const char* buff, unsigned len)
{
   #if M_LITTLE_ENDIAN                   // PC, AlphaAXP, VAX, HP-UX, ...
      Muint16 crc;
      if ( len >= 2 ) // most usual case is first
      {
         crc = (Muint16)~(((Muint16)(Muint8)buff[1] << 8) | (Muint16)(Muint8)buff[0]);
         for ( unsigned i = 2; i < len; ++i )
            crc = DoUpdateCRC(buff[i], crc);
      }
      else if ( len == 1 )
         crc = (Muint16)~(Muint8)buff[0];
      else
         crc = 0; // len = 0
      crc = DoUpdateCRC(0x00, crc);
      crc = DoUpdateCRC(0x00, crc);
      crc = (Muint16)~crc;
      return crc;
   #else                                 // Sun
      #error "Big endian CRC calculation on big endian computers needs implementation!"
   #endif
}

Muint16 MProtocolC12::CalculateCRC16FromBuffer(const char* buff, unsigned len) const
{
   return StaticCalculateCRC16FromBuffer(buff, len);
}

unsigned MProtocolC12::CRC16(const MByteString& buff)
{
   return StaticCalculateCRC16FromBuffer(buff.data(), M_64_CAST(unsigned, buff.size()));
}

void MProtocolC12::DoAppendTableReadResponse(MByteString& data)
{
   unsigned length = ReceiveServiceUInt(2); // two bytes to hold the response length
   if ( length != 0 )
   {
      unsigned prevSize = static_cast<unsigned>(data.size());
      data.resize(prevSize + length);
      m_applicationLayerReader.ReadBuffer(&data[prevSize], length);
   }
   unsigned char checksum = ReceiveServiceByte();
   if ( checksum != (unsigned char)StaticCalculateChecksumFromBuffer(data.data() + data.size() - length, length) )
   {
      MCOMException::Throw(MException::ErrorMeter, M_CODE_STR(MErrorEnum::InvalidChecksum, M_I("Invalid checksum")));
      M_ENSURED_ASSERT(0);
   }
}

Muint8 MProtocolC12::ReceiveServiceCode()
{
   Muint8 responseByte = m_applicationLayerReader.ReadByte(); // buff[0] is the status of command.
   if ( (MEC12NokResponse::ResponseCode)responseByte != MEC12NokResponse::RESPONSE_OK &&
        (responseByte < 0x20u ||
         responseByte >= 0x80u) // code above 0x80 are protocol extensions and it is not clear whether any of these can be error codes
        )                       // at present treat them as error codes
   {
      MByteString parameter;
      m_applicationLayerReader.ReadRemainingBytes(parameter); // this can read zero bytes
      MEC12NokResponse::ThrowWithParameters(responseByte, parameter);
      M_ENSURED_ASSERT(0);
   }
   return responseByte;
}

Muint8 MProtocolC12::ReceiveServiceByte()
{
   return m_applicationLayerReader.ReadByte();
}

unsigned MProtocolC12::ReceiveServiceUInt(unsigned size)
{
   MENumberOutOfRange::CheckUnsignedRange(0, 4, size);

   char bytes [ 4 ];
   m_applicationLayerReader.ReadBuffer(bytes, size);

   unsigned result = 0; // always big endian according to standard
   for ( unsigned i = 0; i < size; ++i )
   {
      result <<= 8;
      result |= static_cast<Muint8>(bytes[i]);
   }
   return result;
}

MByteString MProtocolC12::ReceiveServiceBytes(unsigned size)
{
   MByteString result;
   m_applicationLayerReader.ReadBytes(size, result);
   return result;
}

MByteString MProtocolC12::ReceiveServiceRemainingBytes()
{
   MByteString result;
   m_applicationLayerReader.ReadRemainingBytes(result);
   return result;
}

void MProtocolC12::DoTryPasswordEntry(const MByteString& entry)
{
   M_ASSERT(entry.size() <= 20);
   MByteString password = entry;
   unsigned diff = 20 - unsigned(password.size());
   if ( diff > 0 )
      password.append(diff, ' '); // fill the rest of the password with blanks
   try
   {
      DoApplicationLayerRequest('\x51', &password);
   }
   catch ( MEC12NokResponse& ex )
   {
      if ( ex.GetResponseCode() == MEC12NokResponse::RESPONSE_ERR || ex.GetResponseCode() == MEC12NokResponse::RESPONSE_SME ) // SME is thrown when password list is tried
         ex.SetKind(MException::ErrorSecurity); // override the type for C12 error
      throw;
   }
}

void MProtocolC12::DoEndSessionOnApplicationLayerError(bool issueOnlyTerminate)
{
   try
   {
      m_isInSession = false;

      if ( issueOnlyTerminate )
         Terminate();
      else
         EndSession();
   }
   catch( MException& )
   {
      // ignore errors of Terminate
   }
}

void MProtocolC12::DoCheckCodeTerminateAndThrowOrNotify(MEC12NokResponse& ex, bool retryCondition, unsigned int retryCount, bool issueOnlyTerminate, MProtocol* wrapperProtocol)
{
   try
   {
      if ( !retryCondition )  // "upper" C12 layer uses BSY and DNR as retriables
      {
         ex.Rethrow();
         M_ENSURED_ASSERT(0);
      }
      MProtocolServiceWrapper::StaticNotifyOrThrowRetry(wrapperProtocol, ex, retryCount);
      Sleep(m_applicationLayerRetryDelay); // sleep and retry
   }
   catch ( MEC12NokResponse& )
   {
      if ( m_endSessionOnApplicationLayerError && m_isInSession )
      {
         MEC12NokResponse::ResponseCode code = ex.GetResponseCode();
         if ( code != MEC12NokResponse::RESPONSE_ISSS && code != MEC12NokResponse::RESPONSE_RNO && code != MEC12NokResponse::RESPONSE_SME )
            DoEndSessionOnApplicationLayerError(issueOnlyTerminate);
      }
      throw;
   }
}

void MProtocolC12::DoSetNegotiatedPacketSize(unsigned negotiatedPacketSize)
{
   m_negotiatedPacketSize = negotiatedPacketSize;
}

#if !M_NO_MCOM_KEEP_SESSION_ALIVE

unsigned MProtocolC12::DoSendKeepSessionAliveMessage()
{
   unsigned firstDelay = DoGetKeepSessionAliveFirstDelay();
   if ( firstDelay == 0 )
      return 0u;

   M_ASSERT(firstDelay <= 16000);    // watch that we have a reasonable delay - prevent unsigned number overflow in the expression below
   if ( m_useReadInKeepSessionAlive )
   {
      // can't use TableReadPartial, as it is queuable.
      // Here we have to invent our own mini-service and declare it ServiceNotQueueable
      //
      MProtocolServiceWrapper wrapper(this, M_OPT_STR("TableReadPartial(ST1, 0, 1)"), MProtocolServiceWrapper::ServiceNotQueueable);
      try
      {
         MByteString data;
         DoTableReadPartial(1, data, 0, 1);
      }
      catch ( MException& ex )
      {
         wrapper.HandleFailureAndRethrow(ex);
         M_ENSURED_ASSERT(0);
      }
   }
   else
   {
      M_ASSERT(firstDelay <= 120000); // Sort of guaranteed by DoGetKeepSessionAliveFirstDelay

#if !M_NO_MCOM_CHANNEL_SOCKET
      unsigned waitDelay;
      if ( M_DYNAMIC_CAST_WITH_NULL_CHECK(MChannelSocketBase, m_channel) != 0 ) // Case of socket channel, expect bigger latency
         waitDelay = firstDelay + 8999;
      else  // Case of any other channel, smaller latency
         waitDelay = firstDelay + 2999;
#else // case when there is no socket channel
      unsigned waitDelay = firstDelay + 2999;
#endif
      M_ASSERT(waitDelay <= 255000);
      Wait(waitDelay / 1000);
   }
   return firstDelay; // Next time to call wait is after 5 seconds
}

#endif // !M_NO_MCOM_KEEP_SESSION_ALIVE
#endif // !M_NO_MCOM_PROTOCOL_C1218 || !M_NO_MCOM_PROTOCOL_C1221 || !M_NO_MCOM_PROTOCOL_C1222
