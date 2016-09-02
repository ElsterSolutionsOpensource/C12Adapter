#ifndef MCORE_MCOMMANDLINEPARSER_H
#define MCORE_MCOMMANDLINEPARSER_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MCommandLineParser.h

#include <MCORE/MCOREDefs.h>

#if !M_NO_CONSOLE

/// Command line parser to help dealing with \c argv and \c argc arguments within function \c main.
///
/// This is a C++ only class. Use it to parse UNIX like flags and parameters passed to a command line tool.
/// Here is a typical usage:
/// \code
///      MCommandLineParser parser;
///      parser.SetDescription("Universal communicator command line application");
///      parser.SetBuildDate(__DATE__); // this repeats what is done inside the constructor already
///      parser.DeclareNamedString('f', "config", "Configuration file path", ""), configFile); // -f with value
///      parser.DeclareNamedInt('c', "count", "Device count",                                  // -c with value
///                             "Overwrite count parameter in the configuration file", count);
///      parser.DeclareFlag('s', "save", "Save temporary data", save);                         // -s
///      parser.DeclareFlag('m', "monitor", "Use Monitor.", useMonitor);        // -m
///      parser.DeclareFlag('r', "relay", "Communicate through relay.", useRelay);
///      parser.DeclareNamedString('h', "relay-host", "Relay host.", "", relayHost);
///      int result = parser.Process(argc, argv); // here all the above variables will be initialized
///      if ( result > 0 ) // successful completion with an already handled option such as --version
///         return EXIT_SUCCESS; // everything is done already inside of Process()
///      else if ( result < 0 ) // failure in parameters given, error is reported already
///         return EXIT_FAILURE; // everything is done already inside of Process()
///      // case when the result is zero is when all parameters are parsed
///      // and further processing shall be done by a program
/// \endcode
///
class M_CLASS MCommandLineParser
{
private: // Types:

   // Internal implementation classes
   //
   class MArgumentTarget;
   class FlagTarget;
   class BoolTarget;
   class IntTarget;
   class UnsignedTarget;
   class DoubleTarget;
   class StringTarget;

   // Vector of targets
   //
   typedef std::vector<MArgumentTarget*>
      ArgumentVector;

public: // Constants:

   enum
   {
      HelpOutputLineLength = 80,
      HelpOutputBorder = 20
   };

public:

   /// Create a new command line parser.
   ///
   /// This will typically be a local class within function \c main.
   /// After creation, the next set of actions will be defining parameters and flags with Declare, and calling \ref Process.
   ///
   MCommandLineParser();

   /// Destroy the object and reclaim its resources back to the operating system
   ///
   ~MCommandLineParser();

public: // Properties:

   ///@{
   /// Stream for error reporting
   ///
   MStream* GetErrorStream()
   {
      return m_errorStream;
   }
   void SetErrorStream(MStream* stream)
   {
      M_ASSERT(stream != NULL);
      m_errorStream = stream;
   }
   ///@}

   ///@{
   /// Stream for regular output
   ///
   MStream* GetOutputStream()
   {
      return m_outputStream;
   }
   void SetOutputStream(MStream* stream)
   {
      M_ASSERT(stream != NULL);
      m_outputStream = stream;
   }
   ///@}

   ///@{
   /// Copyright message to use in case the command line parser needs to show it to the user.
   ///
   /// The copyright message is shown as part of usage message.
   /// By default, copyright message is initialized from customization macro \ref M_PRODUCT_LEGAL_COPYRIGHT
   /// and in most cases there is no need to call SetCopyright explicitly.
   /// Otherwise it should be a single string that includes copyright years and message.
   /// Usage example:
   /// \code
   ///    MCommandLineParser parser;
   ///    parser.SetCopyright("Copyright (c) 2000-2010 The CompanyName Here");
   /// \endcode
   ///
   const MStdString& GetCopyright() const
   {
      return m_copyright;
   }
   void SetCopyright(const char* copyrightMessage)
   {
      m_copyright = copyrightMessage;
   }
   void SetCopyright(const MStdString& copyrightMessage)
   {
      m_copyright = copyrightMessage;
   }
   ///@}

