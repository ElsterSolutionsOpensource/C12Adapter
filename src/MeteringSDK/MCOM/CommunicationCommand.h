#ifndef MCOM_COMMUNICATIONCOMMAND_H
#define MCOM_COMMUNICATIONCOMMAND_H
// File MCOM/CommunicationCommand.h

#include <MCOM/MCOMDefs.h>

/// \cond SHOW_INTERNAL

// Command is the utility structure that serves for internal
// representation of the item in the command queue.
//
class MCOM_CLASS MCommunicationCommand
{
   friend class MCommunicationQueue;

public: // Type:

   enum CommandFeatureMask
   {
      FeatureNumberPresent       = 0x0100,
      FeatureRequestPresent      = 0x0200,
      FeatureResponsePresent     = 0x0400,
      FeatureOffsetPresent       = 0x0800,
      FeatureLengthPresent       = 0x1000,
   };

   // Type of the command to execute by the protocol.
   // Used by the queue mechanism.
   //
   enum CommandType
   {
      CommandWriteToMonitor             =  0 | FeatureRequestPresent, // Write some message to a monitor, if one connected
      CommandConnect                    =  1, // Connect to the meter
      CommandDisconnect                 =  2, // Disconnect the meter
      CommandStartSession               =  3, // Start the session
      CommandEndSession                 =  4, // End the session
      CommandEndSessionNoThrow          =  5, // End the session ignoring errors
#if !M_NO_MCOM_IDENTIFY_METER
      CommandIdentifyMeter              =  6 | FeatureResponsePresent,
#endif
      CommandRead                       =  7 | FeatureNumberPresent | FeatureResponsePresent | FeatureLengthPresent,
      CommandWrite                      =  8 | FeatureNumberPresent | FeatureRequestPresent,
      CommandReadPartial                =  9 | FeatureNumberPresent | FeatureResponsePresent | FeatureOffsetPresent | FeatureLengthPresent,
      CommandWritePartial               = 10 | FeatureNumberPresent | FeatureRequestPresent  | FeatureOffsetPresent,
      CommandExecute                    = 11 | FeatureNumberPresent,
      CommandExecuteRequest             = 12 | FeatureNumberPresent | FeatureRequestPresent,
      CommandExecuteResponse            = 13 | FeatureNumberPresent | FeatureResponsePresent | FeatureLengthPresent,
      CommandExecuteRequestResponse     = 14 | FeatureNumberPresent | FeatureRequestPresent | FeatureResponsePresent | FeatureLengthPresent
   };

#if !M_NO_MCOM_COMMAND_QUEUE

   // Command queue used by the protocol.
   //
   typedef std::vector<MCommunicationCommand>
      QueueType;

private:

   // constructor for serialization
   //
   MCommunicationCommand()
   :
      m_id(-1),
#if M_DEBUG
      m_type((CommandType)UINT_MAX),
      m_offset(INT_MIN),
      m_length(INT_MIN),
      m_littleEndian(false),
#endif
      m_responsePresent(false)
   {
   }

   // constructor for serialization
   //
   MCommunicationCommand(const MCommunicationCommand& other)
   :
      m_id(other.m_id),
      m_type(other.m_type),
      m_number(other.m_number),
      m_request(other.m_request),
      m_response(other.m_response),
      m_offset(other.m_offset),
      m_length(other.m_length),
      m_littleEndian(other.m_littleEndian),
      m_responsePresent(other.m_responsePresent)
   {
   }

public: // Constructor and destructor:

   // Constructing static method that creates a command with no parameters.
   //
   static MCommunicationCommand* New(CommandType type);

   MCommunicationCommand* NewClone() const;

   // Destructor.
   //
   ~MCommunicationCommand();

public: // Properties:

   CommandType GetCommandType() const
   {
      return m_type;
   }

   int GetDataId() const
   {
      return m_id;
   }

   void SetDataId(int id)
   {
      m_id = id;
   }

   MCOMNumberConstRef GetNumber() const
   {
      M_ASSERT((m_type & FeatureNumberPresent) != 0);
      return m_number;
   }

