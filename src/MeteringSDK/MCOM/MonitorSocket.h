#ifndef MCOM_MONITORSOCKET_H
#define MCOM_MONITORSOCKET_H
/// \addtogroup MCOM
///@{
/// \file MCOM/MonitorSocket.h

#include <MCOM/MonitorFile.h>

#if !M_NO_MCOM_MONITOR && !M_NO_MULTITHREADING

/// Activity monitor object based on a TCP socket.
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
class MCOM_CLASS MMonitorSocket : public MMonitorFile
{
   friend class MMonitorSocketConnectionHandler;
public: // Constructor and destructor:

   /// Constructor that creates a monitor with a socket with the specified
   /// host parameter, but which is not connected.
   /// The parameter should denote a valid computer IP address or name.
   /// If the parameter is empty, the socket address is determined from configuration.
   ///
   /// \pre The object should be created on a heap with operator new,
   /// and handled with a shared pointer, otherwise the behavior is undefined.
   /// No check is done whether the socket can be open.
   ///
   MMonitorSocket(const MStdString& clientAddress = MVariant::s_emptyString);

   /// Object destructor.
   ///
   virtual ~MMonitorSocket();

public: // Services (property getters and setters):

   /// Whether the monitor is currently open and connected
   /// to MMonitorSocket object. 
   ///
   /// Also please see IsFileOpen of the parent class that tells if the file
   /// open for logging.
   ///
   bool IsSocketOpen() const;

   /// Checks whether the monitor host is a local address or not.
   ///
   bool IsAddressLocal() const;

   ///@{
   /// Client IP address or host name.
   ///
   const MStdString& GetClientAddress() const
   {
      return m_clientAddress;
   }
   void SetClientAddress(const MStdString& address);
   ///@}


   /// Tell that the application is starting a sequence of events that it would like
   /// to monitor, attach by attempting to open the socket.
   ///
   virtual void Attach(const MStdString& mediaIdentification);

protected: // Protected services:
/// \cond SHOW_INTERNAL

   /// Detach from the monitor by closing the socket, if it was connected.
   ///
   virtual void Detach();

   /// Send the buffer to the monitor entity on the background.
   /// The returned value is nonzero if the class is still interested in receiving the messages.
   /// The children classes can overload this service to provide their ways
   /// of monitoring the events. Typically they will call the parent implementation,
   /// and return nonzero unconditionally if the parent returned nonzero.
   ///
   /// \pre Called from the OnIdle procedure of the background thread.
   /// No check is done for such precondition.
   ///
   virtual unsigned DoSendBackgroundBuffer(const MByteString& backgroundThreadBuffer);

/// \endcond SHOW_INTERNAL
private: // Data members:

   // Host name or address to create socket, one actually used in communication.
   // Note it can be different from m_clientAddress, as it assumes some translation
   // to be done for names like localhost or 127.0.0.1.
   //
   MStdString m_host;

   // Client address property. It can be different from m_host, as it does not
   // do the translation from localhost and 127.0.0.1.
   //
   MStdString m_clientAddress;

   // Socket used to transmit information.
   //
   MStreamSocket m_socket;

   // True if no socket open operation has failed so far.
   //
   MInterlocked m_noSocketOpenFailed;

   // Time for the next try to connect to the monitor, in milliseconds.
   // If this is null, the attempt will be made immediately.
   //
   MInterlocked m_nextTimeToConnect;
   
   // Store media identification from the previous Attach here.
   // Will resend this data to new socket in case of reconnection.
   //
   MStdString m_mediaIdentification;

   M_DECLARE_CLASS(MonitorSocket)
};

#endif // !M_NO_MCOM_MONITOR

///@}
#endif
