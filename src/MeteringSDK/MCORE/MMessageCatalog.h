#ifndef MCORE_MMESSAGECATALOG_H
#define MCORE_MMESSAGECATALOG_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MMessageCatalog.h

#include <MCORE/MObject.h>
#include <MCORE/MMessageFile.h>
#include <MCORE/MSharedPointer.h>

///@{
/// Internationalization characters that shall be localized.
/// This is somewhat similar to standard macro gettext_noop.
///
#if !M_NO_VERBOSE_ERROR_INFORMATION
   #define M_I(s)       (MConstLocalChars(s))
#else
   #define M_I(s)       *** // M_I should not be used in devices, wrap it like M_OPT_STR(M_I()) but make sure NULL is handled
#endif
///@}

#if M_NO_MESSAGE_CATALOG

   /// Dummy internationalization string, message catalog is not present.
   ///
   #define M_I_STR(s)   (MStdString((const char*)s))

#else

   /// Internationalization string that shall be localized.
   /// This is somewhat similar to standard macro gettext, except that it returns an MStdString.
   ///
   #define M_I_STR(s)   (MMessageCatalog::GetTextDefault(M_I(s)))

/// Catalog of local strings.
///
/// The design and concepts of this class is done after standard POSIX gettext facility.
/// Please see the documentation available on the web to refer to the notions, file formats, and so on.
///
class M_CLASS MMessageCatalog : public MObject
{
public: // Types:

   /// Dummy type that makes possible calling a no-throw version of the constructor.
   ///
   enum NoThrowEnum
   {
      NoThrowConstructor = 0 // Dummy parameter
   };

   /// Vector of message sources.
   ///
   typedef std::vector<MMessageFile*>
      MessageFileVector;

   /// Translation delegate shall return true if it performed translation of a given string to local language.
   ///
   /// \see SetTranslationDelegate
   ///
   typedef bool (*TranslationDelegate)(MStdString& result, const MMessageCatalog* self, MConstChars domain, MConstLocalChars str, unsigned strSize);

public: // Constructor and destructor:

   /// Message catalog constructor that takes one domain name.
   ///
   /// This constructor can throw an exception related to domain logging.
   ///
   MMessageCatalog(MConstChars domain = NULL);

   /// No-throw version of message catalog constructor that takes one domain name.
   ///
   /// This constructor will silently suppress any exception, while there is a debug check for such.
   /// The result catalog, if loaded with errors, is not going to translate any messages.
   ///
   MMessageCatalog(NoThrowEnum, MConstChars domain) M_NO_THROW;

   /// Message catalog destructor.
   ///
   virtual ~MMessageCatalog() M_NO_THROW;

   ///@{
   /// Access the global default message catalog.
   ///
   /// Default global message catalog is the one used by default by the whole application.
   ///
   static MMessageCatalog* GetDefault() M_NO_THROW;
   static void SetDefault(MMessageCatalog* def) M_NO_THROW;
   ///@}

   /// Access the constant global default message catalog.
   ///
   /// \see GetDefault
   ///
   static const MMessageCatalog* GetDefaultConst()
   {
      return GetDefault();
   }

   ///@{
   /// Translation delegate procedure to use for translating messages.
   ///
   /// This is to set an alternative translation mechanism.
   /// This is usually not a translation but a mapping
   /// between an English string and a localized string.
   ///
   void SetTranslationDelegate(TranslationDelegate delegate = NULL)
   {
      m_translationDelegate = delegate;
   }
   TranslationDelegate GetTranslationDelegate() const
   {
      return m_translationDelegate;
   }
   ///@}

   ///@{
   /// Catalog path.
   ///
   void SetPath(const MStdString&);
   const MStdString& GetPath() const
   {
      return m_path;
   }
   ///@}

   ///@{
   /// Language of this catalog, locale code.
   ///
   /// Setting an empty string to this catalog will mean this catalog does not do any translation.
   ///
   const MStdString& GetLocale() const
   {
      return m_locale;
   }
   void SetLocale(const MStdString& locale);
   ///@}

   /// Add translation domain to the catalog.
   ///
   /// \param domainName domain
   ///
   void AddDomain(const MStdString& domainName);

   /// Clear all messages from the catalog.
   ///
   void Clear() M_NO_THROW;

