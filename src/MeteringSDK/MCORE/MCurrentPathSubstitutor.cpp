// File MCORE/MCurrentPathSubstitutor.cpp

#include "MCOREExtern.h"
#include "MCurrentPathSubstitutor.h"
#include "MUtilities.h"

#if !M_NO_FILESYSTEM

MCurrentPathSubstitutor::MCurrentPathSubstitutor(const MStdString& newPath) M_NO_THROW
:
   m_savedCurrentPath(),
   m_newCurrentPath()
{
   try // try to set the path into one of the current stream path (needed for file stream operations)
   {
      m_savedCurrentPath = MUtilities::GetCurrentPath();
      MUtilities::SetCurrentPath(newPath);
      m_newCurrentPath = newPath; // success
   }
   catch ( ... )
   {
      M_ASSERT(m_newCurrentPath.empty());  // ignore current path error,  but ensure that we're not succeeding
   }
}

MCurrentPathSubstitutor::~MCurrentPathSubstitutor() M_NO_THROW
{
   if ( !m_savedCurrentPath.empty() && !m_newCurrentPath.empty() )
   {
      try // restore the current path
      {
         MUtilities::SetCurrentPath(m_savedCurrentPath);
      }
      catch ( ... )
      {
         // ignore current path error
      }
   }
}

#endif // !M_NO_FILESYSTEM
