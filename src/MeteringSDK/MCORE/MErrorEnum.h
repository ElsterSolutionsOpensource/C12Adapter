#ifndef MCORE_MERRORENUM_H
#define MCORE_MERRORENUM_H

//
// THIS FILE IS GENERATED, DO NOT EDIT!
// Instead edit library's *Errors.inc files and generate this file with bin/update_errors.py.
//

/// \addtogroup MCORE
///@{
/// \file MCORE/MErrorEnum.h
///
/// List of MeteringSDK error constants.
///
/// THIS FILE IS GENERATED, DO NOT EDIT!
/// Instead edit library's *Errors.inc files and generate this file with bin/update_errors.py.
///

#include <MCORE/MObject.h>

/// Error enumeration
///
class MErrorEnum : public MObject
{
public: // Types:

   /// Actual enumeration type.
   ///
   enum Type
   {
      // MCOREErrors.inc

      /// Unknown error
      ///
      /// Text: "Unknown error"
      ///
      /// This is most likely the hard unrecoverable application error such as General Protection Fault, etc.
      ///
      Unknown = 0x80040401,

      /// Bad conversion error
      ///
      ///  - Text: "Could not convert '%s' to a single character"
      ///  - Text: "Could not convert byte string %d bytes long to a single character"
      ///  - Text: "Could not convert byte string %d bytes long to integer, four bytes expected"
      ///  - Text: "Could not convert byte string %d bytes long to unsigned integer, four bytes expected"
      ///  - Text: "Could not convert byte string %d bytes long to floating point number, eight bytes expected"
      ///  - Text: "Could not convert '%s' to integer"
      ///  - Text: "Could not convert '%s' to unsigned integer"
      ///  - Text: "Could not convert '%s' to floating point number"
      ///  - Text: "Could not convert variant to object reference"
      ///  - Text: "Could not convert variant containing object reference to a character"
      ///  - Text: "Could not convert variant containing object reference to a numeric value"
      ///  - Text: "Could not convert variant containing object reference to a string value"
      ///  - Text: "Could not convert 64-bit unsigned integer to '%s'"
      ///  - Text: "Could not convert 64-bit signed integer to '%s'"
      BadConversion = 0x80040402,

      /// No value is present
      ///
      /// - Text: "No value exists"
      /// - Text: "No value given for '%s'"
      ///
      /// This would be thrown if a variable has no value.
      /// For example, this would signify the variable was not initialized previously.
      ///
      NoValue = 0x8004040A,

      /// Number is out of range.
      ///
      /// - Text: "Value %s for '%s' is out of range %s .. %s"
      ///
      NumberOutOfRange = 0x80040418,

      /// Index is out of range
      ///
      ///  - Text: "Index %d for '%s' is out of range %d .. %d"
      ///  - Text: "Index %d is out of range %d .. %d"
      ///
      IndexOutOfRange = 0x8004041A,

      /// String is too long
      ///
      ///  - Text: "Line %d in file '%s' is too long"
      ///  - Text: "String of %d characters is too long for '%s', which has size %d"
      ///
      StringTooLong = 0x80040420,

      /// Generic error in a client application
      ///
      /// - Text: "Error in a client application"
      ///
      /// This error is a default one thrown when no error code is given.
      /// Like when there is THROW "My Message", the error code of the exception will be this one.
      ///
      ClientApplicationError = 0x8004044D,

      /// Invalid or unsupported baud
      ///
      ///  - Text: "Invalid or unsupported baud rate %u"
      ///  - Text: "Meter requested invalid or unsupported baud rate with code 0x%X"
      ///
      InvalidBaud = 0x80040475,

      /// Unknown socket error
      ///
      ///  - Text: "Unknown socket error"
      ///
      UnknownSocketError = 0x8004047F,

      // MCOMErrors.inc

      /// Text: "Characters echoed did not match ones sent. Not a current loop device?"
      ///
      /// This error will result when the echo is set for the channel, 
      /// but the received characters did not match those which are sent.
      ///
      CharactersNotEchoed = 0x8004048B,

      /// Text: "CRC check failed"
      CrcCheckFailed = 0x8004048D,

      /// Text for this error does not exist
      ///
      /// This error code can be used to derive C12 service response from error code,
      /// as it will be much easier than parsing an error code from message text.
      /// This error can still occur in the future in a rare hypothetical condition at which an OK is actually an error.
      ///
      C12ServiceResponseOK = 0x80040490,

      /// Text: "0x%02X Error, no reason provided (ERR)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseERR = 0x80040491,

      /// Text: "0x%02X Service Not Supported (SNS)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseSNS = 0x80040492,

      /// Text: "0x%02X Insufficient Security Clearance (ISC)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseISC = 0x80040493,

      /// Text: "0x%02X Operation Not Possible (ONP)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseONP = 0x80040494,

      /// Text: "0x%02X Inappropriate Action Requested (IAR)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseIAR = 0x80040495,

      /// Text: "0x%02X Device Busy (BSY)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseBSY = 0x80040496,

      /// Text: "0x%02X Data Not Ready (DNR)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseDNR = 0x80040497,

