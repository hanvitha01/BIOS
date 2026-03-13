#ifndef _DATETIME_NV_DATA_H_
#define _DATETIME_NV_DATA_H_

#include <Guid/HiiPlatformSetupFormset.h>

#define DATETIME_VARSTORE_ID    0x1234
#define AUTO_REFRESH_VAR_ID     0x5678

// Main configuration structure
typedef struct {
  UINT16  Year;
  UINT8   Month;
  UINT8   Day;
  UINT8   Hour;
  UINT8   Minute;
  UINT8   Second;
} DATETIME_CONFIGURATION;

// Separate structure for the NVRAM variable (used for suppressif)
typedef struct {
  UINT8   AutoRefreshEnabled;
} DATETIME_AUTOREFRESH_NV_VAR;

#endif