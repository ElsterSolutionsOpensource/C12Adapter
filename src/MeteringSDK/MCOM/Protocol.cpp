// File MCOM/Protocol.cpp

#include "MCOMExtern.h"
#include "Protocol.h"
#include "ProtocolThread.h"
#include "ChannelOpticalProbe.h"
#include "ChannelModem.h"
#include "MCOMExceptions.h"
#include "MCOMFactory.h"
#include <MCORE/MStreamMemory.h>

M_START_PROPERTIES(Protocol)
   M_OBJECT_PROPERTY_PERSISTENT_BYTE_STRING(Protocol, Password, "\0\0\0\0", 4, ST_MByteString_X, ST_X_constMByteStringA)
   M_OBJECT_PROPERTY_BOOL                  (Protocol, MeterIsLittleEndian)
   M_OBJECT_PROPERTY_READONLY_UINT         (Protocol, CountApplicationLayerServicesSuccessful)
   M_OBJECT_PROPERTY_READONLY_UINT         (Protocol, CountApplicationLayerServicesRetried)
   M_OBJECT_PROPERTY_READONLY_UINT         (Protocol, CountApplicationLayerServicesFailed)
   M_OBJECT_PROPERTY_READONLY_UINT         (Protocol, CountLinkLayerPacketsSuccessful)
   M_OBJECT_PROPERTY_READONLY_UINT         (Protocol, CountLinkLayerPacketsRetried)
   M_OBJECT_PROPERTY_READONLY_UINT         (Protocol, CountLinkLayerPacketsFailed)
   M_OBJECT_PROPERTY_READONLY_UINT         (Protocol, MaximumRoundTripTime)
   M_OBJECT_PROPERTY_READONLY_UINT         (Protocol, MinimumRoundTripTime)
   M_OBJECT_PROPERTY_READONLY_UINT         (Protocol, AverageRoundTripTime)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT   (Protocol, IsConnected)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT   (Protocol, IsInSession)
   M_OBJECT_PROPERTY_OBJECT                (Protocol, Channel)
   M_OBJECT_PROPERTY_BOOL_EXACT            (Protocol, IsChannelOwned)
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   M_OBJECT_PROPERTY_BYTE_STRING_COLLECTION(Protocol, PasswordList, ST_constMByteStringVectorA_X)
   M_OBJECT_PROPERTY_READONLY_INT          (Protocol, PasswordListSuccessfulEntry)
#endif
#if !M_NO_MCOM_KEEP_SESSION_ALIVE
   M_OBJECT_PROPERTY_PERSISTENT_BOOL       (Protocol, KeepSessionAlive, false)
#endif
M_START_METHODS(Protocol)
   M_OBJECT_SERVICE           (Protocol, ApplyChannelParameters,             ST_X)
   M_OBJECT_SERVICE           (Protocol, Connect,                            ST_X)
   M_OBJECT_SERVICE           (Protocol, Disconnect,                         ST_X)
   M_OBJECT_SERVICE           (Protocol, StartSession,                       ST_X)
   M_OBJECT_SERVICE           (Protocol, EndSession,                         ST_X)
   M_OBJECT_SERVICE           (Protocol, EndSessionNoThrow,                  ST_X)
   M_OBJECT_SERVICE_OVERLOADED(Protocol, TableRead, TableRead,          2,   ST_MByteString_X_constMVariantA_unsigned)
   M_OBJECT_SERVICE_OVERLOADED(Protocol, TableRead, DoTableReadImpl,    1,   ST_MByteString_X_constMVariantA)  // SWIG_HIDE
   M_OBJECT_SERVICE           (Protocol, TableWrite,                         ST_X_constMVariantA_constMByteStringA)
   M_OBJECT_SERVICE           (Protocol, TableReadPartial,                   ST_MByteString_X_constMVariantA_int_int)
   M_OBJECT_SERVICE           (Protocol, TableWritePartial,                  ST_X_constMVariantA_constMByteStringA_int)
   M_OBJECT_SERVICE           (Protocol, FunctionExecute,                    ST_X_constMVariantA)
   M_OBJECT_SERVICE           (Protocol, FunctionExecuteRequest,             ST_X_constMVariantA_constMByteStringA)
   M_OBJECT_SERVICE           (Protocol, FunctionExecuteResponse,            ST_MByteString_X_constMVariantA)
   M_OBJECT_SERVICE           (Protocol, FunctionExecuteRequestResponse,     ST_MByteString_X_constMVariantA_constMByteStringA)
   M_OBJECT_SERVICE           (Protocol, ResetCounts,                        ST_X)
   M_OBJECT_SERVICE           (Protocol, CalculateChecksum,                  ST_unsigned_X_constMByteStringA)
   M_OBJECT_SERVICE           (Protocol, CalculateCRC16,                     ST_unsigned_X_constMByteStringA)
   M_OBJECT_SERVICE           (Protocol, GetNumberOfDataLinkPackets,         ST_unsigned_X_unsigned_unsigned) // SWIG_HIDE
   M_OBJECT_SERVICE           (Protocol, WriteCountsToMonitor,               ST_X)
#if !M_NO_MCOM_IDENTIFY_METER
   M_OBJECT_SERVICE_OVERLOADED(Protocol, IdentifyMeter, IdentifyMeter,    1, ST_MStdString_X_bool)
   M_OBJECT_SERVICE_OVERLOADED(Protocol, IdentifyMeter, DoIdentifyMeter0, 0, ST_MStdString_X)  // SWIG_HIDE
#endif
   M_OBJECT_SERVICE           (Protocol, ReadStartByte,                      ST_byte_X_constMByteStringA_unsigned)
   M_OBJECT_SERVICE           (Protocol, Sleep,                              ST_X_unsigned)
#if !M_NO_MCOM_COMMAND_QUEUE
#if !M_NO_MCOM_PROTOCOL_THREAD
   M_OBJECT_SERVICE           (Protocol, QNeedToCommit,                      ST_bool_X)
   M_OBJECT_SERVICE           (Protocol, QIsDone,                            ST_bool_X)
#endif
   M_OBJECT_SERVICE           (Protocol, QConnect,                           ST_X)
   M_OBJECT_SERVICE           (Protocol, QDisconnect,                        ST_X)
   M_OBJECT_SERVICE           (Protocol, QStartSession,                      ST_X)
   M_OBJECT_SERVICE           (Protocol, QEndSession,                        ST_X)
   M_OBJECT_SERVICE           (Protocol, QEndSessionNoThrow,                 ST_X)
#if !M_NO_MCOM_IDENTIFY_METER
   M_OBJECT_SERVICE           (Protocol, QIdentifyMeter,                     ST_X)
