#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>

#include <Protocol/DriverBinding.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/ComponentName.h>
#include <Protocol/ComponentName2.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_HANDLE  *HandleBuffer;
  UINTN       HandleCount;
  UINTN       Index;
  UINTN       DriverPrinted = 0;

  Print(L"\r\n===== Loaded Driver Enumeration =====\r\n\r\n");

  //
  // Locate all handles that support Driver Binding Protocol
  //
  Status = gBS->LocateHandleBuffer(
                  ByProtocol,
                  &gEfiDriverBindingProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );

  if (EFI_ERROR(Status)) {
    Print(L"No drivers found: %r\r\n", Status);
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {

    EFI_DRIVER_BINDING_PROTOCOL *DriverBinding;
    EFI_LOADED_IMAGE_PROTOCOL   *LoadedImage;
    EFI_COMPONENT_NAME2_PROTOCOL *ComponentName2;
    EFI_COMPONENT_NAME_PROTOCOL  *ComponentName;
    CHAR16                      *DriverName;
    CHAR16                      *DevicePathText;
    EFI_STATUS                  NameStatus;

    DriverName = NULL;
    DevicePathText = NULL;

    //
    // Get Driver Binding Protocol
    //
    Status = gBS->HandleProtocol(
                    HandleBuffer[Index],
                    &gEfiDriverBindingProtocolGuid,
                    (VOID**)&DriverBinding
                    );

    if (EFI_ERROR(Status)) {
      continue;
    }

    //
    // Get Loaded Image Protocol from ImageHandle
    //
    Status = gBS->HandleProtocol(
                    DriverBinding->ImageHandle,
                    &gEfiLoadedImageProtocolGuid,
                    (VOID**)&LoadedImage
                    );

    if (EFI_ERROR(Status)) {
      continue;
    }

    Print(L"Driver Handle : %p\r\n", HandleBuffer[Index]);
    Print(L"Image Handle  : %p\r\n", DriverBinding->ImageHandle);
    Print(L"Version       : 0x%08X\r\n", DriverBinding->Version);

    //
    // Try modern ComponentName2 protocol first
    //
    NameStatus = gBS->HandleProtocol(
                    HandleBuffer[Index],
                    &gEfiComponentName2ProtocolGuid,
                    (VOID**)&ComponentName2
                    );

    if (!EFI_ERROR(NameStatus)) {
      NameStatus = ComponentName2->GetDriverName(
                                      ComponentName2,
                                      "en",
                                      &DriverName
                                      );
    }

    //
    // Fallback to old ComponentName protocol
    //
    if (EFI_ERROR(NameStatus)) {
      NameStatus = gBS->HandleProtocol(
                      HandleBuffer[Index],
                      &gEfiComponentNameProtocolGuid,
                      (VOID**)&ComponentName
                      );

      if (!EFI_ERROR(NameStatus)) {
        NameStatus = ComponentName->GetDriverName(
                                        ComponentName,
                                        "eng",
                                        &DriverName
                                        );
      }
    }

    if (!EFI_ERROR(NameStatus) && DriverName != NULL) {
      Print(L"Driver Name   : %s\r\n", DriverName);
    } else {
      Print(L"Driver Name   : <Unavailable>\r\n");
    }

    //
    // Print Image Path
    //
    if (LoadedImage->FilePath != NULL) {

      DevicePathText = ConvertDevicePathToText(
                         LoadedImage->FilePath,
                         TRUE,
                         TRUE
                         );

      if (DevicePathText != NULL) {
        Print(L"Image Path    : %s\r\n", DevicePathText);
        FreePool(DevicePathText);
      } else {
        Print(L"Image Path    : <Conversion Failed>\r\n");
      }
    } else {
      Print(L"Image Path    : <None>\r\n");
    }

    Print(L"\r\n");

    DriverPrinted++;
  }

  FreePool(HandleBuffer);

  Print(L"===== Enumeration Completed =====\r\n\r\n");

  Print(L"Total Drivers Found : %d\r\n", DriverPrinted);

  return EFI_SUCCESS;
}