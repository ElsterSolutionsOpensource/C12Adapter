// File MCOM/MCommandLineParser.cpp

#include "MCOREExtern.h"
#include "MCommandLineParser.h"
#include "MUtilities.h"
#include "MStreamFile.h"
#include "MStr.h"

#if !M_NO_CONSOLE

   static void DoAppendPaddingBeforeDescriptioon(MStdString& str)
   {
      MStdString::size_type namesSize = str.size();
      str += ' ';
      int padding = static_cast<int>(MCommandLineParser::HelpOutputBorder) - static_cast<int>(namesSize) - 1;
      if ( padding > 0 )
         str.append(padding, ' ');
   }

// This is a base class for representing one argument value.
// It is inherited by many classes to represent the different types. 
//
class MCommandLineParser::MArgumentTarget
{
public: // Fields:

   MStdString m_label;            // Placeholder for value in help text
   MStdString m_description;      // Description of the value
   MStdString m_longName;         // Long name such as 'verbose' in '--verbose'
   char       m_shortName;        // A single character, flag such as 'v' in '-v'. Zero if this is not named
   bool       m_defaultPresent;   // Whether the default value has to be mentioned in help text

public: // Constructor, destructor:

   MArgumentTarget(const char* label, const char* description, const char* longName, char shortName, bool defaultPresent);

   virtual ~MArgumentTarget();

public: // Properties:

   bool IsParameterPresent() const
   {
      return m_defaultPresent && IsNamed(); // all named except of flags
   }

   bool IsNamed() const
   {
      return m_shortName != '\0' || !m_longName.empty();
   }

public: // Methods:

   MStdString GetNameOrLabel() const;

   virtual void SetValue(const char* value) = 0;

   virtual void AddValue(MStdString&) const = 0;

   void AddParameter(MStdString& str) const;

   MStdString GetUsage() const;
};

MCommandLineParser::MArgumentTarget::MArgumentTarget(const char* label, const char* description, const char* longName, char shortName, bool defaultPresent)
:
   m_label(label),
   m_description(description),
   m_longName(longName),
   m_shortName(shortName),
   m_defaultPresent(defaultPresent)
{
}

MCommandLineParser::MArgumentTarget::~MArgumentTarget()
{
}

MStdString MCommandLineParser::MArgumentTarget::GetNameOrLabel() const
{
   MStdString result;
   if ( !m_longName.empty() )
   {
      result.assign("--", 2);
      result += m_longName;
   }
   else if ( m_shortName != '\0' )
   {
      result += '-';
      result += m_shortName;
   }
   else
      result = m_label;
   return result;
}

void MCommandLineParser::MArgumentTarget::AddParameter(MStdString& str) const
{
   str += '<';
   str += m_label;
   str += '>';
}

MStdString MCommandLineParser::MArgumentTarget::GetUsage() const
{
   MStdString result;
   result.assign("  ", 2);
   if ( IsNamed() )
   {
      if ( m_shortName != '\0' )
      {
         result += '-';
         result += m_shortName;
         if ( IsParameterPresent() )
            AddParameter(result);
      }
      if ( !m_longName.empty() )
      {
         if ( m_shortName != '\0' )
            result.append("  --", 4);
         else
            result.append("--", 2);
         result += m_longName;
         if ( IsParameterPresent() )
         {
            result += '=';
            AddParameter(result);
         }
      }
   }
   else
      AddParameter(result);
   DoAppendPaddingBeforeDescriptioon(result);
   result += m_description;
   if ( m_defaultPresent )
   {
      result.append(". Default: ");
      AddValue(result);
   }
   result = MStr::WordWrap(result, MCommandLineParser::HelpOutputBorder, MCommandLineParser::HelpOutputLineLength);
   result += '\n';
   return result;
}

class MCommandLineParser::FlagTarget : public MCommandLineParser::MArgumentTarget
{
public: // Data:

   bool& m_value;

public: // Constructor, destructor, methods:

   FlagTarget(bool& value, const char* description, const char* longName = "", char shortName = '\0')
   : 
      MArgumentTarget("", description, longName, shortName, false),
      m_value(value)
   {
   }

