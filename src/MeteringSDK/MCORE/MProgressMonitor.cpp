#include "MCOREExtern.h"
#include "MException.h"
#include "MProgressMonitor.h"

#if !M_NO_PROGRESS_MONITOR

   #if !M_NO_REFLECTION

      static MProgressListener* DoNewListener0()
      {
         return M_NEW MProgressListener();
      }

      /// Create progress monitor with no listener associated
      ///
      static MProgressMonitor* DoNew0()
      {
         return M_NEW MProgressMonitor();
      }

      /// Create progress monitor with listener
      ///
      /// \param listener Listener object that will receive progress notifications
      ///
      static MProgressMonitor* DoNew1(MProgressListener* listener)
      {
         return M_NEW MProgressMonitor(M_DYNAMIC_CAST_WITH_THROW(MProgressListener, listener));
      }

   #endif

M_START_PROPERTIES(ProgressListener)
   M_CLASS_ENUMERATION_UINT(ProgressListener, FlagRefreshProgress)
   M_CLASS_ENUMERATION_UINT(ProgressListener, FlagRefreshActionMessage)
   M_CLASS_ENUMERATION_UINT(ProgressListener, FlagRefreshSubActionMessage)
   M_CLASS_ENUMERATION_UINT(ProgressListener, FlagRestoreSubActionMessage)
   M_CLASS_ENUMERATION_UINT(ProgressListener, FlagRefreshAll)
   M_OBJECT_PROPERTY_OBJECT(ProgressListener, Client)
M_START_METHODS(ProgressListener)
   M_CLASS_FRIEND_SERVICE(ProgressListener, New, DoNewListener0, ST_MObjectP_S)
M_END_CLASS(ProgressListener, Object)

MProgressListener::MProgressListener()
:
   m_client(NULL)
{
}

MProgressListener::~MProgressListener()
{
}

void MProgressListener::OnActionMessageChange(const MStdString& message)
{
   #if !M_NO_REFLECTION
      if ( m_client != NULL && m_client->IsServicePresent("OnActionMessageChange") )
         m_client->Call1("OnActionMessageChange", message);
   #endif
}

void MProgressListener::OnSubActionMessageChange(const MStdString& message, bool restoringPrevious)
{
   #if !M_NO_REFLECTION
      if ( m_client != NULL && m_client->IsServicePresent("OnSubActionMessageChange") )
         m_client->Call2("OnSubActionMessageChange", message, restoringPrevious);
   #endif
}

void MProgressListener::OnProgressChange(double totalPercent, double subActionPercent)
{
   #if !M_NO_REFLECTION
      if ( m_client != NULL && m_client->IsServicePresent("OnProgressChange") )
         m_client->Call2("OnProgressChange", totalPercent, subActionPercent);
   #endif
}

void MProgressListener::CommitChanges(unsigned flags)
{
   #if !M_NO_REFLECTION
      if ( m_client != NULL && m_client->IsServicePresent("CommitChanges") )
         m_client->Call1("CommitChanges", flags);
   #endif
}

MProgressAction MProgressMonitor::m_dummyAction;

M_START_PROPERTIES(ProgressMonitor)
   M_OBJECT_PROPERTY_OBJECT(ProgressMonitor, Listener)
   M_OBJECT_PROPERTY_OBJECT(ProgressMonitor, LocalAction)
M_START_METHODS(ProgressMonitor)
   M_CLASS_FRIEND_SERVICE_OVERLOADED (ProgressMonitor, New, DoNew0, 0,       ST_MObjectP_S)
   M_CLASS_FRIEND_SERVICE_OVERLOADED (ProgressMonitor, New, DoNew1, 1,       ST_MObjectP_S_MObjectP)
   M_OBJECT_SERVICE                  (ProgressMonitor, CreateRootAction,     ST_MObjectP_X)
M_END_CLASS(ProgressMonitor, Object)

MProgressMonitor::MProgressMonitor(MProgressListener* listener)
:
   m_localAction(&m_dummyAction),
   m_isLocalActionCompleted(true),
   m_listener(listener),
   m_progress(0.0)
{
}

MProgressMonitor::~MProgressMonitor()
{
   DeleteAllActions();
}

void MProgressMonitor::DeleteAllActions()
{
   unsigned total = M_64_CAST(unsigned, m_actions.size());
   for ( unsigned i = 0; i < total; ++i )
      delete m_actions[i];
   m_actions.clear();
   m_localAction = &m_dummyAction;
   m_progress = 0.0;
}

MProgressAction* MProgressMonitor::CreateRootAction()
{
   DeleteAllActions();
   DoRefresh();
   return CreateAction(100.0);
}

