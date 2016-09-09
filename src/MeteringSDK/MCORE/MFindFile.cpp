// File MCORE/MFindFile.cpp

#include "MCOREExtern.h"
#include "MFindFile.h"
#include "MUtilities.h"
#include "MException.h"
#include "MJavaEnv.h"

#if !M_NO_FILESYSTEM

#if (M_OS & M_OS_POSIX) != 0
   #define BAD_DIRECTORY_SEPARATOR '\\'
#elif (M_OS & (M_OS_DOS | M_OS_WINDOWS)) != 0
   #define BAD_DIRECTORY_SEPARATOR '/'
#else
   #error "Operating system is unknown!"
#endif

#if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI

   static jmethodID  s_idList = NULL;

#endif

    #define excludeLibraries reserved

MFindFile::MFindFile()
:
   m_index(0),
   m_results(),
   m_directory(),
   m_returned()
{
}


MFindFile::MFindFile(MConstChars directory, MConstChars fileMask, bool directories, bool excludeLibraries, bool excludeDotFiles)
:
   m_index(0),
   m_results(),
   m_directory(),
   m_returned()
{
   Init(directory, fileMask, directories, excludeLibraries, excludeDotFiles);
}

MFindFile::~MFindFile() M_NO_THROW
{
}

void MFindFile::Close() M_NO_THROW
{
   m_index = 0;
   m_results.clear();
   m_returned.clear();
}

void MFindFile::Init(MConstChars directory, MConstChars fileMask, bool searchForDirectories, bool excludeLibraries, bool excludeDotFiles)
{
   Close();

   if ( directory == NULL || fileMask == NULL || fileMask[0] == '\0' )
   {
      DoThrowFindArgumentBad();
      M_ENSURED_ASSERT(0);
   }

   m_directory = MUtilities::GetFullPath(directory);
   MAddDirectorySeparatorIfNecessary(m_directory);

   Populate(m_results, m_directory, fileMask, searchForDirectories, excludeLibraries, excludeDotFiles);
}

void MFindFile::DoPopulate(MStdStringVector& result, const MStdString& directory, const MStdString& fileMask, bool searchForDirectories, bool excludeDotFiles)
{
   MStdString path = MUtilities::GetFullPath(directory);

   MStdString::iterator it = path.begin();
   MStdString::iterator itEnd = path.end();
   for ( ; it != itEnd; ++it )
      if ( *it == BAD_DIRECTORY_SEPARATOR ) // convert directory separator to OS-specific
         *it = M_DIRECTORY_SEPARATOR;
   MAddDirectorySeparatorIfNecessary(path);

#if (M_OS & M_OS_WINDOWS) != 0

   path += fileMask;

   WIN32_FIND_DATA findData;
   #if M_UNICODE
      HANDLE handle = ::FindFirstFile(MToWideString(path).c_str(), &findData);
   #else
      HANDLE handle = ::FindFirstFile(path.c_str(), &findData);
   #endif
   if ( handle != INVALID_HANDLE_VALUE )
   {
      do
      {
         LPCTSTR name = findData.cFileName;
         if ( !(name[0] == '.' && (excludeDotFiles || ((name[1] == '\0') || (name[1] == '.' && name[2] == '\0')))) &&
              ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) == searchForDirectories )
         {
            #if M_UNICODE
               result.push_back(MToStdString(findData.cFileName));
            #else
               result.push_back(findData.cFileName);
            #endif
         }
      } while ( ::FindNextFile(handle, &findData) );
   }
   ::FindClose(handle);

#else

