// File MCOMErrors.inc
//
// Error codes for MCOM

// Text: "Invalid operation during active background communication"
M__ERROR(M_ERR_INVALID_OPERATION_DURING_ACTIVE_BACKGROUND_COMMUNICATION, 0x80040458) // (unsigned)2147746904, (int)-2147220392

// Text: "Channel '%s' is unknown"
M__ERROR(M_ERR_UNKNOWN_CHANNEL_S1, 0x80040462) // (unsigned)2147746914, (int)-2147220382

// Text: "Protocol '%s' is unknown"
M__ERROR(M_ERR_UNKNOWN_PROTOCOL_S1, 0x80040463) // (unsigned)2147746915, (int)-2147220381


// Text: "Password should be no more than %d bytes long"
M__ERROR(M_ERR_PASSWORD_SHOULD_BE_NO_MORE_THAN_D1_BYTES_LONG, 0x80040483) // (unsigned)2147746947, (int)-2147220349

// Text: "Authentication key is expected to be %d bytes long"
M__ERROR(M_ERR_AUTHENTICATION_KEY_IS_EXPECTED_TO_BE_D1_BYTES_LONG, 0x80040484) // (unsigned)2147746948, (int)-2147220348

// Text: "User name should be no more than %d bytes long"
M__ERROR(M_ERR_USER_NAME_SHOULD_BE_NO_MORE_THAN_D1_BYTES_LONG, 0x80040486) // (unsigned)2147746950, (int)-2147220346

// Text: "Cannot connect channel '%s' because it is already connected"
M__ERROR(M_ERR_CANNOT_CONNECT_CHANNEL_S1_IS_ALREADY_CONNECTED, 0x80040487) // (unsigned)2147746951, (int)-2147220345


// Text: "Characters echoed did not match ones sent. Not a current loop device?"
//
// This error will result when the echo is set for the channel, 
// but the received characters did not match those which are sent.
//
M__ERROR_ENUM(CharactersNotEchoed, 0x8004048B) // (unsigned)2147746955, (int)-2147220341

// Text: "Expected character 0x%02X, received 0x%02X"
M__ERROR(M_ERR_EXPECTED_X1_GOT_X2, 0x8004048C) // (unsigned)2147746956, (int)-2147220340

// Text: "CRC check failed"
M__ERROR_ENUM(CrcCheckFailed, 0x8004048D) // (unsigned)2147746957, (int)-2147220339


// Text for this error does not exist
//
// This error code can be used to derive C12 service response from error code,
// as it will be much easier than parsing an error code from message text.
// This error can still occur in the future in a rare hypothetical condition at which an OK is actually an error.
//
M__ERROR_ENUM(C12ServiceResponseOK, 0x80040490) // (unsigned)2147746960, (int)-2147220336

// Text: "0x%02X Error, no reason provided (ERR)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseERR, 0x80040491) // (unsigned)2147746961, (int)-2147220335

// Text: "0x%02X Service Not Supported (SNS)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseSNS, 0x80040492) // (unsigned)2147746962, (int)-2147220334

// Text: "0x%02X Insufficient Security Clearance (ISC)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseISC, 0x80040493) // (unsigned)2147746963, (int)-2147220333

// Text: "0x%02X Operation Not Possible (ONP)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseONP, 0x80040494) // (unsigned)2147746964, (int)-2147220332

// Text: "0x%02X Inappropriate Action Requested (IAR)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseIAR, 0x80040495) // (unsigned)2147746965, (int)-2147220331

// Text: "0x%02X Device Busy (BSY)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseBSY, 0x80040496) // (unsigned)2147746966, (int)-2147220330

// Text: "0x%02X Data Not Ready (DNR)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseDNR, 0x80040497) // (unsigned)2147746967, (int)-2147220329

// Text: "0x%02X Data Locked (DLK)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseDLK, 0x80040498) // (unsigned)2147746968, (int)-2147220328

// Text: "0x%02X Renegotiate (RNO)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseRNO, 0x80040499) // (unsigned)2147746969, (int)-2147220327

// Text: "0x%02X Invalid Service Sequence State (ISSS)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseISSS, 0x8004049A) // (unsigned)2147746970, (int)-2147220326

// Text: "0x%02X Security mechanism error detected (SME)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseSME, 0x8004049B) // (unsigned)2147746971, (int)-2147220325

