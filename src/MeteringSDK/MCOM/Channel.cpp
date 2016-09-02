// File MCOM/Channel.cpp

#include "MCOMExtern.h"
#include "Channel.h"
#include "MCOMExceptions.h"
#include "MCOMFactory.h"

M_START_PROPERTIES(Channel)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL     (Channel, AutoAnswer,             false)
   M_OBJECT_PROPERTY_PERSISTENT_UINT     (Channel, AutoAnswerTimeout,        60u)
   M_OBJECT_PROPERTY_PERSISTENT_UINT     (Channel, IntercharacterTimeout,   500u)
   M_OBJECT_PROPERTY_PERSISTENT_UINT     (Channel, ReadTimeout,            1000u)
   M_OBJECT_PROPERTY_PERSISTENT_UINT     (Channel, WriteTimeout,           2000u)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL     (Channel, Echo,                   false)
#if !M_NO_MCOM_MONITOR
   M_OBJECT_PROPERTY_PERSISTENT_BOOL     (Channel, SendEchoBytesToMonitor, false)
#endif
   M_OBJECT_PROPERTY_READONLY_UINT       (Channel, CountBytesSent)
   M_OBJECT_PROPERTY_READONLY_UINT       (Channel, CountBytesReceived)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT (Channel, IsConnected)
   M_OBJECT_PROPERTY_READONLY_STRING     (Channel, MediaIdentification,               ST_MStdString_X)
#if !M_NO_MCOM_MONITOR
   M_OBJECT_PROPERTY_OBJECT_OVERLOADED   (Channel, Monitor, DoGetMonitor, DoSetMonitor)
#endif
M_START_METHODS(Channel)
   M_OBJECT_SERVICE                      (Channel, WriteBytes,                        ST_X_constMByteStringA)
   M_OBJECT_SERVICE                      (Channel, WriteChar,                         ST_X_byte) // actually writes a byte
   M_OBJECT_SERVICE                      (Channel, ReadChar,                          ST_byte_X) // actually reads a byte
   M_OBJECT_SERVICE                      (Channel, Unread,                            ST_X_constMVariantA)
   M_OBJECT_SERVICE                      (Channel, ReadBytes,                         ST_MByteString_X_unsigned)
   M_OBJECT_SERVICE                      (Channel, ReadBytesUntil,                    ST_MByteString_X_constMByteStringA)
   M_OBJECT_SERVICE                      (Channel, ReadAllBytes,                      ST_MByteString_X)
   M_OBJECT_SERVICE                      (Channel, Connect,                           ST_X)
   M_OBJECT_SERVICE                      (Channel, Disconnect,                        ST_X)
   M_OBJECT_SERVICE                      (Channel, ClearInputBuffer,                  ST_X)
   M_OBJECT_SERVICE                      (Channel, ClearInputUntilSilence,            ST_X_unsigned)
   M_OBJECT_SERVICE                      (Channel, FlushOutputBuffer,                 ST_X_unsigned)
   M_OBJECT_SERVICE                      (Channel, ResetCounts,                       ST_X)
   M_OBJECT_SERVICE                      (Channel, CheckIfConnected,                  ST_X)
   M_OBJECT_SERVICE                      (Channel, WriteToMonitor,                    ST_X_constMStdStringA)
   M_OBJECT_SERVICE_OVERLOADED           (Channel, EnterUninterruptibleCommunication, EnterUninterruptibleCommunication,    1, ST_X_bool)
   M_OBJECT_SERVICE_OVERLOADED           (Channel, EnterUninterruptibleCommunication, DoEnterUninterruptibleCommunication0, 0, ST_X) // SWIG_HIDE
   M_OBJECT_SERVICE_OVERLOADED           (Channel, LeaveUninterruptibleCommunication, LeaveUninterruptibleCommunication,    1, ST_X_bool)
   M_OBJECT_SERVICE_OVERLOADED           (Channel, LeaveUninterruptibleCommunication, DoLeaveUninterruptibleCommunication0, 0, ST_X) // SWIG_HIDE
   M_OBJECT_SERVICE                      (Channel, CancelCommunication,               ST_X_bool)
   M_OBJECT_SERVICE                      (Channel, CheckIfOperationIsCancelled,       ST_X)
   M_OBJECT_SERVICE                      (Channel, WaitForNextIncomingConnection,     ST_X_bool)
   M_OBJECT_SERVICE                      (Channel, Sleep,                             ST_X_unsigned)
M_END_CLASS_TYPED(Channel, COMObject, "CHANNEL")

MChannel::MChannel()
:
#if !M_NO_MCOM_MONITOR
   m_monitor(NULL),