   ///@{
   /// Set description message to use in case the command line parser needs to show it to the user.
   ///
   /// The description is shown as part of usage message.
   /// By default, the description is empty, and it is always a good idea to call this method and specify a text.
   /// Usage example:
   /// \code
   ///    MCommandLineParser parser;
   ///    parser.SetDescription("Character encoding tool");
   /// \endcode
   ///
   const MStdString& GetDescription() const
   {
      return m_description;
   }
   void SetDescription(const char* description)
   {
      m_description = description;
   }
   void SetDescription(const MStdString& description)
   {
      m_description = description;
   }
   ///@}

   ///@{
   /// String representation of version for the command line tool
   ///
   /// The version is shown as part of usage message, and also, when the user requests it with --version flag.
   /// Typically, there is no need to specify the version with this call, as by default,
   /// \ref M_PRODUCT_VERSION customization macro is used.
   ///
   /// Usage example:
   /// \code
   ///    MCommandLineParser parser;
   ///    parser.SetVersion("1.2");
   /// \endcode
   ///
   const MStdString& GetVersion() const
   {
      return m_version;
   }
   void SetVersion(const char* versionString)
   {
      m_version = versionString;
   }
   void SetVersion(const MStdString& versionString)
   {
      m_version = versionString;
   }
   ///@}

   ///@{
   /// Executable name, passed explicitly or fetched from argv
   ///
   /// Typically, there is no need to specify the name of the executable,
   /// as it will be taken from the \c argv parameter at the time \ref Process is called.
   /// This method is useful if there is a necessity to overwrite the default name.
   ///
   const MStdString& GetExecutableName() const
   {
      return m_executableName;
   }
   void SetExecutableName(const char* name)
   {
      m_executableName = name;
   }
   void SetExecutableName(const MStdString& name)
   {
      m_executableName = name;
   }
   ///@}

   ///@{
   /// Build date, if it has to be passed explicitly.
   ///
   /// Typically, there is no need to specify the build date, as it is assigned from __DATE__ macro
   /// at the time of library compilation. This call is useful if the library is compiled
   /// separately from the application that uses it, in which case the usage will be like:
   /// \code
   ///      MCommandLineParser parser;
   ///      parser.SetBuildDate(__DATE__);
   /// \endcode
   ///
   const MStdString& GetBuildDate() const
   {
      return m_date;
   }
   void SetBuildDate(const char* date)
   {
      m_date = date;
   }
   void SetBuildDate(const MStdString& date)
   {
      m_date = date;
   }
   ///@}

   ///@{
   /// Set footer message to use in case the command line parser needs to show it to the user.
   ///
   /// The footer is typically a list of examples and further references.
   /// By default, the footer is empty.
   ///
   const MStdString& GetFooter() const
   {
      return m_footer;
   }
   void SetFooter(const char* footer)
   {
      m_footer = footer;
   }
   void SetFooter(const MStdString& footer)
   {
      m_footer = footer;
   }
   ///@}

public: // Methods for named entities such as flags and flags with value:

   /// Declare boolean flag that the command line will take.
   ///
   /// Flag is a named entity with a short name such as \c -r, or long name such as \c --recursive.
   /// Presence of the flag in the command line is determined by a boolean parameter \c destination,
   /// which shall be initialized into \c false prior to this call.
   ///
   /// \param shortName
   ///    One character, the short flag name, such as 'r' for '-r' flag.
   ///    If this is '\0' the flag has no short name (it has to have a long name in this case)
   ///
   /// \param longName
   ///    Parameter long name, such as 'recursive' for '--recursive' flag.
   ///    If this is an empty string the flag has no long name (it has to have a short name in this case)
   ///
   /// \param description
   ///    Description of the flag, an English text.
   ///    This text is shown to the user at \c --help option.
   ///    Text wrapping is done automatically, therefore, the description shall not have line breaks.
   ///
   /// \param destination
   ///    Reference to boolean value that will be controlled by presence of such flag in the command line.
   ///    The particular variable to which this parameter refers will typically be initialized with false.
   ///    When the flag is present in the command line, after calling any of Process functions,
   ///    the value will become true.
   ///
   void DeclareFlag(char shortName, const char* longName, const char* description, bool& destination);

