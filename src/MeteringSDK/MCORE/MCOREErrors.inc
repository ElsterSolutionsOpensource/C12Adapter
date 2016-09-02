// File MCOREErrors.inc
//
// Error codes for MCORE

#if !defined(M__ERROR_ENUM) || !defined(M__ERROR)
   #error "Define M__ERROR_ENUM and M__ERROR prior to including this file"
#endif

/// Unknown error
///
/// Text: "Unknown error"
///
/// This is most likely the hard unrecoverable application error such as General Protection Fault, etc.
///
M__ERROR_ENUM(Unknown, 0x80040401) // (unsigned)2147746817, (int)-2147220479

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
M__ERROR_ENUM(BadConversion, 0x80040402) // (unsigned)2147746818, (int)-2147220478

/// No value is present
///
/// - Text: "No value exists"
/// - Text: "No value given for '%s'"
///
/// This would be thrown if a variable has no value.
/// For example, this would signify the variable was not initialized previously.
///
M__ERROR_ENUM(NoValue, 0x8004040A) // (unsigned)2147746826, (int)-2147220470

// Text: "Time value is bad"
M__ERROR(M_ERR_BAD_TIME_VALUE, 0x8004040B) // (unsigned)2147746827, (int)-2147220469

// Text: "Regular expression is too big"
M__ERROR(M_ERR_REGEXP_TOO_BIG, 0x8004040C) // (unsigned)2147746828, (int)-2147220468

// Text: "Regular expression has too many parentheses"
M__ERROR(M_ERR_REGEXP_TOO_MANY_PARENTHESES, 0x8004040D) // (unsigned)2147746829, (int)-2147220467

// Text: "Regular expression has unterminated parentheses '('"
M__ERROR(M_ERR_REGEXP_UNTERMINATED_PARENTHESES, 0x8004040E) // (unsigned)2147746830, (int)-2147220466

// Text: "Regular expression has unmatched parentheses ')'"
M__ERROR(M_ERR_REGEXP_UNMATCHED_PARENTHESES, 0x8004040F) // (unsigned)2147746831, (int)-2147220465

// Text: "Regular expression operand '*+' could be empty"
M__ERROR(M_ERR_REGEXP_OP_COULD_BE_EMPTY, 0x80040410) // (unsigned)2147746832, (int)-2147220464

// Text: "Regular expression has nested '*?+'"
M__ERROR(M_ERR_REGEXP_NESTED_OP, 0x80040411) // (unsigned)2147746833, (int)-2147220463

// Text: "Regular expression has invalid range within '[]'"
M__ERROR(M_ERR_REGEXP_INVALID_RANGE, 0x80040412) // (unsigned)2147746834, (int)-2147220462

// Text: "Regular expression has unmatched '[]'"
M__ERROR(M_ERR_REGEXP_UNMATCHED_BRACE, 0x80040413) // (unsigned)2147746835, (int)-2147220461

// Text: "Regular expression has '?', '+' or '*' that follows nothing"
M__ERROR(M_ERR_REGEXP_OP_FOLLOWS_NOTHING, 0x80040414) // (unsigned)2147746836, (int)-2147220460

// Text: "Regular expression has trailing '\\'"
M__ERROR(M_ERR_REGEXP_TRAILING_ESC, 0x80040415) // (unsigned)2147746837, (int)-2147220459

/// Number is out of range.
///
/// - Text: "Value %s for '%s' is out of range %s .. %s"
///
M__ERROR_ENUM(NumberOutOfRange, 0x80040418) // (unsigned)2147746840, (int)-2147220456

/// String is too long
///
///  - Text: "Line %d in file '%s' is too long"
///  - Text: "String of %d characters is too long for '%s', which has size %d"
///
M__ERROR_ENUM(StringTooLong, 0x80040420) // (unsigned)2147746848, (int)-2147220448

/// Index is out of range
///
///  - Text: "Index %d for '%s' is out of range %d .. %d"
///  - Text: "Index %d is out of range %d .. %d"
///
M__ERROR_ENUM(IndexOutOfRange, 0x8004041A) // (unsigned)2147746842, (int)-2147220454

// Text: "Syntax error in '%s'"
M__ERROR(M_ERR_SYNTAX_ERROR_IN_S1, 0x80040423) // (unsigned)2147746851, (int)-2147220445

