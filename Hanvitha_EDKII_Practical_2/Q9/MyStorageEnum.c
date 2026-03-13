#include <Uefi.h>

/*
 * UEFI helper libraries:
 * - UefiLib: Print(), basic UEFI utilities
 * - UefiApplicationEntryPoint: UefiMain() entry point definition
 * - UefiBootServicesTableLib: access to gBS (Boot Services)
 * - DevicePathLib: utilities for handling and converting device paths
 */
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DevicePathLib.h>

/*
 * Protocols used:
 * - Block IO Protocol: access information about block storage devices
 * - Device Path Protocol: retrieve and display device paths
 */
#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>

/*
 * UEFI application entry point
 */
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  EFI_HANDLE *HandleBuffer = NULL;  // Will store handles supporting Block IO
  UINTN HandleCount = 0;             // Number of Block IO handles found

  /*
   * Locate all handles that support the Block IO protocol.
   * This finds all storage devices (disks, partitions, USB drives, etc.).
   */
  Status = gBS->LocateHandleBuffer(
                  ByProtocol,
                  &gEfiBlockIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );

  /*
   * If locating Block IO handles fails, print an error and exit.
   */
  if (EFI_ERROR(Status)) {
    Print(L"Failed to locate Block IO handles\n");
    return Status;
  }

  /*
   * Print total number of storage devices found.
   */
  Print(L"\nTotal Storage Devices Found: %u\n", HandleCount);

  /*
   * Iterate over each handle that supports Block IO.
   */
  for (UINTN Index = 0; Index < HandleCount; Index++) {

    EFI_BLOCK_IO_PROTOCOL *BlockIo;
    EFI_DEVICE_PATH_PROTOCOL *DevicePath;

    /*
     * Retrieve the Block IO protocol from the current handle.
     */
    Status = gBS->HandleProtocol(
                    HandleBuffer[Index],
                    &gEfiBlockIoProtocolGuid,
                    (VOID **)&BlockIo
                    );
    if (EFI_ERROR(Status)) {
      // Skip this handle if Block IO cannot be accessed
      continue;
    }

    /*
     * Print a header for this storage device.
     */
    Print(L"\n====================================\n");
    Print(L"Storage Device %u\n", Index);
    Print(L"====================================\n");

    /*
     * Check whether media is present (e.g., no disk inserted).
     */
    if (!BlockIo->Media->MediaPresent) {
      Print(L"Media Present   : No\n");
      continue;
    }

    /*
     * Print media and device characteristics.
     */
    Print(L"Media Present   : Yes\n");
    Print(L"Removable Media : %d\n", BlockIo->Media->RemovableMedia);
    Print(L"Read Only       : %d\n", BlockIo->Media->ReadOnly);
    Print(L"Block Size      : %u bytes\n", BlockIo->Media->BlockSize);
    Print(L"Last Block      : %lu\n", BlockIo->Media->LastBlock);
    Print(L"Logical Partition: %d\n", BlockIo->Media->LogicalPartition);
    Print(L"Write Caching   : %d\n", BlockIo->Media->WriteCaching);

    /*
     * Calculate total disk size:
     * (LastBlock + 1) * BlockSize gives total size in bytes.
     */
    UINT64 DiskSize;
    DiskSize = (BlockIo->Media->LastBlock + 1) * BlockIo->Media->BlockSize;
    Print(L"Disk Size       : %lu MB\n", DiskSize / (1024 * 1024));

    /*
     * Retrieve the Device Path protocol for this handle.
     * This identifies how the device is connected (PCI, SATA, NVMe, USB, etc.).
     */
    Status = gBS->HandleProtocol(
                    HandleBuffer[Index],
                    &gEfiDevicePathProtocolGuid,
                    (VOID **)&DevicePath
                    );
    if (!EFI_ERROR(Status)) {
      CHAR16 *DevicePathStr;

      /*
       * Convert the binary device path into a human-readable string.
       */
      DevicePathStr = ConvertDevicePathToText(
                        DevicePath,
                        TRUE,
                        TRUE
                        );
      if (DevicePathStr != NULL) {
        Print(L"Device Path     : %s\n", DevicePathStr);
      }
    }
  }

  /*
   * Free the handle buffer allocated by LocateHandleBuffer.
   */
  if (HandleBuffer) {
    gBS->FreePool(HandleBuffer);
  }

  /*
   * Indicate completion of storage enumeration.
   */
  Print(L"\nStorage enumeration completed.\n");
  return EFI_SUCCESS;
}