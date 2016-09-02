#ifndef MCORE_MPROGRESSMONITOR_H
#define MCORE_MPROGRESSMONITOR_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MProgressMonitor.h

#include "MCOREDefs.h"

#if !M_NO_PROGRESS_MONITOR

/// Listener interface that should be implemented in order to get updates from progress monitor.
///
/// Typical notification consists of one or more overloaded calls, followed by CommitChanges().
///
class M_CLASS MProgressListener : public MObject
{
public: // Types:

   /// Indicate progress changes that have to be performed by the client application.
   ///
   /// Several update types can be combined with bitwise OR operation.
   ///
   /// \see CommitChanges
   ///
   enum
   {
      FlagRefreshProgress         = 1,         ///< Progress (percent complete) has changed
      FlagRefreshActionMessage    = 2,         ///< Top level actions message change
      FlagRefreshSubActionMessage = 4,         ///< Current sub-action message change
      FlagRestoreSubActionMessage = 8,         ///< Restore parent sub-action message when destroying current sub-action
      FlagRefreshAll              = 0xFFFFFFFF ///< All refresh bits are on
   };

public: // Constructor and destructor:

   /// Construct the progress monitor listener.
   ///
   MProgressListener();

   /// Destroy the progress monitor listener.
   ///
   virtual ~MProgressListener();

public: // Properties:

   /// @{
   /// Client that supports monitor messages through reflection.
   ///
   /// When client is set and any of the overloaded methods are called,
   /// the correspondent client method will be called by name,
   /// if such method is available. 
   ///
   MObject* GetClient() const
   {
      return m_client;
   }
   void SetClient(MObject* client)
   {
      m_client = client;
   }
   /// @}

public: // Overloadable methods:

   /// Top-level root action message has changed.
   ///
   /// Typical implementations should update the GUI so the message is shown to the user.
   /// This particular default implementation uses reflection to call client method
   /// "OnActionMessageChange" if client is present and such method is available in it.
   ///
   /// \param message Message to report, can be an empty string
   ///
   virtual void OnActionMessageChange(const MStdString& message);

   /// The most recently created sub-action message has changed.
   ///
   /// Typical implementations should update the GUI so the message is shown to the user.
   /// This particular default implementation uses reflection to call client method
   /// "OnSubActionMessageChange" if client is present and such method is available in it.
   ///
   /// \param message Message to report, can be an empty string.
   /// \param restoringPrevious is true if the parent action message is being restored
   ///      while destroying one or more child actions. This means that the message
   ///      isn't really a new one, but the one restored from the action stack.
   ///      If restoringPrevious is true, MProgressAction::RestoreSubActionMessage update flag is set
   ///      in CommitChanges(), otherwise MProgressAction::RefreshSubActionMessage flag is set.
   ///
   virtual void OnSubActionMessageChange(const MStdString& message, bool restoringPrevious);

   /// Progress change notification is sent on any progress percentage update.
   ///
   /// Typical implementations should update the progress bar or any other sort of the GUI
   /// that is used to show progress.
   /// Notice that any progress advance on the most current action is immediately propagated
   /// by the action hierarchy, so that the root action's progress is always updated.
   ///
   /// This particular default implementation uses reflection to call client method
   /// "OnProgressChange" if client is present and such method is available in it.
   ///
   /// \param totalPercent refers to the root action percentage progress.
   /// \param subActionPercent refers to the most recently created sub-action progress.
   ///
   virtual void OnProgressChange(double totalPercent, double subActionPercent);

   /// Completes each series of updates.
   ///
   /// Listener implementation is expected to redraw its GUI at this point if there is any sort of
   /// action caching in the GUI.
   ///
   /// This particular default implementation uses reflection to call the client method
   /// "CommitChanges" if client is present and such method is available in it.
   ///
   /// \param flags has one or more bits set according to the following flags:
   ///   - MProgressAction::RefreshProgress         = 1 - Progress (% complete) has changed
   ///   - MProgressAction::RefreshActionMessage    = 2 - Top-level action's message change
   ///   - MProgressAction::RefreshSubActionMessage = 4 - Current sub-action's message change
   ///   - MProgressAction::RestoreSubActionMessage = 8 - Restore parent sub-action's message when destroying current sub-action
   ///
   virtual void CommitChanges(unsigned flags);

private: // Data:

   // Client object for reflected listeners
   //
   MObject* m_client;

   M_DECLARE_CLASS(ProgressListener)
};