      /// Text: "0x%02X Data Locked (DLK)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseDLK = 0x80040498,

      /// Text: "0x%02X Renegotiate (RNO)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseRNO = 0x80040499,

      /// Text: "0x%02X Invalid Service Sequence State (ISSS)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseISSS = 0x8004049A,

      /// Text: "0x%02X Security mechanism error detected (SME)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseSME = 0x8004049B,

      /// Text: "0x%02X Unknown or invalid Called ApTitle is received (UAT)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseUAT = 0x8004049C,

      /// Text: "0x%02X Network timeout detected (NETT)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseNETT = 0x8004049D,

      /// Text: "0x%02X Node is not reachable (NETR)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseNETR = 0x8004049E,

      /// Text: "0x%02X Request too large (RQTL)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseRQTL = 0x8004049F,

      /// Text: "0x%02X Response too large (RSTL)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseRSTL = 0x800404A0,

      /// Text: "0x%02X Segmentation required, but not possible (SGNP)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseSGNP = 0x800404A1,

      /// Text: "0x%02X Segmentation error (SGERR)"
      ///
      /// Thrown by C12 protocol service
      ///
      C12ServiceResponseSGERR = 0x800404A2,

      // MCOREErrors.inc

      /// No such property
      ///
      ///  - Text: "'%s' does not have property '%s'"
      ///
      NoSuchProperty = 0x800404B1,

      // MCOMErrors.inc

      /// Text: "Invalid checksum"
      InvalidChecksum = 0x800404B4,

      /// Text: "Channel read timeout (%u bytes read successfully)"
      ChannelReadTimeout = 0x800404C3,

      /// Text: "Could not connect by modem"
      /// Text: "Could not connect by modem (Line is busy)"
      /// Text: "Could not connect by modem (Timeout)"
      /// Text: "Could not connect by modem (No Dial Tone)"
      /// Text: "Could not connect by modem (No Answer)"
      /// Text: "Could not connect by modem (No Carrier)"
      CouldNotConnectByModem = 0x800404C4,

      /// Text: "Modem responded with error to command"
      /// Text: "Modem did not respond or gave an unknown response to command"
      ModemError = 0x800404C5,

      /// Text: ** no text exists for this procedure result **
      ///
      /// This error code can be used to derive C12 procedure result from error code,
      /// as it will be much easier than parsing an error code from message text.
      /// This error can still occur in the future in a rare hypothetical condition at which an OK is actually an error.
      ///
      C12ProcedureResultOK = 0x800404C9,

      /// Text: "Procedure result code 1, Procedure accepted but not fully completed"
      ///
      /// This is a retryable code
      ///
      C12ProcedureNotCompleted = 0x800404CA,

      /// Text: "Procedure result code 2, invalid parameter"
      C12ProcedureInvalidParameter = 0x800404CB,

      /// Text: "Procedure result code 3, conflict with the current device setup"
      C12ProcedureDeviceConflict = 0x800404CC,

      /// Text: "Procedure result code 4, had to ignore the procedure due to timing constraint"
      C12ProcedureTimingConstraint = 0x800404CD,

      /// Text: "Procedure result code 5, no authorization to perform this procedure"
      C12ProcedureNoAuthorization = 0x800404CE,

      /// Text: "Procedure result code 6, unrecognized or unsupported procedure"
      C12ProcedureUnknown = 0x800404CF,

      // MCOREErrors.inc

      /// Unknown item
      ///
      ///  - Text: "Item '%s' is unknown"
      ///
      /// This is an English-only internal software error.
      ///
      UnknownItem = 0x800404D4,

      // MCOMErrors.inc

      /// Text: "Cannot convert '%s' function or table number"
      ///
      /// Thrown when software attempts to supply table or function number that is of type or value, unsupported by protocol.
      /// For example, if the user gives -1 or "abc" as function number to a C12 protocol.
      /// This is a software error that will not be shown to end users, English only.
      ///
      CannotConvertToTableOrFunctionNumber = 0x800404DD,

      /// Text: "Channel write timeout (%u bytes written successfully)"
      ChannelWriteTimeout = 0x800404E9,

      // MCOREErrors.inc

      /// Unknown system error
      ///
      ///  - Text: "Unknown system error"
      ///
      UnknownSystemError = 0x80040503,

      /// Operation was cancelled by the user or programmatically
      ///
      ///  - Text: "Operation cancelled"
      ///
      OperationCancelled = 0x800407ED,

      /// Class does not have such service (method)
      ///
      ///  - Text: "Class '%s' does not have service with the name '%s'"
      ///
      NoSuchService = 0x80040806,

      /// Unknown string escape sequence
      ///
      ///  - Text: "Unknown string escape sequence with character '%c'"
      ///  - Text: "Unknown string escape sequence with character code 0x%X"
      ///
      UnknownStringEscape = 0x8004080B,

      /// The given entity cannot be indexed
      ///
      ///  - Text: "Item is not an array or set, and cannot be indexed"
      ///
      /// This error is thrown when an array index operation is attempted on object that is not an array or set.
      ///
      CannotIndexItem = 0x80040812,