   virtual ~FlagTarget()
   {
   }

   virtual void SetValue(const char*)
   {
      m_value = true; // mo matter what
   }

   virtual void AddValue(MStdString& result) const
   {
      result += m_value ? '1' : '0';
   }
};

class MCommandLineParser::DoubleTarget : public MArgumentTarget
{
public: // Fields:

   double& m_value;

public: // Constructor, destructor, methods:

   DoubleTarget(double& value, const char* label, const char* description, const char* longName = "", char shortName = '\0', bool defaultPresent = false)
   :
      MArgumentTarget(label, description, longName, shortName, defaultPresent),
      m_value(value)
   {
   }

   virtual ~DoubleTarget()
   {
   }

   virtual void SetValue(const char* value)
   {
      m_value = MToDouble(value);
   }

   virtual void AddValue(MStdString& result) const
   {
      result += MToStdString(m_value);
   }
};

class MCommandLineParser::BoolTarget : public MArgumentTarget
{
public: // Fields:

   bool& m_value;

public: // Constructor, destructor, methods:

   BoolTarget(bool& value, const char* description, const char* longName = "", char shortName = '\0', bool defaultPresent = false)
   :
      MArgumentTarget("0/1", description, longName, shortName, defaultPresent),
      m_value(value)
   {
   }

   virtual ~BoolTarget()
   {
   }

   virtual void SetValue(const char* value)
   {
      M_ASSERT(value != NULL);
      size_t len = strlen(value);
      if ( len == 1 )
      {
         if ( strchr("0fFnN", value[0]) != NULL )
         {
            m_value = false;
            return;
         }
         else if ( strchr("1tTyY", value[0]) != NULL )
         {
            m_value = true;
            return;
         }
      }
      else if ( stricmp(value, "false") == 0 || stricmp(value, "no") == 0 )
      {
         m_value = false;
         return;
      }
      else if ( stricmp(value, "true") == 0 || stricmp(value, "yes") == 0 )
      {
         m_value = true;
         return;
      }
      MException::Throw(MErrorEnum::BadConversion, "Could not convert %s to a boolean, expected 0/1, N/Y, or F/T", value);
      M_ENSURED_ASSERT(0);
   }

   virtual void AddValue(MStdString& result) const
   {
      result += m_value ? '1' : '0';
   }
};

class MCommandLineParser::IntTarget : public MArgumentTarget
{
public: // Fields:

   int& m_value;

public: // Constructor, destructor, methods:

   IntTarget(int& value, const char* label, const char* description, const char* longName = "", char shortName = '\0', bool defaultPresent = false)
   :
      MArgumentTarget(label, description, longName, shortName, defaultPresent),
      m_value(value)
   {
   }

   virtual ~IntTarget()
   {
   }

   virtual void SetValue(const char* value)
   {
      m_value = MToInt(value);
   }

   virtual void AddValue(MStdString& result) const
   {
      result += MToStdString(m_value);
   }
};

class MCommandLineParser::UnsignedTarget : public MArgumentTarget
{
public: // Fields:

   unsigned& m_value;

public: // Constructor, destructor, methods:

   UnsignedTarget(unsigned& value, const char* label, const char* description, const char* longName = "", char shortName = '\0', bool defaultPresent = false)
   :
      MArgumentTarget(label, description, longName, shortName, defaultPresent),
      m_value(value)
   {
   }

   virtual ~UnsignedTarget()
   {
   }

   virtual void SetValue(const char* value)
   {
      m_value = MToUnsigned(value);
   }

   virtual void AddValue(MStdString& result) const
   {
      result += MToStdString(m_value);
   }
};

class MCommandLineParser::StringTarget : public MArgumentTarget
{
public: // Fields:

   MStdString& m_value;

public: // Constructor, destructor, methods:

   StringTarget(MStdString& value, const char* label, const char* description, const char* longName = "", char shortName = '\0', bool defaultPresent = false)
   :
      MArgumentTarget(label, description, longName, shortName, defaultPresent),
      m_value(value)
   {
   }

   virtual ~StringTarget()
   {
   }

