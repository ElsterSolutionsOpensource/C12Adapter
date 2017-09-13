#ifndef MCOM_MCOMEXCEPTIONS_H
#define MCOM_MCOMEXCEPTIONS_H
/// \addtogroup MCOM
///@{
/// \file MCOM/MCOMExceptions.h
///
/// Communication-related exception classes, derived from MException.
///

#include <MCOM/MCOMDefs.h>

/// Root of every exception thrown by the MCOM component.
///
/// Use this exception if there is a need to catch MCOM exceptions only.
/// Notice that MCOM calls can still throw MCORE exceptions -
/// case when MCOM called the methods of MCORE.
///
class MCOM_CLASS MCOMException : public MException
{
   friend class MProtocolLayerWrapper;

protected: // Constructor:

   // Default constructor of the class,
   // should be used by children to build MCOM exception.
   // No throws by this constructor are done, as the code will be unspecified.
   //
   // \pre Should be called only from constructors of concrete
   // children. This is supported syntactically, the service is protected.
   //
   MCOMException();

public: // Constructor and services:

#if !M_NO_VERBOSE_ERROR_INFORMATION
   /// Constructor that takes the message code and the arguments that
   /// correspond to the message code.
   ///
   /// \pre The message code is valid, the parameters
   /// match the message string defined in the resource file.
   ///
   MCOMException(MErrorEnum::Type code, MConstLocalChars str, ...);
#else
   /// Constructor that takes the code.
   ///
   MCOMException(MErrorEnum::Type code);
#endif // !M_NO_VERBOSE_ERROR_INFORMATION

   /// Copy constructor.
   ///
   MCOMException(const MCOMException& ex);

   /// Destructor.
   ///
   virtual ~MCOMException() M_NO_THROW;

   /// Assignment operator.
   ///
   /// Be careful while using the assignment operator when dealing with children of this class,
   /// as the properties of a child might not be copied correctly.
   ///
   /// \param ex
   ///     Exception from which to copy this exception.
   ///
   /// \return Reference to this object.
   ///
   MCOMException& operator=(const MCOMException& ex)
   {
      if ( &ex != this )
         MException::operator=(ex);
      return *this;
   }

   /// Create a new exception of this type, all parameters are clear.
   ///
   static MCOMException* New();

#if !M_NO_VERBOSE_ERROR_INFORMATION

   /// @{
   /// Throws this exception with the parameters given.
   /// Analog of the following code:
   /// \code
   ///     throw MCOMException(code, par1, par2, parN);
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   /// The following is the usage:
   /// \code
   ///     MCOMException::Throw(code, par1, par2, parN);
   /// \endcode
   ///
   static M_NORETURN_FUNC void Throw(MErrorEnum::Type code, MConstLocalChars str, ...);
   static M_NORETURN_FUNC void Throw(MErrorEnum::Type code, const char* str, ...);
   /// @}

   /// @{
   /// Throws this exception with the parameters given.
   /// Analog of the following code:
   /// \code
   ///     throw MCOMException(kind, code, par1, par2, parN);
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   /// The following is the usage:
   /// \code
   ///     MCOMException::Throw(kind, code, par1, par2, parN);
   /// \endcode
   ///
   static M_NORETURN_FUNC void Throw(MException::KindType kind, MErrorEnum::Type code, MConstLocalChars str, ...);
   static M_NORETURN_FUNC void Throw(MException::KindType kind, MErrorEnum::Type code, const char* str, ...);
   /// @}

#else // !M_NO_VERBOSE_ERROR_INFORMATION

   /// @{
   /// Throws this exception with the parameters given.
   /// Analog of the following code:
   /// \code
   ///     throw MCOMException(code);
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   ///
   static M_NORETURN_FUNC void Throw(MErrorEnum::Type code);
   /// @}

   /// @{
   /// Throws this exception with the parameters given.
   /// Analog of the following code:
   /// \code
   ///     throw MCOMException(kind, code);
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   ///
   static M_NORETURN_FUNC void Throw(MException::KindType kind, MErrorEnum::Type code);
   /// @}

#endif // !M_NO_VERBOSE_ERROR_INFORMATION