// Text: "Division by zero"
M__ERROR(M_ERR_DIVISION_BY_ZERO, 0x80040427) // (unsigned)2147746855, (int)-2147220441

// Text: "Unexpected character '%c'"
// Text: "Unexpected character with code 0x%X"
//
// English-only software error
//
M__ERROR(M_ERR_UNEXPECTED_CHARACTER_C1, 0x80040430) // (unsigned)2147746864, (int)-2147220432

// Text: "Unterminated string"
M__ERROR(M_ERR_UNTERMINATED_STRING, 0x80040441) // (unsigned)2147746881, (int)-2147220415

/// Generic error in a client application
///
/// - Text: "Error in a client application"
///
/// This error is a default one thrown when no error code is given.
/// Like when there is THROW "My Message", the error code of the exception will be this one.
///
M__ERROR_ENUM(ClientApplicationError, 0x8004044D) // (unsigned)2147746893, (int)-2147220403

/// Invalid or unsupported baud
///
///  - Text: "Invalid or unsupported baud rate %u"
///  - Text: "Meter requested invalid or unsupported baud rate with code 0x%X"
///
M__ERROR_ENUM(InvalidBaud, 0x80040475) // (unsigned)2147746933, (int)-2147220363

// Text: "Invalid or unsupported parity %u"
M__ERROR(M_ERR_INVALID_OR_UNSUPPORTED_PARITY_U1, 0x80040476) // (unsigned)2147746934, (int)-2147220362

// Text: "Invalid or unsupported number of stop bits %u"
M__ERROR(M_ERR_INVALID_OR_UNSUPPORTED_NUMBER_OF_STOP_BITS_U1, 0x80040477) // (unsigned)2147746935, (int)-2147220361

// Text: "Invalid or unsupported number of data bits %u"
M__ERROR(M_ERR_INVALID_OR_UNSUPPORTED_NUMBER_OF_DATA_BITS_U1, 0x80040478) // (unsigned)2147746936, (int)-2147220360

/// Unknown socket error
///
///  - Text: "Unknown socket error"
///
M__ERROR_ENUM(UnknownSocketError, 0x8004047F) // (unsigned)2147746943, (int)-2147220353


/// No such property
///
///  - Text: "'%s' does not have property '%s'"
///
M__ERROR_ENUM(NoSuchProperty, 0x800404B1) // (unsigned)2147746993, (int)-2147220303

// Text: "Enumeration value '%s' cannot be assigned to"
M__ERROR(M_ERR_ENUMERATION_S1_CANNOT_BE_ASSIGNED_TO, 0x800404B2) // (unsigned)2147746994, (int)-2147220302

// Text: "Cannot set readonly property '%s'"
M__ERROR(M_ERR_CANNOT_SET_READONLY_PROPERTY_S1, 0x800404B3) // (unsigned)2147746995, (int)-2147220301

// Text: "DTR control character with code 0x%X is not known, expected E, D, or H"
M__ERROR(M_ERR_DTR_CONTROL_WITH_CODE_X1_IS_NOT_KNOWN, 0x800404C7) // (unsigned)2147747015, (int)-2147220281

// Text: "RTS control character with code 0x%X is not known, expected E, D, H, or T"
M__ERROR(M_ERR_RTS_CONTROL_WITH_CODE_X1_IS_NOT_KNOWN, 0x800404C8) // (unsigned)2147747016, (int)-2147220280

// Text: "Cannot convert character with code 0x%X into a hexadecimal number"
M__ERROR(M_ERR_CANNOT_CONVERT_CHARACTER_WITH_CODE_X1_INTO_HEX, 0x800404D2) // (unsigned)2147747026, (int)-2147220270

// Text: "Wide character with code 0x%X encountered in place where only eight-bit characters allowed"
M__ERROR(M_ERR_WIDE_CHARACTER_WITH_CODE_X1_IN_PLACE_WHERE_ONLY_ANSI_ALLOWED, 0x800404D3) // (unsigned)2147747027, (int)-2147220269

/// Unknown item
///
///  - Text: "Item '%s' is unknown"
///
/// This is an English-only internal software error.
///
M__ERROR_ENUM(UnknownItem, 0x800404D4) // (unsigned)2147747028, (int)-2147220268


