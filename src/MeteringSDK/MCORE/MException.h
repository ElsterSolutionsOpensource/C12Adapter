#ifndef MCORE_MEXCEPTION_H
#define MCORE_MEXCEPTION_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MException.h

#include <MCORE/MCOREDefs.h>
#include <MCORE/MObject.h>
#include <MCORE/MFileNameAndLineNumber.h>
#include <MCORE/MMessageCatalog.h>
#include <MCORE/MErrorEnum.h>

///@{
/// Helper macro that handles compile mode related to absence of a string in the exception and log.
///
#if !M_NO_VERBOSE_ERROR_INFORMATION
   #define M_CODE_STR(c,s)             (c), (s)       // code and string parameters
   #define M_CODE_STR_P1(c,s,p1)       (c), (s), (p1)
   #define M_CODE_STR_P2(c,s,p1,p2)    (c), (s), (p1), (p2)
   #define M_CODE_STR_P3(c,s,p1,p2,p3) (c), (s), (p1), (p2), (p3)

   #define M_OPT_STR(s)                (s)
#else
   #define M_CODE_STR(c,s)             (c)      // code only
   #define M_CODE_STR_P1(c,s,p1)       (c)
   #define M_CODE_STR_P2(c,s,p1,p2)    (c)
   #define M_CODE_STR_P3(c,s,p1,p2,p3) (c)

   #define M_OPT_STR(s)                NULL
#endif
///@}

/// Common exception base for MeteringSDK.
/// 
/// Exceptions are used in MeteringSDK for program-level error handling. If the error 
/// log has to be made, it has to be made separately - no exceptions are logged.
///
/// Exceptions have an associated message code, which is helpful to implement
/// the interfaces that do not support exceptions (like DCOM). Also there is
/// a severity associated. The majority of the exceptions have the ERROR severity.
/// The severity information is not stored with the exception, but fetched from 
/// the resource. There is a string associated with each exception, 
/// that is fetched from the locale sensitive resource.
///
/// There is a most general message code ErrUnknown.
///
/// File and line information can be associated with the whole exception.
/// In case the error relates to the whole file, like M_FILE_S1_NOT_OPEN,
/// the line number could be zero.
///
/// User applications can throw MException with direct strings.
/// The user exception would normally define a constructor with parameters,
/// which should actually be used during throwing, and also
/// its default constructor, copy constructor, assignment operator and
/// destructor. In most cases there is no need to overload any other services.
///
class M_CLASS MException : public MObject
{
public: // Types:

   /// Enumeration that stands for the kind of the exception.
   /// This enumeration is reflected.
   ///
   /// The client applications can define their own values.
   ///
   enum KindType
   {
      ErrorNone          =  0,  ///< The message is just a string, no severity
      ErrorInformation   =  1,  ///< Information, not an error, for example, "Operation cancelled"
      ErrorWarning       =  2,  ///< The message is a warning
      Error              =  3,  ///< Generalized error
      ErrorFatal         =  4,  ///< Fatal error, one after which the application will not be functional, most likely
      ErrorCommunication =  5,  ///< Communication error
      ErrorSystem        =  6,  ///< System error
      ErrorSocket        =  7,  ///< Socket error
      ErrorConfiguration =  8,  ///< Configuration error, such as missing registry entry
      ErrorSecurity      =  9,  ///< The error is due to security problem
      ErrorMeter         = 10,  ///< The error is in the meter
      
      // Here is a spare region 11 .. 14 for errors that are already present in str file

      ErrorSoftware      = 15,  ///< Exception is not suitable for showing to end users

      // Here is a spare region 16 .. 19 for errors that are not present in str file

      ErrorUser          = 20   ///< Application error start from this code
   };

#if !M_NO_VERBOSE_ERROR_INFORMATION

   enum
   {
      MaximumVisibleParameterLength = 64 ///< maximum desired parameter length when it is represented as string
   };

#endif

public: // Constructors, destructor and assignment operator:

   /// Object constructor. Builds an empty exception.
   ///
   MException();

#if !M_NO_VERBOSE_ERROR_INFORMATION
   /// Object constructor with code, kind and variable number of parameters.
   /// Builds an exception with the given message code and kind.
   /// The string is fetched from the resource and it has format of printf operator, 
   /// which allows multiple parameters to follow it.
   ///
   /// \pre The number of parameters and their types given after the message code
   /// should correspond to the format of the message string. Otherwise the
   /// behavior is undefined.
   ///
   explicit MException(const MStdString& message, MErrorEnum::Type code = MErrorEnum::ClientApplicationError, KindType kind = MException::ErrorUser);
#endif

   /// Object copy constructor.
   ///
   /// \pre The object given as parameter is a valid reference.
   /// Otherwise the behavior is undefined.
   ///
   MException(const MException& ex);

   /// Object destructor.
   ///
   virtual ~MException() M_NO_THROW;

public: // Properties:

   ///@{
   /// Error code of the exception.
   ///
   /// This service is useful to support C interfaces, logging, etc.
   ///
   MErrorEnum::Type GetCode() const
   {
      return m_code;
   }
   void SetCode(MErrorEnum::Type code)
   {
      m_code = code;
   }
   ///@}

   ///@{
   /// Message kind of the exception.
   ///
   /// The client applications can define their own values bigger than value of ErrorUser.
   ///
   KindType GetKind() const
   {
      return m_kind;
   }
   void SetKind(KindType kind)
   {
      m_kind = kind;
   }
   ///@}

public: // Initialization, assignment:

#if !M_NO_VERBOSE_ERROR_INFORMATION

   /// Initialize all exception fields with the exact given data.
   ///
   void InitAll(MException::KindType kind, MErrorEnum::Type code, const MStdString& message) M_NO_THROW;

   //@{
   /// Initialize the exception with the given message code and kind from which
   /// the message string is fetched from the resource. 
   ///
   /// \pre The number of parameters and their types given after message string
   /// should correspond to the format of the message string in the resource. 
   /// Otherwise the behavior is undefined.
   ///
   void Init(KindType kind, MErrorEnum::Type code, MConstLocalChars str, ...) M_NO_THROW;
   void Init(KindType kind, MErrorEnum::Type code, const char* str, ...) M_NO_THROW;
   void Init(MErrorEnum::Type code, MConstLocalChars str, ...) M_NO_THROW;
   void Init(MErrorEnum::Type code, const char* str, ...) M_NO_THROW;
   //@}

   //@{
   /// Initialize the exception with the given message code from the VA list argument. 
   ///
   /// \pre The number of parameters and their types given 
   /// should correspond to the format of the message string within the resource. 
   /// Otherwise the behavior is undefined.
   ///
   void InitVA(KindType kind, MErrorEnum::Type code, MConstLocalChars str, va_list va) M_NO_THROW;
   void InitVA(KindType kind, MErrorEnum::Type code, const char* str, va_list va) M_NO_THROW;
   void InitVA(MErrorEnum::Type code, MConstLocalChars str, va_list va) M_NO_THROW;
   void InitVA(MErrorEnum::Type code, const char* str, va_list va) M_NO_THROW;
   //@}

#else
   ///@{
   /// Initialize all exception fields with the exact given data.
   ///
   void InitAll(MException::KindType kind, MErrorEnum::Type code)
   {
      m_kind = kind;
      m_code = code;
   }
   void Init(MException::KindType kind, MErrorEnum::Type code)
   {
      m_kind = kind;
      m_code = code;
   }
   ///@}

