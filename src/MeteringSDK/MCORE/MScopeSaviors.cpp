// File MCORE/MScopeSaviors.cpp

#include "MCOREExtern.h"
#include "MScopeSaviors.h"
#include "MException.h"

#if !M_NO_REFLECTION

MObjectPropertySavior::MObjectPropertySavior(MObject* obj, const MStdString& propertyName)
:
   m_object(obj),
   m_propertyName(propertyName),
   m_propertyValue(obj->GetProperty(propertyName)) // have to perform it in the initialization list or else the destructor will execute in case of failure
{
}

MObjectPropertySavior::MObjectPropertySavior(MObject* obj, const MStdString& propertyName, const MVariant& value)
:
   m_object(obj),
   m_propertyName(propertyName),
   m_propertyValue(obj->GetProperty(propertyName)) // have to perform it in the initialization list or else the destructor will execute in case of failure
{
   m_object->SetProperty(m_propertyName, value);
}

MObjectPropertySavior::~MObjectPropertySavior() M_NO_THROW
{
   try
   {
      m_object->SetProperty(m_propertyName, m_propertyValue);
   }
   catch ( MException& ex )
   {
      M_USED_VARIABLE(ex);
      M_ASSERT(0); // notify in debug
   }
}

#endif // !M_NO_REFLECTION