#endif // !M_NO_MCOM_MONITOR
   m_cancelCommunicationGuard(0),
   m_cancelCommunication(0),
   m_countBytesSent(0u),
   m_countBytesReceived(0u),
   m_unreadBuffer()
{
   M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(Channel);
}

MChannel::~MChannel()
{
}

#if !M_NO_MCOM_FACTORY // Note that factory is not available without reflection
MChannel* MChannel::CreateClone() const
{
   return MCOMFactory::CreateChannel(GetPersistentPropertyValues(true));
}
#endif

void MChannel::Connect()
{
   if ( IsConnected() )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR_P1(M_ERR_CANNOT_CONNECT_CHANNEL_S1_IS_ALREADY_CONNECTED, "Cannot connect channel '%s' because it is already connected", GetMediaIdentification().c_str()));
      M_ENSURED_ASSERT(0);
   }
   DoInitChannel();
}

void MChannel::CheckIfConnected()
{
   if ( !IsConnected() )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_CONNECTION_NOT_ESTABLISHED_BUT_REQUIRED, "Connection not established, connection is required for this operation"));
      M_ENSURED_ASSERT(0);
   }
}

void MChannel::WaitForNextIncomingConnection(bool)
{
   if ( !m_isAutoAnswer )
   {

      MCOMException::Throw(M_CODE_STR(M_ERR_CHANNEL_NOT_IN_ANSWER_MODE, "Channel is expected to be in answer mode"));
      M_ENSURED_ASSERT(0);
   }
}


void MChannel::SetIntercharacterTimeout(unsigned timeout)
{
   MENumberOutOfRange::CheckNamedUnsignedRange(0, unsigned(INT_MAX), timeout, M_OPT_STR("INTERCHARACTER_TIMEOUT"));
   m_intercharacterTimeout = timeout;
}

void MChannel::SetReadTimeout(unsigned timeout)
{
   MENumberOutOfRange::CheckNamedUnsignedRange(0, unsigned(INT_MAX), timeout, M_OPT_STR("READ_TIMEOUT"));
   m_readTimeout = timeout;
}

void MChannel::SetWriteTimeout(unsigned timeout)
{
   MENumberOutOfRange::CheckNamedUnsignedRange(0, unsigned(INT_MAX), timeout, M_OPT_STR("WRITE_TIMEOUT"));
   m_writeTimeout = timeout;
}

void MChannel::SetAutoAnswerTimeout(unsigned timeout)
{
   MENumberOutOfRange::CheckNamedUnsignedRange(0, unsigned(INT_MAX), timeout, M_OPT_STR("AUTO_ANSWER_TIMEOUT"));
   m_autoAnswerTimeout = timeout;
}

void MChannel::CancelCommunication(bool callDisconnect)
{
   m_cancelCommunication = callDisconnect ? 2 : 1;
   // and don't even touch m_cancelCommunicationGuard here!
}

void MChannel::EnterUninterruptibleCommunication(bool notify) M_NO_THROW
{
   int result = ++m_cancelCommunicationGuard;
   #if M_NO_MCOM_MONITOR
      M_USED_VARIABLE(result);
   #else
      if ( notify && result == 1 && m_monitor != NULL && m_monitor->IsListening() )
      {
         try // the method shall throw no exception
         {
            m_monitor->OnEnterUninterruptibleCommunication();
         }
         catch ( ... )
         {
            M_ASSERT(0); // debug feature only
         }
      }
   #endif
}

void MChannel::DoEnterUninterruptibleCommunication0() M_NO_THROW
{
   EnterUninterruptibleCommunication();
}

void MChannel::LeaveUninterruptibleCommunication(bool notify) M_NO_THROW
{
   int result = --m_cancelCommunicationGuard;
   M_ASSERT(result >= 0);
   M_USED_VARIABLE(result);
   #if !M_NO_MCOM_MONITOR
      if ( notify && result == 0 && m_monitor != NULL && m_monitor->IsListening() )
      {
         try // the method shall throw no exception
         {
            m_monitor->OnLeaveUninterruptibleCommunication();
         }
         catch ( ... )
         {
            M_ASSERT(0); // debug feature only
         }
      }
   #endif
}

void MChannel::DoLeaveUninterruptibleCommunication0() M_NO_THROW
{
   LeaveUninterruptibleCommunication();
}

void MChannel::CheckIfOperationIsCancelled()
{
   int cancel = m_cancelCommunication;
   if ( cancel != 0 &&                    // this check is first
        m_cancelCommunicationGuard == 0 ) // this should be second
   {
      m_cancelCommunication = 0; // reset the request right before throwing an exception
      if ( cancel == 2 )
         Disconnect(); // disconnect does not throw exception

      MEOperationCancelled::Throw();
      M_ENSURED_ASSERT(0);
   }

   // and don't even touch m_cancelCommunicationGuard here!
}

