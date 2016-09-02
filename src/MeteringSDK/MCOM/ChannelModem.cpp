// File MCOM/ChannelModem.cpp

#include "MCOMExtern.h"
#include "ChannelModem.h"
#include "MCOMExceptions.h"
#include <MCORE/MTimer.h>

#if !M_NO_MCOM_CHANNEL_MODEM

M_START_PROPERTIES(ChannelModem)
   M_OBJECT_PROPERTY_PERSISTENT_BYTE_STRING(ChannelModem, AutoAnswerString,      "ATS0=1",           6, ST_constMByteStringA_X, ST_X_constMByteStringA)
   M_OBJECT_PROPERTY_PERSISTENT_BYTE_STRING(ChannelModem, InitString,            "ATZE0Q0V1&C1&D2", 15, ST_constMByteStringA_X, ST_X_constMByteStringA)
   M_OBJECT_PROPERTY_PERSISTENT_BYTE_STRING(ChannelModem, DialString,            "ATD",              3, ST_constMByteStringA_X, ST_X_constMByteStringA)
   M_OBJECT_PROPERTY_PERSISTENT_BYTE_STRING(ChannelModem, PhoneNumber,           "",                 0, ST_constMByteStringA_X, ST_X_constMByteStringA)
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ChannelModem, DialTimeout,           60u)
   M_OBJECT_PROPERTY_PERSISTENT_UINT       (ChannelModem, CommandTimeout,        10u)
   M_OBJECT_PROPERTY_PERSISTENT_BOOL       (ChannelModem, MatchConnectBaud,      false)
// Overwritten default values for the following properties:
   M_OBJECT_PROPERTY_PERSISTENT_BOOL       (ChannelModem, CtsFlow,               true)  // DOXYGEN_HIDE SWIG_HIDE
   M_OBJECT_PROPERTY_PERSISTENT_CHAR       (ChannelModem, DtrControl,            'H')   // DOXYGEN_HIDE SWIG_HIDE
   M_OBJECT_PROPERTY_READONLY_BYTE_STRING  (ChannelModem, ModemResponse,                           ST_constMByteStringA_X)
M_START_METHODS(ChannelModem)
   M_OBJECT_SERVICE                        (ChannelModem, HookOn,                                  ST_X)
   M_OBJECT_SERVICE                        (ChannelModem, SendCommand,                             ST_X_constMByteStringA)
   M_OBJECT_SERVICE                        (ChannelModem, SendCommandCheckOK,                      ST_X_constMByteStringA)
M_END_CLASS_TYPED(ChannelModem, ChannelSerialPort, "CHANNEL_MODEM")

   const unsigned s_readTimeoutCommand =  500;
   const unsigned s_writeTimeoutCommand = 1000;
   const unsigned s_dcdLowTimeThreshold = 500;
   const char s_connectStr[] = "CONNECT";
   const char s_lineCompleteChar = '\r';

   // Strict to map string and response codes to each other.
   //
   struct MStrVsResponse
   {
      // String of modem response.
      //
      const char* m_string;

      // Code of modem response.
      //
      MChannelModem::MModemResponse m_code;
   };

   // Table of responses specified by Hayes standard.
   // The order is important - those answers that shall be recognized first are mentioned first.
   //
   static const MStrVsResponse s_standardResponses[] =
      {
         { s_connectStr,  MChannelModem::MODEM_RESPONSE_CONNECT },
         { "NO CARRIER",  MChannelModem::MODEM_RESPONSE_NO_CARRIER },
         { "ERROR",       MChannelModem::MODEM_RESPONSE_ERROR },
         { "TIMEOUT",     MChannelModem::MODEM_RESPONSE_TIMEOUT },
         { "NO DIAL",     MChannelModem::MODEM_RESPONSE_NO_DIALTONE }, // "NO DIALTONE" or "NO DIAL TONE"...
         { "BUSY",        MChannelModem::MODEM_RESPONSE_BUSY },
         { "NO ANSWER",   MChannelModem::MODEM_RESPONSE_NO_ANSWER },
         { "OK",          MChannelModem::MODEM_RESPONSE_OK },  // Success is the thing that shall be least expected :-)
         { "RING",        MChannelModem::MODEM_RESPONSE_RING } // Ring has even less priority, shall be ignored if any other answer comes
      };

MChannelModem::MChannelModem()
:
   MChannelSerialPort(),
   m_connectCalled(false),
   m_modemResponse(),
   m_isDialing(0),
   m_isReceivingResponse(0)
{
   M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(ChannelModem);
}

MChannelModem::~MChannelModem()
{
   Disconnect();
}