   virtual void SetValue(const char* value)
   {
      m_value = value;
   }

   virtual void AddValue(MStdString& result) const
   {
      result += '"';
      result += m_value;
      result += '"';
   }
};


MCommandLineParser::MCommandLineParser()
:
   m_executableName(),
   m_copyright(M_PRODUCT_LEGAL_COPYRIGHT),
   m_date(__DATE__),
   m_version(M_PRODUCT_VERSION_STRING),
   m_footer(),
   m_errorStream(MStreamFile::GetStdErr()),
   m_outputStream(MStreamFile::GetStdOut()),
   m_namedArguments(),
   m_positionalArguments(),
   m_optionalArguments(),
   m_argumentListLabel(),
   m_argumentListDescription(),
   m_argumentListValue(NULL)
{
}

MCommandLineParser::~MCommandLineParser()
{
   DoDeleteElements(m_namedArguments);
   DoDeleteElements(m_positionalArguments);
   DoDeleteElements(m_optionalArguments);
}

void MCommandLineParser::DoAddNamedArgument(MArgumentTarget* t)
{
   M_ASSERT(t != NULL);
   M_ASSERT(t->IsNamed());
   M_ASSERT(t->m_shortName != '\0' || !t->m_longName.empty());
   M_ASSERT(t->m_shortName == '\0' || DoFindNamedArgument(t->m_shortName) == NULL); // check no duplicates
   M_ASSERT(t->m_longName.empty()  || DoFindNamedArgument(t->m_longName) == NULL);  // check no duplicates

   m_namedArguments.push_back(t);
}

MCommandLineParser::MArgumentTarget* MCommandLineParser::DoFindNamedArgument(const MStdString& longName)
{
   M_ASSERT(!longName.empty());
   ArgumentVector::const_iterator it = m_namedArguments.begin();
   ArgumentVector::const_iterator itEnd = m_namedArguments.end();
   for ( ; it != itEnd; ++it )
      if ( (*it)->m_longName == longName )
         return *it;
   return NULL;
}

MCommandLineParser::MArgumentTarget* MCommandLineParser::DoFindNamedArgument(char shortName)
{
   M_ASSERT(shortName != '\0');
   ArgumentVector::const_iterator it = m_namedArguments.begin();
   ArgumentVector::const_iterator itEnd = m_namedArguments.end();
   for ( ; it != itEnd; ++it )
      if ( (*it)->m_shortName == shortName )
         return *it;
   return NULL;
}

void MCommandLineParser::DoDeleteElements(ArgumentVector& vect)
{
   ArgumentVector::iterator it = vect.begin();
   ArgumentVector::iterator itEnd = vect.end();
   for ( ; it != itEnd; ++it )
      delete *it;
}

// Flags and named entities

void MCommandLineParser::DeclareFlag(char shortName, const char* longName, const char* description, bool& destination)
{
   MArgumentTarget* t = M_NEW FlagTarget(destination, description, longName, shortName);
   DoAddNamedArgument(t);
}

void MCommandLineParser::DeclareNamedBool(char shortName, const char* longName, const char* description, bool& destination)
{
   MArgumentTarget* t = M_NEW BoolTarget(destination, description, longName, shortName, true);
   DoAddNamedArgument(t);
}

void MCommandLineParser::DeclareNamedInt(char shortName, const char* longName, const char* label, const char* description, int& destination)
{
   MArgumentTarget* t = M_NEW IntTarget(destination, label, description, longName, shortName, true);
   DoAddNamedArgument(t);
}

void MCommandLineParser::DeclareNamedUnsignedInt(char shortName, const char* longName, const char* label, const char* description, unsigned& destination)
{
   MArgumentTarget* t = M_NEW UnsignedTarget(destination, label, description, longName, shortName, true);
   DoAddNamedArgument(t);
}

void MCommandLineParser::DeclareNamedDouble(char shortName, const char* longName, const char* label, const char* description, double& destination)
{
   MArgumentTarget* t = M_NEW DoubleTarget(destination, label, description, longName, shortName, true);
   DoAddNamedArgument(t);
}