/// Representation of task activity
///
/// A typical task comprises of a hierarchy of actions, such as in the following example:
/// \code
///    Root action
///       Action1
///       Action2
///          Action21
///          Action22
///       Action3
/// \endcode
/// Action objects belong to the progress monitor, and are freed automatically 
/// when the corresponding action is finished. An action is considered finished
/// whenever either of the following events occurs:
/// <ol>
/// <li> Complete() method is called </li>
/// <li> Any non-const method of any parent action is called 
///       (causing action stack "unwinding" up to that parent action) </li>
/// </ol>
/// The later method is handy when an exception thrown inside a child action is caught inside its parent.
/// Once an action is complete, the corresponding ProgressAction instance is destroyed, 
/// and hence should not be used any more. Notice that merely setting progress to 100% 
/// does not complete the action.
///
/// Each action contributes a certain amount to the overall progress of the parent action.
/// Each action has two main properties: 'message' and 'local progress'. 
/// These properties can change over time, such as:
/// \code
///    action.SetMessage("Initializing");
///    action.ReportProgress(10,"Connecting"); // progress + message
///    action.SetProgress(15);
/// \endcode
/// A typical task delegates a lot of processing to subroutines. In this case, each subroutine can
/// (optionally) have its own action object to report progress of the subroutine.
/// Here's how to create a child action (a.k.a. 'sub-action') to be used by a subroutine:
/// \code
///    action.SetProgress(20);
///    MProgressAction child = action.CreateChild(60);
///    Subroutine(child);
/// \endcode
/// In this example, we are saying that the parent action ('action') 
/// will be 60% complete by the time the child action is finished.
/// In other words, the child action will contribute 60%-20%=40% of 'action' progress.
/// In some cases, it is not possible (or not suitable) to pass a sub-action as a parameter to a subroutine.
/// In such cases, a reference to a child action can be passed via a static variable inside MProgressMonitor
/// (see MProgressMonitor::Set/GetLocalAction).
///
class M_CLASS MProgressAction : public MObject
{
   friend class MProgressMonitor;

public: // Types:
/// \cond SHOW_INTERNAL

      // Legacy enumeration for compatibility, use values from MProgressListener
      //
      enum ProgressUpdateFlags
      {
         REFRESH_PROGRESS       = 1,       // Progress (% complete) has changed
         REFRESH_ACTION_MSG     = 2,       // Top-level action's message change
         REFRESH_SUB_ACTION_MSG = 4,       // Current sub-action's message change
         RESTORE_SUB_ACTION_MSG = 8,       // Restoring parent sub-action's message when destroying current sub-action
         REFRESH_ALL            = 0xFFFF   // All REFRESH_* bits
      };

/// \endcond SHOW_INTERNAL
public: // reflected methods

   ///@{
   /// Action message or action name
   ///
   const MStdString& GetMessage() const
   {
      return m_message;
   }
   void SetMessage(const MStdString& message);
   ///@}

   ///@{
   /// Completion percentage of this action aka local progress.
   ///
   void SetProgress(double percent);
   double GetProgress() const;
   ///@}

   /// Set both the progress and the new message in a single call.
   ///
   /// This is a better way than setting the properties in sequence, as fewer GUI updates are involved.
   ///
   void ReportProgress(double percent, const MStdString& message);

   /// Create a sub-action.
   ///
   /// When the newly create sub action completes, 
   /// the parent action will be at the specified completion percent.
   /// The caller must NOT delete the returned action object - it is deleted automatically upon completion.
   ///
   MProgressAction* CreateChild(double parentPercentByCompletion);

   /// Create a child, and set it as a local action for the monitor.
   ///
   /// Used to pass a sub-action whenever direct passing of MProgressAction object reference 
   /// is not possible or desired.
   ///
   void CreateLocalAction(double parentPercentByCompletion);

   /// Complete this action.
   ///
   /// The action is no longer valid after this call.
   /// In fact, it is physically deleted, so any attempt to use it 
   /// will typically cause access violation.
   ///
   void Complete();

   /// Access to the progress monitor this action belongs to.
   ///
   /// Dummy action will return null.
   ///
   MProgressMonitor* GetOwner() const 
   { 
      return m_owner; 
   }

private:

   MProgressAction(MProgressMonitor&, double from, double to);
   MProgressAction(); // create a dummy action
   ~MProgressAction() {}

   double DoCalculateProgress(double) const;

private: // Disable copying:

   MProgressAction(const MProgressAction&); // cannot copy action objects
   MProgressAction& operator=(const MProgressAction&); // action assignment is not allowed

private: // Data:

   MProgressMonitor* m_owner; // dummy action does not have an owner
   double m_from;
   double m_weight;
   MStdString m_message;

   M_DECLARE_CLASS(ProgressAction)
};