// Text: "Bad value for DSP type"
M__ERROR(M_ERR_BAD_VALUE_FOR_DSP_TYPE, 0x800404E4) // (unsigned)2147747044, (int)-2147220252

// Text: "Attempt to change object type from '%s' to '%s'"
M__ERROR(M_ERR_ATTEMPT_TO_CHANGE_OBJECT_TYPE_FROM_S1_TO_S2, 0x800404E8) // (unsigned)2147747048, (int)-2147220248

/// Unknown system error
///
///  - Text: "Unknown system error"
///
M__ERROR_ENUM(UnknownSystemError, 0x80040503) // (unsigned)2147747075, (int)-2147220221


// Text: "Supply even number of hexadecimal characters, two for each byte"
M__ERROR(M_ERR_SUPPLY_EVEN_NUMBER_OF_HEX_CHARACTERS_TWO_FOR_EACH_BYTE, 0x8004050F) // (unsigned)2147747087, (int)-2147220209

// Text: "Property '%s' is not persistent and it has no default value"
M__ERROR(M_ERR_PROPERTY_S1_IS_NOT_PERSISTENT_AND_HAS_NO_DEFAULT_VALUE, 0x800407EC) // (unsigned)2147747820, (int)-2147219476

/// Operation was cancelled by the user or programmatically
///
///  - Text: "Operation cancelled"
///
M__ERROR_ENUM(OperationCancelled, 0x800407ED) // (unsigned)2147747821, (int)-2147219475


// Text: "Thread should finish execution to get its result"
M__ERROR(M_ERR_THREAD_SHOULD_FINISH_EXECUTION_TO_GET_RESULT, 0x800407F5) // (unsigned)2147747829, (int)-2147219467

// Text: "ISO 8825 binary representation of universal identifier is bad"
M__ERROR(M_ERR_ISO8825_BINARY_REPRESENTATION_OF_UNIVERSAL_IDENTIFIER_IS_BAD, 0x800407F6) // (unsigned)2147747830, (int)-2147219466

// Text: "ISO 8825 string representation of universal identifier is bad"
M__ERROR(M_ERR_ISO8825_STRING_REPRESENTATION_OF_UNIVERSAL_IDENTIFIER_IS_BAD, 0x800407F7) // (unsigned)2147747831, (int)-2147219465

// Text: "ISO 8825 length is bad"
M__ERROR(M_ERR_ISO8825_LENGTH_IS_BAD, 0x800407F8) // (unsigned)2147747832, (int)-2147219464

// Text: "ISO 8825 short length is bad, does not fit in one byte"
M__ERROR(M_ERR_ISO8825_SHORT_LENGTH_IS_BAD, 0x800407F9) // (unsigned)2147747833, (int)-2147219463


// Text: "Object property '%s' cannot be got from a class, without object"
M__ERROR(M_ERR_OBJECT_PROPERTY_S1_CANNOT_BE_GOT_FROM_A_CLASS_WITHOUT_OBJECT, 0x80040804) // (unsigned)2147747844, (int)-2147219452

// Text: "Object property '%s' cannot be set to a class, without object"
M__ERROR(M_ERR_OBJECT_PROPERTY_S1_CANNOT_BE_SET_TO_A_CLASS_WITHOUT_OBJECT, 0x80040805) // (unsigned)2147747845, (int)-2147219451

/// Class does not have such service (method)
///
///  - Text: "Class '%s' does not have service with the name '%s'"
///
M__ERROR_ENUM(NoSuchService, 0x80040806) // (unsigned)2147747846, (int)-2147219450

// Text: "Service '%s' cannot be called without object"
M__ERROR(M_ERR_SERVICE_S1_CANNOT_BE_CALLED_WITHOUT_OBJECT, 0x80040807) // (unsigned)2147747847, (int)-2147219449

// Text: "Service '%s' does not have %d parameters"
//
// English-only software error
//
M__ERROR(M_ERR_SERVICE_S1_DOES_NOT_HAVE_D2_PARAMETERS, 0x80040808) // (unsigned)2147747848, (int)-2147219448

// Text: "Unknown XML sequence '%s'"
M__ERROR(M_ERR_UNKNOWN_XML_ESCAPE_SEQUENCE_S1, 0x80040809) // (unsigned)2147747849, (int)-2147219447

