#ifndef MCOM_CHANNELMODEM_H
#define MCOM_CHANNELMODEM_H
/// \addtogroup MCOM
///@{
/// \file MCOM/ChannelModem.h

#include <MCOM/ChannelSerialPort.h>

#if !M_NO_MCOM_CHANNEL_MODEM

/// Modem channel is a channel based on serial port.
///
/// Win-modems and the other modems that are not based on the serial port have to be
/// separate classes. Among the COM port modems, only Hayes compatible modems are supported.
///
/// The connection attempt can take a long time to complete or time out.
/// If the connection is queued with QConnect and committed asynchronously with QCommit(True),
/// then the connection can be aborted with QAbort.
///
class MCOM_CLASS MChannelModem : public MChannelSerialPort
{
public: // Types:

   /// Possible modem response codes. Not all modems can handle enumerated 
   /// responses, but most of them can.
   ///
   enum MModemResponse
   {
      /// Unknown modem response or timeout as a result of some functions
      MODEM_RESPONSE_UNKNOWN,

      /// OK response
      MODEM_RESPONSE_OK,

      /// Any kind of CONNECT response (CONNECT 19200, CONNECT 9600 and so on)
      MODEM_RESPONSE_CONNECT,

      /// RING response
      MODEM_RESPONSE_RING,

      /// NO CARRIER response
      MODEM_RESPONSE_NO_CARRIER,

      /// ERROR response
      MODEM_RESPONSE_ERROR,

      /// TIMEOUT response
      MODEM_RESPONSE_TIMEOUT,

      /// NO DIAL TONE response
      MODEM_RESPONSE_NO_DIALTONE,

      /// BUSY response
      MODEM_RESPONSE_BUSY,

      /// NO ANSWER response
      MODEM_RESPONSE_NO_ANSWER
   };

public: // Channel specific methods:

   /// Create a channel with initial parameters.
   ///
   MChannelModem();

   /// Destroy channel modem.
   ///
   /// \pre If channel is connected then it will be disconnected.
   ///
   virtual ~MChannelModem();

   /// Connect to the meter, where connection means hook off, 
   /// dialing phone number, checking for modem responses.
   ///
   /// If the channel is configured with \refprop{GetAutoAnswer,AutoAnswer} false, 
   /// then the channel attempts to connect using the configured 
   /// \refprop{GetPhoneNumber,PhoneNumber}, \refprop{GetBaud,Baud}, 
   /// \refprop{GetInitString,InitString}, \refprop{GetDialString,DialString}, and
   /// \refprop{GetDialTimeout,DialTimeout}. An exception is thrown if the 
   /// connection cannot be established during the Dial Timeout.
   /// 
   /// If the channel is configured with \refprop{GetAutoAnswer,AutoAnswer} true, 
   /// the channel waits \refprop{GetAutoAnswerTimeout,AutoAnswerTimeout} for
   /// the connection to be established. If after the first ring, the connection 
   /// cannot be established during \refprop{GetDialTimeout,DialTimeout}, then  
   /// the channel continues to wait for the remaining Auto Answer Timeout
   /// for another call. If the connection cannot be established during  
   /// the Auto Answer Timeout, an exception is thrown.
   ///
   /// \pre Connect throws an exception if the connection to the meter
   /// has already been made, one can use \ref IsConnected to check.
   /// Many operating system and program exceptions can be thrown by this method.
   ///
   virtual void Connect();

   /// When \refprop{GetAutoAnswer,AutoAnswer} true, wait for the incoming 
   /// connection without disconnecting the channel.
   ///
   /// A typical server application sequence that uses this call:
   /// \code
   ///     Connect() // wait for the first incoming connection
   ///     loop until interrupted:
   ///         ... communicate ...
   ///         WaitForNextIncomingConnection() // wait for the next incoming connection
   ///     end loop
   ///     Disconnect()
   /// \endcode
   ///
   /// \param reinitialize Tells if the channel is reinitialized for each new incoming connection.
   ///
   /// \pre Prior to this call, the channel needs to be configured with \refprop{SetAutoAnswer,AutoAnswer} true, 
   ///      and the connection established with Connect(). Not all channels support auto answer mode, 
   ///      and might throw an exception. A timeout exception is thrown if no call is received 
   ///      during the \refprop{GetAutoAnswerTimeout,AutoAnswerTimeout} period.
   ///
   virtual void WaitForNextIncomingConnection(bool reinitialize = true);

