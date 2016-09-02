#ifndef MCORE_MAUTOMATION_H
#define MCORE_MAUTOMATION_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MAutomation.h

#include <MCORE/MCOREDefs.h>
#include <MCORE/MException.h>
#include <MCORE/MVariant.h>
#include <MCORE/MObject.h>
#include <MCORE/MTime.h>

#if !M_NO_AUTOMATION

#if defined(_MSC_VER) // Visual C++
   #include <atlbase.h>
#elif defined(__BORLANDC__) // Borland specific
   #include <atl/atlbase.h>
#endif

/// \cond SHOW_INTERNAL
#define MT_CONVERSION USES_CONVERSION;
#define MTToBStr(str)   A2BSTR(str)
#define MTToMChars(str) OLE2A(str)
#define MTToWStr(str)   A2W(str)
/// \endcond SHOW_INTERNAL

/// Wrapper class allows creating and using any Automation COM object.
///
/// As the class is Reflection-enabled, COM properties and methods
/// can be accessed exactly in the same way as native Reflection properties and methods.
///
/// This class exists only on Windows.
///
class M_CLASS MAutomation : public MObject
{
public:

   /// Initialize and uninitialize COM in constructor and destructor.
   /// Multiple objects of this class can be present in thread.
   ///
   /// This class is not reflected, and it is usable only from C++.
   ///
   class M_CLASS COMInitializer
   {
      bool m_initialized;
      bool m_shouldUninitialize;

   public:

      /// Constructor that initializes COM.
      ///
      /// \param delayInitialization
      ///    Whether to delay initialization to the moment when the first COM call is made.
      ///
      explicit COMInitializer(bool delayInitialization) M_NO_THROW;

      /// When constructor's delayInitialization is true, this call shall be made before any COM call
      /// not made through MAutomation class.
      ///
      void EnsureInitialized() M_NO_THROW;

      /// Destroy object and uninitialize COM.
      ///
      ~COMInitializer() M_NO_THROW;
   };

public: // Constructors:

   /// Constructor that takes the dispatch interface, used internally by CreateObject method.
   ///
   /// Use without CreateObject() shall be done with care, as there is no AddRef done here.
   ///
   /// \param dispatch
   ///    The dispatch interface to use. Shall be nonzero (checked only in debug mode).
   /// 
   MAutomation(IDispatch* dispatch)
   :
      m_dispatch(dispatch)
   {
      M_ASSERT(m_dispatch != NULL);
   }

   /// Copy constructor that creates a new dispatch interface.
   ///
   /// \param other
   ///    The Automation interface from which to create a new object.
   ///    There is a limited debug check present.
   /// 
   MAutomation(const MAutomation& other)
   :
      m_dispatch(other.m_dispatch)
   {
      M_ASSERT(m_dispatch != NULL);
      m_dispatch->AddRef(); // add a reference since we create a copy
   }

public: // Destructor:

   /// Destroy the object and decrement reference, as OLE requires.
   ///
   virtual ~MAutomation();

public: // Operators:

   /// Assignment operator that makes this automation object point to some other interface.
   ///
   /// \param other
   ///    The Automation interface from which to copy a new object.
   ///    There is a limited debug check present.
   /// 
   MAutomation& operator=(const MAutomation& other)
   {
      m_dispatch->Release();
      m_dispatch = other.m_dispatch;
      M_ASSERT(m_dispatch != NULL);
      m_dispatch->AddRef(); // add a reference since we create a copy
      return *this;
   }

public: // Static methods:

   /// Globally define Meter Objects Category IID.
   ///
   static CATID CATID_MeterObjects;

   /// Public creator of the automation object.
   ///
   /// \param objectNameOrGUID
   ///    Name of the Automation object such as "Scripting.FileSystemObject",
   ///    or a GUID string such as "{ABB4186F-9130-11D3-8BD6-005004058322}".
   ///    The GUID shall be wrapped into curly braces.
   ///    The object shall exist in the system, or an exception is thrown.
   ///
   /// \return The object wrapper, which can further be used just as regular Reflection-enabled class.
   ///
   static MAutomation* CreateObject(const MStdString& objectNameOrGUID);

   /// Register meter objects component category, as required by COM Registration.
   ///
   /// This call is necessary to make from facades that expose the function of the
   /// library to COM clients.
   ///
   static void RegisterMeterObjectsComponentCategory();

