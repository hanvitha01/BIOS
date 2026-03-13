#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/LoadedImage.h>
#include <Guid/GlobalVariable.h>


EFI_STATUS EFIAPI UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
)
{
  EFI_STATUS Status;
  UINTN Size = 0;
  UINT32 Attributes;
  UINT16 *BootOrder;
  UINT16 Temp;

  // Get size
  Status = gRT->GetVariable(
    L"BootOrder",
    &gEfiGlobalVariableGuid,
    NULL,
    &Size,
    NULL
  );

  if (Status != EFI_BUFFER_TOO_SMALL) {
    Print(L"Failed to get BootOrder size\n");
    return Status;
  }

  BootOrder = AllocateZeroPool(Size);
  if (!BootOrder) {
    return EFI_OUT_OF_RESOURCES;
  }

  // Read variable
  Status = gRT->GetVariable(
    L"BootOrder",
    &gEfiGlobalVariableGuid,
    &Attributes,
    &Size,
    BootOrder
  );

  if (EFI_ERROR(Status)) {
    Print(L"Failed to read BootOrder\n");
    FreePool(BootOrder);
    return Status;
  }

  if (Size < 2 * sizeof(UINT16)) {
    Print(L"Not enough boot entries to swap\n");
    FreePool(BootOrder);
    return EFI_ABORTED;
  }

  // Swap first two
  Temp = BootOrder[0];
  BootOrder[0] = BootOrder[1];
  BootOrder[1] = Temp;

  // Write back
  Status = gRT->SetVariable(
    L"BootOrder",
    &gEfiGlobalVariableGuid,
    Attributes,
    Size,
    BootOrder
  );

  if (EFI_ERROR(Status)) {
    Print(L"Failed to update BootOrder\n");
  } else {
    Print(L"BootOrder swapped successfully\n");
  }

  FreePool(BootOrder);
  return Status;
}