   /// Initialize exception code.
   ///
   void Init(MErrorEnum::Type code)
   {
      m_kind = MException::Error;
      m_code = code;
   }

#endif // !M_NO_VERBOSE_ERROR_INFORMATION

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
   MException& operator=(const MException& ex);

public: // Services:

   /// Create a new exception of this type, all parameters are clear
   ///
   static MException* New();

   /// Throws this exception as unknown error.
   /// Analog of the following code:
   /// \code
   ///     throw MException();
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   /// The following is the usage:
   /// \code
   ///     MException::ThrowUnknownError();
   /// \endcode
   ///
   static M_NORETURN_FUNC void ThrowUnknownError();

   /// Throws this exception as error that signifies the item cannot be indexed.
   ///
   static M_NORETURN_FUNC void ThrowCannotIndexItem(MConstChars itemName = NULL);

   /// Throws this exception as error that signifies the operation is not supported for such type.
   ///
   static M_NORETURN_FUNC void ThrowNotSupportedForThisType();

   /// Throws this exception as error that signifies the value is absent.
   ///
   static M_NORETURN_FUNC void ThrowNoValue();

#if !M_NO_VERBOSE_ERROR_INFORMATION

   /// Throws this exception as error that signifies the value given in file and line is absent.
   ///
   static M_NORETURN_FUNC void ThrowNoValue(MFileNameAndLineNumber fl);

#endif

   /// Throws this exception as error that signifies the time value is bad.
   ///
   static M_NORETURN_FUNC void ThrowBadTimeValue(MConstChars str = NULL);

   /// Throws this exception as division by zero.
   ///
   static M_NORETURN_FUNC void ThrowDivisionByZero();

   /// Throws an error that the call is made out of sequence.
   ///
   static M_NORETURN_FUNC void ThrowCallOutOfSequence();

#if !M_NO_FILESYSTEM

   /// Throws file has bad format.
   ///
   static M_NORETURN_FUNC void ThrowBadFileFormat(const MStdString& fileName);

#endif

   /// Throws an exception that complains about an unexpected character.
   ///
   /// \param ch Character that is to be reported as unexpected.
   ///
   static M_NORETURN_FUNC void ThrowUnexpectedChar(int ch);

   /// Throws an exception that tells that the string is bigger than the maximum length.
   ///
   /// \param stringLength String length that was encountered, too big.
   /// \param maximumPossibleLength Expected maximum possible length.
   ///
   static M_NORETURN_FUNC void ThrowStringTooLong(int stringLength, int maximumPossibleLength);

#if !M_NO_VARIANT
   ///@{
   /// Throws this exception as syntax error exception.
   /// 
   static M_NORETURN_FUNC void ThrowSyntaxError(MConstChars offendingString);
   static M_NORETURN_FUNC void ThrowSyntaxError(MConstChars offendingString, size_t offendingStringLength);
   ///@}

   /// Throws an error that the given variant type is not supported.
   static M_NORETURN_FUNC void ThrowUnsupportedType(int typetag);
#endif

   ///@{
   /// Throws unknown item exception with the name given as constant pointer to character string.
   /// 
   static M_NORETURN_FUNC void ThrowUnknownItem(MException::KindType kind, MConstChars name);
   static M_NORETURN_FUNC void ThrowUnknownItem(MConstChars name);
   static M_NORETURN_FUNC void ThrowUnknownItem(MException::KindType kind, const MStdString& name);
   static M_NORETURN_FUNC void ThrowUnknownItem(const MStdString& name);
   ///@}

#if !M_NO_VERBOSE_ERROR_INFORMATION
   ///@{
   /// Throws this exception with the parameters given.
   /// Analog of the following code:
   /// \code
   ///     throw MException(code, par1, par2, parN);
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   /// The following is the usage:
   /// \code
   ///     MException::Throw(code, par1, par2, parN);
   /// \endcode
   ///
   static M_NORETURN_FUNC void Throw(MErrorEnum::Type code, MConstLocalChars str, ...);
   static M_NORETURN_FUNC void Throw(MErrorEnum::Type code, const char* str, ...);
   ///@}

   ///@{
   /// Throws this exception with the parameters given.
   /// Analog of the following code:
   /// \code
   ///     throw MException(kind, code, par1, par2, parN);
   /// \endcode
   ///
   /// but saves memory, if there are many throws in the source code.
   /// The following is the usage:
   /// \code
   ///     MException::Throw(kind, code, par1, par2, parN);
   /// \endcode
   ///
   static M_NORETURN_FUNC void Throw(KindType kind, MErrorEnum::Type code, MConstLocalChars str, ...);
   static M_NORETURN_FUNC void Throw(KindType kind, MErrorEnum::Type code, const char* str, ...);
   ///@}

   /// Throw this exception with the user message given as parameter.
   /// Analog of the following code:
   /// \code
   ///     throw MException(message, code);
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   /// The following is the usage:
   /// \code
   ///     MException::Throw("Error in my application");
   /// \endcode
   ///
   static M_NORETURN_FUNC void Throw(const MStdString& message, MErrorEnum::Type code = MErrorEnum::ClientApplicationError);

   /// Return a possibly simplified string that specifies information about exception.
   ///
   /// This is an overloaded service that makes use of knowledge of exceptions of the upper level components.
   /// For this particular implementation \ref SimplifyMessageString() is called.
   /// In many cases, AsString and AsSimplifiedString will return the same string.
   ///
   /// \see AsString
   ///
   virtual MStdString AsSimplifiedString() const;

   /// Attempt to simplify the error message string given.
   ///
   /// This is an overloaded service that makes use of knowledge of exceptions of the upper level components.
   /// Extra information suitable to experts is removed to make the message more readable
   /// to casual users.
   ///
   /// \param message Error message text to simplify.
   ///
   /// \return A simplified message, or the same message as given if no simplification is possible.
   ///
   /// \see AsSimplifiedString()
   ///
   static MStdString SimplifyMessageString(const MStdString& message);

   /// Return the string that specifies extended information about exception.
   /// There is also AsSimplifiedString, which might, or might not do something
   /// to simplify string information, so it is useful for an end user.
   ///
   /// \see AsSimplifiedString
   ///
   virtual MStdString AsString() const;

   /// Returns the message that is stored in this exception object.
   /// Used in serialization.
   ///
   MStdString GetMessageString() const
   {
      return m_message;
   }

   /// Sets the new message to exception.
   /// Used in serialization.
   ///
   void SetMessageString(const MStdString& msg)
   {
      m_message = msg;
   }

