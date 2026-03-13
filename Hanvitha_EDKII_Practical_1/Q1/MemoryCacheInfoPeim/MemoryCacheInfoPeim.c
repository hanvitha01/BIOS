#include <Uefi.h>
#include <PiPei.h>

#include <Library/DebugLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/HobLib.h>
#include <Library/BaseLib.h>

#include <Guid/HobList.h>

/*
 * Get total DRAM size from EFI_RESOURCE_SYSTEM_MEMORY HOBs
 */
STATIC
UINT64
GetTotalDramSize (
  VOID
  )
{
  EFI_PEI_HOB_POINTERS Hob;
  UINT64 TotalMemory = 0;

  Hob.Raw = GetHobList ();

  while (!END_OF_HOB_LIST (Hob)) {
    if (Hob.Header->HobType == EFI_HOB_TYPE_RESOURCE_DESCRIPTOR) {

      EFI_HOB_RESOURCE_DESCRIPTOR *ResHob = Hob.ResourceDescriptor;

      if (ResHob->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) {
        TotalMemory += ResHob->ResourceLength;
      }
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
  }

  return TotalMemory;
}

/*
 * Get L1 Cache size in KB using extended CPUID 0x80000005
 */
STATIC
UINT32
GetL1CacheSizeKb (
  VOID
  )
{
  UINT32 Eax, Ebx, Ecx, Edx;

  AsmCpuid (0x80000005, &Eax, &Ebx, &Ecx, &Edx);

  // ECX[31:24] = L1 Data cache in KB
  // EDX[31:24] = L1 Instruction cache in KB
  UINT32 L1DataKb  = (Ecx >> 24) & 0xFF;
  UINT32 L1InstrKb = (Edx >> 24) & 0xFF;

  return L1DataKb + L1InstrKb;
}

/*
 * Get L2 + L3 Cache size in KB using extended CPUID 0x80000006
 */
STATIC
UINT32
GetL2L3CacheSizeKb (
  VOID
  )
{
  UINT32 Eax, Ebx, Ecx, Edx;

  // Check if 0x80000006 is supported
  AsmCpuid (0x80000000, &Eax, &Ebx, &Ecx, &Edx);
  if (Eax < 0x80000006) {
    return 0;
  }

  AsmCpuid (0x80000006, &Eax, &Ebx, &Ecx, &Edx);

  // ECX[31:16] = L2 cache size in KB
  UINT32 L2Kb = (Ecx >> 16) & 0xFFFF;

  // EDX[31:18] = L3 cache size in 512KB units
  UINT32 L3Kb = ((Edx >> 18) & 0x3FFF) * 512;

  return L2Kb + L3Kb;
}

/*
 * PEIM Entry Point
 */
EFI_STATUS
EFIAPI
MemoryCacheInfoPeimEntry (
  IN EFI_PEI_FILE_HANDLE FileHandle,
  IN CONST EFI_PEI_SERVICES **PeiServices
  )
{
  DEBUG ((DEBUG_INFO, "MemoryCacheInfoPeim: PEIM Loaded Successfully\n"));

  // DRAM Size
  UINT64 DramBytes = GetTotalDramSize ();
  UINT64 DramMb = DramBytes / (1024 * 1024);

  DEBUG ((DEBUG_INFO, "Total DRAM Size: %llu MB\n", DramMb));

  // Cache Size
  UINT32 L1Kb   = GetL1CacheSizeKb ();
  UINT32 L2L3Kb = GetL2L3CacheSizeKb ();

  DEBUG ((DEBUG_INFO, "L1 Cache Size : %u KB\n", L1Kb));
  DEBUG ((DEBUG_INFO, "L2 + L3 Cache : %u KB\n", L2L3Kb));
  DEBUG ((DEBUG_INFO, "Total Cache   : %u KB\n", L1Kb + L2L3Kb));

  return EFI_SUCCESS;
}