   /// Declare a named argument of boolean type, one given as a flag with value.
   ///
   /// Such named argument is always optional, and its presence can be determined by whether or not
   /// the default value of destination is unchanged. It can appear at any position in the list.
   /// Example:
   /// \code
   ///     parser.DeclareNamedBool('g', "go", "Go or no go", goBool);
   ///     // possible uses in the command line:
   ///     //    -g1
   ///     //    -g Y
   ///     //    --go=f
   ///     //    --go n
   /// \endcode
   /// If the argument is not supplied, the above mentioned \c goBool will be unchanged at \ref Process call.
   ///
   /// The accepted argument for boolean types are:
   ///    - "false", "no", 'f', 'n' or '0' for false, all letters are case insensitive
   ///    - "true", "yes", 't', 'y' or '1' for true, all letters are case insensitive
   ///
   /// Different from the other named parameters, the boolean variant does not have label, as it will always be "0/1"
   ///
   /// \param shortName
   ///    One character, the short flag name, such as 'g' for '-g1' flag with value.
   ///    If this is '\0' the flag has no short name (it has to have a long name in this case)
   ///
   /// \param longName
   ///    Parameter long name, such as 'go' for '--go=0' flag with value.
   ///    If this is an empty string the flag has no long name (it has to have a short name in this case)
   ///
   /// \param description
   ///    Description of the flag, an English text.
   ///    This text is shown to the user at \c --help option.
   ///    Text wrapping is done automatically, therefore, the description shall not have line breaks.
   ///
   /// \param destination
   ///    Reference to a boolean value into which the argument will be written at the time
   ///    the \ref Process method is called.
   ///
   void DeclareNamedBool(char shortName, const char* longName, const char* description, bool& destination);

   /// Declare a named argument of integer type, one given as a flag with value.
   ///
   /// Such argument is always optional, and its presence can be determined by whether or not
   /// the default value of destination is unchanged. It can appear at any position in the list.
   /// Example:
   /// \code
   ///     parser.DeclareNamedString('n', "number", "cnt", "Number of actions", num);
   ///     // possible uses in the command line:
   ///     //    -n3
   ///     //    -n 5
   ///     //    --number=5
   ///     //    --number 5
   /// \endcode
   /// If the argument is not supplied, the above mentioned \c num will be unchanged at \ref Process call.
   ///
   /// \param shortName
   ///    One character, the short flag name, such as 'n' for '-n4' flag with value.
   ///    If this is '\0' the flag has no short name (it has to have a long name in this case)
   ///
   /// \param longName
   ///    Parameter long name, such as 'number' for '--number=4' flag with value.
   ///    If this is an empty string the flag has no long name (it has to have a short name in this case)
   ///
   /// \param label
   ///    Name of the value for the argument, typically one word, appears in help as value placeholder.
   ///
   /// \param description
   ///    Description of the flag, an English text.
   ///    This text is shown to the user at \c --help option.
   ///    Text wrapping is done automatically, therefore, the description shall not have line breaks.
   ///
   /// \param destination
   ///    Reference to a integer value into which the argument will be written at the time
   ///    the \ref Process method is called.
   ///
   void DeclareNamedInt(char shortName, const char* longName, const char* label, const char* description, int& destination);

