MACRO(GLOB_FILES OUTPUT)
  FILE(GLOB ${OUTPUT} RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ARGN})
  LIST(SORT ${OUTPUT})
ENDMACRO()

MACRO(GLOB_FILES_RECURSE OUTPUT)
  FILE(GLOB_RECURSE ${OUTPUT} RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ARGN})
  LIST(SORT ${OUTPUT})
ENDMACRO()

MACRO(SET_UNIX_SOVERSION TARGET VERSION)
  IF(UNIX)
    SET_TARGET_PROPERTIES(${TARGET} PROPERTIES SOVERSION ${VERSION})
  ENDIF()
ENDMACRO()

MACRO(SET_MSVC_STATIC_RUNTIME FLAG)
  IF(NOT MSVC)
    MESSAGE(FATAL_ERROR "MSVC is not defined")
  ENDIF()
  IF(${FLAG})
    FOREACH(_FLAGS
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL
        CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL
        CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      STRING(REPLACE "/MD" "/MT" ${_FLAGS} "${${_FLAGS}}")
    ENDFOREACH()
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:atlthunk.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:msvcrtd.lib")
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:libcmt.lib")
    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:libcmtd.lib")
  ELSE()
    FOREACH(_FLAGS
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL
        CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL
        CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      STRING(REPLACE "/MT" "/MD" ${_FLAGS} "${${_FLAGS}}")
    ENDFOREACH()
  ENDIF()
ENDMACRO()

MACRO(SETUP_DEFAULT_METERINGSDK_DEFINITIONS)
  IF(WIN32)
    ADD_DEFINITIONS(-D_WIN32_WINNT=${WINDOWS_VERSION} -DWINVER=${WINDOWS_VERSION})
    IF(M_UNICODE)
      ADD_DEFINITIONS(-D_UNICODE -DUNICODE)
      #ADD_DEFINITIONS(-D_CSTRING_DISABLE_NARROW_WIDE_CONVERSION) # <- Helpful, but does not work in Visual C++ 2010
      #                                                           #    due to a few problems in MFC headers.
      #                                                           #    Enable after fixing these very few headers (compile errors to guide).
    ENDIF()
    IF(MSVC)
      STRING(REPLACE "/EHsc" "" CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}")
      STRING(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
      SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /EHa /Zm128 /FC")
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa /Zm128 /FC")

      # Remove incremental linking option in all configurations as it causes problems with Visual C++ 2008
      IF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008")
        FOREACH(flag_type EXE MODULE SHARED)
          STRING(REPLACE "INCREMENTAL:YES" "INCREMENTAL:NO" flag_tmp "${CMAKE_${flag_type}_LINKER_FLAGS_DEBUG}")
          STRING(REPLACE "/EDITANDCONTINUE" "" flag_tmp "${CMAKE_${flag_type}_LINKER_FLAGS_DEBUG}")
          SET(CMAKE_${flag_type}_LINKER_FLAGS_DEBUG "/INCREMENTAL:NO ${flag_tmp}" CACHE STRING "Overriding default debug ${flag_type} linker flags." FORCE)
          MARK_AS_ADVANCED(CMAKE_${flag_type}_LINKER_FLAGS_DEBUG)
        ENDFOREACH()
      ENDIF()

      IF(DEFINED BUILD_WITH_STATIC_RUNTIME)
        SET_MSVC_STATIC_RUNTIME(${BUILD_WITH_STATIC_RUNTIME})
      ENDIF()
    ENDIF()
  ENDIF()

  IF(UNIX)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-unused-variable")
    SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
    SET(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
    SET(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")
    SET(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-variable")
    SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
    SET(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")
    SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    INCLUDE(CheckTypeSize)
    CHECK_TYPE_SIZE(void* CHECK_TYPE_SIZE_PVOID)
    IF(__uClinux__)
      SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${CPUFLAGS} ${CMAKE_C_FLAGS_MINSIZEREL}")
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CPUFLAGS} ${CMAKE_C_FLAGS_MINSIZEREL}")
    ELSEIF(ANDROID)
      SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fPIC")  # ??
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")  # ??
    ELSEIF(CLANG)
      SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fPIC") # -fPIC so the shared libraries can be built
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC") # -fPIC so the shared libraries can be built
    ELSE()
      SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fPIC") # -fPIC so the shared libraries can be built
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC") # -fPIC so the shared libraries can be built
    ENDIF()
  ENDIF()
ENDMACRO()

MACRO(SETUP_MFC)
  IF(MSVC)
    IF(BUILD_WITH_STATIC_MFC OR BUILD_WITH_STATIC_RUNTIME)
      SET(CMAKE_MFC_FLAG 1)
    ELSE()
      ADD_DEFINITIONS(-D_AFXDLL)
      SET(CMAKE_MFC_FLAG 2)
    ENDIF()
  ENDIF()
ENDMACRO()

# Add precompiled header to current (default) project
#
MACRO(ADD_PRECOMPILED_HEADER_FILE _PCH_FILE _PCH_SOURCE_FILE _SOURCE_FILES)
    IF(MSVC_IDE) # Studio projects
        ADD_DEFINITIONS(/Yu"${_PCH_FILE}")  # instead of SET_TARGET_PROPERTIES(${_PROJECT} PROPERTIES COMPILE_FLAGS /Yu"${_PCH_FILE}")
        SET_SOURCE_FILES_PROPERTIES(${_PCH_SOURCE_FILE} PPROPERTIES COMPILE_FLAGS /Yc"${_PCH_FILE}")
    ELSEIF(MSVC) # Make files
        GET_FILENAME_COMPONENT(_PCH_BASENAME ${_PCH_FILE} NAME_WE)
        SET(_PCH_BINARY "${CMAKE_CURRENT_BINARY_DIR}/${_PCH_BASENAME}.pch")
        SET(_PCH_SOURCES ${${_SOURCE_FILES}})

        SET_SOURCE_FILES_PROPERTIES(${_PCH_SOURCE_FILE}
            PROPERTIES COMPILE_FLAGS "/Yc\"${_PCH_FILE}\" /Fp\"${_PCH_BINARY}\""
            OBJECT_OUTPUTS "${_PCH_BINARY}")
        SET_SOURCE_FILES_PROPERTIES(${Sources}
            PROPERTIES COMPILE_FLAGS "/Yu\"${_PCH_FILE}\" /FI\"${_PCH_BINARY}\" /Fp\"${_PCH_BINARY}\""
            OBJECT_DEPENDS "${_PCH_BINARY}")
        LIST(APPEND ${SourcesVar} ${_PCH_SOURCE_FILE})
    ELSE()
        GET_FILENAME_COMPONENT(_PCH_FILE_NAME ${_PCH_FILE} NAME)
        GET_FILENAME_COMPONENT(_PCH_FILE_FULLPATH ${_PCH_FILE_NAME} ABSOLUTE)
        SET(_GCH_FILE "${_PCH_FILE_NAME}.gch")
        SET(_GCH_ARGS " -c ${_PCH_FILE_FULLPATH} -o ${_GCH_FILE} ${CMAKE_CXX_FLAGS}")
        IF(CMAKE_BUILD_TYPE STREQUAL "Release")
            SET(_GCH_ARGS "${_GCH_ARGS} ${CMAKE_CXX_FLAGS_RELEASE}")
        ELSEIF(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
            SET(_GCH_ARGS "${_GCH_ARGS} ${CMAKE_CXX_FLAGS_MINSIZEREL}")
        ELSEIF(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
            SET(_GCH_ARGS "${_GCH_ARGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
        ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Debug")
            SET(_GCH_ARGS "${_GCH_ARGS} ${CMAKE_CXX_FLAGS_DEBUG}")
        ENDIF()
        GET_DIRECTORY_PROPERTY(_GCH_COMPILE_DEFINITIONS COMPILE_DEFINITIONS)
        FOREACH(_GCH_COMPILE_DEFINITION ${_GCH_COMPILE_DEFINITIONS})
            SET(_GCH_ARGS "${_GCH_ARGS} -D${_GCH_COMPILE_DEFINITION}")
        ENDFOREACH()
        GET_DIRECTORY_PROPERTY(_GCH_INCLUDE_DIRECTORIES INCLUDE_DIRECTORIES)
        FOREACH(_GCH_INCLUDE_DIRECTORY ${_GCH_INCLUDE_DIRECTORIES})
            SET(_GCH_ARGS "${_GCH_ARGS} -I${_GCH_INCLUDE_DIRECTORY}")
        ENDFOREACH()
        IF(${BUILD_SHARED_LIBS})
            SET(_GCH_ARGS "${_GCH_ARGS} ${CMAKE_SHARED_LIBRARY_CXX_FLAGS}")
        ENDIF()
        IF( __uClinux__ )
            SET(_GCH_ARGS "${CMAKE_CXX_COMPILER_ARG1} ${_GCH_ARGS}")
        ENDIF()
        SEPARATE_ARGUMENTS(_GCH_ARGS)
        IF( __uClinux__ )
            ADD_CUSTOM_COMMAND(OUTPUT ${_GCH_FILE}
                COMMAND rm ARGS -f ${_GCH_FILE}
                COMMAND ${CMAKE_CXX_COMPILER} ARGS ${_GCH_ARGS}
                DEPENDS ${_PCH_FILE_FULLPATH})
        ELSE()
            ADD_CUSTOM_COMMAND(OUTPUT ${_GCH_FILE}
                COMMAND rm ARGS -f ${_GCH_FILE}
                COMMAND ${CMAKE_CXX_COMPILER} ARGS ${CMAKE_CXX_COMPILER_ARG1} ${_GCH_ARGS}
                DEPENDS ${_PCH_FILE_FULLPATH})
        ENDIF()
        FOREACH(_SOURCE_FILE ${${_SOURCE_FILES}})
            SET_SOURCE_FILES_PROPERTIES(${_SOURCE_FILE}
                PROPERTIES COMPILE_FLAGS
                "-Winvalid-pch -include${CMAKE_CURRENT_BINARY_DIR}/${_PCH_FILE_NAME}")
        ENDFOREACH()
        LIST(APPEND ${_SOURCE_FILES} ${_GCH_FILE})
    ENDIF()
ENDMACRO()

# Legacy macro, scheduled to be removed from library
MACRO(ADD_PCH_FILE _PCH_FILE _PCH_SOURCE_FILE _SOURCE_FILES _UNUSED_SHARED)
    MESSAGE(STATUS "!!! ADD_PCH_FILE macro is deprecated, please replace it with ADD_PRECOMPILED_HEADER_FILE!")
    ADD_PRECOMPILED_HEADER_FILE(_PCH_FILE _PCH_SOURCE_FILE _SOURCE_FILES)
ENDMACRO()