#endif
   M_OBJECT_SERVICE           (Protocol, QTableRead,                         ST_X_constMVariantA_unsigned_int)
   M_OBJECT_SERVICE           (Protocol, QTableWrite,                        ST_X_constMVariantA_constMByteStringA)
   M_OBJECT_SERVICE           (Protocol, QTableReadPartial,                  ST_X_constMVariantA_int_int_int)
   M_OBJECT_SERVICE           (Protocol, QTableWritePartial,                 ST_X_constMVariantA_constMByteStringA_int)
   M_OBJECT_SERVICE           (Protocol, QFunctionExecute,                   ST_X_constMVariantA)
   M_OBJECT_SERVICE           (Protocol, QFunctionExecuteRequest,            ST_X_constMVariantA_constMByteStringA)
   M_OBJECT_SERVICE_OVERLOADED(Protocol, QFunctionExecuteResponse,        QFunctionExecuteResponse,          3, ST_X_constMVariantA_int_unsigned)
   M_OBJECT_SERVICE_OVERLOADED(Protocol, QFunctionExecuteResponse,        DoQFunctionExecuteResponse,        2, ST_X_constMVariantA_int) // SWIG_HIDE
   M_OBJECT_SERVICE_OVERLOADED(Protocol, QFunctionExecuteRequestResponse, QFunctionExecuteRequestResponse,   4, ST_X_constMVariantA_constMByteStringA_int_unsigned)
   M_OBJECT_SERVICE_OVERLOADED(Protocol, QFunctionExecuteRequestResponse, DoQFunctionExecuteRequestResponse, 3, ST_X_constMVariantA_constMByteStringA_int) // SWIG_HIDE
   M_OBJECT_SERVICE           (Protocol, QGetTableData,                      ST_MByteString_X_constMVariantA_int)
   M_OBJECT_SERVICE           (Protocol, QGetFunctionData,                   ST_MByteString_X_constMVariantA_int)
#if !M_NO_MCOM_IDENTIFY_METER
   M_OBJECT_SERVICE           (Protocol, QGetIdentifyMeterData,              ST_MStdString_X)
#endif
   M_OBJECT_SERVICE_OVERLOADED(Protocol, QCommit, QCommit,                1, ST_X_bool)
   M_OBJECT_SERVICE_OVERLOADED(Protocol, QCommit, DoQCommit0,             0, ST_X) // SWIG_HIDE
   M_OBJECT_SERVICE           (Protocol, QWriteToMonitor,                    ST_X_constMStdStringA)
   M_OBJECT_SERVICE           (Protocol, QAbort,                             ST_X)
#endif
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   M_OBJECT_SERVICE           (Protocol, ClearPasswordList,                  ST_X)
   M_OBJECT_SERVICE           (Protocol, AddToPasswordList,                  ST_X_constMByteStringA)
#endif
M_END_CLASS_TYPED(Protocol, COMObject, "PROTOCOL")

   using namespace std;

MProtocol::MProtocol(MChannel* channel, bool channelIsOwned)
:
   m_password(),
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   m_passwordList(),
   m_passwordListSuccessfulEntry(-1),
#endif
#if !M_NO_MCOM_COMMAND_QUEUE
   m_queue(),
   m_commitDone(false),
#endif
   m_preferredPasswordIsHex(false),
   m_maximumPasswordLength(4), // this property is going to be overwritten by many children
   m_channel(channel),
#if !M_NO_MCOM_PROTOCOL_THREAD
   m_protocolThread(NULL),
   m_backgroundCommunicationIsProgressing(false),
#endif
   m_meterIsLittleEndian(true),
   m_isChannelOwned(channelIsOwned),
   m_isInSession(false),
   m_isFinalized(false),
   m_autoUpdateRoundTripTimes(true),
   m_serviceWrappers(),
#if !M_NO_MCOM_KEEP_SESSION_ALIVE
   m_sessionKeeper(this),
#endif
   m_savedTotalAppLayerServices(0),
   m_countApplicationLayerServicesSuccessful(0),
   m_countApplicationLayerServicesRetried(0),
   m_countApplicationLayerServicesFailed(0),
   m_countLinkLayerPacketsSuccessful(0),
   m_countLinkLayerPacketsRetried(0),
   m_countLinkLayerPacketsFailed(0),
   m_maximumRoundTripTime(0),
   m_minimumRoundTripTime(0),
   m_sumRoundTripTime(0.0),
   m_roundTripCounter(0.0)
#if !M_NO_PROGRESS_MONITOR
   , m_progressMonitor(NULL)
#endif
{
   M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(Protocol);
}

MProtocol::~MProtocol()
{
   Finalize();

   MAes::DestroySecureData(m_password);
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   MAes::DestroySecureData(m_passwordList);
#endif
}

#if !M_NO_MCOM_FACTORY // Note that factory is not available without reflection
MProtocol* MProtocol::CreateClone() const
{
   MUniquePtr<MProtocol> proto(MCOMFactory::CreateProtocol((MChannel*)NULL, GetPersistentPropertyValues(true)));
   if ( m_channel != NULL )
   {
      proto->m_isChannelOwned = m_isChannelOwned;
      proto->m_channel = m_isChannelOwned ? m_channel->CreateClone() : m_channel;
   }
   return proto.release();
}
#endif

void MProtocol::Finalize() M_NO_THROW
{
   if ( !m_isFinalized )
   {
      m_isFinalized = true;
      try // ensure the destructor is always silent
      {
         #if !M_NO_MCOM_KEEP_SESSION_ALIVE
            m_sessionKeeper.InitStopKeeping();
         #endif
         #if !M_NO_MCOM_PROTOCOL_THREAD
            if ( QIsBackgroundCommunicationProgressing() )
               QAbort();
         #endif
         #if !M_NO_MCOM_KEEP_SESSION_ALIVE
            m_sessionKeeper.FinalizeStopKeeping();
         #endif
         #if !M_NO_MCOM_PROTOCOL_THREAD
            if ( m_protocolThread != NULL )
            {
               #if M_OS & M_OS_UCLINUX
                  m_protocolThread->WaitUntilFinished(false);
               #else
                  m_protocolThread->WaitUntilFinished(false, 10000u); // wait for 10 seconds for communication to finish
               #endif
               delete m_protocolThread;
            }
         #endif

         if ( m_isChannelOwned )
            delete m_channel;
      }
      catch ( MException& ex )
      {
         M_USED_VARIABLE(ex); // debug convenience
         M_ASSERT(0); // do not use ensured assert
      }
      M_ASSERT(m_serviceWrappers.empty()); // make sure all services are done at the time of destruction
   }
}


void MProtocol::SetChannel(MChannel* chan)
{
   if ( chan != m_channel ) // matters only when the channel is owned
   {
      if ( m_isChannelOwned )
         delete m_channel; // delete previously owned channel
      m_channel = chan;
   }
}

MByteString MProtocol::GetPassword() const
{
   return m_password;
}

void MProtocol::SetPassword(const MByteString& password)
{
   if ( password.size() > (size_t)m_maximumPasswordLength )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR_P1(M_ERR_PASSWORD_SHOULD_BE_NO_MORE_THAN_D1_BYTES_LONG, M_I("Password should be no more than %d bytes long"), (int)m_maximumPasswordLength));
      M_ENSURED_ASSERT(0);
   }
   MAes::AssignSecureData(m_password, password);
}

#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST

void MProtocol::ClearPasswordList()
{
   m_passwordListSuccessfulEntry = -1;
   m_passwordList.clear();
}

   static void DoVerifyPassword(MProtocol* protocol, const MByteString& password)
   {
      MByteString storedPassword = protocol->GetPassword();
      try
      {
         protocol->SetPassword(password);
      }
      catch ( ... )
      {
         protocol->SetPassword(storedPassword);
         throw;
      }
      protocol->SetPassword(storedPassword);
   }

void MProtocol::AddToPasswordList(const MByteString& password)
{
   m_passwordListSuccessfulEntry = -1;
   DoVerifyPassword(this, password);
   m_passwordList.push_back(password); // no need to assign securely as the item is new
}

void MProtocol::SetPasswordList(const MByteStringVector& passwordList)
{
   m_passwordListSuccessfulEntry = -1;

   MByteStringVector::const_iterator it = passwordList.begin();
   MByteStringVector::const_iterator itEnd = passwordList.end();
   for ( ; it != itEnd; ++it )
      DoVerifyPassword(this, *it); // verify successful conversion into an ANSI string
   MAes::AssignSecureData(m_passwordList, passwordList);
}

int MProtocol::GetPasswordListSuccessfulEntry() const
{
   return m_passwordListSuccessfulEntry;
}

#endif // !M_NO_MCOM_PASSWORD_AND_KEY_LIST

bool MProtocol::GetKeepSessionAlive() const
{
   #if !M_NO_MCOM_KEEP_SESSION_ALIVE
      return m_sessionKeeper.GetKeepSessionAlive();
   #else
      return false;
   #endif
}

void MProtocol::SetKeepSessionAlive(bool alive)
{
   #if !M_NO_MCOM_KEEP_SESSION_ALIVE
      m_sessionKeeper.SetKeepSessionAlive(alive);
   #else
      M_ASSERT(!alive);
   #endif
}

void MProtocol::Connect()
{
   DoCheckChannel();
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   m_passwordListSuccessfulEntry = -1;
#endif
   DoConnect();
}

void MProtocol::DoConnect()
{
   ApplyChannelParameters();
   m_isInSession = false;
   m_channel->Connect();
}

void MProtocol::Disconnect()
{
   DoCheckChannel();
   m_isInSession = false;
#if !M_NO_MCOM_KEEP_SESSION_ALIVE
   m_sessionKeeper.CancelService();
#endif
   unsigned totalAppLayerServices = m_countApplicationLayerServicesFailed + m_countApplicationLayerServicesSuccessful;
   if ( totalAppLayerServices != m_savedTotalAppLayerServices ) // this way, we avoid repeated sends of then-useless statistics
   {
      m_savedTotalAppLayerServices = totalAppLayerServices;
      WriteCountsToMonitor();
   }
   m_channel->Disconnect();
}

bool MProtocol::IsConnected() const
{
   DoCheckChannel(true);
   bool yes = m_channel->IsConnected();
   return yes;
}

bool MProtocol::IsInSession() const
{
   DoCheckChannel(true);
   if ( !m_isInSession )
      return false;
   return IsConnected();
}

void MProtocol::ApplyChannelParameters()
{
   DoCheckChannel();
   // do nothing else currently
}

void MProtocol::ResetCounts()
{
   if ( m_channel != NULL )
   {
      DoCheckChannel(); // check channel is not communicating on the background
      m_channel->ResetCounts();
   }
   m_savedTotalAppLayerServices = 0u;
   m_countApplicationLayerServicesSuccessful = 0u;
   m_countApplicationLayerServicesRetried = 0u;
   m_countApplicationLayerServicesFailed = 0u;
   m_countLinkLayerPacketsSuccessful = 0u;
   m_countLinkLayerPacketsRetried = 0u;
   m_countLinkLayerPacketsFailed = 0u;
   m_maximumRoundTripTime = 0u;
   m_minimumRoundTripTime = 0u;
   m_sumRoundTripTime = 0.0;
   m_roundTripCounter = 0.0;
}

#if !M_NO_VERBOSE_ERROR_INFORMATION
void MProtocol::DoBuildComplexServiceName(MChars fullServiceName, MConstChars serviceName, MCOMNumberConstRef number, int par1, int par2) M_NO_THROW
{
   MStdString numberString;
   try
   {
#if !M_NO_VARIANT
      numberString = number.AsEscapedString();
      size_t len = numberString.size();
      if ( len > MAXIMUM_NUMBER_STRING_SIZE - 1 )
         numberString[len - 1] = '\0'; // this is faster than truncation, and does what we need.
#else
      numberString = MToStdString(number);
#endif
   }
   catch ( ... )
   {
   }

   size_t fullServiceNameSize = ( par1 == -1 && par2 == -1 )
         ? MFormat(fullServiceName, MAXIMUM_SERVICE_NAME_STRING_SIZE, "%s(%s)", serviceName, numberString.c_str())
         : MFormat(fullServiceName, MAXIMUM_SERVICE_NAME_STRING_SIZE, "%s(%s, %d, %d)", serviceName, numberString.c_str(), par1, par2);
   M_ASSERT(fullServiceNameSize > 0 && fullServiceNameSize < MAXIMUM_SERVICE_NAME_STRING_SIZE); // Check if MAXIMUM_SERVICE_NAME_STRING_SIZE is big enough
   M_USED_VARIABLE(fullServiceNameSize);
}

void MProtocol::DoBuildPossiblyNumericComplexServiceName(MChars fullServiceName, MConstChars serviceName, MCOMNumberConstRef number, bool isHex, int par1, int par2) M_NO_THROW
{
   if ( number.IsNumeric() )
   {
      try
      {
         unsigned num = number.AsDWord(); // avoid signed/unsigned differences, have it always unsigned
         size_t fullServiceNameSize;
         if ( par1 == -1 && par2 == -1 )
         {
             if ( isHex )
                fullServiceNameSize = MFormat(fullServiceName, MAXIMUM_SERVICE_NAME_STRING_SIZE, "%s(0x%X)", serviceName, num);
             else
                fullServiceNameSize = MFormat(fullServiceName, MAXIMUM_SERVICE_NAME_STRING_SIZE, "%s(%d)", serviceName, num);
         }
         else
         {
             if ( isHex )
                fullServiceNameSize = MFormat(fullServiceName, MAXIMUM_SERVICE_NAME_STRING_SIZE, "%s(0x%X, %d, %d)", serviceName, num, par1, par2);
             else
                fullServiceNameSize = MFormat(fullServiceName, MAXIMUM_SERVICE_NAME_STRING_SIZE, "%s(%d, %d, %d)", serviceName, num, par1, par2);
         }
         M_ASSERT(fullServiceNameSize > 0 && fullServiceNameSize < MAXIMUM_SERVICE_NAME_STRING_SIZE); // Check if MAXIMUM_SERVICE_NAME_STRING_SIZE is big enough
         M_USED_VARIABLE(fullServiceNameSize);
         return; // success
      }
      catch ( ... )
      {
         // fall into default implementation
      }
   }
   MProtocol::DoBuildComplexServiceName(fullServiceName, serviceName, number, par1, par2);
}
#endif // !M_NO_VERBOSE_ERROR_INFORMATION