   /// Throws this exception with the parameters that signify the the operation is
   /// invalid due to background work.
   ///
   static M_NORETURN_FUNC void ThrowInvalidOperationInForeground();

   /// If the given two values are different then throw an error.
   ///
   /// An error thrown will be "Expected %d bytes, but found %d, possibly the buffer has incorrect layout".
   ///
   static void CheckIfExpectedDataSizeDifferent(unsigned receivedLength, unsigned expectedLength);

   /// Clone the exception, so the new exception has the same final type.
   ///
   /// \return New exception, which is the same as this.
   ///
   /// \see Rethrow() Rethrows the cloned exception with respect to the final exception type.
   ///
   virtual MException* NewClone() const;

   /// Rethrows this exception.
   ///
   /// This polymorphic call is necessary if a generic exception is caught by its base,
   /// and had to be saved for future throwing. Simple "throw *ex" will not work in this case,
   /// as it will call the copy constructor of the declared exception,
   /// not the final class copy constructor.
   ///
   /// \see NewClone() Clones the exception so the new exception has the same final type.
   ///
   virtual M_NORETURN_FUNC void Rethrow();

   M_DECLARE_CLASS(COMException)
};

#if !M_NO_MCOM_PROTOCOL_C1218 || !M_NO_MCOM_PROTOCOL_C1221 || !M_NO_MCOM_PROTOCOL_C1222

/// Exception which is thrown in case the ANSI C12 meter responded with an error defined by the protocol.
///
/// This is an application-level protocol error.
///
class MCOM_CLASS MEC12NokResponse : public MCOMException
{
   // Default constructor. Used only for serialization purposes.
   //
   MEC12NokResponse()
   :
      MCOMException(),
      m_responseCode(RESPONSE_OK) // will be reassigned
   {
   }

public: // Types:

   /// Known response codes defined by a family of C12 protocols.
   ///
   enum ResponseCodeEnum
   {
      /// Okay
      ///
      /// No error, never seen in the exception
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseOK
      ///
      RESPONSE_OK = 0x00,

      /// Generic error, no reason provided
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseERR
      ///
      RESPONSE_ERR = 0x01,

      /// Service Not Supported
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseSNS
      ///
      RESPONSE_SNS = 0x02,

      /// Insufficient Security Clearance
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseISC
      ///
      RESPONSE_ISC = 0x03,

      /// Operation Not Possible
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseONP
      ///
      RESPONSE_ONP = 0x04,

      /// Inappropriate Action Requested
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseIAR
      ///
      RESPONSE_IAR = 0x05,

      /// Device Busy, ran out of retries
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseBSY
      ///
      RESPONSE_BSY = 0x06,

      /// Data Not Ready, ran out of retries
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseDNR
      ///
      RESPONSE_DNR = 0x07,

      /// Data Locked
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseDLK
      ///
      RESPONSE_DLK = 0x08,

      /// Renegotiate
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseRNO
      ///
      RESPONSE_RNO = 0x09,

      /// Invalid Service Sequence State
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseISSS
      ///
      RESPONSE_ISSS = 0x0A,

      /// Security mechanism error detected
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseSME
      ///
      RESPONSE_SME = 0x0B,

      /// Unknown or invalid Called APTitle is received
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseUAT
      ///
      RESPONSE_UAT = 0x0C,

      /// Network timeout detected
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseNETT
      ///
      RESPONSE_NETT = 0x0D,

      /// Node is not reachable
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseNETR
      ///
      RESPONSE_NETR = 0x0E,

      /// Request is too large
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseRQTL
      ///
      /// Parameter is sent with this exception, UINT24, maximum request size.
      ///
      RESPONSE_RQTL = 0x0F,

      /// Response is too large (parameter is UINT24, maximum request size)
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseRSTL
      ///
      RESPONSE_RSTL = 0x10,

      /// Segmentation required, but not possible
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseSGNP
      ///
      RESPONSE_SGNP = 0x11,

      /// Segmentation error
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseSGERR
      ///
      /// Two parameters are sent with this exception: offset and size.
      /// They are both of the same size, from UINT8 to UINT24.
      ///
      RESPONSE_SGERR = 0x12,


