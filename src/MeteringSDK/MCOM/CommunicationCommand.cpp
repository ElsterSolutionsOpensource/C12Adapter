// File MCOM/CommunicationCommand.cpp

#include "MCOMExtern.h"
#include "CommunicationCommand.h"
#include "Protocol.h"
#include "MCOMExceptions.h"

#if !M_NO_MCOM_COMMAND_QUEUE

   using namespace std;

   M_DEFINE_TEMPLATE_CLASS(std::vector<MCommunicationCommand*>)

MCommunicationCommand::~MCommunicationCommand()
{
}

MCommunicationCommand* MCommunicationCommand::New(CommandType type)
{
   MCommunicationCommand* command = M_NEW MCommunicationCommand; // with all data uninitialized
   command->m_type = type;
   return command;
}

MCommunicationCommand* MCommunicationCommand::NewClone() const
{
   return M_NEW MCommunicationCommand(*this); // with all data uninitialized
}

const MByteString& MCommunicationCommand::GetResponse() const
{
    M_ASSERT((m_type & FeatureResponsePresent) != 0);
    if ( !m_responsePresent )
    {
       MException::ThrowNoValue();
       M_ENSURED_ASSERT(0);
    }
    return m_response;
}

void MCommunicationCommand::SetResponse(const MByteString& response)
{
    M_ASSERT((m_type & FeatureResponsePresent) != 0);
    m_response = response;
    m_responsePresent = true;
}

void MCommunicationCommand::AppendResponse(const MByteString& response)
{
    M_ASSERT((m_type & FeatureResponsePresent) != 0);
    m_response += response;
    m_responsePresent = true;
}

#if !M_NO_PROGRESS_MONITOR
double MCommunicationCommand::GetProgressWeight() const M_NO_THROW
{
   double progressWeight;
   switch ( m_type )
   {
   case CommandWriteToMonitor:
   case CommandDisconnect:
      progressWeight = 2.0;
      break;
   case CommandConnect:
      progressWeight = 10.0;
      break;
   case CommandStartSession:
      progressWeight = 50.0;
      break;
   case CommandEndSession:
   case CommandEndSessionNoThrow:
      progressWeight = 20.0;
      break;
#if !M_NO_MCOM_IDENTIFY_METER
   case CommandIdentifyMeter:
      progressWeight = 100.0;
      break;
#endif
   default:
      progressWeight = 2.0;
      try
      {
         if ( (m_type & FeatureRequestPresent) != 0 )
            progressWeight += double(m_request.size());

         if ( (m_type & FeatureLengthPresent) != 0 && GetLength() > 0 )
            progressWeight += double(GetLength());
         else if ( (m_type & FeatureResponsePresent) != 0 ) // response of an unknown length
            progressWeight += 64.0; // add an arbitrary count of bytes

      }
      catch ( ... )
      {
         M_ASSERT(0);
      }
   }
   M_ASSERT(progressWeight > 0.0);
   return progressWeight;
}
#endif // !M_NO_PROGRESS_MONITOR

MCommunicationQueue::MCommunicationQueue()
:
   MCommunicationQueueVectorType()
{
}

MCommunicationQueue::~MCommunicationQueue()
{
   clear();
}

void MCommunicationQueue::clear()
{
   for ( iterator it = begin(); it != end(); ++it )
      delete *it;
   MCommunicationQueueVectorType::clear();
}

void MCommunicationQueue::erase(MCommunicationQueueVectorType::iterator it, MCommunicationQueueVectorType::iterator itEnd)
{
   for ( iterator i = it; i != itEnd; ++i )
      delete *i;
   MCommunicationQueueVectorType::erase(it, itEnd);
}

void MCommunicationQueue::push_back(MCommunicationCommand* command)
{
   if ( ((command->GetCommandType() & MCommunicationCommand::FeatureResponsePresent) != 0) && GetResponseCommandNoThrow(command->m_type, command->m_number, command->m_id) )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_COMMAND_WITH_SUCH_PARAMETERS_IS_QUEUED_ALREADY, "Command with such parameters is queued already"));
      M_ENSURED_ASSERT(0);
   }
   MCommunicationQueueVectorType::push_back(command);
}

   inline MCommunicationCommand::CommandType DoGetGeneralizedCommand(MCommunicationCommand::CommandType type)
   {
      switch ( type )
      {
      case MCommunicationCommand::CommandReadPartial:
         return MCommunicationCommand::CommandRead;
      case MCommunicationCommand::CommandExecuteRequestResponse:
         return MCommunicationCommand::CommandExecuteResponse;
      default:
         return type;
      }
   }

MCommunicationCommand* MCommunicationQueue::GetResponseCommandNoThrow(MCommunicationCommand::CommandType type, MCOMNumberConstRef number, int id)
{
   reverse_iterator it = rbegin(); // go from the end of the queue, this shall be more efficient as the results are usually at the tail
   reverse_iterator itEnd = rend();
   for ( ; it != itEnd; ++it )
   {
      MCommunicationCommand* command = *it;
      if ( DoGetGeneralizedCommand(type) == DoGetGeneralizedCommand(command->GetCommandType()) &&
           ((type & MCommunicationCommand::FeatureResponsePresent) != 0) &&
           ((type & MCommunicationCommand::FeatureNumberPresent) == 0 || command->GetNumber() == number) &&
            command->m_id == id )
      {
         return command; // done, found.
      }
   }
   return NULL;
}

MCommunicationCommand* MCommunicationQueue::GetResponseCommand(MCommunicationCommand::CommandType type, MCOMNumberConstRef number, int id)
{
   MCommunicationCommand* command = GetResponseCommandNoThrow(type, number, id);
   if ( command == NULL )
   {
      MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_COULD_NOT_FIND_DATA_WITH_SPECIFIED_PARAMETERES, "Could not find data with specified parameters"));
      M_ENSURED_ASSERT(0);
   }
   return command;
}

#endif // !M_NO_MCOM_COMMAND_QUEUE