   /// Disconnect channel. This service can be called as many times as the user 
   /// wants. Not exception will be thrown. Disconnect means hook on.
   ///
   virtual void Disconnect();

   /// Returns true if channel is connected and false otherwise. Can be called
   /// at any time.
   ///
   virtual bool IsConnected() const;

   /// Throw an appropriate exception if the channel is not connected.
   /// The exception can be different depending on whether the connection was
   /// not made previously, or if the connection was unexpectedly terminated.
   ///
   /// \pre The channel is connected, or an exception is thrown.
   ///
   virtual void CheckIfConnected();

public: // Property handling routines:

   ///@{
   /// The AT command used to put the modem into Auto Answer mode. 
   /// The channel must also be configured with \refprop{SetAutoAnswer,AutoAnswer} true.   
   ///
   /// The connection attempt can take a long time to complete or time out. If the connection is
   /// queued with QConnect and committed asynchronously with QCommit(True), then the connection
   /// can be aborted with QAbort.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value "ATS0=1"
   ///
   /// \possible_values
   ///  - Your best resource will be your modem manual. You can also find online documentation
   /// for modem commands at sites like http://en.wikibooks.org/wiki/Serial_Programming/Modems_and_AT_Commands
   /// or http://en.wikipedia.org/wiki/Hayes_command_set. Most modems will support "ATS0=#", where #
   /// is the number of rings to wait for before answering the call.
   ///
   /// \pre Channel is not connected, otherwise the behavior is unpredictable.
   ///
   const MByteString& GetAutoAnswerString() const
   {
      return m_autoAnswerString;
   }
   void SetAutoAnswerString(const MByteString& autoAnswerString)
   {
      m_autoAnswerString = autoAnswerString;
   }
   ///@}

   ///@{
   /// This is the modem initialization string sent to the modem prior to making a phone call
   /// (\refprop{GetAutoAnswer,AutoAnswer} = False) or prior to waiting for a 
   /// phone call (\refprop{GetAutoAnswer,AutoAnswer} = True). Not all
   /// modems support the same commands, and the InitString may need to be tuned for your
   /// particular modem. For our MODEM channel implementation, the modem needs to be told to turn
   /// echo off, return verbal result codes, track the DCD state, and hang up when the DTR line
   /// is dropped. All of these modem commands are included in the InitString default value and
   /// are described in the Default Value section below.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value "ATZE0Q0V1&C1&D2" 
   ///     <br> - Z resets the modem to its default settings. 
   ///     <br> - E0 tells the modem to turn echo off. 
   ///     <br> - Q0V1 tells the modem to turn quiet mode off and return verbal
   /// result codes. 
   ///     <br> - &C1 tells the modem to track the Data Carrier Detection state, instead of
   /// forcing it high. 
   ///     <br> - &D2 tells the modem to hang up when the DTR line is dropped. Use caution
   /// when changing this value. If the phone call is not disconnected when the DTR line is
   /// dropped, you could end up with a huge phone bill.
   ///
   /// \possible_values
   ///  - Your best resource will be your modem manual. You can also find online documentation
   /// for modem commands at sites like http://en.wikibooks.org/wiki/Serial_Programming/Modems_and_AT_Commands 
   /// or http://en.wikipedia.org/wiki/Hayes_command_set.
   ///
   /// \pre Channel is not connected. Otherwise the behavior is unpredictable.
   ///
   const MByteString& GetInitString() const
   {
      return m_initString;
   }
   void SetInitString(const MByteString& initString)
   {
      m_initString = initString;
   }
   ///@}