   /// Return a string that represents the exception error kind.
   /// This call is useful for example for showing a dialog box with an error message,
   /// in which case, this would be a heading of the dialog.
   ///
   MStdString GetKindAsString() const;

   /// Return a string that represents the exception error kind using kind as parameter.
   /// This is a static version of GetKindAsString.
   ///
   static MStdString GetKindAsStringStatic(KindType kind);

   /// Get the file name where the error took place, if this information is available.
   ///
   MStdString GetFileName() const
   {
      return m_fileNameAndLineNumber.GetFileName();
   }

   /// Get the line number within file where the error took place.
   ///
   unsigned GetFileLineNumber() const
   {
      return m_fileNameAndLineNumber.GetFileLineNumber();
   }

   ///@{
   /// Get file name and line number where the compile error took place.
   ///
   MFileNameAndLineNumber GetFileNameAndLineNumber() const
   {
      return m_fileNameAndLineNumber;
   }
   void SetFileNameAndLineNumber(MFileNameAndLineNumber fl)
   {
      m_fileNameAndLineNumber = fl;
   }
   ///@}

   /// Set file name and line number where the compile error took place,
   /// if the exception does not have this information already.
   ///
   void UpdateFileNameAndLineNumber(MFileNameAndLineNumber fl)
   {
      if ( m_fileNameAndLineNumber.IsEmpty() )
         m_fileNameAndLineNumber = fl;
   }

   /// If not set already, set file name and line number associated with this exception.
   ///
   /// The method is a convenient shortcut that can be put in every exception catcher
   /// that has context of a file and line.
   /// Also, this call updates the stack associated with this exception.
   ///
   /// \see AddFileAndLineToStack - this call only adds file and line context to stack associated with exception.
   ///
   void UpdateFileAndLine(const MStdString& file, unsigned line) M_NO_THROW;

   /// Add file name and line number context associated with this exception into stack.
   ///
   /// The method is a convenient shortcut that can be put in every exception catcher
   /// that has context of a file and line.
   ///
   /// \see UpdateFileAndLine - This call not only updates the stack, but also sets file and line information of the exception
   ///                          if it was not set previously.
   ///
   void AddFileAndLineToStack(const MStdString& file, unsigned line) M_NO_THROW;

   /// Add file name and line number context associated with this exception into stack.
   ///
   /// The method is a convenient shortcut that can be put in every exception catcher
   /// that has context of a file and line.
   ///
   /// \see UpdateFileAndLine - This call not only updates the stack, but also sets file and line information of the exception
   ///                          if it was not set previously.
   /// \see AddFileAndLineToStack - version that takes file as string and line as number.
   ///
   void AddFileNameAndLineNumberToStack(MFileNameAndLineNumber fl) M_NO_THROW;

   /// Get the stack of file names and line numbers.
   ///
   const MFileNameAndLineNumber::VectorType& GetStack() const
   {
      return m_stack;
   }

   /// Get the stack of file names and line numbers as collection of strings.
   ///
   MStdStringVector GetCallStack() const;

   /// Append the specified string to the end of the exception string.
   /// This service is typically used for the tracing the exception
   /// code. For that someone has to catch MException, issue
   /// Append, and throw it again to the upper level.
   /// Also look at AppendToString for appending message codes.
   /// 
   void Append(const MStdString&) M_NO_THROW;

   ///@{
   /// Append the message specified by the code given to the
   /// current error string of the exception.
   /// This service is typically used for the tracing the exception
   /// code. For that someone has to catch MException, issue
   /// AppendToString, and throw it again to the upper level.
   /// Also look at Append for appending whole strings.
   /// 
   void AppendToString(MConstLocalChars str, ...) M_NO_THROW;
   void AppendToString(const char* str, ...) M_NO_THROW;
   ///@}

   /// Prepend the specified string before the beginning of the exception string.
   /// This service is typically used for the tracing the exception
   /// code. For that someone has to catch MException, issue
   /// Prepend, and throw it again to the upper level.
   /// Also look at PrependBeforeString for appending message codes.
   /// 
   void Prepend(const MStdString&) M_NO_THROW;

   ///@{
   /// Prepend the message specified by the code given before the
   /// current error string of the exception.
   /// This service is typically used for the tracing the exception
   /// code. For that someone has to catch MException, issue
   /// PrependBeforeString, and throw it again to the upper level.
   /// 
   void PrependBeforeString(MConstLocalChars str, ...) M_NO_THROW;
   void PrependBeforeString(const char* str, ...) M_NO_THROW;
   ///@}

   /// Make the given variant suitable for showing in error message.
   ///
   /// This is a static method, use it to convert individual parameters.
   /// Representation of the given variant will not exceed MException::MaximumVisibleParameterLength characters,
   /// and all non-printables in the given string will be C-escaped.
   ///
   /// \param v Value to visualize for showing in the error message.
   ///
   /// \return The result string, truncated and escaped if necessary.
   ///
   static MStdString VisualizeParameter(const MVariant& v) M_NO_THROW;

   /// Make the given variant parameter suitable for showing in error message.
   ///
   /// This is a static method, use it to convert individual parameters in the error message.
   /// The given variant will be converted into string, truncated if longer than 40 characters,
   /// and all non-printables will be C-escaped.
   ///
   /// \param buff Buffer of at least MException::MaximumVisibleParameterLength bytes long where the string will be constructed.
   ///        If more than one parameter in the message has to be converted, they all shall have different buff.
   /// \param v Value to visualize for showing in the error message.
   ///
   /// \return The result zero terminated string buffer, truncated and escaped if necessary, built in the supplied buff parameter.
   ///
   /// \see VisualizeParameter the method that returns a string.
   ///
   static char* VisualizeVariantParameter(char* buff, const MVariant& v) M_NO_THROW;

   /// Make the given string parameter suitable for showing in error message.
   ///
   /// This is a static method, use it to convert individual parameters in the error message.
   /// The given string will be truncated, if longer than 40 characters,
   /// and all non-printables in the given string will be C-escaped.
   ///
   /// \param buff Buffer of at least MException::MaximumVisibleParameterLength bytes long where the string will be constructed.
   ///        If more than one parameter in the message has to be converted, they all shall have different buff.
   /// \param v Value to visualize for showing in the error message.
   ///
   /// \return The result zero terminated string buffer, truncated and escaped if necessary, built in the supplied buff parameter.
   ///
   /// \see VisualizeParameter the method that returns a string.
   ///
   static char* VisualizeStringParameter(char* buff, const MStdString& v) M_NO_THROW;

   /// Make the given zero terminated string parameter suitable for showing in error message.
   ///
   /// This is a static method, use it to convert individual parameters in the error message.
   /// The given string will be truncated, if longer than MException::MaximumVisibleParameterLength characters,
   /// and all non-printables in the given string will be C-escaped.
   ///
   /// \param buff Buffer of at least MException::MaximumVisibleParameterLength bytes long where the string will be constructed.
   ///        If more than one parameter in the message has to be converted, they all shall have different buff.
   /// \param v Value to visualize for showing in the error message.
   ///
   /// \return The result zero terminated string buffer, truncated and escaped if necessary, built in the supplied buff parameter.
   ///
   /// \see VisualizeParameter the method that returns a string.
   ///
   static char* VisualizeCharsParameter(char* buff, const char* v) M_NO_THROW;

