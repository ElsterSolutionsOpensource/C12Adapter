#ifndef MCOM_MONITORSYSLOG_H
#define MCOM_MONITORSYSLOG_H
/// \addtogroup MCOM
///@{
/// \file MCOM/MonitorSyslog.h

#include <MCOM/MCOMDefs.h>
#include <MCOM/Monitor.h>

#if !M_NO_MCOM_MONITOR_SYSLOG

/// Activity monitor object that dumps communication into a syslog.
///
/// This implementation of monitor is for UNIX-like operation systems.
/// It is possible to enable it for Windows as well using a third party syslog library.
///
/// Sharing of monitors among simultaneously communicating channels 
/// can produce obscure output or lead to synchronization errors,
/// therefore, each channel should have its own monitor object.
/// The monitor is registered with the client channel object using the service
/// \refprop{MChannel::SetMonitor,MChannel::Monitor}.
///
/// \if CPP
/// Note that when M_NO_MCOM_MONITOR_SHARED_POINTER=0
/// the monitor object should be created on a heap with operator new,
/// and handled through the pointer type \ref MMonitor::Pointer.
/// The deletion of such object will be done through the stared pointer.
/// When M_NO_MCOM_MONITOR_SHARED_POINTER=1 the user will be responsible
/// for deletion of the object, however the monitor object should outlive
/// the channel that it monitors.
/// \endif
///
class MCOM_CLASS MMonitorSyslog : public MMonitor
{
public: // Constructor and destructor:

   /// Constructor that creates a monitor with the given message prefix.
   ///
   /// \param prefix Log prefix, MCOM by default.
   /// \param verbose Log level, 4 by default.
   ///
   /// \pre The object should be created on a heap with operator new,
   /// and handled with a shared pointer, otherwise the behavior is undefined.
   /// File, if given, shall be a valid directory or a file name,
   /// but the check is deferred to the attaching time.
   ///
   MMonitorSyslog(const MStdString& prefix = "MCOM", int verbose = 4);

   /// Object destructor.
   /// The children of this class shall call DoFinish in their destructors.
   ///
   virtual ~MMonitorSyslog();

public: // Services (property getters and setters):

public: // Services:

   /// Tell that the application is starting a sequence of events that it would like
   /// to monitor, attach by attempting to open the file. The action is not immediate,
   /// and some time might pass before the file is open.
   ///
   /// Client identification is a string that somehow tells about the client.
   /// Like for the channel based on COM port, this would be the com port name,
   /// and for the socket it will be the address and the port.
   ///
   virtual void Attach(const MStdString& clientIdentitfication);

public: // Event handling services:

   /// Send text message with the specified code.
   ///
   /// \pre Code is valid and message has a valid format.
   ///
   virtual void OnMessage(MessageType code, const char* message, int length);

private: // Properties:

   static MStdString CodeToString(int code);
   void LogDump(const char* data, int length);

   MStdString m_prefix;
   int m_verbose;

   M_DECLARE_CLASS(MonitorSyslog)
};

#endif // !M_NO_MCOM_MONITOR_SYSLOG

///@}
#endif // MCOM_MONITORSYSLOG_H