// Text: "Bad version number format '%s'"
M__ERROR(M_ERR_BAD_VERSION_NUMBER_FORMAT_S1, 0x8004080A) // (unsigned)2147747850, (int)-2147219446

/// Unknown string escape sequence
///
///  - Text: "Unknown string escape sequence with character '%c'"
///  - Text: "Unknown string escape sequence with character code 0x%X"
///
M__ERROR_ENUM(UnknownStringEscape, 0x8004080B) // (unsigned)2147747851, (int)-2147219445

// Text: "Operation not supported for this type"
M__ERROR(M_ERR_OPERATION_NOT_SUPPORTED_FOR_THIS_TYPE, 0x8004080F) // (unsigned)2147747855, (int)-2147219441

// Text: "Overflow in operation '%s'"
M__ERROR(M_ERR_OVERFLOW_IN_OPERATION_S1, 0x80040810) // (unsigned)2147747856, (int)-2147219440

// Text: "Underflow in operation '%s'"
M__ERROR(M_ERR_UNDERFLOW_IN_OPERATION_S1, 0x80040811) // (unsigned)2147747857, (int)-2147219439

/// The given entity cannot be indexed
///
///  - Text: "Item is not an array or set, and cannot be indexed"
///
/// This error is thrown when an array index operation is attempted on object that is not an array or set.
///
M__ERROR_ENUM(CannotIndexItem, 0x80040812) // (unsigned)2147747858, (int)-2147219438


/// The given input is not a binary coded decimal number
///
/// Text: "Cannot convert byte with the value 0x%X to BCD"
///
M__ERROR_ENUM(BadBcd, 0x80040814) // (unsigned)2147747860, (int)-2147219436


// Text: "Binary operation between incompatible arguments"
M__ERROR(M_ERR_BINARY_OPERATION_BETWEEN_INCOMPATIBLE_ARGUMENTS, 0x80040820) // (unsigned)2147747872, (int)-2147219424


// Text: "Number %s is too big or too small"
M__ERROR_ENUM(OverflowOrUnderflow, 0x80040822) // (unsigned)2147747874, (int)-2147219422

// Text: "Socket read timeout"
M__ERROR_ENUM(SocketReadTimeout, 0x80040823) // (unsigned)2147747875, (int)-2147219421


// Text: "File '%s' has bad format or it is corrupt"
//
// This error will result when an input file has bad format or its contents is corrupt.
//
M__ERROR_ENUM(BadFileFormat, 0x80040826) // (unsigned)2147747878, (int)-2147219418

// Text: "Sizes of items are different, %d and %d"
M__ERROR(M_ERR_SIZES_OF_ITEMS_ARE_DIFFERENT_D1_AND_D2, 0x80040827) // (unsigned)2147747879, (int)-2147219417

// Text: "Character '%s' is not allowed in RAD40"
M__ERROR(M_ERR_BAD_RAD40_CHARACTER_S1, 0x80040828) // (unsigned)2147747880, (int)-2147219416

// Text: "The dictionary does not have key '%s'"
M__ERROR(M_ERR_DICTIONARY_DOES_NOT_HAVE_KEY_S1, 0x8004082A) // (unsigned)2147747882, (int)-2147219414

// Text: "Cannot modify a readonly object"
// Text: "Attempt to modify a readonly item '%s'"
//
// Software English-only error.
// This will be thrown if an attempt to modify a constant or readonly object is made.
//
M__ERROR_ENUM(CannotModifyConstantOrReadonly, 0x8004082B) // (unsigned)2147747883, (int)-2147219413


// Text: "Could not cast object of type '%s' to '%s'"
M__ERROR(M_ERR_COULD_NOT_CAST_OBJECT_OF_TYPE_S1_TO_S2, 0x80040830) // (unsigned)2147747888, (int)-2147219408

/// Text: "End of stream"
///
/// Unexpected end of stream of file is encountered.
///
M__ERROR_ENUM(EndOfStream, 0x80040831) // (unsigned)2147747889, (int)-2147219407


// Text: "Time function error"
M__ERROR(M_ERR_TIME_FUNCTION_ERROR, 0x80040863) // (unsigned)2147747939, (int)-2147219357