   /// Declare a named argument of unsigned integer type, one given as a flag with value.
   ///
   /// Such argument is always optional, and its presence can be determined by whether or not
   /// the default value of destination is unchanged. It can appear at any position in the list.
   /// Example:
   /// \code
   ///     parser.DeclareNamedUnsignedString('n', "number", "cnt", "Number of actions", num);
   ///     // possible uses in the command line:
   ///     //    -n3
   ///     //    -n 5
   ///     //    --number=5
   ///     //    --number 5
   /// \endcode
   /// If the argument is not supplied, the above mentioned \c num will be unchanged at \ref Process call.
   ///
   /// \param shortName
   ///    One character, the short flag name, such as 'n' for '-n4' flag with value.
   ///    If this is '\0' the flag has no short name (it has to have a long name in this case)
   ///
   /// \param longName
   ///    Parameter long name, such as 'number' for '--number=4' flag with value.
   ///    If this is an empty string the flag has no long name (it has to have a short name in this case)
   ///
   /// \param label
   ///    Name of the value for the argument, typically one word, appears in help as value placeholder.
   ///
   /// \param description
   ///    Description of the flag, an English text.
   ///    This text is shown to the user at \c --help option.
   ///    Text wrapping is done automatically, therefore, the description shall not have line breaks.
   ///
   /// \param destination
   ///    Reference to an unsigned integer value into which the argument will be written at the time
   ///    the \ref Process method is called.
   ///
   void DeclareNamedUnsignedInt(char shortName, const char* longName, const char* label, const char* description, unsigned& destination);

   /// Declare a named argument of double precision floating point type, one given as a flag with value.
   ///
   /// Such argument is always optional, and its presence can be determined by whether or not
   /// the default value of destination is unchanged. It can appear at any position in the list.
   /// Example:
   /// \code
   ///     parser.DeclareNamedString('m', "multiplier", "mul", "Multiplier of the value", mul);
   ///     // possible uses in the command line:
   ///     //    -m3.14
   ///     //    -m 5.7
   ///     //    --multiplier=5.0
   ///     //    --multiplier 123
   /// \endcode
   /// If the argument is not supplied, the above mentioned \c mul will be unchanged at \ref Process call.
   ///
   /// \param shortName
   ///    One character, the short flag name, such as 'x' for '-x3.13' flag with value.
   ///    If this is '\0' the flag has no short name (it has to have a long name in this case)
   ///
   /// \param longName
   ///    Parameter long name, such as 'number' for '--number=4' flag with value.
   ///    If this is an empty string the flag has no long name (it has to have a short name in this case)
   ///
   /// \param label
   ///    Name of the value for the argument, typically one word, appears in help as value placeholder.
   ///
   /// \param description
   ///    Description of the flag, an English text.
   ///    This text is shown to the user at \c --help option.
   ///    Text wrapping is done automatically, therefore, the description shall not have line breaks.
   ///
   /// \param destination
   ///    Reference to a double precision floating point value into which the argument will be written at the time
   ///    the \ref Process method is called.
   ///
   void DeclareNamedDouble(char shortName, const char* longName, const char* label, const char* description, double& destination);

   /// Declare a named argument of string type, one given as a flag with value.
   ///
   /// Such argument is always optional, and its presence can be determined by whether or not
   /// the default value of destination is unchanged. It can appear at any position in the list.
   /// Example:
   /// \code
   ///     parser.DeclareNamedString('o', "output-file", "file.name", "Output file name", outputFile);
   ///     // possible uses in the command line:
   ///     //    -ofilename.txt
   ///     //    -o filename.txt
   ///     //    --output-file=filename.txt
   ///     //    --output-file filename.txt
   /// \endcode
   /// If the argument is not supplied, the above mentioned \c outputFile will be unchanged at \ref Process call.
   ///
   /// \param shortName
   ///    One character, the short flag name, such as 'f' for '-f file.txt' flag with value.
   ///    If this is '\0' the flag has no short name (it has to have a long name in this case)
   ///
   /// \param longName
   ///    Parameter long name, such as 'number' for '--number=4' flag with value.
   ///    If this is an empty string the flag has no long name (it has to have a short name in this case)
   ///
   /// \param label
   ///    Name of the value for the argument, typically one word, appears in help as value placeholder.
   ///
   /// \param description
   ///    Description of the flag, an English text.
   ///    This text is shown to the user at \c --help option.
   ///    Text wrapping is done automatically, therefore, the description shall not have line breaks.
   ///
   /// \param destination
   ///    Reference to a string value into which the argument will be written at the time
   ///    the \ref Process method is called.
   ///
   void DeclareNamedString(char shortName, const char* longName, const char* label, const char* description, MStdString& destination);

public: // Methods for required command line parameters of different type:

   /// Declare mandatory positional argument of Boolean type.
   ///
   /// The order of positional Declare calls determines the order of arguments.
   /// For example:
   /// \code
   ///     parser.DeclareBool("print", "Print or not", print);
   ///     parser.DeclareInt("times", "How many times to perform an action", times);
   /// \endcode
   /// Determines two parameters, first is a Boolean and a second is an integer.
   ///
   /// The accepted argument for boolean types are:
   ///    - "false", "no", 'f', 'n' or '0' for false, all letters are case insensitive
   ///    - "true", "yes", 't', 'y' or '1' for true, all letters are case insensitive
   ///
   /// \param label
   ///    Name of the value for the argument, typically one word, appears in help as value placeholder.
   ///
   /// \param description
   ///    Description of the flag, an English text.
   ///    This text is shown to the user at \c --help option.
   ///    Text wrapping is done automatically, therefore, the description shall not have line breaks.
   ///
   /// \param destination
   ///    Reference to an integer value into which the argument will be written at the time
   ///    the \ref Process method is called.
   ///
   void DeclareBool(const char* label, const char* description, bool& destination);

   /// Declare mandatory positional argument of integer type.
   ///
   /// The order of positional Declare calls determines the order of arguments.
   /// For example:
   /// \code
   ///     parser.DeclareString("action", "Action to perform", action);
   ///     parser.DeclareInt("times", "How many times to perform an action", times);
   /// \endcode
   /// Determines two parameters, first is a string and a second is an integer.
   ///
   /// \param label
   ///    Name of the value for the argument, typically one word, appears in help as value placeholder.
   ///
   /// \param description
   ///    Description of the flag, an English text.
   ///    This text is shown to the user at \c --help option.
   ///    Text wrapping is done automatically, therefore, the description shall not have line breaks.
   ///
   /// \param destination
   ///    Reference to an integer value into which the argument will be written at the time
   ///    the \ref Process method is called.
   ///
   void DeclareInt(const char* label, const char* description, int& destination);

   /// Declare mandatory positional argument of unsigned integer type.
   ///
   /// The order of positional Declare calls determines the order of arguments.
   /// For example:
   /// \code
   ///     parser.DeclareString("action", "Action to perform", action);
   ///     parser.DeclareUnsignedInt("number", "How many times to execute action", number);
   /// \endcode
   /// Determines two parameters, first is a string, and a second is an unsigned number.
   ///
   /// \param label
   ///    Name of the value for the argument, typically one word, appears in help as value placeholder.
   ///
   /// \param description
   ///    Description of the flag, an English text.
   ///    This text is shown to the user at \c --help option.
   ///    Text wrapping is done automatically, therefore, the description shall not have line breaks.
   ///
   /// \param destination
   ///    Reference to an unsigned number value into which the argument will be written at the time
   ///    the \ref Process method is called.
   ///
   void DeclareUnsignedInt(const char* label, const char* description, unsigned& destination);

