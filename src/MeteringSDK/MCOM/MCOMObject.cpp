#include "MCOMExtern.h"
#include "MCOMObject.h"
#include "MCOMFactory.h"
#include "MCOMExceptions.h"
#include <MCORE/MDictionary.h>

M_START_PROPERTIES(COMObject)
M_START_METHODS(COMObject)
   M_OBJECT_SERVICE                 (COMObject, WriteToMonitor,                                                 ST_X_constMStdStringA)
   M_OBJECT_SERVICE                 (COMObject, WritePropertiesToMonitor,                                       ST_X)
   M_OBJECT_SERVICE_OVERLOADED      (COMObject, GetPersistentPropertyValues, GetPersistentPropertyValues,    2, ST_MStdString_X_bool_bool)
   M_OBJECT_SERVICE_OVERLOADED      (COMObject, GetPersistentPropertyValues, DoGetPersistentPropertyValues1, 1, ST_MStdString_X_bool) // SWIG_HIDE
   M_OBJECT_SERVICE_OVERLOADED      (COMObject, GetPersistentPropertyValues, DoGetPersistentPropertyValues0, 0, ST_MStdString_X)      // SWIG_HIDE
   M_OBJECT_SERVICE                 (COMObject, SetPersistentPropertyValues,                                    ST_X_constMStdStringA)
#if !M_NO_MCOM_FACTORY
   M_OBJECT_SERVICE                 (COMObject, CreateClone,                                                    ST_MObjectP_X)
#endif
M_END_CLASS_TYPED(COMObject, Object, "COM_OBJECT")

#if !M_NO_REFLECTION

const MStdString MCOMObject::s_typeString          = "TYPE";
const MStdString MCOMObject::s_typeCamelcaseString = "Type";


MStdStringVector MCOMObject::GetAllPropertyNames() const
{
   MStdStringVector vec = MObject::GetAllPropertyNames();
   DoMakeMCOMPropertyNames(vec);
   return vec;
}

MStdStringVector MCOMObject::GetAllPersistentPropertyNames() const
{
   MStdStringVector vec = MObject::GetAllPersistentPropertyNames();
   DoMakeMCOMPropertyNames(vec);
   return vec;
}

MStdString MCOMObject::DoGetPersistentPropertyValues0() const
{
   return GetPersistentPropertyValues();
}

MStdString MCOMObject::DoGetPersistentPropertyValues1(bool onlyNondefaults) const
{
   return GetPersistentPropertyValues(onlyNondefaults);
}

MStdString MCOMObject::GetPersistentPropertyValues(bool onlyNondefault, bool excludeSecurityRelated) const
{
   MStdString result;
   const MStdStringVector& names = GetAllPersistentPropertyNames();
   MStdStringVector::const_iterator it = names.begin();
   MStdStringVector::const_iterator itEnd = names.end();
   for ( ; it != itEnd; ++it )
   {
      const MStdString& name = *it;
      if ( excludeSecurityRelated && (name == "PASSWORD" || name == "AUTHENTICATION_KEY" || name == "SECURITY_KEY") )
         continue;
      const MVariant& val = GetProperty(name);
      if ( onlyNondefault && (name != MCOMObject::s_typeString && val == GetPersistentPropertyDefaultValue(name)) )
         continue;
      result += name;
      result += '=';
      result += MUtilities::ToRelaxedMDLConstant(val);
      result += ';';
   }
   return result;
}

void MCOMObject::SetPersistentPropertyValues(const MStdString& values)
{
   MDictionary propertyList(values);
   SetPropertyValues(propertyList);
}

void MCOMObject::SetPropertyValues(const MDictionary& propertyList)
{
   const MVariant::VariantVector& names = propertyList.GetAllKeys();
   for ( MVariant::VariantVector::const_iterator it = names.begin() ; it != names.end() ; ++it )
   {
      const MStdString& propertyName = it->AsString();
      SetProperty(propertyName, propertyList[propertyName].AsString());
   }
}

MCOMObject* MCOMObject::CreateClone() const
{
   MException::ThrowNotSupportedForThisType();
   return NULL; // pacify the compiler
}

#if !M_NO_MCOM_FACTORY
void MCOMObject::WriteToMonitor(const MStdString&)
{
   MException::ThrowNotSupportedForThisType();
}
#endif

void MCOMObject::WritePropertiesToMonitor()
{
   #if !M_NO_MCOM_MONITOR
      WriteToMonitor(GetPersistentPropertyValues(true, true));
   #endif
}

void MCOMObject::DoMakeMCOMPropertyNames(MStdStringVector& vec)
{
   MStdStringVector::iterator it = vec.begin();
   MStdStringVector::iterator itEnd = vec.end();
   for ( ; it != itEnd; ++it )
   {
      M_ASSERT(!(*it).empty());

      MStdString result(1, (*it)[0]); // first letter is copied without preceding underscore
      MStdString::const_iterator iit = (*it).begin() + 1; // start iterations from the second symbol
      MStdString::const_iterator iitEnd = (*it).end();
      for ( ; iit != iitEnd; ++iit )
      {
         if ( m_isupper(*iit) )
         {
            result += '_';
            result += *iit;
         }
         else
            result += (char)m_toupper(*iit);
      }
      (*it) = result;
   }
}

#endif