MByteString MChannel::ReadBytes(unsigned numberToRead)
{
   MByteString result(numberToRead, '\0');
   ReadBuffer(const_cast<char*>(result.data()), numberToRead);
   return result;
}

unsigned MChannel::DoReadCancellable(char* buf, unsigned size, unsigned timeout, bool sendToMonitor)
{
   unsigned result = 0;

   if ( (int)timeout < 0 )
      timeout = INT_MAX; // the below code works with signed integers

   unsigned endTime = MUtilities::GetTickCount() + timeout;
   char*    localBuf = buf;

   // inserting unread bytes
   unsigned unreadSize = 0u;
   if ( !m_unreadBuffer.empty() )
   {
      unreadSize =  M_64_CAST(unsigned, m_unreadBuffer.size()) < size ?  M_64_CAST(unsigned, m_unreadBuffer.size()) : size;
      memcpy(localBuf, m_unreadBuffer.data(), unreadSize);

      localBuf += unreadSize;
      m_unreadBuffer.erase((size_t)0, (size_t)unreadSize);
      result += unreadSize;
   }

   unsigned remainingSize = size - unreadSize;
   if ( remainingSize == 0 )
   {
      M_ASSERT(result == size);
      return result;
   }
   unsigned remainingTimeout = timeout;
   for ( ; ;  )
   {
      if ( remainingTimeout > CANCEL_COMMUNICATION_CHECK_OPTIMUM_INTERVAL )
         remainingTimeout = CANCEL_COMMUNICATION_CHECK_OPTIMUM_INTERVAL;
      unsigned localResult = DoRead(localBuf, remainingSize, remainingTimeout);
      if ( localResult > 0  )
      {
         if ( sendToMonitor )
            DoNotifyByteRX(localBuf, localResult);
         result += localResult;
         break;
      }
      CheckIfOperationIsCancelled();

      remainingTimeout = endTime - MUtilities::GetTickCount();
      if ( (int)remainingTimeout <= 0 )
         break;
   }
   return result;
}

void MChannel::ReadBuffer(char* buf, unsigned size)
{
   unsigned resultSize = ReadWithTimeout(buf, size, m_readTimeout);
   if ( resultSize != size )
   {
      M_ASSERT(resultSize < size);
      MEChannelReadTimeout::Throw(resultSize);
      M_ENSURED_ASSERT(0);
   }
}

unsigned MChannel::ReadWithTimeout(char* buf, unsigned size, unsigned timeout)
{
   CheckIfConnected();
   CheckIfOperationIsCancelled();

   unsigned actualSize = 0;
   if ( size != 0 )
   {
      char* localBuf = buf;
      unsigned remainingSize = size;
      if ( m_intercharacterTimeout != 0 ) // regular timeout handling with intercharacter watch
      {
         for ( ; ; )
         {
            unsigned localSize = DoReadCancellable(localBuf, remainingSize, timeout, true);
            if ( localSize == 0 ) // did not read anything per timeout cycle
               break;
            localBuf += localSize;
            actualSize += localSize;
            remainingSize -= localSize;
            if ( remainingSize == 0 )
               break;
            timeout = m_intercharacterTimeout;
         }
      }
      else // special case of zero intercharacter timeout - read timeout is responsible for the whole packet
      {
         unsigned endTime = MUtilities::GetTickCount() + timeout;
         for ( ;; )
         {
            unsigned localSize = DoReadCancellable(localBuf, remainingSize, timeout, true);
            if ( localSize == 0 ) // did not read anything per timeout cycle
               break;
            localBuf += localSize;
            actualSize += localSize;
            remainingSize -= localSize;
            if ( remainingSize == 0 )
               break;
            int timeDiff = int(endTime - MUtilities::GetTickCount());
            if ( timeDiff <= 0 )
               break;
            timeout = unsigned(timeDiff);
         }
      }
   }
   return actualSize;
}

#if !M_NO_VARIANT
void MChannel::Unread(const MVariant& byteOrBytes)
{
   if ( byteOrBytes.IsNumeric() )
   {
      char byte = static_cast<char>(byteOrBytes.AsByte());
      UnreadBuffer(&byte, 1);
   }
   else if ( !byteOrBytes.IsEmpty() )
   {
      const MByteString& bytes = byteOrBytes.AsByteString();
      UnreadBuffer(bytes.data(), static_cast<unsigned>(bytes.size()));
   }
}
#endif