#if !M_NO_MCOM_COMMAND_QUEUE
#if !M_NO_MCOM_PROTOCOL_THREAD

bool MProtocol::QNeedToCommit() const
{
   if ( m_backgroundCommunicationIsProgressing && m_protocolThread != NULL )
      return !m_protocolThread->IsRunning();
   return false;
}

bool MProtocol::QIsDone()
{
   // Luxury of the approach: there is no need to "critical section" even this function

   if ( QNeedToCommit() )
   {
      QCommit(false); // synchronize, possibly throw an exception...
      M_ASSERT(!m_backgroundCommunicationIsProgressing || (m_protocolThread == NULL || !m_protocolThread->IsRunning()));
      return true;
   }
   return !m_backgroundCommunicationIsProgressing;
}

#endif // !M_NO_MCOM_PROTOCOL_THREAD

void MProtocol::QAbort()
{
   // Luxury of the approach: there is no need to "critical section" even this function!

   m_commitDone = true;

   #if !M_NO_MCOM_PROTOCOL_THREAD
      if ( m_backgroundCommunicationIsProgressing )
         m_channel->CancelCommunication(false); // do not call disconnect, it can lock!
   #endif // !M_NO_MCOM_PROTOCOL_THREAD
}

   class PendingQAbort
   {
   public:
      explicit PendingQAbort(MProtocol* p) : m_p(p) {}
      ~PendingQAbort() { m_p->QAbort(); }
   private:
      MProtocol* m_p;
   };

void MProtocol::DoQCommit()
{
   size_t size = m_queue.size();
   if ( size == 0 )
      return; // nothing to be done

#if !M_NO_PROGRESS_MONITOR
   double totalProgress = 0.0;
   for ( size_t i = 0; i < size; ++i )
      totalProgress += m_queue[i]->GetProgressWeight();

   double progressDivisor = totalProgress / 100.0;
   double progressAccumulator = 0.0;

   MProgressAction* action = GetLocalProgressAction();
#endif

   for ( size_t i = 0; i < size; ++i )
   {
      MCommunicationCommand* cmd = m_queue[i];

#if !M_NO_PROGRESS_MONITOR
      double progressWeigth = cmd->GetProgressWeight();
      progressAccumulator += progressWeigth;
      double localActionWeight = progressAccumulator / progressDivisor;
      if ( localActionWeight > 100.0 ) // this can only be the cause of rounding error
      {
         M_ASSERT(localActionWeight < 101.0);
         localActionWeight = 100.0;
      }
      action->CreateLocalAction(localActionWeight);
#endif
      m_meterIsLittleEndian = cmd->GetLittleEndian();
      switch ( cmd->m_type )
      {
      case MCommunicationCommand::CommandWriteToMonitor:
         WriteToMonitor(cmd->GetRequest());
         break;
      case MCommunicationCommand::CommandConnect:
         Connect();
         break;
      case MCommunicationCommand::CommandDisconnect:
         Disconnect();
         break;
      case MCommunicationCommand::CommandStartSession:
         StartSession();
         break;
      case MCommunicationCommand::CommandEndSession:
         EndSession();
         break;
      case MCommunicationCommand::CommandEndSessionNoThrow:
         EndSessionNoThrow();
         break;
#if !M_NO_MCOM_IDENTIFY_METER
      case MCommunicationCommand::CommandIdentifyMeter:
         cmd->SetResponse(IdentifyMeter(true));
         break;
#endif
      case MCommunicationCommand::CommandRead:
         cmd->SetResponse(TableRead(cmd->GetNumber(), cmd->GetLength()));
         break;
      case MCommunicationCommand::CommandWrite:
         TableWrite(cmd->GetNumber(), cmd->GetRequest());
         break;
      case MCommunicationCommand::CommandReadPartial:
         cmd->SetResponse(TableReadPartial(cmd->GetNumber(), cmd->GetOffset(), cmd->GetLength()));
         break;
      case MCommunicationCommand::CommandWritePartial:
         TableWritePartial(cmd->GetNumber(), cmd->GetRequest(), cmd->GetOffset());
         break;
      case MCommunicationCommand::CommandExecute:
         FunctionExecute(cmd->GetNumber());
         break;
      case MCommunicationCommand::CommandExecuteRequest:
         FunctionExecuteRequest(cmd->GetNumber(), cmd->GetRequest());
         break;
      case MCommunicationCommand::CommandExecuteResponse:
         cmd->SetResponse(FunctionExecuteResponse(cmd->GetNumber()));
         break;
      case MCommunicationCommand::CommandExecuteRequestResponse:
         cmd->SetResponse(FunctionExecuteRequestResponse(cmd->GetNumber(), cmd->GetRequest()));
         break;
      default:
         M_ASSERT(0); // warn on debug, ignore on release -- possibility of a new command
      }

#if !M_NO_PROGRESS_MONITOR
      action->SetProgress(localActionWeight);
#endif

   }

#if !M_NO_PROGRESS_MONITOR
   action->Complete();
#endif
}

void MProtocol::QCommit(bool asynchronously)
{
   if ( m_commitDone ) // if this is a second call clear queue and return calling commit for the second time in a row
   {
      m_queue.clear();
#if !M_NO_MCOM_PROTOCOL_THREAD
      if ( m_backgroundCommunicationIsProgressing ) // complete the asynchronous communication
      {
         MValueEndScopeSetter<bool> setter(&m_backgroundCommunicationIsProgressing, false);
         M_ASSERT(m_protocolThread != NULL);
         m_protocolThread->WaitUntilFinished();
      }
#endif
      return;
   }

#if !M_NO_MCOM_PROTOCOL_THREAD
   if ( asynchronously )
   {
      DoCheckChannel();
      if ( m_protocolThread == NULL )
         m_protocolThread = M_NEW MProtocolThread(this);
      m_protocolThread->Start();
      m_backgroundCommunicationIsProgressing = true;
   }
   else
   {
      MValueEndScopeSetter<bool> setter(&m_commitDone, true);
      if ( m_backgroundCommunicationIsProgressing ) // complete the asynchronous communication
      {
         MValueEndScopeSetter<bool> setter(&m_backgroundCommunicationIsProgressing, false);
         M_ASSERT(m_protocolThread != NULL);
         m_protocolThread->WaitUntilFinished();
      }
      else
      {
         PendingQAbort call(this);
         DoQCommit();
      }
   }
#else
   M_ASSERT(!asynchronously);
   MValueEndScopeSetter<bool> setter(&m_commitDone, true);
   PendingQAbort call(this);
   DoQCommit();
#endif
}

void MProtocol::DoAddCommandToQueue(MCommunicationCommand* command)
{
   try
   {
      DoCheckChannel();
   }
   catch ( ... )
   {
      delete command;
      throw;
   }

   if ( m_commitDone )
   {
      m_commitDone = false;
      m_queue.clear();
   }
   command->SetLittleEndian(m_meterIsLittleEndian); // store current value for later use
   m_queue.push_back(command);
}

