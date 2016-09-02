#ifndef MCORE_MDICTIONARY_H
#define MCORE_MDICTIONARY_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MDictionary.h

#include <MCORE/MCOREDefs.h>
#include <MCORE/MException.h>
#include <MCORE/MVariant.h>
#include <MCORE/MObject.h>

#if !M_NO_VARIANT

/// Dictionary of keys and their values, associative collection.
///
/// This is how a dictionary represents itself within a string:
/// \code
///       dictionary :== [ item1 [ ';' itemN ]+ ]?
///       item :== assignment | configuration-name
///       assignment :== key '=' value
/// \endcode
///
/// This is how a dictionary represents itself within a J command (given as an example):
/// \code
///       dictionary :== 'J00' item1 [itemN]+
///       item :== '[' key ':' value ']'
/// \endcode
/// In case of a J command, value can be anything but the closing square brace.
///
class M_CLASS MDictionary : public MObject
{
public: // Constructors and destructor:

   /// Object constructor, creates an empty dictionary.
   ///
   MDictionary();

   /// Object constructor that accepts the list of values.
   ///
   /// Initializes the dictionary by parsing the list of keys and values and
   /// looking in the configuration source for used configuration names.
   ///
   /// \pre The given list of key-value pairs is correct, or multiple exceptions are thrown.
   /// The syntax is described at the MDictionary description.
   /// Configuration location shall have a valid format, but it is not necessarily present.
   ///
   explicit MDictionary(const MStdString& properties);


   /// Object copy constructor, creates an equivalent dictionary.
   ///
   MDictionary(const MDictionary& other);

   /// Object destructor, removes allocated resources from memory
   ///
   virtual ~MDictionary();

public: // Services:

   /// Assignment operator
   ///
   MDictionary& operator=(const MDictionary& other)
   {
      if ( &other != this )
      {
         m_map = other.m_map;
      }
      return *this;
   }

   /// Return the number of entries in the dictionary.
   ///
   unsigned GetCount() const
   {
      return m_map.GetCount();
   }

   /// Access the map object of the dictionary.
   ///
   MVariant& GetMap()
   {
      return m_map;
   }

   /// Access the constant map object of the dictionary.
   ///
   const MVariant& GetMap() const
   {
      return m_map;
   }

   /// Make dictionary empty
   ///
   void Clear();

   /// Return value of the key specified. Throw exception if the key doesn't exist.
   ///
   /// \pre the key must be in the dictionary
   ///
   const MVariant& Item(const MVariant& key) const;

   /// Set (key, value) pair to the dictionary. If key doesn't exist, it is created.
   ///
   void SetItem(const MVariant& key, const MVariant& val);

   /// Get the value associated with the key.
   /// 
   /// \pre The key must exist in the dictionary, or an exception takes place.
   ///
   const MVariant& operator[](const MVariant& key) const
   {
      return Item(key);
   }

   /// Set or get the value associated with the key.
   /// If the key doesn't exist in the dictionary, it is created.
   /// The value of the key associated is created with default constructor.
   ///
   /// \pre Type associated with 'value' in the dictionary must have default constructor
   ///
   MVariant& operator[](const MVariant& key)
   {
      return m_map.AccessItem(key);
   }

   /// Return the list of all available keys.
   ///
   MVariant::VariantVector GetAllKeys() const;  

   /// Return the list of all available values.
   ///
   MVariant::VariantVector GetAllValues() const;            

   ///@{
   /// Access dictionary representation as string with key=value pairs.
   ///
   /// When the property is got, the string returned will have keys sorted.
   /// When the property is set, the string can have key=value pairs in any order.
   /// The string has the following format:
   /// \code
   ///      KEY1=value1;KEY2=value2;KEY3=value3;....
   /// \endcode
   /// Where KEY1, KEY2, ... are key names, value1, value2 ... are their values.
   /// In case the value is of string type, the correspondent
   /// value is enclosed in quotes, and it can have C-like escape sequences.
   ///
   /// \see AsStringUnsorted - returns key=value pairs in the order of their insertion
   ///
   MStdString AsString() const;
   void SetAsString(const MStdString& properties);
   ///@}

   /// Get the string with key=value pairs enumerated in the order of their insertion.
   ///
   /// The order of appearance of keys will be the order at which the keys
   /// were added into the dictionary.
   ///
   /// The string has the following format:
   /// \code
   ///      KEY2=value2;KEY1=value1;KEY3=value3;....
   /// \endcode
   /// Where KEY1, KEY2, ... are key names, value1, value2 ... are their values.
   /// In case the value is of string type, the correspondent
   /// value is enclosed in quotes, and it can have C-like escape sequences.
   ///
   /// \see AsString - returns key=value pairs sorted by key
   ///
   MStdString AsStringUnsorted() const;

   /// Return 'true' if the specified key exists.
   ///
   bool IsKeyPresent(const MVariant& key) const;
     
   /// Return 'true' if the specified value exist.
   ///
   bool IsValuePresent(const MVariant& val) const;

   /// Get a value associated with the key, or NULL if the value does not exist.
   ///
   MVariant* GetValue(const MVariant& key);

   /// const variant of the same function (see above)
   ///
   const MVariant* GetValue(const MVariant& key) const
   {
      return const_cast<MDictionary*>(this)->GetValue(key);
   }

   /// Removes key and value pair from the dictionary.
   ///
   /// \pre Key must exist or an exception is thrown
   ///
   void Remove(const MVariant& key);

   /// Removes key and value pair from the dictionary, if such key is present.
   /// Return true if key was actually removed.
   ///
   /// \pre Key must exist or an exception is thrown
   ///
   bool RemoveIfPresent(const MVariant& key);
  

   /// Merges the current dictionary with given one.
   ///
   void Merge(const MDictionary& dict);

   /// Reflection enabled object copy constructor.
   ///
   MDictionary* NewClone() const;

private: // Services:

   void DoAddKeysValues(const MStdString& values);


private: // Attributes:

   // Map of properties and their values
   //
   MVariant m_map;


   M_DECLARE_CLASS(Dictionary)
};

#endif // !M_NO_VARIANT

///@}
#endif