      /// Not registered to you, code not defined in C12 standard, specific to Elster.
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ServiceResponseNRY
      ///
      RESPONSE_NRY = 0x1E
   };

/// \cond SHOW_INTERNAL

   typedef ResponseCodeEnum ResponseCode; // compatibility type definition for older MeteringSDK clients

/// \endcond SHOW_INTERNAL
public:

   /// Constructor that takes the specific response code defined by the ANSI protocol.
   ///
   /// The given message code determines the result message.
   ///
   /// \param responseCode
   ///     One-byte response code, normally in range 0 .. 0x1F, as defined by ANSI.
   ///
   MEC12NokResponse(Muint8 responseCode);

   /// Constructor that takes the specific response code defined by the ANSI protocol, and an extra parameter.
   ///
   /// The given message code determines the result message.
   /// The extraParameters, if not an empty byte string, will be added to message.
   /// Handling of extraParameters depends on the message code like the following:
   ///
   ///  - RESPONSE_RQTL (code 0x0F): request too large parameter expected to be of type UINT24,
   ///    which will be the maximum request size.
   ///  - RESPONSE_RSTL (code 0x10): response too large parameter expected to be of type UINT24,
   ///    maximum response size.
   ///  - RESPONSE_SGERR (code 0x12): segmentation error parameters are offset and size,
   ///    where both are of the same size type UINT8, UINT16, or UINT24.
   ///  - When response code is different from ones mentioned, or when extraParameters parameters
   ///    do not correspond to the defined ones, extra parameters byte string is added as hex to the message.
   ///
   /// \param responseCode
   ///     One-byte response code, normally in range 0 .. 0x1F, as defined by ANSI.
   ///     Some response codes have known parameters that get handled using the information given as extraParameter.
   ///
   /// \param extraPerameters
   ///     Byte string to represent extra parameters.
   ///
   MEC12NokResponse(Muint8 responseCode, const MByteString& extraPerameters);

   /// Copy constructor.
   ///
   MEC12NokResponse(const MEC12NokResponse&);

   /// Destructor.
   ///
   virtual ~MEC12NokResponse() M_NO_THROW;

   /// Assignment operator.
   ///
   /// Be careful while using the assignment operator when dealing with children of this class,
   /// as the properties of a child might not be copied correctly.
   ///
   /// \param ex
   ///     Exception from which to copy this exception.
   ///
   /// \return Reference to this object.
   ///
   MEC12NokResponse& operator=(const MEC12NokResponse& ex)
   {
      if ( &ex != this )
      {
         m_responseCode = ex.m_responseCode;
         m_extraParameters = ex.m_extraParameters;
         MCOMException::operator=(ex);
      }
      return *this;
   }

   /// Create a new exception of this type, all parameters are clear.
   ///
   static MEC12NokResponse* New();

   ///@{
   /// Response error code of the C12 protocol, as defined by the standard.
   ///
   /// \if CPP
   ///   See the enumeration \ref MEC12NokResponse::ResponseCodeEnum for possible
   ///   defined values returned by this service. Note that the response code
   ///   is not necessarily within the range defined by this enumeration.
   /// \endif
   ///
   ResponseCodeEnum GetResponseCode() const
   {
      return m_responseCode;
   }
   void SetResponseCode(ResponseCodeEnum code)
   {
      m_responseCode = code;
   }
   ///@}

   ///@{
   /// Extra parameters of C12 protocol error.
   ///
   const MByteString& GetExtraParameters() const
   {
      return m_extraParameters;
   }
   void SetExtraParameters(const MByteString& extraParameters)
   {
      m_extraParameters = extraParameters;
   }
   ///@}

   /// Throws exception MEC12NokResponse with the response code given.
   /// Analog of the following code:
   /// \code
   ///     throw MEC12NokResponse(responseCode);
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   /// The following is the usage:
   /// \code
   ///     MEC12NokResponse::Throw(responseCode);
   /// \endcode
   ///
   static M_NORETURN_FUNC void Throw(Muint8 responseCode);

