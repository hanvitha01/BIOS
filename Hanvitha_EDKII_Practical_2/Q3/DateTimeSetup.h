#ifndef _DATE_TIME_SETUP_H_
#define _DATE_TIME_SETUP_H_

#include <Uefi.h>
#include <Guid/MdeModuleHii.h>
#include <Protocol/HiiConfigAccess.h>
#include "DateTimeNvData.h"  // <--- ADD THIS HERE

#define DATETIME_CALLBACK_SIGNATURE  SIGNATURE_32('D','T','C','B')

typedef struct {
  UINTN                           Signature;
  EFI_HANDLE                      DriverHandle;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_HII_CONFIG_ACCESS_PROTOCOL  ConfigAccess;
  EFI_EVENT                       TimerEvent;
  DATETIME_CONFIGURATION          NvData; // Now the compiler knows what this is!
} DATETIME_CALLBACK_DATA;

#define DATETIME_CALLBACK_FROM_THIS(a) \
  CR(a, DATETIME_CALLBACK_DATA, ConfigAccess, DATETIME_CALLBACK_SIGNATURE)

#endif