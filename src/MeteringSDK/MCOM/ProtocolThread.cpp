// File MCOM/ProtocolThread.cpp

#include "MCOMExtern.h"
#include "ProtocolThread.h"
#include "ProtocolC1222.h"
#include "MCOMExceptions.h"

#if !M_NO_MCOM_PROTOCOL_THREAD

MProtocolThread::MProtocolThread(MProtocol* client)
:
   MThreadWorker(),
   m_client(client)
{
}

MProtocolThread::~MProtocolThread()
{
}

void MProtocolThread::Run()
{
   m_client->DoQCommit();
}

#endif // !M_NO_MCOM_PROTOCOL_THREAD