   /// Throws exception MEC12NokResponse with the response code and extra parameters given.
   /// Analog of the following code:
   /// \code
   ///     throw MEC12NokResponse(responseCode, extraParameters);
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   /// The following is the usage:
   /// \code
   ///     MEC12NokResponse::Throw(responseCode);
   /// \endcode
   ///
   static M_NORETURN_FUNC void ThrowWithParameters(Muint8 responseCode, const MByteString& extraParameters);

   /// Clone the exception, so the new exception has the same final type.
   ///
   /// \return New exception, which is the same as this.
   ///
   /// \see Rethrow() Rethrows the cloned exception with respect to the final exception type.
   ///
   virtual MException* NewClone() const;

   /// Rethrows this exception.
   ///
   /// This polymorphic call is necessary if a generic exception is caught by its base,
   /// and had to be saved for future throwing. Simple "throw *ex" will not work in this case,
   /// as it will call the copy constructor of the declared exception,
   /// not the final class copy constructor.
   ///
   /// \see NewClone() Clones the exception so the new exception has the same final type.
   ///
   virtual M_NORETURN_FUNC void Rethrow();

private: // Attributes:

   // Init the uninitialized MEC12NokResponse based on the given response code.
   //
   void Init(Muint8 responseCode);

   // Response code of C12 protocol, only one byte is used.
   //
   ResponseCodeEnum m_responseCode;

   // Additional parameters of C12 error.
   //
   MByteString m_extraParameters;

   M_DECLARE_CLASS(EC12NokResponse)
};

/// Exception which is thrown for the erroneous ANSI C12 procedure response code.
/// This is an application-level protocol error.
///
class MCOM_CLASS MEC12BadProcedureResult : public MCOMException
{
   // Default constructor. Used only for serialization purposes.
   //
   MEC12BadProcedureResult()
   :
      MCOMException(),
      m_procedureResultCode(RESULT_OK)
   {
   }

public: // Types:

   /// Known results of procedure execution, as defined by C12.19.
   ///
   enum ResultCodeEnum
   {
      /// Okay, procedure completed, never thrown code
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ProcedureResultOK
      ///
      RESULT_OK = 0x00,

      /// Procedure accepted but not fully completed, retryable code
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ProcedureNotCompleted
      ///
      RESULT_NOT_COMPLETED = 0x01,

      /// Invalid parameter for known procedure
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ProcedureInvalidParameter
      ///
      RESULT_INVALID_PARAMETER = 0x02,

      /// Conflict with the current device setup
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ProcedureDeviceConflict
      ///
      RESULT_SETUP_CONFLICT = 0x03,

      /// Had to ignore the procedure due to timing constraint
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ProcedureTimingConstraint
      ///
      RESULT_IGNORE_DUE_TIMING = 0x04,

      /// No authorization to perform this procedure
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ProcedureNoAuthorization
      ///
      RESULT_NO_AUTHORIZATION = 0x05,

      /// Unrecognized or unsupported procedure
      ///
      /// Corresponds to \refprop{MException::GetCode,Exception::Code} value MErrorEnum::C12ProcedureUnknown
      ///
      RESULT_UNKNOWN_PROCEDURE = 0x06
   };

public:

   /// Constructor that takes the procedure result code
   /// defined by the ANSI protocol.
   ///
   explicit MEC12BadProcedureResult(ResultCodeEnum resultCode);

   /// Copy constructor.
   ///
   MEC12BadProcedureResult(const MEC12BadProcedureResult&);

   /// Destructor.
   ///
   virtual ~MEC12BadProcedureResult() M_NO_THROW;

   /// Assignment operator.
   ///
   /// Be careful while using the assignment operator when dealing with children of this class,
   /// as the properties of a child might not be copied correctly.
   ///
   /// \param ex
   ///     Exception from which to copy this exception.
   ///
   /// \return Reference to this object.
   ///
   MEC12BadProcedureResult& operator=(const MEC12BadProcedureResult& ex)
   {
      if ( &ex != this )
      {
         m_procedureResultCode = ex.m_procedureResultCode;
         MCOMException::operator=(ex);
      }
      return *this;
   }

   /// Create a new exception of this type, all parameters are clear.
   ///
   static MEC12BadProcedureResult* New();