      /// The given input is not a binary coded decimal number
      ///
      /// Text: "Cannot convert byte with the value 0x%X to BCD"
      ///
      BadBcd = 0x80040814,

      /// Text: "Number %s is too big or too small"
      OverflowOrUnderflow = 0x80040822,

      /// Text: "Socket read timeout"
      SocketReadTimeout = 0x80040823,

      /// Text: "File '%s' has bad format or it is corrupt"
      ///
      /// This error will result when an input file has bad format or its contents is corrupt.
      ///
      BadFileFormat = 0x80040826,

      /// Text: "Cannot modify a readonly object"
      /// Text: "Attempt to modify a readonly item '%s'"
      ///
      /// Software English-only error.
      /// This will be thrown if an attempt to modify a constant or readonly object is made.
      ///
      CannotModifyConstantOrReadonly = 0x8004082B,

      /// Text: "End of stream"
      ///
      /// Unexpected end of stream of file is encountered.
      ///
      EndOfStream = 0x80040831,

      /// Class not found
      ///
      /// Text: "Class %s not found"
      ///
      /// English-only software error that tells the class with such name is not found.
      /// For example, this error is thrown when an object is attempted to be created remotely, but such class does not exist in remote server.
      ///
      ClassNotFound = 0x80040866,

      /// Unsupported type
      ///
      ///  - Text: "Unsupported type %d"
      ///
      /// This error will typically result at attempt to pass a variant type
      /// unsupported by MeteringSDK. Such variant type can come from a COM facade, or other facades.
      /// This is an English-only error.
      ///
      UnsupportedType = 0x8004086A,

      /// Cannot write to a read-only stream
      ///
      ///  - Text: "Cannot write to readonly stream '%s'"
      ///
      CannotWriteToReadonlyStream = 0x80040889,

      /// Cannot read from an write only stream
      ///
      ///  - Text: "Cannot read from writeonly stream '%s'"
      ///
      CannotReadFromWriteonlyStream = 0x8004088A,

      /// Socket connection is closed by peer
      ///
      /// Text: "Socket connection closed by peer"
      ///
      SocketClosedByPeer = 0x8004088D,

      /// Text "Data not validated, tampering possible"
      DataNotValidated = 0x80040899,

      /// Bad stream format
      ///
      ///  - Text: "Two-byte header expected in stream '%s'"
      ///  - Text: "Encrypted stream '%s' has bad format"
      ///
      BadStreamFormat = 0x8004089C,

      /// Socket proxy server returned an error
      ///
      ProxySocketError = 0x8004089D,

      /// Socket write timeout
      ///
      /// - Text: "Socket write timeout"
      ///
      SocketWriteTimeout = 0x8004089F,

      /// Text: "Given address is not recognized as IPv4 or IPv6"
      /// Text: "Given address is not recognized as IPv4, and there is no IPv6 support installed"
      BadIpAddress = 0x800408A1,

      // MCOMErrors.inc

      /// Text: "Received data size %d is different than requested %d bytes"
      ///
      /// This error is thrown when the device supplied data of different size than one requested.
      /// For example, when a partial read of three bytes is requested, but the device returned five bytes this error is thrown.
      /// Most likely, it signifies a severe problem with the device.
      ///
      ReceivedDataSizeDifferent = 0x80040BD5,

      /// Text: "Channel disconnected unexpectedly"
      ChannelDisconnectedUnexpectedly = 0x80040BE9,

      /// Text: "Received packet toggle bit failure, duplicate packet ignored"
      ReceivedPacketToggleBitFailure = 0x80040C17,

      /// Text: "Device reported bad packet CRC"
      ///
      /// The response from device was signifying a data link layer error, like bad CRC.
      ///
      DeviceReportedBadPacketCRC = 0x80040D50,

      /// Text: "Device gave insecure response on a secured request, tampering is suspected"
      /// Text: "Invocation ID mismatch, tampering is suspected"
      /// Text: "Ap title mismatch, tampering is suspected"
      ///
      PossibleTamperingDetected = 0x80040D56,

      /// Text: "Collision detected by a slave protocol"
      CollisionDetected = 0x8004180B,

      /// Text: "Failed to connect within %u seconds"
      ChannelConnectTimeout = 0x8004180E
   };

private: // Prevent abusing this class as value:

   MErrorEnum();
   MErrorEnum(const MErrorEnum&);
   MErrorEnum& operator=(const MErrorEnum&);
   bool operator==(const MErrorEnum& other) const;
   bool operator!=(const MErrorEnum& other) const;

   M_DECLARE_CLASS(ErrorEnum)
};

// Private library errors

// MCOREErrors.inc

/// Text: "Time value is bad"
const MErrorEnum::Type M_ERR_BAD_TIME_VALUE = static_cast<MErrorEnum::Type>(0x8004040B);

/// Text: "Regular expression is too big"
const MErrorEnum::Type M_ERR_REGEXP_TOO_BIG = static_cast<MErrorEnum::Type>(0x8004040C);

/// Text: "Regular expression has too many parentheses"
const MErrorEnum::Type M_ERR_REGEXP_TOO_MANY_PARENTHESES = static_cast<MErrorEnum::Type>(0x8004040D);