// Text: "0x%02X Unknown or invalid Called ApTitle is received (UAT)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseUAT, 0x8004049C) // (unsigned)2147746972, (int)-2147220324

// Text: "0x%02X Network timeout detected (NETT)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseNETT, 0x8004049D) // (unsigned)2147746973, (int)-2147220323

// Text: "0x%02X Node is not reachable (NETR)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseNETR, 0x8004049E) // (unsigned)2147746974, (int)-2147220322

// Text: "0x%02X Request too large (RQTL)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseRQTL, 0x8004049F) // (unsigned)2147746975, (int)-2147220321

// Text: "0x%02X Response too large (RSTL)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseRSTL, 0x800404A0) // (unsigned)2147746976, (int)-2147220320

// Text: "0x%02X Segmentation required, but not possible (SGNP)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseSGNP, 0x800404A1) // (unsigned)2147746977, (int)-2147220319

// Text: "0x%02X Segmentation error (SGERR)"
//
// Thrown by C12 protocol service
//
M__ERROR_ENUM(C12ServiceResponseSGERR, 0x800404A2) // (unsigned)2147746978, (int)-2147220318


// Text: "0x%02X Not registered to you (NRY)"
//
// Not registered to you, code not defined in C12 standard, specific to Elster cNIC devices
//
M__ERROR_ENUM(C12ServiceResponseNRY, 0x800404B0) // (unsigned)2147746992, (int)-2147220304

// Text: "Invalid checksum"
M__ERROR_ENUM(InvalidChecksum, 0x800404B4) // (unsigned)2147746996, (int)-2147220300


// Text: "Requested wait period %u is bigger than supported maximum of 255 seconds"
M__ERROR(M_ERR_WAIT_PERIOD_U1_IS_BIGGER_THAN_MAXIMUM_255, 0x800404C1) // (unsigned)2147747009, (int)-2147220287

// Text: "Timed out while waiting for connection by modem"
// Text: "Timed out while waiting for incoming socket connection"
M__ERROR(M_ERR_TIMED_OUT_WHILE_WAITING_FOR_CONNECTION, 0x800404C2) // (unsigned)2147747010, (int)-2147220286

// Text: "Channel read timeout (%u bytes read successfully)"
M__ERROR_ENUM(ChannelReadTimeout, 0x800404C3) // (unsigned)2147747011, (int)-2147220285

// Text: "Could not connect by modem"
// Text: "Could not connect by modem (Line is busy)"
// Text: "Could not connect by modem (Timeout)"
// Text: "Could not connect by modem (No Dial Tone)"
// Text: "Could not connect by modem (No Answer)"
// Text: "Could not connect by modem (No Carrier)"
M__ERROR_ENUM(CouldNotConnectByModem, 0x800404C4) // (unsigned)2147747012, (int)-2147220284

// Text: "Modem responded with error to command"
// Text: "Modem did not respond or gave an unknown response to command"
M__ERROR_ENUM(ModemError, 0x800404C5) // (unsigned)2147747013, (int)-2147220283

// Text: "Channel is expected to be in answer mode"
M__ERROR(M_ERR_CHANNEL_NOT_IN_ANSWER_MODE, 0x800404C6) // (unsigned)2147747014, (int)-2147220282


// Text: ** no text exists for this procedure result **
//
// This error code can be used to derive C12 procedure result from error code,
// as it will be much easier than parsing an error code from message text.
// This error can still occur in the future in a rare hypothetical condition at which an OK is actually an error.
//
M__ERROR_ENUM(C12ProcedureResultOK, 0x800404C9) // (unsigned)2147747017, (int)-2147220279

// Text: "Procedure result code 1, Procedure accepted but not fully completed"
//
// This is a retryable code
//
M__ERROR_ENUM(C12ProcedureNotCompleted, 0x800404CA) // (unsigned)2147747018, (int)-2147220278

// Text: "Procedure result code 2, invalid parameter"
M__ERROR_ENUM(C12ProcedureInvalidParameter, 0x800404CB) // (unsigned)2147747019, (int)-2147220277

// Text: "Procedure result code 3, conflict with the current device setup"
M__ERROR_ENUM(C12ProcedureDeviceConflict, 0x800404CC) // (unsigned)2147747020, (int)-2147220276