void MCommandLineParser::DeclareNamedString(char shortName, const char* longName, const char* label, const char* description, MStdString& destination)
{
   MArgumentTarget* t = M_NEW StringTarget(destination, label, description, longName, shortName, true);
   DoAddNamedArgument(t);
}

// Required parameters

void MCommandLineParser::DeclareBool(const char* label, const char* description, bool& destination)
{
   MArgumentTarget* t = M_NEW BoolTarget(destination, label, description);
   m_positionalArguments.push_back(t);
}

void MCommandLineParser::DeclareInt(const char* label, const char* description, int& destination)
{
   MArgumentTarget* t = M_NEW IntTarget(destination, label, description);
   m_positionalArguments.push_back(t);
}

void MCommandLineParser::DeclareUnsignedInt(const char* label, const char* description, unsigned& destination)
{
   MArgumentTarget* t = M_NEW UnsignedTarget(destination, label, description);
   m_positionalArguments.push_back(t);
}

void MCommandLineParser::DeclareDouble(const char* label, const char* description, double& destination)
{
   MArgumentTarget* t = M_NEW DoubleTarget(destination, label, description);
   m_positionalArguments.push_back(t);
}

void MCommandLineParser::DeclareString(const char* label, const char* description, MStdString& destination)
{
   MArgumentTarget* t = M_NEW StringTarget(destination, label, description);
   m_positionalArguments.push_back(t);
}

// Optional parameters

void MCommandLineParser::DeclareOptionalInt(const char* label, const char* description, int& destination)
{
   MArgumentTarget* t = M_NEW IntTarget(destination, label, description, "", '\0', true);
   m_optionalArguments.push_back(t);
}

void MCommandLineParser::DeclareOptionalUnsignedInt(const char* label, const char* description, unsigned& destination)
{
   MArgumentTarget* t = M_NEW UnsignedTarget(destination, label, description, "", '\0', true);
   m_optionalArguments.push_back(t);
}

void MCommandLineParser::DeclareOptionalDouble(const char* label, const char* description, double& destination)
{
   MArgumentTarget* t = M_NEW DoubleTarget(destination, label, description, "", '\0', true);
   m_optionalArguments.push_back(t);
}

void MCommandLineParser::DeclareOptionalString(const char* label, const char* description, MStdString& destination)
{
   MArgumentTarget* t = M_NEW StringTarget(destination, label, description, "", '\0', true);
   m_optionalArguments.push_back(t);
}

void MCommandLineParser::DeclareStringVector(const char* label, const char* description, MStdStringVector& destination)
{
   M_ASSERT(m_argumentListValue == NULL);
   M_ASSERT(m_optionalArguments.empty());
   M_ASSERT(destination.empty()); // does not make any sense to start with nonempty vector
   m_argumentListDescription = description;
   m_argumentListLabel = label;
   m_argumentListValue = &destination;
}

void MCommandLineParser::WriteHelp()
{
   WriteHeader();
   WriteUsage();
   WriteFooter();
}

void MCommandLineParser::WriteHeader()
{
   MStdString str;
   str.reserve(256);

   str = m_executableName;
   str.append(" version ", 9);
   str += m_version;
   str.append(" compiled ", 10);
   str += m_date;
   str += '\n';
   if ( !m_description.empty() )
   {
      str += MStr::WordWrap(m_description, 0, MCommandLineParser::HelpOutputLineLength);
      str += '\n';
   }
   if ( !m_copyright.empty() )
   {
      str += MStr::WordWrap(m_copyright, 0, MCommandLineParser::HelpOutputLineLength);
      str += '\n';
   }
   m_outputStream->Write(str);
}