   /// Make the given not zero terminated character parameter suitable for showing in error message.
   ///
   /// This is a static method, use it to convert individual parameters in the error message.
   /// The given string will be truncated, if longer than MException::MaximumVisibleParameterLength characters,
   /// and all non-printables in the given string will be C-escaped.
   ///
   /// \param buff Buffer of at least MException::MaximumVisibleParameterLength bytes long where the string will be constructed.
   ///        If more than one parameter in the message has to be converted, they all shall have different buff.
   /// \param v Value to visualize for showing in the error message, not zero terminated.
   /// \param len Byte size of v.
   ///
   /// \return The result zero terminated string buffer, truncated and escaped if necessary, built in the supplied buff parameter.
   ///
   /// \see VisualizeParameter the method that returns a string.
   ///
   static char* VisualizeCharsParameter(char* buff, const char* v, unsigned len) M_NO_THROW;

#else

   ///@{
   /// Throws this exception with the parameter given.
   /// Analog of the following code:
   /// \code
   ///     throw MException(code);
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   ///
   static M_NORETURN_FUNC void Throw(MErrorEnum::Type code);
   ///@}

   ///@{
   /// Throws this exception with the parameters given.
   /// Analog of the following code:
   /// \code
   ///     throw MException(kind, code);
   /// \endcode
   ///
   /// but saves memory, if there are many throws in the source code.
   ///
   static M_NORETURN_FUNC void Throw(KindType kind, MErrorEnum::Type code);
   ///@}

#endif //!M_NO_VERBOSE_ERROR_INFORMATION

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

public:

#if !M_NO_VERBOSE_ERROR_INFORMATION
   /// Constant string "Item '%s' is unknown", shared through many libraries.
   ///
   static const char s_itemIsUnknownErrorString[];
#endif

protected: // Attributes:

   /// Message code for this exception.
   ///
   MErrorEnum::Type m_code;

   /// The kind type for this exception.
   ///
   KindType m_kind;

#if !M_NO_VERBOSE_ERROR_INFORMATION

   /// Message string that represents this exception.
   ///
   MStdString m_message;

   /// File name and line number where the compile error took place.
   /// This can be zero, meaning no information is available.
   ///
   MFileNameAndLineNumber m_fileNameAndLineNumber;

   /// Stack of file names and line numbers.
   ///
   MFileNameAndLineNumber::VectorType m_stack;

#endif // !M_NO_VERBOSE_ERROR_INFORMATION

   M_DECLARE_CLASS(Exception)
};

/// System-related exception that is thrown in case
/// some system service fails. It uses the last error
/// information that is available within the majority of
/// operating systems.
///
class M_CLASS MESystemError : public MException
{
protected: // Constructor and initialization:

   /// Constructor suitable for serialization of the exception.
   ///
   MESystemError();

#if (M_OS & M_OS_WINDOWS) != 0

   /// Constructor that takes system error code and whether this is an errno.
   ///
   /// \param error System error code.
   /// \param isErrno If the error is essentially an errno.
   ///
   MESystemError(unsigned error, bool isErrno = false);

#else

   /// Constructor that takes system error code.
   ///
   /// \param error System error code.
   ///
   MESystemError(unsigned error);

#endif

public: // Constructors, Destructor, services:

   /// Copy constructor.
   ///
   /// \pre It is assumed, that the exception given as parameter is
   /// a valid system exception with valid parameters. In any case, the
   /// current exception has the same set of attributes as the one given.
   ///
   MESystemError(const MESystemError& ex);

   /// Class destructor.
   ///
   virtual ~MESystemError() M_NO_THROW;

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
   MESystemError& operator=(const MESystemError& ex)
   {
      if ( &ex != this )
      {
         m_systemErrorCode = ex.m_systemErrorCode;
         MException::operator=(ex);
      }
      return *this;
   }

   /// Clear the global system error for this thread. This call is here for convenience.
   ///
   static void ClearGlobalSystemError()
   {
      #if M_OS & M_OS_WINDOWS
         ::SetLastError(ERROR_SUCCESS);
      #elif M_OS & M_OS_CMX
      #else
         errno = 0;
      #endif
   }

   /// Return the last error from the system.
   ///
   static unsigned GetLastGlobalSystemError()
   {
      #if M_OS & M_OS_WINDOWS
         return ::GetLastError();
      #elif M_OS & M_OS_CMX
         return 0;
      #else
         return errno;
      #endif
   }

#if !M_NO_VERBOSE_ERROR_INFORMATION

#if (M_OS & M_OS_WINDOWS) != 0
   /// Return error message made from a given system error.
   ///
   static MStdString MessageFromSystemError(unsigned error, bool isErrno = false);
#else
   /// Return error message made from a given system error.
   ///
   static MStdString MessageFromSystemError(unsigned error);
#endif

   /// Throw the MESystemError with the code given as parameter.
   /// If any extra text is given, it is prepended before string with message.
   /// 
   /// \pre It is assumed that the type is one associated
   /// with system error. The additional parameters given to this
   /// service should match the code string. No checks are done.
   ///
   static M_NORETURN_FUNC void Throw(unsigned error, MConstChars extraMessageText = NULL);

#else

   /// Throw the MESystemError with the code given as parameter.
   ///
   /// \pre It is assumed that the type is one associated
   /// with system error. The additional parameters given to this
   /// service should match the code string. No checks are done.
   ///
   static M_NORETURN_FUNC void Throw(unsigned error);

#endif

   /// Throw the last occurred MESystemError.
   ///
   static M_NORETURN_FUNC void ThrowLastSystemError();

#if !M_NO_VERBOSE_ERROR_INFORMATION
   /// Throw the last occurred MESystemError, combined together with the extra message.
   ///
   static M_NORETURN_FUNC void ThrowLastSystemError(const MStdString& extraMessage);
#endif

   /// If isError is true, check for the last system error and throw it.
   ///
   static void CheckLastSystemError(bool isError);

#if !M_NO_FILESYSTEM
   /// Throws this exception as file not open.
   ///
   static M_NORETURN_FUNC void ThrowFileNotOpen(const MStdString& fileName);

   /// Throws file IO error.
   ///
   static M_NORETURN_FUNC void ThrowInputOutputError(const MStdString& fileName);

#endif

   /// Get the system error code, whatever was returned by the operating system.
   ///
   unsigned GetSystemErrorCode() const
   {
      return m_systemErrorCode;
   }

   /// Throw the MESystemError with the code, received from last global system error.
   ///
   static void VerifySystemError(unsigned error);

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

   /// Create a new exception of this type, all parameters are clear.
   ///
   static MESystemError* New();

protected: // Attributes:

   /// The exact socket error code.
   ///
   unsigned m_systemErrorCode;

#if (M_OS & M_OS_WINDOWS) != 0
   /// Is this error is an errno.
   ///
   bool m_isErrno;
#endif