   /// Convert the given buffer and length into a safe array,
   /// handled by COM Automation interface.
   ///
   /// \param data
   ///    Data buffer to convert. There is a debug check whether the data is not NULL.
   ///
   /// \param size
   ///    Size of the data buffer.
   ///
   /// \return
   ///    COM array object is created and returned, the elements are variants that have bytes.
   ///
   /// \see ToSafeArray(const MByteString&)
   /// \see ToByteString(const SAFEARRAY*)
   ///
   static SAFEARRAY* ToSafeArray(const char* data, unsigned size);

   /// Convert the given byte string into a safe array,
   /// handled by COM Automation interface.
   ///
   /// \param data
   ///    Data array to convert.
   ///
   /// \return
   ///    COM array object is created and returned, the elements are variants that have bytes.
   ///
   /// \see ToSafeArray(const char* data, unsigned size)
   /// \see ToByteString(const SAFEARRAY*)
   ///
   static SAFEARRAY* ToSafeArray(const MByteString& data)
   {
      return ToSafeArray(data.c_str(), M_64_CAST(unsigned, data.size()));
   }

   /// Convert the given COM safe array into a byte string.
   ///
   /// \param safeArray
   ///    Safe array to convert. There is a debug check to make sure the given safe array is not zero.
   ///    The elements of the array should be bytes.
   ///
   /// \return byte string to represent the safe array.
   ///
   static MByteString ToByteString(const SAFEARRAY* safeArray);

   /// Convert COM BSTR into a byte string using MeteringSDK convention.
   ///
   /// \param b
   ///    BSTR string to convert, shall not be NULL and shall only have unicode characters in range 0 .. 255.
   ///
   /// \return Result byte string
   ///
   static MByteString ToByteString(const BSTR b);

   /// Convert COM VARIANT into a byte string using MeteringSDK convention.
   ///
   /// \param b
   ///    VARIANT of type byte array, variant array, or BSTR.
   ///
   /// \return Result byte string
   ///
   static MByteString ToByteString(const VARIANT* b);

   /// Convert the given standard string into COM BSTR.
   ///
   /// \param str
   ///    String to convert into BSTR
   ///
   /// \return Result BSTR, UNICODE string
   ///
   static BSTR ToBSTR(const MStdString& str);

   /// Convert the given byte string into COM BSTR according to MeteringSDK rules for Byte String.
   ///
   /// \param str
   ///    String to convert into Byte String BSTR
   ///
   /// \return Result BSTR, string that has byte values in range 0 .. 255
   ///
   static BSTR ToByteStringBSTR(const MByteString& str);

   /// Convert COM BSTR into a string.
   ///
   /// \param b
   ///    BSTR string to convert, shall not be NULL.
   ///
   /// \return Result string
   ///
   static MStdString ToStdString(const BSTR b);

   /// Convert a boolean into COM boolean type.
   ///
   /// \param value
   ///    Boolean value to convert.
   ///
   /// \return Result VARIANT_BOOL value, either VARIANT_TRUE or VARIANT_FALSE.
   ///
   static VARIANT_BOOL ToOleBool(bool value)
   {
      return value ? VARIANT_TRUE : VARIANT_FALSE;
   }

   /// Convert COM boolean into C++ bool value.
   ///
   /// \param value
   ///    COM Boolean value to convert.
   ///
   /// \return VARIANT_FALSE converts into false, and anything else is true.
   ///
   static bool ToBool(VARIANT_BOOL value)
   {
      return (value != VARIANT_FALSE);
   }

   /// Convert COM DATE into MTime or MTimeSpan.
   ///
   /// COM DATE type is just a double precision value.
   ///
   /// \param date
   ///    The date parameter, shall be valid, otherwise an MTime::IsNull time is returned.
   ///    MTime::IsNull will also result if COM DATE is 0.0.
   ///
   /// \return Variant type is returned that contains either MTime or MTimeSpan.
   ///
   static MVariant ToTimeOrTimeSpan(DATE date);

   /// Convert MTime into COM DATE object.
   ///
   /// COM DATE type is just a double precision value.
   ///
   /// \param time
   ///    The time and date to convert. If the value is MTime::IsNull, COM DATE is 0.0.
   ///
   /// \return COM DATE is returned that corresponds to parameter.
   ///
   static DATE ToOleDate(const MTime& time);