void MProtocol::QWriteToMonitor(const MStdString& message)
{
   MCommunicationCommand* command = MCommunicationCommand::New(MCommunicationCommand::CommandWriteToMonitor);
   command->SetRequest(message);
   DoAddCommandToQueue(command);
}

void MProtocol::QConnect()
{
   DoAddCommandToQueue(MCommunicationCommand::New(MCommunicationCommand::CommandConnect));
}

void MProtocol::QDisconnect()
{
   DoAddCommandToQueue(MCommunicationCommand::New(MCommunicationCommand::CommandDisconnect));
}

#if !M_NO_MCOM_IDENTIFY_METER
void MProtocol::QIdentifyMeter()
{
   DoAddCommandToQueue(MCommunicationCommand::New(MCommunicationCommand::CommandIdentifyMeter));
}
#endif

void MProtocol::QStartSession()
{
   DoAddCommandToQueue(MCommunicationCommand::New(MCommunicationCommand::CommandStartSession));
}

void MProtocol::QEndSession()
{
   DoAddCommandToQueue(MCommunicationCommand::New(MCommunicationCommand::CommandEndSession));
}

void MProtocol::QEndSessionNoThrow()
{
   DoAddCommandToQueue(MCommunicationCommand::New(MCommunicationCommand::CommandEndSessionNoThrow));
}

   static void DoCheckTableOffsetRange(int offset)
   {
      MENumberOutOfRange::CheckNamedIntegerRange(0, MProtocol::MAXIMUM_POSSIBLE_TABLE_OFFSET, offset, "offset");
   }

   static void DoCheckTableLengthRange(int length)
   {
      MENumberOutOfRange::CheckNamedIntegerRange(0, MProtocol::MAXIMUM_POSSIBLE_TABLE_LENGTH, length, "length"); // we do allow zero length now, testing purpose
   }

void MProtocol::QTableRead(MCOMNumberConstRef number, unsigned expectedSize, int id)
{
   DoCheckTableLengthRange(expectedSize);
   MCommunicationCommand* command = MCommunicationCommand::New(MCommunicationCommand::CommandRead);
   command->SetNumber(number);
   command->SetDataId(id);
   command->SetLength(expectedSize);
   DoAddCommandToQueue(command);
}

void MProtocol::QTableWrite(MCOMNumberConstRef number, const MByteString& data)
{
   MCommunicationCommand* command = MCommunicationCommand::New(MCommunicationCommand::CommandWrite);
   command->SetNumber(number);
   command->SetRequest(data);
   DoAddCommandToQueue(command);
}

void MProtocol::QTableReadPartial(MCOMNumberConstRef number, int offset, int size, int id)
{
   DoCheckTableOffsetRange(offset);
   DoCheckTableLengthRange(size);
   MCommunicationCommand* command = MCommunicationCommand::New(MCommunicationCommand::CommandReadPartial);
   command->SetNumber(number);
   command->SetOffset(offset);
   command->SetLength(size);
   command->SetDataId(id);
   DoAddCommandToQueue(command);
}

void MProtocol::QTableWritePartial(MCOMNumberConstRef number, const MByteString& data, int offset)
{
   DoCheckTableOffsetRange(offset);
   MCommunicationCommand* command = MCommunicationCommand::New(MCommunicationCommand::CommandWritePartial);
   command->SetNumber(number);
   command->SetRequest(data);
   command->SetOffset(offset);
   DoAddCommandToQueue(command);
}

void MProtocol::QFunctionExecute(MCOMNumberConstRef number)
{
   MCommunicationCommand* command = MCommunicationCommand::New(MCommunicationCommand::CommandExecute);
   command->SetNumber(number);
   DoAddCommandToQueue(command);
}

void MProtocol::QFunctionExecuteRequest(MCOMNumberConstRef number, const MByteString& request)
{
   MCommunicationCommand* command = MCommunicationCommand::New(MCommunicationCommand::CommandExecuteRequest);
   command->SetNumber(number);
   command->SetRequest(request);
   DoAddCommandToQueue(command);
}

#if !M_NO_REFLECTION

void MProtocol::DoQFunctionExecuteResponse(MCOMNumberConstRef number, int id)
{
   QFunctionExecuteResponse(number, id);
}

void MProtocol::DoQFunctionExecuteRequestResponse(MCOMNumberConstRef number, const MByteString& request, int id)
{
   QFunctionExecuteRequestResponse(number, request, id);
}

#endif

void MProtocol::QFunctionExecuteResponse(MCOMNumberConstRef number, int id, unsigned estimatedResponseLength)
{
   MCommunicationCommand* command = MCommunicationCommand::New(MCommunicationCommand::CommandExecuteResponse);
   command->SetNumber(number);
   command->SetDataId(id);
   command->SetLength(estimatedResponseLength);
   DoAddCommandToQueue(command);
}

void MProtocol::QFunctionExecuteRequestResponse(MCOMNumberConstRef number, const MByteString& request, int id, unsigned estimatedResponseLength)
{
   MCommunicationCommand* command = MCommunicationCommand::New(MCommunicationCommand::CommandExecuteRequestResponse);
   command->SetNumber(number);
   command->SetDataId(id);
   command->SetRequest(request);
   command->SetLength(estimatedResponseLength);
   DoAddCommandToQueue(command);
}


MByteString MProtocol::QGetTableData(MCOMNumberConstRef number, int id)
{
   return m_queue.GetResponseCommand(MCommunicationCommand::CommandRead, number, id)->GetResponse();
}

MByteString MProtocol::QGetFunctionData(MCOMNumberConstRef number, int id)
{
   return m_queue.GetResponseCommand(MCommunicationCommand::CommandExecuteResponse, number, id)->GetResponse();
}


#if !M_NO_MCOM_IDENTIFY_METER
MStdString MProtocol::QGetIdentifyMeterData()
{
   return m_queue.GetResponseCommand(MCommunicationCommand::CommandIdentifyMeter, MVariant(), -1)->GetResponse();
}
#endif

#endif // !M_NO_MCOM_COMMAND_QUEUE


unsigned MProtocol::CalculateChecksum(const MByteString& buffer) const
{
   return CalculateChecksumFromBuffer(buffer.data(), M_64_CAST(unsigned, buffer.size()));
}

unsigned MProtocol::CalculateChecksumFromBuffer(const char* buff, unsigned size) const
{
   return MProtocol::StaticCalculateChecksumFromBuffer(buff, size);
}

unsigned MProtocol::StaticCalculateChecksumFromBuffer(const char* buff, unsigned size)
{
   const unsigned char* buffer = (const unsigned char*)buff;
   unsigned char cks = 0;
   for ( unsigned i = 0; i < size; ++i )
      cks += (unsigned char)*buffer++;
   return (unsigned)(unsigned char)(~cks);
}

unsigned MProtocol::StaticCalculateChecksum(const MByteString& buffer)
{
   return StaticCalculateChecksumFromBuffer(buffer.data(), M_64_CAST(unsigned, buffer.size()));
}

