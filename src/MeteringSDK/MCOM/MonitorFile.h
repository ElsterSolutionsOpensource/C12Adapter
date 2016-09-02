#ifndef MCOM_MONITORFILE_H
#define MCOM_MONITORFILE_H
/// \addtogroup MCOM
///@{
/// \file MCOM/MonitorFile.h

#include <MCOM/MCOMDefs.h>
#include <MCOM/Monitor.h>
#include <MCOM/LogFileWriter.h>

#if !M_NO_MCOM_MONITOR && !M_NO_MULTITHREADING

/// Activity monitor object that dumps communication into a binary file.
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
class MCOM_CLASS MMonitorFile : public MMonitor
{
   friend class MMonitorFilePrivateThread;
   friend class MMonitorSocketConnectionHandler;

public: // Constructor and destructor:

   /// Constructor that creates a monitor with the given file name, 
   /// or without file name, in which case no monitoring will be done.
   ///
   /// \pre The object should be created on a heap with operator new,
   /// and handled with a shared pointer, otherwise the behavior is undefined.
   /// File, if given, shall be a valid directory or a file name,
   /// but the check is deferred to the attaching time.
   ///
   MMonitorFile(const MStdString& fileName = MVariant::s_emptyString);

   /// Object destructor.
   /// The children of this class shall call DoFinish in their destructors.
   ///
   virtual ~MMonitorFile();

public: // Services (property getters and setters):

   ///@{
   /// The path and name of the file that will be used to store communication data.
   ///
   /// If the file name is "", then none of the communication data is saved.
   /// The FileName must be specified before the client wants to start saving data.
   /// The communication data is saved to the file as the transaction occurs.
   /// If the specified file already exists, then no error or
   /// warning is generated and the new data is appended to the existing file.
   ///
   /// The file will be in binary format. To view the communication transactions, load the file
   /// into the Monitor.
   /// The Monitor does NOT have to be running in order for communication
   /// data to be written to the file.
   ///
   /// \default_value "" (empty string)
   ///
   /// \possible_values
   ///  - Any valid path and file name.
   ///
   const MStdString& GetFileName() const
   {
      return m_fileName;
   }
   void SetFileName(const MStdString& name);
   ///@}

   ///@{
   /// Maximum size of the binary log file in kilobytes.
   ///
   /// Once the file reaches the maximum size, it does not get any bigger.
   /// Old data is removed from the file as new data is added. The file will
   /// always contain the latest communication data.
   ///
   /// \default_value 0 : A zero value indicates that there is no restriction on the size of the
   /// file.
   ///
   /// \possible_values
   ///  - 0 .. 2,147,483,647
   ///
   unsigned GetMaxFileSizeKB() const
   {
      return m_maxFileSizeKB;
   }
   void SetMaxFileSizeKB(unsigned size);
   ///@}

   ///@{
   /// Whether or not the file shall be obfuscated.
   ///
   bool GetObfuscate() const
   {
      return m_obfuscate;
   }
   void SetObfuscate(bool yes);
   ///@}

public: // Services:

   /// Tell that the application is starting a sequence of events that it would like
   /// to monitor, attach by attempting to open the file. The action is not immediate, 
   /// and some time might pass before the file is open.
   ///
   /// Client identification is a string that somehow tells about the client.
   /// Like for the channel based on COM port, this would be the com port name,
   /// and for the socket it will be the address and the port. It is used to construct default
   /// file name for MMonitorFile, if the file is not specified directly.
   ///
   virtual void Attach(const MStdString& clientIdentitfication);

   /// Detach from the monitor by attempting to close the file, if it was open previously.
   /// The action is not immediate, and some time might pass before the file is closed.
   ///
   virtual void Detach();

   /// Delete the file, if it was previously created.
   ///
   /// Deletes the binary log file that stores the communication transactions. It does NOT clear
   /// the data from the Monitor if one is attached to the connection.
   ///
   void DeleteFile();

public: // Event handling services:

   /// Send text message with the specified code.
   ///
   /// \pre Code is valid and message has a valid format.
   ///
   virtual void OnMessage(MessageType code, const char* message, int length);

   /// What to do when the new page is about to be written.
   ///
   /// This can be used to write some special message that has to be present on every page.
   /// For instance, feature is used to have absolute timestamps in the log.
   ///
   virtual void OnPageBoundHit();

protected: // Services:

   /// This service finalizes and detaches the file, if it was set up previously.
   ///
   /// \pre There is no error to detach if there was no file attached.
   ///
   void DoFileDetach();

   /// This service shall be called from every destructor of the child class.
   /// It does all necessary preparations to finish the background processing
   /// at the level of the most upper class of the hierarchy
   /// where all virtuals have their final meaning.
   ///
   /// \pre Called from destructor, no check is done for such precondition.
   /// It is okay to call this service multiple times, like from a child destructor,
   /// and from the destructor of its parent.
   ///
   void DoFinish();

   /// Send the buffer to the monitor entity on the background.
   /// The returned value is nonzero if the class is still interested in receiving the messages.
   /// The children classes can overload this service to provide their ways
   /// of monitoring the events. Typically they will call the parent implementation,
   /// and return nonzero unconditionally if the parent returned nonzero.
   ///
   /// \pre Called from the Run procedure of the background thread.
   /// No check is done for such precondition.
   ///
   virtual unsigned DoSendBackgroundBuffer(const MByteString& backgroundThreadBuffer);

   /// Background worker thread callable function which implements monitor communication.
   /// Note it is not currently virtual.
   ///
   /// \pre This shall be called by the worker polling thread,
   /// otherwise the behavior is undefined.
   ///
   void OnIdle() M_NO_THROW;

   /// Places the synchronization message to message queue. Internal service.
   ///
   void PostSyncMessage();

protected: // Data members:
/// \cond SHOW_INTERNAL

   // Mutual exclusion object to protect foreground buffer from accessing
   // it with two threads simultaneously.
   //
   MCriticalSection m_foregroundThreadBufferLock;

   // Mutual exclusion object to exclude the possibility for a file operation to be executed
   // from multiple threads at the same time. Note that m_foregroundThreadBufferLock and
   // m_fileLock are intentionally separated into two locks.
   //
   MCriticalSection m_fileLock;

   // Protected foreground send buffer, which is used by the foreground thread
   // to supply data to the background buffer. The background buffer uses
   // the foreground buffer to copy data to its own unprotected background buffer.
   //
   MByteString m_foregroundThreadBuffer;

   // Maximum file size in kilobytes, or zero if the file size is not restrained, in which case
   // it will grow indefinitely.
   //
   unsigned m_maxFileSizeKB;

   // File or directory name where the logging needs to be done.
   // This parameter is specified by the user.
   // If the directory is specified, the file name will be constructed from the attach string.
   //
   MStdString m_fileName;

   // Log file object, one used for logging.
   //
   MLogFileWriter* m_logFile;

   // internal flag indicating that a synchronization message has already been posted
   // set by PostSyncMessage() service
   //
   bool m_syncMessagePosted;

/// \endcond SHOW_INTERNAL
private: // Properties:

   // Set to true when the object is finalized, and can be destroyed.
   //
   bool m_isFinished;

   // Whether to obfuscate the file.
   //
   bool m_obfuscate;

   // Whether the file was deleted by this class, and needs to be recreated.
   //
   bool m_fileWasDeleted;

   M_DECLARE_CLASS(MonitorFile)
};

#endif // !M_NO_MCOM_MONITOR

///@}
#endif