// Text: "Procedure result code 4, had to ignore the procedure due to timing constraint"
M__ERROR_ENUM(C12ProcedureTimingConstraint, 0x800404CD) // (unsigned)2147747021, (int)-2147220275

// Text: "Procedure result code 5, no authorization to perform this procedure"
M__ERROR_ENUM(C12ProcedureNoAuthorization, 0x800404CE) // (unsigned)2147747022, (int)-2147220274

// Text: "Procedure result code 6, unrecognized or unsupported procedure"
M__ERROR_ENUM(C12ProcedureUnknown, 0x800404CF) // (unsigned)2147747023, (int)-2147220273


// Text: "Protocol violation, response from table 8 is less than four bytes"
M__ERROR(M_ERR_RESPONSE_FROM_TABLE8_IS_LESS_THAN_FOUR_BYTES, 0x800404DC) // (unsigned)2147747036, (int)-2147220260

// Text: "Cannot convert '%s' function or table number"
//
// Thrown when software attempts to supply table or function number that is of type or value, unsupported by protocol.
// For example, if the user gives -1 or "abc" as function number to a C12 protocol.
// This is a software error that will not be shown to end users, English only.
//
M__ERROR_ENUM(CannotConvertToTableOrFunctionNumber, 0x800404DD) // (unsigned)2147747037, (int)-2147220259


// Text: "Incompatibility in table size or contents, cannot identify the meter"
M__ERROR(M_ERR_INCOMPATIBILITY_IN_TABLE_SIZE_OR_CONTENTS_DURING_IDENTIFY, 0x800404E7) // (unsigned)2147747047, (int)-2147220249

// Text: "Channel write timeout (%u bytes written successfully)"
M__ERROR_ENUM(ChannelWriteTimeout, 0x800404E9) // (unsigned)2147747049, (int)-2147220247


// Text: "Meter does not support authentication"
M__ERROR(M_ERR_METER_DOES_NOT_SUPPORT_AUTHENTICATION, 0x80040BD0) // (unsigned)2147748816, (int)-2147218480

// Text: "Meter requested unknown authentication algorithm"
M__ERROR(M_ERR_METER_REQUESTED_UNKNOWN_AUTHENTICATION_ALGORITHM, 0x80040BD1) // (unsigned)2147748817, (int)-2147218479

// Text: "Identify protocol request failed, got unrecognized feature code 0x%02X"
M__ERROR(M_ERR_IDENTIFY_FAILED_GOT_UNRECOGNIZED_FEATURE_CODE_X1, 0x80040BD2) // (unsigned)2147748818, (int)-2147218478


// Text: "Received data size %d is different than requested %d bytes"
//
// This error is thrown when the device supplied data of different size than one requested.
// For example, when a partial read of three bytes is requested, but the device returned five bytes this error is thrown.
// Most likely, it signifies a severe problem with the device.
//
M__ERROR_ENUM(ReceivedDataSizeDifferent, 0x80040BD5) // (unsigned)2147748821, (int)-2147218475


// Text: "ANSI Identify service was not called, identified information is not available"
M__ERROR(M_ERR_IDENTIFIED_INFORMATION_IS_NOT_AVAILABLE, 0x80040BD7) // (unsigned)2147748823, (int)-2147218473


// Text: "ANSI Negotiate service was not called, negotiated information is not available"
M__ERROR(M_ERR_NEGOTIATED_INFORMATION_IS_NOT_AVAILABLE, 0x80040BDA) // (unsigned)2147748826, (int)-2147218470

// Text: "ANSI Logon service was not called, the session idle timeout is not available"
M__ERROR(M_ERR_NEGOTIATED_SESSION_IDLE_TIMEOUT_IS_NOT_AVAILABLE, 0x80040BDB) // (unsigned)2147748827, (int)-2147218469


// Text: "Packet data length is bad"
M__ERROR(M_ERR_INBOUND_PACKET_DATA_LENGTH_IS_BAD, 0x80040BDF) // (unsigned)2147748831, (int)-2147218465

// Text: "Did not get a valid byte among %d garbage bytes (last one had code 0x%X)"
M__ERROR(M_ERR_DID_NOT_GET_A_VALID_BYTE_AMONG_D1_GARBAGE_BYTES_LAST_ONE_HAD_CODE_X2, 0x80040BE0) // (unsigned)2147748832, (int)-2147218464

   // spare 0x80040BE1 (unsigned)2147748833, (int)-2147218463