// Text: "Recurrent time offset %d is not supported by OS"
M__ERROR(M_ERR_RECURRENT_TIME_OFFSET_D1_IS_NOT_SUPPORTED_BY_OS, 0x80040864) // (unsigned)2147747940, (int)-2147219356

/// Class not found
///
/// Text: "Class %s not found"
///
/// English-only software error that tells the class with such name is not found.
/// For example, this error is thrown when an object is attempted to be created remotely, but such class does not exist in remote server.
///
M__ERROR_ENUM(ClassNotFound, 0x80040866) // (unsigned)2147747942, (int)-2147219354

/// Unsupported type
///
///  - Text: "Unsupported type %d"
///
/// This error will typically result at attempt to pass a variant type
/// unsupported by MeteringSDK. Such variant type can come from a COM facade, or other facades.
/// This is an English-only error.
///
M__ERROR_ENUM(UnsupportedType, 0x8004086A) // (unsigned)2147747946, (int)-2147219350

// Text: "Parameter %d type mismatch"
M__ERROR(M_ERR_PARAMETER_D1_TYPE_MISMATCH, 0x8004086C) // (unsigned)2147747948, (int)-2147219348

// Text: "Parameter %d not found"
M__ERROR(M_ERR_PARAMETER_D1_NOT_FOUND, 0x8004086D) // (unsigned)2147747949, (int)-2147219347

// Text: "External application exception, code %d"
M__ERROR(M_ERR_EXTERNAL_APPLICATION_EXCEPTION_CODE_D1, 0x8004086E) // (unsigned)2147747950, (int)-2147219346

// Text: "External application exception, code %d, message: %s"
M__ERROR(M_ERR_EXTERNAL_APPLICATION_EXCEPTION_CODE_D1_MESSAGE_S2, 0x8004086F) // (unsigned)2147747951, (int)-2147219345

// Text: "Single dimension array is required"
M__ERROR(M_ERR_SINGLE_DIMENSION_ARRAY_IS_REQUIRED, 0x80040870) // (unsigned)2147747952, (int)-2147219344

// Text: "Argument '%s' is not optional"
M__ERROR(M_ERR_ARGUMENT_S1_IS_NOT_OPTIONAL, 0x80040871) // (unsigned)2147747953, (int)-2147219343

// Text: "Regular expression is not compiled"
M__ERROR(M_ERR_REGEXP_IS_NOT_COMPILED, 0x80040872) // (unsigned)2147747954, (int)-2147219342


// Text: "Invalid HEX display format: '%s'"
M__ERROR(M_ERR_INVALID_HEX_FORMAT, 0x80040876) // (unsigned)2147747958, (int)-2147219338


// Text: "Time span too large - cannot be represented"
M__ERROR(M_ERR_TIME_SPAN_TOO_LARGE_CANNOT_BE_REPRESENTED, 0x8004087A) // (unsigned)2147747962, (int)-2147219334


// Text: "Time zone '%s' is not found"
M__ERROR(M_ERR_TIME_ZONE_S1_NOT_FOUND, 0x80040888) // (unsigned)2147747976, (int)-2147219320

/// Cannot write to a read-only stream
///
///  - Text: "Cannot write to readonly stream '%s'"
///
M__ERROR_ENUM(CannotWriteToReadonlyStream, 0x80040889) // (unsigned)2147747977, (int)-2147219319

/// Cannot read from an write only stream
///
///  - Text: "Cannot read from writeonly stream '%s'"
///
M__ERROR_ENUM(CannotReadFromWriteonlyStream, 0x8004088A) // (unsigned)2147747978, (int)-2147219318

// Text: "Scrambled stream '%s' cannot be opened for reading and writing simultaneously"
// Text: "Stream '%s' cannot be mangled and encrypted at the same time"
// Text: "Stream '%s' cannot be opened for reading with both UseHeader and any of obfuscation flags"
M__ERROR(M_ERR_BAD_STREAM_FLAG, 0x8004088C) // (unsigned)2147747980, (int)-2147219316

/// Socket connection is closed by peer
///
/// Text: "Socket connection closed by peer"
///
M__ERROR_ENUM(SocketClosedByPeer, 0x8004088D) // (unsigned)2147747981, (int)-2147219315


// Text "Entry not found"
M__ERROR(M_ERR_ENTRY_NOT_FOUND, 0x80040892) // (unsigned)2147747986, (int)-2147219310

