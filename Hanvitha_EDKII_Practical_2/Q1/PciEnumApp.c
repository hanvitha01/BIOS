#include <Uefi.h>

#include <Library/UefiLib.h>                 // For Print() and other UEFI helper functions
#include <Library/UefiBootServicesTableLib.h> // For gBS (Boot Services global pointer)
#include <Library/PrintLib.h>                // For formatted printing support
#include <Library/BaseLib.h>                 // For base utility macros like BIT0, BIT1, etc.

#include <IndustryStandard/Pci.h>            // PCI standard definitions (register offsets, etc.)
#include <Protocol/PciRootBridgeIo.h>        // PCI Root Bridge I/O protocol definition

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,   // Handle for the loaded image
  IN EFI_SYSTEM_TABLE  *SystemTable   // Pointer to EFI System Table
)
{
  EFI_STATUS Status;   // Used to store return status of UEFI function calls
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo; // Pointer to PCI Root Bridge IO Protocol
  UINT8 Bus, Device, Function; // PCI addressing components
  UINT16 VendorId, DeviceId, Command; // PCI configuration register values
  EFI_INPUT_KEY Key;  // Used to capture keyboard input

  // Locate the PCI Root Bridge IO Protocol which allows access to PCI config space
  Status = gBS->LocateProtocol(
                  &gEfiPciRootBridgeIoProtocolGuid,
                  NULL,
                  (VOID **)&PciRootBridgeIo
                  );
  if (EFI_ERROR(Status)) {
    // If protocol is not found, print error and exit
    Print(L"PCI Root Bridge IO Protocol not found\n");
    return Status;
  }

  Print(L"\nPCI Enumeration Application\n");
  Print(L"===========================\n");

  // Iterate through all possible PCI buses (0-255)
  for (Bus = 0; Bus < 256; Bus++) {

    // Iterate through all possible devices per bus (0-31)
    for (Device = 0; Device < 32; Device++) {

      // Each device can have up to 8 functions (0-7)
      for (Function = 0; Function < 8; Function++) {

        // Read Vendor ID (offset 0x00 in PCI config space)
        Status = PciRootBridgeIo->Pci.Read(
          PciRootBridgeIo,
          EfiPciWidthUint16, 
          EFI_PCI_ADDRESS(Bus, Device, Function, 0), 
          1,
          &VendorId
        );

        // If read failed or no device present, skip
        if (EFI_ERROR(Status) || VendorId == 0xFFFF) {
          continue;
        }

        // Read Device ID (offset 0x02)
        PciRootBridgeIo->Pci.Read(
          PciRootBridgeIo,
          EfiPciWidthUint16,
          EFI_PCI_ADDRESS(Bus, Device, Function, 2),
          1,
          &DeviceId
        );

        // Read Command Register (offset 0x04)
        PciRootBridgeIo->Pci.Read(
          PciRootBridgeIo,
          EfiPciWidthUint16,
          EFI_PCI_ADDRESS(Bus, Device, Function, 4),
          1,
          &Command
        );

        // Print device location and IDs
        Print(L"\nBus %02x Dev %02x Func %x\n", Bus, Device, Function);
        Print(L"Vendor ID : %04x\n", VendorId);
        Print(L"Device ID : %04x\n", DeviceId);
        Print(L"Command   : %04x\n", Command);

        // Loop through 6 Base Address Registers (BAR0 - BAR5)
        for (UINT8 Bar = 0; Bar < 6; Bar++) {
          UINT32 BarVal;

          // Read BAR register (each BAR is 4 bytes apart)
          PciRootBridgeIo->Pci.Read(
            PciRootBridgeIo,
            EfiPciWidthUint32,
            EFI_PCI_ADDRESS(Bus, Device, Function,
              PCI_BASE_ADDRESSREG_OFFSET + Bar * 4),
            1,
            &BarVal
          );

          // If BAR value is zero, skip (unused BAR)
          if (BarVal == 0) continue;

          // Check if BAR indicates I/O space (BIT0 set)
          if (BarVal & BIT0)
            Print(L"BAR%d : I/O @ %08x\n", Bar, BarVal & ~0x3); // Mask lower bits
          else
            Print(L"BAR%d : MMIO @ %08x\n", Bar, BarVal & ~0xF); // Mask lower bits
        }

        // Ask user if they want to disable this device
        Print(L"Disable this device? (y/n): ");

        // Wait until a key is pressed (blocking loop)
        while (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key)
               == EFI_NOT_READY);

        // Print the pressed key
        Print(L"%c\n", Key.UnicodeChar);

        // If user pressed 'y' or 'Y'
        if (Key.UnicodeChar == L'y' || Key.UnicodeChar == L'Y') {

          // Clear bits:
          // BIT0 = I/O Space Enable
          // BIT1 = Memory Space Enable
          // BIT2 = Bus Master Enable
          // Clearing these effectively disables the device
          Command &= ~(BIT0 | BIT1 | BIT2);

          // Write modified command register back to PCI config space
          PciRootBridgeIo->Pci.Write(
            PciRootBridgeIo,
            EfiPciWidthUint16,
            EFI_PCI_ADDRESS(Bus, Device, Function, 4),
            1,
            &Command
          );

          Print(L"Device disabled\n");
        }
      }
    }
  }

  Print(L"\nPCI enumeration finished\n");
  return EFI_SUCCESS; // Application completed successfully
}