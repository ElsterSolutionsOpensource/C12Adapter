// File reader.cpp
//
// Universal configurable reader and link checker

#include <MCORE/MCOREExtern.h>
#include <MCOM/MCOM.h>
#include <fstream>
#include <signal.h>
#include "Setup.h"

using namespace std;

struct ST_001_GENERAL_MFG_ID
{
   char   MANUFACTURER[4];    // This string is not zero-terminated
   char   ED_MODEL[8];        // This string is not zero-terminated
   Muint8 HW_VERSION_NUMBER;  // Hardware Version Number
   Muint8 HW_REVISION_NUMBER; // Hardware Revision Number
   Muint8 FW_VERSION_NUMBER;  // Firmware Version Number
   Muint8 FW_REVISION_NUMBER; // Firmware Revision Number
   char   MFG_SERIAL_NUMBER[16];
};

class InterruptHandler // this is actually a singleton
{
   typedef void (*SignalHandlerType)(int);
   static SignalHandlerType s_previousInterruptHandler;
   static bool s_isInterrupted; // Here we know CEO is a singleton object.

   static void MyInterruptHandler(int)
   {
      s_isInterrupted = true;
   }

public:

   static bool IsInterrupted()
   {
      return s_isInterrupted;
   }

   static void ClearIsInterrupted()
   {
      s_isInterrupted = false;
   }

   InterruptHandler()
   {
      s_previousInterruptHandler = signal(SIGINT, MyInterruptHandler); // Handle Ctrl-C
      M_ASSERT(s_previousInterruptHandler != MyInterruptHandler); // check we did not call it twice
   }

   ~InterruptHandler()
   {
      signal(SIGINT, s_previousInterruptHandler); // restore signal
   }
};
bool InterruptHandler::s_isInterrupted = false;
InterruptHandler::SignalHandlerType InterruptHandler::s_previousInterruptHandler = NULL;
static InterruptHandler s_interruptHandler;

unsigned successfulCycles = 0;
unsigned failures = 0;
unsigned linkLayerRetries = 0;

static void CommitCommunication(MProtocol* proto)
{
   proto->QCommit(true);
   while ( !proto->QIsDone() )
   {
      linkLayerRetries = proto->GetCountLinkLayerPacketsRetried();
      MUtilities::Sleep(100);
      if ( s_interruptHandler.IsInterrupted() )
      {
         s_interruptHandler.ClearIsInterrupted();
         proto->GetChannel()->CancelCommunication(true);
      }
   }
   linkLayerRetries = proto->GetCountLinkLayerPacketsRetried();
}