/// ProgressMonitor manages a set of ProgressAction objects.
/// Typical usage scenario:
/// \code
///     // global declaration
///     MProgressMonitor m_mon;
///
///     void SomeLengthyMethod()
///     {
///        MProgressAction action = m_mon.CreateRootAction();
///        action.SetMessage("Initializing");
///        ... // do something
///        action.SetProgress(10); // 10%
///        Subroutine( action.CreateChild(50) ); // action will be 50% complete when child action is finished
///        // use CreateLocalAction() when child action action cannot be passed via parameters:
///        action.CreateLocalAction(80); 
///        AnotherSubroutine(); // subroutine will reclaim its action via MProgressMonitor::GetLocalAction()
///        ... // do something
///        action.ReportProgress(90,"Finishing"); // more efficient way of calling SetProgress() and SetMessage()
///        ... // do something
///        action.Complete();
///    }
/// \endcode
/// ProgressAction object is reflected, while ProgressMonitor is not.
/// However, CreateRootProgressAction() and GetLocalProgressAction() methods exist and are reflected.
///
class M_CLASS MProgressMonitor : public MObject
{
public: // Types:

   // Legacy class for compatibility, use MProgressListener
   //
   class Listener : public MProgressListener
   {
   };

public:

   /// Create progress monitor.
   ///
   /// \param listener Listener object that will be receiving monitoring events.
   ///
   MProgressMonitor(MProgressListener* listener = NULL);

   /// Destroy progress monitor.
   ///
   virtual ~MProgressMonitor();

   /// Purges any existing action hierarchy - start from scratch.
   ///
   /// The caller must NOT delete the returned action object.
   ///
   /// \return root progress action
   ///
   MProgressAction* CreateRootAction();

   /// Access to (the only) dummy action.
   ///
   /// Used when a progress monitor instance is not available.
   /// Dummy action implements all action methods as no-ops.
   /// dummy.CreateChild() will return the same (and the only) dummy instance.
   /// Dummy instance is returned from GetLocalAction() if no local action was set up.
   /// The caller must NOT delete the returned action object.
   ///
   static MProgressAction* GetDummyAction() 
   { 
      return &m_dummyAction; 
   }

   /// Set sub-action for lower level processing.
   /// This is an alternative to passing actions via parameters.
   /// The sub-action pointer is automatically reset after getting.
   void SetLocalAction(MProgressAction*);

   /// Returns a preset (or dummy) action and resets sub action pointer after returning.
   ///
   /// Subsequent calls will return dummy action until SetLocalAction() is called again.
   /// See also GetDummyAction(). The caller must NOT delete the returned action object.
   ///
   MProgressAction* GetLocalAction();

   ///@{
   /// Listener class associated with the progress monitor.
   ///
   /// This can be null to tell there is no associated listener.
   ///
   void SetListener(MProgressListener* listener)
   {
      m_listener = listener;
   }
   MProgressListener* GetListener()
   {
      return m_listener;
   }
   const MProgressListener* GetListenerConst() const
   {
      return m_listener;
   }
   ///@}

   /// Current top-level progress.
   ///
   double GetProgress() const 
   { 
      return m_progress; 
   }

   /// Direct access to the action stack.
   /// Action stack depth (size).
   int GetActionStackDepth() const 
   {
      return M_64_CAST(int, m_actions.size()); 
   }

   /// Access an action in the stack of actions.
   ///
   /// Current action is at zero depth, root action is at [stack size - 1].
   ///
   const MProgressAction* GetActionAt(int depth) const;

private: // Types:

   typedef std::vector<MProgressAction*> Actions;

private: // Methods:

   MProgressAction* CreateAction(double to);
   MProgressAction* PopAction();
   void DeleteAllActions();
   bool CheckActionStack(MProgressAction*, unsigned& flags);
   void UnwindActionStack(int depth);
   void RefreshActionMessage() const;
   void RefreshSubActionMessage(bool restoring) const;
   void RefreshProgress() const;
   void DoRefresh(unsigned flags = MProgressListener::FlagRefreshAll) const;
   bool IsRoot(MProgressAction* action) const 
   { 
      return !m_actions.empty() && m_actions[0] == action; 
   }
   void CompleteLocalAction();

private:

   MProgressMonitor(const MProgressMonitor&);
   MProgressMonitor& operator=(const MProgressMonitor&) const;

   static MProgressAction m_dummyAction;
   Actions m_actions;
   MProgressAction* m_localAction;

   // the fact that the local action is completed, used to complete it forcefully
   bool m_isLocalActionCompleted;

   MProgressListener* m_listener;
   double m_progress;
   friend class MProgressAction;
   M_DECLARE_CLASS(ProgressMonitor)
};

#endif

///@}
#endif
