#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/DevicePathLib.h>

#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiConfigRouting.h>
#include <Protocol/DevicePath.h>
#include <Guid/MdeModuleHii.h> 

#include "DateTimeSetup.h"
#include "DateTimeNvData.h"
#include "DateTimeFormGuid.h"

// Standard EDK2 refresh request identifier (fallback definition)
#ifndef EFI_BROWSER_ACTION_REQUEST_FORM_REFRESH
#define EFI_BROWSER_ACTION_REQUEST_FORM_REFRESH 0x10
#endif

extern UINT8  DateTimeFormsBin[];    
extern UINT8  DateTimeSetupStrings[];
EFI_GUID gDATETIME_FORMSET_GUID = DATETIME_FORMSET_GUID;

STATIC DATETIME_CALLBACK_DATA *mPrivate = NULL;

typedef struct {
  VENDOR_DEVICE_PATH          VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL    End;
} HII_VENDOR_DEVICE_PATH;

STATIC HII_VENDOR_DEVICE_PATH mVendorDevicePath = {
  {
    { HARDWARE_DEVICE_PATH, HW_VENDOR_DP, { (UINT8)(sizeof(VENDOR_DEVICE_PATH)), (UINT8)((sizeof(VENDOR_DEVICE_PATH)) >> 8) } },
    DATETIME_FORMSET_GUID
  },
  { END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE, { (UINT8)(sizeof(EFI_DEVICE_PATH_PROTOCOL)), (UINT8)((sizeof(EFI_DEVICE_PATH_PROTOCOL)) >> 8) } }
};

// --- Forward Declarations ---
EFI_STATUS EFIAPI DateTimeExtractConfig(IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL *This, IN CONST EFI_STRING Request, OUT EFI_STRING *Progress, OUT EFI_STRING *Results);
EFI_STATUS EFIAPI DateTimeRouteConfig(IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL *This, IN CONST EFI_STRING Configuration, OUT EFI_STRING *Progress);
EFI_STATUS EFIAPI DateTimeCallback(IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL *This, IN EFI_BROWSER_ACTION Action, IN EFI_QUESTION_ID QuestionId, IN UINT8 Type, IN EFI_IFR_TYPE_VALUE *Value, OUT EFI_BROWSER_ACTION_REQUEST *ActionRequest);

/**
  Initializes the NVRAM variable "AutoRefreshVar".
  If the variable doesn't exist, it seeds it with the PCD value.
**/
VOID InitializeNvramVariable (VOID) {
  EFI_STATUS Status;
  UINTN      Size = sizeof(UINT8);
  UINT8      Value;

  Status = gRT->GetVariable(L"AutoRefreshVar", &gDATETIME_FORMSET_GUID, NULL, &Size, &Value);
  
  if (EFI_ERROR(Status)) {
    // Variable missing: Use PCD default and write to NVRAM
    Value = (UINT8)PcdGetBool(PcdEnableDateTimeRefresh);
    gRT->SetVariable(
      L"AutoRefreshVar", 
      &gDATETIME_FORMSET_GUID, 
      EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE, 
      sizeof(UINT8), 
      &Value
    );
    DEBUG((DEBUG_INFO, "DateTimeSetup: NVRAM Variable initialized from PCD to %d\n", Value));
  }
}

// --- Extract Config (Handles Automatic Refresh) ---
EFI_STATUS EFIAPI DateTimeExtractConfig (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL *This,
  IN CONST EFI_STRING Request,
  OUT EFI_STRING *Progress,
  OUT EFI_STRING *Results
  ) 
{
  EFI_TIME Time;

  if (Progress == NULL || Results == NULL) return EFI_INVALID_PARAMETER;

  // The HII Browser calls this function every second because of 'refresh interval = 1' in VFR.
  // We fetch the current hardware time so the UI display is always current.
  if (!EFI_ERROR(gRT->GetTime(&Time, NULL))) {
    mPrivate->NvData.Year   = Time.Year;
    mPrivate->NvData.Month  = Time.Month;
    mPrivate->NvData.Day    = Time.Day;
    mPrivate->NvData.Hour   = Time.Hour;
    mPrivate->NvData.Minute = Time.Minute;
    mPrivate->NvData.Second = Time.Second;
  }

  *Progress = Request;
  return gHiiConfigRouting->BlockToConfig(gHiiConfigRouting, Request, (UINT8 *)&mPrivate->NvData, sizeof(DATETIME_CONFIGURATION), Results, Progress);
}