   ///@{
   /// Access the procedure result code, received by the time
   /// the procedure was executed unsuccessfully.
   ///
   ResultCodeEnum GetProcedureResultCode() const
   {
      return m_procedureResultCode;
   }
   void SetProcedureResultCode(ResultCodeEnum code)
   {
      m_procedureResultCode = code;
   }
   ///@}

   /// Clone the exception, so the new exception has the same final type.
   ///
   /// \return New exception, which is the same as this.
   ///
   /// \see Rethrow() Rethrows the cloned exception with respect to the final exception type.
   ///
   virtual MException* NewClone() const;

   /// Rethrows this exception.
   ///
   /// This polymorphic call is necessary if a generic exception is caught by its base,
   /// and had to be saved for future throwing. Simple "throw *ex" will not work in this case,
   /// as it will call the copy constructor of the declared exception,
   /// not the final class copy constructor.
   ///
   /// \see NewClone() Clones the exception so the new exception has the same final type.
   ///
   virtual M_NORETURN_FUNC void Rethrow();

   /// Throws this exception with the parameters given.
   /// Analog of the following code:
   /// \code
   ///     throw MEC12BadProcedureResult(code);
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   /// The following is the usage:
   /// \code
   ///     MEC12BadProcedureResult::Throw(code);
   /// \endcode
   ///
   static M_NORETURN_FUNC void Throw(ResultCodeEnum resultCode);

private: // Attributes:

   // Saved procedure result code available for further retrieval.
   //
   ResultCodeEnum m_procedureResultCode;

   M_DECLARE_CLASS(EC12BadProcedureResult)
};

#endif // !M_NO_MCOM_PROTOCOL_C1218 || !M_NO_MCOM_PROTOCOL_C1221 || !M_NO_MCOM_PROTOCOL_C1222

/// Exception which is thrown in case the timeout condition happens
/// during reading from the channel. This is an channel-level error.
///
class MCOM_CLASS MEChannelReadTimeout : public MCOMException
{
   // Default constructor. Used only for serialization purposes.
   //
   MEChannelReadTimeout()
   :
      MCOMException(),
      m_bytesRead(0)
   {
   }

public:  // Constructor, destructor, services:

   /// Constructor that builds the channel read timeout exception.
   /// The parameter is how much bytes have been successfully read
   /// before the timeout condition took place.
   ///
   explicit MEChannelReadTimeout(unsigned bytesRead);

   /// Copy constructor.
   ///
   MEChannelReadTimeout(const MEChannelReadTimeout&);

   /// Destructor.
   ///
   virtual ~MEChannelReadTimeout() M_NO_THROW;

   /// Assignment operator.
   ///
   /// Be careful while using the assignment operator when dealing with children of this class,
   /// as the properties of a child might not be copied correctly.
   ///
   /// \param ex
   ///     Exception from which to copy this exception.
   ///
   /// \return Reference to this object.
   ///
   MEChannelReadTimeout& operator=(const MEChannelReadTimeout& ex)
   {
      if ( &ex != this )
      {
         m_bytesRead = ex.m_bytesRead;
         MCOMException::operator=(ex);
      }
      return *this;
   }

   /// Create a new exception of this type, all parameters are clear.
   ///
   static MEChannelReadTimeout* New();

   ///@{
   /// Number of bytes read successfully before the timeout took place.
   ///
   /// The exception processing algorithm might depend on the number of bytes
   /// successfully transferred.
   ///
   unsigned GetBytesRead() const
   {
      return m_bytesRead;
   }
   void SetBytesRead(unsigned count)
   {
      m_bytesRead = count;
   }
   ///@}

   /// Throws this exception with the parameters given.
   /// Analog of the following code:
   /// \code
   ///     throw MEChannelReadTimeout(bytesRead);
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   /// The following is the usage:
   /// \code
   ///     MEChannelReadTimeout::Throw(bytesRead);
   /// \endcode
   ///
   static M_NORETURN_FUNC void Throw(unsigned bytesRead);

   /// Clone the exception, so the new exception has the same final type.
   ///
   /// \return New exception, which is the same as this.
   ///
   /// \see Rethrow() Rethrows the cloned exception with respect to the final exception type.
   ///
   virtual MException* NewClone() const;