void MCommandLineParser::WriteUsage()
{
   MStdString str;
   str.reserve(2048);
   str.append("  ", 2);
   str += m_executableName;
   str.append(" [flags]", 8);

   ArgumentVector::const_iterator it = m_positionalArguments.begin();
   ArgumentVector::const_iterator end = m_positionalArguments.end();
   for ( ; it != end; ++it )
   {
      str += ' ';
      (*it)->AddParameter(str);
   }
   
   it = m_optionalArguments.begin();
   end = m_optionalArguments.end();
   for ( ; it != end; ++it )
   {
      str.append(" [", 2);
      (*it)->AddParameter(str);
      str += ']';
   }

   if ( m_argumentListValue != NULL )
   {
      str.append(" [<", 3);
      str += m_argumentListLabel;
      str.append("> ...]", 6);
   }
   str = "USAGE:\n" + MStr::WordWrap(str, MCommandLineParser::HelpOutputBorder, MCommandLineParser::HelpOutputLineLength);
   str += '\n';

   if ( !m_positionalArguments.empty() || !m_optionalArguments.empty() || m_argumentListValue != NULL )
   {
      str.append("ARGUMENTS:\n", 11);
      it = m_positionalArguments.begin();
      end = m_positionalArguments.end();
      for ( ; it != end; ++it )
         str += (*it)->GetUsage();

      it = m_optionalArguments.begin();
      end = m_optionalArguments.end();
      for ( ; it != end; ++it )
         str += (*it)->GetUsage();

      if ( m_argumentListValue != NULL )
      {
         MStdString usage("  <", 3);
         usage += m_argumentListLabel;
         usage += '>';
         DoAppendPaddingBeforeDescriptioon(usage);
         usage += m_argumentListDescription;
         usage = MStr::WordWrap(usage, MCommandLineParser::HelpOutputBorder, MCommandLineParser::HelpOutputLineLength);
         usage += '\n';
         str += usage;
      }
   }

   str += "FLAGS:\n"
          "  -h  --help        Shows this help text\n"
          "  --version         Shows version of this software\n";
   it = m_namedArguments.begin();
   end = m_namedArguments.end();
   for ( ; it != end; ++it )
      str += (*it)->GetUsage();
   m_outputStream->Write(str);
}

void MCommandLineParser::WriteFooter()
{
   if ( !m_footer.empty() )
   {
      if ( *m_footer.rbegin() != '\n' )
         m_outputStream->WriteLine(m_footer);
      else
         m_outputStream->Write(m_footer);
   }
}

void MCommandLineParser::WriteException(MException& ex)
{
   MStdString str;
   if ( !ex.GetFileNameAndLineNumber().IsEmpty() )
   {
      str = ex.GetFileName();
      str += '(';
      str = MToStdString(ex.GetFileLineNumber());
      str.append("): ", 3);
   }
   str += ex.AsString();
   WriteError(str);
}

void MCommandLineParser::WriteError(const char* fmt, ...)
{
   va_list va;
   va_start(va, fmt);
   MStdString msg;
   if ( m_executableName.empty() )
      msg.assign("ERROR", 5);
   else
      msg = m_executableName;
   msg.append(": ", 2);
   msg += MGetStdStringVA(fmt, va);
   if ( *msg.rbegin() != '\n' )
      msg += '\n';
   m_errorStream->Write(msg);
   va_end(va);
}

void MCommandLineParser::WriteError(const MStdString& text)
{
   WriteError("%s", text.c_str());
}

