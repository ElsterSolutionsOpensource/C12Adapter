// sample for communicating with MProtocolC1222

#include <MCORE/MCOREExtern.h>
#include <MCOM/ProtocolC1222.h>
#include <MCOM/ChannelSocket.h>

using namespace std;

#pragma pack(push)
#pragma pack(1)

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

struct ST_005_DEVICE_IDENT
{
   char IDENTIFICATION[20];   // This string is not zero-terminated
};

#pragma pack(pop)

static void DoPrintCharField(const char* name, const char* fld, size_t size)
{
   cout << "   " << name << " : " << MAlgorithm::TrimString(MStdString(fld, size)) << endl;
}

#define PRINT_CHAR_FIELD(fld) DoPrintCharField(#fld, fld, sizeof(fld))

int main(int argc, char** argv)
{
   MChannelSocket channel;
   MProtocolC1222 protocol(&channel, false); // protocol does not own the channel

   MStdString address;
   unsigned int port;

   MStdString password = protocol.GetPassword();
   unsigned int userId = protocol.GetUserId();
   MStdString calledApTitle = protocol.GetCalledApTitle();
   MStdString callingApTitle = protocol.GetCallingApTitle();

   MCommandLineParser cmd;
   cmd.SetDescription("Sample client program for ANSI C1222 communication");

   cmd.DeclareString("address", "IP or DNS name of the meter",  address);
   cmd.DeclareUnsignedInt("port", "IP port of the meter", port);

   cmd.DeclareNamedString('p', "password", "password", "meter password", password);
   cmd.DeclareNamedUnsignedInt('i', "id", "userId", "user id property of protocol", userId);
   cmd.DeclareNamedString('d', "called-ap-title", "calledApTitle", "CalledApTitle property of protocol", calledApTitle);
   cmd.DeclareNamedString('c', "calling-ap-title", "callingApTitle", "callingApTitle property of protocol", calledApTitle);

   int result = cmd.Process(argc, argv);
   if ( result != 0 )
      return result;

   try
   {
      channel.SetPeerAddress(address);
      channel.SetPeerPort(port);

      if ( !password.empty() )
         protocol.SetPassword(password);
      protocol.SetUserId(userId);
      if ( !calledApTitle.empty() )
         protocol.SetCalledApTitle(calledApTitle);
      if ( !callingApTitle.empty() )
         protocol.SetCallingApTitle(callingApTitle);

      cout << "Communicate using session mode\n\n" << endl;
      protocol.SetSessionless(true);

      protocol.QConnect();
      protocol.QStartSession();
      protocol.QTableRead(1, 0, 0); // read ST1
      protocol.QTableRead(5, 0, 1); // read ST5
      protocol.QEndSession();
      protocol.QDisconnect();
      protocol.QCommit();

      const MByteString& st1bytes = protocol.QGetTableData(1, 0); // retrieve st1 data after communication
      ST_001_GENERAL_MFG_ID st1;
      MCOMException::CheckIfExpectedDataSizeDifferent(static_cast<unsigned>(st1bytes.size()), sizeof(st1));
      memcpy(&st1, st1bytes.data(), sizeof(st1));
      PRINT_CHAR_FIELD(st1.MANUFACTURER);
      PRINT_CHAR_FIELD(st1.ED_MODEL);
      PRINT_CHAR_FIELD(st1.MFG_SERIAL_NUMBER);

      const MByteString& st5bytes = protocol.QGetTableData(1, 1); // retrieve st5 data after communication
      ST_005_DEVICE_IDENT st5;
      MCOMException::CheckIfExpectedDataSizeDifferent(static_cast<unsigned>(st5bytes.size()), sizeof(st5));
      memcpy(&st5, st5bytes.data(), sizeof(st5));
      PRINT_CHAR_FIELD(st5.IDENTIFICATION);
   }
   catch ( MException& ex )
   {
      cout << "ERROR: " << ex.AsString() << endl;
      return EXIT_FAILURE;
   }

   cout << "Done" << endl;
   return EXIT_SUCCESS;
}
