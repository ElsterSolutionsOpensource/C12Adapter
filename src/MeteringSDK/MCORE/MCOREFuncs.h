#ifndef MCORE_MCOREFUNCS_H
#define MCORE_MCOREFUNCS_H
// File MCORE/MCOREFuncs.h
//
// Definitions for Common API based on POSIX
//

#ifndef MCORE_MCOREDEFS_H
   #error "Include file MCORE/MCOREDefs.h instead of MCORE/MCOREFuncs.h!"
#endif

#if defined(_WIN32_WCE) // Microsoft proprietary macro
   #define M_MAX_PATH  MAX_PATH
   #define M_MAX_DRIVE MAX_PATH    // there are no drives actually...
   #define M_MAX_DIR   MAX_PATH
   #define M_MAX_FNAME MAX_PATH
   #define M_MAX_EXT   MAX_PATH
#elif (M_OS & M_OS_CMX)
   #define M_MAX_PATH  250
   #define M_MAX_DRIVE 250    // there are no drives actually...
   #define M_MAX_DIR   250
   #define M_MAX_FNAME 250
   #define M_MAX_EXT   250
#elif defined(_MAX_PATH) // Microsoft proprietary macro
   #define M_MAX_PATH  _MAX_PATH
   #define M_MAX_DRIVE _MAX_DRIVE
   #define M_MAX_DIR   _MAX_DIR
   #define M_MAX_FNAME _MAX_FNAME
   #define M_MAX_EXT   _MAX_EXT
#elif defined(MAX_PATH) // Standard C
   #define M_MAX_PATH  MAX_PATH
   #define M_MAX_DRIVE MAX_PATH // M_MAX_DRIVE will not be defined for an ANSI C and POSIX
   #define M_MAX_DIR   MAX_DIR
   #define M_MAX_FNAME MAX_FNAME
   #define M_MAX_EXT   MAX_EXT
#elif defined(MAXPATH)   // Borland proprietary macro
   #define M_MAX_PATH  MAXPATH
   #define M_MAX_DRIVE MAXDRIVE
   #define M_MAX_DIR   MAXDIR
   #define M_MAX_FNAME MAXFILE
   #define M_MAX_EXT   MAXEXT
#elif defined(PATH_MAX) // Cygnus proprietary macro   
   #define M_MAX_PATH  PATH_MAX
   #define M_MAX_DRIVE PATH_MAX  // there are no drives either...
   #define M_MAX_DIR   PATH_MAX
   #define M_MAX_FNAME PATH_MAX
   #define M_MAX_EXT   PATH_MAX
#else
   #error "Please check M_MAX_ macros for your compiler!"
#endif

#if defined(_MSC_VER)

   #define alloca     _alloca
   #define itow       _itow
   #define itoa       _itoa
   #define ltow       _ltow
   #define ltoa       _ltoa
   #define ultow      _ultow
   #define ultoa      _ultoa
   #define i64tow     _i64tow
   #define i64toa     _i64toa
   #define ui64tow    _ui64tow
   #define ui64toa    _ui64toa
   #define wgetenv    _wgetenv
   #define getch      _getch
   #define getche     _getche
   #define gcvt       _gcvt

   // ISO compliant names for DevStudio 2005
   #define stricmp      _stricmp
   #define wcsicmp      _wcsicmp
   #define strnicmp     _strnicmp
   #define wcsnicmp     _wcsnicmp

   #define ff_name name // mapping for ffblk structure(_finddata_t)

   #define S_IFMT       _S_IFMT
   #define S_IFDIR      _S_IFDIR
   #define S_IFCHR      _S_IFCHR
   #define S_IFREG      _S_IFREG
   #define S_IREAD      _S_IREAD
   #define S_IWRITE     _S_IWRITE
   #define S_IEXEC      _S_IEXEC

   #define O_RDONLY     _O_RDONLY
   #define O_WRONLY     _O_WRONLY
   #define O_RDWR       _O_RDWR
   #define O_APPEND     _O_APPEND
   #define O_CREAT      _O_CREAT
   #define O_TRUNC      _O_TRUNC
   #define O_EXCL       _O_EXCL
   #define O_TEXT       _O_TEXT
   #define O_BINARY     _O_BINARY
   #define O_RAW        _O_BINARY
   #define O_TEMPORARY  _O_TEMPORARY
   #define O_NOINHERIT  _O_NOINHERIT
   #define O_SEQUENTIAL _O_SEQUENTIAL
   #define O_RANDOM     _O_RANDOM

   #define SH_DENYRW    _SH_DENYRW
   #define SH_DENYWR    _SH_DENYWR
   #define SH_DENYRD    _SH_DENYRD
   #define SH_DENYNO    _SH_DENYNO

   #define m_ffblk _finddata_t
   #define m_splitpath _splitpath
   #define m_makepath _makepath