   /// Declare mandatory positional argument of double precision floating point type.
   ///
   /// The order of positional Declare calls determines the order of arguments.
   /// For example:
   /// \code
   ///     parser.DeclareString("action", "Action to perform", action);
   ///     parser.DeclareDouble("multiplier", "Multiplier parameter", multiplier);
   /// \endcode
   /// Determines two parameters, first is a string and a second is a floating point number.
   ///
   /// \param label
   ///    Name of the value for the argument, typically one word, appears in help as value placeholder.
   ///
   /// \param description
   ///    Description of the flag, an English text.
   ///    This text is shown to the user at \c --help option.
   ///    Text wrapping is done automatically, therefore, the description shall not have line breaks.
   ///
   /// \param destination
   ///    Reference to a floating point number value into which the argument will be written at the time
   ///    the \ref Process method is called.
   ///
   void DeclareDouble(const char* label, const char* description, double& destination);

   /// Declare mandatory positional argument of string type.
   ///
   /// The order of positional Declare calls determines the order of arguments.
   /// For example:
   /// \code
   ///     parser.DeclareString("source", "Source from which to copy", src);
   ///     parser.DeclareString("destination", "Where to copy", destination);
   /// \endcode
   /// Determines the parameters for a command line utility that copies one given entity into another.
   ///
   /// \param label
   ///    Name of the value for the argument, typically one word, appears in help as value placeholder.
   ///
   /// \param description
   ///    Description of the flag, an English text.
   ///    This text is shown to the user at \c --help option.
   ///    Text wrapping is done automatically, therefore, the description shall not have line breaks.
   ///
   /// \param destination
   ///    Reference to a string value into which the argument will be written at the time
   ///    the \ref Process method is called.
   ///
   /// \see DeclareOptionalString - optional argument
   ///
   void DeclareString(const char* label, const char* description, MStdString& destination);

public: // Methods for optional command line parameters of different type:

   /// Declare optional positional argument of integer type.
   ///
   /// The order of positional Declare calls determines the order of arguments.
   /// Optional arguments can only be at the tail of the argument list,
   /// if any of them are present, \ref DeclareStringVector shall not be used.
   /// Whether or not an optional argument was specified in the command line can determined by
   /// whether or not the destination variable was changed from its initial value.
   ///
   /// \param label
   ///    Name of the value for the argument, typically one word, appears in help as value placeholder.
   ///
   /// \param description
   ///    Description of the flag, an English text.
   ///    This text is shown to the user at \c --help option.
   ///    Text wrapping is done automatically, therefore, the description shall not have line breaks.
   ///
   /// \param destination
   ///    Reference to an integer value into which the argument will be written at the time
   ///    the \ref Process method is called.
   ///
   /// \see DeclareInt - mandatory integer argument
   ///
   void DeclareOptionalInt(const char* label, const char* description, int& destination);

   /// Declare optional positional argument of unsigned integer type.
   ///
   /// The order of positional Declare calls determines the order of arguments.
   /// Optional arguments can only be at the tail of the argument list,
   /// if any of them are present, \ref DeclareStringVector shall not be used.
   /// Whether or not an optional argument was specified in the command line can determined by
   /// whether or not the destination variable was changed from its initial value.
   ///
   /// \param label
   ///    Name of the value for the argument, typically one word, appears in help as value placeholder.
   ///
   /// \param description
   ///    Description of the flag, an English text.
   ///    This text is shown to the user at \c --help option.
   ///    Text wrapping is done automatically, therefore, the description shall not have line breaks.
   ///
   /// \param destination
   ///    Reference to an unsigned integer value into which the argument will be written at the time
   ///    the \ref Process method is called.
   ///
   /// \see DeclareUnsignedInt - mandatory unsigned integer argument
   ///
   void DeclareOptionalUnsignedInt(const char* label, const char* description, unsigned& destination);