   ///@{
   /// Convert a safe array into a variant.
   ///
   /// The specified variant type may have VT_ARRAY bit set (it is ignored).
   ///  - For byte arrays, MByteString variant is returned.
   ///  - For BSTR arrays, MStdStringCollection variant is returned.
   ///  - For the other types, MVariant::VariantVector variant is returned.
   ///
   /// In Windows CE, there is no way to determine SAFEARRAY's data type on CE,
   /// this is why CE version has only one parameter.
   ///
   /// \pre The safe array shall be of the supported type
   /// with one dimension. Otherwise an exception is thrown.
   ///
   static MVariant ToMVariant(const SAFEARRAY*, VARTYPE vt);
#if !(M_OS & M_OS_WIN32_CE) 
   static MVariant ToMVariant(const SAFEARRAY*);
#endif
   ///@}

   /// Convert the COM/OLE variant into MeteringSDK MVariant type.
   ///
   /// The call tries to preserve type information as much as possible,
   /// however the conversions are done.
   ///
   /// \param value
   ///    Value to convert. The variant has to be of Automation-compatible types,
   ///    otherwise an exception is thrown.
   ///
   /// \return MVariant to represent the given COM VARIANT.
   ///
   static MVariant ToMVariant(const VARIANT& value);

   /// Convert MeteringSDK MVariant into COM/OLE variant type.
   ///
   /// The call tries to preserve type information as much as possible,
   /// however the conversions are done.
   ///
   /// \param value
   ///    Value to convert. The variant has to be of Automation-compatible types,
   ///    otherwise an exception is thrown.
   ///
   /// \param treatByteStringAsString
   ///    If the given variant is a byte string, this boolean value tells if it has
   ///    to be converted into a COM UNICODE string where each character will be within 
   ///    range 0 .. 255 to represent a byte.
   ///    Default value for this parameter is false, do not convert byte string into UNICODE string.
   ///
   static CComVariant ToOleVariant(const MVariant& value, bool treatByteStringAsString = false);

   /// Check if the parameter is given, defined.
   ///
   /// \pre If passed object is null then an exception will be thrown.
   ///
   static void CheckParameterExists(void* o, MConstChars argumentName);

public: // Properties:

   /// Access the native dispatch interface of the automation object.
   ///
   IDispatch* GetDispatch() const
   {
      return m_dispatch;
   }

   /// Program ID, reconstructed from the automation interface.
   ///
   /// The call is always successful, but it can return a default value "Automation" 
   /// if something is wrong with the registration of the object.
   ///
   MStdString GetProgId() const;

public: // Object overloads:

   /// Overloaded to provide access to COM properties.
   ///
   virtual MVariant GetProperty(const MStdString& name) const;

   /// Overloaded to allow changing COM properties.
   ///
   virtual void SetProperty(const MStdString& name, const MVariant& value);
   
   /// Overloaded to provide implementation of all Call methods.
   ///   
   virtual MVariant CallV(const MStdString& name, const MVariant::VariantVector& params);

   /// Tell if the service with the given name exists in the Automation interface.
   /// Due to OLE restriction, the service name is indistinguishable from property name.
   /// This way, the given name could actually be a property.
   ///
   virtual bool IsServicePresent(const MStdString& name) const;

   /// Tell if the property with the given name exists in the Automation interface.
   /// Due to OLE restriction, the service name is indistinguishable from property name.
   /// This way, the given name could actually be a property.
   ///
   virtual bool IsPropertyPresent(const MStdString& name) const;

   /// Get proper property of the automation object.
   ///
   /// This service is helpful when MObject property has the same name as 
   /// the automation object property. 
   ///
   MVariant GetAutomationProperty(const MStdString& name) const;

   /// Set proper property of the automation object.
   ///
   /// This service is helpful when MObject property has the same name as 
   /// the automation object property. 
   ///
   void SetAutomationProperty(const MStdString& name, const MVariant& value);

private:

   // Return DISPID of the given method or property.
   //
   // \pre The dispId is a nonzero pointer, there is a debug check.
   // The given name is a name of the existing property or method.
   // Otherwise HRESULT is bad.
   //
   HRESULT DoGetDispId(const MStdString& name, DISPID* dispId) const;

   MVariant DoInvoke(const MStdString& name, DISPPARAMS& params, Muint16 invokeType) const;

private: // Data:

   // Dispatch interface that this wrapper represents
   //
   IDispatch* m_dispatch;

   M_DECLARE_CLASS(Automation)
};

#endif
///@}
#endif