   /// Rethrows this exception.
   ///
   /// This polymorphic call is necessary if a generic exception is caught by its base,
   /// and had to be saved for future throwing. Simple "throw *ex" will not work in this case,
   /// as it will call the copy constructor of the declared exception,
   /// not the final class copy constructor.
   ///
   /// \see NewClone() Clones the exception so the new exception has the same final type.
   ///
   virtual M_NORETURN_FUNC void Rethrow();

private: // Attributes:

   // Number of bytes successfully read
   //
   unsigned m_bytesRead;

   M_DECLARE_CLASS(EChannelReadTimeout)
};

/// Exception which is thrown in case the timeout condition happens
/// during writing to the channel. This is an channel-level error.
///
class MCOM_CLASS MEChannelWriteTimeout : public MCOMException
{
   // Default constructor. Used only for serialization purposes.
   //
   MEChannelWriteTimeout()
   :
      MCOMException(),
      m_bytesWritten(0)
   {
   }

public:  // Constructor, destructor, services:

   /// Constructor that builds the channel write timeout exception.
   /// The parameter is how much bytes have been successfully written
   /// before the timeout condition took place.
   ///
   explicit MEChannelWriteTimeout(unsigned bytesWritten);

   /// Copy constructor.
   ///
   MEChannelWriteTimeout(const MEChannelWriteTimeout&);

   /// Destructor.
   ///
   virtual ~MEChannelWriteTimeout() M_NO_THROW;

   /// Assignment operator.
   ///
   /// Be careful while using the assignment operator when dealing with children of this class,
   /// as the properties of a child might not be copied correctly.
   ///
   /// \param ex
   ///     Exception from which to copy this exception.
   ///
   /// \return Reference to this object.
   ///
   MEChannelWriteTimeout& operator=(const MEChannelWriteTimeout& ex)
   {
      if ( &ex != this )
      {
         m_bytesWritten = ex.m_bytesWritten;
         MCOMException::operator=(ex);
      }
      return *this;
   }

   /// Create a new exception of this type, all parameters are clear.
   ///
   static MEChannelWriteTimeout* New();

   ///@{
   /// Number of bytes written successfully before the timeout took place.
   ///
   /// The exception processing algorithm might depend on the number of bytes
   /// successfully transferred.
   ///
   unsigned GetBytesWritten() const
   {
      return m_bytesWritten;
   }
   void SetBytesWritten(unsigned count)
   {
      m_bytesWritten = count;
   }
   ///@}

   /// Throws this exception with the parameters given.
   /// Analog of the following code:
   /// \code
   ///     throw MEChannelWriteTimeout(bytesWritten);
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   /// The following is the usage:
   /// \code
   ///     MEChannelWriteTimeout::Throw(bytesWritten);
   /// \endcode
   ///
   static M_NORETURN_FUNC void Throw(unsigned bytesWritten);

   /// Clone the exception, so the new exception has the same final type.
   ///
   /// \return New exception, which is the same as this.
   ///
   /// \see Rethrow() Rethrows the cloned exception with respect to the final exception type.
   ///
   virtual MException* NewClone() const;

   /// Rethrows this exception.
   ///
   /// This polymorphic call is necessary if a generic exception is caught by its base,
   /// and had to be saved for future throwing. Simple "throw *ex" will not work in this case,
   /// as it will call the copy constructor of the declared exception,
   /// not the final class copy constructor.
   ///
   /// \see NewClone() Clones the exception so the new exception has the same final type.
   ///
   virtual M_NORETURN_FUNC void Rethrow();

private: // Attributes:

   // Number of bytes successfully written
   //
   unsigned m_bytesWritten;

   M_DECLARE_CLASS(EChannelWriteTimeout)
};

/// Exception which is thrown in case the channel is unexpectedly disconnected.
/// This is an channel-level error which shall not be retried.
///
class MCOM_CLASS MEChannelDisconnectedUnexpectedly : public MCOMException
{
public:  // Constructor, destructor, services:

   /// Constructor that builds the exception.
   ///
   MEChannelDisconnectedUnexpectedly();

   /// Copy constructor.
   ///
   MEChannelDisconnectedUnexpectedly(const MEChannelDisconnectedUnexpectedly&);

