// File MCORE/MTypeCasting.cpp

#include "MCOREExtern.h"

#include "MObject.h" // included instead of MTypeCasting.h
#include "MException.h"

M_FUNC M_NORETURN_FUNC void MDoThrowBadConversionUint64(const char* typeName)
{
   MException::Throw(MException::ErrorSoftware, M_CODE_STR_P1(MErrorEnum::BadConversion, M_I("Could not convert 64-bit unsigned integer to '%s'"), typeName));
   M_ENSURED_ASSERT(0);
}

M_FUNC M_NORETURN_FUNC void MDoThrowBadConversionInt64(const char* typeName)
{
   MException::Throw(MException::ErrorSoftware, M_CODE_STR_P1(MErrorEnum::BadConversion, M_I("Could not convert 64-bit signed integer to '%s'"), typeName));
   M_ENSURED_ASSERT(0);
}