   /// Declare optional positional argument of double precision floating point type.
   ///
   /// The order of positional Declare calls determines the order of arguments.
   /// Optional arguments can only be at the tail of the argument list,
   /// if any of them are present, \ref DeclareStringVector shall not be used.
   /// Whether or not an optional argument was specified in the command line can determined by
   /// whether or not the destination variable was changed from its initial value.
   ///
   /// \param label
   ///    Name of the value for the argument, typically one word, appears in help as value placeholder.
   ///
   /// \param description
   ///    Description of the flag, an English text.
   ///    This text is shown to the user at \c --help option.
   ///    Text wrapping is done automatically, therefore, the description shall not have line breaks.
   ///
   /// \param destination
   ///    Reference to a double precision floating point value into which the argument will be written at the time
   ///    the \ref Process method is called.
   ///
   /// \see DeclareDouble - mandatory double precision floating point argument
   ///
   void DeclareOptionalDouble(const char* label, const char* description, double& destination);

   /// Declare optional positional argument of string type.
   ///
   /// The order of positional Declare calls determines the order of arguments.
   /// Optional arguments can only be at the tail of the argument list,
   /// if any of them are present, \ref DeclareStringVector shall not be used.
   /// Whether or not an optional argument was specified in the command line can determined by
   /// whether or not the destination variable was changed from its initial value.
   ///
   /// \param label
   ///    Name of the value for the argument, typically one word, appears in help as value placeholder.
   ///
   /// \param description
   ///    Description of the flag, an English text.
   ///    This text is shown to the user at \c --help option.
   ///    Text wrapping is done automatically, therefore, the description shall not have line breaks.
   ///
   /// \param destination
   ///    Reference to a string value into which the argument will be written at the time
   ///    the \ref Process method is called.
   ///
   /// \see DeclareString - mandatory argument
   /// \see DeclareStringVector - variable size list of arguments
   ///
   void DeclareOptionalString(const char* label, const char* description, MStdString& destination);

   /// Declare a list of positional arguments, strings.
   ///
   /// This call will declare a list of arguments, and it is the only way to have optional parameters without names.
   /// The list will be located after all positional arguments.
   ///
   /// \param label
   ///    Name of the value for the argument, typically one word, appears in help as value placeholder.
   ///
   /// \param description
   ///    Description of the flag, an English text.
   ///    This text is shown to the user at \c --help option.
   ///    Text wrapping is done automatically, therefore, the description shall not have line breaks.
   ///
   /// \param destination
   ///    Reference to a string vector into which the argument will be written at the time
   ///    the \ref Process method is called.
   ///
   void DeclareStringVector(const char* label, const char* description, MStdStringVector& destination);

public: // Action methods callable after property assignments and Declare:

   ///@{
   /// Process the arguments given as argc and argv using previously defined command line flags and parameters
   ///
   /// Prior to this call, the parameters and flags shall be declared with a set of Declare methods.
   ///
   /// \param argc
   ///     Argument count as given to function \c main
   /// \param argv
   ///     List of arguments as given to function \c main.
   ///     ANSI C standard requires the item after last to be NULL.
   /// \return int - success indicator with the following possible values:
   ///     - Positive value means the parameters are processed successfully, but the given flags are as such that nothing
   ///       shall be done by a program. For example, this is how \c --help or \c --version are handled.
   ///     - Negative value means there is an error in parameters detected, this error is reported,
   ///       and the program shall immediately exit with failure status.
   ///     - Zero means the parameters are processed successfully and variables given to Declare methods are initialized.
   ///       The program is expected to continue with its own actions based on parameters supplied.
   ///
   int Process(int argc, const char** argv);
   int Process(int argc, char** argv)
   {
      return Process(argc, const_cast<const char**>(argv));
   }
   ///@}

   ///@{
   /// Process command line from string using previously defined command line flags and parameters
   ///
   /// Prior to this call, the parameters and flags shall be declared with a set of Declare methods.
   ///
   /// \param commandLine
   ///     Whole command line including the application name
   /// \return int - success indicator with the following possible values:
   ///     - Positive value means the parameters are processed successfully, but the given flags are as such that nothing
   ///       shall be done by a program. For example, this is how \c --help or \c --version are handled.
   ///     - Negative value means there is an error in parameters detected, this error is reported,
   ///       and the program shall immediately exit with failure status.
   ///     - Zero means the parameters are processed successfully and variables given to Declare methods are initialized.
   ///       The program is expected to continue with its own actions based on parameters supplied.
   ///
   int Process(const char* commandLine);
   int Process(const MStdString& commandLine)
   {
      return Process(commandLine.c_str());
   }
   ///@)

