#include <Uefi.h>

/* Libraries */
#include <Library/UefiBootServicesTableLib.h>   // gBS access
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>                   // DEBUG macros
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

/* Protocols */
#include <Protocol/FirmwareVolumeBlock.h>       // FVB protocol

/* PI / FV structures */
#include <Pi/PiFirmwareVolume.h>                // FV header structure
#include <Pi/PiFirmwareFile.h>                  // FFS file structure

// Align value to 8-byte boundary
#define ALIGN8(Value)  (((Value) + 7) & ~7)

EFI_STATUS
EFIAPI
FvFreeSpaceDxeEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  DEBUG((DEBUG_ERROR, "MyFwVolDxe: ENTERED DRIVER\n"));

  EFI_STATUS Status;
  EFI_HANDLE *Handles = NULL;
  UINTN       HandleCount = 0;
  UINTN       Index;

  // Get all handles that support Firmware Volume Block protocol
  Status = gBS->LocateHandleBuffer(
                  ByProtocol,
                  &gEfiFirmwareVolumeBlockProtocolGuid,
                  NULL,
                  &HandleCount,
                  &Handles
                  );
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR,
      "FvFreeSpaceDxe: LocateHandleBuffer failed: %r\n",
      Status
    ));
    return Status;
  }

  // Loop through each Firmware Volume
  for (Index = 0; Index < HandleCount; Index++) {

    EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL *Fvb;
    EFI_FIRMWARE_VOLUME_HEADER          FvHeader;
    UINTN                               Size;
    UINT64                              FvLength;
    UINTN                               Offset;
    BOOLEAN                             IsDxeFv = FALSE;

    // Get FVB protocol for this handle
    Status = gBS->HandleProtocol(
                    Handles[Index],
                    &gEfiFirmwareVolumeBlockProtocolGuid,
                    (VOID **)&Fvb
                    );
    if (EFI_ERROR(Status)) {
      continue;
    }

    // Read Firmware Volume header
    Size = sizeof(EFI_FIRMWARE_VOLUME_HEADER);
    Status = Fvb->Read(
                    Fvb,
                    0,
                    0,
                    &Size,
                    (UINT8 *)&FvHeader
                    );

    // Validate FV signature
    if (EFI_ERROR(Status) || FvHeader.Signature != EFI_FVH_SIGNATURE) {
      continue;
    }

    FvLength = FvHeader.FvLength;
    Offset   = ALIGN8(FvHeader.HeaderLength);

    /* -------- First pass: check if this is DXE FV -------- */
    {
      UINTN ScanOffset = Offset;

      while (ScanOffset + sizeof(EFI_FFS_FILE_HEADER) < FvLength) {

        EFI_FFS_FILE_HEADER Ffs;
        UINTN               ReadSize;
        UINTN               FileSize;

        ReadSize = sizeof(EFI_FFS_FILE_HEADER);
        Status = Fvb->Read(Fvb, 0, ScanOffset, &ReadSize, (UINT8 *)&Ffs);

        // Stop if invalid or empty region
        if (EFI_ERROR(Status) || *(UINT32 *)&Ffs == 0xFFFFFFFF) {
          break;
        }

        // Get file size (3-byte field)
        FileSize =
          Ffs.Size[0] |
          (Ffs.Size[1] << 8) |
          (Ffs.Size[2] << 16);

        // Check if DXE Core file exists
        if (Ffs.Type == EFI_FV_FILETYPE_DXE_CORE) {
          IsDxeFv = TRUE;
          break;
        }

        // Move to next file
        ScanOffset += ALIGN8(FileSize);
      }
    }

    // Skip if not DXE Firmware Volume
    if (!IsDxeFv) {
      continue;
    }

    DEBUG((DEBUG_INFO,
      "\n--- DXE FIRMWARE VOLUME FOUND ---\nFV Length : %Lu bytes\n",
      FvLength
    ));

    /* -------- Second pass: calculate used space -------- */
    while (Offset + sizeof(EFI_FFS_FILE_HEADER) < FvLength) {

      EFI_FFS_FILE_HEADER Ffs;
      UINTN               ReadSize;
      UINTN               FileSize;

      ReadSize = sizeof(EFI_FFS_FILE_HEADER);
      Status = Fvb->Read(Fvb, 0, Offset, &ReadSize, (UINT8 *)&Ffs);

      // Stop if empty region
      if (EFI_ERROR(Status) || *(UINT32 *)&Ffs == 0xFFFFFFFF) {
        break;
      }

      // Get file size
      FileSize =
        Ffs.Size[0] |
        (Ffs.Size[1] << 8) |
        (Ffs.Size[2] << 16);

      // Move offset forward
      Offset += ALIGN8(FileSize);
    }

    // Calculate used and free space
    {
      UINT64 UsedSize = Offset;
      UINT64 FreeSize = FvLength - UsedSize;
      UINTN  FreePct  = (UINTN)((FreeSize * 100) / FvLength);

      DEBUG((DEBUG_INFO,
        "Used : %Lu bytes\nFree : %Lu bytes (%u%%)\n",
        UsedSize, FreeSize, FreePct
      ));

      // Simple check for large free space
      if (FreePct > 50) {
        DEBUG((DEBUG_WARN,
          "Free space > 50%% — Eligible for NEW FV creation\n"
        ));
      }
    }
  }

  // Free allocated handle buffer
  FreePool(Handles);

  return EFI_SUCCESS;
}
