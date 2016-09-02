#ifndef MCORE_MDYNAMICLIBRARY_H
#define MCORE_MDYNAMICLIBRARY_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MDynamicLibrary.h

#include <MCORE/MCOREDefs.h>
#include <MCORE/MNonCopyable.h>

#if !M_NO_DYNAMIC_LIBRARY

/// Wrapper for shared object (dll) API
///
class M_CLASS MDynamicLibrary : private MNonCopyable
{
#if M_OS & M_OS_WIN32
   /// Internal platform-dependent constructor.
   /// Creates shared object's instance.
   ///
   MDynamicLibrary(HMODULE handle, const MStdString& name);
#else
   #error "MDynamicLibrary is not implemented for this operating system"
#endif

public: // Types:

   /// Generic procedure type
   ///
   /// \see GetProcedureAddress
   ///
   typedef void (*GlobalProcedureType)();

public:

   /// Destructor unloads the shared object from the process memory space
   /// (in fact, the behavior depends on OS: for instance Windows just
   /// decrements a reference counter for this DLL and unloads it only if
   /// the counter is equal to zero)
   ///
   ~MDynamicLibrary();

   /// Loads the specified MeteringSDK library into the address space of
   /// the current process.
   /// The Load function translates its parameter by appending
   /// the following information: _underline_ _vc[u][d]|cb_ .dll.
   ///
   static MDynamicLibrary* Load(const MStdString& name);

   /// Loads the specified shared object into the address space of the
   /// current process.
   ///
   static MDynamicLibrary* LoadExact(const MStdString& name, const MStdString& shortName);

   /// Make sure the given library is loaded
   ///
   /// \param name If library with this name is not already loaded it will be loaded.
   ///
   static void EnsureLibraryIsLoaded(const MStdString& name);

   /// Returns name of the shared object
   ///
   const MStdString& GetName() const
   {
      return m_name;
   }

   /// Returns system path to the shared object
   ///
   MStdString GetPath() const;

   /// Get the address of the global procedure within the dynamic library
   ///
   /// \pre Library shall be created, and procedure with such name shall exist,
   /// or an exception is thrown.
   ///
   GlobalProcedureType GetProcedureAddress(MConstChars procedureName);

private:
   // Name of shared object
   //
   MStdString m_name;

#if M_OS & M_OS_WIN32
   // platform-dependent handle
   HMODULE m_handle;
#else
   #error "MDynamicLibrary is not implemented for this operating system"
#endif
};

#endif // !M_NO_DYNAMIC_LIBRARY

///@}
#endif