/// Text: "Regular expression has unterminated parentheses '('"
const MErrorEnum::Type M_ERR_REGEXP_UNTERMINATED_PARENTHESES = static_cast<MErrorEnum::Type>(0x8004040E);

/// Text: "Regular expression has unmatched parentheses ')'"
const MErrorEnum::Type M_ERR_REGEXP_UNMATCHED_PARENTHESES = static_cast<MErrorEnum::Type>(0x8004040F);

/// Text: "Regular expression operand '*+' could be empty"
const MErrorEnum::Type M_ERR_REGEXP_OP_COULD_BE_EMPTY = static_cast<MErrorEnum::Type>(0x80040410);

/// Text: "Regular expression has nested '*?+'"
const MErrorEnum::Type M_ERR_REGEXP_NESTED_OP = static_cast<MErrorEnum::Type>(0x80040411);

/// Text: "Regular expression has invalid range within '[]'"
const MErrorEnum::Type M_ERR_REGEXP_INVALID_RANGE = static_cast<MErrorEnum::Type>(0x80040412);

/// Text: "Regular expression has unmatched '[]'"
const MErrorEnum::Type M_ERR_REGEXP_UNMATCHED_BRACE = static_cast<MErrorEnum::Type>(0x80040413);

/// Text: "Regular expression has '?', '+' or '*' that follows nothing"
const MErrorEnum::Type M_ERR_REGEXP_OP_FOLLOWS_NOTHING = static_cast<MErrorEnum::Type>(0x80040414);

/// Text: "Regular expression has trailing '\\'"
const MErrorEnum::Type M_ERR_REGEXP_TRAILING_ESC = static_cast<MErrorEnum::Type>(0x80040415);

/// Text: "Syntax error in '%s'"
const MErrorEnum::Type M_ERR_SYNTAX_ERROR_IN_S1 = static_cast<MErrorEnum::Type>(0x80040423);

/// Text: "Division by zero"
const MErrorEnum::Type M_ERR_DIVISION_BY_ZERO = static_cast<MErrorEnum::Type>(0x80040427);

/// Text: "Unexpected character '%c'"
/// Text: "Unexpected character with code 0x%X"
///
/// English-only software error
///
const MErrorEnum::Type M_ERR_UNEXPECTED_CHARACTER_C1 = static_cast<MErrorEnum::Type>(0x80040430);

/// Text: "Unterminated string"
const MErrorEnum::Type M_ERR_UNTERMINATED_STRING = static_cast<MErrorEnum::Type>(0x80040441);

// MCOMErrors.inc

/// Text: "Invalid operation during active background communication"
const MErrorEnum::Type M_ERR_INVALID_OPERATION_DURING_ACTIVE_BACKGROUND_COMMUNICATION = static_cast<MErrorEnum::Type>(0x80040458);

/// Text: "Channel '%s' is unknown"
const MErrorEnum::Type M_ERR_UNKNOWN_CHANNEL_S1 = static_cast<MErrorEnum::Type>(0x80040462);

/// Text: "Protocol '%s' is unknown"
const MErrorEnum::Type M_ERR_UNKNOWN_PROTOCOL_S1 = static_cast<MErrorEnum::Type>(0x80040463);

// MCOREErrors.inc

/// Text: "Invalid or unsupported parity %u"
const MErrorEnum::Type M_ERR_INVALID_OR_UNSUPPORTED_PARITY_U1 = static_cast<MErrorEnum::Type>(0x80040476);

/// Text: "Invalid or unsupported number of stop bits %u"
const MErrorEnum::Type M_ERR_INVALID_OR_UNSUPPORTED_NUMBER_OF_STOP_BITS_U1 = static_cast<MErrorEnum::Type>(0x80040477);

/// Text: "Invalid or unsupported number of data bits %u"
const MErrorEnum::Type M_ERR_INVALID_OR_UNSUPPORTED_NUMBER_OF_DATA_BITS_U1 = static_cast<MErrorEnum::Type>(0x80040478);

// MCOMErrors.inc

/// Text: "Password should be no more than %d bytes long"
const MErrorEnum::Type M_ERR_PASSWORD_SHOULD_BE_NO_MORE_THAN_D1_BYTES_LONG = static_cast<MErrorEnum::Type>(0x80040483);

/// Text: "Authentication key is expected to be %d bytes long"
const MErrorEnum::Type M_ERR_AUTHENTICATION_KEY_IS_EXPECTED_TO_BE_D1_BYTES_LONG = static_cast<MErrorEnum::Type>(0x80040484);

/// Text: "User name should be no more than %d bytes long"
const MErrorEnum::Type M_ERR_USER_NAME_SHOULD_BE_NO_MORE_THAN_D1_BYTES_LONG = static_cast<MErrorEnum::Type>(0x80040486);

/// Text: "Cannot connect channel '%s' because it is already connected"
const MErrorEnum::Type M_ERR_CANNOT_CONNECT_CHANNEL_S1_IS_ALREADY_CONNECTED = static_cast<MErrorEnum::Type>(0x80040487);

/// Text: "Expected character 0x%02X, received 0x%02X"
const MErrorEnum::Type M_ERR_EXPECTED_X1_GOT_X2 = static_cast<MErrorEnum::Type>(0x8004048C);

