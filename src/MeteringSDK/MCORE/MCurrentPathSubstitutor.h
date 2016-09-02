#ifndef MCORE_MCURRENTPATHSUBSTITUTOR_H
#define MCORE_MCURRENTPATHSUBSTITUTOR_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MCurrentPathSubstitutor.h

#include <MCORE/MCOREDefs.h>

#if !M_NO_FILESYSTEM

/// Locally substitute the current path of an application to another current path.
///
/// The previous current path is restored in the destructor of the class.
/// Worth noting that this call shall be used with caution in a multithreaded
/// environment, as it replaces process current path. Therefore,
/// if some other thread makes the same call, or expects the current path to stay unchanged,
/// the behavior will be undefined.
///
/// This class would be used in a function that needs to temporarily set a different
/// current path, and might throw an exception. The algorithms of this class ensure
/// that the current application path will be restored whether or not an exception
/// is thrown, or a function ended peacefully. To make this class work, one needs
/// to declare a local variable of this class as a automatic stack-based variable, like:
/// \code
///        void MyFuncThatNeedsToSubstitutePath()
///        {
///            MCurrentPathSubstitutor substitutePath("c:\\new\\temporary\\path");
///            .... // this is where the path will be new
///        } // after the function returns in whatever way, the old path will be restored 
/// \endcode
/// Static service GetCurrentPath of MUtilities can be used at any time to
/// get the current path.
///
/// Note there is a popular C++ logic error not to mention the variable after the class.
/// Like in the example above, one could omit substitutePath without the compiler complaining.
/// However in this case the path will be substituted per only one line of code (up the semicolon), 
/// which is not an intent of this class. Also, one shall not attempt to copy or compare this class. 
/// Provisions through private services are made.
/// 
class M_CLASS MCurrentPathSubstitutor
{
public: // Constructor and destructor:

   /// Construct current path substitutor object, supply a new directory name, or a file name located in the required path.
   ///
   /// In case of failure, this constructor will not throw an exception,
   /// but property GetNewCurrentPath will be empty if path could not be changed.
   ///
   /// \param newPath
   ///     The new current directory name.
   ///     The directory should exist, and have appropriate permissions for the current user,
   ///     or the behavior is undefined, otherwise the path will not be changed.
   ///
   MCurrentPathSubstitutor(const MStdString& newPath) M_NO_THROW;

   /// Destroy the object, and restore the old default path which existed at the
   /// time of creation of the object.
   ///
   ~MCurrentPathSubstitutor() M_NO_THROW;

public: // Properties:

   /// Get the old application path which existed before the constructor of this class
   /// has attempted to set a new path.
   ///
   const MStdString& GetSavedCurrentPath() const
   {
      return m_savedCurrentPath;
   }

   /// Get the new application path, which was attempted to be set in the constructor of this class.
   ///
   ///  - If the path was replaced, this is the same as the file or directory given.
   ///  - If the path was not set for any reason, this will be an empty string.
   ///
   const MStdString& GetNewCurrentPath() const
   {
      return m_newCurrentPath;
   }

private: // Prevent certain operations on utilities:

   MCurrentPathSubstitutor(const MCurrentPathSubstitutor&);
   void operator=(const MCurrentPathSubstitutor&);
   bool operator==(const MCurrentPathSubstitutor&) const;
   bool operator!=(const MCurrentPathSubstitutor&) const;

private: // Data:

   // Saved current path, one which will be restored at destruction of an object
   //
   MStdString m_savedCurrentPath;

   // New current path, nonempty if the attempt to set it was successful
   //
   MStdString m_newCurrentPath;
};

#endif // !M_NO_FILESYSTEM

///@}
#endif 