#elif defined(__BORLANDC__)

   #define i64tow   _i64tow
   #define i64toa   _i64toa
   #define ui64tow  _ui64tow
   #define ui64toa  _ui64toa
   #define wgetenv  _wgetenv
   #define ltoa     _ltoa
   #define ltow     _ltow
   #define ultow    _ultow

   #define timezone _timezone
   #define daylight _daylight

   #if M_UNICODE
      #define m_ffblk _wffblk
      #define m_splitpath _wfnsplit
      #define m_makepath _wfnmerge
   #else
      #define m_ffblk ffblk
      #define m_splitpath fnsplit
      #define m_makepath fnmerge
   #endif

#endif

// Because of the popularity of the following names in plain C++ code, we cannot just use m_read.
// This is why we add mstd prefix.
//
#if defined(_MSC_VER)
   #define mstd_lseek      _lseek
   #define mstd_filelength _filelength
   #define mstd_chsize     _chsize
   #define mstd_read       _read
   #define mstd_write      _write
   #define mstd_open       _open
   #define mstd_sopen      _sopen
#else
   #define mstd_lseek      lseek
   #define mstd_filelength filelength
   #define mstd_chsize     chsize
   #define mstd_read       read
   #define mstd_write      write
   #define mstd_open       open
   #define mstd_sopen      sopen
#endif

#define m_asctime   asctime
#define m_strftime  strftime
#define m_getenv    getenv
#define m_isalpha   isalpha
#define m_isupper   isupper
#define m_islower   islower
#define m_isdigit   isdigit
#define m_isxdigit  isxdigit
#define m_isspace   isspace
#define m_ispunct   ispunct
#define m_isalnum   isalnum
#define m_isprint   isprint
#define m_isgraph   isgraph
#define m_iscntrl   iscntrl
#define m_isascii   isascii
#define m_itoa      itoa
#define m_ltoa      ltoa
#define m_ultoa     ultoa
#define m_i64toa    i64toa
#define m_ui64toa   ui64toa
#define m_toupper   toupper
#define m_tolower   tolower
#define m_printf    printf
#define m_fprintf   fprintf
#define m_sprintf   sprintf
#define m_snprintf  _snprintf
#define m_vprintf   vprintf
#define m_vfprintf  vfprintf
#define m_vsprintf  vsprintf
#define m_vsnprintf vsnprintf
#define m_scanf     scanf
#define m_fopen     fopen
#define m_fscanf    fscanf
#define m_sscanf    sscanf
#define m_fgetc     fgetc
#define m_fgetchar  fgetchar
#define m_fgets     fgets
#define m_fputc     fputc
#define m_fputchar  fputchar
#define m_fputs     fputs
#define m_getc      getc
#define m_gets      gets
#define m_putc      putc
#define m_puts      puts
#define m_ungetc    ungetc
#define m_strtod    strtod
#define m_strtol    strtol
#define m_strtoul   strtoul
#define m_strcat    strcat
#define m_strchr    strchr
#define m_strrchr   strrchr
#define m_strcmp    strcmp
#define m_strcpy    strcpy
#define m_strcpy_s  strcpy_s
#define m_strcspn   strcspn
#define m_strlen    strlen
#define m_strncat   strncat
#define m_strncmp   strncmp
#define m_strncpy   strncpy
#define m_strpbrk   strpbrk
#define m_strrchr   strrchr
#define m_strspn    strspn
#define m_strstr    strstr
#define m_strtok    strtok
#define m_strdup    strdup
   
#if (M_OS & M_OS_UNIX) || (M_OS & M_OS_LINUX) || (M_OS & M_OS_CYGWIN) || (M_OS & M_OS_BSD)
// GNU-style String/Array Comparison functions
#ifndef strcmpi
   #define strcmpi strcasecmp
#endif
#ifndef stricmp
   #define stricmp strcasecmp
#endif
#ifndef strncmpi
   #define strncmpi strncasecmp
#endif
#ifndef strnicmp
   #define strnicmp strncasecmp
#endif
#endif
   
#define m_stricmp   stricmp
#define m_strnicmp  strnicmp
#define m_strnset   strnset
#define m_strrev    strrev
#define m_strset    strset
#define m_strlwr    strlwr
#define m_strupr    strupr
#define m_strxfrm   strxfrm
#define m_strcoll   strcoll
#define m_stricoll  stricoll
#define m_remove    remove
#define m_rename    rename
#define m_cout      cout
#define m_cerr      cerr
#define m_cin       cin
#define m_setlocale setlocale
#define m_main      main

#endif