void MChannel::UnreadBuffer(const char* buff, unsigned size)
{
   CheckIfConnected();

   m_unreadBuffer.insert((size_t)0, buff, size);

   #if !M_NO_MCOM_MONITOR

      // Reuse echo bytes visibility for unread
      if ( m_sendEchoBytesToMonitor && m_monitor != NULL && m_monitor->IsListening() )
      {
         MStdString msg;
         msg.reserve(size * 3 + 8);
         msg.assign("Unread(", 7);
         msg += MUtilities::BufferToHex(buff, size, true);
         msg += ')';
         m_monitor->Write(msg);
      }

   #endif // M_NO_MCOM_MONITOR
}

MByteString MChannel::ReadAllBytes()
{
   MByteString result;

   CheckIfConnected();
   CheckIfOperationIsCancelled();

   char buff [ 0x2000 ];
   unsigned timeout = m_readTimeout;
   for ( ;; )
   {
      unsigned localSize = DoReadCancellable(buff, sizeof(buff), timeout, true);
      if ( localSize == 0 )
         break; // done reading available bytes
      result.append(buff, localSize);
      if ( localSize != sizeof(buff) ) // we've read all available bytes
         break;
      timeout = 0; // otherwise attempt one extra cycle, do not wait if there is nothing in the buffer
   }
   return result;
}

void MChannel::DoInitChannel()
{
   m_cancelCommunication = 0;
   m_cancelCommunicationGuard = 0;

   m_unreadBuffer.clear();

   #if !M_NO_MCOM_MONITOR
      if ( m_monitor != NULL ) // don't check for m_monitor->IsListening here!
      {
         m_monitor->Attach(GetMediaIdentification());
         #if !M_NO_MCOM_MONITOR && !M_NO_REFLECTION
            WritePropertiesToMonitor();
         #endif
      }
   #endif
}

void MChannel::ClearInputBuffer()
{
   m_unreadBuffer.clear();
   DoClearInputBuffer();
}

void MChannel::ClearInputUntilSilence(unsigned milliseconds)
{
   char buff [ 1024 ];
   while ( DoReadCancellable(buff, sizeof(buff), milliseconds, true) > 0 )
      ;
}

MByteString MChannel::ReadBytesUntil(const MByteString& terminatingString)
{
   MByteString result;
   if ( !terminatingString.empty() ) // otherwise return immediately
   {
      result = ReadBytes(unsigned(terminatingString.size()));

      MChannel::ReadTimeoutSavior timeoutSavior(this, m_intercharacterTimeout);
      const char finisher = *terminatingString.begin();
      if ( terminatingString.size() == 1 ) // a usual case, handle it efficiently
      {
         for ( ;; )
         {
            if ( *result.rbegin() == finisher )
               return result;
            result += ReadChar();
         }
      }
      else if ( terminatingString.find(finisher, 1) == MByteString::npos ) // if there is no finisher repeated in string
      {
         const unsigned footerSize = (unsigned)terminatingString.size() - 1;
         const char* const terminatingRemainder = terminatingString.data() + 1; // help the compiler figure out it does not change in the loop
         do
         {
            result += ReadBytesUntilAnyByte(&finisher, 1, 0, footerSize);
         } while ( memcmp(result.data() + result.size() - footerSize, terminatingRemainder, footerSize) != 0 );
      }
      else // rare complex case, do it simple, but inefficiently
      {
#ifdef M_USE_USTL
         while ( result.substr(result.size() - terminatingString.size(), terminatingString.size()).compare(terminatingString) != 0 )
            result += ReadChar();
#else
         while ( result.compare(result.size() - terminatingString.size(), terminatingString.size(), terminatingString) != 0 )
            result += ReadChar();
#endif
      }
   }
   return result;
}

   inline bool DoExistIn(char ch, const char* finisher, unsigned finisherSize)
   {
      const char* it = finisher;
      const char* itEnd = it + finisherSize;
      for ( ; it != itEnd; ++it )
         if ( *it == ch )
            return true;
      return false;
   }

MByteString MChannel::ReadBytesUntilAnyByte(const char* finisher, unsigned finisherSize, unsigned headerSize, unsigned footerSize)
{
   MByteString result;

   result = ReadBytes(headerSize + footerSize + 1);
   MChannel::ReadTimeoutSavior timeoutSavior(this, m_intercharacterTimeout);
   if ( footerSize == 0 )
   {
      while ( DoExistIn(*result.rbegin(), finisher, finisherSize) )
         result += ReadChar();
   }
   else
   {
      ++footerSize; // make footer size include finisher char
      for ( ;; )
      {
         MByteString::const_iterator itEnd = result.end();
         for ( MByteString::const_iterator it = itEnd - footerSize; it != itEnd; ++it )
         {
            if ( DoExistIn(*it, finisher, finisherSize) )
            {
               unsigned remainingBytes = footerSize - unsigned(itEnd - it);
               if ( remainingBytes > 0 )
                  result += ReadBytes(remainingBytes);
               return result;
            }
         }
         result += ReadBytes(footerSize);
      }
   }
   return result;
}