   /// Enumerate all created catalogs and set the given locale string to each.
   ///
   static void SetLocaleToAllCatalogs(const MStdString& locale);

   /// Translate the given English text into current language.
   ///
   /// \param str Text, typically in English.
   ///
   /// \return Translated text in a locale specific language.
   ///
   MStdString GetText(MConstLocalChars str) const M_NO_THROW;

   /// Translate the given English text into current language.
   ///
   /// \param str Text, typically in English.
   /// \param strSize Length of the text.
   ///
   /// \return Translated text in a locale specific language.
   ///
   MStdString GetText(MConstLocalChars str, unsigned strSize) const M_NO_THROW;

   /// Translate the given English text into current language given the translation domain.
   ///
   /// \param domain Translation domain, a string denoting the sub-dictionary.
   /// \param str Text, typically in English.
   ///
   /// \return Translated text in a locale specific language.
   ///
   MStdString GetText(MConstChars domain, MConstLocalChars str) const M_NO_THROW;

   /// Translate the given English text into current language given the translation domain.
   ///
   /// \param domain Translation domain, a string denoting the sub-dictionary.
   /// \param str Text, typically in English.
   /// \param strSize Length of the text.
   ///
   /// \return Translated text in a locale specific language.
   ///
   MStdString GetText(MConstChars domain, MConstLocalChars str, unsigned strSize) const M_NO_THROW;

   /// Translate the given English text with parameters into current language.
   ///
   /// \param str Text with parameters, typically in English.
   /// \param ... Optional parameters that should correspond to printf specifiers in str.
   ///
   /// \return Translated text in a locale specific language.
   ///
   MStdString GetFormattedText(MConstLocalChars str, ...) const M_NO_THROW;

   /// Translate the given English text with parameters into current language given the translation domain.
   ///
   /// \param domain Translation domain, a string denoting the sub-dictionary.
   /// \param str Text with parameters, typically in English.
   /// \param ... Optional parameters that should correspond to printf specifiers in str.
   ///
   /// \return Translated text in a locale specific language.
   ///
   MStdString GetFormattedText(MConstChars domain, MConstLocalChars str, ...) const M_NO_THROW;

   /// Translate the given English text with parameters into current language.
   ///
   /// \param str Text with parameters, typically in English.
   /// \param va Parameter list that should correspond to printf specifiers in str.
   ///
   /// \return Translated text in a locale specific language.
   ///
   MStdString GetVaText(MConstLocalChars str, va_list va) const M_NO_THROW;

   /// Translate the given English text with parameters into current language given the translation domain.
   ///
   /// \param domain Translation domain, a string denoting the sub-dictionary.
   /// \param str Text with parameters, typically in English.
   /// \param va Parameter list that should correspond to printf specifiers in str.
   ///
   /// \return Translated text in a locale specific language.
   ///
   MStdString GetVaText(MConstChars domain, MConstLocalChars str, va_list va) const M_NO_THROW;

   /// Translate the given English text into current language.
   ///
   /// \param str Text, typically in English.
   ///
   /// \return Translated text in a locale specific language.
   ///
   MStdString GetStdString(const MStdString& str) const M_NO_THROW;

   /// Translate the given English text into current language given the translation domain.
   ///
   /// \param domain Translation domain, a string denoting the sub-dictionary.
   /// \param str Text, typically in English.
   ///
   /// \return Translated text in a locale specific language.
   ///
   MStdString GetStdStringDomain(const MStdString& domain, const MStdString& str) const M_NO_THROW;

   /// Static variant that calls GetText of the default catalog.
   ///
   /// \see GetText(MConstLocalChars)const
   /// \seeprop{GetDefault,Default}
   ///
   static MStdString GetTextDefault(MConstLocalChars str) M_NO_THROW;

   /// Static variant that calls GetText of the default catalog.
   ///
   /// \see GetText(MConstLocalChars,unsigned)const
   /// \seeprop{GetDefault,Default}
   ///
   static MStdString GetTextDefault(MConstLocalChars str, unsigned strSize) M_NO_THROW;

   /// Static variant that calls GetText of the default catalog.
   ///
   /// \see GetText(MConstChars,MConstLocalChars)const
   /// \seeprop{GetDefault,Default}
   ///
   static MStdString GetTextDefault(MConstChars domain, MConstLocalChars str) M_NO_THROW;

