// File MCORE/MTimeRecurrent.cpp

#include "MCOREExtern.h"
#include "MTimeRecurrent.h"
#include "MException.h"

#if !M_NO_TIME

M_START_PROPERTIES(TimeRecurrent)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT  (TimeRecurrent, IsValid)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT  (TimeRecurrent, IsNull)
M_START_METHODS(TimeRecurrent)
   M_OBJECT_SERVICE                       (TimeRecurrent, GetPertinent,   ST_MObjectByValue_X_MObjectP)
   M_OBJECT_SERVICE                       (TimeRecurrent, SetToNull,      ST_X)
   M_OBJECT_SERVICE                       (TimeRecurrent, CheckIsValid,   ST_X)
   M_OBJECT_SERVICE                       (TimeRecurrent, CheckIfNotNull, ST_X)
M_END_CLASS(TimeRecurrent, Object)

bool MTimeRecurrent::IsValid() const M_NO_THROW
{
   try
   {
      CheckIsValid();
   }
   catch ( ... )
   {
      return false;
   }
   return true;
}

void MTimeRecurrent::CheckIfNotNull() const
{
   if ( IsNull() )
   {
      MException::ThrowNoValue();
      M_ENSURED_ASSERT(0);
   }
}

#endif
