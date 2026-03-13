#include <Uefi.h>

#include <Library/UefiDriverEntryPoint.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/BaseLib.h>

#include <Pi/PiHob.h>

EFI_STATUS
EFIAPI
HobDumperDxeEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_PEI_HOB_POINTERS Hob;

  DEBUG ((DEBUG_INFO, "\n===== HobDumper DXE Started =====\n"));

  Hob.Raw = GetHobList();
  if (Hob.Raw == NULL) {
    DEBUG ((DEBUG_ERROR, "HOB list not found!\n"));
    return EFI_NOT_FOUND;
  }

  while (TRUE) {

    DEBUG ((DEBUG_INFO,
      "HOB Type: 0x%04x  Length: %d\n",
      Hob.Header->HobType,
      Hob.Header->HobLength
    ));

    switch (Hob.Header->HobType) {

    case EFI_HOB_TYPE_HANDOFF: {
      EFI_HOB_HANDOFF_INFO_TABLE *Handoff;
      Handoff = Hob.HandoffInformationTable;
      DEBUG ((DEBUG_INFO,
        "  HANDOFF HOB\n"
        "    Boot Mode     : %d\n"
        "    Memory Bottom : 0x%lx\n"
        "    Memory Top    : 0x%lx\n",
        Handoff->BootMode,
        Handoff->EfiMemoryBottom,
        Handoff->EfiMemoryTop
      ));
      break;
    }

    case EFI_HOB_TYPE_RESOURCE_DESCRIPTOR: {
      EFI_HOB_RESOURCE_DESCRIPTOR *Res;
      Res = Hob.ResourceDescriptor;
      DEBUG ((DEBUG_INFO,
        "  RESOURCE DESCRIPTOR\n"
        "    Resource Type : %d\n"
        "    Physical Start: 0x%lx\n"
        "    Resource Size : 0x%lx\n",
        Res->ResourceType,
        Res->PhysicalStart,
        Res->ResourceLength
      ));
      break;
    }

    case EFI_HOB_TYPE_MEMORY_ALLOCATION: {
      EFI_HOB_MEMORY_ALLOCATION *Mem;
      Mem = Hob.MemoryAllocation;
      DEBUG ((DEBUG_INFO,
        "  MEMORY ALLOCATION\n"
        "    Memory Base : 0x%lx\n"
        "    Memory Size : 0x%lx\n"
        "    Memory Type : %d\n",
        Mem->AllocDescriptor.MemoryBaseAddress,
        Mem->AllocDescriptor.MemoryLength,
        Mem->AllocDescriptor.MemoryType
      ));
      break;
    }

    case EFI_HOB_TYPE_FV: {
      EFI_HOB_FIRMWARE_VOLUME *Fv;
      Fv = Hob.FirmwareVolume;
      DEBUG ((DEBUG_INFO,
        "  FIRMWARE VOLUME\n"
        "    FV Base   : 0x%lx\n"
        "    FV Length : 0x%lx\n",
        Fv->BaseAddress,
        Fv->Length
      ));
      break;
    }

    case EFI_HOB_TYPE_CPU: {
      EFI_HOB_CPU *Cpu;
      Cpu = Hob.Cpu;
      DEBUG ((DEBUG_INFO,
        "  CPU HOB\n"
        "    Memory Space Size : %d bits\n"
        "    IO Space Size     : %d bits\n",
        Cpu->SizeOfMemorySpace,
        Cpu->SizeOfIoSpace
      ));
      break;
    }

    case EFI_HOB_TYPE_GUID_EXTENSION: {
      EFI_HOB_GUID_TYPE *GuidHob;
      GuidHob = Hob.Guid;
      DEBUG ((DEBUG_INFO,
        "  GUID HOB : %g\n",
        &GuidHob->Name
      ));
      break;
    }

    case EFI_HOB_TYPE_END_OF_HOB_LIST:
      DEBUG ((DEBUG_INFO, "===== END OF HOB LIST =====\n"));
      DEBUG ((DEBUG_INFO, "===== HobDumper DXE Finished =====\n"));
      return EFI_SUCCESS;

    default:
      DEBUG ((DEBUG_INFO, "  Unknown HOB Type (ignored)\n"));
      break;
    }

    Hob.Raw = GET_NEXT_HOB (Hob);
  }
}