M_NORETURN_FUNC void MChannelModem::DoThrowModemResponseError(MChannelModem::MModemResponse response)
{
   MErrorEnum::Type err = MErrorEnum::CouldNotConnectByModem;
   MConstLocalChars str;
   switch ( response )
   {
   case MChannelModem::MODEM_RESPONSE_CONNECT:
   case MChannelModem::MODEM_RESPONSE_RING:
      str = M_I("Unexpected incoming call detected at '%s'");
      break;
   case MChannelModem::MODEM_RESPONSE_BUSY:
      str = M_I("Could not connect by modem at '%s' (Line is busy)");
      break;
   case MChannelModem::MODEM_RESPONSE_TIMEOUT:
      str = M_I("Could not connect by modem at '%s' (Timeout), verify modem connection");
      break;
   case MChannelModem::MODEM_RESPONSE_NO_DIALTONE:
      str = M_I("Could not connect by modem at '%s' (No Dial Tone), verify modem connection");
      break;
   case MChannelModem::MODEM_RESPONSE_NO_ANSWER:
      str = M_I("Could not connect by modem at '%s' (No Answer)");
      break;
   case MChannelModem::MODEM_RESPONSE_NO_CARRIER:
      str = M_I("Could not connect by modem at '%s' (No Carrier)");
      break;
   case MChannelModem::MODEM_RESPONSE_ERROR:
      err = MErrorEnum::ModemError;
      str = M_I("Modem at '%s' responded with error to command");
      break;
   case MChannelModem::MODEM_RESPONSE_UNKNOWN:
      err = MErrorEnum::ModemError;
      str = M_I("Modem at '%s' did not respond or gave an unknown response to command");
      break;
   default:
      err = MErrorEnum::ModemError;
      str = M_I("Could not connect by modem at '%s'");
      break;
   }
   MCOMException::Throw(err, str, m_portName.c_str());
   M_ENSURED_ASSERT(0);
}

void MChannelModem::Connect()
{
   m_modemResponse.clear();
   MChannel::Connect();

   m_isDialing = 0;
   m_isReceivingResponse = 0;

   MChannelSerialPort::DoConnect();
   if ( !m_isAutoAnswer ) // Outgoing call. Else-variant is implemented in MChannelSerialPort::DoConnect()
   {
      try
      {
         if ( m_phoneNumber.empty() )
         {
            MCOMException::Throw(MException::ErrorSoftware, M_ERR_NO_PHONE_NUMBER_SPECIFIED, M_I("No phone number specified"));
            M_ENSURED_ASSERT(0);
         }
         HookOn();
         SendCommandCheckOK(m_initString);
         SendCommand(m_dialString + m_phoneNumber);
         MModemResponse responseCode = ReceiveKnownResponse(m_dialTimeout);
         if ( responseCode != MODEM_RESPONSE_CONNECT )
         {
            DoThrowModemResponseError(responseCode);
            M_ENSURED_ASSERT(0);
         }
         DoAdjustModemAfterConnect();
      }
      catch ( MException&
         #if !M_NO_MCOM_MONITOR
            ex
         #endif
         )
      {
         #if !M_NO_MCOM_MONITOR
            if ( m_monitor != NULL && m_monitor->IsListening() )
            {
               const MStdString str = ex.AsString();
               m_monitor->OnMessage(MMonitor::MessageProtocolLinkLayerFail, str.c_str(), M_64_CAST(unsigned, str.size()));
            }
         #endif

         // During port closing DTR also becomes low and modem automatically
         // breaks the connection if any.
         MChannelSerialPort::Disconnect();
         throw;
      }
   }
   m_connectCalled = true;
}

void MChannelModem::DoAdjustModemAfterConnect()
{
   if ( m_matchConnectBaud )
   {
      MByteString::size_type connectPos = m_modemResponse.find(s_connectStr);
      if ( connectPos != MByteString::npos ) // actually can be an assert...
      {
         const char* msg = m_modemResponse.c_str() + connectPos + (sizeof(s_connectStr) - 1);
         while ( *msg && !isdigit(*msg) ) // skip until the first digit
            ++msg;
         char* endPtr;
         unsigned long speed = strtoul(msg, &endPtr, 10);
         try
         {
            SetBaud(speed); // only changes the property
            m_port.UpdatePortParametersOrTimeoutsIfChanged(); // propagates to the UART
         }
         catch ( ... )
         {
            // If the speed returned is bad, we do not do anything.
            // In case of strtoul error, 0 will be returned, so it will satisfy us
            //     -- we will do nothing
            //
         }
      }
   }
   MUtilities::Sleep(200); // give the meter a chance to notice the connection
   FlushOutputBuffer();
   ClearInputBuffer();
   DoNotifyConnect();
}