// MCOREErrors.inc

/// Text: "Enumeration value '%s' cannot be assigned to"
const MErrorEnum::Type M_ERR_ENUMERATION_S1_CANNOT_BE_ASSIGNED_TO = static_cast<MErrorEnum::Type>(0x800404B2);

/// Text: "Cannot set readonly property '%s'"
const MErrorEnum::Type M_ERR_CANNOT_SET_READONLY_PROPERTY_S1 = static_cast<MErrorEnum::Type>(0x800404B3);

// MCOMErrors.inc

/// Text: "Requested wait period %u is bigger than supported maximum of 255 seconds"
const MErrorEnum::Type M_ERR_WAIT_PERIOD_U1_IS_BIGGER_THAN_MAXIMUM_255 = static_cast<MErrorEnum::Type>(0x800404C1);

/// Text: "Timed out while waiting for connection by modem"
/// Text: "Timed out while waiting for incoming socket connection"
const MErrorEnum::Type M_ERR_TIMED_OUT_WHILE_WAITING_FOR_CONNECTION = static_cast<MErrorEnum::Type>(0x800404C2);

/// Text: "Channel is expected to be in answer mode"
const MErrorEnum::Type M_ERR_CHANNEL_NOT_IN_ANSWER_MODE = static_cast<MErrorEnum::Type>(0x800404C6);

// MCOREErrors.inc

/// Text: "DTR control character with code 0x%X is not known, expected E, D, or H"
const MErrorEnum::Type M_ERR_DTR_CONTROL_WITH_CODE_X1_IS_NOT_KNOWN = static_cast<MErrorEnum::Type>(0x800404C7);

/// Text: "RTS control character with code 0x%X is not known, expected E, D, H, or T"
const MErrorEnum::Type M_ERR_RTS_CONTROL_WITH_CODE_X1_IS_NOT_KNOWN = static_cast<MErrorEnum::Type>(0x800404C8);

/// Text: "Cannot convert character with code 0x%X into a hexadecimal number"
const MErrorEnum::Type M_ERR_CANNOT_CONVERT_CHARACTER_WITH_CODE_X1_INTO_HEX = static_cast<MErrorEnum::Type>(0x800404D2);

/// Text: "Wide character with code 0x%X encountered in place where only eight-bit characters allowed"
const MErrorEnum::Type M_ERR_WIDE_CHARACTER_WITH_CODE_X1_IN_PLACE_WHERE_ONLY_ANSI_ALLOWED = static_cast<MErrorEnum::Type>(0x800404D3);

// MCOMErrors.inc

/// Text: "Protocol violation, response from table 8 is less than four bytes"
const MErrorEnum::Type M_ERR_RESPONSE_FROM_TABLE8_IS_LESS_THAN_FOUR_BYTES = static_cast<MErrorEnum::Type>(0x800404DC);

// MCOREErrors.inc

/// Text: "Bad value for DSP type"
const MErrorEnum::Type M_ERR_BAD_VALUE_FOR_DSP_TYPE = static_cast<MErrorEnum::Type>(0x800404E4);

// MCOMErrors.inc

/// Text: "Incompatibility in table size or contents, cannot identify the meter"
const MErrorEnum::Type M_ERR_INCOMPATIBILITY_IN_TABLE_SIZE_OR_CONTENTS_DURING_IDENTIFY = static_cast<MErrorEnum::Type>(0x800404E7);

// MCOREErrors.inc

/// Text: "Attempt to change object type from '%s' to '%s'"
const MErrorEnum::Type M_ERR_ATTEMPT_TO_CHANGE_OBJECT_TYPE_FROM_S1_TO_S2 = static_cast<MErrorEnum::Type>(0x800404E8);

/// Text: "Supply even number of hexadecimal characters, two for each byte"
const MErrorEnum::Type M_ERR_SUPPLY_EVEN_NUMBER_OF_HEX_CHARACTERS_TWO_FOR_EACH_BYTE = static_cast<MErrorEnum::Type>(0x8004050F);

/// Text: "Property '%s' is not persistent and it has no default value"
const MErrorEnum::Type M_ERR_PROPERTY_S1_IS_NOT_PERSISTENT_AND_HAS_NO_DEFAULT_VALUE = static_cast<MErrorEnum::Type>(0x800407EC);

/// Text: "Thread should finish execution to get its result"
const MErrorEnum::Type M_ERR_THREAD_SHOULD_FINISH_EXECUTION_TO_GET_RESULT = static_cast<MErrorEnum::Type>(0x800407F5);

/// Text: "ISO 8825 binary representation of universal identifier is bad"
const MErrorEnum::Type M_ERR_ISO8825_BINARY_REPRESENTATION_OF_UNIVERSAL_IDENTIFIER_IS_BAD = static_cast<MErrorEnum::Type>(0x800407F6);

/// Text: "ISO 8825 string representation of universal identifier is bad"
const MErrorEnum::Type M_ERR_ISO8825_STRING_REPRESENTATION_OF_UNIVERSAL_IDENTIFIER_IS_BAD = static_cast<MErrorEnum::Type>(0x800407F7);

