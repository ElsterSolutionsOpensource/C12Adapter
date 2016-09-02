// File MCOM/MonitorSyslog.cpp
#include "MCOMExtern.h"
#include "MCOMExceptions.h"
#include "MonitorSyslog.h"
#include "MonitorFilePrivateThread.h"
#include <MCORE/MTime.h>

#if !M_NO_MCOM_MONITOR_SYSLOG

#include <syslog.h>

   #if !M_NO_REFLECTION

      /// Constructor that creates a monitor with the given message prefix.
      ///
      /// \param prefix Log prefix, MCOM by default.
      ///
      /// \pre The object should be created on a heap with operator new,
      /// and handled with a shared pointer, otherwise the behavior is undefined.
      /// File, if given, shall be a valid directory or a file name,
      /// but the check is deferred to the attaching time.
      ///
      static MMonitorSyslog* DoNew1(const MStdString& prefix)
      {
         return M_NEW MMonitorSyslog(prefix);
      }

      /// Constructor that creates a monitor with the default message prefix.
      ///
      /// \pre The object should be created on a heap with operator new,
      /// and handled with a shared pointer, otherwise the behavior is undefined.
      /// File, if given, shall be a valid directory or a file name,
      /// but the check is deferred to the attaching time.
      ///
      static MMonitorSyslog* DoNew0()
      {
         return DoNew1("MCOM");
      }
   #endif

M_START_PROPERTIES(MonitorSyslog)
M_START_METHODS(MonitorSyslog)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(MonitorFile, New, DoNew1, 1, ST_MObjectP_S_constMStdStringA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(MonitorFile, New, DoNew0, 0, ST_MObjectP_S)
M_END_CLASS(MonitorSyslog, Monitor)

MMonitorSyslog::MMonitorSyslog(const MStdString& prefix, int verbose)
:
   m_prefix(prefix),
   m_verbose(verbose)
{
}

MMonitorSyslog::~MMonitorSyslog()
{
}

void MMonitorSyslog::Attach(const MStdString& mediaIdentification)
{
   m_listening = -1; // initiate sending of the information
   MMonitor::Attach(mediaIdentification);
}

void MMonitorSyslog::OnMessage(MMonitor::MessageType code, const char* data, int length)
{
   if ( m_verbose < 1 )
      return;
   else if ( m_verbose == 1 && code < MessageProtocolApplicationLayerFail )
      return;
   else if ( m_verbose == 2 && code < MessageProtocolLinkLayerInformation )
      return;
   else if ( m_verbose == 3 && code < MessageChannelAttach )
      return;

   if ( code == MessageChannelByteRx )
   {
      syslog(LOG_DEBUG, "%s: RX < (%i byte(s))", m_prefix.c_str(), length);
      LogDump(data, length);
   }
   else if ( code == MessageChannelByteTx )
   {
      syslog(LOG_DEBUG, "%s: TX > (%i byte(s))", m_prefix.c_str(), length);
      LogDump(data, length);
   }
   else
   {
      MStdString messageText = CodeToString(code);
      syslog(LOG_DEBUG, "%s: %s %s", m_prefix.c_str(), messageText.c_str(), data);
   }
}

MStdString MMonitorSyslog::CodeToString(int code)
{
   MStdString result;
   switch ( code )
   {
   case MessageChannelAttach:
      result.assign("Attach", 6);
      break;
   case MessageChannelConnect:
      result.assign("Channel Connected", 17);
      break;
   case MessageChannelDisconnect:
      result.assign("Channel Disconnected", 20);
      break;
   default:
      ; // leave empty
   }
   return result;
}

void MMonitorSyslog::LogDump(const char* data, int length)
{
   while ( length > 0 )
   {
      const int maxRowLength = 16;
      int rowLength = length > maxRowLength ? maxRowLength : length;
      MByteString row = MUtilities::BytesToHex(MByteString(data, rowLength), "XX ");
      syslog(LOG_DEBUG, "%s: %s", m_prefix.c_str(), row.c_str());
      length -= rowLength;
      data += rowLength;
   }
}

#endif // !M_NO_MCOM_MONITOR_SYSLOG