MProgressAction* MProgressMonitor::CreateAction(double to)
{
   MProgressAction* action = M_NEW MProgressAction(*this, m_progress, to);
   m_actions.push_back(action);
   // Note: we don't reset sub-action progress until sub-action name is assigned.
   // Until this happens, the parent sub-action's name and progress will be shown to the user.
   return action;
}

bool MProgressMonitor::CheckActionStack(MProgressAction* action, unsigned& flags)
{
   int lastIndex = M_64_CAST(int, m_actions.size()) - 1;
   for ( int i = lastIndex; i >= 0 ; --i )
      if ( m_actions[i] == action )
      {
         if ( i < lastIndex )
         {
            UnwindActionStack(lastIndex-i);
            flags |= MProgressListener::FlagRestoreSubActionMessage;
               // notice that since the root action can never be unwound, there's no need to refresh its name
         }
         return true;
      }
   M_ASSERT(false); // invalid progress action
   return false;
}

void MProgressMonitor::UnwindActionStack(int depth)
{
   for ( int i = 0; i < depth; ++i )
      delete PopAction();
}

// pop action stack
MProgressAction* MProgressMonitor::PopAction()
{
   M_ASSERT(!m_actions.empty());
   
   Actions::iterator last = m_actions.end() - 1;
   MProgressAction* action = *last;
   m_actions.erase(last);

   M_ASSERT(action->m_owner == this); // action does not belong to this monitor?
   if ( m_localAction == action )
      m_localAction = &m_dummyAction;

   return action;
}

void MProgressMonitor::SetLocalAction(MProgressAction* action)
{
   M_ASSERT(action != NULL);
   if ( action->m_owner == this && action != m_localAction )
   {
      CompleteLocalAction();
      m_localAction = action;
      m_isLocalActionCompleted = false;
   }
   else
      M_ASSERT(false); // invalid action, ignore in release
}

MProgressAction* MProgressMonitor::GetLocalAction()
{
   MProgressAction* action = m_localAction;
   m_localAction = &m_dummyAction;
   return action;
}

void MProgressMonitor::RefreshActionMessage() const
{
   if ( m_listener != NULL )
   {
      if ( m_actions.empty() )
         m_listener->OnActionMessageChange(MVariant::s_emptyString);
      else
         m_listener->OnActionMessageChange(m_actions[0]->GetMessage());
   }
}

void MProgressMonitor::RefreshSubActionMessage(bool restoring) const
{
   if ( m_listener != NULL )
   {
      Actions::size_type count = m_actions.size();
      if ( count > 1 )
         m_listener->OnSubActionMessageChange(m_actions[count - 1]->GetMessage(), restoring);
      else
         m_listener->OnSubActionMessageChange(MVariant::s_emptyString, restoring);
   }
}

void MProgressMonitor::RefreshProgress() const
{
   if ( m_listener != NULL )
   {
      Actions::size_type count = m_actions.size();
      double subprogress = count > 1 ? m_actions[count - 1]->GetProgress() : 0.0;
      m_listener->OnProgressChange(m_progress, subprogress);
   }
}

void MProgressMonitor::DoRefresh(unsigned flags) const
{
   if ( flags && m_listener != NULL )
   {
      if ( flags & MProgressListener::FlagRefreshProgress )
         RefreshProgress();
      if ( flags & MProgressListener::FlagRefreshActionMessage )
         RefreshActionMessage();
      if ( flags & MProgressListener::FlagRefreshSubActionMessage )
         RefreshSubActionMessage(false);
      else if ( flags & MProgressListener::FlagRestoreSubActionMessage )
         RefreshSubActionMessage(true);
      if ( flags & MProgressListener::FlagRefreshSubActionMessage )
         flags &= ~MProgressListener::FlagRestoreSubActionMessage; // restore is irrelevant if new message is being set
      m_listener->CommitChanges(flags);
   }
}

void MProgressMonitor::CompleteLocalAction()
{
   if ( m_localAction != &m_dummyAction && !m_isLocalActionCompleted )
      m_localAction->Complete();
   m_localAction = &m_dummyAction;
   m_isLocalActionCompleted = true;
}

const MProgressAction* MProgressMonitor::GetActionAt(int depth) const
{
   M_ASSERT(depth >= 0);
   int at = M_64_CAST(int, m_actions.size()) - depth - 1;
   M_ASSERT(at >= 0);
   return m_actions[at];
}