void MChannel::WriteBytes(const MByteString &buf)
{
   WriteBuffer(buf.c_str(), M_64_CAST(unsigned, buf.size()));
}

void MChannel::WriteChar(char buf)
{
   WriteBuffer(&buf, 1);
}

char MChannel::ReadChar()
{
   char ch;
   ReadBuffer(&ch, 1);
   return ch;
}

#if !M_NO_MCOM_MONITOR

void MChannel::SetMonitor(MMonitor::Pointer monitor)
{
   if ( monitor != NULL )
   {
      M_DYNAMIC_CAST_WITH_THROW(MMonitor, monitor); // this throws an exception if a bad monitor object is given
   }
   m_monitor = monitor;
}

#endif

void MChannel::WriteToMonitor(const MStdString& message)
{
   #if !M_NO_MCOM_MONITOR
      if ( m_monitor != NULL )
      {
         if ( !m_monitor->IsListening() )
            m_monitor->Attach(GetMediaIdentification());
         m_monitor->Write(message);
      }
   #endif
}

M_NORETURN_FUNC void MChannel::DoThrowCharactersNotEchoed()
{
   CheckIfOperationIsCancelled();
   MCOMException::Throw(M_CODE_STR(MErrorEnum::CharactersNotEchoed, M_I("Characters echoed did not match ones sent. Not a current loop device?")));
   M_ENSURED_ASSERT(0);
}

void MChannel::WriteBuffer(const char* buf, unsigned len)
{
   CheckIfConnected();

   unsigned actualLen = DoWrite(buf, len);
   if ( actualLen > 0 )
      DoNotifyByteTX(buf, actualLen);

   if ( actualLen != len )
   {
      CheckIfOperationIsCancelled();
      MEChannelWriteTimeout::Throw(actualLen);
      M_ENSURED_ASSERT(0);
   }

   if ( m_echo ) // read the written characters back
   {
      const unsigned echoBuffLen = 256;
      char echoBuff [ echoBuffLen ];
      for ( unsigned i = 0; ; )
      {
         unsigned buffLen = len - i;
         if ( int(buffLen) <= 0 )
            break;
         if ( buffLen > echoBuffLen )
            buffLen = echoBuffLen;

#if !M_NO_MCOM_MONITOR
         unsigned newBuffLen = DoReadCancellable(echoBuff, buffLen, m_intercharacterTimeout, m_sendEchoBytesToMonitor);
#else
         unsigned newBuffLen = DoReadCancellable(echoBuff, buffLen, m_intercharacterTimeout, false);
#endif
         if ( buffLen != newBuffLen || memcmp(buf + i, echoBuff, newBuffLen) != 0 )
         {
            DoThrowCharactersNotEchoed();
            M_ENSURED_ASSERT(0);
         }
         i += newBuffLen;
      }
   }
}

void MChannel::Sleep(unsigned milliseconds)
{
   if ( milliseconds <= CANCEL_COMMUNICATION_CHECK_OPTIMUM_INTERVAL )
   {
      MUtilities::Sleep(milliseconds);
      CheckIfOperationIsCancelled();
   }
   else
   {
      unsigned currTime = MUtilities::GetTickCount();
      unsigned nextTimeMark = currTime + CANCEL_COMMUNICATION_CHECK_OPTIMUM_INTERVAL;
      unsigned endTimeMark = currTime + milliseconds;
      for ( ;; )
      {
         unsigned deltaCurr = nextTimeMark - currTime;
         unsigned deltaEnd = endTimeMark - currTime;
         if ( deltaEnd <= deltaCurr )
         {
            MUtilities::Sleep(deltaEnd);
            CheckIfOperationIsCancelled();
            break;
         }
         MUtilities::Sleep(deltaCurr);
         CheckIfOperationIsCancelled();
         currTime = MUtilities::GetTickCount();
         if ( currTime >= endTimeMark )
            break;
         nextTimeMark += CANCEL_COMMUNICATION_CHECK_OPTIMUM_INTERVAL;
      }
   }
}

void MChannel::DoClearInputBuffer()
{
   // this method is as pure virtual
   M_ASSERT(0);
}