// Text: "Channel disconnected unexpectedly"
M__ERROR_ENUM(ChannelDisconnectedUnexpectedly, 0x80040BE9) // (unsigned)2147748841, (int)-2147218455

// Text: "Connection not established, connection is required for this operation"
M__ERROR(M_ERR_CONNECTION_NOT_ESTABLISHED_BUT_REQUIRED, 0x80040BEA) // (unsigned)2147748842, (int)-2147218454


// Text: "No phone number specified"
M__ERROR(M_ERR_NO_PHONE_NUMBER_SPECIFIED, 0x80040BF7) // (unsigned)2147748855, (int)-2147218441


// Text: "Received packet toggle bit failure, duplicate packet ignored"
M__ERROR_ENUM(ReceivedPacketToggleBitFailure, 0x80040C17) // (unsigned)2147748887, (int)-2147218409


// Text: "Command with such parameters is queued already"
M__ERROR(M_ERR_COMMAND_WITH_SUCH_PARAMETERS_IS_QUEUED_ALREADY, 0x80040C3E) // (unsigned)2147748926, (int)-2147218370

// Text: "Could not find data with specified parameters"
M__ERROR(M_ERR_COULD_NOT_FIND_DATA_WITH_SPECIFIED_PARAMETERES, 0x80040C3F) // (unsigned)2147748927, (int)-2147218369


// Text: "Bad ACSE element %2X received"
M__ERROR(M_ERR_BAD_DATA_IN_ACSE_RESPONSE, 0x80040C46) // (unsigned)2147748934, (int)-2147218362


// Text: "Device reported bad packet CRC"
//
// The response from device was signifying a data link layer error, like bad CRC.
//
M__ERROR_ENUM(DeviceReportedBadPacketCRC, 0x80040D50) // (unsigned)2147749200, (int)-2147218096

// Text: "Request length exceeds packet size of C12.22 data format"
M__ERROR(M_ERR_REQUEST_LENGTH_EXCEEDS_C1222_DATA_FORMAT_PACKET_SIZE, 0x80040D52) // (unsigned)2147749202, (int)-2147218094

    // unused 0x80040D53

// Text: "Application context shall be an absolute UID"
M__ERROR(M_ERR_APPLICATION_CONTEXT_SHALL_BE_ABSOULTE, 0x80040D54) // (unsigned)2147749204, (int)-2147218092


// Text: "Device gave insecure response on a secured request, tampering is suspected"
// Text: "Invocation ID mismatch, tampering is suspected"
// Text: "Ap title mismatch, tampering is suspected"
//
M__ERROR_ENUM(PossibleTamperingDetected, 0x80040D56) // (unsigned)2147749206, (int)-2147218090

// Protocol implementation mismatch
//
M__ERROR(M_ERR_PROTOCOL_IMPLEMENTATION_MISMATCH, 0x80040D57) // (unsigned)2147749207, (int)-2147218089

// Operation is not supported in one way mode
//
M__ERROR(M_ERR_NOT_SUPPORTED_IN_ONE_WAY_MODE, 0x80040D58) // (unsigned)2147749208, (int)-2147218088


// Text: "Error when dialing RAS connection '%s'"
// Text: "Invalid parameter for RAS connection '%s'"
// Text: "User decision to disconnect RAS connection '%s' was made"
// Text: "Unknown error when establishing RAS connection '%s'"
M__ERROR(M_ERR_RAS_DIAL_NOT_CONNECTED, 0x80041808) // (unsigned)2147751944, (int)-2147215352

// Text: "Channel is already connected or it has already dialed a RAS connection"
M__ERROR(M_ERR_RAS_DIAL_ALREADY_CONNECTED, 0x80041809) // (unsigned)2147751945, (int)-2147215351

// Text: "RAS dial name is empty"
M__ERROR(M_ERR_RAS_DIAL_NAME_EMPTY, 0x8004180A) // (unsigned)2147751946, (int)-2147215350

// Text: "Collision detected by a slave protocol"
M__ERROR_ENUM(CollisionDetected, 0x8004180B) // (unsigned)2147751947, (int)-2147215349


// Text: "Failed to connect within %u seconds"
M__ERROR_ENUM(ChannelConnectTimeout, 0x8004180E) // (unsigned)2147751950, (int)-2147215346