   ///@{
   /// The string that initiates dialing, typically this is the ATD command. The "DialString" and
   /// "PhoneNumber" are combined to yield the full dialing command that is sent to the modem.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value "ATD"
   ///
   /// \possible_values
   ///  - Your best resource will be your modem manual. You can also find online documentation
   /// for modem commands at sites like http://en.wikibooks.org/wiki/Serial_Programming/Modems_and_AT_Commands 
   /// or http://en.wikipedia.org/wiki/Hayes_command_set. Common values are "ATDT" for tone dialing,
   /// and "ATDP" for pulse dialing.
   ///
   /// \pre Channel is not connected. Otherwise the behavior is unpredictable.
   ///
   const MByteString& GetDialString() const
   {
      return m_dialString;
   }
   void SetDialString(const MByteString& dialString)
   {
      m_dialString = dialString;
   }
   ///@}

   ///@{
   /// The phone number to be dialed. Typically, this would be just a phone number like 555-5555,
   /// but it can contain a number of control characters, like P, T, comma, etc. The "DialString"
   /// and "PhoneNumber" are combined to yield the full dialing command that is sent to the
   /// modem.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value "" (empty string)
   ///
   /// \possible_values
   ///  - Your best resource will be your modem manual. You can also find online documentation
   /// for modem commands at sites like www.modem.com/general/extendat.html or
   /// http://en.wikipedia.org/wiki/Hayes_command_set. Some of the possible values are "" (modem
   /// goes off-hook without dialing), "P555-5555" (pulse dial a local number, "T1,(919)
   /// 123-4567" (tone dial a long distance number with a pause after the 1).
   ///
   /// \pre Channel is not connected. Otherwise the behavior is unpredictable.
   ///
   const MByteString& GetPhoneNumber() const
   {
      return m_phoneNumber;
   }
   void SetPhoneNumber(const MByteString& phoneNumber)
   {
      m_phoneNumber = phoneNumber;
   }
   ///@}

   ///@{
   /// Dial timeout.
   ///
   /// Number of seconds to wait for the connection after dialing the phone number. \refprop{SetAutoAnswer,AutoAnswer}
   /// must be False for the channel to make an outbound call. This property is not used for
   /// inbound calls (when Auto Answer = True).
   ///
   /// The connection attempt can take a long time to complete or time out. If the connection is
   /// queued with QConnect and committed asynchronously with QCommit(True), then the connection
   /// can be aborted with QAbort.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 60 seconds
   ///
   /// \possible_values
   ///  - Typically 30 to 60 seconds. Anything below 10 will most likely fail.
   ///
   unsigned GetDialTimeout() const
   {
      return m_dialTimeout;
   }
   void SetDialTimeout(unsigned timeout)
   {
      m_dialTimeout = timeout;
   }
   ///@}

   ///@{
   /// Command timeout.
   ///
   /// Number of seconds to wait for the completion of a command until an answer is received.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value 10 seconds
   ///
   /// \possible_values
   ///  - Typically about 2 seconds for most modems.
   ///
   unsigned GetCommandTimeout() const
   {
      return m_commandTimeout;
   }
   void SetCommandTimeout(unsigned timeout)
   {
      m_commandTimeout = timeout;
   }
   ///@}

   ///@{
   /// Whether to match port and modem baud rates.
   ///
   /// When this flag is true, then during connection, the CONNECT response from the meter is
   /// parsed in order to recognize the line connect speed. If the number is present, the
   /// software tries to set the modem speed to match the connect speed (or to be slightly below it),
   /// so there is no need for the flow control to be imposed between the modem and the computer.
   /// This is rarely needed, but is available for those internal modems which
   /// do not support flow control.
   ///
   /// If the flag is set to false, no attempt to match port and modem baud rates is made.
   ///
   /// \since MeteringSDK Version 2.1.27.
   ///
   /// \default_value False : Satisfactory for most modems
   ///
   /// \possible_values
   ///  - True [1]
   ///  - False [0]
   ///
   bool GetMatchConnectBaud() const
   {
      return m_matchConnectBaud;
   }
   void SetMatchConnectBaud(bool doMatch)
   {
      m_matchConnectBaud = doMatch;
   }
   ///@}