   void SetNumber(MCOMNumberConstRef number)
   {
      M_ASSERT((m_type & FeatureNumberPresent) != 0);
      m_number = number;
   }

   const MByteString& GetRequest() const
   {
      M_ASSERT((m_type & FeatureRequestPresent) != 0);
      return m_request;
   }
   void SetRequest(const MByteString& request)
   {
      M_ASSERT((m_type & FeatureRequestPresent) != 0);
      m_request = request;
   }

   const MByteString& GetResponse() const;
   void SetResponse(const MByteString& response);
   void AppendResponse(const MByteString& response);

   int GetOffset() const
   {
      M_ASSERT((m_type & FeatureOffsetPresent) != 0);
      M_ASSERT(m_offset != INT_MIN);
      return m_offset;
   }

   void SetOffset(int offset)
   {
      M_ASSERT((m_type & FeatureOffsetPresent) != 0);
      m_offset = offset;
      M_ASSERT(m_offset != INT_MIN);
   }

   int GetLength() const
   {
      M_ASSERT((m_type & FeatureLengthPresent) != 0);
      M_ASSERT(m_length != INT_MIN);
      return m_length;
   }

   void SetLength(int length)
   {
      M_ASSERT((m_type & FeatureLengthPresent) != 0);
      m_length = length;
      M_ASSERT(m_length != INT_MIN);
   }


   bool GetLittleEndian() const
   {
      return m_littleEndian;
   }
   void SetLittleEndian(bool yes)
   {
      m_littleEndian = yes;
   }

#if !M_NO_PROGRESS_MONITOR
   double GetProgressWeight() const M_NO_THROW;
#endif

private:

   MCommunicationCommand& operator=(const MCommunicationCommand&);

public: // Properties are accessible publicly:

   // Data identifier, or -1 if the identifier is not provided, always present.
   //
   int m_id;

   // The command type of this command entry, always present.
   //
   CommandType m_type;

   // The number of the entry, typically the function or the table number.
   // Generically, it can be of any type, but most often it is an integer.
   // IEC protocols also allow it to be a string.
   // FeatureNumberPresent flag shall be set for this parameter to apply.
   //
   MCOMNumber m_number;

   // The request parameter, whatever is defined by the command.
   // FeatureRequestPresent flag shall be set for this parameter to apply.
   //
   MByteString m_request;

   // Response parameter, whatever is defined by the command.
   // FeatureResponsePresent flag shall be set for this parameter to apply.
   //
   MByteString m_response;

   // Offset of the data, case for the partial read or write for example.
   // FeatureOffsetPresent flag shall be set for this parameter to apply.
   //
   int m_offset;

   // Length of the data, case for the partial read or write for example.
   // FeatureLengthPresent flag shall be set for this parameter to apply.
   //
   int m_length;


   // Whether the command has to be done in little endian vs. big endian context
   //
   bool m_littleEndian;

   // Whether the response was present
   //
   bool m_responsePresent;

#endif // !M_NO_MCOM_COMMAND_QUEUE
};

#if !M_NO_MCOM_COMMAND_QUEUE

typedef MCOM_TEMPLATE_CLASS std::vector<MCommunicationCommand*>
   MCommunicationQueueVectorType;

// Command queue used by the protocol.
// The command queue owns their polymorphic objects.
//
class MCOM_CLASS MCommunicationQueue : public MCommunicationQueueVectorType
{
public:

   MCommunicationQueue();
   ~MCommunicationQueue();

   MCommunicationCommand* GetResponseCommandNoThrow(MCommunicationCommand::CommandType type, MCOMNumberConstRef number, int id = -1);
   MCommunicationCommand* GetResponseCommand(MCommunicationCommand::CommandType type, MCOMNumberConstRef number, int id = -1);

   void push_back(MCommunicationCommand* command);
   void clear();
   void erase(MCommunicationQueueVectorType::iterator it, MCommunicationQueueVectorType::iterator itEnd);
};

#endif // !M_NO_MCOM_COMMAND_QUEUE

/// \endcond SHOW_INTERNAL
#endif
