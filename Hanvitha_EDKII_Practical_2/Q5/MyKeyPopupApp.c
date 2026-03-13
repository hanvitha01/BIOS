#include <Uefi.h>

#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_INPUT_KEY Key;
  CHAR16        Buffer[64];

  SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
  Print(L"Press keys (ESC to exit)...\n");

  while (TRUE) {

    //
    // Wait for key press
    //
    gBS->WaitForEvent(
      1,
      &SystemTable->ConIn->WaitForKey,
      NULL
    );

    //
    // Read key
    //
    if (EFI_ERROR(
          SystemTable->ConIn->ReadKeyStroke(
            SystemTable->ConIn,
            &Key
          ))) {
      continue;
    }

    //
    // ESC → clear screen and exit immediately
    //
    if (Key.ScanCode == SCAN_ESC) {
      SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
      Print(L"ESC pressed. Exiting application.\n");
      return EFI_SUCCESS;
    }

    //
    // Prepare popup text
    //
    if (Key.UnicodeChar != 0) {
      UnicodeSPrint(
        Buffer,
        sizeof(Buffer),
        L"Key Pressed: '%c' (Unicode: 0x%04x)",
        Key.UnicodeChar,
        Key.UnicodeChar
      );
    } else {
      UnicodeSPrint(
        Buffer,
        sizeof(Buffer),
        L"Special Key Pressed (ScanCode: 0x%x)",
        Key.ScanCode
      );
    }

    //
    // Show popup
    //
    CreatePopUp(
      EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE,
      NULL,
      Buffer,
      L"Press ESC to exit",
      NULL
    );

    //
    // Allow user to see popup, then CLEAR it
    //
    gBS->Stall(800000); // 0.8 seconds
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    Print(L"Press keys (ESC to exit)...\n");
  }
}