   /// Full modem response after the successful connection, or after any successful control command. 
   ///
   /// All non-printable characters will be replaced with blanks. 
   /// The modem can be configured to respond with numeric codes instead of verbs,
   /// however MeteringSDK can not interpret them and will throw an "unknown response from the
   /// modem" error.
   ///
   /// The modem response is the response sent by the modem in response to a MeteringSDK Connect
   /// attempt. The response could contain text like CONNECT 2400, ERROR, BUSY, NO CARRIER, etc.
   /// anything that the modem will respond with.
   ///
   /// This property will be an empty string if queried before the connect attempt or if
   /// MeteringSDK did not get the modem response during the connect attempt.
   ///
   /// \since MeteringSDK Version 3.1.5.
   ///
   /// \default_value "" (empty string)
   ///
   const MByteString& GetModemResponse() const
   {
      return m_modemResponse;
   }

public: // Overload services:

   /// Request canceling of the communication.
   /// This service is overloaded to support canceling of modem dialing.
   /// Disconnect will be called only if the parameter of the function is true.
   ///
   virtual void CancelCommunication(bool callDisconnect = false);

public: // Modem specific calls:

   /// Auxiliary function "hook on". Hook on is made by setting DTR low, then high, 
   /// which works on the most of modems. 
   /// "+++" command is not used.
   ///
   /// \pre Serial port is opened.
   ///
   void HookOn();

   /// Send command to the modem and complete it with line completion char (see
   /// m_lineCompletionChar).
   ///
   /// \pre Channel is in command mode.
   ///
   void SendCommand(const MByteString& command);

   /// Receive response string from the modem after the command was sent.
   ///
   /// \pre Modem was sent by the command.
   ///
   MByteString ReceiveResponse();

   /// Get the response which is enlisted in MModemResponse type definition.
   /// Possibly return MODEM_RESPONSE_UNKNOWN.
   ///
   /// \pre Channel is in the command mode.
   ///
   MModemResponse ReceiveKnownResponse(int timeout);

   /// Send command and completion char then check response for OK. If no 
   /// response is received before 'm_commandTimeout' timeout expires, 
   /// or the response is not OK then the appropriate exception will be raised.
   ///
   /// \pre Modem is in command mode.
   ///
   void SendCommandCheckOK(const MByteString& command);

private: // Methods:

   void DoAdjustModemAfterConnect();
   bool DoSendCommandWhileWaitingForIncomingConnection(const MByteString& command);
   M_NORETURN_FUNC void DoThrowModemResponseError(MChannelModem::MModemResponse response);

private: // Implementations detail.

   // Whether or not to set the UART-to-modem baud to match the connect speed of the line.
   // Needed for the cases when the modems do not support flow control.
   //
   bool m_matchConnectBaud;

   // True will mean that the Connect was called successfully. Note this is 
   // not the same as IsConnected. In fact, the connection can be broken, 
   // IsConnected will return false, but this variable will be still true. 
   // This is the behavior required for correct handling of 
   // MEChannelDisconnectedUnexpectedly exception, which is throwable
   // only on read and write operation, not at IsConnected.
   //
   bool m_connectCalled;

   // Timeout during which computer waits for modem command response.
   //
   unsigned m_commandTimeout;

   // Timeout during which computer attempts to establish session. Used in 
   // "dial" and "auto answer" modes. In auto answer mode used to control the
   // timeout between the first RING and end of attempt to establish the session.
   //
   unsigned m_dialTimeout;

   // String that puts the modem into the auto answer mode. 
   // It is sent to the modem before waiting for the incoming call. 
   // Should not switch modem to numeric response code.
   //
   // A typical value is ATS0=1.
   //
   MByteString m_autoAnswerString;

   // Initialize string string for modem. Send to the modem before any 
   // connection. Should not switch the modem to numeric response code.
   //
   MByteString m_initString;

   // Dial prefix.
   //
   MByteString m_dialString;

   // Phone number to dial.
   //
   MByteString m_phoneNumber;

   // Modem response string, last one from the modem or an empty string 
   // if no connection was successfully established as a result of the Connect call.
   //
   MByteString m_modemResponse;

   // Thread-protected variable that tells that current action is dialing.
   // This is for supporting canceling the communication.
   //
   MInterlocked m_isDialing;

   // Thread-protected variable that tells that current action is receiving meter response.
   // This is for support of canceling of communication.
   //
   MInterlocked m_isReceivingResponse;

   M_DECLARE_CLASS(ChannelModem)
};

#endif // !M_NO_MCOM_CHANNEL_MODEM

///@}
#endif