// Text "File path too long"
M__ERROR(M_ERR_FILE_PATH_TOO_LONG, 0x80040893) // (unsigned)2147747987, (int)-2147219309

// Text "Argument of File Find method is bad"
M__ERROR(M_ERR_FIND_ARGUMENT_IS_BAD, 0x80040894) // (unsigned)2147747988, (int)-2147219308


// Text "Size of byte string representation of a number shall be in range 1 to 8"
// Text "Key size is expected to be 16 bytes"
M__ERROR(M_ERR_SIZE_OF_NUMBER_OUTSIDE_RANGE, 0x80040898) // (unsigned)2147747992, (int)-2147219304

// Text "Data not validated, tampering possible"
M__ERROR_ENUM(DataNotValidated, 0x80040899) // (unsigned)2147747993, (int)-2147219303


/// Bad stream format
///
///  - Text: "Two-byte header expected in stream '%s'"
///  - Text: "Encrypted stream '%s' has bad format"
///
M__ERROR_ENUM(BadStreamFormat, 0x8004089C) // (unsigned)2147747996, (int)-2147219300

/// Socket proxy server returned an error
///
M__ERROR_ENUM(ProxySocketError, 0x8004089D) // (unsigned)2147747997, (int)-2147219299

// Format of GUID is bad
//
//  - Text: "Bad GUID format"
//
M__ERROR(M_ERR_BAD_GUID_FORMAT, 0x8004089E) // (unsigned)2147747998, (int)-2147219298

/// Socket write timeout
///
/// - Text: "Socket write timeout"
///
M__ERROR_ENUM(SocketWriteTimeout, 0x8004089F) // (unsigned)2147747999, (int)-2147219297


// Text: "Registry not open"
M__ERROR(M_ERR_CONFIGURATION_NOT_OPEN, 0x800408A0) // (unsigned)2147748000, (int)-2147219296

// Text: "Given address is not recognized as IPv4 or IPv6"
// Text: "Given address is not recognized as IPv4, and there is no IPv6 support installed"
M__ERROR_ENUM(BadIpAddress, 0x800408A1) // (unsigned)2147748001, (int)-2147219295


// Text: "Cannot set name to node of this type"
// Text: "Cannot set value to node of this type"
M__ERROR(M_ERR_CANNOT_SET_TO_SUCH_NODE, 0x800408A3) // (unsigned)2147748003, (int)-2147219293

// Text: "The outgoing packet does not fit into datagram"
//
M__ERROR(M_ERR_PACKET_IS_TOO_BIG, 0x800408A4) // (unsigned)2147748004, (int)-2147219292

// Text: "Java native interface not initialized"
//
M__ERROR(M_ERR_JAVA_NATIVE_INTERFACE_ERROR, 0x800408A5) // (unsigned)2147748005, (int)-2147219291

// Text: "OpenSSL error %d: %u"
//
M__ERROR(M_ERR_OPENSSL_ERROR, 0x800408A6) // (unsigned)2147748006, (int)-2147219290

// Text "Call is made out of sequence"
//
M__ERROR(M_ERR_OUT_OF_SEQUENCE, 0x800408A7) // (unsigned)2147748007, (int)-2147219289

// Text "Bad print format '%s'"
//
M__ERROR(M_ERR_BAD_PRINT_FORMAT_S1, 0x800408A8) // (unsigned)2147748008, (int)-2147219288

// Text: "Bad string for encoding '%s'"
//
M__ERROR(M_ERR_BAD_STRING_FOR_ENCODING_S1, 0x800408A9) // (unsigned)2147748009, (int)-2147219287

// Text: "Invalid numeric string format '%s'"
//
M__ERROR(M_ERR_INVALID_NUMERIC_STRING_FORMAT_S1, 0x800408AA) // (unsigned)2147748010, (int)-2147219286

// Text: "Invalid character in numeric string"
//
M__ERROR(M_ERR_INVALID_CHARACTER_IN_NUMERIC_STRING, 0x800408AB) // (unsigned)2147748011, (int)-2147219285

// Text: "Cannot change an asset within apk"
// Text: "Invalid operation on apk asset"
//
M__ERROR(M_ERR_INVALID_OPERATION_ON_APK_ASSET, 0x800408AC) // (unsigned)2147748012, (int)-2147219284