   M_DECLARE_CLASS(ESystemError)
};

#if !M_NO_SOCKETS

/// Sockets exception that is thrown in case of the sockets failure. 
///
/// It uses the sockets last error information that 
/// is available within the majority of operating systems.
///
class M_CLASS MESocketError : public MException
{
public:

   /// Exception default constructor.
   ///
   MESocketError();

   /// Copy constructor.
   ///
   /// \pre It is assumed, that the exception given as parameter is
   /// a valid socket exception with valid parameters. In any case the
   /// current exception has the same set of attributes as one given.
   ///
   MESocketError(const MESocketError&);

   /// Class destructor.
   ///
   virtual ~MESocketError() M_NO_THROW;

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
   MESocketError& operator=(const MESocketError& ex)
   {
      if ( &ex != this )
      {
         m_socketErrorCode = ex.m_socketErrorCode;
         MException::operator=(ex);
      }
      return *this;
   }

   /// Get socket error code, system error.
   ///
   unsigned GetSocketErrorCode() const
   {
      return m_socketErrorCode;
   }

   ///@{
   /// Throw the MESocketError with the code provided as parameter.
   /// This convenience service allows to save some executable file size,
   /// as this exception is thrown from a lot of places, and the executable
   /// code for throw operator and calling the constructor is quite big.
   /// 
   /// Calling this service is the same as the following:
   /// \code
   ///       throw MESocketError(code);
   /// \endcode
   ///
   /// \pre It is assumed that the type is one associated
   /// with socket error. The additional parameters given to this
   /// service should match the code string. No checks are done.
   ///
#if !M_NO_VERBOSE_ERROR_INFORMATION
   static M_NORETURN_FUNC void ThrowSocketError(unsigned socketError, MErrorEnum::Type code, MConstLocalChars str);
   static M_NORETURN_FUNC void ThrowSocketError(unsigned socketError, MErrorEnum::Type code, MConstChars str);
#else
   static M_NORETURN_FUNC void ThrowSocketError(unsigned socketError, MErrorEnum::Type code);
#endif
   static M_NORETURN_FUNC void ThrowSocketError(unsigned socketError);
   static M_NORETURN_FUNC void ThrowLastSocketError();
   static M_NORETURN_FUNC void ThrowSocketReadTimeout();
   static M_NORETURN_FUNC void ThrowSocketWriteTimeout();
   static M_NORETURN_FUNC void ThrowSocketErrorFromReturnValue(int returnValue);
   ///@}

   /// Access the last global socket error.
   static unsigned GetLastGlobalSocketError();

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

   /// Create a new exception of this type, all parameters are clear.
   ///
   static MESocketError* New();

private:

#if !M_NO_VERBOSE_ERROR_INFORMATION
   static void Throw(unsigned, MErrorEnum::Type, MConstLocalChars);
#endif

protected: // Attributes:

   /// The exact socket error code.
   ///
   unsigned m_socketErrorCode;

   M_DECLARE_CLASS(ESocketError)
};

#endif // !M_NO_SOCKETS

/// Exception which is thrown in case the operation is canceled.
/// (typically by some asynchronous user request).
/// Generally speaking, this is not an error.
///
class M_CLASS MEOperationCancelled : public MException
{
public:

   /// Constructor.
   ///
   MEOperationCancelled();

   /// Copy constructor.
   ///
   MEOperationCancelled(const MEOperationCancelled&);

   /// Destructor.
   ///
   virtual ~MEOperationCancelled() M_NO_THROW;

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
   MEOperationCancelled& operator=(const MEOperationCancelled& ex)
   {
      if ( &ex != this )
         MException::operator=(ex);
      return *this;
   }

   /// Throws this exception with the parameters given.
   /// Analog of the following code:
   /// \code
   ///     throw MEOperationCancelled();
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   /// The following is the usage:
   /// \code
   ///     MEOperationCancelled::Throw();
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

   /// Create a new exception of this type, all parameters are clear.
   ///
   static MEOperationCancelled* New();

   M_DECLARE_CLASS(EOperationCancelled)
};

/// Exception, number is out of range.
///
/// \anchor NumberOutOfRangeText
/// The text of the message of the exception depends on the exception parameters/properties.
/// English texts for parameters 'val', 'name', 'lo' and 'hi' are:
///   - "Value 'val' is out of range" -
///     when the range is invalid (high bound is lower than low) and the item name is not given.
///   - "Value 'val' for 'name' is out of range" -
///     when the range is invalid (high bound is lower than low) and there is an item name.
///   - "Value 'val' is out of range 'lo' .. 'hi'" - range is valid, no item name.
///   - "Value 'val' for 'name' is out of range 'lo' .. 'hi'" - range is valid and item name is given.
///
class M_CLASS MENumberOutOfRange : public MException
{
public: // Constructors, destructor:

   /// Constructor that takes all properties as parameters.
   ///
   /// The text thrown by this exception depends on the parameters given,
   /// as described \ref NumberOutOfRangeText "here".
   ///
   /// \param lo Low bound of the range, inclusive.
   /// \param hi High bound of the range, inclusive.
   ///    It is not an error to have the value of hi smaller than low,
   ///    but in this case the range will not be included into the error message.
   /// \param val Offending out of range value.
   ///    There is a debug time assert that checks if the given value
   ///    is indeed out of range determined by inclusive range lo .. hi.
   /// \param itemName Optional name of the item, typically the array name.
   ///    If absent, the name will not be reported.
   ///
   MENumberOutOfRange(double lo, double hi, double val, MConstChars itemName = NULL);

   /// Copy constructor.
   ///
   /// \param ex
   ///     Exception from which to create this object.
   ///
   MENumberOutOfRange(const MENumberOutOfRange& ex);

   /// Destructor.
   ///
   virtual ~MENumberOutOfRange() M_NO_THROW;

public: // Helper services:

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
   MENumberOutOfRange& operator=(const MENumberOutOfRange& ex)
   {
      if ( &ex != this )
      {
         m_hi = ex.m_hi;
         m_lo = ex.m_lo;
         m_value = ex.m_value;
      #if !M_NO_VERBOSE_ERROR_INFORMATION
         m_name = ex.m_name;
      #endif
         MException::operator=(ex);
      }
      return *this;
   }

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

public: // Static methods that throw this exception:

   ///@{
   /// Throw an out of range exception with a single value.
   /// 
   /// The English text of the exception message will be: "Value 'val' is out of range".
   ///
   /// \param val Offending out of range value.
   ///
   static M_NORETURN_FUNC void ThrowValue(double val);
   static M_NORETURN_FUNC void Throw(double val)
   {
      ThrowValue(val);
   }
   ///@}

   ///@{
   /// Throw an out of range exception with a single value and name.
   ///
   /// The English text of the exception message will be:
   ///    "Value 'val' for 'itemName' is out of range".
   ///
   /// \param val Offending out of range value.
   /// \param itemName Optional name of the item, typically the array name.
   ///    If absent, the name will not be reported.
   ///
   static M_NORETURN_FUNC void ThrowNamedValue(double val, MConstChars itemName);
   static M_NORETURN_FUNC void Throw(double val, MConstChars itemName)
   {
      ThrowNamedValue(val, itemName);
   }
   ///@}

