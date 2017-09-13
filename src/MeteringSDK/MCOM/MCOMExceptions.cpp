// File MCOM/MCOMException.cpp

#include "MCOMExtern.h"
#include "MCOMExceptions.h"
#include <MCORE/MAlgorithm.h>

M_START_PROPERTIES(COMException)
M_START_METHODS(COMException)
   M_CLASS_SERVICE                     (COMException, New,                ST_MObjectP_S)
M_END_CLASS(COMException, Exception)

MCOMException::MCOMException()
:
   MException()
{
   m_kind = MException::ErrorCommunication;
}

#if !M_NO_VERBOSE_ERROR_INFORMATION

MCOMException::MCOMException(MErrorEnum::Type code, MConstLocalChars str, ...)
:
   MException()
{
   va_list va;
   va_start(va, str);
   InitVA(ErrorCommunication, code, str, va);
   va_end(va);
}

#else

MCOMException::MCOMException(MErrorEnum::Type code)
:
   MException()
{
   Init(ErrorCommunication, code);
}

#endif

MCOMException::MCOMException(const MCOMException& ex)
:
   MException(ex)
{
}

MCOMException::~MCOMException() M_NO_THROW
{
}

MCOMException* MCOMException::New()
{
   return M_NEW MCOMException;
}

#if !M_NO_VERBOSE_ERROR_INFORMATION