unsigned MProtocol::GetNumberOfDataLinkPackets(MCommunicationCommand::CommandType, unsigned) M_NO_THROW
{
   return 1; // default implementation that fits many protocols
}

void MProtocol::DoSetBaudIfOpticalProbe(unsigned baud, unsigned numberOfCharsInBuffer)
{
#if !M_NO_SERIAL_PORT
   MChannelOpticalProbe* opticalProbe = M_DYNAMIC_CAST_WITH_NULL_CHECK(MChannelOpticalProbe, m_channel);
   if ( opticalProbe != NULL )
   {
      if ( opticalProbe->IsConnected() )
      {
         opticalProbe->FlushOutputBuffer(numberOfCharsInBuffer);
         opticalProbe->SetBaud(baud);
      }
      else
         opticalProbe->SetBaud(baud);
   }
#endif
}

void MProtocol::DoSetParametersIfOpticalProbe(unsigned baud, int databits, char parity, int stopBits, unsigned numberOfCharsInBuffer)
{
#if !M_NO_SERIAL_PORT
   MChannelOpticalProbe* opticalProbe = M_DYNAMIC_CAST_WITH_NULL_CHECK(MChannelOpticalProbe, m_channel);
   if ( opticalProbe != NULL )
   {
      if ( opticalProbe->IsConnected() )
      {
         opticalProbe->FlushOutputBuffer(numberOfCharsInBuffer);
         opticalProbe->SetParameters(baud, databits, parity, stopBits);
      }
      else
         opticalProbe->SetParameters(baud, databits, parity, stopBits);
   }
#endif
}

#if !M_NO_SERIAL_PORT
   #if !M_NO_MCOM_CHANNEL_MODEM
      inline bool IsNotModem(const MChannelSerialPort* serialPort)
      {
         return M_DYNAMIC_CAST(const MChannelModem, serialPort) == NULL;
      }
   #else
      inline bool IsNotModem(const MChannelSerialPort* serialPort)
      {
         return true;
      }
   #endif
#endif

void MProtocol::DoSetBaudIfOpticalProbeOrDirect(unsigned baud, unsigned numberOfCharsInBuffer)
{
#if !M_NO_SERIAL_PORT
   MChannelSerialPort* serialPort = M_DYNAMIC_CAST_WITH_NULL_CHECK(MChannelSerialPort, m_channel);
   if ( serialPort != NULL && IsNotModem(serialPort) ) // serial port but not modem -- that's the condition
   {
      serialPort->FlushOutputBuffer(numberOfCharsInBuffer);
      serialPort->SetBaud(baud);
   }
#endif
}

void MProtocol::DoSetParametersIfOpticalProbeOrDirect(unsigned baud, int databits, char parity, int stopBits, unsigned numberOfCharsInBuffer)
{
#if !M_NO_SERIAL_PORT
   MChannelSerialPort* serialPort = M_DYNAMIC_CAST_WITH_NULL_CHECK(MChannelSerialPort, m_channel);
   if ( serialPort != NULL && IsNotModem(serialPort) ) // serial port but not modem -- that's the condition
   {
      if ( serialPort->IsConnected() )
         serialPort->FlushOutputBuffer(numberOfCharsInBuffer);
      serialPort->SetParameters(baud, databits, parity, stopBits);
   }
#endif
}

void MProtocol::WriteToMonitor(const MStdString& message)
{
   #if !M_NO_MCOM_MONITOR
      if ( m_channel != NULL )
         m_channel->WriteToMonitor(message);
   #endif
}

void MProtocol::WriteCountsToMonitor()
{
   #if !M_NO_MCOM_MONITOR
      if ( m_channel != NULL )
      {
         m_channel->WriteToMonitor(MGetStdString("Bytes sent/received: %d/%d", m_channel->GetCountBytesSent(), m_channel->GetCountBytesReceived()));
         m_channel->WriteToMonitor(MGetStdString("Application Layer successes/retries/failures: %d/%d/%d", m_countApplicationLayerServicesSuccessful, m_countApplicationLayerServicesRetried, m_countApplicationLayerServicesFailed));
         if ( m_countLinkLayerPacketsSuccessful != 0 || m_countLinkLayerPacketsFailed != 0 ) // link layer could be absent in protocol
            m_channel->WriteToMonitor(MGetStdString("Link Layer successes/retries/failures: %d/%d/%d", m_countLinkLayerPacketsSuccessful, m_countLinkLayerPacketsRetried, m_countLinkLayerPacketsFailed));
         m_channel->WriteToMonitor(MGetStdString("Round trip milliseconds maximum/average/minimum: %d/%d/%d", m_maximumRoundTripTime, GetAverageRoundTripTime(), m_minimumRoundTripTime));
      }
   #endif
}

