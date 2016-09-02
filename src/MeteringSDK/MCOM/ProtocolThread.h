#ifndef MCOM_PROTOCOLTHREAD_H
#define MCOM_PROTOCOLTHREAD_H
// File MCOM/ProtocolThread.h

#include <MCOM/Protocol.h>

#if !MCOM_PROJECT_COMPILING
   #error "This file is private to MCOM implementation, one shall not include it to client projects"
#endif

#if !M_NO_MCOM_PROTOCOL_THREAD
/// \cond SHOW_INTERNAL

// Thread associated with a protocol, helper class that executes the command queue in a background thread.
// The client protocol class is handled.
//
class MCOM_CLASS MProtocolThread : public MThreadWorker
{
   friend class MProtocol;

private: // Constructor:

   // Private constructor that creates a protocol thread.
   // Client protocol object is given as parameter.
   // The service is private because only MProtocol, which is a friend of this class can create it.
   //
   MProtocolThread(MProtocol* client);

public: // Destructor:

   // Destroy the thread object.
   //
   virtual ~MProtocolThread();

private: // Services:

   // Protocol worker thread running function.
   //
   virtual void Run();

private: // Attributes:

   // Client protocol object to do the job
   //
   MProtocol* m_client;
};

/// \endcond SHOW_INTERNAL
#endif
#endif