   /// Static variant that calls GetText of the default catalog.
   ///
   /// \see GetText(MConstChars,MConstLocalChars,unsigned)const
   /// \seeprop{GetDefault,Default}
   ///
   static MStdString GetTextDefault(MConstChars domain, MConstLocalChars str, unsigned strSize) M_NO_THROW;

   /// Static variant that calls GetFormattedText of the default catalog.
   ///
   /// \see GetFormattedText(MConstLocalChars,...)
   /// \seeprop{GetDefault,Default}
   ///
   static MStdString GetFormattedTextDefault(MConstLocalChars str, ...) M_NO_THROW;

   /// Static variant that calls GetFormattedText of the default catalog.
   ///
   /// \see GetFormattedText(MConstChars,MConstLocalChars,...)
   /// \seeprop{GetDefault,Default}
   ///
   static MStdString GetFormattedTextDefault(MConstChars domain, MConstLocalChars str, ...) M_NO_THROW;

   /// Static variant that calls GetVaText of the default catalog.
   ///
   /// \see GetVaText(MConstLocalChars,va_list)const
   /// \seeprop{GetDefault,Default}
   ///
   static MStdString GetVaTextDefault(MConstLocalChars str, va_list va) M_NO_THROW;

   /// Static variant that calls GetVaText of the default catalog.
   ///
   /// \see GetVaText(MConstChars,MConstLocalChars,va_list)const
   /// \seeprop{GetDefault,Default}
   ///
   static MStdString GetVaTextDefault(MConstChars domain, MConstLocalChars str, va_list va) M_NO_THROW;

   /// Static variant that calls GetStdString of the default catalog.
   ///
   /// \see GetStdString
   /// \seeprop{GetDefault,Default}
   ///
   static MStdString GetStdStringDefault(const MStdString& str) M_NO_THROW;

   /// Static variant that calls GetStdString of the default catalog.
   ///
   /// \see GetStdStringDomain
   /// \seeprop{GetDefault,Default}
   ///
   static MStdString GetStdStringDomainDefault(const MStdString& domain, const MStdString& str) M_NO_THROW;

   /// Get a human readable description of the given locale.
   ///
   /// \param lang Locale name, which is the language code.
   ///
   /// \return Human readable description of the locale.
   ///
   static MStdString StaticGetLocaleDescription(const MStdString& lang) M_NO_THROW;

#if (M_OS & M_OS_WINDOWS) != 0

   /// Get language ID of the given language.
   ///
   /// This is a Windows specific method.
   ///
   /// \param lang Locale name, which is the language code.
   ///
   /// \return Windows language ID code.
   ///
   static LANGID StaticGetLangID(const MStdString& lang) M_NO_THROW;

#endif

private:

   void DoLoadOneFileIfInDomain(MessageFileVector* files, MConstChars name, bool isNotPosix);
   void DoLoadOneCatalogSubDirectory(MessageFileVector* files, const MStdString& locale, MStdString dir);
   void DoLoadOneCatalogDirectory(MessageFileVector* files, const MStdString& dir);
   void DoReloadCatalog();
   void DoClearPrevSources() M_NO_THROW;

private: // Fields:

   // Catalog path
   //
   MStdString m_path;

   // Text domains for this catalog
   //
   MStdStringVector m_domains;

   // Currently loaded message files.
   // Pointer used to facilitate no-lock locale change from many threads.
   //
   MessageFileVector* m_sources;

   // Previously loaded message files.
   // Pointer used to facilitate no-lock locale change from many threads.
   //
   MessageFileVector* m_prevSources;

#if (M_OS & M_OS_WINDOWS) != 0

   // Windows Language ID that corresponds to m_locale
   //
   LANGID m_langId;

   // Windows code page that corresponds to m_locale
   //
   UINT m_codePage;

#endif

   // Language for this catalog
   //
   MStdString m_locale;

   // If this function is defined, it will do translation instead of default message catalog implementation.
   //
   TranslationDelegate m_translationDelegate;

   M_SHARED_POINTER_CLASS(MessageCatalog)
   M_DECLARE_CLASS(MessageCatalog)
};

#endif // !M_NO_MESSAGE_CATALOG
///@}
#endif