/// Text: "ISO 8825 length is bad"
const MErrorEnum::Type M_ERR_ISO8825_LENGTH_IS_BAD = static_cast<MErrorEnum::Type>(0x800407F8);

/// Text: "ISO 8825 short length is bad, does not fit in one byte"
const MErrorEnum::Type M_ERR_ISO8825_SHORT_LENGTH_IS_BAD = static_cast<MErrorEnum::Type>(0x800407F9);

/// Text: "Object property '%s' cannot be got from a class, without object"
const MErrorEnum::Type M_ERR_OBJECT_PROPERTY_S1_CANNOT_BE_GOT_FROM_A_CLASS_WITHOUT_OBJECT = static_cast<MErrorEnum::Type>(0x80040804);

/// Text: "Object property '%s' cannot be set to a class, without object"
const MErrorEnum::Type M_ERR_OBJECT_PROPERTY_S1_CANNOT_BE_SET_TO_A_CLASS_WITHOUT_OBJECT = static_cast<MErrorEnum::Type>(0x80040805);

/// Text: "Service '%s' cannot be called without object"
const MErrorEnum::Type M_ERR_SERVICE_S1_CANNOT_BE_CALLED_WITHOUT_OBJECT = static_cast<MErrorEnum::Type>(0x80040807);

/// Text: "Service '%s' does not have %d parameters"
///
/// English-only software error
///
const MErrorEnum::Type M_ERR_SERVICE_S1_DOES_NOT_HAVE_D2_PARAMETERS = static_cast<MErrorEnum::Type>(0x80040808);

/// Text: "Unknown XML sequence '%s'"
const MErrorEnum::Type M_ERR_UNKNOWN_XML_ESCAPE_SEQUENCE_S1 = static_cast<MErrorEnum::Type>(0x80040809);

/// Text: "Bad version number format '%s'"
const MErrorEnum::Type M_ERR_BAD_VERSION_NUMBER_FORMAT_S1 = static_cast<MErrorEnum::Type>(0x8004080A);

/// Text: "Operation not supported for this type"
const MErrorEnum::Type M_ERR_OPERATION_NOT_SUPPORTED_FOR_THIS_TYPE = static_cast<MErrorEnum::Type>(0x8004080F);

/// Text: "Overflow in operation '%s'"
const MErrorEnum::Type M_ERR_OVERFLOW_IN_OPERATION_S1 = static_cast<MErrorEnum::Type>(0x80040810);

/// Text: "Underflow in operation '%s'"
const MErrorEnum::Type M_ERR_UNDERFLOW_IN_OPERATION_S1 = static_cast<MErrorEnum::Type>(0x80040811);

/// Text: "Binary operation between incompatible arguments"
const MErrorEnum::Type M_ERR_BINARY_OPERATION_BETWEEN_INCOMPATIBLE_ARGUMENTS = static_cast<MErrorEnum::Type>(0x80040820);

/// Text: "Sizes of items are different, %d and %d"
const MErrorEnum::Type M_ERR_SIZES_OF_ITEMS_ARE_DIFFERENT_D1_AND_D2 = static_cast<MErrorEnum::Type>(0x80040827);

/// Text: "Character '%s' is not allowed in RAD40"
const MErrorEnum::Type M_ERR_BAD_RAD40_CHARACTER_S1 = static_cast<MErrorEnum::Type>(0x80040828);

/// Text: "The dictionary does not have key '%s'"
const MErrorEnum::Type M_ERR_DICTIONARY_DOES_NOT_HAVE_KEY_S1 = static_cast<MErrorEnum::Type>(0x8004082A);

/// Text: "Could not cast object of type '%s' to '%s'"
const MErrorEnum::Type M_ERR_COULD_NOT_CAST_OBJECT_OF_TYPE_S1_TO_S2 = static_cast<MErrorEnum::Type>(0x80040830);

/// Text: "Time function error"
const MErrorEnum::Type M_ERR_TIME_FUNCTION_ERROR = static_cast<MErrorEnum::Type>(0x80040863);

/// Text: "Recurrent time offset %d is not supported by OS"
const MErrorEnum::Type M_ERR_RECURRENT_TIME_OFFSET_D1_IS_NOT_SUPPORTED_BY_OS = static_cast<MErrorEnum::Type>(0x80040864);

/// Text: "Parameter %d type mismatch"
const MErrorEnum::Type M_ERR_PARAMETER_D1_TYPE_MISMATCH = static_cast<MErrorEnum::Type>(0x8004086C);

/// Text: "Parameter %d not found"
const MErrorEnum::Type M_ERR_PARAMETER_D1_NOT_FOUND = static_cast<MErrorEnum::Type>(0x8004086D);

/// Text: "External application exception, code %d"
const MErrorEnum::Type M_ERR_EXTERNAL_APPLICATION_EXCEPTION_CODE_D1 = static_cast<MErrorEnum::Type>(0x8004086E);

/// Text: "External application exception, code %d, message: %s"
const MErrorEnum::Type M_ERR_EXTERNAL_APPLICATION_EXCEPTION_CODE_D1_MESSAGE_S2 = static_cast<MErrorEnum::Type>(0x8004086F);

