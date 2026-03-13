#include <PiSmm.h>

#include <Library/DebugLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/BaseLib.h>

#include <Protocol/SmmSwDispatch2.h>

STATIC EFI_HANDLE mSwHandlerHandle = NULL;

/**
  SW SMI callback that triggers a system reset through the reset port.

  @param[in]  DispatchHandle     The handle returned during registration.
  @param[in]  Context            The context data that triggered the SMI event.
  @param[in]  CommBuffer         Optional communication buffer (unused).
  @param[in]  CommBufferSize     Size of optional buffer (unused).

  @retval EFI_SUCCESS            Always returns success after scheduling reset.
**/
EFI_STATUS
EFIAPI
SmmResetCallback (
  IN EFI_HANDLE                        DispatchHandle,
  IN CONST EFI_SMM_SW_REGISTER_CONTEXT *Context,
  IN OUT VOID                          *CommBuffer OPTIONAL,
  IN OUT UINTN                         *CommBufferSize OPTIONAL
  )
{
  DEBUG ((DEBUG_INFO, "SMM Reset Callback: Received SW SMI, issuing reset.\n"));

  //
  // Write to the reset control port to initiate reset on QEMU/OVMF.
  //
  IoWrite8 (0xCF9, 0x06);

  //
  // Halt CPU if reset does not occur immediately.
  //
  CpuDeadLoop ();

  return EFI_SUCCESS;
}

/**
  Entry point that installs the SW SMI handler.

  @param[in]  ImageHandle      Handle for this driver image.
  @param[in]  SystemTable      Pointer to the system table.

  @retval EFI_SUCCESS         Handler registered successfully.
  @retval other error         Registration failed.
**/
EFI_STATUS
EFIAPI
InstallSmmResetHandler (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                     Status;
  EFI_SMM_SW_DISPATCH2_PROTOCOL *SwDispatchProtocol;
  EFI_SMM_SW_REGISTER_CONTEXT    RegisterContext;

  //
  // Locate the SMM SW Dispatch protocol.
  //
  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmSwDispatch2ProtocolGuid,
                    NULL,
                    (VOID **)&SwDispatchProtocol
                    );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate SW Dispatch protocol: %r\n", Status));
    return Status;
  }

  //
  // Specify the SW SMI value that triggers the callback.
  //
  RegisterContext.SwSmiInputValue = 0x77;

  //
  // Register the callback with the SMM dispatcher.
  //
  Status = SwDispatchProtocol->Register (
                                 SwDispatchProtocol,
                                 SmmResetCallback,
                                 &RegisterContext,
                                 &mSwHandlerHandle
                                 );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to register SW SMI handler: %r\n", Status));
  }

  return Status;
}