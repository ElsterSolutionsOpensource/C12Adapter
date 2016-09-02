#ifndef MCOM_MCOMOBJECT_H
#define MCOM_MCOMOBJECT_H
/// \addtogroup MCOM
///@{
/// \file MCOM/MCOMObject.h

#include <MCOM/MCOMDefs.h>

/// Root communication object that defines default property handling and
/// configuration location. It is also able to stream the persistent 
/// properties into string.
///
/// There are no new public properties defined within this class,
/// only those which are in MObject.
///
class MCOM_ABSTRACT_CLASS MCOMObject : public MObject
{
protected: // Constructor:

   /// Object constructor, protected as the class is abstract.
   ///
   MCOMObject() 
   :
      MObject()
      #if !M_NO_REFLECTION
         , m_configurationName()
      #endif
   {
   }

public: // Destructor and services:

#if !M_NO_MCOM_FACTORY
   /// Virtual abstract copy constructor, creates MCOM object.
   ///
   virtual MCOMObject* CreateClone() const;
#endif

   /// Object destructor.
   ///
   virtual ~MCOMObject()
   {
   }

#if !M_NO_REFLECTION

public: // Methods:


   /// Return the list of publicly available properties in MCOM syntax.
   /// In contrast with the original MCORE implementation, the property names 
   /// will all be uppercased and name separation will be done with underscores.
   /// For persistent properties, one can use GetAllPersistentPropertyNames.
   ///
   virtual MStdStringVector GetAllPropertyNames() const;

   /// Return the list of publicly available persistent properties in MCOM syntax.
   /// In contrast with the original MCORE implementation, the property names 
   /// will all be uppercased and name separation will be done with underscores.
   /// For all properties, persistent and dynamic, one can use GetAllPropertyNames.
   ///
   virtual MStdStringVector GetAllPersistentPropertyNames() const;

   /// Get the string with the list of persistent property names and their values.
   ///
   /// The string has the following format:
   /// \code
   ///      PROPERTY1=value1;PROPERTY2=value2;....
   /// \endcode
   /// Where PROPERTY1, PROPERTY2, ... are property names,
   /// value1, value2 ... are their values.
   /// In case the property is of string type, the correspondent
   /// value is enclosed in quotes, and it can have C-like escape sequences.
   ///
   /// \param onlyNondefaults
   ///      If true, only TYPE and non-default properties are returned.
   /// \param excludeSecurityRelated
   ///      If true, do not return values of PASSWORD, AUTHENTICATION_KEY and SECURITY_KEY
   ///      properties if these are present in the protocol.
   ///
   /// \return String representing the values of the properties.
   ///
   MStdString GetPersistentPropertyValues(bool onlyNondefaults = false, bool excludeSecurityRelated = false) const;

   /// Set the persistent properties for the object using the string with the following format:
   /// \code
   ///      PROPERTY1=value1;PROPERTY2=value2;....
   /// \endcode
   /// Where PROPERTY1, PROPERTY2, ... are property names,
   /// value1, value2 ... are their values.
   ///
   /// In case the property is of string type, the correspondent
   /// value should be enclosed in quotes, and it can have C-like escape sequences.
   ///
   /// The order for the properties is not significant. It is not required that
   /// all object properties are present. In case the property is not present,
   /// its value will not be changed.
   ///
   /// \pre The properties mentioned should exist for the object,
   /// and the values given to those properties should be valid.
   /// No properties in the list can be Get-only.
   /// Otherwise this service throws an appropriate exception.
   /// Only those properties are set to the object, which were mentioned
   /// before the offending property.
   ///
   void SetPersistentPropertyValues(const MStdString& values);

   /// Set the properties for the object using the property list object.
   ///
   /// The order for the properties is not significant. It is not required that
   /// all object properties are present. In case the property is not present,
   /// its value will not be changed.
   ///
   /// \pre The properties mentioned should exist for the object,
   /// and the values given to those properties should be valid.
   /// No properties in the list can be Get-only.
   /// Otherwise this service throws an appropriate exception.
   /// In case the exception is thrown, only those properties are set 
   /// to the object, which were mentioned before the offending property.
   ///
   void SetPropertyValues(const MDictionary& values);

   /// Synchronously write a message to the monitor, if it is connected.
   ///
   /// No error is thrown if the Monitor is not running or the Monitor log file is
   /// not accumulating.
   ///
   /// \param message The message to write to the Monitor and the Monitor log file.
   ///
   virtual void WriteToMonitor(const MStdString& message);

   /// Write all non-default values of protocol properties into monitor
   ///
   /// This is a convenience method for all types of troubleshooting.
   /// No errors are thrown. If there is no monitor connected, nothing is done.
   ///
   void WritePropertiesToMonitor();

public: // Semi-public helper:

   /// Get the string with the whole list of persistent property names and their values.
   ///
   /// Non-default values and security properties are all included.
   /// The string has the following format:
   /// \code
   ///      PROPERTY1=value1;PROPERTY2=value2;....
   /// \endcode
   /// Where PROPERTY1, PROPERTY2, ... are property names,
   /// value1, value2 ... are their values.
   /// In case the property is of string type, the correspondent
   /// value is enclosed in quotes, and it can have C-like escape sequences.
   ///
   /// \return String representing the values of the properties
   ///
   MStdString DoGetPersistentPropertyValues0() const;

   /// Get the string with the list of persistent property names and their values.
   ///
   /// Security properties are included.
   /// The string has the following format:
   /// \code
   ///      PROPERTY1=value1;PROPERTY2=value2;....
   /// \endcode
   /// Where PROPERTY1, PROPERTY2, ... are property names,
   /// value1, value2 ... are their values.
   /// In case the property is of string type, the correspondent
   /// value is enclosed in quotes, and it can have C-like escape sequences.
   ///
   /// \param onlyNondefaults
   ///      If true, only TYPE and non-default properties are returned.
   ///
   /// \return String representing the values of the properties.
   ///
   MStdString DoGetPersistentPropertyValues1(bool onlyNondefaults) const;

protected: // Methods:
/// \cond SHOW_INTERNAL

   // Make the MCOM property names out of the original object names.
   // Like if IntercharacterTimeout is in the list, INTERCHARACTER_TIMEOUT will be made.
   //
   static void DoMakeMCOMPropertyNames(MStdStringVector& vec);


public: // Data:

   /// String that is equivalent to "TYPE" and "Type".
   /// Used in may places in code to denote the Type property.
   ///
   static const MStdString s_typeString;
   static const MStdString s_typeCamelcaseString;

protected: // Attributes:

   // The name of the configuration, as requested by the user.
   //
   MStdString m_configurationName;

/// \endcond SHOW_INTERNAL
#endif // !M_NO_REFLECTION

   M_DECLARE_CLASS(COMObject)
};

///@}
#endif