static void Communicate(MProtocol* proto, const MStdStringVector& tables)
{
   proto->QConnect();
   proto->QStartSession();
   proto->QTableRead(1, 0, 10000);
   CommitCommunication(proto);

   MByteString t1buff = proto->QGetTableData(1, 10000);

   MStdStringVector::const_iterator it = tables.begin();
   MStdStringVector::const_iterator itEnd = tables.end();
   for ( int count = 1; it != itEnd; ++it, ++count )
   {
      MStdString item = *it; // going to modify item
      try
      {
         if ( item.size() > 2 && !m_isdigit(item[0]) && !m_isdigit(item[0]) )
         {
            if ( item[1] == 'T' && (item[0] == 'S' || item[0] == 'M') ) // table read
            {
               int itemInt = MToLong(item.substr(2));
               if ( item[0] == 'M' )
                  itemInt += 2048;
               proto->QTableRead(itemInt, 0, count);
            }
            else if ( item[1] == 'F' && (item[0] == 'S' || item[0] == 'M') )
            {
               MStdString::size_type openingBrace = item.find('(');
               MStdString::size_type closingBrace = item.find_last_of(')');
               if ( openingBrace == MStdString::npos || closingBrace == MStdString::npos || openingBrace >= closingBrace )
               {
                  MException::Throw("Expected function syntax is like SF3(), MF150(01 02 03), ...");
               }
               int itemInt = MToLong(item.substr(2, openingBrace - 2));
               if ( item[0] == 'M' )
                  itemInt += 2048;
               MByteString request;
               MStdString::size_type requestSize = closingBrace - openingBrace - 1;
               if ( requestSize > 0 )
                  request = MUtilities::HexStringToBytes(item.substr(openingBrace + 1, requestSize));
               proto->QFunctionExecuteRequestResponse(itemInt, request, count);
            }
            else
               MException::Throw("Only prefixes supported are ST, MT, SF, MF");
         }
         else
         {
            int itemInt = MToLong(item);
            proto->QTableRead(itemInt, 0, count);
         }
      }
      catch ( MException& ex )
      {
         ex.Prepend("Bad syntax of argument '" + item + "': ");
         throw;
      }
   }

   proto->QEndSession();
   CommitCommunication(proto);

   it = tables.begin();
   for ( int count = 1; it != itEnd; ++it, ++count )
   {
      MStdString item = *it; // going to modify item
      int itemInt = -1;
      if ( item.size() > 2 && !m_isdigit(item[0]) && !m_isdigit(item[0]) )
      {
         if ( item[1] == 'T' && (item[0] == 'S' || item[0] == 'M') ) // table read
         {
            itemInt = MToLong(item.substr(2));
            if ( item[0] == 'M' )
               itemInt += 2048;
         }
      }
      else
         itemInt = MToLong(item);
      cout << item << ":\n"
           << MUtilities::BytesToHexString(proto->QGetTableData(itemInt, count),
                                           "  XX XX XX XX  XX XX XX XX  XX XX XX XX  XX XX XX XX\n")
           << endl;
   }

   // Report
   //
   if ( t1buff.size() != sizeof(ST_001_GENERAL_MFG_ID) )
      MException::Throw("Table 1 size mismatch!");
   ST_001_GENERAL_MFG_ID t1;
   memcpy(&t1, t1buff.data(), sizeof(ST_001_GENERAL_MFG_ID));
   ++successfulCycles;

   MStdString str = "Device ";
   str += MAlgorithm::TrimString(MStdString(t1.ED_MODEL, sizeof(t1.ED_MODEL)));
   str += '(';
   str += MToStdString(static_cast<unsigned>(t1.FW_VERSION_NUMBER));
   str += '.';
   str += MToStdString(static_cast<unsigned>(t1.FW_REVISION_NUMBER));
   str += ") cycle/errors/retries: ";
   str += MToStdString(successfulCycles);
   str += '/';
   str += MToStdString(failures);
   str += '/';
   str += MToStdString(linkLayerRetries);
   cout << str << endl;
   proto->WriteToMonitor(str);
}

int main(int argc, char** argv)
{
   Setup setup;
   if ( !setup.Initialize(argc, argv) )
      return EXIT_FAILURE;

   MProtocol* proto = setup.GetProtocol();
   M_ASSERT(proto != NULL); // ensured by successful return from Initialize

   MProtocolC12* protoC12 = M_DYNAMIC_CAST(MProtocolC12, proto);
   if ( protoC12 != NULL )
      protoC12->SetEndSessionOnApplicationLayerError(true); // this is the only property to override

   cout << "Entering test loop. Press Ctrl-C to interrupt." << endl;
   for ( int i = 0; i < setup.GetNumberOfIterations(); ++i )
   {
      try
      {
         Communicate(proto, setup.GetTableNames());
      }
      catch ( MEOperationCancelled& )
      {
        cout << "Test loop is cancelled with Ctrl-C." << endl;
        break;
      }
      catch ( MException& ex )
      {
         cerr << "### Error: " << ex.AsString() << endl;
         ++failures;
      }

      proto->Disconnect(); // never throws
   }

   linkLayerRetries = proto->GetCountLinkLayerPacketsRetried();
   cout << "Successful cycles: " << successfulCycles 
        << ", errors: " << failures
        << ", retries: " << linkLayerRetries;
   unsigned total = successfulCycles - failures;
   if ( total != 0 )
      cout << ", success rate: " << MMath::Round(double(successfulCycles) * 100.0 / double(total), 4) << "%";
   cout << endl;
   return (failures != 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