// --- Route Config (Handles Manual User Changes) ---
EFI_STATUS EFIAPI DateTimeRouteConfig (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL *This, 
  IN CONST EFI_STRING Configuration, 
  OUT EFI_STRING *Progress
  ) 
{
  UINTN BufferSize = sizeof (DATETIME_CONFIGURATION);
  EFI_STATUS Status;

  if (Configuration == NULL || Progress == NULL) return EFI_INVALID_PARAMETER;
  
  Status = gHiiConfigRouting->ConfigToBlock(gHiiConfigRouting, Configuration, (UINT8 *)&mPrivate->NvData, &BufferSize, Progress);
  
  if (!EFI_ERROR(Status)) {
      EFI_TIME Time;
      // If user manually modifies the menu fields, sync those changes to the hardware RTC
      if (!EFI_ERROR(gRT->GetTime(&Time, NULL))) {
        Time.Year   = mPrivate->NvData.Year;
        Time.Month  = mPrivate->NvData.Month;
        Time.Day    = mPrivate->NvData.Day;
        Time.Hour   = mPrivate->NvData.Hour;
        Time.Minute = mPrivate->NvData.Minute;
        Time.Second = mPrivate->NvData.Second;
        gRT->SetTime(&Time);
      }
  }
  return Status;
}

// --- Callback (Handles UI Refresh Logic) ---
EFI_STATUS EFIAPI DateTimeCallback (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL *This,
  IN EFI_BROWSER_ACTION Action,
  IN EFI_QUESTION_ID QuestionId,
  IN UINT8 Type,
  IN EFI_IFR_TYPE_VALUE *Value,
  OUT EFI_BROWSER_ACTION_REQUEST *ActionRequest
  ) 
{
  if (Action != EFI_BROWSER_ACTION_CHANGED) return EFI_SUCCESS;

  if (QuestionId == 0x102) {
    // Checkbox toggled. We tell the browser to refresh the UI so that the
    // 'suppressif' block re-evaluates and shows/hides the clock fields immediately.
    DEBUG((DEBUG_INFO, "DateTimeSetup: Auto-Refresh Toggled. Requesting UI Refresh.\n"));
    
    // Using FORM_APPLY as a robust alternative if FORM_REFRESH is picky in your env
    *ActionRequest = EFI_BROWSER_ACTION_REQUEST_FORM_APPLY;
  }
  
  return EFI_SUCCESS;
}

// --- Constructor ---
EFI_STATUS EFIAPI DateTimeSetupConstructor (IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
  EFI_STATUS Status;

  mPrivate = AllocateZeroPool(sizeof(DATETIME_CALLBACK_DATA));
  if (mPrivate == NULL) return EFI_OUT_OF_RESOURCES;

  mPrivate->Signature = DATETIME_CALLBACK_SIGNATURE;
  mPrivate->DriverHandle = NULL; 
  mPrivate->ConfigAccess.ExtractConfig = DateTimeExtractConfig;
  mPrivate->ConfigAccess.RouteConfig   = DateTimeRouteConfig;
  mPrivate->ConfigAccess.Callback      = DateTimeCallback;

  // Ensure NVRAM exists (seeded by PCD if first boot)
  InitializeNvramVariable();

  Status = gBS->InstallMultipleProtocolInterfaces(
                  &mPrivate->DriverHandle, 
                  &gEfiDevicePathProtocolGuid, &mVendorDevicePath, 
                  &gEfiHiiConfigAccessProtocolGuid, &mPrivate->ConfigAccess, 
                  NULL
                  );
  
  if (EFI_ERROR(Status)) {
    FreePool(mPrivate);
    return Status;
  }

  mPrivate->HiiHandle = HiiAddPackages(&gDATETIME_FORMSET_GUID, mPrivate->DriverHandle, DateTimeFormsBin, DateTimeSetupStrings, NULL);
  
  return EFI_SUCCESS;
}

// --- Destructor ---
EFI_STATUS EFIAPI DateTimeSetupDestructor (IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
  if (mPrivate == NULL) return EFI_SUCCESS;

  if (mPrivate->HiiHandle != NULL) {
    HiiRemovePackages(mPrivate->HiiHandle);
  }

  gBS->UninstallMultipleProtocolInterfaces(
         mPrivate->DriverHandle, 
         &gEfiDevicePathProtocolGuid, &mVendorDevicePath, 
         &gEfiHiiConfigAccessProtocolGuid, &mPrivate->ConfigAccess, 
         NULL
         );

  FreePool(mPrivate);
  mPrivate = NULL;
  return EFI_SUCCESS;
}