/// Text: "Single dimension array is required"
const MErrorEnum::Type M_ERR_SINGLE_DIMENSION_ARRAY_IS_REQUIRED = static_cast<MErrorEnum::Type>(0x80040870);

/// Text: "Argument '%s' is not optional"
const MErrorEnum::Type M_ERR_ARGUMENT_S1_IS_NOT_OPTIONAL = static_cast<MErrorEnum::Type>(0x80040871);

/// Text: "Regular expression is not compiled"
const MErrorEnum::Type M_ERR_REGEXP_IS_NOT_COMPILED = static_cast<MErrorEnum::Type>(0x80040872);

/// Text: "Invalid HEX display format: '%s'"
const MErrorEnum::Type M_ERR_INVALID_HEX_FORMAT = static_cast<MErrorEnum::Type>(0x80040876);

/// Text: "Time span too large - cannot be represented"
const MErrorEnum::Type M_ERR_TIME_SPAN_TOO_LARGE_CANNOT_BE_REPRESENTED = static_cast<MErrorEnum::Type>(0x8004087A);

/// Text: "Time zone '%s' is not found"
const MErrorEnum::Type M_ERR_TIME_ZONE_S1_NOT_FOUND = static_cast<MErrorEnum::Type>(0x80040888);

/// Text: "Scrambled stream '%s' cannot be opened for reading and writing simultaneously"
/// Text: "Stream '%s' cannot be mangled and encrypted at the same time"
/// Text: "Stream '%s' cannot be opened for reading with both UseHeader and any of obfuscation flags"
const MErrorEnum::Type M_ERR_BAD_STREAM_FLAG = static_cast<MErrorEnum::Type>(0x8004088C);

/// Text "Entry not found"
const MErrorEnum::Type M_ERR_ENTRY_NOT_FOUND = static_cast<MErrorEnum::Type>(0x80040892);

/// Text "File path too long"
const MErrorEnum::Type M_ERR_FILE_PATH_TOO_LONG = static_cast<MErrorEnum::Type>(0x80040893);

/// Text "Argument of File Find method is bad"
const MErrorEnum::Type M_ERR_FIND_ARGUMENT_IS_BAD = static_cast<MErrorEnum::Type>(0x80040894);

/// Text "Size of byte string representation of a number shall be in range 1 to 8"
/// Text "Key size is expected to be 16 bytes"
const MErrorEnum::Type M_ERR_SIZE_OF_NUMBER_OUTSIDE_RANGE = static_cast<MErrorEnum::Type>(0x80040898);

/// Format of GUID is bad
///
///  - Text: "Bad GUID format"
///
const MErrorEnum::Type M_ERR_BAD_GUID_FORMAT = static_cast<MErrorEnum::Type>(0x8004089E);

/// Text: "Registry not open"
const MErrorEnum::Type M_ERR_CONFIGURATION_NOT_OPEN = static_cast<MErrorEnum::Type>(0x800408A0);

/// Text: "Cannot set name to node of this type"
/// Text: "Cannot set value to node of this type"
const MErrorEnum::Type M_ERR_CANNOT_SET_TO_SUCH_NODE = static_cast<MErrorEnum::Type>(0x800408A3);

/// Text: "The outgoing packet does not fit into datagram"
///
const MErrorEnum::Type M_ERR_PACKET_IS_TOO_BIG = static_cast<MErrorEnum::Type>(0x800408A4);

/// Text: "Java native interface not initialized"
///
const MErrorEnum::Type M_ERR_JAVA_NATIVE_INTERFACE_ERROR = static_cast<MErrorEnum::Type>(0x800408A5);

/// Text: "OpenSSL error %d: %u"
///
const MErrorEnum::Type M_ERR_OPENSSL_ERROR = static_cast<MErrorEnum::Type>(0x800408A6);

/// Text "Call is made out of sequence"
///
const MErrorEnum::Type M_ERR_OUT_OF_SEQUENCE = static_cast<MErrorEnum::Type>(0x800408A7);

/// Text "Bad print format '%s'"
///
const MErrorEnum::Type M_ERR_BAD_PRINT_FORMAT_S1 = static_cast<MErrorEnum::Type>(0x800408A8);

/// Text: "Bad string for encoding '%s'"
///
const MErrorEnum::Type M_ERR_BAD_STRING_FOR_ENCODING_S1 = static_cast<MErrorEnum::Type>(0x800408A9);

/// Text: "Invalid numeric string format '%s'"
///
const MErrorEnum::Type M_ERR_INVALID_NUMERIC_STRING_FORMAT_S1 = static_cast<MErrorEnum::Type>(0x800408AA);

/// Text: "Invalid character in numeric string"
///
const MErrorEnum::Type M_ERR_INVALID_CHARACTER_IN_NUMERIC_STRING = static_cast<MErrorEnum::Type>(0x800408AB);

/// Text: "Cannot change an asset within apk"
/// Text: "Invalid operation on apk asset"
///
const MErrorEnum::Type M_ERR_INVALID_OPERATION_ON_APK_ASSET = static_cast<MErrorEnum::Type>(0x800408AC);