bool MChannelModem::DoSendCommandWhileWaitingForIncomingConnection(const MByteString& command)
{
   M_ASSERT(m_isAutoAnswer);
   SendCommand(command);
   for ( ;; )
   {
      MModemResponse responseCode = ReceiveKnownResponse(m_commandTimeout);
      switch ( responseCode )
      {
      case MODEM_RESPONSE_OK:
         return false;
      case MODEM_RESPONSE_CONNECT:   // call received successfully
         DoAdjustModemAfterConnect();
         return true; // connected
      case MODEM_RESPONSE_NO_CARRIER: // something went wrong and we have disconnected
         HookOn(); // make sure we are in the command mode
         break;    //    and continue rolling through this loop (have to resend command if it was sent previously)
      case MODEM_RESPONSE_RING:    // ring is ignored in auto answer mode, loop again
         break;
      default: // every other error is reported
         DoThrowModemResponseError(responseCode);
         M_ENSURED_ASSERT(0);
      }
   }
}

void MChannelModem::WaitForNextIncomingConnection(bool initialize)
{
   if ( !m_isAutoAnswer )
   {
      MChannel::WaitForNextIncomingConnection(false); // this throws exception "channel is not in answer mode"
      M_ENSURED_ASSERT(0);
   }
   HookOn(); // make sure we are in the command mode

   if ( (initialize && DoSendCommandWhileWaitingForIncomingConnection(m_initString)) ||
        DoSendCommandWhileWaitingForIncomingConnection(m_autoAnswerString) )
   {
      return; // connected
   }

   MTimer endTime(MTimer::SecondsToTimerMilliseconds(m_autoAnswerTimeout)); // timeout is in seconds
   do
   {
      MModemResponse responseCode = ReceiveKnownResponse(1); // every second
      switch ( responseCode )
      {
      case MODEM_RESPONSE_CONNECT:   // call received successfully
         DoAdjustModemAfterConnect();
         return; // connected
      case MODEM_RESPONSE_NO_CARRIER: // something went wrong and we have disconnected
         HookOn(); // make sure we are in the command mode
         break;    //    and continue rolling through this loop (have to resend command if it was sent previously)
      case MODEM_RESPONSE_OK:
         M_ASSERT(0); // Notify on debug, ignore such strange "okay" in release
         break;
      case MODEM_RESPONSE_RING:    // ring is ignored in auto answer mode
         break;
      case MODEM_RESPONSE_TIMEOUT: // timeout is handled below in the loop condition
         break;
      default: // every other error is reported
         DoThrowModemResponseError(responseCode);
         M_ENSURED_ASSERT(0);
      }
   } while ( !endTime.IsExpired() );

   MCOMException::Throw(M_ERR_TIMED_OUT_WHILE_WAITING_FOR_CONNECTION, M_I("Timed out while waiting for connection by modem"));
   M_ENSURED_ASSERT(0);
}

void MChannelModem::Disconnect()
{
   if ( MChannelSerialPort::IsConnected() )
   {
      try
      {
         FlushOutputBuffer();
         MUtilities::Sleep(100);
         HookOn();
      }
      catch ( ... ) // ignore exceptions at disconnect
      {
      }
   }
   MChannelSerialPort::Disconnect(); // call disconnect in any case
   m_connectCalled = false;
}

bool MChannelModem::IsConnected() const
{
   if ( !m_connectCalled ) // quick check
      return false;

   try
   {
      // Honest check of the port and modem status
      //
      return MChannelSerialPort::IsConnected() &&
             !m_isDialing && !m_isReceivingResponse &&  // these two are needed to protect from a bad call from a separate thread!
             MChannelSerialPort::GetDCD() == true;      // If GetDCD is called during dialup, it holds until the modem is dialed up!
   }
   catch ( ... ) // IsConnected never throws exception, assume not connected otherwise
   {
   }
   return false;
}

void MChannelModem::SendCommand(const MByteString& command)
{
   m_modemResponse.clear();

   MChannelSerialPort::ClearInputBuffer();
   CheckIfOperationIsCancelled(); // possibly throw MEOperationCancelled

   MByteString commandCR = command;
   commandCR += s_lineCompleteChar;

   unsigned savedWriteTimeout = GetWriteTimeout();
   SetWriteTimeout(s_writeTimeoutCommand);

   m_isDialing = 1;
   try
   {
      // We use direct port writes because they do not echo and do not check if connected
      MChannelSerialPort::DoWrite(commandCR.data(), M_64_CAST(unsigned, commandCR.size()));
      DoNotifyByteTX(commandCR.data(), M_64_CAST(unsigned, commandCR.size()));
   }
   catch ( MException& ex )
   {
      ex.AppendToString(M_I(". Verify modem connection on '%s'"), m_portName.c_str());
      m_isDialing = 0;
      SetWriteTimeout(savedWriteTimeout); // recover the channel timeout in case of error
      CheckIfOperationIsCancelled(); // possibly throw MEOperationCancelled
      throw;
   }
   m_isDialing = 0;
   SetWriteTimeout(savedWriteTimeout); // recover the channel timeout in case of error
}