int MCommandLineParser::Process(int argc, const char** argv)
{
   if ( argc < 1 || argv == NULL || argv[argc] != NULL )
   {
      WriteError("Bad argc or/and argv");
      M_ASSERT(0); // this is almost definitely a program error, signal on debug
      return -1;
   }

   if ( m_executableName.empty() )
      m_executableName = MUtilities::GetPathFileName(argv[0]); // do not include file extension into name

   ++argv; // skip program name and process --help and --version prior to anything else
   for ( const char** it = argv; *it != NULL; ++it )
   {
      if ( strcmp(*it, "--") == 0 ) // stop scanning
         break;
      if ( strcmp(*it, "--help") == 0 || m_strcmp(*it, "-h") == 0 )
      {
         WriteHelp();
         return 1;
      }
      else if ( strcmp(*it, "--version") == 0 )
      {
         m_outputStream->WriteLine(m_version);
         return 1;
      }
   }

   bool flagsEnded = false;
   ArgumentVector::iterator itUnnamed = m_positionalArguments.begin();
   ArgumentVector::iterator itOptionalUnnamed = m_optionalArguments.begin();
   for ( const char** it = argv; *it != NULL; ++it )
   {
      MArgumentTarget* target = NULL;
      const char* arg = *it;
      const char* param = NULL;
      if ( !flagsEnded && arg[0] == '-' )
      {
         if ( arg[1] == '-' )
         {
            if ( arg[2] == '\0' ) // seen -- alone
               flagsEnded = true;
            else // otherwise this is a long --argument
            {
               MStdString name;
               const char* eqPos = m_strchr(arg, '=');
               if ( eqPos != NULL )
               {
                  name.assign(arg + 2, eqPos - arg - 2);
                  param = eqPos + 1;
               }
               else
                  name.assign(arg + 2);
               target = DoFindNamedArgument(name);
            }
         }
         else // short name
         {
            if ( arg[1] == '\0' )
            {
               WriteError("Unknown flag %s", arg);
               return -1;
            }
            target = DoFindNamedArgument(arg[1]);
            if ( arg[2] != '\0' )
               param = arg + 2;
         }

         if ( target == NULL )
         {
            WriteError("Unknown flag %s", arg);
            return -1;
         }

         if ( !target->IsParameterPresent() )
         {
            if ( param != NULL )
            {
               WriteError("%s cannot have a parameter", target->GetNameOrLabel().c_str());
               return -1;
            }
         }
         else if ( param == NULL )
         {
            ++it;
            param = *it;
            if ( param == NULL || param[0] == '\0' || param[0] == '-' ) // Parameter with such syntax cannot start with -
            {
               WriteError("%s requires a parameter", target->GetNameOrLabel().c_str());
               return -1;
            }
         }
      }
      else // arguments
      {
         param = arg;
         if ( itUnnamed != m_positionalArguments.end() )
         {
            target = *itUnnamed;
            ++itUnnamed;
         }
         else if ( itOptionalUnnamed != m_optionalArguments.end() )
         {
            target = *itOptionalUnnamed;
            ++itOptionalUnnamed;
         }
         else if ( m_argumentListValue != NULL )
         {
            m_argumentListValue->push_back(param);
            continue; // very special case
         }
         else
         {
            WriteError("Too many arguments");
            return -1;
         }
      }
      try
      {
         target->SetValue(param);
      }
      catch ( MException& ex )
      {
         M_ASSERT(target != NULL);
         WriteError("%s: %s", target->GetNameOrLabel().c_str(), ex.AsString().c_str());
         return -1;
      }
   }

   if ( itUnnamed != m_positionalArguments.end() )
   {
      WriteError("Too few arguments");
      return -1;
   }
   return 0;
}

int MCommandLineParser::Process(const char* commandLine)
{
   MStdStringVector args;
   std::vector<const char*> argv;
   if ( commandLine != NULL )
   {
      MStdString param;
      bool slashed = false;
      bool quoted = false;
      for ( const char* s = commandLine; ; ++s )
      {
         const char c = *s;
         if ( slashed )
         {
            slashed = false;
            if ( c == '"' ) // only quotes are slashed
            {
               param += c;
               continue;
            }
            param += '\\';
         }

         if ( c == '\0' )
         {
            if ( !param.empty() ) // add last if present
               args.push_back(param);
            break;
         }
         else if ( c == '\\' )
            slashed = true;
         else if ( c == '"' ) // only slashed quotes go to parameter
            quoted = !quoted;
         else if ( isspace(c) )
         {
            if ( quoted )
               param += c;
            else if ( !param.empty() )
            {
               args.push_back(param); // add quoted parameter
               param.clear();
            }
         }
         else 
            param += c;
      }
   }

   // This step has to be done after the args vector is completely constructed
   //
   MStdStringVector::const_iterator it = args.begin();
   MStdStringVector::const_iterator itEnd = args.end();
   for ( ; it != itEnd; ++it )
      argv.push_back((*it).c_str()); // this assumes the regular string implementation that returns real data with c_str
   argv.push_back(NULL);
   return Process(static_cast<int>(argv.size() - 1), &argv[0]);
}

#endif // !M_NO_CONSOLE