// MCOMErrors.inc

/// Text: "Meter does not support authentication"
const MErrorEnum::Type M_ERR_METER_DOES_NOT_SUPPORT_AUTHENTICATION = static_cast<MErrorEnum::Type>(0x80040BD0);

/// Text: "Meter requested unknown authentication algorithm"
const MErrorEnum::Type M_ERR_METER_REQUESTED_UNKNOWN_AUTHENTICATION_ALGORITHM = static_cast<MErrorEnum::Type>(0x80040BD1);

/// Text: "Identify protocol request failed, got unrecognized feature code 0x%02X"
const MErrorEnum::Type M_ERR_IDENTIFY_FAILED_GOT_UNRECOGNIZED_FEATURE_CODE_X1 = static_cast<MErrorEnum::Type>(0x80040BD2);

/// Text: "ANSI Identify service was not called, identified information is not available"
const MErrorEnum::Type M_ERR_IDENTIFIED_INFORMATION_IS_NOT_AVAILABLE = static_cast<MErrorEnum::Type>(0x80040BD7);

/// Text: "ANSI Negotiate service was not called, negotiated information is not available"
const MErrorEnum::Type M_ERR_NEGOTIATED_INFORMATION_IS_NOT_AVAILABLE = static_cast<MErrorEnum::Type>(0x80040BDA);

/// Text: "ANSI Logon service was not called, the session idle timeout is not available"
const MErrorEnum::Type M_ERR_NEGOTIATED_SESSION_IDLE_TIMEOUT_IS_NOT_AVAILABLE = static_cast<MErrorEnum::Type>(0x80040BDB);

/// Text: "Packet data length is bad"
const MErrorEnum::Type M_ERR_INBOUND_PACKET_DATA_LENGTH_IS_BAD = static_cast<MErrorEnum::Type>(0x80040BDF);

/// Text: "Did not get a valid byte among %d garbage bytes (last one had code 0x%X)"
const MErrorEnum::Type M_ERR_DID_NOT_GET_A_VALID_BYTE_AMONG_D1_GARBAGE_BYTES_LAST_ONE_HAD_CODE_X2 = static_cast<MErrorEnum::Type>(0x80040BE0);

/// Text: "Connection not established, connection is required for this operation"
const MErrorEnum::Type M_ERR_CONNECTION_NOT_ESTABLISHED_BUT_REQUIRED = static_cast<MErrorEnum::Type>(0x80040BEA);

/// Text: "No phone number specified"
const MErrorEnum::Type M_ERR_NO_PHONE_NUMBER_SPECIFIED = static_cast<MErrorEnum::Type>(0x80040BF7);

/// Text: "Command with such parameters is queued already"
const MErrorEnum::Type M_ERR_COMMAND_WITH_SUCH_PARAMETERS_IS_QUEUED_ALREADY = static_cast<MErrorEnum::Type>(0x80040C3E);

/// Text: "Could not find data with specified parameters"
const MErrorEnum::Type M_ERR_COULD_NOT_FIND_DATA_WITH_SPECIFIED_PARAMETERES = static_cast<MErrorEnum::Type>(0x80040C3F);

/// Text: "Bad ACSE element %2X received"
const MErrorEnum::Type M_ERR_BAD_DATA_IN_ACSE_RESPONSE = static_cast<MErrorEnum::Type>(0x80040C46);

/// Text: "Request length exceeds packet size of C12.22 data format"
const MErrorEnum::Type M_ERR_REQUEST_LENGTH_EXCEEDS_C1222_DATA_FORMAT_PACKET_SIZE = static_cast<MErrorEnum::Type>(0x80040D52);

/// Text: "Application context shall be an absolute UID"
const MErrorEnum::Type M_ERR_APPLICATION_CONTEXT_SHALL_BE_ABSOULTE = static_cast<MErrorEnum::Type>(0x80040D54);

/// Protocol implementation mismatch
///
const MErrorEnum::Type M_ERR_PROTOCOL_IMPLEMENTATION_MISMATCH = static_cast<MErrorEnum::Type>(0x80040D57);

/// Operation is not supported in one way mode
///
const MErrorEnum::Type M_ERR_NOT_SUPPORTED_IN_ONE_WAY_MODE = static_cast<MErrorEnum::Type>(0x80040D58);

/// Text: "Error when dialing RAS connection '%s'"
/// Text: "Invalid parameter for RAS connection '%s'"
/// Text: "User decision to disconnect RAS connection '%s' was made"
/// Text: "Unknown error when establishing RAS connection '%s'"
const MErrorEnum::Type M_ERR_RAS_DIAL_NOT_CONNECTED = static_cast<MErrorEnum::Type>(0x80041808);

/// Text: "Channel is already connected or it has already dialed a RAS connection"
const MErrorEnum::Type M_ERR_RAS_DIAL_ALREADY_CONNECTED = static_cast<MErrorEnum::Type>(0x80041809);

/// Text: "RAS dial name is empty"
const MErrorEnum::Type M_ERR_RAS_DIAL_NAME_EMPTY = static_cast<MErrorEnum::Type>(0x8004180A);

///@}
#endif