M_START_PROPERTIES(ProgressAction)
   M_OBJECT_PROPERTY_STRING         (ProgressAction, Message, ST_constMStdStringA_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_DOUBLE         (ProgressAction, Progress)
   M_OBJECT_PROPERTY_READONLY_OBJECT(ProgressAction, Owner)
M_START_METHODS(ProgressAction)
   M_OBJECT_SERVICE                 (ProgressAction, ReportProgress,    ST_X_double_constMStdStringA)
   M_OBJECT_SERVICE                 (ProgressAction, CreateChild,       ST_MObjectP_X_double)
   M_OBJECT_SERVICE                 (ProgressAction, CreateLocalAction, ST_X_double)
   M_OBJECT_SERVICE                 (ProgressAction, Complete,          ST_X)
M_END_CLASS(ProgressAction, Object)

MProgressAction::MProgressAction(MProgressMonitor& owner, double from, double to)
:
   m_owner(&owner),
   m_from(from),
   m_weight((to-from)/100.0)
{
   M_ASSERT(to >= from && to <= 100.01); // invalid sub-action range (note: empty range is OK)
   if ( m_weight < 0.0 )
      m_weight = 0.0;
   if ( m_weight > 100.0 )
      m_weight = 100.0;
}

MProgressAction::MProgressAction()
:
   m_owner(0),
   m_from(0.0),
   m_weight(0.0)
{
}

double MProgressAction::GetProgress() const
{
   // calculate local progress based on global
   if ( m_owner && m_weight > 0.0 )
      return (m_owner->GetProgress() - m_from) / m_weight;
   else
      return 0.0;
}

double MProgressAction::DoCalculateProgress(double percent) const
{
   // calculate global progress based on action starting point and weight
   M_ASSERT(percent >= 0.0 && percent <= 100.01);
   if ( percent < 0.0 ) 
      percent = 0.0;
   if ( percent > 100.0 ) 
      percent = 100.0;
   return m_from + percent * m_weight;
}

void MProgressAction::SetMessage(const MStdString& message)
{
   unsigned flags = 0;
   if ( m_owner && m_owner->CheckActionStack(this, flags) )
   {
      bool isRoot = m_owner->IsRoot(this);
      flags |= isRoot ? MProgressListener::FlagRefreshActionMessage : MProgressListener::FlagRefreshSubActionMessage;
      if ( !isRoot && m_message.empty() )
         flags |= MProgressListener::FlagRefreshProgress; // we don't refresh sub action progress before its message is set,
                                                                // so let's do this now
      m_message = message;
      m_owner->DoRefresh(flags);
   }
}

void MProgressAction::SetProgress(double percent)
{
   unsigned flags = MProgressListener::FlagRefreshProgress;
   if ( m_owner && m_owner->CheckActionStack(this, flags) )
   {
      m_owner->m_progress = DoCalculateProgress(percent);
      m_owner->DoRefresh(flags);
   }
}

void MProgressAction::ReportProgress(double percent, const MStdString& message)
{
   unsigned flags = MProgressListener::FlagRefreshProgress;
   if ( m_owner && m_owner->CheckActionStack(this, flags) )
   {
      m_message = message;
      m_owner->m_progress = DoCalculateProgress(percent);
      flags |= m_owner->IsRoot(this) ? MProgressListener::FlagRefreshActionMessage : MProgressListener::FlagRefreshSubActionMessage;
      m_owner->DoRefresh(flags);
   }
}

MProgressAction* MProgressAction::CreateChild(double to)
{
   unsigned flags = 0;
   if ( m_owner && m_owner->CheckActionStack(this, flags) )
   {
      m_owner->DoRefresh(flags); // CreateAction doesn't need to refresh anything
      return m_owner->CreateAction(DoCalculateProgress(to));
   }
   else
      return this; // dummy action does not reproduce
}

void MProgressAction::CreateLocalAction(double to)
{
   if ( m_owner )
   {
      m_owner->CompleteLocalAction();
      m_owner->SetLocalAction(CreateChild(to));
   }
}

void MProgressAction::Complete() 
{ 
   unsigned flags = MProgressListener::FlagRefreshProgress;
   if ( m_owner && m_owner->CheckActionStack(this, flags) )
   {
      m_owner->m_progress = DoCalculateProgress(100.0);
      flags |= m_owner->IsRoot(this) ? MProgressListener::FlagRefreshActionMessage : MProgressListener::FlagRestoreSubActionMessage;
      m_owner->PopAction();
      m_owner->DoRefresh(flags);
      if ( this == m_owner->m_localAction )
         m_owner->m_isLocalActionCompleted = true;
      delete this;
   }
}

#endif // !M_NO_MCOM_PROGRESS_MONITOR