   ///@{
   /// Throw an out of range exception with the value and range.
   ///
   /// The English version of the text thrown by this method is
   ///   - "Value 'val' is out of range 'lo' .. 'hi'".
   ///
   /// \param lo Low bound of the range, inclusive.
   /// \param hi High bound of the range, inclusive.
   ///    It is not an error to have the value of hi smaller than low,
   ///    but in this case the range will not be included into the error message.
   /// \param val Offending out of range value.
   ///    There is a debug time assert that checks if the given value
   ///    is indeed out of range determined by inclusive range lo .. hi.
   ///
   static M_NORETURN_FUNC void ThrowRange(double lo, double hi, double val);
   static M_NORETURN_FUNC void Throw(double lo, double hi, double val)
   {
      ThrowRange(lo, hi, val);
   }
   ///@}

   ///@{
   /// Throw an out of range exception with the value, range, and name.
   ///
   /// The English version of the text thrown by this method is
   ///   - "Value 'val' for 'itemName' is out of range 'lo' .. 'hi'".
   ///
   /// \param lo Low bound of the range, inclusive.
   /// \param hi High bound of the range, inclusive.
   ///    It is not an error to have the value of hi smaller than low,
   ///    but in this case the range will not be included into the error message.
   /// \param val Offending out of range value.
   ///    There is a debug time assert that checks if the given value
   ///    is indeed out of range determined by inclusive range lo .. hi.
   /// \param itemName Optional name of the item, typically the array name.
   ///    If absent, the name will not be reported.
   ///
   static M_NORETURN_FUNC void ThrowNamedRange(double lo, double hi, double val, MConstChars itemName);
   static M_NORETURN_FUNC void Throw(double lo, double hi, double val, MConstChars itemName)
   {
      ThrowNamedRange(lo, hi, val, itemName);
   }
   ///@}

public: // Static methods that check the range and throw this exception if the value does not fit:

   ///@{
   /// Checks whether the double precision value is in the given range, and throws an error if it is not.
   ///
   /// \pre The value shall be in the given range, or an exception is thrown.
   ///
   /// \param lo Low bound of the range, inclusive.
   /// \param hi High bound of the range, inclusive.
   ///    It is not an error to have the value of hi smaller than low,
   ///    but in this case the range will not be included into the error message.
   ///    Obviously, in this case the check will always fail
   ///    and an exception will always be thrown.
   /// \param val The value to test.
   ///
   static void CheckRange(double lo, double hi, double val);
   static void Check(double lo, double hi, double val)
   {
      CheckRange(lo, hi, val);
   }
   ///@}

   ///@{
   /// Checks whether the named double precision value is in the given range, and throws an error if it is not.
   ///
   /// \pre The value shall be in the given range, or an exception is thrown.
   ///
   /// \param lo Low bound of the range, inclusive.
   /// \param hi High bound of the range, inclusive.
   ///    It is not an error to have the value of hi smaller than low,
   ///    but in this case the range will not be included into the error message.
   ///    Obviously, in this case the check will always fail
   ///    and an exception will always be thrown.
   /// \param val The value to test.
   /// \param itemName Name of the item, typically the array name.
   ///    If absent, the name will not be reported.
   ///
   static void CheckNamedRange(double lo, double hi, double val, MConstChars itemName);
   static void Check(double lo, double hi, double val, MConstChars itemName)
   {
      CheckNamedRange(lo, hi, val, itemName);
   }
   ///@}

   ///@{
   /// Checks whether the integer value is in the given range, and throws an error if it is not.
   ///
   /// \pre The value shall be in the given range, or an exception is thrown.
   ///
   /// \param lo Low bound of the range, inclusive.
   /// \param hi High bound of the range, inclusive.
   ///    It is not an error to have the value of hi smaller than low,
   ///    but in this case the range will not be included into the error message.
   ///    Obviously, in this case the check will always fail
   ///    and an exception will always be thrown.
   /// \param val The value to test.
   ///
   static void CheckIntegerRange(int lo, int hi, int val);
   static void CheckInteger(int lo, int hi, int val)
   {
      CheckIntegerRange(lo, hi, val);
   }
   ///@}

   ///@{
   /// Checks whether the named integer value is in the given range, and throws an error if it is not.
   ///
   /// \pre The value shall be in the given range, or an exception is thrown.
   ///
   /// \param lo Low bound of the range, inclusive.
   /// \param hi High bound of the range, inclusive.
   ///    It is not an error to have the value of hi smaller than low,
   ///    but in this case the range will not be included into the error message.
   ///    Obviously, in this case the check will always fail
   ///    and an exception will always be thrown.
   /// \param val The value to test.
   /// \param itemName Name of the item, typically the array name.
   ///    If absent, the name will not be reported.
   ///
   static void CheckNamedIntegerRange(int lo, int hi, int val, MConstChars itemName);
   static void CheckInteger(int lo, int hi, int val, MConstChars itemName)
   {
      CheckNamedIntegerRange(lo, hi, val, itemName);
   }
   ///@}

   /// Checks whether the unsigned integer value is in the given range, and throws an error if it is not.
   ///
   /// \pre The value shall be in the given range, or an exception is thrown.
   ///
   /// \param lo Low bound of the range, inclusive.
   /// \param hi High bound of the range, inclusive.
   ///    It is not an error to have the value of hi smaller than low,
   ///    but in this case the range will not be included into the error message.
   ///    Obviously, in this case the check will always fail
   ///    and an exception will always be thrown.
   /// \param val The value to test.
   ///
   static void CheckUnsignedRange(unsigned lo, unsigned hi, unsigned val);

   /// Checks whether the named unsigned integer value is in the given range, and throws an error if it is not.
   ///
   /// \pre The value shall be in the given range, or an exception is thrown.
   ///
   /// \param lo Low bound of the range, inclusive.
   /// \param hi High bound of the range, inclusive.
   ///    It is not an error to have the value of hi smaller than low,
   ///    but in this case the range will not be included into the error message.
   ///    Obviously, in this case the check will always fail
   ///    and an exception will always be thrown.
   /// \param val The value to test.
   /// \param itemName Name of the item, typically the array name.
   ///    If absent, the name will not be reported.
   ///
   static void CheckNamedUnsignedRange(unsigned lo, unsigned hi, unsigned val, MConstChars itemName);

   /// Checks whether the named unsigned 64-bit integer value is in the given range, and throws an error if it is not.
   ///
   /// \pre The value shall be in the given range, or an exception is thrown.
   ///
   /// \param lo Low bound of the range, inclusive.
   /// \param hi High bound of the range, inclusive.
   ///    It is not an error to have the value of hi smaller than low,
   ///    but in this case the range will not be included into the error message.
   ///    Obviously, in this case the check will always fail
   ///    and an exception will always be thrown.
   /// \param val The value to test.
   /// \param itemName Name of the item, typically the array name.
   ///    If absent, the name will not be reported.
   ///
   static void CheckNamedUInt64Range(Muint64 lo, Muint64 hi, Muint64 val, MConstChars itemName = NULL);

