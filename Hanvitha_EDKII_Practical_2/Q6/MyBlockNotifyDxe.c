#include <Uefi.h>

// Provides access to gBS (Boot Services) and related helpers
#include <Library/UefiBootServicesTableLib.h>

// DEBUG() macro support
#include <Library/DebugLib.h>

// For ConvertDevicePathToText()
#include <Library/DevicePathLib.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>

// Event that will be signaled when Block I/O protocol is installed
EFI_EVENT  gBlockIoEvent;

// Registration key returned by RegisterProtocolNotify()
// Used by LocateHandle(ByRegisterNotify, ...)
VOID      *gBlockIoRegistration;


/**
  Block I/O notify callback

  This function is called whenever a new handle installs
  the EFI_BLOCK_IO_PROTOCOL.

  It processes all new handles that triggered the notification.
**/
VOID
EFIAPI
BlockIoNotifyCallback (
  IN EFI_EVENT Event,
  IN VOID      *Context
  )
{
  EFI_STATUS               Status;
  EFI_HANDLE               Handle;
  UINTN                    BufferSize;
  EFI_BLOCK_IO_PROTOCOL    *BlockIo;
  EFI_DEVICE_PATH_PROTOCOL *DevicePath;
  CHAR16                   *DevicePathStr;

  // Loop to retrieve all handles that triggered the notification
  while (TRUE) {

    // We expect to retrieve one EFI_HANDLE at a time
    BufferSize = sizeof(EFI_HANDLE);

    // Get next handle that installed Block I/O protocol
    Status = gBS->LocateHandle (
                    ByRegisterNotify,          // Retrieve handles registered by notification
                    NULL,
                    gBlockIoRegistration,      
                    &BufferSize,
                    &Handle
                    );

    // If no more handles, exit loop
    if (EFI_ERROR(Status)) {
      break;
    }

    // Get the Block I/O protocol from the handle
    Status = gBS->HandleProtocol (
                    Handle,
                    &gEfiBlockIoProtocolGuid,
                    (VOID **)&BlockIo
                    );
    if (EFI_ERROR(Status)) {
      continue;   // Skip if protocol not accessible
    }

    // Skip logical partitions (like GPT/MBR partitions)
    // Only process whole physical disks
    if (BlockIo->Media->LogicalPartition) {
      continue;
    }

    // Get Device Path protocol from the same handle
    Status = gBS->HandleProtocol (
                    Handle,
                    &gEfiDevicePathProtocolGuid,
                    (VOID **)&DevicePath
                    );
    if (EFI_ERROR(Status)) {
      continue;
    }

    // Convert binary device path into human-readable string
    DevicePathStr = ConvertDevicePathToText (
                      DevicePath,
                      TRUE,   // Display full device path
                      TRUE    // Display shortened form if possible
                      );

    // Print disk detection info
    if (DevicePathStr != NULL) {
      DEBUG((DEBUG_INFO, "\n[DXE] Disk Detected\n"));
      DEBUG((DEBUG_INFO, "Device Path: %s\n", DevicePathStr));
    }
  }
}


/**
  Check for Block I/O protocols that already exist

  When this driver loads, some disks may already have installed
  Block I/O protocol. This function ensures we process them too.
**/
VOID
CheckExistingBlockIo (
  VOID
  )
{
  EFI_STATUS  Status;
  EFI_HANDLE  *Handles;
  UINTN       HandleCount;

  // Locate all handles that already support Block I/O protocol
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiBlockIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &Handles
                  );

  // If at least one Block I/O handle exists
  if (!EFI_ERROR(Status) && HandleCount > 0) {

    DEBUG((DEBUG_INFO, "[DXE] Existing Block I/O found\n"));

    // Manually signal the event so callback processes them
    gBS->SignalEvent(gBlockIoEvent);
  }
}


/**
  DXE driver entry point

  This function runs when the DXE driver is loaded.
  It registers for Block I/O protocol notifications.
**/
EFI_STATUS
EFIAPI
MyBlockNotifyDxeEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;

  DEBUG((DEBUG_INFO, "[DXE] Block I/O Notify Driver Loaded\n"));

  // Create an event that will trigger our callback
  // whenever it is signaled
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,      // Event type
                  TPL_CALLBACK,           // Task priority level
                  BlockIoNotifyCallback,  // Callback function
                  NULL,
                  &gBlockIoEvent
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  // Register for notification when Block I/O protocol is installed
  Status = gBS->RegisterProtocolNotify (
                  &gEfiBlockIoProtocolGuid,
                  gBlockIoEvent,
                  &gBlockIoRegistration
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  // Handle disks that already exist before this driver was loaded
  CheckExistingBlockIo();

  return EFI_SUCCESS;
}