#if (M_OS & M_OS_ANDROID) != 0 & !M_NO_JNI
   if ( !directory.empty() && directory[0] == ':' ) // android resource convention
   {
      AAssetManager* assetManager = MJavaEnv::GetJniAssetManager();

      MStdString::const_iterator end = directory.end();
      if ( *(end - 1) == '/' || *(end - 1) == '\\' ) // remove trailing slash
         --end;
      MStdString dirString(directory.begin() + 1, end);
      const char* dirName = dirString.c_str();

      AAssetDir* dir = AAssetManager_openDir(assetManager, dirName);
      while ( const char* name = AAssetDir_getNextFileName(dir) )
      {
         const MStdString& fileName = MUtilities::GetPathFileNameAndExtension(name);
         if ( fileName.empty() ) // something is badly wrong with the name, recover
            continue;
         if ( excludeDotFiles && fileName[0] == '.' )
            continue;

         if ( fnmatch(fileMask.c_str(), fileName.c_str(), 0) == 0 )
            result.push_back(fileName);
      }
      AAssetDir_close(dir);

      if ( searchForDirectories )
      {
         MJavaEnv env;
         jobject javaAssetManager = env.GetAssetManager();
         if ( s_idList == NULL )
         {
            jclass clazz = env.FindClass("android/content/res/AssetManager");
            s_idList = env.GetMethodID(clazz, "list", "(Ljava/lang/String;)[Ljava/lang/String;");
         }

         jstring path = env.NewLocalStringUTF(dirName);
         jobjectArray names = reinterpret_cast<jobjectArray>(env->CallObjectMethod(javaAssetManager, s_idList, path));
         env.CheckForJavaException(); // names will not be returned if there is an exception

         MStdStringVector fileResult;
         fileResult.swap(result); // result becomes empty, fileResult holds files

         int size = env->GetArrayLength(names);
         for ( int i = 0; i < size; ++i )
         {
            jstring jStr = (jstring)env->GetObjectArrayElement(names, i);
            const char* cStr = env->GetStringUTFChars(jStr, 0);
            if ( cStr != NULL &&
                 cStr[0] != '\0' &&
                 (!excludeDotFiles || cStr[0] != '.') &&
                 fnmatch(fileMask.c_str(), cStr, 0) == 0 )
            {
               MStdString name = cStr;
               if ( std::find(fileResult.begin(), fileResult.end(), name) == fileResult.end() ) // not a file, directory
                  result.push_back(name); // it is
            }
            env->ReleaseStringUTFChars(jStr, cStr);
            env->DeleteLocalRef(jStr);
         }
      }
   }
   else
#endif
   {
      DIR* dir = opendir(path.c_str());   // access directory directly
      if ( dir != NULL )
      {
         for ( ; ; )
         {
#if (M_OS & M_OS_QNXNTO)
            struct dirent *entry, *entryPtr;
            entry = (dirent*)malloc(sizeof(*entry) + NAME_MAX + 1);
            int error = readdir_r(dir, entry, &entryPtr);
            if ( error != 0 || entryPtr == NULL )
            {
               free(entry);
               break;
            }
#else
            struct dirent* entryPtr = readdir(dir);
            if ( entryPtr == NULL )
               break;
#endif
            // using default search rules
            const char* name = entryPtr->d_name;
            if ( fnmatch(fileMask.c_str(), name, 0) == 0 )  // maybe found
            {
               // ignore special '..' and '.' files
               //
               bool isdir = MUtilities::IsPathDirectory(path + name);
               if ( (name[0] == '.') ||
                    (searchForDirectories && !isdir) ||
                    (!searchForDirectories && isdir))
               {
#if (M_OS & M_OS_QNXNTO)
                  free(entry);
#endif
                  continue;
               }

               result.push_back(name);
            }

#if (M_OS & M_OS_QNXNTO)
            free(entry);
#endif
         }
         closedir(dir);
      }
   }
#endif
}


MConstChars MFindFile::FindNext(bool returnFullPath)
{
   m_returned.clear();

   if ( m_index >= (unsigned)m_results.size() )
      return NULL;

   if ( returnFullPath )
      m_returned = m_directory;
   m_returned += m_results[m_index];
   ++ m_index;
   return m_returned.c_str();
}

M_NORETURN_FUNC void MFindFile::DoThrowFindArgumentBad()
{
   MException::Throw(MException::ErrorSoftware, M_ERR_FIND_ARGUMENT_IS_BAD, "Argument of File Find method is bad");
   M_ENSURED_ASSERT(0);
}

#endif // M_NO_FILESYSTEM