   /// Checks whether the named unsigned long integer value is in the given range, and throws an error if it is not.
   ///
   /// \pre The value shall be in the given range, or an exception is thrown.
   ///
   /// \param lo Low bound of the range, inclusive.
   /// \param hi High bound of the range, inclusive.
   ///    It is not an error to have the value of hi smaller than low,
   ///    but in this case the range will not be included into the error message.
   ///    Obviously, in this case the check will always fail
   ///    and an exception will always be thrown.
   /// \param val The value to test.
   /// \param itemName Name of the item, typically the array name.
   ///    If absent, the name will not be reported.
   ///
   static void CheckNamedUnsignedLongRange(unsigned long lo, unsigned long hi, unsigned long val, MConstChars itemName = NULL)
   {
      #if UINT_MAX == ULONG_MAX
         CheckNamedUnsignedRange(static_cast<unsigned>(lo), static_cast<unsigned>(hi), static_cast<unsigned>(val), itemName);
      #else
         CheckNamedUInt64Range(static_cast<Muint64>(lo), static_cast<Muint64>(hi), static_cast<Muint64>(val), itemName);
      #endif
   }

   /// Create a new exception of this type, all parameters are clear.
   ///
   static MENumberOutOfRange* New();

public: // Property accessors:

   ///@{
   /// Maximum value of exception-guided range.
   ///
   /// When this value is lower than the minimum range value the range is not included
   /// into the error message.
   ///
   double GetRangeMax() const
   {
      return m_hi;
   }
   void SetRangeMax(double hi)
   {
      m_hi = hi;
   }
   ///@}

   ///@{
   /// Minimum value of exception-guided range.
   ///
   /// When this value is bigger than the maximum range value the range is not included
   /// into the error message.
   ///
   double GetRangeMin() const
   {
      return m_lo;
   }
   void SetRangeMin(double lo)
   {
      m_lo = lo;
   }
   ///@}

   ///@{
   /// Value that is outside of the exception-guided range.
   ///
   double GetValue() const
   {
      return m_value;
   }
   void SetValue(double value)
   {
      m_value = value;
   }
   ///@}

#if !M_NO_VERBOSE_ERROR_INFORMATION
   ///@{
   /// Name of the item that exceeds exception-guided range.
   ///
   const MStdString& GetName() const
   {
      return m_name;
   }
   void SetName(const MStdString& name)
   {
      m_name = name;
   }
   ///@}
#endif

private: // class has the attributes to save

   // Low value of the range
   //
   double m_lo;

   // High value of the range
   //
   double m_hi;

   // Value itself that failed the boundaries
   //
   double m_value;

#if !M_NO_VERBOSE_ERROR_INFORMATION

   // Name of the item, if the name was given
   //
   MStdString m_name;

#endif

   M_DECLARE_CLASS(ENumberOutOfRange)
};

/// Exception, array index is out of range.
///
/// \anchor IndexOutOfRangeText
/// The text of the message of the exception depends on the exception parameters/properties.
/// English texts for parameters 'val', 'name', 'lo' and 'hi' are:
///  - "Index val for 'name' is out of range lo .. hi" when the name is given.
///  - "Index %d is out of range %d .. %d" when there is no name.
/// It is not an error to have the value of hi smaller than low,
/// and the error message might read 0 .. -1 for an empty array.
///
class M_CLASS MEIndexOutOfRange : public MException
{
public:

   /// Constructor that takes all properties as parameters.
   ///
   /// The text thrown by this exception depends on the parameters given,
   /// as described \ref IndexOutOfRangeText "here".
   ///
   /// \param lo Low bound of the array, inclusive.
   /// \param hi High bound of the array, inclusive.
   ///    It is not an error to have the value of hi smaller than low,
   ///    and the error message might read 0 .. -1 for an empty array.
   /// \param val Offending out of range value.
   ///    There is a debug time assert that checks if the given value
   ///    is indeed out of range determined by inclusive range lo .. hi.
   /// \param arrayName Optional name of the array.
   ///    If absent, the name will not be reported.
   ///
   MEIndexOutOfRange(int lo, int hi, int val, MConstChars arrayName = NULL);

   /// Copy constructor.
   ///
   /// \param ex
   ///     Exception from which to create this object.
   ///
   MEIndexOutOfRange(const MEIndexOutOfRange& ex);

   /// Destructor.
   ///
   virtual ~MEIndexOutOfRange() M_NO_THROW;

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
   MEIndexOutOfRange& operator=(const MEIndexOutOfRange& ex)
   {
      if ( &ex != this )
      {
         m_hi = ex.m_hi;
         m_lo = ex.m_lo;
         m_value = ex.m_value;
         MException::operator=(ex);
      }
      return *this;
   }

public: // Static methods:

   ///@{
   /// Throws this exception with the range and the offending index.
   ///
   /// Analog of the following code:
   /// \code
   ///     throw MEIndexOutOfIndex(lo, hi, val);
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   /// 
   /// \param lo Low bound of the array, inclusive.
   /// \param hi High bound of the array, inclusive.
   ///    It is not an error to have the value of hi smaller than low,
   ///    and the error message might read 0 .. -1 for an empty array.
   /// \param val Offending out of range value.
   ///    There is a debug time assert that checks if the given value
   ///    is indeed out of range determined by inclusive range lo .. hi.
   ///
   static M_NORETURN_FUNC void ThrowIndex(int lo, int hi, int val);
   static M_NORETURN_FUNC void Throw(int lo, int hi, int val)
   {
      ThrowIndex(lo, hi, val);
      M_ENSURED_ASSERT(0);
   }
   ///@}

   ///@{
   /// Throws this exception with the range and the offending index.
   ///
   /// Analog of the following code:
   /// \code
   ///     throw MEIndexOutOfIndex(lo, hi, val);
   /// \endcode
   /// but saves memory, if there are many throws in the source code.
   ///
   /// \param lo Low bound of the array, inclusive.
   /// \param hi High bound of the array, inclusive.
   ///    It is not an error to have the value of hi smaller than low,
   ///    and the error message might read 0 .. -1 for an empty array.
   /// \param val Offending out of range value.
   ///    There is a debug time assert that checks if the given value
   ///    is indeed out of range determined by inclusive range lo .. hi.
   /// \param arrayName Optional name of the item, typically the array name.
   ///    If absent, the name will not be reported.
   ///
   static M_NORETURN_FUNC void ThrowNamedIndex(int lo, int hi, int val, MConstChars arrayName);
   static M_NORETURN_FUNC void Throw(int lo, int hi, int val, MConstChars arrayName)
   {
      ThrowNamedIndex(lo, hi, val, arrayName);
      M_ENSURED_ASSERT(0);
   }
   ///@}

