#ifndef MCORE_MREGISTRY_H
#define MCORE_MREGISTRY_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MRegistry.h

#include <MCORE/MObject.h>

#if !M_NO_REGISTRY

/// Provides the access to Windows registry, system dependent class.
///
/// Only string data types are supported for values, however MRegistry provides services
/// that allow handling them as integers. There is no check provided, and for example,
/// one can store 1.2 as string, and read it as a rounded integer, or vice versa.
///
/// It is possible to request read-only access to a registry.
///
/// \since MeteringSDK Version 6.4.0.4870.
///
class M_CLASS MRegistry : public MObject
{
public: // Types:

   /// Key type within the registry, determines the location of the registry hive
   ///
   enum PredefinedKeyEnum
   {
      /// Classes root, corresponds to HKEY_CLASSES_ROOT, if added mask 0x80000000
      ///
      KeyClassesRoot = 0,

      /// Current user, corresponds to HKEY_CURRENT_USER, if added mask 0x80000000
      ///
      KeyCurrentUser = 1,

      /// Local machine, corresponds to HKEY_LOCAL_MACHINE, if added mask 0x80000000
      ///
      KeyLocalMachine = 2,

      /// All users, corresponds to HKEY_USERS, if added mask 0x80000000
      ///
      KeyUsers = 3
   };

public: // Constructor, destructor:

   /// Creates an empty uninitialized registry access object.
   ///
   /// \see Open - the next logical step after registry access object is created
   ///
   MRegistry();

   ///@{
   /// Initializes the registry from location.
   ///
   /// It allows opening global versus user registry access object, as read-only or read-write.
   ///
   /// \param parentKey
   ///     Predefined key to use as parent, any of these enumeration values:
   ///       - \ref MRegistry::KeyClassesRoot
   ///       - \ref MRegistry::KeyCurrentUser
   ///       - \ref MRegistry::KeyLocalMachine
   ///       - \ref MRegistry::KeyUsers
   ///
   /// \param location
   ///     A string that defines the location within the registry, as described in class header.
   ///
   /// \param readonly
   ///     When true, the registry is opened in read-only mode.
   ///     Otherwise, and this is the default, the registry is open for both reading and writing.
   ///
   /// \pre The location is valid and the requested access is allowed, otherwise the exception is thrown.
   ///
   explicit MRegistry(PredefinedKeyEnum parentKey, const MStdString& location, bool readonly);
   explicit MRegistry(PredefinedKeyEnum parentKey, const char* location, bool readonly); // SWIG_HIDE due to the above prototype
   ///@}

   /// Creates a sub-entry of the given opened registry object.
   ///
   /// When there is already an open registry, and there is a need to access its
   /// "subdirectory", this method is both a convenience and performance improvement.
   /// The read-only flag is copied from the parent.
   ///
   /// \param parent
   ///    An open registry to use for creation of its child.
   ///
   /// \param subLocation
   ///    Location relative to parent where to open the result registry access object
   ///
   /// \pre The parent is open, the subLocation is a valid registry location, and the access is allowed,
   ///      so the process of opening the registry access object succeeds. Otherwise the exception is thrown.
   ///
   MRegistry(const MRegistry& parent, const MStdString& subLocation);

   /// Object destructor, closes the registry access object and frees the resources.
   ///
   virtual ~MRegistry() M_NO_THROW;

public: // Property accessors:

   /// Get the list of subkeys in the registry.
   ///
   /// \return the list of all subkeys in the currently open registry access object
   ///
   /// \pre The registry should be open or an exception is raised.
   ///      Also, storage-related exceptions can be thrown.
   ///
   /// \seeprop{GetAllValues, AllValues} - list all values
   ///
   MStdStringVector GetAllSubkeys() const;

   /// Get the list of value names in the registry access object.
   ///
   /// \return the list of all values in the currently open registry
   ///
   /// \pre The registry object should be open or an exception is raised.
   ///      Also, storage-related exceptions can be thrown.
   ///
   /// \seeprop{GetAllSubkeys, AllSubkeys} - list all subkeys
   ///
   MStdStringVector GetAllValues() const;

   /// Tells whether the registry is open.
   ///
   /// \see Open - open registry access object
   /// \see Close - close registry access object
   /// \see CheckIfOpen - throw an exception if registry access object is not open
   ///
   bool IsOpen() const
   {
      return m_key != 0;
   }

   /// Tells whether the registry access object is read-only.
   ///
   /// This property is not about the rights of the particular registry entry,
   /// but rather in which way the registry access object was opened. The same registry access object
   /// can be opened in either read-write or read-only mode, should the user have the appropriate rights.
   ///
   /// \return
   ///    Whether the open registry access object is one of these types:
   ///      - True means the registry access object is read-only, and no value changes can be done through it.
   ///      - False means the registry access object object can be used for both reading and writing.
   ///
   /// \pre The registry access object shall be open or an exception is raised.
   ///
   bool IsReadOnly() const;

public: // Services:

   /// Open the registry folder for the desired access.
   ///
   /// \param parentKey
   ///     Predefined key to use as parent, any of these enumeration values:
   ///       - \ref MRegistry::KeyClassesRoot
   ///       - \ref MRegistry::KeyCurrentUser
   ///       - \ref MRegistry::KeyLocalMachine
   ///       - \ref MRegistry::KeyUsers
   ///
   /// \param location
   ///     A string that defines the location within the registry, as described in class header.
   ///
   /// \param readonly
   ///     When true, the registry access object is opened in read-only mode.
   ///     Otherwise, and this is the default, the registry access object is open for both reading and writing.
   ///
   /// \pre The location is valid and the requested access is allowed, otherwise the exception is thrown.
   ///
   /// \see IsOpen - tell whether the registry access object is open
   /// \see Close - explicitly close registry access object
   ///
   void Open(PredefinedKeyEnum parentKey, const MStdString& location, bool readonly);

   /// Open a sub-entry of the given opened registry object.
   ///
   /// When there is already an open registry, and there is a need to access its
   /// "subdirectory", this method is both a convenience and performance improvement.
   /// The read-only flag is copied from the parent.
   ///
   /// \param parent
   ///    An open registry to use for creation of its child.
   ///
   /// \param subLocation
   ///    Location relative to parent where to open the result registry access object
   ///
   /// \pre The parent is open, the subLocation is a valid registry location, and the access is allowed,
   ///      so the process of opening the registry access object succeeds. Otherwise the exception is thrown.
   ///
   void OpenSubkey(const MRegistry& parent, const MStdString& subLocation);

   /// Make sure the changes are flushed into persistent storage and close the registry.
   ///
   /// If the registry access object was not open prior to this method, it will succeed by doing nothing.
   ///
   /// \see IsOpen - tell whether the registry access object is open
   ///
   void Close();

   /// Whether the key or value with such name is present in the open registry access object.
   ///
   /// \param keyOrValue
   ///    A subkey or value to find in the registry.
   ///
   /// \pre The registry access object is open, or an exception is thrown.
   ///
   /// \see IsSubkeyPresent - tell if such subkey is present
   /// \see IsValuePresent - tell if such value is present
   ///
   bool IsPresent(const MStdString& keyOrValue) const;

   /// Whether the key with such name is present in the open registry access object.
   ///
   /// \param key
   ///    A subkey to find in the registry.
   ///
   /// \pre The registry access object is open, or an exception is thrown.
   ///
   /// \see IsPresent - tell if such subkey or value is present
   /// \see IsValuePresent - tell if such value is present
   ///
   bool IsSubkeyPresent(const MStdString& key) const;

   /// Whether the value with such name is present in the open registry access object.
   ///
   /// The value shall not be prefixed by key, or it will not be found.
   ///
   /// \param valueName
   ///    A value to find in the registry.
   ///
   /// \pre The registry access object is open, or an exception is thrown.
   ///
   /// \see IsPresent - tell if such subkey or value is present
   /// \see IsSubkeyPresent - tell if such subkey is present
   ///
   bool IsValuePresent(const MStdString& valueName) const;

   /// Get the string representation of the value name, or default if value with such name does not exist.
   ///
   /// \param valueName
   ///    Name of the value in the open registry access object.
   ///
   /// \param defaultValue
   ///    What to return if there is no such value in registry.
   ///
   /// \pre Registry access object shall be open. Also, storage-related exceptions can be thrown.
   ///
   /// \see GetExistingString for variation that throws an exception in case the name does not exist.
   ///
   MStdString GetString(const MStdString& valueName, const MStdString& defaultValue) const;

   /// Get the string representation of the value associated with the value name given.
   ///
   /// \param valueName
   ///    Name of the value in the open registry access object.
   ///
   /// \pre Registry access object shall be open. Also, storage-related exceptions can be thrown.
   ///
   /// \see GetString for variation that allows specifying the default value for the case when the string does not exist.
   ///
   MStdString GetExistingString(const MStdString& valueName) const;

   /// Get the integer representation of the value name, or default if value with such name does not exist.
   ///
   /// \param valueName
   ///    Name of the value in the open registry access object.
   ///
   /// \param defaultValue
   ///    What to return if there is no such value in registry access object.
   ///
   /// \pre Registry access object shall be open. Also, storage-related exceptions can be thrown.
   ///
   /// \see GetExistingInteger for variation that throws an exception in case the name does not exist.
   ///
   int GetInteger(const MStdString& valueName, int defaultValue) const;

   /// Get the integer representation of the value associated with the value name given.
   ///
   /// \param valueName
   ///    Name of the value in the open registry access object.
   ///
   /// \pre Registry access object shall be open. Also, storage-related exceptions can be thrown.
   ///
   /// \see GetInteger for variation that allows specifying the default value for the case when the string does not exist.
   ///
   int GetExistingInteger(const MStdString& valueName) const;