   /// Destructor.
   ///
   virtual ~MEChannelDisconnectedUnexpectedly() M_NO_THROW;

   /// Assignment operator.
   ///
   /// Be careful while using the assignment operator when dealing with children of this class,
   /// as the properties of a child might not be copied correctly.
   ///
   /// \param ex
   ///     Exception from which to copy this exception.
   ///
   /// \return Reference to this object.
   ///
   MEChannelDisconnectedUnexpectedly& operator=(const MEChannelDisconnectedUnexpectedly& ex)
   {
      if ( &ex != this )
      {
         MCOMException::operator=(ex);
      }
      return *this;
   }

   /// Create a new exception of this type, all parameters are clear.
   ///
   static MEChannelDisconnectedUnexpectedly* New();

   /// Throws this exception with the parameters given.
   /// Analog of the following code:
   /// \code
   ///     throw MEChannelDisconnectedUnexpectedly();
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   /// The following is the usage:
   /// \code
   ///     MEChannelDisconnectedUnexpectedly::Throw();
   /// \endcode
   ///
   static M_NORETURN_FUNC void Throw();

   /// Clone the exception, so the new exception has the same final type.
   ///
   /// \return New exception, which is the same as this.
   ///
   /// \see Rethrow() Rethrows the cloned exception with respect to the final exception type.
   ///
   virtual MException* NewClone() const;

   /// Rethrows this exception.
   ///
   /// This polymorphic call is necessary if a generic exception is caught by its base,
   /// and had to be saved for future throwing. Simple "throw *ex" will not work in this case,
   /// as it will call the copy constructor of the declared exception,
   /// not the final class copy constructor.
   ///
   /// \see NewClone() Clones the exception so the new exception has the same final type.
   ///
   virtual M_NORETURN_FUNC void Rethrow();

   M_DECLARE_CLASS(EChannelDisconnectedUnexpectedly)
};

/// Exception which is thrown in case the channel detects the collision, and it has to yield to the peer.
///
/// This exception is thrown by a protocol that is in a slave mode,
/// which means if two peers start communication at the same time the slave 
/// should immediately give up and listen to master.
///
class MCOM_CLASS MECollisionDetected : public MCOMException
{
public:  // Constructor, destructor, services:

   /// Constructor that builds the exception.
   ///
   MECollisionDetected();

   /// Copy constructor.
   ///
   MECollisionDetected(const MECollisionDetected&);

   /// Destructor.
   ///
   virtual ~MECollisionDetected() M_NO_THROW;

   /// Assignment operator.
   ///
   /// Be careful while using the assignment operator when dealing with children of this class,
   /// as the properties of a child might not be copied correctly.
   ///
   /// \param ex
   ///     Exception from which to copy this exception.
   ///
   /// \return Reference to this object.
   ///
   MECollisionDetected& operator=(const MECollisionDetected& ex)
   {
      if ( &ex != this )
      {
         MCOMException::operator=(ex);
      }
      return *this;
   }

   /// Create a new exception of this type, all parameters are clear.
   ///
   static MECollisionDetected* New();

   /// Throws this exception.
   ///
   /// Analog of the following code:
   /// \code
   ///     throw MECollisionDetected();
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   /// The following is the usage:
   /// \code
   ///     MECollisionDetected::Throw();
   /// \endcode
   ///
   static M_NORETURN_FUNC void Throw();

   /// Clone the exception, so the new exception has the same final type.
   ///
   /// \return New exception, which is the same as this.
   ///
   /// \see Rethrow() Rethrows the cloned exception with respect to the final exception type.
   ///
   virtual MException* NewClone() const;

   /// Rethrows this exception.
   ///
   /// This polymorphic call is necessary if a generic exception is caught by its base,
   /// and had to be saved for future throwing. Simple "throw *ex" will not work in this case,
   /// as it will call the copy constructor of the declared exception,
   /// not the final class copy constructor.
   ///
   /// \see NewClone() Clones the exception so the new exception has the same final type.
   ///
   virtual M_NORETURN_FUNC void Rethrow();

   M_DECLARE_CLASS(ECollisionDetected)
};

///@}
#endif
