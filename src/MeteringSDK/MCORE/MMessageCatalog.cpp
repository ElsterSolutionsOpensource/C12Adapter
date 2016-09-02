// File MCORE/MMessageCatalog.cpp

#include "MCOREExtern.h"
#include "MMessageCatalog.h"
#include "MMessageFile.h"
#include "MCriticalSection.h"
#include "MUtilities.h"
#include "MFindFile.h"
#include "MAlgorithm.h"
#include "MException.h"

#if !M_NO_MESSAGE_CATALOG

M_START_PROPERTIES(MessageCatalog)
   M_OBJECT_PROPERTY_STRING           (MessageCatalog, Path,    ST_constMStdStringA_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_STRING           (MessageCatalog, Locale,  ST_constMStdStringA_X, ST_X_constMStdStringA)
   M_CLASS_PROPERTY_READONLY_OBJECT   (MessageCatalog, Default)
M_START_METHODS(MessageCatalog)
   M_OBJECT_SERVICE_OVERLOADED        (MessageCatalog, GetText, GetStdStringDomain, 2, ST_MStdString_X_constMStdStringA_constMByteStringA)
   M_OBJECT_SERVICE_OVERLOADED        (MessageCatalog, GetText, GetStdString,       1, ST_MStdString_X_constMByteStringA)
   M_CLASS_SERVICE                    (MessageCatalog, SetLocaleToAllCatalogs,         ST_S_constMStdStringA)
M_END_CLASS(MessageCatalog, Object)

   typedef std::vector<MMessageCatalog*>
      MMessageCatalogVector;

   class MMessageCatalogProtectedCollection : private MMessageCatalogVector
   {
   public:

      // Constructor
      //
      // We know that Meyers singletons are always initialized within its own critical section
      //
      MMessageCatalogProtectedCollection() M_NO_THROW
      :
         m_criticalSection(),
         m_initialLocale()
      {
         M_ASSERT(!s_initialized);
         s_initialized = true;
      }
      
      ~MMessageCatalogProtectedCollection() M_NO_THROW
      {
         M_ASSERT(s_initialized);
         MCriticalSection::Locker locker(m_criticalSection);
         s_initialized = false; // notify the collection is done
         clear(); // destroy under lock
      }

      static MMessageCatalogProtectedCollection* GetSingleton() M_NO_THROW;

      void Add(MMessageCatalog* catalog) M_NO_THROW
      {
         M_ASSERT(s_initialized);
         MCriticalSection::Locker locker(m_criticalSection);

         if ( !m_initialLocale.empty() )
         {
            try
            {
               catalog->SetLocale(m_initialLocale);
            }
            catch ( ... )
            {
               M_ASSERT(0); // ignore this type of errors in release
            }
         }

         M_ASSERT(std::find(begin(), end(), catalog) == end());
         push_back(catalog);
      }

      void Remove(MMessageCatalog* catalog)
      {
         if ( s_initialized ) // due to possible static variables, it is normal when remove is called after destruction of this singleton
         {
            MCriticalSection::Locker locker(m_criticalSection);

            // Implement reverse find, it will work faster
            iterator it = std::find(begin(), end(), catalog);
            M_ASSERT(it != end());
            this->erase(it);
         }
      }

      void SetLocaleToAllCatalogs(const MStdString& locale)
      {
         MCriticalSection::Locker locker(m_criticalSection);
         for ( iterator it = begin(); it != end(); ++it )
            (*it)->SetLocale(locale);
         m_initialLocale = locale;
      }

   public: // Fields:

      // Singleton is created prior to any use, but we also have to handle the case
      // when it is destroyed prior to some static variable
      //
      static bool s_initialized;

      // Protective critical section for the object, should be the first field
      //
      MCriticalSection m_criticalSection;

      // Initial locale code, the one with which all new message catalogs objects are created
      //
      MStdString m_initialLocale;
   };

   bool MMessageCatalogProtectedCollection::s_initialized = false;

   MMessageCatalogProtectedCollection* MMessageCatalogProtectedCollection::GetSingleton() M_NO_THROW
   {
      static MMessageCatalogProtectedCollection s_messageCatalogProtectedCollection; // Meyers' singleton
      return &s_messageCatalogProtectedCollection;
   }

   static MMessageCatalog** GetDefaultCatalogLocation() M_NO_THROW
   {
      static MMessageCatalog s_initialCatalog(MMessageCatalog::NoThrowConstructor, M_GLOBAL_MESSAGE_CATALOG_DOMAIN);
      static MMessageCatalog* s_default = &s_initialCatalog;
      return &s_default;
   }

MMessageCatalog::MMessageCatalog(MConstChars domain)
:
   m_path(),
   m_domains(),
   m_sources(NULL),
   m_prevSources(NULL),
#if (M_OS & M_OS_WINDOWS) != 0
   m_langId(0),
   m_codePage(0),
#endif
   m_locale(),
   m_translationDelegate(NULL),
   M_SHARED_POINTER_CLASS_INIT
{
   if ( domain != NULL )
      AddDomain(domain);

   MMessageCatalogProtectedCollection::GetSingleton()->Add(this);
}

MMessageCatalog::MMessageCatalog(NoThrowEnum, MConstChars domain) M_NO_THROW
:
   m_path(),
   m_domains(),
   m_sources(NULL),
   m_prevSources(NULL),
#if (M_OS & M_OS_WINDOWS) != 0
   m_langId(0),
   m_codePage(0),
#endif
   m_locale(),
   m_translationDelegate(NULL),
   M_SHARED_POINTER_CLASS_INIT
{
   M_ASSERT(domain != NULL);
   try
   {
      AddDomain(domain);
   }
   catch ( ... )
   {
      M_ASSERT(0); // debug time only
   }
   
   MMessageCatalogProtectedCollection::GetSingleton()->Add(this);
}

MMessageCatalog::~MMessageCatalog() M_NO_THROW
{
   MMessageCatalogProtectedCollection::GetSingleton()->Remove(this); // first remove from list
   Clear();                                 // then clear current into previous
   DoClearPrevSources();                    // then finally clear the previous
}

MMessageCatalog* MMessageCatalog::GetDefault() M_NO_THROW
{
   return *GetDefaultCatalogLocation();
}

void MMessageCatalog::SetDefault(MMessageCatalog* def) M_NO_THROW
{
   *GetDefaultCatalogLocation() = def;
}

void MMessageCatalog::DoClearPrevSources() M_NO_THROW
{
   MessageFileVector* tmpSources = m_prevSources; // use temporary to prevent having a pointer to a partially destroyed object
   if ( tmpSources != NULL )
   {
      m_prevSources = NULL;
      MessageFileVector::iterator it = tmpSources->begin();
      MessageFileVector::iterator itEnd = tmpSources->end();
      for ( ; it != itEnd; ++it )
         delete *it;
      delete tmpSources;
   }
}

void MMessageCatalog::Clear() M_NO_THROW
{
   MessageFileVector* tmpSources = m_sources; // use temporary to prevent having a pointer to a partially destroyed object
   if ( tmpSources != NULL )
   {
      m_sources = NULL;
      DoClearPrevSources();
      m_prevSources = tmpSources;
   }
}

void MMessageCatalog::AddDomain(const MStdString& domainName)
{
   MStdStringVector::iterator it = std::find(m_domains.begin(), m_domains.end(), domainName);
   if ( it == m_domains.end() )
   {
      m_domains.insert(m_domains.begin(), domainName);
      DoReloadCatalog();
   }
}

void MMessageCatalog::DoLoadOneFileIfInDomain(MessageFileVector* files, MConstChars name, bool isNotPosix)
{
   MStdString fullFileName = name;
   MStdString domain = MUtilities::GetPathFileName(fullFileName); // for standard POSIX catalog, domain is the file name
   if ( isNotPosix )
      domain = MUtilities::GetPathFileName(domain); // for nonstandard catalog remove secondary extension
   if ( std::find(m_domains.begin(), m_domains.end(), domain) != m_domains.end() )
      files->push_back(M_NEW MMessageFile(fullFileName, domain));
}

void MMessageCatalog::DoLoadOneCatalogSubDirectory(MessageFileVector* files, const MStdString& locale, MStdString dir)
{
   M_ASSERT(!locale.empty() && !m_domains.empty()); // there shall be locale defined, and there is a domain to search

   MAddDirectorySeparatorIfNecessary(dir);

   // At first, try a POSIX standard locale directory
   //
   MConstChars name;
   MStdString posixLocale = dir;
   posixLocale.append("locale/LC_MESSAGES/", 19);
   posixLocale += locale;
   MAddDirectorySeparatorIfNecessary(posixLocale);
   if ( MUtilities::IsPathDirectory(posixLocale) ) // ...and exists
   {
      MFindFile dirSearch(posixLocale.c_str(), "*.mo", false);
      while ( (name = dirSearch.FindNext()) != NULL )
         DoLoadOneFileIfInDomain(files, name, false);
   }

   // Next, try custom way
   //
   MStdString fileMask("*.", 2); // format is "*.<lang>.mo", for example, ".ru_RU.mo"
   fileMask += locale;
   fileMask.append(".mo", 3);
   MFindFile dirSearch(dir.c_str(), fileMask.c_str(), false);
   while ( (name = dirSearch.FindNext()) != NULL )
      DoLoadOneFileIfInDomain(files, name, true);
}

void MMessageCatalog::DoLoadOneCatalogDirectory(MessageFileVector* files, const MStdString& dir)
{
   MConstChars name;
   DoLoadOneCatalogSubDirectory(files, m_locale, dir); // scan the root catalog directory
   MFindFile dirSearch(dir.c_str(), "*", true);
   while ( (name = dirSearch.FindNext()) != NULL )
      DoLoadOneCatalogSubDirectory(files, m_locale, name);

   MStdString::size_type pos = m_locale.find('_');
   if ( pos == MStdString::npos )
      pos = m_locale.find('-'); // windows-style
   if ( pos != MStdString::npos )
   {
      MStdString lang(m_locale.c_str(), pos);
      DoLoadOneCatalogSubDirectory(files, lang, dir); // scan the root catalog directory
      dirSearch.Init(dir.c_str(), "*", true);
      while ( (name = dirSearch.FindNext()) != NULL )
         DoLoadOneCatalogSubDirectory(files, lang, name);
   }
}

void MMessageCatalog::DoReloadCatalog()
{
   if ( m_locale.empty() || m_domains.empty() )
      Clear();
   else
   {
      MUniquePtr<MessageFileVector> files(M_NEW MessageFileVector);
      if ( m_path.empty() )
         DoLoadOneCatalogDirectory(files.get(), MUtilities::GetModulePath());
      else
      {
         MStdStringVector directories = MAlgorithm::SplitWithDelimiter(m_path, ';', true);
         MStdStringVector::const_iterator it = directories.begin();
         MStdStringVector::const_iterator itEnd = directories.end();
         for ( ; it != itEnd; ++it )
            DoLoadOneCatalogDirectory(files.get(), *it);
      }

      MessageFileVector* tmpSources = m_sources; // use temporary to prevent having a pointer to a partially destroyed object
      if ( tmpSources != NULL )
      {
         m_sources = files.release();
         DoClearPrevSources();
         m_prevSources = tmpSources;
      }
      else
         m_sources = files.release();
   }
}

void MMessageCatalog::SetPath(const MStdString& path)
{
   m_path = path;
   DoReloadCatalog();
}

void MMessageCatalog::SetLocale(const MStdString& locale)
{
   #if (M_OS & M_OS_WINDOWS) != 0
      #if M_UNICODE
         M_ENSURED_ASSERT(M_WINDOWS_CP_ACP == CP_UTF8);
         m_codePage = CP_UTF8;
      #else
         if ( locale.empty() )
         {
            m_codePage = CP_ACP;
            m_langId = 0;
         }
         else
         {
            m_codePage = CP_ACP;
            m_langId = StaticGetLangID(locale);
            if ( m_langId != 0 )
            {
               #if M_UNICODE
                  wchar_t buffer [ 16 ];
               #else
                  char buffer [ 16 ];
               #endif
               LCID lcid = MAKELCID(m_langId, SORT_DEFAULT);
               if ( GetLocaleInfo(lcid, LOCALE_IDEFAULTANSICODEPAGE, buffer, M_NUMBER_OF_ARRAY_ELEMENTS(buffer)) > 0 )
               {
                   m_codePage = (UINT)MToLong(buffer);
               }
            }
         }
      #endif
   #endif

   m_locale = locale;
   DoReloadCatalog();
}

void MMessageCatalog::SetLocaleToAllCatalogs(const MStdString& locale)
{
   MMessageCatalogProtectedCollection::GetSingleton()->SetLocaleToAllCatalogs(locale);
}

MStdString MMessageCatalog::GetText(MConstLocalChars str, unsigned strSize) const M_NO_THROW
{
   return GetText(NULL, str, strSize);
}

MStdString MMessageCatalog::GetText(MConstLocalChars str) const M_NO_THROW
{
   return GetText(NULL, str, static_cast<unsigned>(strlen((const char*)str)));
}

MStdString MMessageCatalog::GetText(MConstChars domain, MConstLocalChars str, unsigned strSize) const M_NO_THROW
{
   MStdString result;
   M_ASSERT(str != NULL);
   try
   {
      if ( m_translationDelegate != NULL )
      {
         (*m_translationDelegate)(result, this, domain, str, strSize);
      }
      else
      {
         unsigned transSize = strSize;    // strSize can change below
         const char* trans = NULL;
         if ( m_sources != NULL )
         {
            MessageFileVector* tmpSources = m_sources; // use temporary to prevent having inconsistent pointer
            MessageFileVector::iterator it = tmpSources->begin();
            MessageFileVector::iterator itEnd = tmpSources->end();
            for ( ; it != itEnd; ++it )
            {
               const MMessageFile* source = (*it);
               if ( domain == NULL || m_strcmp(source->GetDomain().c_str(), domain) == 0 )
               {
                  M_ASSERT(transSize == strSize);            // Make sure before the below call the two are equal
                  trans = source->Translate(str, transSize); // This can change transSize in case of successful translation
                  if ( trans != NULL )
                  {
                     M_ENSURED_ASSERT(result.empty());
                     if ( transSize != 0 )
                     {
                        #if !M_UNICODE && (M_OS & M_OS_WINDOWS) != 0
                           // Windows does it through buffer conversion. UTF-8 to UNICODE, then UNICODE to local
                           //
                           int resultWideSize = MultiByteToWideChar(CP_UTF8, 0, trans, transSize, NULL, 0);
                           if ( resultWideSize <= 0 )
                              break; // return English

                           std::wstring wideStr; // use wstring instead of MWideString to avoid dependency on M_NO_WCHAR_T
                           wideStr.resize(resultWideSize);
                           MultiByteToWideChar(CP_UTF8, 0, trans, transSize, &(wideStr[0]), resultWideSize);
                           int resultSize = WideCharToMultiByte(m_codePage, 0, wideStr.c_str(), resultWideSize, NULL, 0, 0, 0);
                           if ( resultSize <= 0 )
                              break; // return English

                           result.resize(resultSize);
                           WideCharToMultiByte(m_codePage, 0, wideStr.c_str(), resultWideSize, &(result[0]), resultSize, 0, 0);
                           //?? if ( m_langId != 0 )
                           //??   ::SetThreadLocale(MAKELCID(m_langId, SORT_DEFAULT));
                        #else
                           M_ENSURED_ASSERT(M_UNICODE);
                           result.assign(trans, transSize); // Unix representation is UTF-8
                        #endif
                     }
                     break;
                  }
               }
            }
         }
      }
   }
   catch ( ... )
   {
      M_ASSERT(0); // warn on debug only
   }

   // Here we did not find translation, or could not convert it to local characters. Return original string
   //
   if ( result.empty() )
      result.assign((const char*)str, strSize);
   return result;
}

MStdString MMessageCatalog::GetText(MConstChars domain, MConstLocalChars str) const M_NO_THROW
{
   return GetText(domain, str, static_cast<unsigned>(strlen((const char*)str)));
}

MStdString MMessageCatalog::GetFormattedText(MConstLocalChars str, ...) const M_NO_THROW
{
   va_list va;
   va_start(va, str);
   const MStdString& result = GetVaText(NULL, str, va);
   va_end(va);
   return result;
}

MStdString MMessageCatalog::GetFormattedText(MConstChars domain, MConstLocalChars str, ...) const M_NO_THROW
{
   va_list va;
   va_start(va, str);
   const MStdString& result = GetVaText(domain, str, va);
   va_end(va);
   return result;
}

MStdString MMessageCatalog::GetVaText(MConstLocalChars str, va_list va) const M_NO_THROW
{
   return GetVaText(NULL, str, va);
}

MStdString MMessageCatalog::GetVaText(MConstChars domain, MConstLocalChars str, va_list va) const M_NO_THROW
{
   return MGetStdStringVA(GetText(domain, str).c_str(), va);
}

MStdString MMessageCatalog::GetStdString(const MStdString& str) const M_NO_THROW
{
   return GetText((MConstLocalChars)str.c_str(), static_cast<unsigned>(str.size()));
}

MStdString MMessageCatalog::GetStdStringDomain(const MStdString& domain, const MStdString& str) const M_NO_THROW
{
   return GetText(domain.c_str(), (MConstLocalChars)str.c_str(), static_cast<unsigned>(str.size()));
}

MStdString MMessageCatalog::GetTextDefault(MConstLocalChars str) M_NO_THROW
{
   return GetDefault()->GetText(str);
}

MStdString MMessageCatalog::GetTextDefault(MConstLocalChars str, unsigned strSize) M_NO_THROW
{
   return GetDefault()->GetText(str, strSize);
}

MStdString MMessageCatalog::GetTextDefault(MConstChars domain, MConstLocalChars str, unsigned strSize) M_NO_THROW
{
   return GetDefault()->GetText(domain, str, strSize);
}

MStdString MMessageCatalog::GetFormattedTextDefault(MConstLocalChars str, ...) M_NO_THROW
{
   va_list va;
   va_start(va, str);
   const MStdString& result = GetDefault()->GetFormattedText(NULL, str, va);
   va_end(va);
   return result;
}

MStdString MMessageCatalog::GetFormattedTextDefault(MConstChars domain, MConstLocalChars str, ...) M_NO_THROW
{
   va_list va;
   va_start(va, str);
   const MStdString& result = GetDefault()->GetFormattedText(domain, str, va);
   va_end(va);
   return result;
}

MStdString MMessageCatalog::GetVaTextDefault(MConstLocalChars str, va_list va) M_NO_THROW
{
   return GetDefault()->GetVaText(NULL, str, va);
}

MStdString MMessageCatalog::GetVaTextDefault(MConstChars domain, MConstLocalChars str, va_list va) M_NO_THROW
{
   return GetDefault()->GetVaText(domain, str, va);
}

MStdString MMessageCatalog::GetStdStringDefault(const MStdString& str) M_NO_THROW
{
   return GetDefault()->GetStdString(str);
}

MStdString MMessageCatalog::GetStdStringDomainDefault(const MStdString& domain, const MStdString& str) M_NO_THROW
{
   return GetDefault()->GetStdStringDomain(domain, str);
}

   #if (M_OS & M_OS_WINDOWS) != 0
   
      struct MLangDef
      {
         MChar       m_lang[8];
         LANGID      m_langId;
         MConstChars m_localeName;
      };

      #define M__LANG(lang, winLangId, winSublangId, name)     {lang, MAKELANGID(winLangId, winSublangId), name}

   #else

      struct MLangDef
      {
         MChar       m_lang[8];
         MConstChars m_localeName;
      };

      #define M__LANG(lang, winLangId, winSublangId, name)     {lang, name}

   #endif
   
   #if (!defined LANG_MONGOLIAN) && (M_OS & M_OS_WIN32_CE)
      // definition copied from WinNT.h - not available on Pocket PC
      #define LANG_MONGOLIAN                   0x50
   #endif

   static const MLangDef s_lang[] = 
   {
      M__LANG("af"   , LANG_AFRIKAANS , SUBLANG_DEFAULT                   , "Afrikaans"),
      M__LANG("sq"   , LANG_ALBANIAN  , SUBLANG_DEFAULT                   , "Albanian"),
      M__LANG("ar"   , LANG_ARABIC    , SUBLANG_DEFAULT                   , "Arabic"),
      M__LANG("ar_DZ", LANG_ARABIC    , SUBLANG_ARABIC_ALGERIA            , "Arabic (Algeria)"),
      M__LANG("ar_BH", LANG_ARABIC    , SUBLANG_ARABIC_BAHRAIN            , "Arabic (Bahrain)"),
      M__LANG("ar_EG", LANG_ARABIC    , SUBLANG_ARABIC_EGYPT              , "Arabic (Egypt)"),
      M__LANG("ar_IQ", LANG_ARABIC    , SUBLANG_ARABIC_IRAQ               , "Arabic (Iraq)"),
      M__LANG("ar_JO", LANG_ARABIC    , SUBLANG_ARABIC_JORDAN             , "Arabic (Jordan)"),
      M__LANG("ar_KW", LANG_ARABIC    , SUBLANG_ARABIC_KUWAIT             , "Arabic (Kuwait)"),
      M__LANG("ar_LB", LANG_ARABIC    , SUBLANG_ARABIC_LEBANON            , "Arabic (Lebanon)"),
      M__LANG("ar_LY", LANG_ARABIC    , SUBLANG_ARABIC_LIBYA              , "Arabic (Libya)"),
      M__LANG("ar_MA", LANG_ARABIC    , SUBLANG_ARABIC_MOROCCO            , "Arabic (Morocco)"),
      M__LANG("ar_OM", LANG_ARABIC    , SUBLANG_ARABIC_OMAN               , "Arabic (Oman)"),
      M__LANG("ar_QA", LANG_ARABIC    , SUBLANG_ARABIC_QATAR              , "Arabic (Qatar)"),
      M__LANG("ar_SA", LANG_ARABIC    , SUBLANG_ARABIC_SAUDI_ARABIA       , "Arabic (Saudi Arabia)"),
      M__LANG("ar_SY", LANG_ARABIC    , SUBLANG_ARABIC_SYRIA              , "Arabic (Syria)"),
      M__LANG("ar_TN", LANG_ARABIC    , SUBLANG_ARABIC_TUNISIA            , "Arabic (Tunisia)"),
      M__LANG("ar_AE", LANG_ARABIC    , SUBLANG_ARABIC_UAE                , "Arabic (Uae)"),
      M__LANG("ar_YE", LANG_ARABIC    , SUBLANG_ARABIC_YEMEN              , "Arabic (Yemen)"),
      M__LANG("hy"   , LANG_ARMENIAN  , SUBLANG_DEFAULT                   , "Armenian"),
      M__LANG("as"   , LANG_ASSAMESE  , SUBLANG_DEFAULT                   , "Assamese"),
      M__LANG("az"   , LANG_AZERI     , SUBLANG_DEFAULT                   , "Azeri"),
      M__LANG("eu"   , LANG_BASQUE    , SUBLANG_DEFAULT                   , "Basque"),
      M__LANG("be"   , LANG_BELARUSIAN, SUBLANG_DEFAULT                   , "Belarusian"),
      M__LANG("bn"   , LANG_BENGALI   , SUBLANG_DEFAULT                   , "Bengali"),
      M__LANG("bg"   , LANG_BULGARIAN , SUBLANG_DEFAULT                   , "Bulgarian"),
      M__LANG("ca"   , LANG_CATALAN   , SUBLANG_DEFAULT                   , "Catalan"),
      M__LANG("zh"   , LANG_CHINESE   , SUBLANG_DEFAULT                   , "Chinese"),
      M__LANG("zh_CN", LANG_CHINESE   , SUBLANG_CHINESE_SIMPLIFIED        , "Chinese (Simplified)"),
      M__LANG("zh_TW", LANG_CHINESE   , SUBLANG_CHINESE_TRADITIONAL       , "Chinese (Traditional)"),
      M__LANG("zh_HK", LANG_CHINESE   , SUBLANG_CHINESE_HONGKONG          , "Chinese (Hongkong)"),
      M__LANG("zh_MO", LANG_CHINESE   , SUBLANG_CHINESE_MACAU             , "Chinese (Macau)"),
      M__LANG("zh_SG", LANG_CHINESE   , SUBLANG_CHINESE_SINGAPORE         , "Chinese (Singapore)"),
      M__LANG("zh_TW", LANG_CHINESE   , SUBLANG_CHINESE_TRADITIONAL       , "Chinese (Taiwan)"),
      M__LANG("hr"   , LANG_CROATIAN  , SUBLANG_DEFAULT                   , "Croatian"),
      M__LANG("cs"   , LANG_CZECH     , SUBLANG_DEFAULT                   , "Czech"),
      M__LANG("da"   , LANG_DANISH    , SUBLANG_DEFAULT                   , "Danish"),
      M__LANG("nl"   , LANG_DUTCH     , SUBLANG_DUTCH                     , "Dutch"),
      M__LANG("nl_BE", LANG_DUTCH     , SUBLANG_DUTCH_BELGIAN             , "Dutch (Belgian)"),
      M__LANG("en"   , LANG_ENGLISH   , SUBLANG_ENGLISH_US                , "English"),
      M__LANG("en_GB", LANG_ENGLISH   , SUBLANG_ENGLISH_UK                , "English (U.K.)"),
      M__LANG("en_US", LANG_ENGLISH   , SUBLANG_ENGLISH_US                , "English (U.S.)"),
      M__LANG("en_AU", LANG_ENGLISH   , SUBLANG_ENGLISH_AUS               , "English (Australia)"),
      M__LANG("en_BZ", LANG_ENGLISH   , SUBLANG_ENGLISH_BELIZE            , "English (Belize)"),
      M__LANG("en_CA", LANG_ENGLISH   , SUBLANG_ENGLISH_CAN               , "English (Canada)"),
      M__LANG("en_CB", LANG_ENGLISH   , SUBLANG_ENGLISH_CARIBBEAN         , "English (Caribbean)"),
      M__LANG("en_IE", LANG_ENGLISH   , SUBLANG_ENGLISH_EIRE              , "English (Eire)"),
      M__LANG("en_JM", LANG_ENGLISH   , SUBLANG_ENGLISH_JAMAICA           , "English (Jamaica)"),
      M__LANG("en_NZ", LANG_ENGLISH   , SUBLANG_ENGLISH_NZ                , "English (New Zealand)"),
      M__LANG("en_PH", LANG_ENGLISH   , SUBLANG_ENGLISH_PHILIPPINES       , "English (Philippines)"),
      M__LANG("en_ZA", LANG_ENGLISH   , SUBLANG_ENGLISH_SOUTH_AFRICA      , "English (South Africa)"),
      M__LANG("en_TT", LANG_ENGLISH   , SUBLANG_ENGLISH_TRINIDAD          , "English (Trinidad)"),
      M__LANG("en_ZW", LANG_ENGLISH   , SUBLANG_ENGLISH_ZIMBABWE          , "English (Zimbabwe)"),
      M__LANG("et"   , LANG_ESTONIAN  , SUBLANG_DEFAULT                   , "Estonian"),
      M__LANG("fo"   , LANG_FAEROESE  , SUBLANG_DEFAULT                   , "Faeroese"),
      M__LANG("fa"   , LANG_FARSI     , SUBLANG_DEFAULT                   , "Farsi"),
      M__LANG("fi"   , LANG_FINNISH   , SUBLANG_DEFAULT                   , "Finnish"),
      M__LANG("fr"   , LANG_FRENCH    , SUBLANG_FRENCH                    , "French"),
      M__LANG("fr_BE", LANG_FRENCH    , SUBLANG_FRENCH_BELGIAN            , "French (Belgian)"),
      M__LANG("fr_CA", LANG_FRENCH    , SUBLANG_FRENCH_CANADIAN           , "French (Canadian)"),
      M__LANG("fr_LU", LANG_FRENCH    , SUBLANG_FRENCH_LUXEMBOURG         , "French (Luxembourg)"),
      M__LANG("fr_MC", LANG_FRENCH    , SUBLANG_FRENCH_MONACO             , "French (Monaco)"),
      M__LANG("fr_CH", LANG_FRENCH    , SUBLANG_FRENCH_SWISS              , "French (Swiss)"),
      M__LANG("ka"   , LANG_GEORGIAN  , SUBLANG_DEFAULT                   , "Georgian"),
      M__LANG("de"   , LANG_GERMAN    , SUBLANG_GERMAN                    , "German"),
      M__LANG("de_AT", LANG_GERMAN    , SUBLANG_GERMAN_AUSTRIAN           , "German (Austrian)"),
      M__LANG("de_LI", LANG_GERMAN    , SUBLANG_GERMAN_LIECHTENSTEIN      , "German (Liechtenstein)"),
      M__LANG("de_LU", LANG_GERMAN    , SUBLANG_GERMAN_LUXEMBOURG         , "German (Luxembourg)"),
      M__LANG("de_CH", LANG_GERMAN    , SUBLANG_GERMAN_SWISS              , "German (Swiss)"),
      M__LANG("el_GR", LANG_GREEK     , SUBLANG_DEFAULT                   , "Greek"),
      M__LANG("gu"   , LANG_GUJARATI  , SUBLANG_DEFAULT                   , "Gujarati"),
      M__LANG("he"   , LANG_HEBREW    , SUBLANG_DEFAULT                   , "Hebrew"),
      M__LANG("hi"   , LANG_HINDI     , SUBLANG_DEFAULT                   , "Hindi"),
      M__LANG("hu"   , LANG_HUNGARIAN , SUBLANG_DEFAULT                   , "Hungarian"),
      M__LANG("is"   , LANG_ICELANDIC , SUBLANG_DEFAULT                   , "Icelandic"),
      M__LANG("id"   , LANG_INDONESIAN, SUBLANG_DEFAULT                   , "Indonesian"),
      M__LANG("it"   , LANG_ITALIAN   , SUBLANG_ITALIAN                   , "Italian"),
      M__LANG("ja"   , LANG_JAPANESE  , SUBLANG_DEFAULT                   , "Japanese"),
      M__LANG("kn"   , LANG_KANNADA   , SUBLANG_DEFAULT                   , "Kannada"),
      M__LANG("ks"   , LANG_KASHMIRI  , SUBLANG_DEFAULT                   , "Kashmiri"),
      M__LANG("kk"   , LANG_KAZAK     , SUBLANG_DEFAULT                   , "Kazakh"),
      M__LANG("ko"   , LANG_KOREAN    , SUBLANG_KOREAN                    , "Korean"),
      M__LANG("lv"   , LANG_LATVIAN   , SUBLANG_DEFAULT                   , "Latvian"),
      M__LANG("lt"   , LANG_LITHUANIAN, SUBLANG_LITHUANIAN                , "Lithuanian"),
      M__LANG("mk"   , LANG_MACEDONIAN, SUBLANG_DEFAULT                   , "Macedonian"),
      M__LANG("ml"   , LANG_MALAYALAM , SUBLANG_DEFAULT                   , "Malayalam"),
      M__LANG("ms"   , LANG_MALAY     , SUBLANG_DEFAULT                   , "Malay"),
      M__LANG("mr"   , LANG_MARATHI   , SUBLANG_DEFAULT                   , "Marathi"),
      M__LANG("mn"   , LANG_MONGOLIAN , SUBLANG_DEFAULT                   , "Mongolian"),
      M__LANG("ne"   , LANG_NEPALI    , SUBLANG_DEFAULT                   , "Nepali"),
      M__LANG("nb"   , LANG_NORWEGIAN , SUBLANG_NORWEGIAN_BOKMAL          , "Norwegian (Bokmal)"),
      M__LANG("nn"   , LANG_NORWEGIAN , SUBLANG_NORWEGIAN_NYNORSK         , "Norwegian (Nynorsk)"),
      M__LANG("or"   , LANG_ORIYA     , SUBLANG_DEFAULT                   , "Oriya"),
      M__LANG("pl"   , LANG_POLISH    , SUBLANG_DEFAULT                   , "Polish"),
      M__LANG("pt"   , LANG_PORTUGUESE, SUBLANG_PORTUGUESE                , "Portuguese"),
      M__LANG("pt_BR", LANG_PORTUGUESE, SUBLANG_PORTUGUESE_BRAZILIAN      , "Portuguese (Brazilian)"),
      M__LANG("pa"   , LANG_PUNJABI   , SUBLANG_DEFAULT                   , "Punjabi"),
      M__LANG("ro"   , LANG_ROMANIAN  , SUBLANG_DEFAULT                   , "Romanian"),
      M__LANG("ru"   , LANG_RUSSIAN   , SUBLANG_DEFAULT                   , "Russian"),
      M__LANG("sa"   , LANG_SANSKRIT  , SUBLANG_DEFAULT                   , "Sanskrit"),
      M__LANG("sr"   , LANG_SERBIAN   , SUBLANG_SERBIAN_CYRILLIC          , "Serbian (Cyrillic)"),
      M__LANG("sd"   , LANG_SINDHI    , SUBLANG_DEFAULT                   , "Sindhi"),
      M__LANG("sk"   , LANG_SLOVAK    , SUBLANG_DEFAULT                   , "Slovak"),
      M__LANG("sl"   , LANG_SLOVENIAN , SUBLANG_DEFAULT                   , "Slovenian"),
      M__LANG("es"   , LANG_SPANISH   , SUBLANG_SPANISH                   , "Spanish"),
      M__LANG("es_AR", LANG_SPANISH   , SUBLANG_SPANISH_ARGENTINA         , "Spanish (Argentina)"),
      M__LANG("es_BO", LANG_SPANISH   , SUBLANG_SPANISH_BOLIVIA           , "Spanish (Bolivia)"),
      M__LANG("es_CL", LANG_SPANISH   , SUBLANG_SPANISH_CHILE             , "Spanish (Chile)"),
      M__LANG("es_CO", LANG_SPANISH   , SUBLANG_SPANISH_COLOMBIA          , "Spanish (Colombia)"),
      M__LANG("es_CR", LANG_SPANISH   , SUBLANG_SPANISH_COSTA_RICA        , "Spanish (Costa Rica)"),
      M__LANG("es_DO", LANG_SPANISH   , SUBLANG_SPANISH_DOMINICAN_REPUBLIC, "Spanish (Dominican republic)"),
      M__LANG("es_EC", LANG_SPANISH   , SUBLANG_SPANISH_ECUADOR           , "Spanish (Ecuador)"),
      M__LANG("es_SV", LANG_SPANISH   , SUBLANG_SPANISH_EL_SALVADOR       , "Spanish (El Salvador)"),
      M__LANG("es_GT", LANG_SPANISH   , SUBLANG_SPANISH_GUATEMALA         , "Spanish (Guatemala)"),
      M__LANG("es_HN", LANG_SPANISH   , SUBLANG_SPANISH_HONDURAS          , "Spanish (Honduras)"),
      M__LANG("es_MX", LANG_SPANISH   , SUBLANG_SPANISH_MEXICAN           , "Spanish (Mexican)"),
      M__LANG("es_ES", LANG_SPANISH   , SUBLANG_SPANISH_MODERN            , "Spanish (Modern)"),
      M__LANG("es_NI", LANG_SPANISH   , SUBLANG_SPANISH_NICARAGUA         , "Spanish (Nicaragua)"),
      M__LANG("es_PA", LANG_SPANISH   , SUBLANG_SPANISH_PANAMA            , "Spanish (Panama)"),
      M__LANG("es_PY", LANG_SPANISH   , SUBLANG_SPANISH_PARAGUAY          , "Spanish (Paraguay)"),
      M__LANG("es_PE", LANG_SPANISH   , SUBLANG_SPANISH_PERU              , "Spanish (Peru)"),
      M__LANG("es_PR", LANG_SPANISH   , SUBLANG_SPANISH_PUERTO_RICO       , "Spanish (Puerto Rico)"),
      M__LANG("es_UY", LANG_SPANISH   , SUBLANG_SPANISH_URUGUAY           , "Spanish (Uruguay)"),
      M__LANG("es_VE", LANG_SPANISH   , SUBLANG_SPANISH_VENEZUELA         , "Spanish (Venezuela)"),
      M__LANG("sw"   , LANG_SWAHILI   , SUBLANG_DEFAULT                   , "Swahili"),
      M__LANG("sv"   , LANG_SWEDISH   , SUBLANG_SWEDISH                   , "Swedish"),
      M__LANG("ta"   , LANG_TAMIL     , SUBLANG_DEFAULT                   , "Tamil"),
      M__LANG("tt"   , LANG_TATAR     , SUBLANG_DEFAULT                   , "Tatar"),
      M__LANG("te"   , LANG_TELUGU    , SUBLANG_DEFAULT                   , "Telugu"),
      M__LANG("th"   , LANG_THAI      , SUBLANG_DEFAULT                   , "Thai"),
      M__LANG("tr"   , LANG_TURKISH   , SUBLANG_DEFAULT                   , "Turkish"),
      M__LANG("uk"   , LANG_UKRAINIAN , SUBLANG_DEFAULT                   , "Ukrainian"),
      M__LANG("ur"   , LANG_URDU      , SUBLANG_DEFAULT                   , "Urdu"),
      M__LANG("uz"   , LANG_UZBEK     , SUBLANG_DEFAULT                   , "Uzbek"),
      M__LANG("vi"   , LANG_VIETNAMESE, SUBLANG_DEFAULT                   , "Vietnamese")
   };    

   static const MLangDef* DoGetLangDef(const MStdString& localeName) M_NO_THROW
   {
      MConstChars langChars = localeName.c_str();
      for ( int i = 0; i < (int)M_NUMBER_OF_ARRAY_ELEMENTS(s_lang); ++i )
      {
         const MLangDef* lang = &(s_lang[i]);
         if ( m_stricmp(langChars, lang->m_lang) == 0 )
            return lang;
      }
      return NULL;
   }

MStdString MMessageCatalog::StaticGetLocaleDescription(const MStdString& lang) M_NO_THROW
{
   MStdString result;
   const MLangDef* def = DoGetLangDef(lang);
   if ( def != NULL )
      result = def->m_localeName;
   return result;
}

#if (M_OS & M_OS_WINDOWS) != 0

LANGID MMessageCatalog::StaticGetLangID(const MStdString& lang) M_NO_THROW
{
   const MLangDef* def = DoGetLangDef(lang);
   if ( def != NULL )
      return def->m_langId;
   return 0;
}

#endif

#endif // !M_NO_MESSAGE_CATALOG