   ///@{
   /// Checks whether the value is in the given index range, and throws an error if it is not.
   ///
   /// \pre The value shall be within the given range, or an exception is thrown.
   ///
   /// \param lo Lower index bound such as 0.
   /// \param hi Upper index bound such as arraySize - 1.
   /// \param val Value of the index that should be within the given bounds, inclusively.
   ///
   static void CheckIndex(int lo, int hi, int val);
   static void Check(int lo, int hi, int val)
   {
      CheckIndex(lo, hi, val);
   }
   ///@}

   ///@{
   /// Checks whether the named value is in the given index range, and throws an error if it is not.
   ///
   /// \pre The value shall be within the given range, or an exception is thrown.
   ///
   /// \param lo Lower index bound such as 0.
   /// \param hi Upper index bound such as arraySize - 1.
   /// \param val Value of the index that should be within the given bounds, inclusively.
   /// \param arrayName Array name to report at failure.
   ///
   static void CheckNamedIndex(int lo, int hi, int val, MConstChars arrayName);
   static void Check(int lo, int hi, int val, MConstChars arrayName)
   {
      CheckNamedIndex(lo, hi, val, arrayName);
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

public: // Semi-public reflected services:

   /// Create a new exception of this type, all parameters are clear.
   ///
   static MEIndexOutOfRange* New();

public: // Property accessors:

   ///@{
   /// Minimum value of exception-guided index.
   ///
   int GetIndexMin() const
   {
      return m_lo;
   }
   void SetIndexMin(int lo)
   {
      m_lo = lo;
   }
   ///@}

   ///@{
   /// Maximum value of exception-guided index.
   ///
   int GetIndexMax() const
   {
      return m_hi;
   }
   void SetIndexMax(int hi)
   {
      m_hi = hi;
   }
   ///@}

   ///@{
   /// Value that exceeds exception-guided range.
   ///
   int GetValue() const
   {
      return m_value;
   }
   void SetValue(int value)
   {
      m_value = value;
   }
   ///@}

private: // Properties:

   // Highest possible value of the index
   //
   int m_lo;

   // Highest possible value of the index
   //
   int m_hi;

   // Failed value
   //
   int m_value;

   M_DECLARE_CLASS(EIndexOutOfRange)
};

/// Exception, Error during mathematical operation.
///
/// These are raised by class MMath for its functions such as Sqrt
/// in case the parameters are wrong.
///
class M_CLASS MEMath : public MException
{
public:

   /// Constructor.
   ///
   MEMath();

   /// Copy constructor.
   ///
   MEMath(const MEMath&);

   /// Destructor.
   ///
   virtual ~MEMath() M_NO_THROW;

public: // Methods:

   /// Clear the math related error.
   ///
   /// Operating system and compiler dependent implementation will clear the
   /// flags responsible for math error checking, so the mathematical call can be performed.
   ///
   static void ClearMathError() M_NO_THROW;

   /// Prepare for checking of math function parameters.
   ///
   /// This has to be called before executing any math related operation.
   /// Operating system and compiler dependent implementation will clear the
   /// flags responsible for math error checking, so the mathematical call can be performed.
   ///
   /// \see \ref ClearMathError - underlying implementation of is call.
   ///
   static void BeforeDoingMath() M_NO_THROW
   {
      ClearMathError();
   }

   /// Check the result of the execution of math function.
   ///
   /// This is where the math error checking is done.
   /// Exception MEMath is raised in case the function had bad parameters.
   /// Bad parameters condition is when result is NaN and/or when errno is set.
   ///
   /// \param result Result of the operation, NaN will be reported as error.
   /// \param operationName Name of the operation which caused the error.
   ///
   static void AfterDoingMath(double result, const char* operationName = NULL);

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

   /// Create a new exception of this type, all parameters are clear.
   ///
   static MEMath* New();

   /// Helper reflection-enabling Throw service.
   ///
   static M_NORETURN_FUNC void Throw();

private: // Data:

   M_DECLARE_CLASS(EMath)
};

#if defined(_MSC_VER) && (M_OS & M_OS_WIN32_CE) == 0

/// Internal error that is never seen by the user application.
/// It is thrown and caught internally, then rethrown as MException.
///
/// \anchor MEProgramError_Guard_usage
/// Its usage is as such:
/// \code
///    {
///        MEProgramError::Guard guard; // initialize the guard
///        try
///        {
///            .... // any possible bad deeds here
///        }
///        catch ( MException& ex )
///        {
///            .... // handle normally
///        }
///        catch ( MEProgramError& ex )
///        {
///            ex.PassToGuard(guard);
///        }
///        guard.RethrowIfError(); // this has to be done outside of catch
///    }
/// \endcode
///
class M_CLASS MEProgramError
{
public: // types:

   /// The guard is used for enabling and disabling program error handling.
   /// See \ref MEProgramError_Guard_usage "this usage example".
   ///
   class M_CLASS Guard
   {
      friend class MEProgramError;

   public:

      Guard();

      ~Guard();

      void RethrowIfError();
      void InitializeException(MException* ex) M_NO_THROW;

      bool WasProgramError() const
      {
         return m_programErrorCode != 0;
      }

   private:

      unsigned                m_programErrorCode;
      MFileNameAndLineNumber  m_fileNameAndLineNumber;
      _se_translator_function m_savedTranslator;
   };

public:

   MEProgramError(unsigned programErrorCode)
   :
      m_programErrorCode(programErrorCode),
      m_fileNameAndLineNumber()
   {
   }

   MEProgramError(const MEProgramError& other)
   :
      m_programErrorCode(other.m_programErrorCode),
      m_fileNameAndLineNumber(other.m_fileNameAndLineNumber)
   {
   }

   ~MEProgramError()
   {
   }

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
   MEProgramError& operator=(const MEProgramError& ex)
   {
      if ( &ex != this )
      {
         m_programErrorCode = ex.m_programErrorCode;
         m_fileNameAndLineNumber = ex.m_fileNameAndLineNumber;
      }
      return *this;
   }

   void PassToGuard(Guard& guard)
   {
      guard.m_programErrorCode = m_programErrorCode;
      guard.m_fileNameAndLineNumber = m_fileNameAndLineNumber;
   }

   void UpdateFileNameAndLineNumber(MFileNameAndLineNumber fl)
   {
      if ( m_fileNameAndLineNumber.IsEmpty() )
         m_fileNameAndLineNumber = fl;
   }

   static void StackOverflowTester();

private:

   unsigned m_programErrorCode;
   MFileNameAndLineNumber m_fileNameAndLineNumber;
};

#else
/// \cond SHOW_INTERNAL

struct MEProgramError
{
   struct Guard
   {
      void RethrowIfError(){}
      void InitializeException(MException*) {M_ASSERT(0);}
      bool WasProgramError() {return false;}
   };

   void PassToGuard(Guard&){}

   static void StackOverflowTester() {}

#if !M_NO_VERBOSE_ERROR_INFORMATION
   void UpdateFileNameAndLineNumber(MFileNameAndLineNumber) {}
#endif
};

/// \endcond SHOW_INTERNAL
#endif

///@}
#endif