   /// Write program help to the output stream
   ///
   /// Write Header, Usage and Footer to the output stream.
   /// This method is of rare use, as the command line parser
   /// handles \c --help and \c -h all by itself internally.
   ///
   /// \see WriteHeader
   /// \see WriteUsage
   /// \see WriteFooter
   ///
   void WriteHelp();

   /// Write software header to the stream.
   ///
   /// Write program name and copyright message to the output stream.
   ///
   /// \see WriteUsage
   /// \see WriteFooter
   /// \see WriteHelp - combines WriteHeader, WriteUsage, and WriteFooter
   ///
   void WriteHeader();

   /// Write usage to the output stream supplied based on the already given Declare methods.
   ///
   /// \see WriteHeader
   /// \see WriteFooter
   /// \see WriteHelp - combines WriteHeader, WriteUsage, and WriteFooter
   ///
   void WriteUsage();

   /// Write software header to the stream.
   ///
   /// \see WriteHeader
   /// \see WriteUsage
   /// \see WriteHelp - combines WriteHeader, WriteUsage, and WriteFooter
   ///
   void WriteFooter();

   /// Write the given exception to the error stream
   ///
   /// This is a convenience method for helping programs report errors
   ///
   /// \param ex Exception to report
   ///
   void WriteException(MException& ex);

   /// Write the given error text with variable number of arguments to the error stream
   ///
   /// This is a convenience method for helping programs report errors
   ///
   /// \param fmt Standard C format string followed by arguments
   ///
   void WriteError(const char* fmt, ...);

   /// Write the given error text to the error stream
   ///
   /// This is a convenience method for helping programs report errors
   ///
   /// \param text String to write as error
   ///
   void WriteError(const MStdString& text);

private: // methods:

   // prevent copying
   //
   MCommandLineParser(const MCommandLineParser&);
   const MCommandLineParser& operator=(const MCommandLineParser&);

   void DoAddNamedArgument(MArgumentTarget*);
   MArgumentTarget* DoFindNamedArgument(const MStdString& longName);
   MArgumentTarget* DoFindNamedArgument(char shortName);
   void DoDeleteElements(ArgumentVector& vect);

private: // Declarations:

   // Name of the program. If not given explicitly it is fetched from argv
   //
   MStdString m_executableName;

   // Copyright message
   //
   MStdString m_copyright;

   // Program description, typically a single string
   //
   MStdString m_description;

   // String representation of program creation date, can be empty
   //
   MStdString m_date;

   // String representation of version
   //
   MStdString m_version;

   // Footer text to output by --help, empty by default
   //
   MStdString m_footer;

   // Error stream to use for all error output
   //
   MStream* m_errorStream;

   // Output stream to use for --help and --version
   //
   MStream* m_outputStream;

   // Named arguments such as -v or --verbose
   //
   ArgumentVector m_namedArguments;

   // Positional arguments
   //
   ArgumentVector m_positionalArguments;

   // Optional positional arguments, cannot appear together with argument list
   //
   ArgumentVector m_optionalArguments;

   // Label of variable size list of optional positional arguments, cannot appear together with optional arguments
   //
   MStdString m_argumentListLabel;

   // Description of variable size list of optional positional arguments, cannot appear together with optional arguments
   //
   MStdString m_argumentListDescription;

   // Value of argument list, an array of strings
   //
   MStdStringVector* m_argumentListValue;
};

#endif // !M_NO_CONSOLE
///@}
#endif // MCORE_MCOMMANDLINEPARSER_H
