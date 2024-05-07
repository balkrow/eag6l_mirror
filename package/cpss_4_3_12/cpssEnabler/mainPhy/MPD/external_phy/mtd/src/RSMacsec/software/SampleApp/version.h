#pragma once

#if defined(WIN32) || defined(_WIN32)
#include "macsec_cfg.h"
#else
#include "macsec_cfg.h"
#define SVN_REVISION  MACSEC_SWAPI_VERSION_BUILD
#endif

#define _STRINGIFY(s)   #s
#define  STRINGIFY(s)    _STRINGIFY(s)

#define VERSION_MAJOR       MACSEC_SWAPI_VERSION_MAJOR
#define VERSION_MINOR       MACSEC_SWAPI_VERSION_MINOR
#define VERSION_REVISION    MACSEC_SWAPI_VERSION_RELEASE
#define VERSION_BUILD       SVN_REVISION

#if SVN_LOCAL_MODIFICATIONS
#define VERSION_MODIFIER "M"
#else
#define VERSION_MODIFIER
#endif

#define VER_FILE_DESCRIPTION_STR    "Rianta MACSec Software Simulator. Built " STRINGIFY(SVN_TIME_NOW)
#define VER_FILE_VERSION            VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD
#define VER_FILE_VERSION_STR        STRINGIFY(VERSION_MAJOR)        \
                                    "." STRINGIFY(VERSION_MINOR)    \
                                    "." STRINGIFY(VERSION_REVISION) \
                                    "." STRINGIFY(VERSION_BUILD)    \

#define VER_PRODUCTNAME_STR         "MacSecTst"
#define VER_PRODUCT_VERSION         VER_FILE_VERSION
#define VER_PRODUCT_VERSION_STR     VER_FILE_VERSION_STR
#define VER_ORIGINAL_FILENAME_STR   VER_PRODUCTNAME_STR ".exe"
#define VER_COPYRIGHT_STR           "Copyright (C) 2019"

#ifdef _DEBUG
#define VER_VER_DEBUG       VS_FF_DEBUG
#else 
#define VER_VER_DEBUG       0
#endif

#define VER_FILEOS          VOS_NT_WINDOWS32
#define VER_FILEFLAGS       VER_VER_DEBUG
#define VER_FILETYPE        VFT_APP