   /// Get the binary representation of the value name, or default if value with such name does not exist.
   ///
   /// \param valueName
   ///    Name of the value in the open registry access object
   ///
   /// \param defaultValue
   ///    What to return if there is no such value in registry access object.
   ///
   /// \pre Registry access object shall be open. Also, storage-related exceptions can be thrown.
   ///
   /// \see GetExistingBinary for variation that throws an exception in case the name does not exist.
   ///
   MByteString GetBinary(const MStdString& valueName, const MByteString& defaultValue) const;

   /// Get the binary representation of the value associated with the value name given.
   ///
   /// \param valueName
   ///    Name of the value in the open registry access object.
   ///
   /// \pre Registry access object shall be open. Also, storage-related exceptions can be thrown.
   ///
   /// \see GetBinary for variation that allows specifying the default value for the case when the string does not exist.
   ///
   MByteString GetExistingBinary(const MStdString& valueName) const;

   /// Associate the given string value with the name.
   ///
   /// In case the registry access object item with such name does not exist,
   /// it is created, and the value is initialized to one given.
   ///
   /// \param valueName
   ///    Name of the value in the registry.
   ///
   /// \param value
   ///    New value for the given name.
   ///
   /// \pre Storage-related exceptions can be thrown, like if the attempt to set a read-only value is made.
   ///
   void SetString(const MStdString& valueName, const MStdString& value);

   /// Associate the given integer value with the name.
   ///
   /// In case the registry access object item with such name does not exist,
   /// it is created, and the value is initialized to one given.
   ///
   /// \param valueName
   ///    Name of the value in the registry.
   ///
   /// \param value
   ///    New value for the given name.
   ///
   /// \pre Storage-related exceptions can be thrown, like if the attempt to set a read-only value is made.
   ///
   void SetInteger(const MStdString& valueName, int value);

   /// Associate the given binary value with the name.
   ///
   /// In case the registry access object item with such name does not exist,
   /// it is created, and the value is initialized to one given.
   ///
   /// \param valueName
   ///    Name of the value in the registry.
   ///
   /// \param value
   ///    New value for the given name.
   ///
   /// \pre Storage-related exceptions can be thrown, like if the attempt to set a read-only value is made.
   ///
   void SetBinary(const MStdString& valueName, const MByteString& value);

   /// Remove either the key or the value with such name from the registry access object.
   ///
   /// \param keyOrValue
   ///     Name of the value or subkey to remove.
   ///
   /// \pre The value name or key name should exist, and the user should have rights to remove it from registry.
   ///
   void Remove(const MStdString& keyOrValue);

   /// Remove value with such name from the registry access object.
   ///
   /// \param valueName
   ///     Name of the value to remove.
   ///
   /// \pre The value name should exist, and the user should have rights to remove it from registry.
   ///
   void RemoveValue(const MStdString& valueName);

   /// Remove key with such name from the registry.
   ///
   /// \param key
   ///     Name of the key to remove.
   ///
   /// \pre The key name should exist, and the user should have rights to remove it from registry.
   ///
   void RemoveSubkey(const MStdString& key);

   /// Check if the registry access object is open, throw an exception otherwise.
   ///
   /// \see Open - open registry access object
   /// \see Close - close registry access object
   /// \see IsOpen - return a boolean instead of throwing an exception
   ///
   void CheckIfOpen() const;

private: // Services:

   // Implementation of open method
   //
   void DoOpenSubkey(HKEY parentKey, const MStdString& location, bool readonly);

   // Query if the string for existence of such key, and if it does, 
   // fill the value buffer and return.the length.
   // If there is no such item, -1 is returned.
   //
   int DoQueryValue(const MStdString& key, DWORD* type = NULL) const;

   MStdString DoGetString(const MStdString& key, const MStdString* defaultValue = NULL) const;
   int DoGetInteger(const MStdString& key, int* defaultValue = NULL) const;

private: // Hiding invalid operators and methods to pacify Coverity:

   MRegistry& operator=(const MRegistry&);

private: // Attributes:

   // Registry key for the settings
   //
   HKEY m_key;

   // Temporary buffer used for holding the value.
   // The buffer is stored in the class to minimize allocating/deallocating.
   // It can grow depending on the necessity, but it will never shrink -
   // will only be deleted when the class is deleted.
   //
   mutable char* m_valueBuffer;

   // Current temporary buffer size in bytes.
   // It can grow depending on the necessity.
   //
   mutable unsigned m_valueBufferLength;

   // Whether the registry is opened for read-only access
   //
   bool m_readonly;

   M_DECLARE_CLASS(Registry)
};

#endif // !M_NO_REGISTRY
///@}
#endif