M_NORETURN_FUNC void MCOMException::Throw(MErrorEnum::Type code, MConstLocalChars str, ...)
{
   MCOMException ex;
   va_list va;
   va_start(va, str);
   ex.InitVA(ErrorCommunication, code, str, va);
   va_end(va);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MCOMException::Throw(MErrorEnum::Type code, const char* str, ...)
{
   MCOMException ex;
   va_list va;
   va_start(va, str);
   ex.InitVA(ErrorCommunication, code, str, va);
   va_end(va);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MCOMException::Throw(MException::KindType kind, MErrorEnum::Type code, MConstLocalChars str, ...)
{
   MCOMException ex;
   va_list va;
   va_start(va, str);
   ex.InitVA(kind, code, str, va);
   va_end(va);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MCOMException::Throw(MException::KindType kind, MErrorEnum::Type code, const char* str, ...)
{
   MCOMException ex;
   va_list va;
   va_start(va, str);
   ex.InitVA(kind, code, str, va);
   va_end(va);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

#else // !M_NO_VERBOSE_ERROR_INFORMATION

M_NORETURN_FUNC void MCOMException::Throw(MErrorEnum::Type code)
{
   MCOMException ex;
   ex.Init(ErrorCommunication, code);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MCOMException::Throw(MException::KindType kind, MErrorEnum::Type code)
{
   MCOMException ex;
   ex.Init(kind, code);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

#endif // !M_NO_VERBOSE_ERROR_INFORMATION

M_NORETURN_FUNC void MCOMException::ThrowInvalidOperationInForeground()
{
   MCOMException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_INVALID_OPERATION_DURING_ACTIVE_BACKGROUND_COMMUNICATION, "Invalid operation during active background communication"));
   M_ENSURED_ASSERT(0);
}

void MCOMException::CheckIfExpectedDataSizeDifferent(unsigned receivedLength, unsigned expectedLength)
{
   if ( receivedLength != expectedLength )
   {
      MCOMException::Throw(MException::ErrorMeter, M_CODE_STR_P2(MErrorEnum::ReceivedDataSizeDifferent, M_I("Expected %d bytes, but found %d, possibly the buffer has incorrect layout"), expectedLength, receivedLength));
      M_ENSURED_ASSERT(0);
   }
}

MException* MCOMException::NewClone() const
{
   return M_NEW MCOMException(*this);
}

M_NORETURN_FUNC void MCOMException::Rethrow()
{
   throw *this;
}

#if !M_NO_MCOM_PROTOCOL_C1218 || !M_NO_MCOM_PROTOCOL_C1221 || !M_NO_MCOM_PROTOCOL_C1222

M_START_PROPERTIES(EC12NokResponse)
   M_OBJECT_PROPERTY_INT               (EC12NokResponse, ResponseCode)
   M_OBJECT_PROPERTY_BYTE_STRING       (EC12NokResponse, ExtraParameters,       ST_constMByteStringA_X, ST_X_constMByteStringA)
M_START_METHODS(EC12NokResponse)
   M_CLASS_SERVICE                     (EC12NokResponse, New,                 ST_MObjectP_S)
   M_CLASS_SERVICE                     (EC12NokResponse, Throw,               ST_S_byte)
   M_CLASS_SERVICE                     (EC12NokResponse, ThrowWithParameters, ST_S_byte_constMByteStringA)
M_END_CLASS(EC12NokResponse, COMException)

MEC12NokResponse::MEC12NokResponse(Muint8 responseCode)
:
   MCOMException()
{
   Init(responseCode);
}

MEC12NokResponse::MEC12NokResponse(Muint8 responseCode, const MByteString& extraParameters)
:
   MCOMException()
{
   Init(responseCode);

   // adding extra parameters string
   if ( !extraParameters.empty() )
   {
      m_extraParameters = extraParameters;
#if !M_NO_VERBOSE_ERROR_INFORMATION
      static MConstLocalChars s_segmentOffsetSize = M_I(". Segment byte offset %d, APDU size %d");
      // in these cases we know the interpretation of parameters, insert the appropriate string
      switch ( responseCode )
      {
      case RESPONSE_RQTL:
      case RESPONSE_RSTL:
         if ( extraParameters.size() == 4 )
         {
            AppendToString(M_I(". Maximum possible size is %d"), MFromBigEndianUINT32(extraParameters.data()));
            return;
         }
         break;
      case RESPONSE_SGERR:
         switch ( extraParameters.size() )
         {
         case 2:
            AppendToString(s_segmentOffsetSize, (unsigned)(Muint8)extraParameters[0], (unsigned)(Muint8)extraParameters[1]);
            return;
         case 4:
            AppendToString(s_segmentOffsetSize, MFromBigEndianUINT16(extraParameters.data()), MFromBigEndianUINT16(extraParameters.data() + 2));
            return;
         case 6:
            AppendToString(s_segmentOffsetSize, MFromBigEndianUINT24(extraParameters.data()), MFromBigEndianUINT24(extraParameters.data() + 3));
            return;
         }
         break;
      }
      AppendToString(M_I(". Extra parameters: "));
      Append(MUtilities::BytesToHex(extraParameters, true));
#endif // !M_NO_VERBOSE_ERROR_INFORMATION
   }
}

MEC12NokResponse::MEC12NokResponse(const MEC12NokResponse& ex)
:
   MCOMException(ex),
   m_responseCode(ex.m_responseCode),
   m_extraParameters(ex.m_extraParameters)
{
}

MEC12NokResponse::~MEC12NokResponse() M_NO_THROW
{
}

MEC12NokResponse* MEC12NokResponse::New()
{
   return M_NEW MEC12NokResponse;
}

void MEC12NokResponse::Init(Muint8 responseCode)
{
   m_extraParameters.clear();
   m_responseCode = (ResponseCode)responseCode;
   MException::KindType kind = MException::ErrorCommunication;
   const MErrorEnum::Type errorCode = MErrorEnum::Type(MErrorEnum::C12ServiceResponseOK + unsigned(responseCode));
#if !M_NO_VERBOSE_ERROR_INFORMATION
   MConstLocalChars str;
   switch ( responseCode )
   {
   case RESPONSE_ERR:   str = M_I("0x%02X Error, no reason provided (ERR)");                                                            break;
   case RESPONSE_SNS:   str = M_I("0x%02X Service Not Supported (SNS)");                         kind = MException::ErrorMeter;         break;
   case RESPONSE_ISC:   str = M_I("0x%02X Insufficient Security Clearance (ISC)");               kind = MException::ErrorSecurity;      break;
   case RESPONSE_ONP:   str = M_I("0x%02X Operation Not Possible (ONP)");                        kind = MException::ErrorMeter;         break;
   case RESPONSE_IAR:   str = M_I("0x%02X Inappropriate Action Requested (IAR)");                kind = MException::ErrorSoftware;      break;
   case RESPONSE_BSY:   str = M_I("0x%02X Device Busy (BSY)");                                   kind = MException::ErrorMeter;         break;
   case RESPONSE_DNR:   str = M_I("0x%02X Data Not Ready (DNR)");                                kind = MException::ErrorMeter;         break;
   case RESPONSE_DLK:   str = M_I("0x%02X Data Locked (DLK)");                                   kind = MException::ErrorMeter;         break;
   case RESPONSE_RNO:   str = M_I("0x%02X Renegotiate (RNO)");                                   kind = MException::ErrorMeter;         break;
   case RESPONSE_ISSS:  str = M_I("0x%02X Invalid Service Sequence State (ISSS)");                                                      break;
   case RESPONSE_SME:   str = M_I("0x%02X Security mechanism error detected (SME)");             kind = MException::ErrorSecurity;      break;
   case RESPONSE_UAT:   str = M_I("0x%02X Unknown or invalid Called ApTitle is received (UAT)");                                        break;
   case RESPONSE_NETT:  str = M_I("0x%02X Network timeout detected (NETT)");                                                            break;
   case RESPONSE_NETR:  str = M_I("0x%02X Node is not reachable (NETR)");                                                               break;
   case RESPONSE_RQTL:  str = M_I("0x%02X Request too large (RQTL)");                            kind = MException::ErrorMeter;         break;  // For successfully restart service after changing protocol parameters
   case RESPONSE_RSTL:  str = M_I("0x%02X Response too large (RSTL)");                           kind = MException::ErrorMeter;         break;  // For successfully restart service after changing protocol parameters
   case RESPONSE_SGNP:  str = M_I("0x%02X Segmentation required, but not possible (SGNP)");                                             break;
   case RESPONSE_SGERR: str = M_I("0x%02X Segmentation error (SGERR)");                                                                 break;
   case RESPONSE_NRY:  str = M_I("0x%02X Not registered to you (NRY)");                                                               break;
   default:             str = M_I("0x%02X, Unknown response");                                                                          break;
   }
   MCOMException::Init(kind, errorCode, str, unsigned(responseCode));
#else
   switch ( responseCode )
   {
   case RESPONSE_SNS:   kind = MException::ErrorMeter;         break;
   case RESPONSE_ISC:   kind = MException::ErrorSecurity;      break;
   case RESPONSE_ONP:   kind = MException::ErrorMeter;         break;
   case RESPONSE_IAR:   kind = MException::ErrorSoftware;      break;
   case RESPONSE_BSY:   kind = MException::ErrorMeter;         break;
   case RESPONSE_DNR:   kind = MException::ErrorMeter;         break;
   case RESPONSE_DLK:   kind = MException::ErrorMeter;         break;
   case RESPONSE_RNO:   kind = MException::ErrorMeter;         break;
   case RESPONSE_SME:   kind = MException::ErrorSecurity;      break;
   case RESPONSE_RQTL:  kind = MException::ErrorMeter;         break;
   case RESPONSE_RSTL:  kind = MException::ErrorMeter;         break;
   default: ; // have kind intact
   }
   MCOMException::Init(kind, errorCode);
#endif
}

MException* MEC12NokResponse::NewClone() const
{
   return M_NEW MEC12NokResponse(*this);
}

M_NORETURN_FUNC void MEC12NokResponse::Throw(Muint8 responseCode)
{
   MEC12NokResponse(responseCode).Rethrow();
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MEC12NokResponse::ThrowWithParameters(Muint8 responseCode, const MByteString& extraParameters)
{
   MEC12NokResponse(responseCode, extraParameters).Rethrow();
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MEC12NokResponse::Rethrow()
{
   throw *this;
}

M_START_PROPERTIES(EC12BadProcedureResult)
   M_OBJECT_PROPERTY_INT               (EC12BadProcedureResult, ProcedureResultCode)
M_START_METHODS(EC12BadProcedureResult)
   M_CLASS_SERVICE                     (EC12BadProcedureResult, New,                ST_MObjectP_S)
   M_CLASS_SERVICE                     (EC12BadProcedureResult, Throw,              ST_S_int)
M_END_CLASS(EC12BadProcedureResult, COMException)

MEC12BadProcedureResult::MEC12BadProcedureResult(ResultCodeEnum resultCode)
:
   MCOMException(),
   m_procedureResultCode(resultCode)
{
   M_ASSERT(resultCode != 0); // this should not be here...
   MException::KindType kind = MException::ErrorMeter;
   MErrorEnum::Type errorCode = MErrorEnum::Type(MErrorEnum::C12ProcedureResultOK + resultCode);
#if !M_NO_VERBOSE_ERROR_INFORMATION
   MConstLocalChars str;
   switch ( resultCode )
   {
   case RESULT_NOT_COMPLETED:     str = M_I("Procedure result code 1, Procedure accepted but not fully completed");                                    break;
   case RESULT_INVALID_PARAMETER: str = M_I("Procedure result code 2, invalid parameter");                           kind = MException::ErrorSoftware; break;
   case RESULT_SETUP_CONFLICT:    str = M_I("Procedure result code 3, conflict with the current device setup");                                        break;
   case RESULT_IGNORE_DUE_TIMING: str = M_I("Procedure result code 4, had to ignore the procedure due to timing constraint");                          break;
   case RESULT_NO_AUTHORIZATION:  str = M_I("Procedure result code 5, no authorization to perform this procedure");  kind = MException::ErrorSecurity; break;
   case RESULT_UNKNOWN_PROCEDURE: str = M_I("Procedure result code 6, unrecognized or unsupported procedure");       kind = MException::ErrorSoftware; break;
   default: str = M_I("Unknown procedure result code 0x%X");                   errorCode = MErrorEnum::Type(MErrorEnum::C12ProcedureInvalidParameter); break;
   }
   Init(kind, errorCode, str, (unsigned)resultCode); // we know it is okay to supply more parameters than needed
#else
   switch ( resultCode )
   {
   case RESULT_NOT_COMPLETED:                                       break;
   case RESULT_INVALID_PARAMETER: kind = MException::ErrorSoftware; break;
   case RESULT_SETUP_CONFLICT:                                      break;
   case RESULT_IGNORE_DUE_TIMING:                                   break;
   case RESULT_NO_AUTHORIZATION:  kind = MException::ErrorSecurity; break;
   case RESULT_UNKNOWN_PROCEDURE: kind = MException::ErrorSoftware; break;
   default: errorCode = MErrorEnum::Type(MErrorEnum::C12ProcedureInvalidParameter); break;
   }
   Init(kind, errorCode);
#endif
}

MEC12BadProcedureResult::MEC12BadProcedureResult(const MEC12BadProcedureResult& ex)
:
   MCOMException(ex),
   m_procedureResultCode(ex.m_procedureResultCode)
{
}

MEC12BadProcedureResult::~MEC12BadProcedureResult() M_NO_THROW
{
}

MEC12BadProcedureResult* MEC12BadProcedureResult::New()
{
   return M_NEW MEC12BadProcedureResult;
}

MException* MEC12BadProcedureResult::NewClone() const
{
   return M_NEW MEC12BadProcedureResult(*this);
}

M_NORETURN_FUNC void MEC12BadProcedureResult::Throw(ResultCodeEnum resultCode)
{
   MEC12BadProcedureResult(resultCode).Rethrow();
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MEC12BadProcedureResult::Rethrow()
{
   throw *this;
}

#endif // !M_NO_MCOM_PROTOCOL_C1218 || !M_NO_MCOM_PROTOCOL_C1221 || !M_NO_MCOM_PROTOCOL_C1222

M_START_PROPERTIES(EChannelReadTimeout)
   M_OBJECT_PROPERTY_UINT              (EChannelReadTimeout, BytesRead)
M_START_METHODS(EChannelReadTimeout)
   M_CLASS_SERVICE                     (EChannelReadTimeout, New,                ST_MObjectP_S)
   M_CLASS_SERVICE                     (EChannelReadTimeout, Throw,              ST_S_unsigned)
M_END_CLASS(EChannelReadTimeout, COMException)

MEChannelReadTimeout::MEChannelReadTimeout(unsigned bytesRead)
:
   MCOMException(M_CODE_STR_P1(MErrorEnum::ChannelReadTimeout, M_I("Channel read timeout (%u bytes read successfully)"), bytesRead)),
   m_bytesRead(bytesRead)
{
}

MEChannelReadTimeout::MEChannelReadTimeout(const MEChannelReadTimeout& ex)
:
   MCOMException(ex),
   m_bytesRead(ex.m_bytesRead)
{
   M_ASSERT(m_kind == MException::ErrorCommunication); // make sure it is never overridden
}

MEChannelReadTimeout::~MEChannelReadTimeout() M_NO_THROW
{
}

M_NORETURN_FUNC void MEChannelReadTimeout::Throw(unsigned bytesRead)
{
   MEChannelReadTimeout(bytesRead).Rethrow();
   M_ENSURED_ASSERT(0);
}

MEChannelReadTimeout* MEChannelReadTimeout::New()
{
   return M_NEW MEChannelReadTimeout;
}

MException* MEChannelReadTimeout::NewClone() const
{
   return M_NEW MEChannelReadTimeout(*this);
}

M_NORETURN_FUNC void MEChannelReadTimeout::Rethrow()
{
   throw *this;
}

M_START_PROPERTIES(EChannelWriteTimeout)
   M_OBJECT_PROPERTY_UINT              (EChannelWriteTimeout, BytesWritten)
M_START_METHODS(EChannelWriteTimeout)
   M_CLASS_SERVICE                     (EChannelWriteTimeout, New,                  ST_MObjectP_S)
   M_CLASS_SERVICE                     (EChannelWriteTimeout, Throw,                ST_S_unsigned)
M_END_CLASS(EChannelWriteTimeout, COMException)

MEChannelWriteTimeout::MEChannelWriteTimeout(unsigned bytesWritten)
:
   MCOMException(M_CODE_STR_P1(MErrorEnum::ChannelWriteTimeout, M_I("Channel write timeout (%u bytes written successfully)"), bytesWritten)),
   m_bytesWritten(bytesWritten)
{
}

MEChannelWriteTimeout::MEChannelWriteTimeout(const MEChannelWriteTimeout& ex)
:
   MCOMException(ex),
   m_bytesWritten(ex.m_bytesWritten)
{
   M_ASSERT(m_kind == MException::ErrorCommunication); // make sure it is never overridden
}

MEChannelWriteTimeout::~MEChannelWriteTimeout() M_NO_THROW
{
}

M_NORETURN_FUNC void MEChannelWriteTimeout::Throw(unsigned bytesWritten)
{
   MEChannelWriteTimeout(bytesWritten).Rethrow();
   M_ENSURED_ASSERT(0);
}

MEChannelWriteTimeout* MEChannelWriteTimeout::New()
{
   return M_NEW MEChannelWriteTimeout;
}

MException* MEChannelWriteTimeout::NewClone() const
{
   return M_NEW MEChannelWriteTimeout(*this);
}

M_NORETURN_FUNC void MEChannelWriteTimeout::Rethrow()
{
   throw *this;
}

M_START_PROPERTIES(EChannelDisconnectedUnexpectedly)
M_START_METHODS(EChannelDisconnectedUnexpectedly)
   M_CLASS_SERVICE                     (EChannelDisconnectedUnexpectedly, New,   ST_MObjectP_S)
   M_CLASS_SERVICE                     (EChannelDisconnectedUnexpectedly, Throw, ST_S)
M_END_CLASS(EChannelDisconnectedUnexpectedly, COMException)

MEChannelDisconnectedUnexpectedly::MEChannelDisconnectedUnexpectedly()
:
   MCOMException(M_CODE_STR(MErrorEnum::ChannelDisconnectedUnexpectedly, M_I("Channel disconnected unexpectedly")))
{
}

MEChannelDisconnectedUnexpectedly::MEChannelDisconnectedUnexpectedly(const MEChannelDisconnectedUnexpectedly& ex)
:
   MCOMException(ex)
{
   M_ASSERT(m_kind == MException::ErrorCommunication); // make sure it is never overridden
}

MEChannelDisconnectedUnexpectedly::~MEChannelDisconnectedUnexpectedly() M_NO_THROW
{
}

MEChannelDisconnectedUnexpectedly* MEChannelDisconnectedUnexpectedly::New()
{
   return M_NEW MEChannelDisconnectedUnexpectedly;
}

M_NORETURN_FUNC void MEChannelDisconnectedUnexpectedly::Throw()
{
   MEChannelDisconnectedUnexpectedly().Rethrow();
   M_ENSURED_ASSERT(0);
}

MException* MEChannelDisconnectedUnexpectedly::NewClone() const
{
   return M_NEW MEChannelDisconnectedUnexpectedly(*this);
}

M_NORETURN_FUNC void MEChannelDisconnectedUnexpectedly::Rethrow()
{
   throw *this;
}

M_START_PROPERTIES(ECollisionDetected)
M_START_METHODS(ECollisionDetected)
   M_CLASS_SERVICE                     (ECollisionDetected, New,   ST_MObjectP_S)
   M_CLASS_SERVICE                     (ECollisionDetected, Throw, ST_S)
M_END_CLASS(ECollisionDetected, COMException)

MECollisionDetected::MECollisionDetected()
:
   MCOMException(M_CODE_STR(MErrorEnum::CollisionDetected, M_I("Collision detected by a slave protocol")))
{
}

MECollisionDetected::MECollisionDetected(const MECollisionDetected& ex)
:
   MCOMException(ex)
{
   M_ASSERT(m_kind == MException::ErrorCommunication); // make sure it is never overridden
}

MECollisionDetected::~MECollisionDetected() M_NO_THROW
{
}

MECollisionDetected* MECollisionDetected::New()
{
   return M_NEW MECollisionDetected;
}

M_NORETURN_FUNC void MECollisionDetected::Throw()
{
   MECollisionDetected().Rethrow();
   M_ENSURED_ASSERT(0);
}

MException* MECollisionDetected::NewClone() const
{
   return M_NEW MECollisionDetected(*this);
}

M_NORETURN_FUNC void MECollisionDetected::Rethrow()
{
   throw *this;
}