MChannelModem::MModemResponse MChannelModem::ReceiveKnownResponse(int timeout)
{
   MModemResponse responseCode = MODEM_RESPONSE_UNKNOWN;
   MByteString response;

   M_ASSERT(timeout >= 0);
   MTimer endTime(MTimer::SecondsToTimerMilliseconds(timeout)); // timeout is in seconds
   m_isReceivingResponse = 1;
   MValueEndScopeSetter<MInterlocked> responseEndScopeSetter(&m_isReceivingResponse, 0);
   do
   {
      char buff [ 128 ];
      unsigned len = DoReadCancellable(buff, sizeof(buff), s_readTimeoutCommand, true);
      response.append(buff, len);
      if ( response.empty() ) // at a minimum, expect two symbols
         MUtilities::Sleep(100);
      else
      {
         MAlgorithm::InplaceTrim(response, "\r\n \t"); // remove surrounding blanks of all expected kinds (do not remove those, which are between words)
         for ( unsigned i = 0; i < M_NUMBER_OF_ARRAY_ELEMENTS(s_standardResponses); ++i )
         {
            MByteString::size_type pos = response.find(s_standardResponses[i].m_string);
            if ( pos != MByteString::npos && (pos == 0 || m_isspace(response[pos - 1])) ) // we expect the string starts with the recognized answer
            {
               responseCode = s_standardResponses[i].m_code;
               goto HANDLE_RESPONSE_AND_RETURN;
            }
         }

         // If we did not recognize anything, then remove any older lines from response (leaving the last line alone).
         //
         MByteString::size_type pos = response.find_last_of("\n\r");
         if ( pos != MByteString::npos )
            response.erase(0, pos);

      }
   } while ( !endTime.IsExpired() );

HANDLE_RESPONSE_AND_RETURN:

   if ( response.empty() )
      return MODEM_RESPONSE_TIMEOUT; // and do not overwrite m_modemResponse for this case

   MByteString::iterator it = response.begin();
   MByteString::iterator itEnd = response.end();
   for ( ; it != itEnd; ++it )
   {
      unsigned c = (unsigned)(unsigned char)*it;
      if ( c < ' ' || c > '~' ) // we do not use isprint, as it is locale-dependent
         *it = ' ';
   }
   m_modemResponse = response;
   return responseCode;
}

void MChannelModem::SendCommandCheckOK(const MByteString& command)
{
   SendCommand(command);
   MModemResponse responseCode = ReceiveKnownResponse(m_commandTimeout);
   if (  responseCode != MODEM_RESPONSE_OK )
   {
      DoThrowModemResponseError(responseCode);
      M_ENSURED_ASSERT(0);
   }
}

void MChannelModem::CancelCommunication(bool callDisconnect)
{
   MChannel::CancelCommunication(callDisconnect);
   try
   {
      if ( m_isDialing != 0 )
      {
         m_isDialing = 0;
         HookOn(); // try to interrupt a long command
      }
      else if ( m_isReceivingResponse != 0 )
      {
         m_isReceivingResponse = 0;
         char c = '\3'; // Control-C
         m_port.Write(&c, 1);
      }
   }
   catch ( ... ) // do nothing if the attempt is unsuccessful
   {
   }
}

void MChannelModem::HookOn()
{
   // Set DTR low to hook on.
   SetDtrControl('D');
   MUtilities::Sleep(400);
   SetDtrControl('E');
   ClearInputBuffer();
}

void MChannelModem::CheckIfConnected()
{
   if ( !m_connectCalled )
      MChannelSerialPort::CheckIfConnected(); // call parent, throw
   else if ( !GetDCD() ) // if DCD is low, we are either temporarily or constantly disconnected...
   {
      MTimer timer;
      do
      {
         MUtilities::Sleep(50); // yield the thread, plus some little nap
         if ( GetDCD() ) // connection is reestablished
            return; // success, no throw, we reconnected back!
      } while ( timer < s_dcdLowTimeThreshold );

      m_connectCalled = false; // this way, notify only once

      // We shall close the port in this case, as we are disconnected....
      // Do not call Disconnect from neither class, as we shall not notify
      // successful disconnecting (an error notification will be made instead)
      //
      m_port.Close();

      MEChannelDisconnectedUnexpectedly e;
      e.AppendToString(M_I(". Modem connection lost"));
      e.Rethrow();
      M_ENSURED_ASSERT(0);
   }
}

#endif // !M_NO_MCOM_CHANNEL_MODEM