void MProtocol::StartSession()
{
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   m_passwordListSuccessfulEntry = -1;
#endif
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("StartSession"), MProtocolServiceWrapper::ServiceStartsSessionKeeping);
   try
   {
      #if !M_NO_MCOM_MONITOR && !M_NO_REFLECTION
         WritePropertiesToMonitor();
      #endif
      DoStartSession();
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

void MProtocol::EndSession()
{
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("EndSession"), MProtocolServiceWrapper::ServiceEndsSessionKeeping);
   try
   {
      DoEndSession();
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

void MProtocol::EndSessionNoThrow() M_NO_THROW
{
   try
   {
      EndSession();
   }
   catch ( ... )
   {
   }
}

MByteString MProtocol::TableRead(MCOMNumberConstRef number, unsigned expectedSize)
{
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("TableRead"), number, -1, -1);
   MByteString data;
   try
   {
      DoTableRead(number, data, expectedSize);
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
   return data;
}

MByteString MProtocol::TableReadNoThrow(MCOMNumberConstRef number, MException** ppException, unsigned expectedSize)
{
   M_ASSERT(ppException != NULL);
   *ppException = NULL;
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("TableReadNoThrow"), number, -1, -1);
   MByteString data;
   try
   {
      DoTableRead(number, data, expectedSize);
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureNoThrow(ex);
      *ppException = ex.NewClone();
      return data;
   }
   return data;
}

#if !M_NO_REFLECTION

MByteString MProtocol::DoTableReadImpl(MCOMNumberConstRef number)
{
   return TableRead(number);
}

#endif // !M_NO_REFLECTION

#if !M_NO_MCOM_IDENTIFY_METER
MStdString MProtocol::DoIdentifyMeter0()
{
   return IdentifyMeter(false);
}

MStdString MProtocol::IdentifyMeter(bool sessionIsStarted)
{
   return IdentifyMeterWithContext(sessionIsStarted, 0);
}
#endif

   static void DoCheckReceivedDataSize(size_t receivedLength, unsigned expectedLength)
   {
      if ( receivedLength != expectedLength )
      {
         MCOMException::Throw(MException::ErrorMeter, M_CODE_STR_P2(MErrorEnum::ReceivedDataSizeDifferent, M_I("Received data size %u is different than requested %u bytes"), static_cast<unsigned>(receivedLength), expectedLength));
         M_ENSURED_ASSERT(0);
      }
   }

void MProtocol::TableReadBuffer(MCOMNumberConstRef number, void* buff, unsigned size)
{
   MByteString t = TableRead(number, size);
   DoCheckReceivedDataSize(t.size(), size);
   memcpy(buff, t.data(), size);
}

void MProtocol::TableWrite(MCOMNumberConstRef number, const MByteString& data)
{
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("TableWrite"), number, -1, -1);
   try
   {
      MChannel::UninterruptibleCommunication protect(m_channel);
      DoTableWrite(number, data);
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

void MProtocol::TableWriteBuffer(MCOMNumberConstRef number, const void* buff, unsigned size)
{
   M_ASSERT(buff != NULL);
   TableWrite(number, MByteString((const char*)buff, size));
}

MByteString MProtocol::TableReadPartial(MCOMNumberConstRef number, int offset, int size)
{
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("TableReadPartial"), number, offset, size);
   MByteString data;
   try
   {
      DoCheckTableOffsetRange(offset);
      DoCheckTableLengthRange(size);
      DoTableReadPartial(number, data, offset, size);
      DoCheckReceivedDataSize(data.size(), size);
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
   return data;
}

void MProtocol::TableReadPartialBuffer(MCOMNumberConstRef number, int offset, void* buff, unsigned size)
{
   MByteString t = TableReadPartial(number, offset, size);
   M_ASSERT(t.size() == size); // guaranteed by the above call
   memcpy(buff, t.data(), size);
}

void MProtocol::TableWritePartial(MCOMNumberConstRef number, const MByteString& data, int offset)
{
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("TableWritePartial"), number, offset, M_64_CAST(int, data.size()));
   try
   {
      MChannel::UninterruptibleCommunication protect(m_channel);
      DoCheckTableOffsetRange(offset);
      DoTableWritePartial(number, data, offset);
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

void MProtocol::TableWritePartialBuffer(MCOMNumberConstRef number, int offset, const void* buff, unsigned size)
{
   M_ASSERT(buff != NULL);
   TableWritePartial(number, MByteString((const char*)buff, size), offset);
}

void MProtocol::FunctionExecute(MCOMNumberConstRef number)
{
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("FunctionExecute"), number, -1, -1);
   try
   {
      DoFunctionExecute(number);
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

void MProtocol::FunctionExecuteRequest(MCOMNumberConstRef number, const MByteString& request)
{
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("FunctionExecuteRequest"), number, -1, -1);
   try
   {
      DoFunctionExecuteRequest(number, request);
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

MByteString MProtocol::FunctionExecuteResponse(MCOMNumberConstRef number)
{
   MByteString response;
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("FunctionExecuteResponse"), number, -1, -1);
   try
   {
      DoFunctionExecuteResponse(number, response);
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
   return response;
}

MByteString MProtocol::FunctionExecuteRequestResponse(MCOMNumberConstRef number, const MByteString& request)
{
   MByteString response;
   MProtocolServiceWrapper wrapper(this, M_OPT_STR("FunctionExecuteRequestResponse"), number, -1, -1);
   try
   {
      DoFunctionExecuteRequestResponse(number, request, response);
   }
   catch ( MException& ex )
   {
      wrapper.HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
   return response;
}


#if !M_NO_MCOM_IDENTIFY_METER
MStdString MProtocol::IdentifyMeterWithContext(bool sessionIsStarted, TableRawDataVector* tablesRead)
{
   return DoIdentifyMeter(sessionIsStarted, tablesRead);
}
#endif // !M_NO_MCOM_IDENTIFY_METER

void MProtocol::DoTryPasswordOrPasswordList()
{
#if !M_NO_MCOM_PASSWORD_AND_KEY_LIST
   m_passwordListSuccessfulEntry = -1;
   if ( m_passwordList.empty() )
      DoTryPasswordEntry(m_password); // use PASSWORD property directly
   else
   {
      size_t num = m_passwordList.size();
      for ( size_t i = 0; i < num; ++i )
      {
         try
         {
            DoTryPasswordEntry(m_passwordList[i]); // use entry in the password list
            m_passwordListSuccessfulEntry = (int)i;
            return; // success
         }
         catch ( MException& ex )
         {
            MProtocolServiceWrapper::StaticNotifyOrThrowRetry(this, ex, i == num - 1 ? 0 : 1); // set retry count to nonzero, so we do not throw
         }
      }
      M_ENSURED_ASSERT(0); // we are never here due to the condition under catch()
   }
#else
   DoTryPasswordEntry(m_password); // use PASSWORD property directly
#endif
}

Muint8 MProtocol::ReadStartByte(const MByteString& validStartBytes, unsigned trafficTimeout)
{
    return static_cast<Muint8>(DoReadStartCharacter(validStartBytes.c_str(), trafficTimeout, UINT_MAX));
}

char MProtocol::DoReadStartCharacter(const char* validStartCharacters, unsigned trafficTimeout, unsigned lastTurnAroundSize)
{
   M_ASSERT(validStartCharacters != NULL && validStartCharacters[0] != '\0');

   unsigned startTick = MUtilities::GetTickCount();
   if ( (int)trafficTimeout < 0 )
      trafficTimeout = INT_MAX; // the below code works with signed integers
   unsigned timeoutEnd = startTick + trafficTimeout; // moment at which the traffic timeout takes place

   unsigned timeout = trafficTimeout; // save trafficTimeout unchanged for debugging convenience
   char ch;
   int cnt;
   for ( cnt = 0; ; ++cnt ) // try to synchronize with the packet header
   {
      unsigned len = m_channel->DoReadCancellable(&ch, 1, timeout, true);
      if ( len == 0 )
         break;
      unsigned i = 0u;
      for ( const char* c = validStartCharacters; *c != '\0'; ++c, ++i )
      {
         if ( *c == ch )
         {
            if ( m_autoUpdateRoundTripTimes && i < lastTurnAroundSize ) // update the round trip information only in case the received character is in turn-around set
            {
               unsigned endTick = MUtilities::GetTickCount();
               unsigned roundTripTime = endTick - startTick; // rollovers will behave correctly here, as we do twos complement number subtraction
               DoUpdateRoundTripTimes(roundTripTime);
            }
            return ch;                                 // Note it needs to be big enough, since the meter often sends a start char in a separate write request
         }
      }
      timeout = timeoutEnd - MUtilities::GetTickCount();
      if ( (int)timeout <= 0 )
         break;
   }
   if ( cnt == 0 )
      MEChannelReadTimeout::Throw(0);
   else
      MCOMException::Throw(M_CODE_STR_P2(M_ERR_DID_NOT_GET_A_VALID_BYTE_AMONG_D1_GARBAGE_BYTES_LAST_ONE_HAD_CODE_X2, M_I("Did not get a valid byte among %d garbage bytes (last one had code 0x%X)"), cnt, (unsigned)(unsigned char)ch));
   M_ENSURED_ASSERT(0);
   return '\0'; // pacify compiler warnings
}


Muint16 MProtocol::CalculateCRC16FromBuffer(const char*, unsigned) const
{
   MException::ThrowNotSupportedForThisType();
   M_ENSURED_ASSERT(0);
   return 0; // pacify compilers
}

void MProtocol::DoStartSession()
{
   MException::ThrowNotSupportedForThisType();
   M_ENSURED_ASSERT(0);
}

void MProtocol::DoEndSession()
{
   MException::ThrowNotSupportedForThisType();
   M_ENSURED_ASSERT(0);
}

void MProtocol::DoTryPasswordEntry(const MByteString&)
{
   MException::ThrowNotSupportedForThisType();
   M_ENSURED_ASSERT(0);
}

void MProtocol::DoTableRead(MCOMNumberConstRef, MByteString&, unsigned)
{
   MException::ThrowNotSupportedForThisType();
   M_ENSURED_ASSERT(0);
}

void MProtocol::DoTableWrite(MCOMNumberConstRef number, const MByteString& data)
{
   DoTableWritePartial(number, data, 0); // this implementation fits majority of cases, put in the root class
}

void MProtocol::DoTableReadPartial(MCOMNumberConstRef, MByteString&, int, int)
{
   MException::ThrowNotSupportedForThisType();
   M_ENSURED_ASSERT(0);
}

void MProtocol::DoTableWritePartial(MCOMNumberConstRef, const MByteString&, int)
{
   MException::ThrowNotSupportedForThisType();
   M_ENSURED_ASSERT(0);
}

void MProtocol::DoFunctionExecute(MCOMNumberConstRef number)
{
   DoFunctionExecuteRequest(number, MByteString());
}

void MProtocol::DoFunctionExecuteRequest(MCOMNumberConstRef number, const MByteString& request)
{
   MByteString response;  // this implementation fits majority of cases, put in the root class
   DoFunctionExecuteRequestResponse(number, request, response);
   #if !M_NO_MCOM_MONITOR
      if ( !response.empty() )
         WriteToMonitor(MGetStdString("Unexpected function response, %u bytes", static_cast<unsigned>(response.size())));
   #endif
}

void MProtocol::DoFunctionExecuteResponse(MCOMNumberConstRef number, MByteString& response)
{
   DoFunctionExecuteRequestResponse(number, MByteString(), response);  // this implementation fits majority of cases, put in the root class
}

void MProtocol::DoFunctionExecuteRequestResponse(MCOMNumberConstRef, const MByteString&, MByteString&)
{
   MException::ThrowNotSupportedForThisType();
   M_ENSURED_ASSERT(0);
}


#if !M_NO_MCOM_IDENTIFY_METER
MStdString MProtocol::DoIdentifyMeter(bool, TableRawDataVector*)
{
   MException::ThrowNotSupportedForThisType();
   M_ENSURED_ASSERT(0);
   return MStdString(); // pacify some compilers
}
#endif // !M_NO_MCOM_IDENTIFY_METER

void MProtocol::DoCheckChannel(bool allowBackgroundCommunication) const
{
   if ( m_channel == NULL )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_UNKNOWN_CHANNEL_S1, "Channel was not assigned to protocol"));
      M_ENSURED_ASSERT(0);
   }

   #if !M_NO_MCOM_PROTOCOL_THREAD
      if ( !allowBackgroundCommunication && m_backgroundCommunicationIsProgressing && m_protocolThread->GetThreadId() != MThreadCurrent::GetStaticCurrentThreadId() )
      {
         MCOMException::ThrowInvalidOperationInForeground();
         M_ENSURED_ASSERT(0);
      }
   #endif
}

#if !M_NO_MCOM_KEEP_SESSION_ALIVE

unsigned MProtocol::DoGetKeepSessionAliveFirstDelay() const
{
   return 0u;
}

unsigned MProtocol::DoSendKeepSessionAliveMessage()
{
   return 0u;
}

#endif

#if !M_NO_PROGRESS_MONITOR

MProgressAction* MProtocol::CreateRootProgressAction()
{
   return m_progressMonitor ? m_progressMonitor->CreateRootAction() : MProgressMonitor::GetDummyAction();
}

MProgressAction* MProtocol::GetLocalProgressAction()
{
   return m_progressMonitor ? m_progressMonitor->GetLocalAction() : MProgressMonitor::GetDummyAction();
}

#endif // !M_NO_PROGRESS_MONITOR

void MProtocol::DoAddCounts(MProtocol& from)
{
   M_ASSERT(&from != this); // it makes no sense to add counts from a protocol to itself

   m_savedTotalAppLayerServices += from.m_savedTotalAppLayerServices;
   m_countApplicationLayerServicesSuccessful += from.m_countApplicationLayerServicesSuccessful;
   m_countApplicationLayerServicesRetried += from.m_countApplicationLayerServicesRetried;
   m_countApplicationLayerServicesFailed += from.m_countApplicationLayerServicesFailed;
   m_countLinkLayerPacketsSuccessful += from.m_countLinkLayerPacketsSuccessful;
   m_countLinkLayerPacketsRetried += from.m_countLinkLayerPacketsRetried;
   m_countLinkLayerPacketsFailed += from.m_countLinkLayerPacketsFailed;

   if ( from.m_maximumRoundTripTime > m_maximumRoundTripTime )
      m_maximumRoundTripTime = from.m_maximumRoundTripTime;
   if ( from.m_minimumRoundTripTime < m_minimumRoundTripTime )
      m_minimumRoundTripTime = from.m_minimumRoundTripTime;
   
   m_sumRoundTripTime += from.m_sumRoundTripTime;
   m_roundTripCounter += from.m_roundTripCounter;
}

unsigned MProtocol::DoConvertNumberToUnsigned(MCOMNumberConstRef number, unsigned upperValue)
{
#if !M_NO_VARIANT
   try
   {
      unsigned ret = number.AsUInt();
      if ( ret <= upperValue )
         return ret;
   }
   catch ( ... )
   {
   }
   MException::Throw(MException::ErrorSoftware,
                     M_CODE_STR_P1(MErrorEnum::CannotConvertToTableOrFunctionNumber,
                                   "Cannot convert '%s' to table or function number", number.AsEscapedString().c_str()));
   M_ENSURED_ASSERT(0);
   return 0;
#else
   MENumberOutOfRange::CheckUnsignedRange(0, upperValue, number);
   return number;
#endif
}

void MProtocol::Sleep(unsigned milliseconds)
{
   if ( m_channel != NULL )
      m_channel->Sleep(milliseconds);
   else
      MUtilities::Sleep(milliseconds); // noninterruptible, but what to do...
}

void MProtocol::DoUpdateRoundTripTimes(unsigned roundTripTime)
{
   if ( m_maximumRoundTripTime < roundTripTime )
      m_maximumRoundTripTime = roundTripTime;
   if ( m_minimumRoundTripTime == 0 || roundTripTime < m_minimumRoundTripTime )
      m_minimumRoundTripTime = roundTripTime;
   m_sumRoundTripTime += (double)roundTripTime;
   ++m_roundTripCounter;
   M_ASSERT(m_roundTripCounter > 0.0);
}
