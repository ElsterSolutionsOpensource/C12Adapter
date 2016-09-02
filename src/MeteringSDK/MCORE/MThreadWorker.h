#ifndef MCORE_MTHREADWORKER_H
#define MCORE_MTHREADWORKER_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MThreadWorker.h

#include <MCORE/MThread.h>
#include <MCORE/MCriticalSection.h>

#if !M_NO_MULTITHREADING

/// Abstract worker thread, the one optimized for doing work outside of the
/// currently running thread.
///
/// Every thread, child of this class will have to overload the function Run()
/// to specify what exactly the thread should be doing.
///
/// For every thread, standard C random number generator is seeded once.
/// On Windows, and when COM support is enabled, COM is initialized.
/// Also, there is a way of statically adding a user defined function
/// to be called at every thread creation, see SetThreadStartFunction().
///
class M_CLASS MThreadWorker : public MThread
{
public: // Types:

   /// Global user redefined function for thread execution.
   ///
   /// This is a hook that can be used in place of default StaticRun function 
   /// for executing any code that is specific to all threads created by the library. 
   /// As an example, it can be used to set a per thread crash handler.
   ///
   /// Typically, the user defined function will do some custom initialization
   /// and then call MThreadWorker::StaticRun, which is the default implementation.
   /// It is not recommended to completely replace StaticRun.
   ///
   typedef void (*StaticRunFunctionType)(MThreadWorker*);

protected: // Constructor:

   /// Worker thread constructor.
   /// The real worker threads will derive from MThreadWorker, this is why
   /// the constructor is protected.
   ///
   MThreadWorker();

   /// Destructor, destroys the thread object.
   ///
   /// \pre There should be no thread running (the thread function
   /// should be exited). Otherwise the behavior is undefined, as the function
   /// might want to continue using its worker thread object.
   ///
   virtual ~MThreadWorker();

public: // Thread client calls:

   /// Create and start the thread by execution of Run virtual function.
   /// This is a client thread call.
   ///
   /// \pre There is enough resources to create and start a thread,
   /// otherwise a system error is thrown.
   ///
   virtual void Start();

   /// A request of the thread client to wait until the thread finishes execution
   /// using the normal execution path. If the thread finished already,
   /// return immediately.
   ///
   /// If there was no thread created, or it was destroyed,
   /// WaitUntilFinished returns true immediately, a success.
   ///
   /// If throwIfError is true, the exception with which the thread
   /// is finished will be rethrown in the context of this thread.
   ///
   /// \pre This call should be made by the thread client,
   /// as it does not make any sense to be called by the worker thread itself.
   ///
#if M_OS & M_OS_UCLINUX
   bool WaitUntilFinished(bool throwIfError = true);
#else
   bool WaitUntilFinished(bool throwIfError = true, long timeout = -1);
#endif

   /// Tells if the thread finished execution. Note that by convention,
   /// if the thread is not started, it is also not finished.
   ///
   /// This service can be called by both the worker thread, and its client.
   ///
   /// \pre True returned if the thread finished running.
   ///
   bool IsFinished() const;

   /// Get the exception with which the thread was finished.
   /// If the thread was finished normally with return from Run, this is NULL.
   /// Note that the clients should not attempt to delete the returned exception.
   ///
   /// \pre IsFinished is true, otherwise the exception
   /// will notify about improper usage.
   ///
   MException* GetExitException();

   /// Get the constant return exception after the thread is finished.
   /// Note that the clients should not attempt to delete the returned exception.
   ///
   /// \pre IsFinished is true, otherwise the exception
   /// will notify about improper usage.
   ///
   const MException* GetExitException() const
   {
      return const_cast<MThreadWorker*>(this)->GetExitException();
   }

   ///@{
   /// Statically defined function to call at thread execution.
   ///
   /// By default, it is MThreadWorker::StaticRun.
   ///
   /// Having it global and static allows applications to employ a hook
   /// that alters all threads created by MeteringSDK, which is a convenience
   /// for cases such as installing per thread crash handlers.
   ///
   /// There is no synchronization available for getting and setting of this function,
   /// therefore, the best place to call this method is prior to creation of any thread,
   /// such as the first few lines of main function.
   ///
   /// Typically, the user defined function will do some custom initialization
   /// and then call MThreadWorker::StaticRun, the default implementation.
   /// It is not recommended to completely replace StaticRun with the custom code.
   ///
   static StaticRunFunctionType GetStaticRunFunction()
   {
      return s_staticRunFunction;
   }
   static void SetStaticRunFunction(StaticRunFunctionType func)
   {
      s_staticRunFunction = func;
   }
   ///@}

   /// Static runner of the thread that is called for thread execution.
   ///
   /// There is a way of overriding this function globally by calling
   /// SetCustomStaticRunFunction(). The custom call will typically eventually call 
   /// StaticRun.
   ///
   /// \param thread  Pointer to thread object, self.
   ///
   static void StaticRun(MThreadWorker* thread);

   /// Worker thread abstract running function.
   ///
   /// User shall redefine this method to perform desired actions in a separate thread.
   /// This is called from StaticRun, or from a custom user defined global thread function
   /// in order to perform actions specific to thread.
   ///
   virtual void Run() = 0;

private: // Methods:

   // Operating system dependent thread implementation function
   //
#if M_OS & M_OS_WIN32
   static unsigned __stdcall DoThreadRoutineCallback(MThreadWorker* thread);
#elif M_OS & M_OS_POSIX
   static void* DoThreadRoutineCallback(MThreadWorker* thread);
#endif

private: // Attributes:

   // Universal lock for the services of the thread.
   // Used in the implementation of the thread.
   //
   mutable MCriticalSection m_threadLock;

   // The exception with which the thread was exited, NULL if the thread did not
   // exit, or exited with no exception.
   //
   MUniquePtr<MException> m_exitException;
   
   // Global function that is to be called to process the thread actions
   //
   static StaticRunFunctionType s_staticRunFunction;
};

#endif // !M_NO_MULTITHREADING

///@}
#endif
